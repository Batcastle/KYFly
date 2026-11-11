// SPDX-License-Identifier: GPL-3.0-or-later
#include "dronelink.h"

#include <QTimer>
#include <QUdpSocket>

namespace {
constexpr int kHeartbeatMs = 1000;
constexpr int kLinkTimeoutMs = 3000;
constexpr int kPulseMs = 1000;      // takeoff / land / stop, as in FlyController
constexpr int kGyroPulseMs = 2000;
constexpr int kFlipPulseMs = 12 * DroneLink::kSendIntervalMs; // 600 ms
constexpr qreal kFlipDeflection = 0.7;
constexpr qreal kFlipCrossAxis = 0.25;
constexpr int kTrimLimit = 12;

// Right-stick travel per speed level (DeviceGLFragment.getHalfLen)
const int kHalfLen[3] = {45, 60, 127};
const int kTrimMult[3] = {2, 3, 4};
const int kSpeedPercent[3] = {30, 60, 100};

inline int clampByte(int v) { return v < 1 ? 1 : (v > 255 ? 255 : v); }

bool isNoGL(int i) // WifiIdUtils.isNoGL -> 8-byte frames
{
    switch (i) {
    case 5: case 9: case 12: case 19: case 20: case 21: case 23: case 24:
    case 31: case 45: case 51: case 63: case 64: case 65: case 66: case 67: case 72:
        return true;
    default:
        return false;
    }
}

bool isGL(int i) // WifiIdUtils.isGL -> 20-byte frames
{
    return i == 85 || (i >= 90 && i <= 101) || i == 103;
}
} // namespace

DroneLink::DroneLink(QObject *parent)
    : QObject(parent)
    , m_heartbeat(new QTimer(this))
    , m_sendTimer(new QTimer(this))
    , m_linkCheck(new QTimer(this))
{
    m_clock.start();
    m_heartbeat->setInterval(kHeartbeatMs);
    m_sendTimer->setInterval(kSendIntervalMs);
    m_sendTimer->setTimerType(Qt::PreciseTimer);
    m_linkCheck->setInterval(500);
    connect(m_heartbeat, &QTimer::timeout, this, &DroneLink::sendHeartbeat);
    connect(m_sendTimer, &QTimer::timeout, this, &DroneLink::onSendTick);
    connect(m_linkCheck, &QTimer::timeout, this, &DroneLink::onLinkCheck);
}

// ---------------------------------------------------------------- static helpers

int DroneLink::formatForModel(int modelId)
{
    if (isNoGL(modelId))
        return 8;
    if (isGL(modelId))
        return 20;
    return 20; // app's deviceType stays 0 for unknown IDs -> 20-byte path
}

QByteArray DroneLink::buildPacket(int format, const ControlFrame &f)
{
    // Same sanitising FlyControlTask does right before sending.
    const int r = clampByte(f.roll), p = clampByte(f.pitch), y = clampByte(f.yaw);
    const int t = (f.throttle == 1) ? 0 : qBound(0, f.throttle, 255);
    QByteArray b;
    b.append(char(0x03)); // "flight control" prefix used on the UDP path
    if (format == 8) {
        int flags = 0;
        if (f.takeOff) flags |= 0x01;
        if (f.land) flags |= 0x02;
        if (f.emergencyStop) flags |= 0x04;
        if (f.flip) flags |= 0x08;
        if (f.headless) flags |= 0x10;
        if (f.fastReturn) flags |= 0x20;
        if (f.light) flags |= 0x40;
        if (f.gyro) flags |= 0x80;
        const int x = r ^ p ^ t ^ y ^ flags;
        const char pkt[8] = {char(0x66), char(r), char(p), char(t), char(y),
                             char(flags), char(x), char(0x99)};
        b.append(pkt, 8);
    } else {
        int f1 = 0, f2 = 0;
        if (f.takeOff || f.land) f1 |= 0x01; // one toggle bit in this family
        if (f.emergencyStop) f1 |= 0x02;
        if (f.gyro) f1 |= 0x04;
        if (f.flip) f1 |= 0x08;
        if (f.light) f1 |= 0x10;
        if (f.headless) f2 |= 0x01;
        if (f.altitudeHold) f2 |= 0x02;
        const int x = f1 ^ r ^ p ^ t ^ y ^ f2;
        QByteArray pkt(20, '\0');
        pkt[0] = char(0x66);
        pkt[1] = char(0x14);
        pkt[2] = char(r);
        pkt[3] = char(p);
        pkt[4] = char(t);
        pkt[5] = char(y);
        pkt[6] = char(f1);
        pkt[7] = char(f2);
        pkt[18] = char(x);
        pkt[19] = char(0x99);
        b.append(pkt);
    }
    return b;
}

