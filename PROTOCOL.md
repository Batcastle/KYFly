# KY UFO drone protocol

Recovered from `com.cooingdv.kyufo` 1.8.0 (version code 60) with jadx. Class
names below refer to that APK. Everything here is implemented in
`src/dronelink.cpp` and `src/rtspworker.cpp`, and the control-packet code is
tested byte-for-byte against a port of the decompiled Java
(`tests/packet_cases.py`, 8000 cases).

The app contains two unrelated drone stacks and runs both at once:

1. **RTSP family** (this document): video over RTSP, control over UDP 7099.
2. **BL60x family**: a native-library MJPEG-over-UDP protocol (`libuav_gl.so`,
   `libuav_tc.so`, package `com.cooingdv.bl60xmjpeg`). Not implemented. If
   `rtsp://192.168.1.1:7070/webcam` works with ffplay, your drone is family 1.

The camera board's player is a custom ijkplayer/FFmpeg build from the chipset
vendor Buildwin.

## Network

The drone is a Wi-Fi access point; the phone joins it. Drone address is
`192.168.1.1`.

| Port | Proto | Use |
|---|---|---|
| 7070 | RTSP/TCP | Live video, `rtsp://192.168.1.1:7070/webcam` |
| 7099 | UDP | Heartbeat, status, flight control, commands |
| 21 | FTP | SD card (user `ftp`, password `ftp`, root `/0/`) |
| 80 | HTTP | Thumbnails: `/DCIM/<file>`, `/PHOTO/T/<file>`, originals `/PHOTO/O/<file>` |

Recorded videos on the card play over `rtsp://192.168.1.1:7070/file/DCIM/<file>`.

## Video

MJPEG over RTP (RFC 2435, payload type 26), negotiated over RTSP. Evidence: the
app's player sets a vendor option `rtp-jpeg-parse-packet-method` and an
`mjpeg-pix-fmt`, and the vendor FFmpeg build enables only the mjpeg, mpeg4 and
h264 decoders. RTP/JPEG can only carry 4:2:0 (type 1) and 4:2:2 (type 0)
chroma.

**The stream stalls unless the heartbeat (below) is running.**

### Orientation

The official app's renderer (`MjpegThread`) rotates a frame 90° clockwise when
it is 240×320, 120×160 or 160×272 (portrait-shaped; the bottom camera on
"orientation camera" models), unless the model is 94 or 99
(`WifiIdUtils.isNoRotate`). It also crops 800×600 frames to 800×540. Model IDs
90, 91, 101, 103–106 are "orientation camera" models; 100 is a three-camera model.

### Gimbal

Version 1.8.0 of the app has no gimbal/camera-tilt command or control: the
only camera-related commands are the camera switch and screen flip below. The
20-byte control frame has 10 bytes that this app always sends as zero.

## UDP 7099

All datagrams go to `192.168.1.1:7099` from one local socket; replies come back
to that socket.

### Heartbeat

Send `01 01` once per second (`UdpClient.HeartBeatTask`). The app also sends one
extra 500 ms after starting.

### Status reply

The drone answers the heartbeat with up to ~20 bytes:

| Byte | Meaning |
|---|---|
| 0 | **Model ID.** Selects the control format (below) and advertised resolution. If it is `0x66`, the datagram is instead a remote-control button event (photo/record) and the rest of this table doesn't apply. |
| 1 | Camera-switch restart mode: `1` = restart video immediately after a switch, `2` = stop video, wait 600 ms, restart. |
| 2 | Image orientation / three-camera state (`1`, `2`, `3`). On the 8-byte family, `77` (`'M'`) / `88` (`'X'`) here mean "RC photo / record button pressed", with a counter in byte 3 / byte 4. |

Model ID to control format (`WifiIdUtils.isNoGL` / `isGL`):

| Format | Model IDs |
|---|---|
| 8-byte | 5, 9, 12, 19, 20, 21, 23, 24, 31, 45, 51, 63, 64, 65, 66, 67, 72 |
| 20-byte | 85, 90–101, 103 |
| (unknown) | treated as 20-byte, like the app |

The ID also encodes the "resolution" the app advertises (e.g. 9 = 720p,
12 = 1080p, 19 = 4K, 20 = 8K). These are marketing numbers; the actual stream
size is whatever the JPEG frames say.

### Flight control frames

While the on-screen controls are enabled, the app sends one frame every 50 ms
(`FlyController.FlyControlTask`). On the UDP path every frame is prefixed with
`0x03`.

Stick bytes are `1..255`, `128` = centre. Throttle bottom is `0` (the code turns
a computed `1` into `0`).

**8-byte format** (9 bytes on the wire with the prefix):

```
03 66 RR PP TT YY FL XX 99
```

`XX = RR ^ PP ^ TT ^ YY ^ FL`

| `FL` bit | Meaning |
|---|---|
| `0x01` | Take off (held 1 s) |
| `0x02` | Land (held 1 s) |
| `0x04` | Emergency stop (held 1 s) |
| `0x08` | Flip (held 600 ms with roll or pitch at 1 or 255) |
| `0x10` | Headless mode |
| `0x20` | Fast return / unlock (never set by this app version) |
| `0x40` | Light |
| `0x80` | Gyro calibration (held 2 s) |

**20-byte format** (21 bytes on the wire with the prefix):

```
03 66 14 RR PP TT YY F1 F2 00 00 00 00 00 00 00 00 00 00 XX 99
```

`XX = F1 ^ RR ^ PP ^ TT ^ YY ^ F2`

| Field | Bit | Meaning |
|---|---|---|
| F1 | `0x01` | Take off **or** land: one toggle bit (held 1 s) |
| F1 | `0x02` | Emergency stop |
| F1 | `0x04` | Gyro calibration |
| F1 | `0x08` | Flip (bit exists, but this family's screen never uses it) |
| F1 | `0x10` | Light |
| F2 | `0x01` | Headless mode |
| F2 | `0x02` | Altitude hold (always set by the app for this family) |

### Stick mapping

From `DeviceBLFragment.onMove` / `DeviceGLFragment.onMove`, with stick
deflection normalized to −1..+1 (right/up positive):

* **Yaw** = `int(x × 127) + 128`, with a dead band: results in 104..152 become 128.
  Yaw trim adds `2 × trim` (trim −12..+12).
* **Throttle** = `int(y × 127) + 128`.
* **Roll / pitch** = `int(axis × L) + 128 + trim × M`, where by speed setting:

  | Speed | L (travel) | M (trim multiplier) |
  |---|---|---|
  | 30% (default) | 45 | 2 |
  | 60% | 60 | 3 |
  | 100% | 127 | 4 |

* Throttle at control start: `0` (stick at bottom, not sprung) without altitude
  hold, `128` (sprung to centre) with it. The 8-byte family defaults altitude hold
  **off** (user preference); the 20-byte family forces it **on**.
* Flip: arm it, then the first right-stick push past 70% on one axis picks the
  direction; roll/pitch go to `1`/`255`, the flip bit is held for 600 ms, and stick
  input is ignored meanwhile.
* The 20-byte family's screen has a single take-off/land button, toggling based
  on whether the app thinks it's flying.

### Other commands

| Bytes | Meaning |
|---|---|
| `08 01` | Controls off (sent once when the on-screen sticks are disabled) |
| `06 01` / `06 02` | Switch to camera 1 / camera 2 (then restart video per status byte 1) |
| `09 01` / `09 02` | Context dependent: screen flip normal/flipped (`SocketClient.switchScreen`), but on the 8-byte family also the acknowledgement of an RC photo / record button event. This app flips the picture locally instead of sending these. |
