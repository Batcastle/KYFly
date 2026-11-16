// SPDX-License-Identifier: GPL-3.0-or-later
#include <QCommandLineParser>
#include <QDir>
#include <QGuiApplication>
#include <QLibrary>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QStandardPaths>
#include <QTimer>
#include <cstdio>

#include "dronelink.h"
#include "flightinput.h"
#include "mediacapture.h"
#include "screenkeeper.h"
#include "videoitem.h"
#include "videoreceiver.h"

// If the click bundles Qt's WebP image plugin (see CMakeLists.txt), make Qt
// find it, and load its libraries from the click by full path so they resolve
// regardless of the launcher's library path.
static void setupBundledImagePlugins()
{
    const QDir appDir(QCoreApplication::applicationDirPath());
    if (!appDir.exists(QStringLiteral("plugins/imageformats")))
        return;
    QCoreApplication::addLibraryPath(appDir.filePath(QStringLiteral("plugins")));
    QDir libDir(appDir.filePath(QStringLiteral("lib")));
    const QStringList triplets = libDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const char *pattern : {"libsharpyuv.so*", "libwebp.so*", "libwebpdemux.so*", "libwebpmux.so*"}) {
        for (const QString &t : triplets) {
            const QDir d(libDir.filePath(t));
            for (const QString &f : d.entryList({QString::fromLatin1(pattern)}, QDir::Files)) {
                QLibrary lib(d.filePath(f));
                lib.setLoadHints(QLibrary::ExportExternalSymbolsHint);
                lib.load(); // stays loaded for the life of the process
            }
        }
    }
}

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    // Under Ubuntu Touch confinement an app may only write to directories named
    // after its package. Using the click name as the organization makes
    // QSettings / Qt.labs.settings land in ~/.config/kyfly.batcastle/.
    app.setOrganizationName(QStringLiteral("kyfly.batcastle"));
    app.setApplicationName(QStringLiteral("kyfly"));
    app.setApplicationVersion(QStringLiteral(KYFLY_VERSION));

    QCommandLineParser cli;
    cli.addHelpOption();
    cli.addOptions({
        {QStringLiteral("host"), QStringLiteral("Drone IP address (testing)."), QStringLiteral("ip")},
        {QStringLiteral("windowed"), QStringLiteral("Don't go fullscreen.")},
        {QStringLiteral("screenshot"), QStringLiteral("Dev: save a window grab to <file> and quit."), QStringLiteral("file")},
        {QStringLiteral("screenshot-delay"), QStringLiteral("Dev: ms before --screenshot."), QStringLiteral("ms"), QStringLiteral("4000")},
        {QStringLiteral("arm"), QStringLiteral("Dev: enable flight controls at start.")},
        {QStringLiteral("media-dir"), QStringLiteral("Where photos and videos go (testing)."), QStringLiteral("dir")},
        {QStringLiteral("list-image-formats"), QStringLiteral("Diagnostics: print the image formats photos can use, then exit.")},
    });
    cli.process(app);
    setupBundledImagePlugins();
    if (cli.isSet(QStringLiteral("list-image-formats"))) {
        std::printf("%s\n", qPrintable(MediaCapture(QDir::tempPath()).availablePhotoFormats().join(' ')));
        return 0;
    }

    qmlRegisterType<VideoItem>("KyFly", 1, 0, "VideoItem");
    qmlRegisterUncreatableType<FlightInput>("KyFly", 1, 0, "FlightInput", QStringLiteral("use 'controls'"));
    qmlRegisterUncreatableType<VideoReceiver>("KyFly", 1, 0, "VideoReceiver", QStringLiteral("use 'video'"));
    qmlRegisterUncreatableType<DroneLink>("KyFly", 1, 0, "DroneLink", QStringLiteral("use 'drone'"));
    qmlRegisterUncreatableType<MediaCapture>("KyFly", 1, 0, "MediaCapture", QStringLiteral("use 'capture'"));

    // Declaration order matters: objects are destroyed in reverse, so the video
    // worker (which feeds `capture`) goes first and `capture` finalizes any
    // recording last. Inside the app's data folder, the only place a confined
    // app may write: ~/.local/share/kyfly.batcastle/kyfly/{Pictures,Videos}
    FlightInput input;
    const QString mediaRoot = cli.isSet(QStringLiteral("media-dir"))
                                  ? cli.value(QStringLiteral("media-dir"))
                                  : QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    MediaCapture capture(mediaRoot);
    DroneLink drone;
    VideoReceiver video;
    ScreenKeeper screenKeeper;
    screenKeeper.setKeepOn(true);
    drone.setInput(&input);
    video.setSink(&capture);
    QObject::connect(&drone, &DroneLink::videoRestartRequested, &video, &VideoReceiver::restart);
    // Photo/record from the drone's remote controller or any FlightInput source.
    QObject::connect(&drone, &DroneLink::photoRequested, &capture, &MediaCapture::takePhoto);
    QObject::connect(&drone, &DroneLink::recordToggleRequested, &capture, &MediaCapture::toggleRecording);

    // Lifecycle. When the shell suspends us, stop flying and streaming cleanly
    // (disarm sends 08 01). Merely losing focus only centres the touch sticks.
    QObject::connect(&app, &QGuiApplication::applicationStateChanged, [&](Qt::ApplicationState s) {
        if (s == Qt::ApplicationSuspended || s == Qt::ApplicationHidden) {
            drone.setActive(false);
            capture.stopRecording(); // finalize the file while we still can
            video.stop();
            screenKeeper.setKeepOn(false);
        } else if (s == Qt::ApplicationInactive) {
            input.releaseSource(FlightInput::Touch);
        } else if (s == Qt::ApplicationActive) {
            screenKeeper.setKeepOn(true);
            drone.setActive(true);
            if (!video.running())
                video.start();
        }
    });

    // Always tell the drone controls are off when we exit.
    QObject::connect(&app, &QCoreApplication::aboutToQuit, [&]() { drone.setActive(false); });

    QQmlApplicationEngine engine;
    QQmlContext *ctx = engine.rootContext();
    ctx->setContextProperty(QStringLiteral("controls"), &input);
    ctx->setContextProperty(QStringLiteral("drone"), &drone);
    ctx->setContextProperty(QStringLiteral("video"), &video);
    ctx->setContextProperty(QStringLiteral("screenKeeper"), &screenKeeper);
    ctx->setContextProperty(QStringLiteral("capture"), &capture);
    ctx->setContextProperty(QStringLiteral("cliHost"), cli.value(QStringLiteral("host")));
    ctx->setContextProperty(QStringLiteral("cliWindowed"), cli.isSet(QStringLiteral("windowed")));
    ctx->setContextProperty(QStringLiteral("cliArm"), cli.isSet(QStringLiteral("arm")));
    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    if (engine.rootObjects().isEmpty())
        return 1;

    if (cli.isSet(QStringLiteral("screenshot"))) {
        auto *win = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
        const QString file = cli.value(QStringLiteral("screenshot"));
        QTimer::singleShot(cli.value(QStringLiteral("screenshot-delay")).toInt(), [win, file]() {
            win->grabWindow().save(file);
            QCoreApplication::quit();
        });
    }
    return app.exec();
}
