// SPDX-License-Identifier: GPL-3.0-or-later
// Lists captured photos and videos (newest first) with Export and Delete.
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Popup {
    id: panel
    property real unit: 40
    property var files: []
    property string confirmDelete: ""
    signal message(string text)

    function reload() { files = capture.mediaFiles(); confirmDelete = "" }
    function sizeText(b) {
        return b > 1048576 ? (b / 1048576).toFixed(1) + " MB" : Math.max(1, Math.round(b / 1024)) + " KB"
    }
    function exportFile(path, isVideo) {
        if (exportLoader.item) {
            exportLoader.item.exportFile(path, isVideo)
        } else {
            panel.message("Exporting needs Ubuntu Touch's Content Hub. Files are in " +
                          (isVideo ? capture.videoDir : capture.photoDir))
        }
    }

    onOpened: reload()
    Connections { target: capture; function onMediaChanged() { if (panel.opened) panel.reload() } }

    modal: true
    focus: true
    anchors.centerIn: Overlay.overlay
    width: Math.min(parent ? parent.width * 0.92 : 800, unit * 15)
    height: parent ? parent.height * 0.92 : 600
    padding: unit * 0.3
    background: Rectangle { color: "#f2202124"; radius: 10; border.color: "#44ffffff" }

    palette.window: "#202124"
    palette.windowText: "white"
    palette.text: "white"
    palette.base: "#303134"
    palette.button: "#3c4043"
    palette.buttonText: "white"
    palette.highlight: "#4fc3f7"

    ColumnLayout {
        anchors.fill: parent
        spacing: unit * 0.2

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Photos & videos"
                color: "white"; font.pixelSize: unit * 0.5; font.bold: true
                Layout.fillWidth: true
            }
            Button { text: "Close"; onClicked: panel.close() }
        }
        Label {
            visible: panel.files.length === 0
            text: "Nothing captured yet. Use the photo and record buttons on the left of the flight screen."
            color: "#ccffffff"; wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: unit * 0.12
            model: panel.files
            ScrollBar.vertical: ScrollBar {}

            delegate: Rectangle {
                width: list.width
                height: unit * 1.5
                radius: 6
                color: "#2c2d30"

                RowLayout {
                    anchors { fill: parent; margins: unit * 0.15 }
                    spacing: unit * 0.2

                    Rectangle {
                        Layout.preferredWidth: unit * 1.7
                        Layout.fillHeight: true
                        color: "#111"
                        radius: 4
                        clip: true
                        Image {
                            anchors.fill: parent
                            visible: !modelData.video
                            source: modelData.video ? "" : "file://" + modelData.path
                            sourceSize.width: 160
                            fillMode: Image.PreserveAspectCrop
                            autoTransform: true
                            asynchronous: true
                        }
                        Text {
                            anchors.centerIn: parent
                            visible: modelData.video
                            text: "▶"
                            color: "white"; font.pixelSize: unit * 0.5
                        }
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Label {
                            text: modelData.name
                            color: "white"; elide: Text.ElideMiddle
                            Layout.fillWidth: true
                        }
                        Label {
                            text: Qt.formatDateTime(modelData.modified, "yyyy-MM-dd hh:mm:ss") +
                                  "   " + panel.sizeText(modelData.size)
                            color: "#99ffffff"; font.pixelSize: unit * 0.24
                        }
                    }
                    Button {
                        text: "Export"
                        onClicked: panel.exportFile(modelData.path, modelData.video)
                    }
                    Button {
                        text: panel.confirmDelete === modelData.path ? "Really delete?" : "Delete"
                        onClicked: {
                            if (panel.confirmDelete === modelData.path) {
                                if (!capture.deleteFile(modelData.path))
                                    panel.message("Could not delete " + modelData.name)
                            } else {
                                panel.confirmDelete = modelData.path
                            }
                        }
                    }
                }
            }
        }
        Label {
            text: "Saved in the app's own folder. Export sends a copy to another app, such as Gallery."
            color: "#88ffffff"; font.pixelSize: unit * 0.22
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }

    // Content Hub picker, if this system has it (Lomiri.Content, or the older
    // Ubuntu.Content name). Covers the whole window while choosing an app.
    Loader {
        id: exportLoader
        parent: Overlay.overlay
        anchors.fill: parent
        z: 100
        source: "ContentExport.qml"
        onStatusChanged: if (status === Loader.Error && source.toString().indexOf("Ubuntu") < 0)
                             source = "ContentExportUbuntu.qml"
        Connections {
            target: exportLoader.item
            ignoreUnknownSignals: true
            function onDone(text) { panel.message(text) }
        }
    }
}
