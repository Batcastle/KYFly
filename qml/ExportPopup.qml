// SPDX-License-Identifier: GPL-3.0-or-later
// Full-screen popup hosting the Content Hub app picker.
//
// It is its own Popup, opened on top of the media panel, so the picker is
// always attached to the window and receives input. (Parenting the picker to
// Overlay.overlay from inside the media panel left it with no parent while the
// panel was closed, so Export showed nothing.)
import QtQuick 2.15
import QtQuick.Controls 2.15

Popup {
    id: popup
    // Fires with a human-readable result: sent, or why export isn't possible.
    signal message(string text)

    readonly property bool available: loader.status === Loader.Ready

    function exportFile(path, isVideo) {
        if (!available) {
            popup.message("Exporting needs Ubuntu Touch's Content Hub, which isn't available here.")
            return
        }
        loader.item.path = path
        loader.item.isVideo = isVideo
        open()
    }

    modal: true
    focus: true
    x: 0
    y: 0
    width: parent ? parent.width : 0
    height: parent ? parent.height : 0
    padding: 0
    closePolicy: Popup.CloseOnEscape
    background: Rectangle { color: "#202124" }

    contentItem: Loader {
        id: loader
        // Lomiri.Content on current Ubuntu Touch; Ubuntu.Content on older systems.
        source: "ContentExport.qml"
        onStatusChanged: if (status === Loader.Error && source.toString().indexOf("Ubuntu") < 0)
                             source = "ContentExportUbuntu.qml"
    }

    Connections {
        target: loader.item
        ignoreUnknownSignals: true
        function onDone(text) { popup.close(); popup.message(text) }
        function onCancelled() { popup.close() }
    }
}
