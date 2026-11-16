// SPDX-License-Identifier: GPL-3.0-or-later
// Builds a plain (non-fragmented) MP4 from MJPEG frames already stored in
// another file (the crash-safe MKV the recorder writes). The JPEG data is
// copied as-is: sample entry "mp4v" with object type 0x6C (JPEG), which is
// what ffmpeg writes for MJPEG in MP4.
#pragma once

#include <QSize>
#include <QString>
#include <QVector>

#include "mkvwriter.h"

namespace Mp4Writer {

// rotation: clockwise display rotation, written as the track matrix.
bool build(const QString &sourcePath, const QVector<MkvWriter::Sample> &samples,
           const QString &destPath, QSize frameSize, int rotation, QString *error = nullptr);

} // namespace Mp4Writer
