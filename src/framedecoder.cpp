// SPDX-License-Identifier: GPL-3.0-or-later
#include "framedecoder.h"

#include <QElapsedTimer>

FrameDecoder::FrameDecoder(QObject *parent)
    : QThread(parent)
{
    setObjectName(QStringLiteral("decode"));
}

FrameDecoder::~FrameDecoder()
{
    shutdown();
}

void FrameDecoder::shutdown()
{
    {
        QMutexLocker lock(&m_mutex);
        m_quit = true;
        m_cond.wakeAll();
    }
    wait();
}

void FrameDecoder::submit(const QByteArray &jpeg)
{
    QMutexLocker lock(&m_mutex);
    if (!m_pending.isEmpty())
        ++m_skipped; // the decoder never got to the previous one
    m_pending = jpeg;
    m_cond.wakeOne();
}

FrameDecoder::Stats FrameDecoder::takeStats()
{
    QMutexLocker lock(&m_mutex);
    Stats s;
    s.decoded = m_decoded;
    s.skipped = m_skipped;
    s.avgMs = m_decoded ? m_msSum / m_decoded : 0;
    s.size = m_size;
    m_decoded = m_skipped = 0;
    m_msSum = 0;
    return s;
}

void FrameDecoder::run()
{
    for (;;) {
        QByteArray jpeg;
        {
            QMutexLocker lock(&m_mutex);
            while (m_pending.isEmpty() && !m_quit)
                m_cond.wait(&m_mutex);
            if (m_quit)
                return;
            jpeg.swap(m_pending);
        }
        QElapsedTimer t;
        t.start();
        QImage img;
        if (!img.loadFromData(jpeg, "JPEG") || img.isNull())
            continue;
        const double ms = t.nsecsElapsed() / 1e6;
        {
            QMutexLocker lock(&m_mutex);
            ++m_decoded;
            m_msSum += ms;
            m_size = img.size();
        }
        emit frameReady(img);
    }
}
