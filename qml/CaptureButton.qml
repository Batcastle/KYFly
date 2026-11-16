// SPDX-License-Identifier: GPL-3.0-or-later
// Round shutter-style button for photo / record.
import QtQuick 2.15

Item {
    id: btn
    property real size: 56
    property bool recordStyle: false   // red dot instead of a white disc
    property bool active: false        // recording in progress
    property string label: ""
    signal clicked()

    width: size
    height: size + labelText.height + size * 0.05
    opacity: enabled ? 1 : 0.4

    Rectangle {
        id: ring
        width: btn.size; height: width; radius: width / 2
        color: area.pressed ? "#88ffffff" : "#66000000"
        border.color: btn.active ? "#ff5252" : "white"
        border.width: Math.max(2, btn.size * 0.06)

        Rectangle { // inner disc / dot / stop square
            anchors.centerIn: parent
            width: btn.active ? parent.width * 0.36 : parent.width * (btn.recordStyle ? 0.5 : 0.64)
            height: width
            radius: btn.active ? width * 0.18 : width / 2
            color: btn.recordStyle ? "#f44336" : "white"
            Behavior on width { NumberAnimation { duration: 120 } }
        }
    }
    Text {
        id: labelText
        anchors { top: ring.bottom; topMargin: btn.size * 0.05; horizontalCenter: ring.horizontalCenter }
        text: btn.label
        color: btn.active ? "#ff8a80" : "white"
        font.pixelSize: Math.max(10, btn.size * 0.24)
        font.bold: btn.active
        style: Text.Outline; styleColor: "black"
    }
    MouseArea {
        id: area
        anchors.fill: ring
        onClicked: btn.clicked()
    }
}
