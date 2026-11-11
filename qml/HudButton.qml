// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15

Rectangle {
    id: btn
    property string text: ""
    property string sub: ""
    property bool checked: false
    property color accent: "#4fc3f7"
    property bool danger: false
    property real unit: 40
    signal clicked()

    implicitWidth: Math.max(unit * 2.2, label.implicitWidth + unit * 0.6)
    implicitHeight: unit * 1.15
    radius: height / 4
    opacity: enabled ? 1 : 0.35
    color: danger ? (area.pressed ? "#ff5252" : "#d32f2f")
                  : (checked ? Qt.rgba(accent.r, accent.g, accent.b, 0.85)
                             : (area.pressed ? "#99ffffff" : "#66000000"))
    border.color: danger ? "#ffcdd2" : (checked ? "white" : "#55ffffff")
    border.width: 1

    Column {
        anchors.centerIn: parent
        Text {
            id: label
            anchors.horizontalCenter: parent.horizontalCenter
            text: btn.text
            color: "white"
            font.pixelSize: btn.unit * 0.38
            font.bold: true
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: btn.sub !== ""
            text: btn.sub
            color: "#ddffffff"
            font.pixelSize: btn.unit * 0.24
        }
    }
    MouseArea {
        id: area
        anchors.fill: parent
        onClicked: btn.clicked()
    }
}
