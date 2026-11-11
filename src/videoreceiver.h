// SPDX-License-Identifier: GPL-3.0-or-later
// GUI-thread facade for RtspWorker. Owns the worker thread and exposes state
// and statistics to QML.
#pragma once

#include <QImage>
#include <QObject>
#include <QThread>

class RtspWorker;
class QTimer;

class VideoReceiver : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(bool useTcp READ useTcp WRITE setUseTcp NOTIFY useTcpChanged)
    Q_PROPERTY(int state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString status READ status NOTIFY stateChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(double fps READ fps NOTIFY statsChanged)
    Q_PROPERTY(double decodeMs READ decodeMs NOTIFY statsChanged)
    Q_PROPERTY(int frameWidth READ frameWidth NOTIFY statsChanged)
    Q_PROPERTY(int frameHeight READ frameHeight NOTIFY statsChanged)
    Q_PROPERTY(double rxFps READ rxFps NOTIFY statsChanged)
    Q_PROPERTY(double lostPerSec READ lostPerSec NOTIFY statsChanged)
    Q_PROPERTY(double droppedPerSec READ droppedPerSec NOTIFY statsChanged)
    Q_PROPERTY(double skippedPerSec READ skippedPerSec NOTIFY statsChanged)
    Q_PROPERTY(int rcvBuf READ rcvBuf NOTIFY statsChanged)

public:
    enum State { Idle = 0, Connecting = 1, Playing = 2, Failed = 3 };
    Q_ENUM(State)

    explicit VideoReceiver(QObject *parent = nullptr);
    ~VideoReceiver() override;

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    // Stop now and start again after delayMs (used after camera switches).
    Q_INVOKABLE void restart(int delayMs = 0);

    QString url() const { return m_url; }
    void setUrl(const QString &u);
    bool useTcp() const { return m_useTcp; }
    void setUseTcp(bool on);
    int state() const { return m_state; }
    QString status() const { return m_status; }
    bool running() const { return m_running; }
    double fps() const { return m_fps; }
    double decodeMs() const { return m_decodeMs; }
    int frameWidth() const { return m_w; }
    int frameHeight() const { return m_h; }
    double rxFps() const { return m_rxFps; }
    double lostPerSec() const { return m_lost; }
    double droppedPerSec() const { return m_dropped; }
    double skippedPerSec() const { return m_skipped; }
    int rcvBuf() const { return m_rcvBuf; }

signals:
    void frameReady(const QImage &image);
    void urlChanged();
    void useTcpChanged();
    void stateChanged();
    void runningChanged();
    void statsChanged();
    // internal, queued to the worker thread
    void startWorker(const QString &url, bool useTcp);
    void stopWorker();

private slots:
    void onWorkerState(int state, const QString &message);
    void onWorkerStats(double fps, double rxFps, double lost, double dropped, double skipped,
                       int w, int h, double decodeMs, int rcvBuf);

private:
    QThread m_thread;
    RtspWorker *m_worker = nullptr;
    QTimer *m_restartTimer = nullptr;
    QString m_url = QStringLiteral("rtsp://192.168.1.1:7070/webcam");
    bool m_useTcp = false;
    bool m_running = false;
    int m_state = Idle;
    QString m_status;
    double m_fps = 0, m_decodeMs = 0;
    int m_w = 0, m_h = 0;
    double m_rxFps = 0, m_lost = 0, m_dropped = 0, m_skipped = 0;
    int m_rcvBuf = 0;
};
