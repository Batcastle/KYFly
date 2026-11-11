#!/usr/bin/env python3
"""Build RTP/JPEG (RFC 2435) packet captures from real libjpeg output, run them
through the C++ depayloader, and check the rebuilt frames decode pixel-for-pixel
identical to the originals.

usage: depay_cases.py <path-to-depay_test-binary>
"""
import io, os, shutil, struct, subprocess, sys, tempfile
from PIL import Image, ImageDraw

BIN = sys.argv[1]
MTU_PAYLOAD = 1400


def test_image(w, h, seed):
    img = Image.new("RGB", (w, h))
    px = img.load()
    for y in range(h):
        for x in range(w):
            px[x, y] = ((x * 255 // w + seed * 40) % 256, (y * 255 // h) % 256, ((x + y + seed * 17) * 3) % 256)
    d = ImageDraw.Draw(img)
    d.ellipse((w // 4 + seed * 10, h // 4, w // 2 + seed * 10, h // 2), fill=(250, 250, 40))
    d.rectangle((w // 2, h // 2, w // 2 + 60, h // 2 + 40), fill=(10, 200, 90))
    return img


def encode(img, quality, subsampling, **kw):
    b = io.BytesIO()
    img.save(b, "JPEG", quality=quality, subsampling=subsampling, optimize=False, **kw)
    return b.getvalue()


def split_jpeg(data):
    """Return (qtables_bytes, restart_interval, scan_bytes) from a baseline JPEG."""
    i, qt, dri = 2, {}, 0
    while True:
        assert data[i] == 0xFF
        m = data[i + 1]
        L = struct.unpack(">H", data[i + 2:i + 4])[0]
        seg = data[i + 4:i + 2 + L]
        if m == 0xDB:
            j = 0
            while j < len(seg):
                pq, tq = seg[j] >> 4, seg[j] & 15
                assert pq == 0
                qt[tq] = seg[j + 1:j + 65]
                j += 65
        elif m == 0xDD:
            dri = struct.unpack(">H", seg[:2])[0]
        elif m == 0xDA:
            scan = data[i + 2 + L:]
            return qt[0] + qt[1], dri, scan
        i += 2 + L


def packetize(scan, rtp_type, q, w, h, ts, seq, qtables=None, dri=0, qlen_override=None):
    pkts, off = [], 0
    while off < len(scan) or off == 0:
        chunk = scan[off:off + MTU_PAYLOAD]
        marker = off + len(chunk) >= len(scan)
        jh = struct.pack(">B3sBBBB", 0, off.to_bytes(3, "big"), rtp_type, q, w // 8, h // 8)
        if rtp_type >= 64:
            jh += struct.pack(">HH", dri, 0xFFFF)  # F=1 L=1 count=0x3FFF
        if off == 0 and q >= 128:
            tables = qtables if qtables is not None else b""
            qlen = len(tables) if qlen_override is None else qlen_override
            jh += struct.pack(">BBH", 0, 0, qlen) + (tables if qlen else b"")
        rtp = struct.pack(">BBHII", 0x80, (0x80 if marker else 0) | 26, seq & 0xFFFF, ts, 0x1234ABCD)
        pkts.append(rtp + jh + chunk)
        seq += 1
        off += len(chunk)
        if marker:
            break
    return pkts, seq


def run_case(name, frames, expect_frames, drop=None, fallback=None, transform=None):
    """frames: list of (original_jpeg_bytes, packets)"""
    tmp = tempfile.mkdtemp()
    allp = []
    for fi, (_, pk) in enumerate(frames):
        for pi, p in enumerate(pk):
            if drop and (fi, pi) in drop:
                continue
            allp.append(p)
    if transform:
        allp = transform(allp, frames)
    with open(os.path.join(tmp, "p.bin"), "wb") as f:
        for p in allp:
            f.write(struct.pack("<I", len(p)) + p)
    args = [BIN, os.path.join(tmp, "p.bin"), tmp] + ([str(fallback[0]), str(fallback[1])] if fallback else [])
    res = subprocess.run(args, capture_output=True, text=True)
    outs = sorted(x for x in os.listdir(tmp) if x.startswith("frame_"))
    ok = len(outs) == len(expect_frames)
    detail = ""
    for k, fname in enumerate(outs):
        if k >= len(expect_frames):
            break
        orig = Image.open(io.BytesIO(frames[expect_frames[k]][0])).convert("RGB")
        got = Image.open(os.path.join(tmp, fname))
        got.load()
        if got.size != orig.size or got.convert("RGB").tobytes() != orig.tobytes():
            ok = False
            detail += f" frame{k}:pixel-mismatch(size {got.size} vs {orig.size})"
    print(f"[{'PASS' if ok else 'FAIL'}] {name}: {res.stdout.strip()} outputs={len(outs)}{detail}")
    shutil.rmtree(tmp)
    return ok


def main():
    results = []
    W, H = 640, 360

    def frames_for(n, quality, sub, rtp_type, q_field, mode="computed", **kw):
        out, seq = [], 1000
        for k in range(n):
            j = encode(test_image(W, H, k), quality, sub, **kw)
            qt, dri, scan = split_jpeg(j)
            if mode == "computed":
                pk, seq = packetize(scan, rtp_type, q_field, W, H, 90000 * k // 25, seq, dri=dri)
            elif mode == "inband":
                pk, seq = packetize(scan, rtp_type, q_field, W, H, 90000 * k // 25, seq, qtables=qt, dri=dri)
            elif mode == "static-once":
                pk, seq = packetize(scan, rtp_type, q_field, W, H, 90000 * k // 25, seq,
                                    qtables=qt, dri=dri, qlen_override=None if k == 0 else 0)
            out.append((j, pk))
        return out

    results.append(run_case("4:2:0 type1 Q=50 computed tables", frames_for(3, 50, 2, 1, 50), [0, 1, 2]))
    results.append(run_case("4:2:2 type0 Q=80 computed tables", frames_for(3, 80, 1, 0, 80), [0, 1, 2]))
    results.append(run_case("Q=20 low quality", frames_for(2, 20, 2, 1, 20), [0, 1]))
    results.append(run_case("Q=99 high quality", frames_for(2, 99, 2, 1, 99), [0, 1]))
    results.append(run_case("Q=255 in-band tables (gst/ffmpeg style)", frames_for(3, 70, 2, 1, 255, "inband"), [0, 1, 2]))
    results.append(run_case("Q=200 static tables sent once (len=0 reuse)", frames_for(3, 65, 2, 1, 200, "static-once"), [0, 1, 2]))

    f = frames_for(3, 60, 2, 1, 60)
    mid = len(f[1][1]) // 2
    results.append(run_case("lost packet mid-frame drops only that frame", f, [0, 2], drop={(1, mid)}))
    results.append(run_case("lost marker packet drops only that frame", f, [0, 2], drop={(1, len(f[1][1]) - 1)}))
    results.append(run_case("lost first packet drops only that frame", f, [0, 2], drop={(1, 0)}))

    # Reordering within a frame: swap two packets in the middle of frame 1.
    f = frames_for(3, 60, 2, 1, 60)
    def swap_mid(allp, frames):
        base = len(frames[0][1]); m = base + len(frames[1][1]) // 2
        allp[m], allp[m + 1] = allp[m + 1], allp[m]
        return allp
    results.append(run_case("reordered packets inside a frame are still used", f, [0, 1, 2], transform=swap_mid))

    # Whole frame delivered backwards (marker first, offset 0 last).
    def reverse_frame1(allp, frames):
        a = len(frames[0][1]); b = a + len(frames[1][1])
        return allp[:a] + allp[a:b][::-1] + allp[b:]
    results.append(run_case("frame delivered in reverse order", f, [0, 1, 2], transform=reverse_frame1))

    # Frame 1's last packet arrives after frame 2 has started: frame 1 is
    # given up (no waiting = no added latency), the straggler is ignored.
    def straggler(allp, frames):
        a = len(frames[0][1]); b = a + len(frames[1][1])
        last = allp[b - 1]
        rest = allp[:b - 1] + allp[b:]
        rest.insert(b, last)  # after frame 2's first packet
        return rest
    results.append(run_case("straggler across frame boundary", f, [0, 2], transform=straggler))

    try:
        fr = frames_for(2, 60, 2, 65, 60, restart_marker_blocks=4)
        results.append(run_case("restart markers type 65 + DRI", fr, [0, 1]))
    except TypeError:
        print("[SKIP] restart markers (Pillow too old for restart_marker_blocks)")

    # Raw JPEG inside RTP (some vendor firmwares do this)
    raw, seq = [], 5
    for k in range(2):
        j = encode(test_image(W, H, k), 70, 2)
        pk, seq = packetize(j, 1, 70, W, H, 3600 * k, seq)
        raw.append((j, pk))
    results.append(run_case("raw full-JPEG payload passthrough", raw, [0, 1]))

    # Width/height = 0 (>2040 px) with SDP fallback size
    j = encode(test_image(W, H, 0), 70, 2)
    qt, dri, scan = split_jpeg(j)
    pk, _ = packetize(scan, 1, 70, 0, 0, 0, 1)
    results.append(run_case("zero size uses SDP fallback", [(j, pk)], [0], fallback=(W, H)))

    print(f"\n{sum(results)}/{len(results)} cases passed")
    sys.exit(0 if all(results) else 1)


main()
