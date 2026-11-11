// SPDX-License-Identifier: GPL-3.0-or-later
#include "rtpjpeg.h"

#include <cstring>

#include "jpeg_tables.inc"

namespace {

// RFC 2435 Appendix A base tables, in zig-zag order (as written to DQT).
const uint8_t kLumaQuant[64] = {
    16, 11, 12, 14, 12, 10, 16, 14, 13, 14, 18, 17, 16, 19, 24, 40,
    26, 24, 22, 22, 24, 49, 35, 37, 29, 40, 58, 51, 61, 60, 57, 51,
    56, 55, 64, 72, 92, 78, 64, 68, 87, 69, 55, 56, 80, 109, 81, 87,
    95, 98, 103, 104, 103, 62, 77, 113, 121, 112, 100, 120, 92, 101, 103, 99};

const uint8_t kChromaQuant[64] = {
    17, 18, 18, 24, 21, 24, 47, 26, 26, 47, 99, 66, 56, 66, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99};

constexpr int kMaxFrameBytes = 8 * 1024 * 1024;

inline void put16(QByteArray &b, int v)
{
    b.append(char((v >> 8) & 0xff));
    b.append(char(v & 0xff));
}

void putHuffman(QByteArray &b, const uint8_t *codelens, int ncodes,
                const uint8_t *symbols, int nsymbols, int tableNo, int tableClass)
{
    b.append(char(0xff));
    b.append(char(0xc4)); // DHT
    put16(b, 3 + ncodes + nsymbols);
    b.append(char((tableClass << 4) | tableNo));
    b.append(reinterpret_cast<const char *>(codelens), ncodes);
    b.append(reinterpret_cast<const char *>(symbols), nsymbols);
}

} // namespace

RtpJpegDepayloader::RtpJpegDepayloader() = default;

void RtpJpegDepayloader::reset()
{
    m_haveSeq = false;
    m_haveLastTs = false;
    m_inFrame = false;
    m_fragments.clear();
    m_staticTables.clear();
    m_packets = m_lost = m_frames = m_dropped = 0;
}

void RtpJpegDepayloader::setFallbackSize(int width, int height)
{
    m_fallbackW = width;
    m_fallbackH = height;
}

void RtpJpegDepayloader::makeTables(int q, uint8_t *lqt, uint8_t *cqt)
{
    int factor = q;
    if (factor < 1) factor = 1;
    if (factor > 99) factor = 99;
    const int scale = (q < 50) ? 5000 / factor : 200 - factor * 2;
    for (int i = 0; i < 64; ++i) {
        int lq = (kLumaQuant[i] * scale + 50) / 100;
        int cq = (kChromaQuant[i] * scale + 50) / 100;
        lqt[i] = uint8_t(lq < 1 ? 1 : (lq > 255 ? 255 : lq));
        cqt[i] = uint8_t(cq < 1 ? 1 : (cq > 255 ? 255 : cq));
    }
}

