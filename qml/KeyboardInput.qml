// SPDX-License-Identifier: GPL-3.0-or-later
// Keyboard flight input. Mostly for desktop testing, but it is also a template
// for any non-touch input device: it only talks to `controls` (FlightInput),
// exactly like a gamepad backend will.
//
//   W/S throttle   A/D yaw   arrows pitch/roll
//   T takeoff  L land  Space emergency stop  F flip  C camera
//   Enter controls on/off  H headless  G gyro calibrate  1/2/3 speed
import QtQuick 2.15
import KyFly 1.0

Item {
    id: kb
    focus: true
    property var held: ({})
    // Shown in the debug overlay. Lets you check on a device whether a paired
    // gamepad's buttons reach the app as key events (see GAMEPAD.md).
    property string lastKey: "—"
    function noteKey(event) {
        lastKey = "key 0x" + event.key.toString(16) + "  scan " + event.nativeScanCode +
                  (event.text && event.text.charCodeAt(0) > 31 ? "  '" + event.text + "'" : "")
    }
    property real throttle: controls.throttleSprings ? 0 : -1

    function axis(neg, pos) { return (held[pos] ? 1 : 0) - (held[neg] ? 1 : 0) }

    function push() {
        if (controls.throttleSprings)
            throttle = axis(Qt.Key_S, Qt.Key_W)
        controls.setAxes(FlightInput.Keyboard,
                         axis(Qt.Key_A, Qt.Key_D), throttle,
                         axis(Qt.Key_Left, Qt.Key_Right), axis(Qt.Key_Down, Qt.Key_Up))
    }

    // Unsprung throttle ramps while W/S are held, like moving a real stick.
    Timer {
        interval: 20
        repeat: true
        running: !controls.throttleSprings && !!(kb.held[Qt.Key_W] || kb.held[Qt.Key_S])
        onTriggered: {
            kb.throttle = Math.max(-1, Math.min(1, kb.throttle + kb.axis(Qt.Key_S, Qt.Key_W) * 0.03))
            kb.push()
        }
    }

    Connections {
        target: controls
        // Keep our idea of throttle in sync when it is reset elsewhere.
        function onAxesChanged() {
            if (controls.activeSource !== FlightInput.Keyboard) kb.throttle = controls.throttle
        }
    }

    Keys.onPressed: (event) => {
        noteKey(event)
        if (event.isAutoRepeat) { event.accepted = true; return }
        var k = event.key
        var axes = [Qt.Key_W, Qt.Key_S, Qt.Key_A, Qt.Key_D, Qt.Key_Up, Qt.Key_Down, Qt.Key_Left, Qt.Key_Right]
        if (axes.indexOf(k) >= 0) {
            var h = held; h[k] = true; held = h
            push()
        } else if (k === Qt.Key_T) controls.trigger(drone.packetFormat === 20 ? FlightInput.TakeOffOrLand : FlightInput.TakeOff)
        else if (k === Qt.Key_L) controls.trigger(drone.packetFormat === 20 ? FlightInput.TakeOffOrLand : FlightInput.Land)
        else if (k === Qt.Key_Space) controls.trigger(FlightInput.EmergencyStop)
        else if (k === Qt.Key_F) controls.trigger(FlightInput.ArmFlip)
        else if (k === Qt.Key_C) controls.trigger(FlightInput.SwitchCamera)
        else if (k === Qt.Key_Return || k === Qt.Key_Enter) controls.trigger(FlightInput.ToggleArmed)
        else if (k === Qt.Key_H) controls.trigger(FlightInput.ToggleHeadless)
        else if (k === Qt.Key_G) controls.trigger(FlightInput.CalibrateGyro)
        else if (k === Qt.Key_1) drone.speedLevel = 0
        else if (k === Qt.Key_2) drone.speedLevel = 1
        else if (k === Qt.Key_3) drone.speedLevel = 2
        else return
        event.accepted = true
    }
    Keys.onReleased: (event) => {
        if (event.isAutoRepeat) { event.accepted = true; return }
        if (held[event.key]) {
            var h = held; delete h[event.key]; held = h
            push()
            event.accepted = true
        }
    }
}
