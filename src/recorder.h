// SPDX-License-Identifier: GPL-3.0-or-later
// Video recorder thread. Receives complete JPEG frames from the network thread
// (enqueue() never blocks), writes them to a crash-safe MKV, and, when MP4 is
// selected, builds the MP4 from that MKV after recording stops.
//
// A change in frame size (e.g. switching to the bottom camera) closes the
// current file and continues in a new one ("..._2.mkv"), so every file has a
// single resolution and the right rotation for that camera.
#pragma once

#include <QByteArray>
#include <QElapsedTimer>
#include <QMutex>
#include <QQueue>
#include <QStringList>
#include <QThread>
#include <QWaitCondition>
#include <atomic>

class MkvWriter;

class Recorder : public QThread
{
    Q_OBJECT
public:
    enum Format { Mkv = 0, Mp4 = 1 };

    explicit Recorder(QObject *parent = nullptr);
    ~Recorder() override;

    // GUI thread
    bool startRecording(const QString &dir, const QString &baseName, Format format,
                        int landscapeRotation, int portraitRotation);
    void stopRecording();             // returns at once; finished() reports the files
    void setRotations(int landscape, int portrait);

    // any thread
    void enqueue(const QByteArray &jpeg);
    bool isRecording() const { return m_recording.load(); }
    bool isBusy() const { return m_busy.load(); }

    struct Status { qint64 ms = 0; qint64 bytes = 0; int frames = 0; int dropped = 0; };
    Status status() const;

    static constexpr int kMaxQueue = 90;              // frames waiting for the disk
    static constexpr qint64 kMinFreeBytes = 100ll << 20; // stop below 100 MiB free

signals:
    void recordingStarted(const QString &firstFile);
    void finalizing();                                 // MP4 conversion running
    void finished(const QStringList &files, const QString &error);

protected:
    void run() override;

private:
    struct Frame { QByteArray jpeg; qint64 ms; };
    bool openSegment(QSize size, QString *error);
    bool closeSegment(QString *error);

    mutable QMutex m_mutex;
    QWaitCondition m_cond;
    QQueue<Frame> m_queue;
    bool m_quit = false;
    bool m_stopRequested = false;
    std::atomic<bool> m_recording{false};
    std::atomic<bool> m_busy{false};   // recording or finalizing
    QElapsedTimer m_clock;

    // owned by the recorder thread while recording
    QString m_dir, m_base;
    Format m_format = Mkv;
    std::atomic<int> m_rotLandscape{0}, m_rotPortrait{90};
    MkvWriter *m_writer = nullptr;
    int m_segment = 0;
    qint64 m_segmentStartMs = 0;
    qint64 m_prevMs = -1;
    QStringList m_files;
    QString m_error;

    // status (guarded by m_mutex)
    qint64 m_statusMs = 0;
    qint64 m_statusBytes = 0;
    qint64 m_doneBytes = 0;
    int m_statusFrames = 0;
    int m_dropped = 0;
};
