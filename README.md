# KY Fly

Ubuntu Touch app for KY UFO family Wi-Fi drones (the ones controlled by the
Android app `com.cooingdv.kyufo`): live low-latency FPV video plus full flight
control. Protocol details are in [PROTOCOL.md](PROTOCOL.md); notes on adding
gamepads are in [GAMEPAD.md](GAMEPAD.md).

## Design in one paragraph

Video is MJPEG over RTP. Instead of a media framework, the app has its own
minimal RTSP client and RTP/JPEG depayloader (`src/rtspworker.*`,
`src/rtpjpeg.*`) running on a worker thread, decoding with Qt's built-in JPEG
reader and drawing with a scene-graph texture. There is no jitter buffer and no
clock sync. If frames arrive faster than they can be shown, stale ones are
skipped, so the picture is always the newest frame. Everything uses Qt Network
only, so the app needs nothing beyond the `networking` AppArmor policy group and
bundles no libraries.

## Build on a desktop (for testing)

Needs Qt 5.15 development packages and CMake. On Ubuntu-based systems:

```bash
sudo apt install cmake g++ qtbase5-dev qtdeclarative5-dev \
  qml-module-qtquick2 qml-module-qtquick-window2 qml-module-qtquick-controls2 \
  qml-module-qtquick-layouts qml-module-qt-labs-settings

cmake -S . -B build -DKYFLY_BUILD_TOOLS=ON
cmake --build build -j$(nproc)
(cd build && ctest --output-on-failure)     # needs python3 + python3-pil
```

### Try it against the real drone from your PC first

Join the drone's Wi-Fi, then:

```bash
./build/kyfly-probe --host 192.168.1.1 --seconds 15 --save frame.png
```

The probe uses exactly the app's networking and decoding code. It only sends
heartbeats, never flight commands. It prints the drone's model ID (which
picks the control format), frame rate, decode time and packet loss. Add `--tcp`
if UDP video doesn't come through.

Then the full app, windowed:

```bash
./build/kyfly --windowed
```

Keyboard controls: **Enter** controls on/off, **W/S** throttle, **A/D** yaw,
arrows pitch/roll, **T** takeoff, **L** land, **Space** emergency stop,
**F** flip, **C** camera, **H** headless, **G** gyro calibrate, **1/2/3** speed.

### Without the drone

`tools/mock_drone.py` is a fake drone: a real RTSP MJPEG server on port 7070
plus a UDP 7099 responder that decodes and validates every control frame and
prints it.

```bash
sudo apt install python3-gi gir1.2-gst-rtsp-server-1.0 \
  gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-x
python3 tools/mock_drone.py --model-id 9     # 9 = 8-byte family, 90 = 20-byte
./build/kyfly --host 127.0.0.1 --windowed     # in another terminal
```

## Build and install on Ubuntu Touch

