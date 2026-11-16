// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Popup {
    id: panel
    property real unit: 40
    property var store
    signal hostEdited(string h)
    signal openMedia()

    readonly property var photoLabels: ({ jpeg: "JPEG", png: "PNG", webp: "WEBP", raw: "RAW" })
    readonly property var photoNotes: ({
        jpeg: "The drone's own JPEG data, unchanged, with a tag so viewers show it rotated like the screen.",
        png: "Lossless picture of what's on screen. Larger files.",
        webp: "Smaller files than PNG at near-identical quality.",
        raw: "The frame exactly as the drone sent it: no rotation, no changes at all."
    })

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
        contentHeight: col.height
        contentWidth: width
        clip: true
        ScrollBar.vertical: ScrollBar {}

        ColumnLayout {
            id: col
            width: panel.availableWidth
            spacing: unit * 0.2

            RowLayout {
                Layout.fillWidth: true
                Label {
                    text: "Settings"; color: "white"; font.pixelSize: unit * 0.5; font.bold: true
                    Layout.fillWidth: true
                }
                Button { text: "Photos && videos"; onClicked: panel.openMedia() }
            }

            Label { text: "Photos"; color: "white"; font.bold: true }
            RowLayout {
                Layout.fillWidth: true
                Label { text: "Format"; color: "white"; Layout.preferredWidth: unit * 2.4 }
                ComboBox {
                    id: photoBox
                    Layout.fillWidth: true
                    model: capture.availablePhotoFormats.map(function (f) { return panel.photoLabels[f] })
                    currentIndex: Math.max(0, capture.availablePhotoFormats.indexOf(store.photoFormat))
                    onActivated: store.photoFormat = capture.availablePhotoFormats[index]
                }
            }
            Label {
                Layout.fillWidth: true; wrapMode: Text.WordWrap; color: "#99ffffff"
                text: panel.photoNotes[capture.availablePhotoFormats[photoBox.currentIndex]] +
                      (capture.availablePhotoFormats.indexOf("webp") < 0 ? " (WEBP isn't available on this system.)" : "")
            }

            Label { text: "Videos"; color: "white"; font.bold: true; Layout.topMargin: unit * 0.15 }
            RowLayout {
                Layout.fillWidth: true
                Label { text: "Format"; color: "white"; Layout.preferredWidth: unit * 2.4 }
                ComboBox {
                    Layout.fillWidth: true
                    model: ["MKV (recommended)", "MP4"]
                    currentIndex: store.videoFormat === "mp4" ? 1 : 0
                    onActivated: store.videoFormat = index === 1 ? "mp4" : "mkv"
                }
            }
            Label {
                Layout.fillWidth: true; wrapMode: Text.WordWrap; color: "#99ffffff"
                text: (store.videoFormat === "mp4"
                       ? "Recorded as MKV and converted to MP4 when you stop, so a crash still leaves a playable MKV. Plays in VLC, mpv and most players."
                       : "Playable right up to the last frame even if the app, phone or video feed dies mid-recording.") +
                      " Video is stored exactly as the drone sends it (no re-compression)." +
                      (capture.recording ? " A change applies to the next recording." : "")
            }

            Label { text: "Drone address"; color: "#ccffffff" }
            TextField {
                Layout.fillWidth: true
                text: store.host
                inputMethodHints: Qt.ImhPreferNumbers
                onEditingFinished: if (text !== store.host) panel.hostEdited(text)
            }

            Switch {
                text: "Prefer TCP for video (falls back to UDP if the drone doesn't support it)"
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
