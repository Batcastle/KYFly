#!/usr/bin/env python3
"""Fake KY UFO drone for testing without hardware.

* RTSP MJPEG stream at rtsp://<bind>:7070/webcam (RTP/JPEG, like the real drone)
* UDP 7099: answers heartbeats with [model_id, camera_reset_mode, 1] and
  decodes/validates every control frame it receives.

Needs: python3-gi, gir1.2-gst-rtsp-server-1.0, gstreamer1.0-plugins-good
usage: mock_drone.py [--bind 127.0.0.1] [--model-id 9] [--size 640x360] [--fps 25]
"""
import argparse, socket, threading, time
import gi
gi.require_version("Gst", "1.0")
gi.require_version("GstRtspServer", "1.0")
from gi.repository import Gst, GstRtspServer, GLib

p = argparse.ArgumentParser()
p.add_argument("--bind", default="127.0.0.1")
p.add_argument("--model-id", type=int, default=9, help="9 = 8-byte family, 90 = 20-byte family")
p.add_argument("--size", default="640x360")
p.add_argument("--fps", type=int, default=25)
p.add_argument("--camera-reset", type=int, default=1)
args = p.parse_args()
W, H = map(int, args.size.split("x"))

def log(*a):
    print(time.strftime("%H:%M:%S"), *a, flush=True)

def decode_control(d):
    if len(d) == 9 and d[1] == 0x66 and d[8] == 0x99:
        r, pt, t, y, fl, x = d[2:8]
        ok = (r ^ pt ^ t ^ y ^ fl) == x
        names = [n for b, n in [(1, "TAKEOFF"), (2, "LAND"), (4, "STOP"), (8, "FLIP"), (16, "headless"),
                                (32, "return"), (64, "light"), (128, "GYRO")] if fl & b]
        return "8-byte", ok, (r, pt, t, y), names
    if len(d) == 21 and d[1] == 0x66 and d[2] == 0x14 and d[20] == 0x99:
        r, pt, t, y, f1, f2 = d[3:9]
        ok = (f1 ^ r ^ pt ^ t ^ y ^ f2) == d[19] and not any(d[9:19])
        names = [n for b, n in [(1, "TAKEOFF/LAND"), (2, "STOP"), (4, "GYRO"), (8, "FLIP"), (16, "light")] if f1 & b]
        names += [n for b, n in [(1, "headless"), (2, "alt-hold")] if f2 & b]
        return "20-byte", ok, (r, pt, t, y), names
    return None, False, None, []

def udp_loop():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind((args.bind, 7099))
    log(f"UDP control on {args.bind}:7099, model ID {args.model_id}")
    last, count, t0 = None, 0, time.time()
    while True:
        d, addr = s.recvfrom(256)
        if d == b"\x01\x01":
            s.sendto(bytes([args.model_id, args.camera_reset, 1]), addr)
        elif d[:1] == b"\x03":
            fmt, ok, sticks, flags = decode_control(d)
            if count == 0:
                t0 = time.time()  # measure rate from the first frame
            count += 1
            state = (fmt, ok, sticks, tuple(flags))
            if state != last:
                log(f"control {fmt or '??'} {'OK ' if ok else 'BAD'} roll={sticks[0] if sticks else '-'} "
                    f"pitch={sticks[1] if sticks else '-'} thr={sticks[2] if sticks else '-'} "
                    f"yaw={sticks[3] if sticks else '-'} {' '.join(flags)}  [{d.hex(' ')}]")
                last = state
            if time.time() - t0 >= 5:
                log(f"control rate {(count - 1) / (time.time() - t0):.1f} frames/s")
                count = 0
        elif d == b"\x08\x01":
            log("controls OFF (08 01)"); last = None; count = 0
        elif d[:1] == b"\x06":
            log(f"camera switch -> {d[1]}")
        else:
            log("unknown packet", d.hex(" "))

threading.Thread(target=udp_loop, daemon=True).start()

Gst.init(None)
server = GstRtspServer.RTSPServer()
server.set_address(args.bind)
server.set_service("7070")
factory = GstRtspServer.RTSPMediaFactory()
# A running clock in the picture makes latency easy to eyeball; it needs the
# pango plugin (gstreamer1.0-x on Debian/Ubuntu), so skip it if missing.
overlay = "! timeoverlay font-desc=\"Sans 28\" " if Gst.ElementFactory.find("timeoverlay") else ""
if not overlay:
    log("timeoverlay not available (install gstreamer1.0-x); streaming without clock overlay")
factory.set_launch(
    f"( videotestsrc is-live=true pattern=ball ! video/x-raw,width={W},height={H},framerate={args.fps}/1 "
    f"{overlay}! videoconvert ! video/x-raw,format=I420 ! jpegenc quality=70 ! rtpjpegpay name=pay0 pt=26 )")
factory.set_shared(True)
server.get_mount_points().add_factory("/webcam", factory)
server.attach(None)
log(f"RTSP on rtsp://{args.bind}:7070/webcam ({W}x{H}@{args.fps} MJPEG)")
GLib.MainLoop().run()
