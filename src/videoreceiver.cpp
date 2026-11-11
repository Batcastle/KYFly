// SPDX-License-Identifier: GPL-3.0-or-later
#include "videoreceiver.h"

#include <QTimer>

#include "rtspworker.h"

VideoReceiver::VideoReceiver(QObject *parent)
    : QObject(parent)
    , m_worker(new RtspWorker)
    , m_restartTimer(new QTimer(this))
{
    qRegisterMetaType<QImage>("QImage");


    m_thread.setObjectName(QStringLiteral("video"));
    m_worker->moveToThread(&m_thread);
    connect(&m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(this, &VideoReceiver::startWorker, m_worker, &RtspWorker::start);
    connect(this, &VideoReceiver::stopWorker, m_worker, &RtspWorker::stop);
    // One queued hop from the worker thread straight to whoever displays frames.
    connect(m_worker, &RtspWorker::frameReady, this, &VideoReceiver::frameReady);
    connect(m_worker, &RtspWorker::stateChanged, this, &VideoReceiver::onWorkerState);
    connect(m_worker, &RtspWorker::statsUpdated, this, &VideoReceiver::onWorkerStats);
    m_thread.start();

    m_restartTimer->setSingleShot(true);
    connect(m_restartTimer, &QTimer::timeout, this, &VideoReceiver::start);
}

VideoReceiver::~VideoReceiver()
{
    QMetaObject::invokeMethod(m_worker, "stop", Qt::BlockingQueuedConnection);
    m_thread.quit();
    m_thread.wait();
}

void VideoReceiver::start()
{
    m_restartTimer->stop();
    emit startWorker(m_url, m_useTcp);
    if (!m_running) {
        m_running = true;
        emit runningChanged();
    }
}

void VideoReceiver::stop()
{
    m_restartTimer->stop();
    emit stopWorker();
    if (m_running) {
        m_running = false;
        emit runningChanged();
    }
}

void VideoReceiver::restart(int delayMs)
{
    if (delayMs <= 0) {
        start(); // RtspWorker::start tears down the old session first
        return;
    }
    emit stopWorker();
    m_restartTimer->start(delayMs);
}

void VideoReceiver::setUrl(const QString &u)
{
    if (u == m_url)
        return;
    m_url = u;
    emit urlChanged();
    if (m_running)
        start();
}

void VideoReceiver::setUseTcp(bool on)
{
    if (on == m_useTcp)
        return;
    m_useTcp = on;
    emit useTcpChanged();
    if (m_running)
        start();
}

void VideoReceiver::onWorkerState(int state, const QString &message)
{
    m_state = state;
    m_status = message;
    emit stateChanged();
}

void VideoReceiver::onWorkerStats(double fps, double rxFps, double lost, double dropped,
                                  double skipped, int w, int h, double decodeMs, int rcvBuf)
{
    m_fps = fps;
    m_rxFps = rxFps;
    m_lost = lost;
    m_dropped = dropped;
    m_skipped = skipped;
    m_w = w;
    m_h = h;
    m_decodeMs = decodeMs;
    m_rcvBuf = rcvBuf;
    emit statsChanged();
}
