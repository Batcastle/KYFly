// Feeds a file of length-prefixed RTP packets through RtpJpegDepayloader and
// writes every completed JPEG to <outdir>/frame_<n>.jpg.
// File format: repeated { uint32 little-endian length, <length> bytes }.
#include "../src/rtpjpeg.h"

#include <QFile>
#include <QtEndian>
#include <cstdio>

int main(int argc, char **argv)
{
    if (argc < 3) {
        std::fprintf(stderr, "usage: %s packets.bin outdir [fallbackW fallbackH]\n", argv[0]);
        return 2;
    }
    QFile f(QString::fromLocal8Bit(argv[1]));
    if (!f.open(QIODevice::ReadOnly))
        return 2;
    const QByteArray all = f.readAll();

    RtpJpegDepayloader depay;
    if (argc >= 5)
        depay.setFallbackSize(atoi(argv[3]), atoi(argv[4]));

    int pos = 0, n = 0;
    QByteArray jpeg;
    while (pos + 4 <= all.size()) {
        const quint32 len = qFromLittleEndian<quint32>(
            reinterpret_cast<const uchar *>(all.constData() + pos));
        pos += 4;
        if (pos + int(len) > all.size())
            break;
        if (depay.push(reinterpret_cast<const uint8_t *>(all.constData() + pos), int(len), jpeg)) {
            QFile o(QStringLiteral("%1/frame_%2.jpg").arg(QString::fromLocal8Bit(argv[2])).arg(n++));
            o.open(QIODevice::WriteOnly);
            o.write(jpeg);
        }
        pos += int(len);
    }
    std::printf("packets=%llu frames=%llu dropped=%llu lost=%llu\n",
                (unsigned long long)depay.packets(), (unsigned long long)depay.frames(),
                (unsigned long long)depay.droppedFrames(), (unsigned long long)depay.lostPackets());
    return 0;
}
