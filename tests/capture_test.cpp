// Drives the capture code for tests/capture_cases.py.
//   capture_test mkv <framesdir> <out.mkv> <rotation> <fps> [crashAfterFrames]
//   capture_test mp4 <framesdir> <out.mp4> <rotation> <fps>
//   capture_test photo <frame.jpg> <outdir> <jpeg|png|webp|raw> <rotL> <rotP>
//   capture_test recorder <dirA> <dirB> <outdir> <mkv|mp4> <rotL> <rotP>
// Frame directories hold JPEG files, used in name order.
#include "../src/jpegutil.h"
#include "../src/mediacapture.h"
#include "../src/mkvwriter.h"
#include "../src/mp4writer.h"
#include "../src/recorder.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QThread>
#include <cstdio>
#include <unistd.h>

static QList<QByteArray> loadFrames(const QString &dir)
{
    QList<QByteArray> out;
    QDir d(dir);
    for (const QString &f : d.entryList({QStringLiteral("*.jpg")}, QDir::Files, QDir::Name)) {
        QFile file(d.filePath(f));
        file.open(QIODevice::ReadOnly);
        out << file.readAll();
    }
    return out;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    const QStringList a = app.arguments();
    if (a.size() < 2)
        return 2;
    const QString cmd = a.at(1);

    if (cmd == "mkv" || cmd == "mp4") {
        const QList<QByteArray> frames = loadFrames(a.at(2));
        const int rot = a.at(4).toInt();
        const double fps = a.at(5).toDouble();
        const int crashAfter = a.size() > 6 ? a.at(6).toInt() : -1;
        const QString mkvPath = cmd == "mkv" ? a.at(3) : a.at(3) + ".tmp.mkv";
        MkvWriter w;
        if (!w.open(mkvPath, JpegUtil::frameSize(frames.first()), rot)) {
            std::fprintf(stderr, "%s\n", qPrintable(w.errorString()));
            return 1;
        }
        for (int i = 0; i < frames.size(); ++i) {
            if (i == crashAfter)
                _exit(0); // simulate the app dying mid-recording: no close()
            // slightly uneven timing, like a real network feed
            const qint64 ms = qint64(i * 1000.0 / fps) + (i % 3 == 1 ? 7 : 0);
            if (!w.writeFrame(frames.at(i), ms))
                return 1;
        }
        const auto samples = w.samples();
        const QSize size = w.frameSize();
        if (!w.close())
            return 1;
        if (cmd == "mp4") {
            QString err;
            if (!Mp4Writer::build(mkvPath, samples, a.at(3), size, rot, &err)) {
                std::fprintf(stderr, "%s\n", qPrintable(err));
                return 1;
            }
            QFile::remove(mkvPath);
        }
        return 0;
    }

    if (cmd == "photo") {
        QFile f(a.at(2));
        f.open(QIODevice::ReadOnly);
        QString err;
        const QString path = MediaCapture::savePhoto(f.readAll(), a.at(3), "photo", a.at(4),
                                                     a.at(5).toInt(), a.at(6).toInt(), &err);
        std::printf("%s\n", qPrintable(path.isEmpty() ? "ERROR " + err : path));
        return path.isEmpty() ? 1 : 0;
    }

    if (cmd == "recorder") {
        const QList<QByteArray> A = loadFrames(a.at(2)), B = loadFrames(a.at(3));
        Recorder rec;
        int rc = 1;
        QObject::connect(&rec, &Recorder::finished, &app, [&](const QStringList &files, const QString &error) {
            for (const QString &f : files)
                std::printf("FILE %s\n", qPrintable(f));
            if (!error.isEmpty())
                std::printf("ERROR %s\n", qPrintable(error));
            rc = error.isEmpty() ? 0 : 1;
            app.quit();
        });
        const auto fmt = a.at(5) == "mp4" ? Recorder::Mp4 : Recorder::Mkv;
        if (!rec.startRecording(a.at(4), "rec", fmt, a.at(6).toInt(), a.at(7).toInt()))
            return 1;
        // Feed from another thread, as the network thread would.
        QThread *feeder = QThread::create([&]() {
            for (const QByteArray &f : A) { rec.enqueue(f); QThread::msleep(40); }
            for (const QByteArray &f : B) { rec.enqueue(f); QThread::msleep(40); }
            rec.stopRecording();
        });
        feeder->start();
        app.exec();
        feeder->wait();
        delete feeder;
        return rc;
    }
    return 2;
}