void DroneLink::mapAxes(const FlightInput::Axes &a, int speedLevel, int trimRoll, int trimPitch,
                        int trimYaw, ControlFrame &f)
{
    const int lvl = qBound(0, speedLevel, 2);
    // Left stick: yaw with the app's +-24 deadband, throttle full range.
    int yaw = int(a.yaw * 127.0);
    if (yaw + 128 >= 104 && yaw + 128 <= 152)
        yaw = 0;
    f.yaw = clampByte(yaw + 128 + trimYaw * 2);
    f.throttle = int(a.throttle * 127.0) + 128;
    if (f.throttle == 1)
        f.throttle = 0; // FlyController quirk: bottom of stick sends 0
    // Right stick: travel limited by speed level, plus trims.
    f.roll = clampByte(int(a.roll * kHalfLen[lvl]) + 128 + trimRoll * kTrimMult[lvl]);
    f.pitch = clampByte(int(a.pitch * kHalfLen[lvl]) + 128 + trimPitch * kTrimMult[lvl]);
}

// ---------------------------------------------------------------- properties

void DroneLink::setHost(const QString &h)
{
    QHostAddress a(h);
    if (a.isNull() || a == m_host)
        return;
    m_host = a;
    emit hostChanged();
}

void DroneLink::setActive(bool on)
{
    if (on == m_active)
        return;
    m_active = on;
    if (on) {
        ensureSocket();
        sendHeartbeat();
        QTimer::singleShot(500, this, &DroneLink::sendHeartbeat); // app sends again at +500 ms
        m_heartbeat->start();
        m_linkCheck->start();
    } else {
        setArmed(false);
        m_heartbeat->stop();
        m_linkCheck->stop();
        if (m_connected) {
            m_connected = false;
            emit connectedChanged();
        }
    }
    emit activeChanged();
}

void DroneLink::setFormatOverride(int f)
{
    if (f != 0 && f != 8 && f != 20)
        f = 0;
    if (f == m_formatOverride)
        return;
    const bool oldHold = altitudeHold();
    m_formatOverride = f;
    emit formatChanged();
    if (altitudeHold() != oldHold) {
        if (m_input) {
            m_input->setThrottleSprings(altitudeHold());
            m_input->resetThrottle();
        }
        emit altitudeHoldChanged();
    }
}

int DroneLink::packetFormat() const
{
    return m_formatOverride ? m_formatOverride : formatForModel(m_modelId);
}

void DroneLink::setArmed(bool on)
{
    if (on == m_armed)
        return;
    m_armed = on;
    if (on) {
        ensureSocket();
        if (m_input) {
            m_input->setThrottleSprings(altitudeHold());
            m_input->resetThrottle();
        }
        m_flipArmed = false;
        m_sendTimer->start();
    } else {
        m_sendTimer->stop();
        send(QByteArray("\x08\x01", 2)); // "control mode off" (FlyController.setController(false))
        m_takeOffUntil = m_landUntil = m_stopUntil = m_gyroUntil = m_flipUntil = 0;
        if (m_flipArmed) {
            m_flipArmed = false;
            emit flipArmedChanged();
        }
    }
    emit armedChanged();
}

void DroneLink::setSpeedLevel(int l)
{
    l = qBound(0, l, 2);
    if (l == m_speedLevel)
        return;
    m_speedLevel = l;
    emit speedLevelChanged();
}

int DroneLink::speedPercent() const { return kSpeedPercent[m_speedLevel]; }

bool DroneLink::altitudeHold() const
{
    return packetFormat() == 20 ? true : m_altitudeHold; // 20-byte family always holds altitude
}

void DroneLink::setAltitudeHold(bool on)
{
    if (altitudeHoldLocked() || on == m_altitudeHold)
        return;
    m_altitudeHold = on;
    if (m_input) {
        m_input->setThrottleSprings(on);
        m_input->resetThrottle();
    }
    emit altitudeHoldChanged();
}

void DroneLink::setHeadless(bool on)
{
    if (on == m_headless) return;
    m_headless = on;
    emit headlessChanged();
}

void DroneLink::setLight(bool on)
{
    if (on == m_light) return;
    m_light = on;
    emit lightChanged();
}

void DroneLink::setTrimRoll(int v)
{
    v = qBound(-kTrimLimit, v, kTrimLimit);
    if (v != m_trimRoll) { m_trimRoll = v; emit trimChanged(); }
}

void DroneLink::setTrimPitch(int v)
{
    v = qBound(-kTrimLimit, v, kTrimLimit);
    if (v != m_trimPitch) { m_trimPitch = v; emit trimChanged(); }
}

