// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Popup {
    id: panel
    property real unit: 40
    property var store
    signal hostEdited(string h)

    modal: true
    focus: true
    anchors.centerIn: Overlay.overlay
    width: Math.min(parent ? parent.width * 0.9 : 800, unit * 14)
    height: parent ? parent.height * 0.9 : 600
    padding: unit * 0.3

    background: Rectangle { color: "#ee202124"; radius: 10; border.color: "#44ffffff" }

    // Explicit dark palette so text stays readable whichever Controls style
    // the system applies (Default on desktop, Suru on Ubuntu Touch).
    palette.window: "#202124"
    palette.windowText: "white"
    palette.text: "white"
    palette.base: "#303134"
    palette.button: "#3c4043"
    palette.buttonText: "white"
    palette.highlight: "#4fc3f7"
    palette.highlightedText: "black"
    palette.mid: "#5f6368"
    palette.light: "#5f6368"
    palette.dark: "#9aa0a6"


    Flickable {
        anchors.fill: parent
        contentHeight: col.implicitHeight
        clip: true
        ScrollBar.vertical: ScrollBar {}

        ColumnLayout {
            id: col
            width: parent.width
            spacing: unit * 0.2

            Label { text: "Settings"; color: "white"; font.pixelSize: unit * 0.5; font.bold: true }

            Label { text: "Drone address"; color: "#ccffffff" }
            TextField {
                Layout.fillWidth: true
                text: store.host
                inputMethodHints: Qt.ImhPreferNumbers
                onEditingFinished: if (text !== store.host) panel.hostEdited(text)
            }

            Switch {
                text: "Stream video over TCP (try if video won't start)"
                checked: store.useTcp
                onToggled: { store.useTcp = checked; video.useTcp = checked }
            }

            RowLayout {
                Label { text: "Control packet format"; color: "white" }
                ComboBox {
                    Layout.fillWidth: true
                    model: ["Auto (from model ID)", "8-byte", "20-byte"]
                    currentIndex: store.formatOverride === 8 ? 1 : (store.formatOverride === 20 ? 2 : 0)
                    onActivated: {
                        var v = [0, 8, 20][index]
                        store.formatOverride = v
                        drone.formatOverride = v
                    }
                }
            }
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: "#99ffffff"
                text: drone.connected
                      ? "Drone reports model ID " + drone.modelId + " (0x" + drone.modelId.toString(16) +
                        "), using " + drone.packetFormat + "-byte control frames."
                      : "Not connected to a drone."
            }

            Switch { text: "Swap sticks (throttle on the right)"; checked: store.swapSticks
                     onToggled: store.swapSticks = checked }
            Switch { text: "Crop video to fill the screen"; checked: store.fillMode
                     onToggled: store.fillMode = checked }
            Switch { text: "Show debug info"; checked: store.showDebug
                     onToggled: store.showDebug = checked }

            Label { text: "Trim"; color: "white"; font.bold: true; Layout.topMargin: unit * 0.2 }
            Repeater {
                model: [
                    { name: "Roll", prop: "trimRoll" },
                    { name: "Pitch", prop: "trimPitch" },
                    { name: "Yaw", prop: "trimYaw" }
                ]
                RowLayout {
                    Label { text: modelData.name; color: "white"; Layout.preferredWidth: unit * 2 }
                    Button { text: "−"; onClicked: drone[modelData.prop] = drone[modelData.prop] - 1 }
                    Label {
                        text: drone[modelData.prop]
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        Layout.preferredWidth: unit * 1.2
                    }
                    Button { text: "+"; onClicked: drone[modelData.prop] = drone[modelData.prop] + 1 }
                }
            }
            Button {
                text: "Reset trims"
                onClicked: { drone.trimRoll = 0; drone.trimPitch = 0; drone.trimYaw = 0 }
            }

            Button {
                Layout.alignment: Qt.AlignRight
                Layout.topMargin: unit * 0.3
                text: "Close"
                onClicked: panel.close()
            }
        }
    }
}
