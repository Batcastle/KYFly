// SPDX-License-Identifier: GPL-3.0-or-later
#include "rtspworker.h"

#include <QHostAddress>
#include <QRandomGenerator>
#include <QTcpSocket>
#include <QTimer>
#include <QUdpSocket>

namespace {
constexpr int kReconnectDelayMs = 500;   // same as the official app
constexpr int kFailRetryDelayMs = 2000;
constexpr int kNoDataTimeoutMs = 3000;
constexpr int kRtcpIntervalMs = 5000;
constexpr int kUdpRcvBuf = 2 * 1024 * 1024;
const QByteArray kUserAgent = "kyfly/0.1";
} // namespace

RtspWorker::RtspWorker(QObject *parent)
    : QObject(parent)
    , m_keepAlive(new QTimer(this))
    , m_watchdog(new QTimer(this))
    , m_stats(new QTimer(this))
    , m_reconnect(new QTimer(this))
{
    m_decoder = new FrameDecoder(this);
    // Frames go straight from the decoder thread to whoever is listening
    // (queued into the GUI thread there): one hop, no detour via this thread.
    connect(m_decoder, &FrameDecoder::frameReady, this, &RtspWorker::frameReady, Qt::DirectConnection);
    connect(m_decoder, &FrameDecoder::frameReady, this, [this](const QImage &img) {
        if (img.size() != m_lastSize) {
            m_lastSize = img.size();
            emit stateChanged(Playing, tr("%1×%2 MJPEG").arg(img.width()).arg(img.height()));
        }
    }, Qt::QueuedConnection);
    m_ssrc = QRandomGenerator::global()->generate();
    m_keepAlive->setInterval(kRtcpIntervalMs);
    m_watchdog->setInterval(500);
    m_stats->setInterval(1000);
    m_reconnect->setSingleShot(true);
    connect(m_keepAlive, &QTimer::timeout, this, &RtspWorker::onKeepAlive);
    connect(m_watchdog, &QTimer::timeout, this, &RtspWorker::onWatchdog);
    connect(m_stats, &QTimer::timeout, this, &RtspWorker::onStatsTick);
    connect(m_reconnect, &QTimer::timeout, this, &RtspWorker::reconnect);
}

RtspWorker::~RtspWorker()
{
    teardownSockets(false);
    m_decoder->shutdown();
}

void RtspWorker::start(const QString &url, bool useTcp)
{
    teardownSockets(true);
    m_url = url;
    m_useTcp = useTcp;
    m_running = true;
    if (!m_decoder->isRunning())
        m_decoder->start();
    m_depay.reset();
    m_prevLost = m_prevDropped = 0;
    m_rxFrames = 0;
    m_rcvBuf = 0;
    m_decoder->takeStats();
    m_lastSize = QSize();

    const QUrl u(url);
    if (!u.isValid() || u.scheme() != QLatin1String("rtsp") || u.host().isEmpty()) {
        fail(tr("Invalid stream URL: %1").arg(url));
        return;
    }

    emit stateChanged(Connecting, tr("Connecting to %1…").arg(u.host()));
    m_tcp = new QTcpSocket(this);
    connect(m_tcp, &QTcpSocket::connected, this, &RtspWorker::onConnected);
    connect(m_tcp, &QTcpSocket::readyRead, this, &RtspWorker::onTcpReadyRead);
    connect(m_tcp, &QAbstractSocket::errorOccurred, this, &RtspWorker::onTcpError);
    m_tcp->connectToHost(u.host(), quint16(u.port(554)));

    m_lastPacket.restart();
    m_statsClock.restart();
    m_watchdog->start();
    m_stats->start();
}

void RtspWorker::stop()
{
    m_running = false;
    m_keepAlive->stop();
    m_watchdog->stop();
    m_stats->stop();
    m_reconnect->stop();
    teardownSockets(true);
    emit stateChanged(Idle, tr("Stopped"));
}

void RtspWorker::reconnect()
{
    if (m_running)
        start(m_url, m_useTcp);
}

void RtspWorker::fail(const QString &msg)
{
    m_keepAlive->stop();
    teardownSockets(false);
    emit stateChanged(Failed, msg);
    if (m_running)
        m_reconnect->start(kFailRetryDelayMs);
}

void RtspWorker::teardownSockets(bool sendTeardown)
{
    if (m_tcp) {
        if (sendTeardown && !m_session.isEmpty() &&
            m_tcp->state() == QAbstractSocket::ConnectedState) {
            sendRequest(QStringLiteral("TEARDOWN"), m_url);
            m_tcp->flush();
        }
        m_tcp->disconnect(this);
        m_tcp->abort();
        m_tcp->deleteLater();
        m_tcp = nullptr;
    }
    for (QUdpSocket **s : {&m_rtp, &m_rtcp}) {
        if (*s) {
            (*s)->disconnect(this);
            (*s)->close();
            (*s)->deleteLater();
            *s = nullptr;
        }
    }
    m_rx.clear();
    m_session.clear();
    m_step = StepNone;
    m_keepAlive->stop();
}