void DroneLink::setTrimYaw(int v)
{
    v = qBound(-kTrimLimit, v, kTrimLimit);
    if (v != m_trimYaw) { m_trimYaw = v; emit trimChanged(); }
}

void DroneLink::setInput(FlightInput *in)
{
    if (in == m_input)
        return;
    if (m_input)
        disconnect(m_input, nullptr, this, nullptr);
    m_input = in;
    if (m_input) {
        connect(m_input, &FlightInput::actionTriggered, this, &DroneLink::onAction);
        m_input->setThrottleSprings(altitudeHold());
    }
    emit inputChanged();
}

void DroneLink::setFlying(bool on)
{
    if (on == m_flying) return;
    m_flying = on;
    emit flyingChanged();
}

// ---------------------------------------------------------------- commands

void DroneLink::takeOff()
{
    if (packetFormat() == 20) { // separate takeoff would toggle; only send if grounded
        if (!m_flying) takeOffOrLand();
        return;
    }
    m_takeOffUntil = m_clock.elapsed() + kPulseMs;
    setFlying(true);
}

void DroneLink::land()
{
    if (packetFormat() == 20) {
        if (m_flying) takeOffOrLand();
        return;
    }
    m_landUntil = m_clock.elapsed() + kPulseMs;
    setFlying(false);
}

void DroneLink::takeOffOrLand()
{
    // DeviceGLFragment: one button, toggles between FastFly and FastDrop.
    if (m_flying) {
        m_landUntil = m_clock.elapsed() + kPulseMs;
        setFlying(false);
    } else {
        m_takeOffUntil = m_clock.elapsed() + kPulseMs;
        setFlying(true);
    }
}

void DroneLink::emergencyStop()
{
    m_stopUntil = m_clock.elapsed() + kPulseMs;
    setFlying(false);
    if (!m_armed) {
        // Make sure a stop always goes out, even with controls off.
        ControlFrame f = currentFrame();
        f.emergencyStop = true;
        for (int i = 0; i < 3; ++i)
            send(buildPacket(packetFormat(), f));
    }
}

void DroneLink::calibrateGyro()
{
    m_gyroUntil = m_clock.elapsed() + kGyroPulseMs;
}

void DroneLink::toggleFlip()
{
    if (!flipSupported() || pulseActive(m_flipUntil))
        return;
    m_flipArmed = !m_flipArmed;
    emit flipArmedChanged();
}

void DroneLink::switchCamera()
{
    m_secondCamera = !m_secondCamera;
    send(QByteArray(m_secondCamera ? "\x06\x02" : "\x06\x01", 2));
    // Heartbeat reply byte 1 says how the video must be restarted (SocketClient.switchCamera).
    if (m_cameraResetMode == 1)
        emit videoRestartRequested(0);
    else if (m_cameraResetMode == 2)
        emit videoRestartRequested(600);
}

void DroneLink::sendRaw(const QByteArray &bytes)
{
    send(bytes);
}

void DroneLink::onAction(int action)
{
    switch (action) {
    case FlightInput::TakeOff: takeOff(); break;
    case FlightInput::Land: land(); break;
    case FlightInput::TakeOffOrLand: takeOffOrLand(); break;
    case FlightInput::EmergencyStop: emergencyStop(); break;
    case FlightInput::CalibrateGyro: calibrateGyro(); break;
    case FlightInput::ToggleHeadless: setHeadless(!m_headless); break;
    case FlightInput::ToggleAltitudeHold: setAltitudeHold(!m_altitudeHold); break;
    case FlightInput::ToggleLight: setLight(!m_light); break;
    case FlightInput::ArmFlip: toggleFlip(); break;
    case FlightInput::SwitchCamera: switchCamera(); break;
    case FlightInput::SpeedUp: setSpeedLevel(m_speedLevel + 1); break;
    case FlightInput::SpeedDown: setSpeedLevel(m_speedLevel - 1); break;
    case FlightInput::ToggleArmed: setArmed(!m_armed); break;
    case FlightInput::RotateVideo: emit rotateVideoRequested(); break;
    case FlightInput::TrimRollLeft: setTrimRoll(m_trimRoll - 1); break;
    case FlightInput::TrimRollRight: setTrimRoll(m_trimRoll + 1); break;
    case FlightInput::TrimPitchBack: setTrimPitch(m_trimPitch - 1); break;
    case FlightInput::TrimPitchForward: setTrimPitch(m_trimPitch + 1); break;
    case FlightInput::TrimYawLeft: setTrimYaw(m_trimYaw - 1); break;
    case FlightInput::TrimYawRight: setTrimYaw(m_trimYaw + 1); break;
    default: break;
    }
}

