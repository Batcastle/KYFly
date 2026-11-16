// SPDX-License-Identifier: GPL-3.0-or-later
#include "mp4writer.h"

#include <QFile>

namespace {

void be16(QByteArray &b, quint32 v) { b.append(char(v >> 8)); b.append(char(v)); }
void be32(QByteArray &b, quint32 v)
{
    b.append(char(v >> 24)); b.append(char(v >> 16)); b.append(char(v >> 8)); b.append(char(v));
}
void be64(QByteArray &b, quint64 v) { be32(b, quint32(v >> 32)); be32(b, quint32(v)); }

QByteArray box(const char *type, const QByteArray &payload)
{
    QByteArray b;
    be32(b, quint32(8 + payload.size()));
    b.append(type, 4);
    b.append(payload);
    return b;
}

QByteArray fullBox(const char *type, quint8 version, quint32 flags, const QByteArray &payload)
{
    QByteArray p;
    be32(p, (quint32(version) << 24) | (flags & 0xffffff));
    p.append(payload);
    return box(type, p);
}

// Display matrix for a clockwise rotation of a w x h picture (ISO 14496-12 8.3.2).
void matrix(QByteArray &b, int rotation, int w, int h)
{
    const qint32 one = 0x10000, w16 = w << 16, h16 = h << 16;
    qint32 m[9] = {one, 0, 0, 0, one, 0, 0, 0, 0x40000000};
    switch (rotation) {
    case 90:  m[0] = 0;    m[1] = one;  m[3] = -one; m[4] = 0;    m[6] = h16; break;
    case 180: m[0] = -one; m[4] = -one; m[6] = w16;  m[7] = h16; break;
    case 270: m[0] = 0;    m[1] = -one; m[3] = one;  m[4] = 0;    m[7] = w16; break;
    default: break;
    }
    for (qint32 v : m)
        be32(b, quint32(v));
}

QByteArray descriptor(quint8 tag, const QByteArray &payload)
{
    QByteArray b;
    b.append(char(tag));
    // 4-byte size form, as ffmpeg writes it
    const quint32 n = quint32(payload.size());
    b.append(char(0x80 | ((n >> 21) & 0x7f)));
    b.append(char(0x80 | ((n >> 14) & 0x7f)));
    b.append(char(0x80 | ((n >> 7) & 0x7f)));
    b.append(char(n & 0x7f));
    b.append(payload);
    return b;
}

} // namespace

