// SPDX-License-Identifier: GPL-3.0-or-later
// Photo and video capture from the live feed.
//
// The network thread hands every complete JPEG frame to onJpeg() (thread-safe,
// never blocks). Photos are taken from the newest frame; recordings copy every
// frame into a file without re-encoding.
//
// Files go to the app's own data folder. Confined apps can't write to
// ~/Pictures or ~/Videos without reserved AppArmor groups, so the UI exports
// them through Content Hub instead (see qml/MediaPanel.qml).
#pragma once

#include <QByteArray>
#include <QMutex>
#include <QObject>
#include <QVariantList>

#include "framedecoder.h"

class Recorder;
class QTimer;

class MediaCapture : public QObject, public JpegSink
{
    Q_OBJECT
    Q_PROPERTY(QString photoFormat READ photoFormat WRITE setPhotoFormat NOTIFY photoFormatChanged)
    Q_PROPERTY(QString videoFormat READ videoFormat WRITE setVideoFormat NOTIFY videoFormatChanged)
    Q_PROPERTY(QStringList availablePhotoFormats READ availablePhotoFormats CONSTANT)
    Q_PROPERTY(int landscapeRotation READ landscapeRotation WRITE setLandscapeRotation NOTIFY rotationChanged)
    Q_PROPERTY(int portraitRotation READ portraitRotation WRITE setPortraitRotation NOTIFY rotationChanged)
    Q_PROPERTY(bool hasFrame READ hasFrame NOTIFY hasFrameChanged)
    Q_PROPERTY(bool recording READ recording NOTIFY recordingChanged)
    Q_PROPERTY(bool finalizing READ finalizing NOTIFY recordingChanged)
    Q_PROPERTY(qint64 recordingMs READ recordingMs NOTIFY recordingStatusChanged)
    Q_PROPERTY(qint64 recordingBytes READ recordingBytes NOTIFY recordingStatusChanged)
    Q_PROPERTY(int recordingDropped READ recordingDropped NOTIFY recordingStatusChanged)
    Q_PROPERTY(QString photoDir READ photoDir CONSTANT)
    Q_PROPERTY(QString videoDir READ videoDir CONSTANT)
    Q_PROPERTY(int mediaCount READ mediaCount NOTIFY mediaChanged)

public:
    explicit MediaCapture(const QString &mediaRoot, QObject *parent = nullptr);
    ~MediaCapture() override;

    // JpegSink (network thread)
    void onJpeg(const QByteArray &jpeg) override;

    Q_INVOKABLE void takePhoto();
    Q_INVOKABLE void toggleRecording();
    Q_INVOKABLE void startRecording();
    Q_INVOKABLE void stopRecording();
    // Newest first: [{path, name, video, size, modified}]
    Q_INVOKABLE QVariantList mediaFiles() const;
    Q_INVOKABLE bool deleteFile(const QString &path);

    QString photoFormat() const { return m_photoFormat; }
    void setPhotoFormat(const QString &f);
    QString videoFormat() const { return m_videoFormat; }
    void setVideoFormat(const QString &f);
    QStringList availablePhotoFormats() const;
    int landscapeRotation() const { return m_rotLandscape; }
    void setLandscapeRotation(int r);
    int portraitRotation() const { return m_rotPortrait; }
    void setPortraitRotation(int r);
    bool hasFrame() const { return m_hasFrame; }
    bool recording() const;
    bool finalizing() const { return m_finalizing; }
    qint64 recordingMs() const { return m_recMs; }
    qint64 recordingBytes() const { return m_recBytes; }
    int recordingDropped() const { return m_recDropped; }
    QString photoDir() const { return m_photoDir; }
    QString videoDir() const { return m_videoDir; }
    int mediaCount() const;

    static bool webpSupported();

    // Writes one photo; used by takePhoto() and the tests. Returns the path or "".
    static QString savePhoto(const QByteArray &jpeg, const QString &dir, const QString &base,
                             const QString &format, int landscapeRotation, int portraitRotation,
                             QString *error = nullptr);

signals:
    void photoFormatChanged();
    void videoFormatChanged();
    void rotationChanged();
    void hasFrameChanged();
    void recordingChanged();
    void recordingStatusChanged();
    void mediaChanged();
    void photoSaved(const QString &path);
    void videoSaved(const QStringList &paths);
    void captureError(const QString &message);

private:
    QString timestampName() const;
    void pollStatus();
    bool inMediaDirs(const QString &path) const;

    QString m_photoDir, m_videoDir;
    QString m_photoFormat = QStringLiteral("jpeg");
    QString m_videoFormat = QStringLiteral("mkv");
    int m_rotLandscape = 0, m_rotPortrait = 90;

    mutable QMutex m_frameMutex;
    QByteArray m_latest;          // guarded by m_frameMutex
    bool m_hasFrame = false;      // GUI thread copy

    Recorder *m_recorder = nullptr;
    QTimer *m_statusTimer = nullptr;
    bool m_finalizing = false;
    qint64 m_recMs = 0, m_recBytes = 0;
    int m_recDropped = 0;
    int m_photosInFlight = 0;
};
