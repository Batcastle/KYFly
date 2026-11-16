#!/usr/bin/env python3
"""Checks recordings and photos written by the capture code.

Uses ffprobe/ffmpeg (what most players are built on) and GStreamer (what
Ubuntu Touch's media stack uses) to confirm files decode, that an MKV cut off
mid-recording still plays, and that rotation metadata makes players show the
picture turned the right way.

usage: capture_cases.py <path-to-capture_test-binary>
needs: ffmpeg, gst-launch-1.0 (+ plugins-good), python3-pil
"""
import json, os, shutil, subprocess, sys, tempfile
from PIL import Image, ImageDraw

BIN = sys.argv[1]
results = []


def run(*args, **kw):
    return subprocess.run([str(a) for a in args], capture_output=True, text=True, **kw)


def make_frames(d, n, w, h, sub=2):
    """Frames with a red block in the top-left corner (orientation marker)."""
    os.makedirs(d, exist_ok=True)
    for i in range(n):
        img = Image.new("RGB", (w, h), (30, 60, 90))
        dr = ImageDraw.Draw(img)
        dr.rectangle((0, 0, w // 4, h // 4), fill=(230, 20, 20))
        x = int((w - 40) * i / max(1, n - 1))
        dr.ellipse((x, h // 2, x + 40, h // 2 + 40), fill=(240, 240, 60))
        img.save(os.path.join(d, f"{i:04d}.jpg"), "JPEG", quality=80, subsampling=sub)
    return d


def check(name, ok, detail=""):
    results.append(ok)
    print(f"[{'PASS' if ok else 'FAIL'}] {name}{(': ' + detail) if detail else ''}")


def probe(path):
    r = run("ffprobe", "-v", "error", "-select_streams", "v:0", "-count_frames",
            "-show_entries", "stream=codec_name,width,height,nb_read_frames:stream_side_data=rotation:format=duration,format_name",
            "-of", "json", path)
    return json.loads(r.stdout or "{}"), r.stderr


def decode_errors(path):
    r = run("ffmpeg", "-v", "error", "-i", path, "-f", "null", "-")
    return r.stderr.strip()


def gst_decodes(path, demux):
    r = run("gst-launch-1.0", "-q", "filesrc", f"location={path}", "!", demux, "!", "jpegdec", "!", "fakesink")
    return r.returncode == 0, r.stderr.strip()[-300:]


def marker_corner(png):
    """Which corner the red marker ended up in after the player's auto-rotation."""
    im = Image.open(png).convert("RGB")
    w, h = im.size
    corners = {"top-left": (w * 0.06, h * 0.06), "top-right": (w * 0.94, h * 0.06),
               "bottom-left": (w * 0.06, h * 0.94), "bottom-right": (w * 0.94, h * 0.94)}
    for k, (x, y) in corners.items():
        r, g, b = im.getpixel((int(x), int(y)))
        if r > 180 and g < 90 and b < 90:
            return k, im.size
    return None, im.size


def first_frame(path, out):
    run("ffmpeg", "-v", "error", "-y", "-i", path, "-frames:v", "1", out)
    return out


EXPECT_CORNER = {0: "top-left", 90: "top-right", 180: "bottom-right", 270: "bottom-left"}

tmp = tempfile.mkdtemp()
land = make_frames(os.path.join(tmp, "land"), 50, 640, 368)
port = make_frames(os.path.join(tmp, "port"), 30, 240, 320)

# ---- basic MKV / MP4
for fmt in ("mkv", "mp4"):
    out = os.path.join(tmp, f"basic.{fmt}")
    r = run(BIN, fmt, land, out, 0, 25)
    info, err = probe(out)
    st = (info.get("streams") or [{}])[0]
    ok = (r.returncode == 0 and st.get("codec_name") == "mjpeg" and int(st.get("nb_read_frames", 0)) == 50
          and st.get("width") == 640 and st.get("height") == 368)
    dur = float(info.get("format", {}).get("duration", 0))
    check(f"{fmt}: ffprobe sees 50 MJPEG frames 640x368", ok, f"duration {dur:.2f}s {err.strip()[:120]}")
    check(f"{fmt}: duration about 2 s", 1.9 <= dur <= 2.15, f"{dur:.3f}s")
    e = decode_errors(out)
    check(f"{fmt}: ffmpeg decodes every frame without errors", e == "", e[:200])
    ok, e = gst_decodes(out, "matroskademux" if fmt == "mkv" else "qtdemux")
    check(f"{fmt}: GStreamer ({'matroskademux' if fmt == 'mkv' else 'qtdemux'}) decodes it", ok, e)
    # Pull frame 1 back out without decoding: must be the exact JPEG we put in.
    pkt = os.path.join(tmp, f"pkt_{fmt}.jpg")
    run("ffmpeg", "-v", "error", "-y", "-i", out, "-map", "0:v", "-c:v", "copy", "-frames:v", "1", "-f", "image2", pkt)
    same = os.path.exists(pkt) and open(pkt, "rb").read() == open(os.path.join(land, "0000.jpg"), "rb").read()
    check(f"{fmt}: frame 1 is byte-identical to the drone's JPEG (no re-encoding)", same)

r = run("mkvinfo", "--all", os.path.join(tmp, "basic.mkv"))
if r.returncode == 0 or "Segment" in r.stdout:
    check("mkv: mkvinfo parses it (Cues, SeekHead, Duration present)",
          all(k in r.stdout for k in ("Cue point", "Seek head", "Duration")) and "rror" not in r.stdout, r.stderr.strip()[:150])

# ---- crash safety: process dies mid-recording, file never closed
crash = os.path.join(tmp, "crash.mkv")
run(BIN, "mkv", land, crash, 0, 25, 31)
info, err = probe(crash)
n = int((info.get("streams") or [{}])[0].get("nb_read_frames", 0))
check("mkv: app killed after 31 frames -> all 31 frames still play", n == 31, f"read {n} {err.strip()[:150]}")
ok, e = gst_decodes(crash, "matroskademux")
check("mkv: killed recording still decodes in GStreamer", ok, e)
# ...and the file also being cut mid-frame (power loss during a write)
cut = os.path.join(tmp, "cut.mkv")
data = open(os.path.join(tmp, "basic.mkv"), "rb").read()
open(cut, "wb").write(data[: int(len(data) * 0.6)])
info, _ = probe(cut)
n = int((info.get("streams") or [{}])[0].get("nb_read_frames", 0))
check("mkv: file cut at 60% mid-frame -> everything before the cut plays", n >= 28, f"read {n} of 50")

# ---- rotation metadata, checked by letting ffmpeg auto-rotate
for fmt in ("mkv", "mp4"):
    for rot in (90, 180, 270):
        out = os.path.join(tmp, f"rot{rot}.{fmt}")
        run(BIN, fmt, port, out, rot, 20)
        corner, size = marker_corner(first_frame(out, os.path.join(tmp, f"rot{rot}_{fmt}.png")))
        want_size = (320, 240) if rot in (90, 270) else (240, 320)
        check(f"{fmt}: {rot}° clockwise shows as expected in players",
              corner == EXPECT_CORNER[rot] and size == want_size, f"marker {corner}, size {size}")

# ---- recorder: camera switch mid-recording splits into two files
for fmt in ("mkv", "mp4"):
    outdir = os.path.join(tmp, f"rec_{fmt}")
    r = run(BIN, "recorder", land, port, outdir, fmt, 0, 90, timeout=60)
    files = [l[5:] for l in r.stdout.splitlines() if l.startswith("FILE ")]
    ok = r.returncode == 0 and len(files) == 2 and all(f.endswith("." + fmt) for f in files)
    left = sorted(os.listdir(outdir)) if os.path.isdir(outdir) else []
    check(f"recorder {fmt}: camera switch -> 2 files, nothing else left behind",
          ok and len(left) == 2, f"{[os.path.basename(f) for f in files]} dir={left} {r.stdout[-200:]}")
    if ok:
        a, _ = probe(files[0]); b, _ = probe(files[1])
        sa, sb = a["streams"][0], b["streams"][0]
        check(f"recorder {fmt}: file 1 = 50 landscape frames, file 2 = 30 portrait frames",
              (sa["width"], int(sa["nb_read_frames"])) == (640, 50) and (sb["width"], int(sb["nb_read_frames"])) == (240, 30),
              f"{sa['width']}x{sa['height']}/{sa['nb_read_frames']}  {sb['width']}x{sb['height']}/{sb['nb_read_frames']}")
        corner, size = marker_corner(first_frame(files[1], os.path.join(tmp, f"rec2_{fmt}.png")))
        check(f"recorder {fmt}: portrait file carries the 90° rotation", corner == "top-right", f"{corner} {size}")

# ---- photos
src = os.path.join(port, "0000.jpg")
orig = open(src, "rb").read()
for fmt in ("jpeg", "png", "webp", "raw"):
    outdir = os.path.join(tmp, "photos_" + fmt)
    r = run(BIN, "photo", src, outdir, fmt, 0, 90)
    path = r.stdout.strip()
    if fmt == "webp" and r.returncode != 0:
        check("photo webp", False, r.stdout.strip())
        continue
    ok = r.returncode == 0 and os.path.exists(path)
    if fmt == "raw":
        check("photo raw: byte-for-byte the drone's frame", ok and open(path, "rb").read() == orig, path)
        continue
    im = Image.open(path)
    if fmt == "jpeg":
        orient = im.getexif().get(274)
        same_pixels = Image.open(src).convert("RGB").tobytes() == im.convert("RGB").tobytes()
        check("photo jpeg: original image data + EXIF orientation 6 (90° CW)",
              ok and orient == 6 and same_pixels, f"orientation={orient} pixels_same={same_pixels}")
        from PIL import ImageOps
        im = ImageOps.exif_transpose(im)
        im.save(os.path.join(tmp, "jpeg_shown.png"))
        corner, size = marker_corner(os.path.join(tmp, "jpeg_shown.png"))
    else:
        im.convert("RGB").save(os.path.join(tmp, f"{fmt}_shown.png"))
        corner, size = marker_corner(os.path.join(tmp, f"{fmt}_shown.png"))
    check(f"photo {fmt}: viewers show it rotated 90° clockwise", ok and corner == "top-right" and size == (320, 240),
          f"{corner} {size} {os.path.basename(path)}")

# landscape frame with rotation 0: JPEG is untouched, no EXIF added
r = run(BIN, "photo", os.path.join(land, "0000.jpg"), os.path.join(tmp, "photos_land"), "jpeg", 0, 90)
check("photo jpeg, no rotation: file identical to the drone's frame",
      open(r.stdout.strip(), "rb").read() == open(os.path.join(land, "0000.jpg"), "rb").read())

shutil.rmtree(tmp)
print(f"\n{sum(results)}/{len(results)} capture checks passed")
sys.exit(0 if all(results) else 1)
