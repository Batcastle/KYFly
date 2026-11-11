#!/usr/bin/env python3
"""Compare C++ packet building / stick mapping with a direct port of the
decompiled Java (FlyController$FlyControlTask.run, DeviceBLFragment.onMove)."""
import random, subprocess, sys

def java_packet(fmt, b1, b2, acc, turn, fastFly, fastDrop, estop, circleEnd, noHead, fastRet, light, gyro, fixedH):
    # FlyControlTask.run (fallback-decompiled bytecode)
    i7 = (1 if fastFly else 0) + (2 if fastDrop else 0) + (4 if estop else 0) + (8 if circleEnd else 0) \
         + (16 if noHead else 0) + (32 if fastRet else 0) + (64 if light else 0) + (128 if gyro else 0)
    if acc == 1: acc = 0
    i8 = b1 ^ b2 ^ acc ^ turn ^ (i7 & 255)
    if fmt == 8:   # deviceType == 10
        p = [102, b1, b2, acc, turn, i7, i8, 0x99]
    else:
        i9 = (1 if (fastFly or fastDrop) else 0) + (2 if estop else 0) + (4 if gyro else 0) + (8 if circleEnd else 0) + (16 if light else 0)
        i10 = (1 if noHead else 0) + (2 if fixedH else 0)
        x = (i9 ^ (b1 ^ b2 ^ acc ^ turn)) ^ (i10 & 255)
        p = [102, 20, b1, b2, acc, turn, i9, i10] + [0] * 10 + [x, 0x99]
    return bytes([3] + [v & 0xff for v in p]).hex()

def jint(x):  # Java (int) cast truncates toward zero
    return int(x)

def java_map(yaw, thr, roll, pitch, speed, tr, tp, ty):
    # DeviceBLFragment.onMove: f/f3 is our normalized axis
    i3 = jint(yaw * 127.0); i4 = jint(thr * 127.0)
    if 104 <= i3 + 128 <= 152: i3 = 0
    turn = i3 + ty * 2 + 128
    acc = i4 + 128
    half = [45, 60, 127][speed]; mult = [2, 3, 4][speed]
    r = jint(roll * half) + tr * mult + 128
    p = jint(pitch * half) + tp * mult + 128
    clamp = lambda v: 255 if v > 255 else (1 if v < 1 else v)   # FlyControlTask clamps
    if acc == 1: acc = 0
    return clamp(r), clamp(p), acc, clamp(turn)

random.seed(7)
lines, expect = [], []
for _ in range(4000):
    fmt = random.choice([8, 20])
    vals = [random.randint(1, 255) for _ in range(4)]
    flags = [random.random() < 0.3 for _ in range(9)]
    lines.append("pkt %d %d %d %d %d " % (fmt, *vals) + " ".join(str(int(f)) for f in flags))
    expect.append(java_packet(fmt, *vals, *flags))
grid = [-1, -0.75, -0.5, -0.19, -0.18, 0, 0.18, 0.19, 0.5, 0.9, 1]
for _ in range(4000):
    a = [random.choice(grid + [round(random.uniform(-1, 1), 3)]) for _ in range(4)]
    s = random.randint(0, 2); t = [random.randint(-12, 12) for _ in range(3)]
    lines.append("map %s %s %s %s %d %d %d %d" % (*a, s, *t))
    expect.append("%d %d %d %d" % java_map(*a, s, *t))

out = subprocess.run([sys.argv[1]], input="\n".join(lines) + "\n", capture_output=True, text=True,
                     env={"QT_QPA_PLATFORM": "offscreen"}).stdout.split("\n")
bad = [(l, e, o) for l, e, o in zip(lines, expect, out) if e != o]
print(f"{len(lines) - len(bad)}/{len(lines)} match the decompiled Java")
for l, e, o in bad[:10]:
    print("MISMATCH", l, "\n  java:", e, "\n  c++ :", o)
sys.exit(1 if bad else 0)
