// SPDX-License-Identifier: GPL-3.0-or-later
// UDP control link to KY UFO family drones (port 7099).
//
// Protocol recovered from com.cooingdv.kyufo 1.8.0; see PROTOCOL.md.
//   * Heartbeat 01 01 every second (video stalls without it).
//   * Reply byte 0 = model ID, which selects the 8-byte or 20-byte control format.
//   * While armed, a control frame every 50 ms, prefixed with 0x03.
//
// DroneLink never reads input devices itself. It samples a FlightInput on its
// own send clock, so touch, keyboard and (later) gamepads all work the same way.
#pragma once

#include <QByteArray>
#include <QElapsedTimer>
#include <QHostAddress>
#include <QObject>
#include <QPointer>

#include "flightinput.h"

class QTimer;
class QUdpSocket;

class DroneLink : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(int modelId READ modelId NOTIFY modelIdChanged)
    Q_PROPERTY(int formatOverride READ formatOverride WRITE setFormatOverride NOTIFY formatChanged)
    Q_PROPERTY(int packetFormat READ packetFormat NOTIFY formatChanged)
    Q_PROPERTY(bool armed READ armed WRITE setArmed NOTIFY armedChanged)
    Q_PROPERTY(int speedLevel READ speedLevel WRITE setSpeedLevel NOTIFY speedLevelChanged)
    Q_PROPERTY(int speedPercent READ speedPercent NOTIFY speedLevelChanged)
    Q_PROPERTY(bool altitudeHold READ altitudeHold WRITE setAltitudeHold NOTIFY altitudeHoldChanged)
    Q_PROPERTY(bool altitudeHoldLocked READ altitudeHoldLocked NOTIFY formatChanged)
    Q_PROPERTY(bool headless READ headless WRITE setHeadless NOTIFY headlessChanged)
    Q_PROPERTY(bool light READ light WRITE setLight NOTIFY lightChanged)
    Q_PROPERTY(bool flipArmed READ flipArmed NOTIFY flipArmedChanged)
    Q_PROPERTY(bool flipSupported READ flipSupported NOTIFY formatChanged)
    Q_PROPERTY(bool flying READ flying NOTIFY flyingChanged)
    Q_PROPERTY(int trimRoll READ trimRoll WRITE setTrimRoll NOTIFY trimChanged)
    Q_PROPERTY(int trimPitch READ trimPitch WRITE setTrimPitch NOTIFY trimChanged)
    Q_PROPERTY(int trimYaw READ trimYaw WRITE setTrimYaw NOTIFY trimChanged)
    Q_PROPERTY(QString lastPacketHex READ lastPacketHex NOTIFY lastPacketChanged)
    Q_PROPERTY(QString lastReplyHex READ lastReplyHex NOTIFY lastReplyChanged)
    Q_PROPERTY(FlightInput *input READ input WRITE setInput NOTIFY inputChanged)

