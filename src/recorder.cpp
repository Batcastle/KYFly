// SPDX-License-Identifier: GPL-3.0-or-later
#include "recorder.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStorageInfo>

#include "jpegutil.h"
#include "mkvwriter.h"
#include "mp4writer.h"

Recorder::Recorder(QObject *parent)
    : QThread(parent)
{
    setObjectName(QStringLiteral("record"));
    start(QThread::LowPriority);
}

Recorder::~Recorder()
{
    {
        QMutexLocker lock(&m_mutex);
        m_stopRequested = true;   // finish (and finalize) any recording first
        m_quit = true;
        m_cond.wakeAll();
    }
    wait();
}

bool Recorder::startRecording(const QString &dir, const QString &baseName, Format format,
                              int landscapeRotation, int portraitRotation)
{
    QMutexLocker lock(&m_mutex);
    if (m_busy.load())
        return false; // still recording or still finalizing the previous one
    if (!QDir().mkpath(dir))
        return false;
    m_dir = dir;
    m_base = baseName;
    m_format = format;
    m_rotLandscape = landscapeRotation;
    m_rotPortrait = portraitRotation;
    m_segment = 0;
    m_files.clear();
    m_error.clear();
    m_queue.clear();
    m_stopRequested = false;
    m_prevMs = -1;
    m_statusMs = m_statusBytes = m_doneBytes = 0;
    m_statusFrames = m_dropped = 0;
    m_clock.start();
    m_busy = true;
    m_recording = true;
    return true;
}

void Recorder::stopRecording()
{
    QMutexLocker lock(&m_mutex);
    if (!m_recording.load())
        return;
    m_recording = false; // no more frames accepted
    m_stopRequested = true;
    m_cond.wakeAll();
}

void Recorder::setRotations(int landscape, int portrait)
{
    m_rotLandscape = landscape; // applies from the next file
    m_rotPortrait = portrait;
}

void Recorder::enqueue(const QByteArray &jpeg)
{
    if (!m_recording.load())
        return;
    QMutexLocker lock(&m_mutex);
    if (!m_recording.load())
        return;
    if (m_queue.size() >= kMaxQueue) {
        ++m_dropped; // disk can't keep up; drop rather than stall the network
        return;
    }
    m_queue.enqueue({jpeg, m_clock.elapsed()});
    m_cond.wakeOne();
}

Recorder::Status Recorder::status() const
{
    QMutexLocker lock(&m_mutex);
    return {m_statusMs, m_statusBytes, m_statusFrames, m_dropped};
}

bool Recorder::openSegment(QSize size, QString *error)
{
    ++m_segment;
    const QString name = m_base + (m_segment > 1 ? QStringLiteral("_%1").arg(m_segment) : QString()) +
                         QStringLiteral(".mkv");
    const int rot = size.height() > size.width() ? m_rotPortrait.load() : m_rotLandscape.load();
    m_writer = new MkvWriter;
    if (!m_writer->open(QDir(m_dir).filePath(name), size, rot)) {
        *error = m_writer->errorString();
        delete m_writer;
        m_writer = nullptr;
        return false;
    }
    m_files.append(m_writer->path());
    if (m_segment == 1)
        emit recordingStarted(m_writer->path());
    return true;
}

bool Recorder::closeSegment(QString *error)
{
    if (!m_writer)
        return true;
    const bool ok = m_writer->close();
    if (!ok && error->isEmpty())
        *error = m_writer->errorString();
    {
        QMutexLocker lock(&m_mutex);
        m_doneBytes += QFileInfo(m_writer->path()).size();
    }
    if (m_format == Mp4 && m_writer->frames() > 0) {
        emit finalizing();
        const QString mkv = m_writer->path();
        const QString mp4 = mkv.left(mkv.size() - 4) + QStringLiteral(".mp4");
        QString err;
        if (Mp4Writer::build(mkv, m_writer->samples(), mp4, m_writer->frameSize(), m_writer->rotation(), &err)) {
            QFile::remove(mkv);
            m_files.replace(m_files.indexOf(mkv), mp4);
        } else {
            // Keep the MKV: the footage is safe even if the MP4 couldn't be made.
            QFile::remove(mp4);
            if (error->isEmpty())
                *error = QStringLiteral("MP4 conversion failed (%1); kept the MKV").arg(err);
        }
    } else if (m_writer->frames() == 0) {
        m_files.removeAll(m_writer->path());
        QFile::remove(m_writer->path());
    }
    delete m_writer;
    m_writer = nullptr;
    return ok;
}

void Recorder::run()
{
    QElapsedTimer spaceCheck;
    spaceCheck.start();
    for (;;) {
        Frame f;
        bool stop = false, quit = false;
        {
            QMutexLocker lock(&m_mutex);
            while (m_queue.isEmpty() && !m_stopRequested && !m_quit)
                m_cond.wait(&m_mutex);
            if (!m_queue.isEmpty())
                f = m_queue.dequeue();
            else
                stop = m_stopRequested;
            quit = m_quit && m_queue.isEmpty();
        }

        if (!f.jpeg.isEmpty()) {
            const QSize size = JpegUtil::frameSize(f.jpeg);
            if (!size.isValid())
                continue;
            if (m_writer && size != m_writer->frameSize()) {
                closeSegment(&m_error);
                m_prevMs = -1;
            }
            if (!m_writer) {
                QString err;
                if (!openSegment(size, &err)) {
                    m_error = err;
                    stopRecording();
                    continue;
                }
                m_segmentStartMs = f.ms;
            }
            qint64 ms = f.ms - m_segmentStartMs;
            if (ms <= m_prevMs && m_writer->frames() > 0)
                ms = m_prevMs + 1;           // strictly increasing timestamps
            if (!m_writer->writeFrame(f.jpeg, ms)) {
                m_error = m_writer->errorString();
                stopRecording();
                continue;
            }
            m_prevMs = ms;
            {
                QMutexLocker lock(&m_mutex);
                m_statusMs = f.ms;
                m_statusBytes = m_doneBytes + m_writer->bytesWritten();
                ++m_statusFrames;
            }
            if (spaceCheck.elapsed() > 3000) {
                spaceCheck.restart();
                const QStorageInfo si(m_dir);
                if (si.isValid() && si.bytesAvailable() < kMinFreeBytes) {
                    m_error = QStringLiteral("Storage almost full, recording stopped");
                    stopRecording();
                }
            }
            continue;
        }

        if (stop || quit) {
            if (m_writer || !m_files.isEmpty() || !m_error.isEmpty()) {
                closeSegment(&m_error);
                const QStringList files = m_files;
                const QString error = m_error;
                {
                    QMutexLocker lock(&m_mutex);
                    m_stopRequested = false;
                    m_files.clear();
                    m_error.clear();
                }
                m_busy = false;
                emit finished(files, error);
            } else {
                m_busy = false;
                QMutexLocker lock(&m_mutex);
                m_stopRequested = false;
            }
            m_prevMs = -1;
            if (quit)
                return;
        }
    }
}
