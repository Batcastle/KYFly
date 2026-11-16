// SPDX-License-Identifier: GPL-3.0-or-later
#include "mkvwriter.h"

#include <QRandomGenerator>
#include <cstring>

namespace {
// EBML / Matroska element IDs
enum : quint32 {
    kEBML = 0x1A45DFA3, kEBMLVersion = 0x4286, kEBMLReadVersion = 0x42F7,
    kEBMLMaxIDLength = 0x42F2, kEBMLMaxSizeLength = 0x42F3, kDocType = 0x4282,
    kDocTypeVersion = 0x4287, kDocTypeReadVersion = 0x4285,
    kSegment = 0x18538067, kSeekHead = 0x114D9B74, kSeek = 0x4DBB, kSeekID = 0x53AB,
    kSeekPosition = 0x53AC,
    kInfo = 0x1549A966, kTimestampScale = 0x2AD7B1, kDuration = 0x4489,
    kMuxingApp = 0x4D80, kWritingApp = 0x5741,
    kTracks = 0x1654AE6B, kTrackEntry = 0xAE, kTrackNumber = 0xD7, kTrackUID = 0x73C5,
    kTrackType = 0x83, kFlagLacing = 0x9C, kCodecID = 0x86, kLanguage = 0x22B59C,
    kVideo = 0xE0, kPixelWidth = 0xB0, kPixelHeight = 0xBA,
    kProjection = 0x7670, kProjectionType = 0x7671, kProjectionPoseRoll = 0x7675,
    kCluster = 0x1F43B675, kTimestamp = 0xE7, kSimpleBlock = 0xA3,
    kCues = 0x1C53BB6B, kCuePoint = 0xBB, kCueTime = 0xB3, kCueTrackPositions = 0xB7,
    kCueTrack = 0xF7, kCueClusterPosition = 0xF1,
};
constexpr int kSeekHeadReserve = 128;     // bytes reserved for the SeekHead
constexpr int kDurationReserve = 11;      // 2-byte ID + 1-byte size + 8-byte float
constexpr qint64 kClusterMs = 1000;       // start a new cluster at least every second
const QByteArray kUnknownSize8("\x01\xff\xff\xff\xff\xff\xff\xff", 8);

QByteArray idBytes(quint32 id)
{
    QByteArray b;
    if (id > 0xFFFFFF) b.append(char(id >> 24));
    if (id > 0xFFFF) b.append(char(id >> 16));
    if (id > 0xFF) b.append(char(id >> 8));
    b.append(char(id));
    return b;
}
} // namespace

MkvWriter::~MkvWriter()
{
    if (m_file.isOpen())
        close();
}

QByteArray MkvWriter::vint(quint64 value, int length)
{
    if (length == 0) {
        length = 1;
        while (length < 8 && value >= (quint64(1) << (7 * length)) - 1)
            ++length;
    }
    QByteArray b(length, '\0');
    for (int i = length - 1; i >= 0; --i) {
        b[i] = char(value & 0xff);
        value >>= 8;
    }
    b[0] = char(uchar(b[0]) | (0x80 >> (length - 1)));
    return b;
}

QByteArray MkvWriter::element(quint32 id, const QByteArray &payload)
{
    return idBytes(id) + vint(quint64(payload.size())) + payload;
}

QByteArray MkvWriter::uintElement(quint32 id, quint64 v)
{
    QByteArray p;
    do {
        p.prepend(char(v & 0xff));
        v >>= 8;
    } while (v);
    return element(id, p);
}

QByteArray MkvWriter::floatElement(quint32 id, double v)
{
    quint64 bits;
    std::memcpy(&bits, &v, 8);
    QByteArray p(8, '\0');
    for (int i = 7; i >= 0; --i) {
        p[i] = char(bits & 0xff);
        bits >>= 8;
    }
    return element(id, p);
}

QByteArray MkvWriter::stringElement(quint32 id, const QByteArray &s)
{
    return element(id, s);
}

