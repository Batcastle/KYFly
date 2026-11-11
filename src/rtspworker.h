// SPDX-License-Identifier: GPL-3.0-or-later
// Minimal RTSP client + RTP/JPEG receiver + JPEG decoder.
//
// Lives in a worker thread. Designed for minimum latency rather than smooth
// playback: there is no jitter buffer, no clock sync, and when frames arrive
// faster than they can be decoded, stale frames are skipped so only the newest
// one is ever decoded and shown.
#pragma once

#include <QByteArray>
#include <QElapsedTimer>
#include <QHash>
#include <QImage>
#include <QObject>
#include <QUrl>

#include "framedecoder.h"
#include "rtpjpeg.h"

class QTcpSocket;
class QUdpSocket;
class QTimer;

class RtspWorker : public QObject
{
    Q_OBJECT
public:
    enum State { Idle = 0, Connecting = 1, Playing = 2, Failed = 3 };
    Q_ENUM(State)

    explicit RtspWorker(QObject *parent = nullptr);
    ~RtspWorker() override;

public slots:
    void start(const QString &url, bool useTcp);
    void stop();

signals:
    void frameReady(const QImage &image);
    void stateChanged(int state, const QString &message);
    // Per-second figures. rxFps = complete frames received, shownFps = frames
    // decoded and handed to the display. rcvBuf = kernel UDP buffer (bytes).
    void statsUpdated(double shownFps, double rxFps, double lostPerSec, double droppedPerSec,
                      double skippedPerSec, int width, int height, double decodeMs, int rcvBuf);

private slots:
    void onConnected();
    void onTcpReadyRead();
    void onTcpError();
    void onRtpReadyRead();
    void onRtcpReadyRead();
    void onKeepAlive();
    void onWatchdog();
    void onStatsTick();
    void reconnect();

private:
    enum Step { StepNone, StepOptions, StepDescribe, StepSetup, StepPlay, StepPlaying };

    void teardownSockets(bool sendTeardown);
    void sendRequest(const QString &method, const QString &uri,
                     const QList<QPair<QByteArray, QByteArray>> &extra = {});
    void handleResponse(int code, const QHash<QByteArray, QByteArray> &headers,
                        const QByteArray &body);
    bool parseSdp(const QByteArray &sdp, const QString &baseUrl);
    bool bindUdpPair();
    void sendRtcpReceiverReport();
    void handleRtp(const uint8_t *data, int len);
    void fail(const QString &msg);

    // config
    QString m_url;
    bool m_useTcp = false;
    bool m_running = false;

    // RTSP
    QTcpSocket *m_tcp = nullptr;
    QByteArray m_rx;
    int m_cseq = 0;
    Step m_step = StepNone;
    QString m_controlUrl;
    QByteArray m_session;
    int m_sessionTimeout = 60;
    quint16 m_serverRtcpPort = 0;
    bool m_isJpeg = true;

    // RTP
    QUdpSocket *m_rtp = nullptr;
    QUdpSocket *m_rtcp = nullptr;
    quint32 m_ssrc = 0;
    RtpJpegDepayloader m_depay;
    FrameDecoder *m_decoder = nullptr;
    QByteArray m_scratch;
    int m_rcvBuf = 0;

    // timers
    QTimer *m_keepAlive = nullptr;
    QTimer *m_watchdog = nullptr;
    QTimer *m_stats = nullptr;
    QTimer *m_reconnect = nullptr;
    QElapsedTimer m_lastPacket;

    // stats
    int m_keepAliveTicks = 0;
    int m_rxFrames = 0;
    quint64 m_prevLost = 0;
    quint64 m_prevDropped = 0;
    QSize m_lastSize;
    QElapsedTimer m_statsClock;
};
