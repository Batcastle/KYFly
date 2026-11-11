// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Window 2.15
import Qt.labs.settings 1.0
import KyFly 1.0

Window {
    id: root
    visible: true
    width: 1280
    height: 720
    visibility: cliWindowed ? Window.Windowed : Window.FullScreen
    color: "black"
    title: "KY Fly"

    readonly property real unit: Math.max(32, Math.min(width, height) / 9)
    readonly property bool eightByte: drone.packetFormat === 8
    readonly property var throttleStick: appSettings.swapSticks ? rightStick : leftStick
    readonly property var cyclicStick: appSettings.swapSticks ? leftStick : rightStick
    readonly property bool framePortrait: videoView.sourceHeight > videoView.sourceWidth
    readonly property int videoRotation: framePortrait ? appSettings.portraitRotation
                                                       : appSettings.landscapeRotation
    function rotateVideo() {
        if (framePortrait) appSettings.portraitRotation = (appSettings.portraitRotation + 90) % 360
        else appSettings.landscapeRotation = (appSettings.landscapeRotation + 90) % 360
    }

    // "No drone" help appears only after a grace period, so it doesn't flash at startup.
    readonly property bool videoPlaying: video.state === VideoReceiver.Playing && videoView.hasFrame
    readonly property bool noDroneCondition: !drone.connected && !videoPlaying
    property bool noDroneTimeout: false
    onNoDroneConditionChanged: if (!noDroneCondition) noDroneTimeout = false
    Timer {
        interval: 3000
        running: root.noDroneCondition && !root.noDroneTimeout
        onTriggered: root.noDroneTimeout = true
    }

    Settings {
        id: appSettings
        property string host: "192.168.1.1"
        property bool useTcp: false
        property int formatOverride: 0
        property bool swapSticks: false
        property bool showDebug: false
        // Rotation per frame shape. Portrait-shaped frames (the bottom camera on
        // orientation-camera models) default to 90° clockwise, like the official app.
        property int landscapeRotation: 0
        property int portraitRotation: 90
        property bool fillMode: false
        property int speedLevel: 0
        property bool altitudeHold: false
        property int trimRoll: 0
        property int trimPitch: 0
        property int trimYaw: 0
    }

    function applyHost(h) {
        drone.host = h
        video.url = "rtsp://" + h + ":7070/webcam"
    }

    Component.onCompleted: {
        applyHost(cliHost !== "" ? cliHost : appSettings.host)
        video.useTcp = appSettings.useTcp
        drone.formatOverride = appSettings.formatOverride
        drone.speedLevel = appSettings.speedLevel
        drone.altitudeHold = appSettings.altitudeHold
        drone.trimRoll = appSettings.trimRoll
        drone.trimPitch = appSettings.trimPitch
        drone.trimYaw = appSettings.trimYaw
        drone.active = true
        video.start()
        if (cliArm) drone.armed = true
    }

    // Persist changes no matter which input device made them.
    Connections {
        target: drone
        function onSpeedLevelChanged() { appSettings.speedLevel = drone.speedLevel }
        function onAltitudeHoldChanged() {
            if (!drone.altitudeHoldLocked) appSettings.altitudeHold = drone.altitudeHold
        }
        function onTrimChanged() {
            appSettings.trimRoll = drone.trimRoll
            appSettings.trimPitch = drone.trimPitch
            appSettings.trimYaw = drone.trimYaw
        }
        function onRotateVideoRequested() { root.rotateVideo() }
        function onArmedChanged() { if (!drone.armed) controls.releaseSource(FlightInput.Touch) }
    }

    // Sticks display the commanded values whenever they aren't being touched,
    // so they also show keyboard / gamepad input.
    Connections {
        target: controls
        function onAxesChanged() {
            if (!throttleStick.pressed) { throttleStick.xValue = controls.yaw; throttleStick.yValue = controls.throttle }
            if (!cyclicStick.pressed) { cyclicStick.xValue = controls.roll; cyclicStick.yValue = controls.pitch }
        }
    }

    function pushTouch() {
        controls.setAxes(FlightInput.Touch, throttleStick.xValue, throttleStick.yValue,
                         cyclicStick.xValue, cyclicStick.yValue)
    }

    // ------------------------------------------------------------------ video
    VideoItem {
        id: videoView
        // Rotated as a whole; for 90°/270° the item is sized with width and
        // height swapped so aspect-fit still uses the full screen.
        readonly property bool sideways: root.videoRotation % 180 !== 0
        anchors.centerIn: parent
        width: sideways ? parent.height : parent.width
        height: sideways ? parent.width : parent.height
        rotation: root.videoRotation
        receiver: video
        fillMode: appSettings.fillMode
    }

    Column {
        anchors.centerIn: parent
        spacing: unit * 0.2
        visible: !root.videoPlaying && !noDroneCard.visible
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: video.status !== "" ? video.status : "Starting…"
            color: "white"
            font.pixelSize: unit * 0.45
            style: Text.Outline; styleColor: "black"
        }
    }

    // ------------------------------------------------------------------ no drone help
    Rectangle {
        id: noDroneCard
        z: 10   // above the debug overlay and buttons
        visible: root.noDroneTimeout
        anchors.centerIn: parent
        width: Math.min(parent.width * 0.62, unit * 11)
        height: helpCol.implicitHeight + unit * 0.8
        radius: unit * 0.25
        color: "#e6202124"
        border.color: "#66ffffff"
        Column {
            id: helpCol
            anchors { left: parent.left; right: parent.right; top: parent.top; margins: unit * 0.4 }
            spacing: unit * 0.18
            Text {
                text: "No drone found"
                color: "white"; font.pixelSize: unit * 0.5; font.bold: true
            }
            Text {
                width: parent.width
                wrapMode: Text.WordWrap
                color: "#e0ffffff"; font.pixelSize: unit * 0.3
                textFormat: Text.StyledText
                text: "1. Turn on the drone and wait for its lights to start flashing.<br>" +
                      "2. Connect this phone to the drone's Wi-Fi network, following the " +
                      "instructions that came with your drone.<br>" +
                      "3. Come back to this app. It connects automatically."
            }
            Text {
                width: parent.width
                wrapMode: Text.WordWrap
                color: "#99ffffff"; font.pixelSize: unit * 0.24
                text: "Looking for the drone at " + drone.host +
                      ". If your drone uses a different address, change it in Settings."
            }
        }
    }

    KeyboardInput { id: keyboard; anchors.fill: parent }

    // ------------------------------------------------------------------ sticks
    Stick {
        id: leftStick
        visible: drone.armed
        anchors { left: parent.left; bottom: parent.bottom }
        width: parent.width * 0.38
        height: parent.height * 0.7
        baseSize: unit * 3.4
        baseX: unit * 2.4
        baseY: height - unit * 2.3
        springX: true
        springY: appSettings.swapSticks ? true : drone.altitudeHold
        labelY: appSettings.swapSticks ? "pitch" : "throttle"
        labelX: appSettings.swapSticks ? "roll" : "yaw"
        onMoved: root.pushTouch()
    }
    Stick {
        id: rightStick
        visible: drone.armed
        anchors { right: parent.right; bottom: parent.bottom }
        width: parent.width * 0.38
        height: parent.height * 0.7
        baseSize: unit * 3.4
        baseX: width - unit * 2.4
        baseY: height - unit * 2.3
        springX: true
        springY: appSettings.swapSticks ? drone.altitudeHold : true
        labelY: appSettings.swapSticks ? "throttle" : "pitch"
        labelX: appSettings.swapSticks ? "yaw" : "roll"
        onMoved: root.pushTouch()
    }

    // ------------------------------------------------------------------ status (top left)
    Rectangle {
        anchors { fill: statusCol; margins: -unit * 0.12 }
        color: "#66000000"
        radius: 6
    }
    Column {
        id: statusCol
        anchors { left: parent.left; top: parent.top; margins: unit * 0.25 }
        spacing: unit * 0.1
        Row {
            spacing: unit * 0.15
            Rectangle {
                width: unit * 0.28; height: width; radius: width / 2
                anchors.verticalCenter: parent.verticalCenter
                color: drone.connected ? "#66bb6a" : "#ef5350"
            }
            Text {
                text: drone.connected
                      ? "Drone · model " + drone.modelId + " · " + drone.packetFormat + "-byte"
                      : "No drone"
                color: "white"; font.pixelSize: unit * 0.3
                style: Text.Outline; styleColor: "black"
            }
        }
        Text {
            text: video.state === VideoReceiver.Playing
                  ? "Video " + video.fps.toFixed(0) + " fps · " + videoView.sourceWidth + "×" + videoView.sourceHeight
                  : "Video: " + video.status
            color: "white"; font.pixelSize: unit * 0.3
            style: Text.Outline; styleColor: "black"
        }
        Text {
            visible: drone.armed
            text: "Speed " + drone.speedPercent + "%" + (drone.altitudeHold ? " · alt hold" : "")
                  + (drone.headless ? " · headless" : "")
            color: "#ffe082"; font.pixelSize: unit * 0.3
            style: Text.Outline; styleColor: "black"
        }
    }

    // ------------------------------------------------------------------ right column
    Column {
        anchors { right: parent.right; top: parent.top; margins: unit * 0.25 }
        spacing: unit * 0.15
        HudButton {
            anchors.right: parent.right
            unit: root.unit * 0.8
            text: drone.armed ? "CONTROLS ON" : "CONTROLS OFF"
            checked: drone.armed
            accent: "#66bb6a"
            onClicked: controls.trigger(FlightInput.ToggleArmed)
        }
        HudButton { anchors.right: parent.right; unit: root.unit * 0.8; text: "CAMERA"
                    onClicked: controls.trigger(FlightInput.SwitchCamera) }
        HudButton { anchors.right: parent.right; unit: root.unit * 0.8; text: "ROTATE"
                    sub: root.videoRotation + "°"
                    onClicked: root.rotateVideo() }
        HudButton { anchors.right: parent.right; unit: root.unit * 0.8; text: "SETTINGS"
                    onClicked: settingsPanel.open() }
    }

    // ------------------------------------------------------------------ emergency stop
    HudButton {
        anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: unit * 0.25 }
        unit: root.unit * 1.2
        visible: drone.armed || drone.connected
        danger: true
        text: "STOP"
        sub: "motors off"
        onClicked: controls.trigger(FlightInput.EmergencyStop)
    }

    Text {
        anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: unit * 1.9 }
        visible: drone.armed && !drone.connected
        text: "⚠ No reply from drone"
        color: "#ff8a80"; font.pixelSize: unit * 0.35; font.bold: true
        style: Text.Outline; styleColor: "black"
    }
    Text {
        anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: unit * 1.9 }
        visible: drone.flipArmed
        text: "FLIP ARMED: push the right stick in a direction"
        color: "#ffe082"; font.pixelSize: unit * 0.35; font.bold: true
        style: Text.Outline; styleColor: "black"
    }

    // ------------------------------------------------------------------ flight buttons
    // Kept inside the gap between the two stick bases so they never overlap.
    readonly property real stickExtent: unit * 4.2
    Item {
        id: centerGap
        visible: drone.armed
        anchors { bottom: parent.bottom; bottomMargin: unit * 0.3; horizontalCenter: parent.horizontalCenter }
        width: Math.max(unit * 5, root.width - 2 * root.stickExtent)
        height: primaryRow.height + secondary.height + unit * 0.2

        Flow {
            id: secondary
            anchors { bottom: primaryRow.top; bottomMargin: unit * 0.2; horizontalCenter: parent.horizontalCenter }
            width: Math.min(parent.width, implicitRowWidth)
            spacing: unit * 0.12
            property real implicitRowWidth: {
                var w = 0
                for (var i = 0; i < children.length; ++i)
                    if (children[i].visible) w += children[i].width + spacing
                return w
            }
            HudButton { unit: root.unit * 0.7; visible: drone.flipSupported; text: "FLIP"; checked: drone.flipArmed
                        accent: "#ffca28"; onClicked: controls.trigger(FlightInput.ArmFlip) }
            HudButton { unit: root.unit * 0.7; text: "HEADLESS"; checked: drone.headless
                        onClicked: controls.trigger(FlightInput.ToggleHeadless) }
            HudButton { unit: root.unit * 0.7; text: "GYRO"; sub: "calibrate"
                        onClicked: controls.trigger(FlightInput.CalibrateGyro) }
            HudButton { unit: root.unit * 0.7; text: drone.speedPercent + "%"; sub: "speed"
                        onClicked: drone.speedLevel = (drone.speedLevel + 1) % 3 }
            HudButton { unit: root.unit * 0.7; visible: !drone.altitudeHoldLocked; text: "ALT HOLD"
                        checked: drone.altitudeHold; onClicked: controls.trigger(FlightInput.ToggleAltitudeHold) }
        }

        Row {
            id: primaryRow
            anchors { bottom: parent.bottom; horizontalCenter: parent.horizontalCenter }
            spacing: unit * 0.2
            HudButton { unit: root.unit * 0.9; visible: eightByte; text: "TAKE OFF"; accent: "#66bb6a"
                        onClicked: controls.trigger(FlightInput.TakeOff) }
            HudButton { unit: root.unit * 0.9; visible: eightByte; text: "LAND"; accent: "#ffa726"
                        onClicked: controls.trigger(FlightInput.Land) }
            HudButton { unit: root.unit * 0.9; visible: !eightByte; text: drone.flying ? "LAND" : "TAKE OFF"
                        sub: "toggle"; onClicked: controls.trigger(FlightInput.TakeOffOrLand) }
        }
    }

    // ------------------------------------------------------------------ debug overlay
    Rectangle {
        visible: appSettings.showDebug
        anchors { left: parent.left; top: statusCol.bottom; topMargin: unit * 0.3; leftMargin: unit * 0.25 }
        width: dbg.implicitWidth + unit * 0.3
        height: dbg.implicitHeight + unit * 0.2
        color: "#99000000"
        radius: 6
        Text {
            id: dbg
            anchors.centerIn: parent
            color: "#b2ff59"
            font.family: "monospace"
            font.pixelSize: unit * 0.24
            text: "tx " + drone.lastPacketHex + "\n" +
                  "rx " + drone.lastReplyHex + "\n" +
                  "frames/s  received " + video.rxFps.toFixed(1) + "  shown " + video.fps.toFixed(1) +
                  "  incomplete " + video.droppedPerSec.toFixed(1) + "  skipped " + video.skippedPerSec.toFixed(1) + "\n" +
                  "packets lost/s " + video.lostPerSec.toFixed(1) + "  decode " + video.decodeMs.toFixed(1) + " ms" +
                  "  udp buf " + (video.rcvBuf > 0 ? Math.round(video.rcvBuf / 1024) + " KiB" : (video.useTcp ? "tcp" : "?")) + "\n" +
                  "screen-on " + screenKeeper.status + "  rotation " + root.videoRotation + "°\n" +
                  "input " + ["none", "touch", "keyboard", "gamepad", "scripted"][controls.activeSource] +
                  "  y " + controls.yaw.toFixed(2) + " t " + controls.throttle.toFixed(2) +
                  " r " + controls.roll.toFixed(2) + " p " + controls.pitch.toFixed(2) + "\n" +
                  "last " + keyboard.lastKey
        }
    }

    SettingsPanel {
        id: settingsPanel
        unit: root.unit
        store: appSettings
        onHostEdited: { appSettings.host = h; root.applyHost(h) }
        onClosed: keyboard.forceActiveFocus()
    }
}