QByteArray RtpJpegDepayloader::makeHeaders(int type, int width, int height,
                                           const QByteArray &qtables, int precision,
                                           int restartInterval)
{
    QByteArray h;
    h.reserve(700);

    h.append(char(0xff));
    h.append(char(0xd8)); // SOI

    // DQT: one segment per table. Precision bit i => table i is 16-bit.
    const int lumLen = (precision & 1) ? 128 : 64;
    const int chmLen = (precision & 2) ? 128 : 64;
    const int nTables = qtables.size() >= lumLen + chmLen ? 2 : 1;
    int pos = 0;
    for (int t = 0; t < nTables; ++t) {
        const int len = (t == 0) ? lumLen : chmLen;
        const int pq = (t == 0) ? (precision & 1) : ((precision >> 1) & 1);
        h.append(char(0xff));
        h.append(char(0xdb));
        put16(h, 3 + len);
        h.append(char((pq << 4) | t));
        h.append(qtables.constData() + pos, len);
        pos += len;
    }

    if (restartInterval > 0) {
        h.append(char(0xff));
        h.append(char(0xdd)); // DRI
        put16(h, 4);
        put16(h, restartInterval);
    }

    // SOF0 (baseline), or SOF1 if any 16-bit quant table is used.
    h.append(char(0xff));
    h.append(char(precision ? 0xc1 : 0xc0));
    put16(h, 17);
    h.append(char(8));
    put16(h, height);
    put16(h, width);
    h.append(char(3));
    h.append(char(0));                             // Y
    h.append(char(type == 0 ? 0x21 : 0x22));       // 4:2:2 or 4:2:0
    h.append(char(0));
    h.append(char(1));                             // Cb
    h.append(char(0x11));
    h.append(char(nTables == 2 ? 1 : 0));
    h.append(char(2));                             // Cr
    h.append(char(0x11));
    h.append(char(nTables == 2 ? 1 : 0));

    putHuffman(h, kLumDcCodelens, 16, kLumDcSymbols, sizeof(kLumDcSymbols), 0, 0);
    putHuffman(h, kLumAcCodelens, 16, kLumAcSymbols, sizeof(kLumAcSymbols), 0, 1);
    putHuffman(h, kChmDcCodelens, 16, kChmDcSymbols, sizeof(kChmDcSymbols), 1, 0);
    putHuffman(h, kChmAcCodelens, 16, kChmAcSymbols, sizeof(kChmAcSymbols), 1, 1);

    // SOS
    h.append(char(0xff));
    h.append(char(0xda));
    put16(h, 12);
    h.append(char(3));
    h.append(char(0));
    h.append(char(0x00));
    h.append(char(1));
    h.append(char(0x11));
    h.append(char(2));
    h.append(char(0x11));
    h.append(char(0));
    h.append(char(63));
    h.append(char(0));
    return h;
}

void RtpJpegDepayloader::dropCurrent()
{
    if (m_inFrame) {
        ++m_dropped;
        m_haveLastTs = true;
        m_lastFinishedTs = m_ts; // late fragments of this frame are ignored
    }
    m_inFrame = false;
    m_fragments.clear();
    m_bytes = 0;
}

bool RtpJpegDepayloader::frameComplete() const
{
    if (!m_haveStart || m_endOffset < 0)
        return false;
    qint64 expected = 0;
    for (auto it = m_fragments.constBegin(); it != m_fragments.constEnd(); ++it) {
        if (qint64(it.key()) != expected)
            return false;
        expected += it.value().size();
    }
    return expected == m_endOffset;
}

