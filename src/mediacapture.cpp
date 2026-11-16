// SPDX-License-Identifier: GPL-3.0-or-later
#include "mediacapture.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageWriter>
#include <QPointer>
#include <QThreadPool>
#include <QTimer>
#include <QTransform>

#include "jpegutil.h"
#include "recorder.h"

namespace {
int normRot(int r) { return ((r % 360) + 360) % 360 / 90 * 90; }
const QStringList kVideoExt = {QStringLiteral("mkv"), QStringLiteral("mp4")};
const QStringList kPhotoExt = {QStringLiteral("jpg"), QStringLiteral("png"), QStringLiteral("webp")};
} // namespace

MediaCapture::MediaCapture(const QString &mediaRoot, QObject *parent)
    : QObject(parent)
    , m_photoDir(QDir(mediaRoot).filePath(QStringLiteral("Pictures")))
    , m_videoDir(QDir(mediaRoot).filePath(QStringLiteral("Videos")))
    , m_recorder(new Recorder(this))
    , m_statusTimer(new QTimer(this))
{
    QDir().mkpath(m_photoDir);
    QDir().mkpath(m_videoDir);

    m_statusTimer->setInterval(250);
    connect(m_statusTimer, &QTimer::timeout, this, &MediaCapture::pollStatus);

    connect(m_recorder, &Recorder::finalizing, this, [this]() {
        m_finalizing = true;
        emit recordingChanged();
    }, Qt::QueuedConnection);
    connect(m_recorder, &Recorder::finished, this, [this](const QStringList &files, const QString &error) {
        m_statusTimer->stop();
        m_finalizing = false;
        emit recordingChanged();
        if (!error.isEmpty())
            emit captureError(error);
        if (!files.isEmpty())
            emit videoSaved(files);
        emit mediaChanged();
    }, Qt::QueuedConnection);
}

MediaCapture::~MediaCapture()
{
    m_recorder->stopRecording();
    delete m_recorder; // waits for the file to be finalized
    m_recorder = nullptr;
    QThreadPool::globalInstance()->waitForDone(5000); // let pending photos finish
}

void MediaCapture::onJpeg(const QByteArray &jpeg)
{
    bool first = false;
    {
        QMutexLocker lock(&m_frameMutex);
        first = m_latest.isEmpty();
        m_latest = jpeg; // implicitly shared, no copy
    }
    if (m_recorder)
        m_recorder->enqueue(jpeg);
    if (first)
        QMetaObject::invokeMethod(this, [this]() {
            if (!m_hasFrame) { m_hasFrame = true; emit hasFrameChanged(); }
        }, Qt::QueuedConnection);
}

QString MediaCapture::timestampName() const
{
    return QStringLiteral("KYFly_") +
           QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd_HH-mm-ss-zzz"));
}

bool MediaCapture::webpSupported()
{
    return QImageWriter::supportedImageFormats().contains("webp");
}

QStringList MediaCapture::availablePhotoFormats() const
{
    QStringList f = {QStringLiteral("jpeg"), QStringLiteral("png")};
    if (webpSupported())
        f << QStringLiteral("webp");
    f << QStringLiteral("raw");
    return f;
}

QString MediaCapture::savePhoto(const QByteArray &jpeg, const QString &dir, const QString &base,
                                const QString &format, int landscapeRotation, int portraitRotation,
                                QString *error)
{
    auto fail = [error](const QString &e) { if (error) *error = e; return QString(); };
    const QSize size = JpegUtil::frameSize(jpeg);
    if (!size.isValid())
        return fail(QStringLiteral("no valid frame"));
    const int rot = normRot(size.height() > size.width() ? portraitRotation : landscapeRotation);
    QDir().mkpath(dir);

    if (format == QLatin1String("raw") || format == QLatin1String("jpeg")) {
        // RAW: the frame exactly as the drone sent it.
        // JPEG: the same data plus an EXIF orientation tag, so viewers show it
        //       the way it appears on screen without re-compressing it.
        const bool raw = format == QLatin1String("raw");
        const QString path = QDir(dir).filePath(base + (raw ? QStringLiteral("_raw.jpg") : QStringLiteral(".jpg")));
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly))
            return fail(f.errorString());
        const QByteArray data = raw ? jpeg : JpegUtil::withExifOrientation(jpeg, rot);
        if (f.write(data) != data.size())
            return fail(f.errorString());
        return path;
    }

    QImage img;
    if (!img.loadFromData(jpeg, "JPEG"))
        return fail(QStringLiteral("could not decode frame"));
    if (rot)
        img = img.transformed(QTransform().rotate(rot)); // clockwise on screen
    const bool webp = format == QLatin1String("webp");
    const QString path = QDir(dir).filePath(base + (webp ? QStringLiteral(".webp") : QStringLiteral(".png")));
    QImageWriter w(path, webp ? "webp" : "png");
    if (webp)
        w.setQuality(92);
    if (!w.write(img))
        return fail(w.errorString());
    return path;
}