QByteArray MkvWriter::voidElement(int totalLength)
{
    // ID (1 byte) + size; use a 1-byte size when it fits, else an 8-byte one.
    if (totalLength < 2)
        return {};
    if (totalLength - 2 <= 126)
        return QByteArray(1, char(0xEC)) + vint(quint64(totalLength - 2), 1) +
               QByteArray(totalLength - 2, '\0');
    return QByteArray(1, char(0xEC)) + vint(quint64(totalLength - 9), 8) +
           QByteArray(totalLength - 9, '\0');
}

bool MkvWriter::fail(const QString &why)
{
    m_error = why + QStringLiteral(": ") + m_file.errorString();
    return false;
}

bool MkvWriter::put(const QByteArray &bytes)
{
    if (m_file.write(bytes) != bytes.size())
        return fail(QStringLiteral("write failed"));
    return true;
}

bool MkvWriter::open(const QString &path, QSize frameSize, int rotation)
{
    m_file.setFileName(path);
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return fail(QStringLiteral("cannot create %1").arg(path));
    m_size = frameSize;
    m_rotation = ((rotation % 360) + 360) % 360;
    m_samples.clear();
    m_cues.clear();
    m_clusterPos = m_clusterSizePos = -1;
    m_lastMs = 0;

    QByteArray ebml;
    ebml += uintElement(kEBMLVersion, 1);
    ebml += uintElement(kEBMLReadVersion, 1);
    ebml += uintElement(kEBMLMaxIDLength, 4);
    ebml += uintElement(kEBMLMaxSizeLength, 8);
    ebml += stringElement(kDocType, "matroska");
    ebml += uintElement(kDocTypeVersion, 4);
    ebml += uintElement(kDocTypeReadVersion, 2);
    if (!put(element(kEBML, ebml)))
        return false;

    if (!put(idBytes(kSegment) + kUnknownSize8))
        return false;
    m_segmentDataStart = m_file.pos();

    m_seekHeadPos = m_file.pos();
    if (!put(voidElement(kSeekHeadReserve)))
        return false;

    // Info. Duration is a Void placeholder until close() knows it.
    QByteArray info;
    info += uintElement(kTimestampScale, 1000000); // 1 ms
    info += stringElement(kMuxingApp, "KY Fly");
    info += stringElement(kWritingApp, "KY Fly " KYFLY_VERSION);
    const int durationOffsetInInfo = info.size();
    info += voidElement(kDurationReserve);
    const QByteArray infoEl = element(kInfo, info);
    m_infoPos = m_file.pos();
    m_durationPos = m_infoPos + (infoEl.size() - info.size()) + durationOffsetInInfo;
    if (!put(infoEl))
        return false;

    QByteArray video;
    video += uintElement(kPixelWidth, quint64(frameSize.width()));
    video += uintElement(kPixelHeight, quint64(frameSize.height()));
    if (m_rotation) {
        // Matroska's roll is counter-clockwise; ffmpeg maps it to its display
        // matrix the same way it does for MP4 (verified in tests/capture_cases.py).
        QByteArray proj;
        proj += uintElement(kProjectionType, 0); // rectangular
        proj += floatElement(kProjectionPoseRoll, -double(m_rotation));
        video += element(kProjection, proj);
    }
    QByteArray track;
    track += uintElement(kTrackNumber, 1);
    track += uintElement(kTrackUID, (quint64(QRandomGenerator::global()->generate()) << 32 |
                                     QRandomGenerator::global()->generate()) | 1);
    track += uintElement(kTrackType, 1); // video
    track += uintElement(kFlagLacing, 0);
    track += stringElement(kCodecID, "V_MJPEG");
    track += stringElement(kLanguage, "und");
    track += element(kVideo, video);
    m_tracksPos = m_file.pos();
    if (!put(element(kTracks, element(kTrackEntry, track))))
        return false;
    m_file.flush();
    return true;
}