void RtspWorker::sendRequest(const QString &method, const QString &uri,
                             const QList<QPair<QByteArray, QByteArray>> &extra)
{
    if (!m_tcp)
        return;
    QByteArray req = method.toLatin1() + ' ' + uri.toLatin1() + " RTSP/1.0\r\n";
    req += "CSeq: " + QByteArray::number(++m_cseq) + "\r\n";
    req += "User-Agent: " + kUserAgent + "\r\n";
    if (!m_session.isEmpty())
        req += "Session: " + m_session + "\r\n";
    for (const auto &h : extra)
        req += h.first + ": " + h.second + "\r\n";
    req += "\r\n";
    m_tcp->write(req);
}

void RtspWorker::onConnected()
{
    m_tcp->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    m_step = StepOptions;
    sendRequest(QStringLiteral("OPTIONS"), m_url);
}

void RtspWorker::onTcpError()
{
    if (!m_tcp)
        return;
    const QString err = m_tcp->errorString();
    fail(tr("Stream connection error: %1").arg(err));
}

void RtspWorker::onTcpReadyRead()
{
    if (!m_tcp)
        return;
    m_rx += m_tcp->readAll();
    m_lastPacket.restart();

    while (!m_rx.isEmpty()) {
        if (m_rx.at(0) == '$') { // interleaved RTP/RTCP (TCP transport)
            if (m_rx.size() < 4)
                break;
            const int ch = uchar(m_rx.at(1));
            const int len = (uchar(m_rx.at(2)) << 8) | uchar(m_rx.at(3));
            if (m_rx.size() < 4 + len)
                break;
            if (ch == 0)
                handleRtp(reinterpret_cast<const uint8_t *>(m_rx.constData() + 4), len);
            m_rx.remove(0, 4 + len);
            continue;
        }

        const int hdrEnd = m_rx.indexOf("\r\n\r\n");
        if (hdrEnd < 0) {
            if (m_rx.size() > 64 * 1024) {
                fail(tr("Garbage on RTSP connection"));
                return;
            }
            break;
        }
        const QList<QByteArray> lines = m_rx.left(hdrEnd).split('\n');
        QHash<QByteArray, QByteArray> headers;
        for (int i = 1; i < lines.size(); ++i) {
            const QByteArray line = lines.at(i).trimmed();
            const int colon = line.indexOf(':');
            if (colon > 0)
                headers.insert(line.left(colon).trimmed().toLower(), line.mid(colon + 1).trimmed());
        }
        const int contentLength = headers.value("content-length", "0").toInt();
        if (m_rx.size() < hdrEnd + 4 + contentLength)
            break;
        const QByteArray body = m_rx.mid(hdrEnd + 4, contentLength);
        const QByteArray status = lines.value(0).trimmed();
        m_rx.remove(0, hdrEnd + 4 + contentLength);

        if (!status.startsWith("RTSP/"))
            continue; // server-initiated request or junk: ignore
        const QList<QByteArray> parts = status.split(' ');
        handleResponse(parts.value(1).toInt(), headers, body);
        if (!m_tcp)
            return; // handleResponse failed and tore everything down
    }
}

