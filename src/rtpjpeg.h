// SPDX-License-Identifier: GPL-3.0-or-later
// RTP/JPEG (RFC 2435) depayloader with JPEG header reconstruction.
//
// The drone streams MJPEG as RTP payload type 26. Each RTP packet carries a
// fragment of the entropy-coded scan data plus a small header giving the
// image type, quality factor and size. The JFIF headers (quant tables,
// Huffman tables, SOF, SOS) are NOT sent, so we rebuild them here.
//
// Pure C++/QtCore so it can be unit-tested without a network.
#pragma once

#include <QByteArray>
#include <QHash>
#include <QMap>
#include <cstdint>

class RtpJpegDepayloader
{
public:
    RtpJpegDepayloader();

    // Feed one complete RTP datagram (including the 12+ byte RTP header).
    // Returns true when a full JPEG frame was completed; it is written to `out`.
    bool push(const uint8_t *data, int len, QByteArray &out);

    void reset();

    // Used when the RTP/JPEG header says width/height = 0 (images > 2040 px);
    // the size then has to come from the SDP (a=x-dimensions / a=framesize).
    void setFallbackSize(int width, int height);

    quint64 packets() const { return m_packets; }
    quint64 lostPackets() const { return m_lost; }
    quint64 frames() const { return m_frames; }
    quint64 droppedFrames() const { return m_dropped; }

    // Exposed for tests.
    static void makeTables(int q, uint8_t *lqt, uint8_t *cqt);
    static QByteArray makeHeaders(int type, int width, int height,
                                  const QByteArray &qtables, int precision,
                                  int restartInterval);

private:
    void dropCurrent();
    bool frameComplete() const;

    // sequence tracking (statistics only)
    bool m_haveSeq = false;
    uint16_t m_lastSeq = 0;

    // Frame being assembled. Fragments are keyed by byte offset, so packets
    // that arrive out of order within a frame are still used.
    bool m_inFrame = false;
    bool m_haveStart = false;    // offset-0 fragment (carries the header info) seen
    bool m_tooBig = false;
    bool m_raw = false;          // payload already contains a full JPEG (FFD8...)
    uint32_t m_ts = 0;
    bool m_haveLastTs = false;
    uint32_t m_lastFinishedTs = 0;
    qint64 m_endOffset = -1;     // known once the marker packet arrives
    qint64 m_bytes = 0;
    QMap<uint32_t, QByteArray> m_fragments;
    int m_type = 0;
    int m_width = 0;
    int m_height = 0;
    int m_dri = 0;
    int m_precision = 0;
    QByteArray m_qtables;

    // caches
    QHash<int, QByteArray> m_staticTables;   // Q 128..254 -> in-band tables
    QHash<int, QByteArray> m_computedTables; // Q 1..127 -> computed tables
    int m_fallbackW = 0;
    int m_fallbackH = 0;

    quint64 m_packets = 0;
    quint64 m_lost = 0;
    quint64 m_frames = 0;
    quint64 m_dropped = 0;
};
