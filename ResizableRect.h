#ifndef RESIZABLERECT_H
#define RESIZABLERECT_H

#include <QGraphicsRectItem>

class ResizableRect : public QGraphicsRectItem
{
public:
    static const int THICKNESS;
    static const int THICKNESS_DOUBLE;
    ResizableRect(
            const QColor &color,
            const QSize &sizeRatio = QSize(),
            QGraphicsItem *parent = nullptr);
    void setSizeRatio(const QSize &sizeRatio);

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    void setCallbackOnResized(
            std::function<void(const QPointF &pos, const QRectF &)> callback);


private:
    bool m_resizing;
    bool m_moving;
    bool m_moveTopLeft;
    bool m_moveTopRight;
    bool m_moveBottomLeft;
    bool m_moveBottomRight;
    bool m_moveBottom;
    bool m_moveTop;
    bool m_moveLeft;
    bool m_moveRight;
    QSize m_sizeRatio;
    QPointF m_pointMovingStarted;
    void _initMoveBools();
    void _adaptRatioRectWidth(QRectF &rect);
    void _adaptRatioRectHeight(QRectF &rect);
    std::function<void(const QPointF &, const QRectF &)> m_callbackOnResized;
};

#endif // RESIZABLERECT_H
