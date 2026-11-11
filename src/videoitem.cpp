// SPDX-License-Identifier: GPL-3.0-or-later
#include "videoitem.h"

#include <QQuickWindow>
#include <QSGSimpleTextureNode>
#include <QSGTexture>

namespace {
// Owns its texture explicitly so behaviour doesn't depend on Qt version.
class FrameNode : public QSGSimpleTextureNode
{
public:
    ~FrameNode() override { delete m_tex; }
    void replaceTexture(QSGTexture *t)
    {
        setTexture(t);
        delete m_tex;
        m_tex = t;
    }

private:
    QSGTexture *m_tex = nullptr;
};
} // namespace

VideoItem::VideoItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
}

void VideoItem::setReceiver(VideoReceiver *r)
{
    if (r == m_receiver)
        return;
    if (m_receiver)
        disconnect(m_receiver, nullptr, this, nullptr);
    m_receiver = r;
    if (m_receiver)
        connect(m_receiver, &VideoReceiver::frameReady, this, &VideoItem::setFrame);
    emit receiverChanged();
}

void VideoItem::setFillMode(bool on)
{
    if (on == m_fill) return;
    m_fill = on;
    emit fillModeChanged();
    update();
}

void VideoItem::setFrame(const QImage &image)
{
    if (image.isNull())
        return;
    m_pending = image; // overwrite: only the newest frame matters
    m_dirty = true;
    if (image.size() != m_sourceSize) {
        m_sourceSize = image.size();
        emit sourceSizeChanged();
    }
    if (!m_hasFrame) {
        m_hasFrame = true;
        emit hasFrameChanged();
    }
    update();
}

void VideoItem::clear()
{
    m_pending = QImage();
    m_dirty = false;
    m_clearRequested = true;
    if (m_hasFrame) {
        m_hasFrame = false;
        emit hasFrameChanged();
    }
    update();
}

QSGNode *VideoItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    auto *node = static_cast<FrameNode *>(oldNode);
    if (m_clearRequested || !m_hasFrame) {
        m_clearRequested = false;
        delete node;
        return nullptr;
    }
    if (!node) {
        node = new FrameNode;
        node->setFiltering(QSGTexture::Linear);
    }
    if (m_dirty && !m_pending.isNull()) {
        QSGTexture *tex = window()->createTextureFromImage(m_pending);
        node->replaceTexture(tex);
        m_frameSize = m_pending.size();
        m_pending = QImage();
        m_dirty = false;
    }
    if (m_frameSize.isEmpty())
        return node;

    // Aspect-correct placement.
    const QRectF bounds = boundingRect();
    QSizeF s = QSizeF(m_frameSize).scaled(bounds.size(), m_fill ? Qt::KeepAspectRatioByExpanding
                                                                : Qt::KeepAspectRatio);
    QRectF r(QPointF(bounds.center().x() - s.width() / 2, bounds.center().y() - s.height() / 2), s);
    if (m_fill) {
        // Crop via source rect instead of drawing outside the item.
        const QRectF vis = r.intersected(bounds);
        const qreal sx = m_frameSize.width() / r.width(), sy = m_frameSize.height() / r.height();
        node->setSourceRect(QRectF((vis.x() - r.x()) * sx, (vis.y() - r.y()) * sy,
                                   vis.width() * sx, vis.height() * sy));
        r = vis;
    } else {
        node->setSourceRect(QRectF(QPointF(0, 0), QSizeF(m_frameSize)));
    }
    node->setRect(r);
    return node;
}