Targets **Ubuntu Touch 24.04** (framework `ubuntu-touch-24.04-1.x`, Qt 5.15).
Uses [Clickable](https://clickable-ut.dev) **8.4 or newer** (Docker-based); run
`clickable update-images` first if you haven't recently. With the phone connected
over USB and developer mode on:

```bash
clickable build --arch arm64        # builds the .click
clickable install --arch arm64      # pushes and installs it
clickable launch --arch arm64       # starts it
clickable logs --arch arm64         # follow its output
clickable review --arch arm64       # run the OpenStore review checks locally
```

Plain `clickable --arch arm64` does build + install + launch in one go.

The framework and AppArmor policy version are filled in by Clickable at build
time (`@CLICK_FRAMEWORK@` / `@APPARMOR_POLICY@`), so nothing in the project
hard-codes them. The 24.04-2.x series runs apps built for the
`ubuntu-touch-24.04-1.x` framework; if a future Clickable adds a dedicated 2.x
framework, change the one line in `clickable.yaml`. Builds for 24.04 won't run on
20.04 devices.

## First flight checklist

1. Check video and the model ID with `kyfly-probe` from the PC.
2. **Props off** for the first armed test. With *Settings → Show debug info*
   on, the `tx` line shows each control frame as it is sent.
3. The red **STOP** button (and Space on a keyboard) sends emergency stop, and
   works even when controls are off.
4. If the app goes to the background it disarms and sends `08 01`. If the
   phone freezes, control frames simply stop and the drone's own failsafe takes
   over.

## Confinement notes (for OpenStore auto-review)

* AppArmor: `networking` and `keep-display-on`. Both are standard groups; no
  reserved groups, no `unconfined`.
* The screen is kept on by calling `keepDisplayOn` on
  `com.canonical.Unity.Screen` (system bus) while the app is in the foreground,
  which is exactly what the `keep-display-on` group permits. The request is
  released when the app is suspended, and dropped automatically if it exits.
* Settings live in `~/.config/kyfly.batcastle/` because the organization name is
  set to the click name in `main.cpp`. Change both together if you rename the app.
* No bundled libraries, no plugins scanned at runtime, no helper processes.

## Things I couldn't verify without a device

* **Screen keep-awake** uses `QtSystemInfo`'s `ScreenSaver` through a `Loader`
  (`qml/KeepAwake.qml`). If the module is missing or blocked, the app just runs
  without it. If the screen still dims while flying, that's where to look.
* **Landscape lock** is requested through `X-Lomiri-Supported-Orientations` /
  `X-Ubuntu-Supported-Orientations` in the desktop file.
* **Screen keep-on**: the debug overlay's `screen-on` field shows `on` when the
  request succeeded, or the D-Bus error name if it didn't.

## Reading the debug overlay (video)

*Settings → Show debug info* shows, per second:

* **received**: complete frames reassembled from the network.
* **shown**: frames decoded and displayed. If *received* is high but *shown*
  is low, decoding is the bottleneck (and *skipped* will be non-zero).
* **incomplete**: frames given up because a packet never arrived. With large
  frames (many packets each), a little Wi-Fi loss removes a lot of frames.
* **packets lost/s**: sequence-number gaps on the RTP stream.
* **udp buf**: the kernel receive buffer the app actually got (the kernel may
  cap the 2 MiB it asks for). Linux reports double the usable size.

If *incomplete* and *lost* are high on the front camera, try *Settings → Stream
video over TCP*: TCP retransmits lost packets, which usually restores the frame
rate at the cost of occasional short hitches. If *received* itself is low with
no loss, the drone is simply sending that few frames.

## Video orientation

Frames whose shape is portrait (taller than wide) are rotated 90° clockwise,
mirroring the official app, which does this for the bottom camera on
"orientation camera" models (model 90 included). The **ROTATE** button steps the
rotation by 90° for the current frame shape, and the choice is remembered
separately for portrait and landscape frames.

## Latency notes

* The app adds roughly one display refresh on top of decode time (~1–5 ms per
  frame). Everything else is the drone's encoder and the Wi-Fi link.
* To measure, point the drone's camera at a stopwatch on a monitor and
  photograph both screens together.
* If the phone is noticeably laggier than the PC, Wi-Fi power saving on the
  phone is a likely cause. That can't be changed from a confined app, but for
  testing over `adb shell` (developer mode): `sudo iw dev wlan0 set power_save off`.

## Layout

```
src/rtpjpeg.*        RTP/JPEG depayloader + JPEG header rebuild (RFC 2435)
src/rtspworker.*     RTSP client, RTP receive, decode (worker thread)
src/videoreceiver.*  GUI-thread facade for the worker
src/videoitem.*      Scene-graph item showing the newest frame
src/flightinput.*    Device-agnostic pilot input (touch, keyboard, gamepad later)
src/dronelink.*      UDP 7099: heartbeat, status, control frames, commands
qml/                 UI (Main, Stick, HudButton, SettingsPanel, KeyboardInput)
tools/probe.cpp      PC diagnostic using the app's code
tools/mock_drone.py  Fake drone for testing
tests/               Depayloader and control-packet tests
```

Source files carry a `GPL-3.0-or-later` SPDX header as a placeholder; change it
if you prefer a different license.