void RtspWorker::handleResponse(int code, const QHash<QByteArray, QByteArray> &headers,
                                const QByteArray &body)
{
    switch (m_step) {
    case StepOptions:
        // Some cameras answer OPTIONS badly; DESCRIBE is what matters.
        m_step = StepDescribe;
        sendRequest(QStringLiteral("DESCRIBE"), m_url, {{"Accept", "application/sdp"}});
        return;

    case StepDescribe: {
        if (code != 200) {
            fail(tr("DESCRIBE failed (RTSP %1)").arg(code));
            return;
        }
        QString base = QString::fromLatin1(headers.value("content-base"));
        if (base.isEmpty())
            base = QString::fromLatin1(headers.value("content-location"));
        if (base.isEmpty())
            base = m_url;
        if (!parseSdp(body, base))
            return;
        m_step = StepSetup;
        if (m_useTcp) {
            sendRequest(QStringLiteral("SETUP"), m_controlUrl,
                        {{"Transport", "RTP/AVP/TCP;unicast;interleaved=0-1"}});
        } else {
            if (!bindUdpPair()) {
                fail(tr("Could not open local UDP ports"));
                return;
            }
            const QByteArray t = "RTP/AVP;unicast;client_port=" +
                                 QByteArray::number(m_rtp->localPort()) + '-' +
                                 QByteArray::number(m_rtcp->localPort());
            sendRequest(QStringLiteral("SETUP"), m_controlUrl, {{"Transport", t}});
        }
        return;
    }

    case StepSetup: {
        if (code == 461 && !m_useTcp) { // Unsupported transport -> fall back to TCP
            m_useTcp = true;
            sendRequest(QStringLiteral("SETUP"), m_controlUrl,
                        {{"Transport", "RTP/AVP/TCP;unicast;interleaved=0-1"}});
            return;
        }
        if (code != 200) {
            fail(tr("SETUP failed (RTSP %1)").arg(code));
            return;
        }
        const QByteArray session = headers.value("session");
        const QList<QByteArray> sp = session.split(';');
        m_session = sp.value(0).trimmed();
        for (int i = 1; i < sp.size(); ++i) {
            const QByteArray kv = sp.at(i).trimmed();
            if (kv.startsWith("timeout="))
                m_sessionTimeout = qMax(10, kv.mid(8).toInt());
        }
        const QByteArray transport = headers.value("transport");
        const int sp1 = transport.indexOf("server_port=");
        if (sp1 >= 0) {
            QByteArray ports = transport.mid(sp1 + 12);
            ports = ports.left(ports.indexOf(';') < 0 ? ports.size() : ports.indexOf(';'));
            const QList<QByteArray> pp = ports.split('-');
            m_serverRtcpPort = quint16(pp.size() > 1 ? pp.at(1).toInt() : pp.at(0).toInt() + 1);
        }
        m_step = StepPlay;
        sendRequest(QStringLiteral("PLAY"), m_url, {{"Range", "npt=0.000-"}});
        return;
    }

    case StepPlay:
        if (code != 200) {
            fail(tr("PLAY failed (RTSP %1)").arg(code));
            return;
        }
        m_step = StepPlaying;
        emit stateChanged(Connecting, tr("Waiting for video…"));
        sendRtcpReceiverReport();
        m_keepAlive->start();
        return;

    case StepPlaying:
    case StepNone:
        return; // keep-alive replies
    }
}

bool RtspWorker::parseSdp(const QByteArray &sdp, const QString &baseUrl)
{
    bool inVideo = false, sawVideo = false;
    QString control, codec;
    int pt = -1, xw = 0, xh = 0;

    for (QByteArray line : sdp.split('\n')) {
        line = line.trimmed();
        if (line.startsWith("m=")) {
            if (sawVideo)
                break; // only the first video stream
            inVideo = line.startsWith("m=video");
            if (inVideo) {
                sawVideo = true;
                const QList<QByteArray> f = line.split(' ');
                pt = f.value(3).toInt();
            }
            continue;
        }
        if (!inVideo)
            continue;
        if (line.startsWith("a=control:"))
            control = QString::fromLatin1(line.mid(10));
        else if (line.startsWith("a=rtpmap:"))
            codec = QString::fromLatin1(line.mid(line.indexOf(' ') + 1)).section('/', 0, 0).toUpper();
        else if (line.startsWith("a=x-dimensions:")) {
            const QList<QByteArray> d = line.mid(15).split(',');
            xw = d.value(0).toInt();
            xh = d.value(1).toInt();
        } else if (line.startsWith("a=framesize:")) {
            const QList<QByteArray> d = line.mid(line.indexOf(' ') + 1).split('-');
            xw = d.value(0).toInt();
            xh = d.value(1).toInt();
        }
    }

    if (!sawVideo) {
        fail(tr("Stream has no video track"));
        return false;
    }
    m_isJpeg = (pt == 26) || codec == QLatin1String("JPEG");
    if (!m_isJpeg) {
        fail(tr("Stream codec is %1, but this build only decodes MJPEG")
                 .arg(codec.isEmpty() ? QString::number(pt) : codec));
        return false;
    }
    m_depay.setFallbackSize(xw, xh);

    if (control.isEmpty() || control == QLatin1String("*"))
        m_controlUrl = baseUrl;
    else if (control.startsWith(QLatin1String("rtsp://")))
        m_controlUrl = control;
    else
        m_controlUrl = baseUrl + (baseUrl.endsWith('/') ? QString() : QStringLiteral("/")) + control;
    return true;
}

