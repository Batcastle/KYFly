// SPDX-License-Identifier: GPL-3.0-or-later
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QTimer>

#include "dronelink.h"
#include "flightinput.h"
#include "screenkeeper.h"
#include "videoitem.h"
#include "videoreceiver.h"

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
    });
    cli.process(app);

    qmlRegisterType<VideoItem>("KyFly", 1, 0, "VideoItem");
    qmlRegisterUncreatableType<FlightInput>("KyFly", 1, 0, "FlightInput", QStringLiteral("use 'controls'"));
    qmlRegisterUncreatableType<VideoReceiver>("KyFly", 1, 0, "VideoReceiver", QStringLiteral("use 'video'"));
    qmlRegisterUncreatableType<DroneLink>("KyFly", 1, 0, "DroneLink", QStringLiteral("use 'drone'"));

    FlightInput input;
    DroneLink drone;
    VideoReceiver video;
    ScreenKeeper screenKeeper;
    screenKeeper.setKeepOn(true);
    drone.setInput(&input);
    QObject::connect(&drone, &DroneLink::videoRestartRequested, &video, &VideoReceiver::restart);

    // Lifecycle. When the shell suspends us, stop flying and streaming cleanly
    // (disarm sends 08 01). Merely losing focus only centres the touch sticks.
    QObject::connect(&app, &QGuiApplication::applicationStateChanged, [&](Qt::ApplicationState s) {
        if (s == Qt::ApplicationSuspended || s == Qt::ApplicationHidden) {
            drone.setActive(false);
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
