// SPDX-License-Identifier: GPL-3.0-or-later
// Minimal Matroska writer for MJPEG (V_MJPEG), stream copy, crash-safe.
//
// The file is valid at every moment while recording:
//   * the Segment is written with "unknown size",
//   * each Cluster starts with "unknown size" and gets its real size patched
//     in when the next one starts,
//   * Duration, Cues and the SeekHead are only added by close().
// If the app or the video feed dies mid-recording, everything up to the last
// frame written plays in ffmpeg/mpv/VLC/GStreamer.
#pragma once

#include <QByteArray>
#include <QFile>
#include <QSize>
#include <QString>
#include <QVector>

class MkvWriter
{
public:
    struct Sample { qint64 offset; int size; qint64 ms; }; // JPEG payload location in the file

    MkvWriter() = default;
    ~MkvWriter();

    // rotation: clockwise display rotation (0/90/180/270), stored as metadata.
    bool open(const QString &path, QSize frameSize, int rotation);
    // ms: presentation time relative to the start of this file, increasing.
    bool writeFrame(const QByteArray &jpeg, qint64 ms);
    bool close();

    bool isOpen() const { return m_file.isOpen(); }
    QString path() const { return m_file.fileName(); }
    QSize frameSize() const { return m_size; }
    int rotation() const { return m_rotation; }
    qint64 bytesWritten() const { return m_file.isOpen() ? m_file.pos() : 0; }
    qint64 lastMs() const { return m_lastMs; }
    int frames() const { return m_samples.size(); }
    // Payload locations, used to build an MP4 from this file without re-parsing.
    const QVector<Sample> &samples() const { return m_samples; }
    QString errorString() const { return m_error; }

    // EBML helpers (public for tests).
    static QByteArray vint(quint64 value, int length = 0);
    static QByteArray element(quint32 id, const QByteArray &payload);
    static QByteArray uintElement(quint32 id, quint64 v);
    static QByteArray floatElement(quint32 id, double v);
    static QByteArray stringElement(quint32 id, const QByteArray &s);
    static QByteArray voidElement(int totalLength);

private:
    bool startCluster(qint64 ms);
    bool finishCluster();
    bool put(const QByteArray &bytes);
    bool fail(const QString &why);

    QFile m_file;
    QString m_error;
    QSize m_size;
    int m_rotation = 0;
    qint64 m_segmentDataStart = 0;
    qint64 m_seekHeadPos = 0;    // reserved Void, replaced by SeekHead on close
    qint64 m_durationPos = 0;    // Void placeholder, replaced by Duration on close
    qint64 m_infoPos = 0;
    qint64 m_tracksPos = 0;
    qint64 m_clusterPos = -1;    // file position of current Cluster ID
    qint64 m_clusterSizePos = -1;
    qint64 m_clusterMs = 0;
    qint64 m_lastMs = 0;
    QVector<QPair<qint64, qint64>> m_cues; // (cluster ms, cluster position rel. to segment data)
    QVector<Sample> m_samples;
};
