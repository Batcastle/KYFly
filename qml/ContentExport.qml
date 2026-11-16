// SPDX-License-Identifier: GPL-3.0-or-later
// Sends a photo or video to another app (Gallery, Files, …) through Content
// Hub. Needs the "content_exchange_source" AppArmor policy group, which is a
// common (auto-approved) group. Loaded through a Loader so a missing module
// (e.g. on a desktop) doesn't break the app.
import QtQuick 2.15
import Lomiri.Content 1.3

Item {
    id: exporter
    property string pendingPath
    property bool pendingVideo: false
    signal done(string message)

    function exportFile(path, isVideo) {
        pendingPath = path
        pendingVideo = isVideo
        picker.visible = true
    }

    anchors.fill: parent
    visible: picker.visible

    Rectangle { anchors.fill: parent; color: "#202124" }

    ContentPeerPicker {
        id: picker
        anchors.fill: parent
        visible: false
        showTitle: true
        handler: ContentHandler.Destination
        contentType: exporter.pendingVideo ? ContentType.Videos : ContentType.Pictures
        onPeerSelected: {
            var transfer = peer.request()
            transfer.items = [itemTemplate.createObject(exporter, { url: "file://" + exporter.pendingPath })]
            transfer.state = ContentTransfer.Charged
            picker.visible = false
            exporter.done("Sent to " + peer.name)
        }
        onCancelPressed: picker.visible = false
    }

    Component { id: itemTemplate; ContentItem {} }
}
