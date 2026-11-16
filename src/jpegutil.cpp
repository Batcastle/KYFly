// SPDX-License-Identifier: GPL-3.0-or-later
#include "jpegutil.h"

namespace JpegUtil {

QSize frameSize(const QByteArray &jpeg)
{
    const auto *d = reinterpret_cast<const uchar *>(jpeg.constData());
    const int n = jpeg.size();
    if (n < 4 || d[0] != 0xff || d[1] != 0xd8)
        return {};
    int i = 2;
    while (i + 4 <= n) {
        if (d[i] != 0xff) {
            ++i;
            continue;
        }
        const int m = d[i + 1];
        if (m == 0xff) { // fill byte
            ++i;
            continue;
        }
        if (m == 0xd8 || m == 0x01 || (m >= 0xd0 && m <= 0xd7)) { // no length
            i += 2;
            continue;
        }
        const int len = (d[i + 2] << 8) | d[i + 3];
        // SOF0..SOF15 except DHT (C4), JPG (C8), DAC (CC)
        if (m >= 0xc0 && m <= 0xcf && m != 0xc4 && m != 0xc8 && m != 0xcc) {
            if (i + 9 > n)
                return {};
            const int h = (d[i + 5] << 8) | d[i + 6];
            const int w = (d[i + 7] << 8) | d[i + 8];
            return QSize(w, h);
        }
        if (m == 0xda || m == 0xd9) // scan started / end: no SOF found
            return {};
        i += 2 + len;
    }
    return {};
}

int exifOrientationForRotation(int degreesClockwise)
{
    switch (((degreesClockwise % 360) + 360) % 360) {
    case 90: return 6;
    case 180: return 3;
    case 270: return 8;
    default: return 1;
    }
}

QByteArray withExifOrientation(const QByteArray &jpeg, int degreesClockwise)
{
    const int orientation = exifOrientationForRotation(degreesClockwise);
    if (orientation == 1 || jpeg.size() < 4)
        return jpeg;

    // APP1: "Exif\0\0" + big-endian TIFF header + IFD0 with one SHORT entry.
    static const char tiff[] = {
        'E', 'x', 'i', 'f', 0, 0,
        'M', 'M', 0, 42, 0, 0, 0, 8,     // TIFF header, IFD0 at offset 8
        0, 1,                            // 1 entry
        0x01, 0x12, 0, 3, 0, 0, 0, 1,    // tag 0x0112 Orientation, SHORT, count 1
        0, 0, 0, 0,                      // value (patched below) + padding
        0, 0, 0, 0                       // no next IFD
    };
    QByteArray app1(tiff, int(sizeof tiff));
    app1[6 + 8 + 2 + 8 + 1] = char(orientation); // low byte of the SHORT
    QByteArray seg;
    seg.append(char(0xff));
    seg.append(char(0xe1));
    const int len = app1.size() + 2;
    seg.append(char(len >> 8));
    seg.append(char(len & 0xff));
    seg.append(app1);

    int insertAt = 2;
    const auto *d = reinterpret_cast<const uchar *>(jpeg.constData());
    if (jpeg.size() > 6 && d[2] == 0xff && d[3] == 0xe0) // keep JFIF APP0 first
        insertAt = 4 + ((d[4] << 8) | d[5]);
    QByteArray out = jpeg;
    out.insert(insertAt, seg);
    return out;
}

} // namespace JpegUtil