bool RtpJpegDepayloader::push(const uint8_t *d, int len, QByteArray &out)
{
    // ---- RTP header ----
    if (len < 12 || (d[0] >> 6) != 2)
        return false;
    ++m_packets;

    const bool padding = d[0] & 0x20;
    const bool extension = d[0] & 0x10;
    const int csrcCount = d[0] & 0x0f;
    const bool marker = d[1] & 0x80;
    const uint16_t seq = uint16_t((d[2] << 8) | d[3]);
    const uint32_t ts = (uint32_t(d[4]) << 24) | (uint32_t(d[5]) << 16) |
                        (uint32_t(d[6]) << 8) | uint32_t(d[7]);

    int hdr = 12 + csrcCount * 4;
    if (extension) {
        if (len < hdr + 4)
            return false;
        hdr += 4 + ((d[hdr + 2] << 8) | d[hdr + 3]) * 4;
    }
    int end = len;
    if (padding)
        end -= d[len - 1];
    if (end - hdr < 8)
        return false;

    // Loss statistics: gaps count as lost, a late packet filling a gap undoes it.
    if (m_haveSeq) {
        const uint16_t diff = uint16_t(seq - m_lastSeq);
        if (diff == 0)
            return false; // duplicate
        if (diff < 0x8000) {
            m_lost += diff - 1;
            m_lastSeq = seq;
        } else if (m_lost > 0) {
            --m_lost; // reordered packet arrived late
        }
    } else {
        m_haveSeq = true;
        m_lastSeq = seq;
    }

    // ---- RTP/JPEG main header ----
    const uint8_t *p = d + hdr;
    const uint8_t *pend = d + end;
    const uint32_t offset = (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | p[3];
    int type = p[4];
    const int q = p[5];
    const int width = p[6] * 8;
    const int height = p[7] * 8;
    p += 8;

    int dri = 0;
    if (type >= 64 && type <= 127) {
        if (pend - p < 4)
            return false;
        dri = (p[0] << 8) | p[1];
        p += 4;
        type -= 64;
    }

    QByteArray qtables;
    int precision = 0;
    if (offset == 0) {
        if (q >= 128) {
            if (pend - p < 4)
                return false;
            precision = p[1];
            const int qlen = (p[2] << 8) | p[3];
            p += 4;
            if (qlen > 0) {
                if (pend - p < qlen)
                    return false;
                qtables = QByteArray(reinterpret_cast<const char *>(p), qlen);
                p += qlen;
                if (q < 255)
                    m_staticTables.insert(q, qtables);
            } else {
                qtables = m_staticTables.value(q); // "sent before" (RFC 2435 4.2)
            }
        } else {
            auto it = m_computedTables.find(q);
            if (it == m_computedTables.end()) {
                uint8_t lqt[64], cqt[64];
                makeTables(q, lqt, cqt);
                QByteArray t(reinterpret_cast<const char *>(lqt), 64);
                t.append(reinterpret_cast<const char *>(cqt), 64);
                it = m_computedTables.insert(q, t);
            }
            qtables = it.value();
        }
    }
    const int dlen = int(pend - p);

    // ---- which frame does this belong to? ----
    if (m_inFrame && ts != m_ts) {
        if (int32_t(ts - m_ts) < 0)
            return false;     // late fragment of an older frame
        dropCurrent();        // a newer frame started before this one completed
    }
    if (!m_inFrame) {
        if (m_haveLastTs && int32_t(ts - m_lastFinishedTs) <= 0)
            return false;     // straggler of a frame we already finished or dropped
        m_inFrame = true;
        m_ts = ts;
        m_haveStart = false;
        m_tooBig = false;
        m_endOffset = -1;
        m_bytes = 0;
        m_fragments.clear();
    }

    if (offset == 0) {
        m_haveStart = true;
        m_type = type;
        m_width = width ? width : m_fallbackW;
        m_height = height ? height : m_fallbackH;
        m_dri = dri;
        m_precision = precision;
        m_qtables = qtables;
        m_raw = dlen >= 2 && p[0] == 0xff && p[1] == 0xd8;
    }
    if (!m_fragments.contains(offset)) {
        m_fragments.insert(offset, QByteArray(reinterpret_cast<const char *>(p), dlen));
        m_bytes += dlen;
        if (m_bytes > kMaxFrameBytes)
            m_tooBig = true;
    }
    if (marker)
        m_endOffset = qint64(offset) + dlen;

    if (m_tooBig) {
        dropCurrent();
        return false;
    }
    if (!frameComplete())
        return false;

    // ---- frame complete ----
    const bool usable = m_raw || (!m_qtables.isEmpty() && (m_type == 0 || m_type == 1) &&
                                  m_width > 0 && m_height > 0);
    if (!usable) {
        dropCurrent();
        return false;
    }
    QByteArray scan;
    scan.reserve(int(m_bytes));
    for (const QByteArray &f : qAsConst(m_fragments))
        scan.append(f);
    if (m_raw) {
        out = scan;
    } else {
        out = makeHeaders(m_type, m_width, m_height, m_qtables, m_precision, m_dri);
        out.append(scan);
    }
    const int n = out.size();
    if (n < 2 || uint8_t(out[n - 2]) != 0xff || uint8_t(out[n - 1]) != 0xd9) {
        out.append(char(0xff));
        out.append(char(0xd9)); // EOI
    }
    m_inFrame = false;
    m_haveLastTs = true;
    m_lastFinishedTs = m_ts;
    m_fragments.clear();
    ++m_frames;
    return true;
}
