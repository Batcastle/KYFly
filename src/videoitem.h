// SPDX-License-Identifier: GPL-3.0-or-later
// Scene-graph item that shows the newest decoded video frame.
// If several frames arrive between two renders, only the last one is uploaded.
#pragma once

#include <QImage>
#include <QPointer>
#include <QQuickItem>

#include "videoreceiver.h"

class VideoItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(VideoReceiver *receiver READ receiver WRITE setReceiver NOTIFY receiverChanged)
    Q_PROPERTY(bool fillMode READ fillMode WRITE setFillMode NOTIFY fillModeChanged)
    Q_PROPERTY(bool hasFrame READ hasFrame NOTIFY hasFrameChanged)
    // Size of the most recent frame, updated immediately (used for auto-rotation).
    Q_PROPERTY(int sourceWidth READ sourceWidth NOTIFY sourceSizeChanged)
    Q_PROPERTY(int sourceHeight READ sourceHeight NOTIFY sourceSizeChanged)

public:
    explicit VideoItem(QQuickItem *parent = nullptr);

    VideoReceiver *receiver() const { return m_receiver; }
    void setReceiver(VideoReceiver *r);
    bool fillMode() const { return m_fill; }   // true = crop to fill, false = letterbox
    void setFillMode(bool on);
    bool hasFrame() const { return m_hasFrame; }
    int sourceWidth() const { return m_sourceSize.width(); }
    int sourceHeight() const { return m_sourceSize.height(); }

    Q_INVOKABLE void clear();

public slots:
    void setFrame(const QImage &image);

signals:
    void receiverChanged();
    void sourceSizeChanged();
    void fillModeChanged();
    void hasFrameChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;

private:
    QPointer<VideoReceiver> m_receiver;
    QImage m_pending;
    bool m_dirty = false;
    bool m_hasFrame = false;
    QSize m_sourceSize;
    bool m_fill = false;
    bool m_clearRequested = false;
    QSize m_frameSize;
};
