// SPDX-License-Identifier: GPL-3.0-or-later
// Device-agnostic pilot input.
//
// Every input device (on-screen sticks, keyboard, and later Bluetooth/USB
// gamepads) feeds this one object. Nothing else in the app knows which device
// is in use:
//
//   * Continuous input goes through setAxes(source, yaw, throttle, roll, pitch),
//     all normalized to -1..+1 (right/up/forward positive).
//   * Discrete input (buttons) goes through trigger(action).
//
// DroneLink samples axes() on its own 50 ms send clock, so input devices can
// report at any rate (a gamepad might send 250 events/s, touch ~60/s) without
// affecting what is sent to the drone.
//
// Arbitration: the most recently *deliberately moved* source wins. A source
// only takes over from another one when it moves an axis past a threshold,
// so a resting gamepad with stick drift can't fight the touch sticks.
//
// To add a gamepad later: write a class that reads the device and calls
// setAxes(Gamepad, ...), trigger(...) and releaseSource(Gamepad) on disconnect.
// See GAMEPAD.md.
#pragma once

#include <QElapsedTimer>
#include <QObject>

class FlightInput : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int activeSource READ activeSource NOTIFY activeSourceChanged)
    Q_PROPERTY(qreal yaw READ yaw NOTIFY axesChanged)
    Q_PROPERTY(qreal throttle READ throttle NOTIFY axesChanged)
    Q_PROPERTY(qreal roll READ roll NOTIFY axesChanged)
    Q_PROPERTY(qreal pitch READ pitch NOTIFY axesChanged)
    // When true (altitude hold), throttle returns to centre on releaseSource().
    // When false, throttle is left where it was, like a non-sprung RC stick.
    Q_PROPERTY(bool throttleSprings READ throttleSprings WRITE setThrottleSprings NOTIFY throttleSpringsChanged)

public:
    enum Source { NoSource = 0, Touch = 1, Keyboard = 2, Gamepad = 3, Scripted = 4 };
    Q_ENUM(Source)

    // Named actions. Touch buttons and gamepad buttons both map onto these.
    enum Action {
        TakeOff = 0,
        Land,
        TakeOffOrLand,      // single toggle used by the 20-byte drone family
        EmergencyStop,
        CalibrateGyro,
        ToggleHeadless,
        ToggleAltitudeHold,
        ToggleLight,
        ArmFlip,
        SwitchCamera,
        SpeedUp,
        SpeedDown,
        ToggleArmed,        // flight controls on/off
        RotateVideo,
        TrimRollLeft, TrimRollRight,
        TrimPitchBack, TrimPitchForward,
        TrimYawLeft, TrimYawRight,
        ActionCount
    };
    Q_ENUM(Action)

    struct Axes { qreal yaw = 0, throttle = -1, roll = 0, pitch = 0; };

    explicit FlightInput(QObject *parent = nullptr);

    Q_INVOKABLE void setAxes(int source, qreal yaw, qreal throttle, qreal roll, qreal pitch);
    Q_INVOKABLE void releaseSource(int source);
    Q_INVOKABLE void trigger(int action);
    // Reset throttle to its resting position (bottom, or centre with altitude hold).
    Q_INVOKABLE void resetThrottle();

    Axes axes() const { return m_axes; }
    int activeSource() const { return m_active; }
    qreal yaw() const { return m_axes.yaw; }
    qreal throttle() const { return m_axes.throttle; }
    qreal roll() const { return m_axes.roll; }
    qreal pitch() const { return m_axes.pitch; }
    bool throttleSprings() const { return m_throttleSprings; }
    void setThrottleSprings(bool on);

    // Milliseconds since the active source last reported (for watchdogs).
    qint64 msSinceLastInput() const { return m_lastInput.isValid() ? m_lastInput.elapsed() : -1; }

    static constexpr qreal kTakeoverThreshold = 0.15;

signals:
    void axesChanged();
    void activeSourceChanged();
    void throttleSpringsChanged();
    void actionTriggered(int action);
    // Emitted when the active source goes away (e.g. gamepad disconnected).
    void sourceLost(int source);

private:
    static qreal clamp1(qreal v) { return v < -1 ? -1 : (v > 1 ? 1 : v); }
    Axes m_axes;
    int m_active = NoSource;
    bool m_throttleSprings = false;
    QElapsedTimer m_lastInput;
};
