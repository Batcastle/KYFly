// SPDX-License-Identifier: GPL-3.0-or-later
// Drag-relative virtual stick. Press anywhere inside the item; the knob moves
// by how far the finger travels. Sprung axes return to centre on release,
// unsprung axes (throttle without altitude hold) stay where they were left.
//
// When not being touched the knob shows whatever value is set from outside,
// so the sticks also visualize keyboard or gamepad input.
import QtQuick 2.15

Item {
    id: stick

    property bool springX: true
    property bool springY: true
    property real xValue: 0          // -1..1, right positive
    property real yValue: 0          // -1..1, up positive
    property string labelX: ""
    property string labelY: ""
    property real baseSize: Math.min(width, height) * 0.62
    property real baseX: width / 2   // centre of the drawn base
    property real baseY: height / 2
    readonly property bool pressed: touch.pressedNow
    readonly property real travel: baseSize / 2 - knob.width / 2

    signal moved(real x, real y)

    function clamp(v) { return Math.max(-1, Math.min(1, v)) }

    Rectangle {
        id: base
        width: stick.baseSize
        height: width
        radius: width / 2
        x: stick.baseX - width / 2
        y: stick.baseY - height / 2
        color: "#30000000"
        border.color: stick.pressed ? "#ccffffff" : "#66ffffff"
        border.width: 2

        Rectangle { // cross-hair
            anchors.centerIn: parent; width: parent.width * 0.9; height: 1; color: "#33ffffff"
        }
        Rectangle {
            anchors.centerIn: parent; height: parent.height * 0.9; width: 1; color: "#33ffffff"
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.bottom
            anchors.topMargin: 4
            text: stick.labelY + (stick.labelX ? " / " + stick.labelX : "")
            color: "#aaffffff"
            font.pixelSize: Math.max(10, stick.baseSize * 0.07)
        }
    }

    Rectangle {
        id: knob
        width: stick.baseSize * 0.36
        height: width
        radius: width / 2
        x: stick.baseX + stick.xValue * stick.travel - width / 2
        y: stick.baseY - stick.yValue * stick.travel - height / 2
        color: stick.pressed ? "#ddffffff" : "#88ffffff"
        border.color: "#ff000000"
        border.width: 1
    }

    MultiPointTouchArea {
        id: touch
        anchors.fill: parent
        maximumTouchPoints: 1
        mouseEnabled: true
        property bool pressedNow: false
        property real startX: 0
        property real startY: 0
        property real startXValue: 0
        property real startYValue: 0
        touchPoints: [ TouchPoint { id: tp } ]

        onPressed: {
            pressedNow = true
            startX = tp.x; startY = tp.y
            startXValue = stick.springX ? 0 : stick.xValue
            startYValue = stick.springY ? 0 : stick.yValue
        }
        onUpdated: {
            stick.xValue = stick.clamp(startXValue + (tp.x - startX) / stick.travel)
            stick.yValue = stick.clamp(startYValue - (tp.y - startY) / stick.travel)
            stick.moved(stick.xValue, stick.yValue)
        }
        function release() {
            pressedNow = false
            if (stick.springX) stick.xValue = 0
            if (stick.springY) stick.yValue = 0
            stick.moved(stick.xValue, stick.yValue)
        }
        onReleased: release()
        onCanceled: release()
    }
}
