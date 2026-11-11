// SPDX-License-Identifier: GPL-3.0-or-later
// JPEG decoding on its own thread, with a one-slot "latest frame" mailbox.
//
// The network thread hands over every completed JPEG with submit(), which
// never blocks. If the decoder is still busy, the waiting frame is replaced
// by the newer one, so decode speed can never make video lag or make the
// network thread stall (which would overflow the kernel's UDP buffer).
#pragma once

#include <QByteArray>
#include <QImage>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>

class FrameDecoder : public QThread
{
    Q_OBJECT
public:
    explicit FrameDecoder(QObject *parent = nullptr);
    ~FrameDecoder() override;

    void submit(const QByteArray &jpeg);   // thread-safe, non-blocking
    void shutdown();

    // Counters since the last call (thread-safe).
    struct Stats { int decoded = 0; int skipped = 0; double avgMs = 0; QSize size; };
    Stats takeStats();

signals:
    void frameReady(const QImage &image);   // emitted from the decoder thread

protected:
    void run() override;

private:
    QMutex m_mutex;
    QWaitCondition m_cond;
    QByteArray m_pending;
    bool m_quit = false;
    int m_decoded = 0;
    int m_skipped = 0;
    double m_msSum = 0;
    QSize m_size;
};
