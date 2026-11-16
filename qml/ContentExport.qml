// SPDX-License-Identifier: GPL-3.0-or-later
// Content Hub app picker for sending a photo or video to another app
// (Gallery, Files, …). Needs the "content_exchange_source" AppArmor policy
// group, which is a common (auto-approved) group.
//
// Shown inside ExportPopup, which loads it through a Loader so a missing
// module (e.g. on a desktop) can't break the app.
import QtQuick 2.15
import Lomiri.Content 1.3

Item {
    id: exporter
    property string path
    property bool isVideo: false
    // Kept alive until the receiving app has collected the file.
    property var activeTransfer: null

    signal done(string message)
    signal cancelled()

    Rectangle { anchors.fill: parent; color: "#202124" }

    ContentPeerPicker {
        id: picker
        anchors.fill: parent
        showTitle: true
        handler: ContentHandler.Destination
        contentType: exporter.isVideo ? ContentType.Videos : ContentType.Pictures
        onPeerSelected: {
            exporter.activeTransfer = peer.request()
            exporter.activeTransfer.items = [itemTemplate.createObject(exporter, { url: "file://" + exporter.path })]
            exporter.activeTransfer.state = ContentTransfer.Charged
            exporter.done("Sent to " + peer.name)
        }
        onCancelPressed: exporter.cancelled()
    }

    Component { id: itemTemplate; ContentItem {} }
}