// ---------------------------------------------------------------- networking

void DroneLink::ensureSocket()
{
    if (m_socket)
        return;
    m_socket = new QUdpSocket(this);
    m_socket->bind(QHostAddress(QHostAddress::AnyIPv4), 0);
    connect(m_socket, &QUdpSocket::readyRead, this, &DroneLink::onReadyRead);
}

void DroneLink::send(const QByteArray &bytes)
{
    ensureSocket();
    m_socket->writeDatagram(bytes, m_host, kControlPort);
}

void DroneLink::sendHeartbeat()
{
    if (m_active)
        send(QByteArray("\x01\x01", 2));
}

void DroneLink::onReadyRead()
{
    while (m_socket->hasPendingDatagrams()) {
        QByteArray d(int(m_socket->pendingDatagramSize()), '\0');
        QHostAddress from;
        m_socket->readDatagram(d.data(), d.size(), &from);
        if (d.isEmpty())
            continue;
        m_lastReplyAt = m_clock.elapsed();
        if (!m_connected) {
            m_connected = true;
            emit connectedChanged();
        }
        const QString hex = QString::fromLatin1(d.toHex(' '));
        if (hex != m_lastReplyHex) {
            m_lastReplyHex = hex;
            emit lastReplyChanged();
        }
        const int b0 = uchar(d.at(0));
        if (b0 == 0x66)
            continue; // remote-controller button events (photo/record), not status
        if (b0 != m_modelId) {
            const int oldFormat = packetFormat();
            const bool oldHold = altitudeHold();
            m_modelId = b0;
            emit modelIdChanged();
            if (packetFormat() != oldFormat)
                emit formatChanged();
            if (altitudeHold() != oldHold) {
                if (m_input) {
                    m_input->setThrottleSprings(altitudeHold());
                    m_input->resetThrottle();
                }
                emit altitudeHoldChanged();
            }
        }
        if (d.size() >= 2)
            m_cameraResetMode = uchar(d.at(1));
    }
}

void DroneLink::onLinkCheck()
{
    const bool ok = m_lastReplyAt >= 0 && m_clock.elapsed() - m_lastReplyAt < kLinkTimeoutMs;
    if (ok != m_connected) {
        m_connected = ok;
        emit connectedChanged();
    }
}

DroneLink::ControlFrame DroneLink::currentFrame()
{
    ControlFrame f;
    const qint64 now = m_clock.elapsed();
    FlightInput::Axes a = m_input ? m_input->axes() : FlightInput::Axes{0, altitudeHold() ? 0.0 : -1.0, 0, 0};

    // Flip: once armed, the first decisive right-stick push picks the direction
    // (DeviceBLFragment.onMove / turn360), then the flip bit is held for 600 ms.
    if (m_flipArmed && !pulseActive(m_flipUntil)) {
        int roll = 128, pitch = 128;
        if (qAbs(a.roll) > kFlipDeflection && qAbs(a.pitch) < kFlipCrossAxis)
            roll = a.roll <= 0 ? 1 : 255;
        else if (qAbs(a.pitch) > kFlipDeflection && qAbs(a.roll) < kFlipCrossAxis)
            pitch = a.pitch <= 0 ? 1 : 255;
        if (roll != 128 || pitch != 128) {
            m_flipRoll = roll;
            m_flipPitch = pitch;
            m_frozenAxes = a;
            m_flipUntil = now + kFlipPulseMs;
            m_flipArmed = false;
            emit flipArmedChanged();
        }
    }
    const bool flipping = now < m_flipUntil;
    if (flipping)
        a = m_frozenAxes; // app ignores stick input during the flip

    mapAxes(a, m_speedLevel, m_trimRoll, m_trimPitch, m_trimYaw, f);
    if (flipping) {
        if (m_flipRoll != 128) f.roll = m_flipRoll;
        if (m_flipPitch != 128) f.pitch = m_flipPitch;
    }

    f.takeOff = now < m_takeOffUntil;
    f.land = now < m_landUntil;
    f.emergencyStop = now < m_stopUntil;
    f.gyro = now < m_gyroUntil;
    f.flip = flipping;
    f.headless = m_headless;
    f.light = m_light;
    f.altitudeHold = altitudeHold();
    return f;
}

void DroneLink::onSendTick()
{
    const QByteArray pkt = buildPacket(packetFormat(), currentFrame());
    send(pkt);
    const QString hex = QString::fromLatin1(pkt.toHex(' '));
    if (hex != m_lastPacketHex) {
        m_lastPacketHex = hex;
        emit lastPacketChanged();
    }
}