bool MkvWriter::startCluster(qint64 ms)
{
    if (!finishCluster())
        return false;
    m_clusterPos = m_file.pos();
    m_cues.append(qMakePair(ms, m_clusterPos - m_segmentDataStart));
    if (!put(idBytes(kCluster)))
        return false;
    m_clusterSizePos = m_file.pos();
    if (!put(kUnknownSize8) || !put(uintElement(kTimestamp, quint64(ms))))
        return false;
    m_clusterMs = ms;
    return true;
}

bool MkvWriter::finishCluster()
{
    if (m_clusterSizePos < 0)
        return true;
    const qint64 end = m_file.pos();
    const qint64 size = end - (m_clusterSizePos + 8);
    if (!m_file.seek(m_clusterSizePos) || !put(vint(quint64(size), 8)) || !m_file.seek(end))
        return fail(QStringLiteral("seek failed"));
    m_clusterSizePos = -1;
    return true;
}

bool MkvWriter::writeFrame(const QByteArray &jpeg, qint64 ms)
{
    if (!m_file.isOpen())
        return false;
    if (ms < m_lastMs)
        ms = m_lastMs;
    if (m_clusterSizePos < 0 || ms - m_clusterMs >= kClusterMs || ms - m_clusterMs > 32767) {
        if (!startCluster(ms))
            return false;
    }
    const qint64 rel = ms - m_clusterMs;
    QByteArray hdr = idBytes(kSimpleBlock) + vint(quint64(jpeg.size()) + 4);
    hdr.append(char(0x81));            // track 1
    hdr.append(char((rel >> 8) & 0xff));
    hdr.append(char(rel & 0xff));
    hdr.append(char(0x80));            // keyframe (every MJPEG frame is)
    if (!put(hdr))
        return false;
    const qint64 payloadPos = m_file.pos();
    if (!put(jpeg))
        return false;
    m_samples.append({payloadPos, jpeg.size(), ms});
    m_lastMs = ms;
    // Push to the kernel after every frame so an app crash loses nothing.
    m_file.flush();
    return true;
}

bool MkvWriter::close()
{
    if (!m_file.isOpen())
        return false;
    bool ok = finishCluster();

    // Cues (one per cluster) at the end.
    const qint64 cuesPos = m_file.pos();
    if (ok && !m_cues.isEmpty()) {
        QByteArray cues;
        for (const auto &c : qAsConst(m_cues)) {
            QByteArray tp = uintElement(kCueTrack, 1) + uintElement(kCueClusterPosition, quint64(c.second));
            cues += element(kCuePoint, uintElement(kCueTime, quint64(c.first)) + element(kCueTrackPositions, tp));
        }
        ok = put(element(kCues, cues));
    }
    const qint64 end = m_file.pos();

    // Duration: last frame time plus one typical frame interval.
    if (ok) {
        double dur = double(m_lastMs);
        if (m_samples.size() > 1)
            dur += double(m_lastMs - m_samples.first().ms) / (m_samples.size() - 1);
        else
            dur += 40;
        const QByteArray d = floatElement(kDuration, dur);
        ok = d.size() == kDurationReserve && m_file.seek(m_durationPos) && put(d);
    }

    // SeekHead into the reserved space at the start.
    if (ok) {
        auto seek = [this](quint32 id, qint64 pos) {
            return element(kSeek, element(kSeekID, idBytes(id)) +
                                      idBytes(kSeekPosition) + vint(8) +
                                      QByteArray::fromHex(QByteArray::number(pos - m_segmentDataStart, 16)
                                                              .rightJustified(16, '0')));
        };
        QByteArray sh = seek(kInfo, m_infoPos) + seek(kTracks, m_tracksPos);
        if (!m_cues.isEmpty())
            sh += seek(kCues, cuesPos);
        QByteArray block = element(kSeekHead, sh);
        block += voidElement(kSeekHeadReserve - block.size());
        ok = block.size() == kSeekHeadReserve && m_file.seek(m_seekHeadPos) && put(block);
    }

    // Segment size, now that it's known.
    if (ok)
        ok = m_file.seek(m_segmentDataStart - 8) && put(vint(quint64(end - m_segmentDataStart), 8));

    m_file.seek(end);
    m_file.close();
    return ok;
}
