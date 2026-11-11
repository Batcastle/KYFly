// SPDX-License-Identifier: GPL-3.0-or-later
// kyfly-probe: check the drone link and video from a PC, using exactly the
// same code as the app. Sends heartbeats only; never sends flight commands.
//
//   kyfly-probe [--host 192.168.1.1] [--seconds 10] [--tcp] [--save frame.jpg]
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QImage>
#include <QTimer>
#include <cstdio>

#include "dronelink.h"
#include "rtspworker.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCommandLineParser cli;
    cli.addHelpOption();
    cli.addOptions({
        {"host", "Drone IP.", "ip", "192.168.1.1"},
        {"seconds", "How long to run.", "s", "10"},
        {"tcp", "Request RTP over TCP instead of UDP."},
        {"save", "Save the last decoded frame here.", "file"},
    });
    cli.process(app);
    const QString host = cli.value("host");

    DroneLink link;
    link.setHost(host);
    QObject::connect(&link, &DroneLink::modelIdChanged, [&]() {
        std::printf("[link] model ID %d (0x%02x) -> %d-byte control frames\n", link.modelId(),
                    link.modelId(), link.packetFormat());
    });
    QObject::connect(&link, &DroneLink::lastReplyChanged, [&]() {
        std::printf("[link] reply: %s\n", qPrintable(link.lastReplyHex()));
    });
    QObject::connect(&link, &DroneLink::connectedChanged, [&]() {
        std::printf("[link] %s\n", link.connected() ? "heartbeat answered" : "no heartbeat reply");
    });
    link.setActive(true);

    RtspWorker worker;
    QImage last;
    int frames = 0;
    // Context object = queued into this thread (frames are emitted by the decoder thread).
    QObject::connect(&worker, &RtspWorker::frameReady, &app, [&](const QImage &img) { last = img; ++frames; });
    QObject::connect(&worker, &RtspWorker::stateChanged, [](int s, const QString &m) {
        static const char *names[] = {"idle", "connecting", "playing", "failed"};
        std::printf("[video] %s: %s\n", names[qBound(0, s, 3)], qPrintable(m));
    });
    QObject::connect(&worker, &RtspWorker::statsUpdated,
                     [](double fps, double rxFps, double lost, double dropped, double skipped, int w, int h,
                        double ms, int rcvBuf) {
                         std::printf("[video] shown %5.1f fps  received %5.1f fps  %dx%d  decode %.1f ms  "
                                     "lost %.0f pkt/s  incomplete %.0f fr/s  skipped %.0f fr/s  rcvbuf %d KiB\n",
                                     fps, rxFps, w, h, ms, lost, dropped, skipped, rcvBuf / 1024);
                     });
    worker.start(QStringLiteral("rtsp://%1:7070/webcam").arg(host), cli.isSet("tcp"));

    QTimer::singleShot(cli.value("seconds").toInt() * 1000, [&]() {
        worker.stop();
        link.setActive(false);
        std::printf("\n[done] %d frames decoded", frames);
        if (cli.isSet("save") && !last.isNull()) {
            last.save(cli.value("save"));
            std::printf(", last frame saved to %s", qPrintable(cli.value("save")));
        }
        std::printf("\n");
        QCoreApplication::exit(frames > 0 ? 0 : 1);
    });
    return app.exec();
}
