// SPDX-License-Identifier: GPL-3.0-or-later
// Small JPEG helpers that work on the compressed bytes (no decoding).
#pragma once

#include <QByteArray>
#include <QSize>

namespace JpegUtil {

// Width/height from the frame's SOF marker; invalid QSize if not found.
QSize frameSize(const QByteArray &jpeg);

// EXIF orientation value (1, 3, 6, 8) for a clockwise display rotation.
int exifOrientationForRotation(int degreesClockwise);

// Returns a copy of `jpeg` with an EXIF APP1 segment carrying only the
// Orientation tag, inserted after SOI (and after JFIF APP0 if present).
// The image data itself is untouched. Returns `jpeg` unchanged for rotation 0.
QByteArray withExifOrientation(const QByteArray &jpeg, int degreesClockwise);

} // namespace JpegUtil