void MediaCapture::takePhoto()
{
    QByteArray jpeg;
    {
        QMutexLocker lock(&m_frameMutex);
        jpeg = m_latest;
    }
    if (jpeg.isEmpty()) {
        emit captureError(tr("No video to take a photo of yet"));
        return;
    }
    QString format = m_photoFormat;
    if (format == QLatin1String("webp") && !webpSupported())
        format = QStringLiteral("png");
    const QString dir = m_photoDir, base = timestampName();
    const int rl = m_rotLandscape, rp = m_rotPortrait;
    QPointer<MediaCapture> self(this);
    ++m_photosInFlight;
    // Encoding (PNG especially) can take a while; keep it off the GUI thread.
    QThreadPool::globalInstance()->start([=]() {
        QString err;
        const QString path = savePhoto(jpeg, dir, base, format, rl, rp, &err);
        QMetaObject::invokeMethod(qApp, [self, path, err]() {
            if (!self) return;
            --self->m_photosInFlight;
            if (path.isEmpty()) {
                emit self->captureError(tr("Photo not saved: %1").arg(err));
            } else {
                emit self->photoSaved(path);
                emit self->mediaChanged();
            }
        }, Qt::QueuedConnection);
    });
}

bool MediaCapture::recording() const
{
    return m_recorder && m_recorder->isRecording();
}

void MediaCapture::startRecording()
{
    if (recording())
        return;
    if (m_recorder->isBusy()) {
        emit captureError(tr("Still saving the last video"));
        return;
    }
    const auto fmt = m_videoFormat == QLatin1String("mp4") ? Recorder::Mp4 : Recorder::Mkv;
    if (!m_recorder->startRecording(m_videoDir, timestampName().left(25), fmt, m_rotLandscape, m_rotPortrait)) {
        emit captureError(tr("Could not start recording in %1").arg(m_videoDir));
        return;
    }
    m_recMs = m_recBytes = 0;
    m_recDropped = 0;
    m_statusTimer->start();
    emit recordingChanged();
    emit recordingStatusChanged();
}

void MediaCapture::stopRecording()
{
    if (!recording())
        return;
    m_recorder->stopRecording();
    emit recordingChanged();
}

void MediaCapture::toggleRecording()
{
    if (recording())
        stopRecording();
    else
        startRecording();
}

void MediaCapture::pollStatus()
{
    const Recorder::Status s = m_recorder->status();
    m_recMs = s.frames ? s.ms : 0;
    m_recBytes = s.bytes;
    m_recDropped = s.dropped;
    emit recordingStatusChanged();
    if (!m_recorder->isRecording() && !m_recorder->isBusy())
        m_statusTimer->stop();
}

void MediaCapture::setPhotoFormat(const QString &f)
{
    static const QStringList ok = {QStringLiteral("jpeg"), QStringLiteral("png"),
                                   QStringLiteral("webp"), QStringLiteral("raw")};
    if (!ok.contains(f) || f == m_photoFormat)
        return;
    m_photoFormat = f;
    emit photoFormatChanged();
}

void MediaCapture::setVideoFormat(const QString &f)
{
    if ((f != QLatin1String("mkv") && f != QLatin1String("mp4")) || f == m_videoFormat)
        return;
    m_videoFormat = f; // takes effect at the next recording
    emit videoFormatChanged();
}

void MediaCapture::setLandscapeRotation(int r)
{
    r = normRot(r);
    if (r == m_rotLandscape) return;
    m_rotLandscape = r;
    m_recorder->setRotations(m_rotLandscape, m_rotPortrait);
    emit rotationChanged();
}

void MediaCapture::setPortraitRotation(int r)
{
    r = normRot(r);
    if (r == m_rotPortrait) return;
    m_rotPortrait = r;
    m_recorder->setRotations(m_rotLandscape, m_rotPortrait);
    emit rotationChanged();
}

QVariantList MediaCapture::mediaFiles() const
{
    QFileInfoList all;
    QStringList photoFilters, videoFilters;
    for (const QString &e : kPhotoExt) photoFilters << QStringLiteral("*.") + e;
    for (const QString &e : kVideoExt) videoFilters << QStringLiteral("*.") + e;
    all += QDir(m_photoDir).entryInfoList(photoFilters, QDir::Files);
    all += QDir(m_videoDir).entryInfoList(videoFilters, QDir::Files);
    std::sort(all.begin(), all.end(), [](const QFileInfo &a, const QFileInfo &b) {
        return a.lastModified() > b.lastModified();
    });
    QVariantList out;
    for (const QFileInfo &fi : all) {
        QVariantMap m;
        m.insert(QStringLiteral("path"), fi.absoluteFilePath());
        m.insert(QStringLiteral("name"), fi.fileName());
        m.insert(QStringLiteral("video"), kVideoExt.contains(fi.suffix().toLower()));
        m.insert(QStringLiteral("size"), fi.size());
        m.insert(QStringLiteral("modified"), fi.lastModified());
        out.append(m);
    }
    return out;
}

int MediaCapture::mediaCount() const
{
    return mediaFiles().size();
}

bool MediaCapture::inMediaDirs(const QString &path) const
{
    const QString dir = QFileInfo(path).absolutePath();
    return dir == QFileInfo(m_photoDir).absoluteFilePath() || dir == QFileInfo(m_videoDir).absoluteFilePath();
}

bool MediaCapture::deleteFile(const QString &path)
{
    if (!inMediaDirs(path) || !QFile::remove(path))
        return false;
    emit mediaChanged();
    return true;
}