namespace Mp4Writer {

bool build(const QString &sourcePath, const QVector<MkvWriter::Sample> &samples,
           const QString &destPath, QSize size, int rotation, QString *error)
{
    auto fail = [error](const QString &e) { if (error) *error = e; return false; };
    if (samples.isEmpty())
        return fail(QStringLiteral("no frames"));
    rotation = ((rotation % 360) + 360) % 360;

    QFile src(sourcePath);
    if (!src.open(QIODevice::ReadOnly))
        return fail(QStringLiteral("cannot read %1").arg(sourcePath));
    QFile out(destPath);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return fail(QStringLiteral("cannot create %1").arg(destPath));

    // ftyp
    QByteArray ftyp;
    ftyp.append("isom", 4);
    be32(ftyp, 0x200);
    ftyp.append("isomiso2mp41", 12);
    out.write(box("ftyp", ftyp));

    // mdat with 64-bit size, patched after copying
    const qint64 mdatPos = out.pos();
    QByteArray mdatHdr;
    be32(mdatHdr, 1);
    mdatHdr.append("mdat", 4);
    be64(mdatHdr, 0);
    out.write(mdatHdr);

    QVector<quint64> offsets;
    offsets.reserve(samples.size());
    quint32 maxSize = 0;
    quint64 totalBytes = 0;
    for (const auto &s : samples) {
        if (!src.seek(s.offset))
            return fail(QStringLiteral("seek failed in source"));
        const QByteArray data = src.read(s.size);
        if (data.size() != s.size)
            return fail(QStringLiteral("short read from source"));
        offsets.append(quint64(out.pos()));
        if (out.write(data) != data.size())
            return fail(QStringLiteral("write failed: %1").arg(out.errorString()));
        maxSize = qMax(maxSize, quint32(s.size));
        totalBytes += quint64(s.size);
    }
    const qint64 mdatEnd = out.pos();
    out.seek(mdatPos + 8);
    QByteArray mdatSize;
    be64(mdatSize, quint64(mdatEnd - mdatPos));
    out.write(mdatSize);
    out.seek(mdatEnd);

    // Per-sample durations (ms). The last frame gets the average interval.
    const int n = samples.size();
    QVector<quint32> deltas(n);
    const qint64 span = samples.last().ms - samples.first().ms;
    const quint32 avg = n > 1 ? quint32(qMax<qint64>(1, span / (n - 1))) : 40;
    for (int i = 0; i < n - 1; ++i)
        deltas[i] = quint32(qMax<qint64>(1, samples[i + 1].ms - samples[i].ms));
    deltas[n - 1] = avg;
    quint64 duration = 0;
    for (quint32 d : deltas)
        duration += d;

    const int w = size.width(), h = size.height();

    QByteArray mvhd;
    be32(mvhd, 0); be32(mvhd, 0);          // creation / modification
    be32(mvhd, 1000);                      // timescale (ms)
    be32(mvhd, quint32(duration));
    be32(mvhd, 0x00010000);                // rate 1.0
    be16(mvhd, 0x0100);                    // volume 1.0
    mvhd.append(QByteArray(10, '\0'));
    matrix(mvhd, 0, 0, 0);
    mvhd.append(QByteArray(24, '\0'));     // pre_defined
    be32(mvhd, 2);                         // next track ID

    QByteArray tkhd;
    be32(tkhd, 0); be32(tkhd, 0);
    be32(tkhd, 1);                         // track ID
    be32(tkhd, 0);
    be32(tkhd, quint32(duration));
    tkhd.append(QByteArray(8, '\0'));
    be16(tkhd, 0); be16(tkhd, 0);          // layer, alternate group
    be16(tkhd, 0); be16(tkhd, 0);          // volume, reserved
    matrix(tkhd, rotation, w, h);
    be32(tkhd, quint32(w) << 16);
    be32(tkhd, quint32(h) << 16);

    QByteArray mdhd;
    be32(mdhd, 0); be32(mdhd, 0);
    be32(mdhd, 1000);
    be32(mdhd, quint32(duration));
    be16(mdhd, 0x55C4);                    // language "und"
    be16(mdhd, 0);

    QByteArray hdlr;
    be32(hdlr, 0);
    hdlr.append("vide", 4);
    hdlr.append(QByteArray(12, '\0'));
    hdlr.append("VideoHandler", 13);       // includes the terminating NUL

    QByteArray vmhd;
    be16(vmhd, 0); be16(vmhd, 0); be16(vmhd, 0); be16(vmhd, 0);

    QByteArray dref;
    be32(dref, 1);
    dref.append(fullBox("url ", 0, 1, {}));

    // esds: ES_Descriptor > DecoderConfigDescriptor(JPEG) + SLConfigDescriptor
    QByteArray dcd;
    dcd.append(char(0x6C));                // objectTypeIndication: JPEG
    dcd.append(char(0x11));                // streamType visual (4) << 2 | reserved 1
    dcd.append(char((maxSize >> 16) & 0xff));
    dcd.append(char((maxSize >> 8) & 0xff));
    dcd.append(char(maxSize & 0xff));      // bufferSizeDB
    const quint32 avgBitrate = quint32(duration ? totalBytes * 8000 / duration : 0);
    be32(dcd, avgBitrate);                 // maxBitrate (approximate)
    be32(dcd, avgBitrate);                 // avgBitrate
    QByteArray es;
    be16(es, 1);                           // ES_ID
    es.append(char(0));                    // flags
    es.append(descriptor(0x04, dcd));
    es.append(descriptor(0x06, QByteArray(1, char(0x02))));
    const QByteArray esds = fullBox("esds", 0, 0, descriptor(0x03, es));

    QByteArray mp4v;
    mp4v.append(QByteArray(6, '\0'));
    be16(mp4v, 1);                         // data reference index
    mp4v.append(QByteArray(16, '\0'));     // pre_defined / reserved
    be16(mp4v, quint32(w));
    be16(mp4v, quint32(h));
    be32(mp4v, 0x00480000);                // 72 dpi
    be32(mp4v, 0x00480000);
    be32(mp4v, 0);
    be16(mp4v, 1);                         // frame count
    mp4v.append(QByteArray(32, '\0'));     // compressor name
    be16(mp4v, 0x0018);                    // depth
    be16(mp4v, 0xFFFF);                    // pre_defined -1
    mp4v.append(esds);

    QByteArray stsd;
    be32(stsd, 1);
    stsd.append(box("mp4v", mp4v));

    QByteArray stts;
    QVector<QPair<quint32, quint32>> runs;
    for (quint32 d : deltas) {
        if (!runs.isEmpty() && runs.last().second == d)
            ++runs.last().first;
        else
            runs.append(qMakePair(quint32(1), d));
    }
    be32(stts, quint32(runs.size()));
    for (const auto &r : runs) { be32(stts, r.first); be32(stts, r.second); }

    QByteArray stsc;                       // one sample per chunk
    be32(stsc, 1); be32(stsc, 1); be32(stsc, 1); be32(stsc, 1);

    QByteArray stsz;
    be32(stsz, 0);
    be32(stsz, quint32(n));
    for (const auto &s : samples)
        be32(stsz, quint32(s.size));

    QByteArray co64;
    be32(co64, quint32(n));
    for (quint64 o : offsets)
        be64(co64, o);

    // Every MJPEG frame is a sync sample, so no stss box is needed.
    const QByteArray stbl = box("stbl", fullBox("stsd", 0, 0, stsd) + fullBox("stts", 0, 0, stts) +
                                            fullBox("stsc", 0, 0, stsc) + fullBox("stsz", 0, 0, stsz) +
                                            fullBox("co64", 0, 0, co64));
    const QByteArray minf = box("minf", fullBox("vmhd", 0, 1, vmhd) +
                                            box("dinf", fullBox("dref", 0, 0, dref)) + stbl);
    const QByteArray mdia = box("mdia", fullBox("mdhd", 0, 0, mdhd) + fullBox("hdlr", 0, 0, hdlr) + minf);
    const QByteArray trak = box("trak", fullBox("tkhd", 0, 3, tkhd) + mdia);
    const QByteArray moov = box("moov", fullBox("mvhd", 0, 0, mvhd) + trak);
    if (out.write(moov) != moov.size())
        return fail(QStringLiteral("write failed: %1").arg(out.errorString()));
    if (!out.flush())
        return fail(QStringLiteral("flush failed"));
    out.close();
    return true;
}

} // namespace Mp4Writer