public:
    // One control frame, in the drone's own units (1..255, 128 = centre).
    struct ControlFrame {
        int roll = 128, pitch = 128, throttle = 128, yaw = 128;
        bool takeOff = false, land = false, emergencyStop = false, flip = false;
        bool headless = false, fastReturn = false, light = false, gyro = false;
        bool altitudeHold = false;
    };

    static constexpr quint16 kControlPort = 7099;
    static constexpr int kSendIntervalMs = 50;

    explicit DroneLink(QObject *parent = nullptr);

    // Builds the complete UDP payload (including the 0x03 prefix).
    static QByteArray buildPacket(int format, const ControlFrame &f);
    // 8 or 20, following WifiIdUtils.isNoGL/isGL; unknown IDs use 20 like the app.
    static int formatForModel(int modelId);
    // Maps normalized pilot axes to drone units, exactly like the official app.
    static void mapAxes(const FlightInput::Axes &a, int speedLevel, int trimRoll, int trimPitch,
                        int trimYaw, ControlFrame &f);

    Q_INVOKABLE void takeOff();
    Q_INVOKABLE void land();
    Q_INVOKABLE void takeOffOrLand();
    Q_INVOKABLE void emergencyStop();
    Q_INVOKABLE void calibrateGyro();
    Q_INVOKABLE void toggleFlip();
    Q_INVOKABLE void switchCamera();
    Q_INVOKABLE void sendRaw(const QByteArray &bytes);

    QString host() const { return m_host.toString(); }
    void setHost(const QString &h);
    bool active() const { return m_active; }
    void setActive(bool on);
    bool connected() const { return m_connected; }
    int modelId() const { return m_modelId; }
    int formatOverride() const { return m_formatOverride; }
    void setFormatOverride(int f);
    int packetFormat() const;
    bool armed() const { return m_armed; }
    void setArmed(bool on);
    int speedLevel() const { return m_speedLevel; }
    void setSpeedLevel(int l);
    int speedPercent() const;
    bool altitudeHold() const;
    void setAltitudeHold(bool on);
    bool altitudeHoldLocked() const { return packetFormat() == 20; }
    bool headless() const { return m_headless; }
    void setHeadless(bool on);
    bool light() const { return m_light; }
    void setLight(bool on);
    bool flipArmed() const { return m_flipArmed; }
    bool flipSupported() const { return packetFormat() == 8; }
    bool flying() const { return m_flying; }
    int trimRoll() const { return m_trimRoll; }
    int trimPitch() const { return m_trimPitch; }
    int trimYaw() const { return m_trimYaw; }
    void setTrimRoll(int v);
    void setTrimPitch(int v);
    void setTrimYaw(int v);
    QString lastPacketHex() const { return m_lastPacketHex; }
    QString lastReplyHex() const { return m_lastReplyHex; }
    FlightInput *input() const { return m_input; }
    void setInput(FlightInput *in);

    // Build the frame that would be sent right now (used by the send tick and tests).
    ControlFrame currentFrame();

signals:
    void hostChanged();
    void activeChanged();
    void connectedChanged();
    void modelIdChanged();
    void formatChanged();
    void armedChanged();
    void speedLevelChanged();
    void altitudeHoldChanged();
    void headlessChanged();
    void lightChanged();
    void flipArmedChanged();
    void flyingChanged();
    void trimChanged();
    void lastPacketChanged();
    void lastReplyChanged();
    void inputChanged();
    // Camera switch needs the video stream restarted (after delayMs).
    void videoRestartRequested(int delayMs);
    void rotateVideoRequested();

private slots:
    void sendHeartbeat();
    void onReadyRead();
    void onSendTick();
    void onLinkCheck();
    void onAction(int action);

private:
    void send(const QByteArray &bytes);
    bool pulseActive(qint64 until) const { return m_clock.elapsed() < until; }
    void setFlying(bool on);
    void ensureSocket();

    QHostAddress m_host{QStringLiteral("192.168.1.1")};
    QUdpSocket *m_socket = nullptr;
    QTimer *m_heartbeat = nullptr;
    QTimer *m_sendTimer = nullptr;
    QTimer *m_linkCheck = nullptr;
    QElapsedTimer m_clock;
    qint64 m_lastReplyAt = -1;
    QPointer<FlightInput> m_input;

    bool m_active = false;
    bool m_connected = false;
    int m_modelId = 0;
    int m_cameraResetMode = 0;
    bool m_secondCamera = false;
    int m_formatOverride = 0;
    bool m_armed = false;
    int m_speedLevel = 0;
    bool m_altitudeHold = false;
    bool m_headless = false;
    bool m_light = false;
    bool m_flipArmed = false;
    bool m_flying = false;
    int m_trimRoll = 0, m_trimPitch = 0, m_trimYaw = 0;

    // momentary button pulses (monotonic ms deadlines)
    qint64 m_takeOffUntil = 0, m_landUntil = 0, m_stopUntil = 0, m_gyroUntil = 0;
    qint64 m_flipUntil = 0;
    int m_flipRoll = 128, m_flipPitch = 128;
    FlightInput::Axes m_frozenAxes;

    QString m_lastPacketHex;
    QString m_lastReplyHex;
};