bool RtspWorker::bindUdpPair()
{
    for (int attempt = 0; attempt < 32; ++attempt) {
        const quint16 port = quint16(40000 + 2 * QRandomGenerator::global()->bounded(10000));
        auto *rtp = new QUdpSocket(this);
        auto *rtcp = new QUdpSocket(this);
        if (rtp->bind(QHostAddress::AnyIPv4, port) && rtcp->bind(QHostAddress::AnyIPv4, port + 1)) {
            rtp->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, kUdpRcvBuf);
            // The kernel may cap this (net.core.rmem_max); report what we got.
            m_rcvBuf = rtp->socketOption(QAbstractSocket::ReceiveBufferSizeSocketOption).toInt();
            m_rtp = rtp;
            m_rtcp = rtcp;
            connect(m_rtp, &QUdpSocket::readyRead, this, &RtspWorker::onRtpReadyRead);
            connect(m_rtcp, &QUdpSocket::readyRead, this, &RtspWorker::onRtcpReadyRead);
            return true;
        }
        delete rtp;
        delete rtcp;
    }
    return false;
}

void RtspWorker::onRtpReadyRead()
{
    while (m_rtp && m_rtp->hasPendingDatagrams()) {
        const qint64 size = m_rtp->pendingDatagramSize();
        m_scratch.resize(int(qMax<qint64>(size, 0)));
        const qint64 got = m_rtp->readDatagram(m_scratch.data(), m_scratch.size());
        if (got > 0)
            handleRtp(reinterpret_cast<const uint8_t *>(m_scratch.constData()), int(got));
    }
}

void RtspWorker::onRtcpReadyRead()
{
    while (m_rtcp && m_rtcp->hasPendingDatagrams()) {
        char buf[1500];
        m_rtcp->readDatagram(buf, sizeof buf);
    }
}

void RtspWorker::handleRtp(const uint8_t *data, int len)
{
    m_lastPacket.restart();
    QByteArray jpeg;
    if (m_depay.push(data, len, jpeg)) {
        ++m_rxFrames;
        m_decoder->submit(jpeg); // never blocks; decoder keeps only the newest
    }
}

void RtspWorker::sendRtcpReceiverReport()
{
    if (m_useTcp || !m_rtcp || !m_tcp || m_serverRtcpPort == 0)
        return;
    QByteArray p;
    auto put32 = [&p](quint32 v) {
        p.append(char(v >> 24)); p.append(char(v >> 16)); p.append(char(v >> 8)); p.append(char(v));
    };
    // RR with no report blocks
    p.append(char(0x80)); p.append(char(201)); p.append(char(0)); p.append(char(1));
    put32(m_ssrc);
    // SDES CNAME
    const QByteArray cname = "kyfly";
    QByteArray chunk;
    chunk.append(char(1)); chunk.append(char(cname.size())); chunk.append(cname); chunk.append(char(0));
    while ((4 + chunk.size()) % 4)
        chunk.append(char(0));
    const int words = (4 + 4 + chunk.size()) / 4 - 1;
    p.append(char(0x81)); p.append(char(202)); p.append(char(words >> 8)); p.append(char(words));
    put32(m_ssrc);
    p.append(chunk);
    m_rtcp->writeDatagram(p, m_tcp->peerAddress(), m_serverRtcpPort);
}

void RtspWorker::onKeepAlive()
{
    if (m_step != StepPlaying)
        return;
    sendRtcpReceiverReport();
    if (++m_keepAliveTicks * kRtcpIntervalMs >= m_sessionTimeout * 500) { // every timeout/2
        m_keepAliveTicks = 0;
        sendRequest(QStringLiteral("OPTIONS"), m_url);
    }
}

void RtspWorker::onWatchdog()
{
    if (!m_running || m_reconnect->isActive())
        return;
    if (m_lastPacket.elapsed() > kNoDataTimeoutMs) {
        teardownSockets(false);
        emit stateChanged(Connecting, m_step == StepPlaying || m_lastSize.isValid()
                                          ? tr("Video stalled, reconnecting…")
                                          : tr("No answer from drone, retrying…"));
        m_reconnect->start(kReconnectDelayMs);
    }
}

void RtspWorker::onStatsTick()
{
    const double secs = qMax<qint64>(1, m_statsClock.restart()) / 1000.0;
    const FrameDecoder::Stats d = m_decoder->takeStats();
    const quint64 lost = m_depay.lostPackets(), dropped = m_depay.droppedFrames();
    emit statsUpdated(d.decoded / secs, m_rxFrames / secs,
                      (lost - qMin(lost, m_prevLost)) / secs,
                      (dropped - qMin(dropped, m_prevDropped)) / secs,
                      d.skipped / secs, m_lastSize.width(), m_lastSize.height(), d.avgMs,
                      m_useTcp ? 0 : m_rcvBuf);
    m_prevLost = lost;
    m_prevDropped = dropped;
    m_rxFrames = 0;
}
