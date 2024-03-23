#include <QPen>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>

#include "ResizableRect.h"

//----------------------------------------
const int ResizableRect::THICKNESS = 4;
const int ResizableRect::THICKNESS_DOUBLE
= ResizableRect::THICKNESS * 2;
//----------------------------------------
ResizableRect::ResizableRect(
        const QColor &color,
        const QSize &sizeRatio,
        QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
{
    setPen(QPen(color, THICKNESS));
    m_sizeRatio = sizeRatio;
    m_resizing = false;
    m_moving = false;
    m_callbackOnResized = [](const QPointF &, const QRectF &){};
}
//----------------------------------------
void ResizableRect::setSizeRatio(const QSize &sizeRatio)
{
    m_sizeRatio = sizeRatio;
    if (m_sizeRatio.isValid()) {
        auto oldRect = rect();
        _adaptRatioRectWidth(oldRect);
        setRect(oldRect);
        m_callbackOnResized(pos(), oldRect);
    }
}
//----------------------------------------
void ResizableRect::_initMoveBools()
{
    m_moveTopLeft = false;
    m_moveTopRight = false;
    m_moveBottomLeft = false;
    m_moveBottomRight = false;
    m_moveBottom = false;
    m_moveTop = false;
    m_moveLeft = false;
    m_moveRight = false;
}
//----------------------------------------
void ResizableRect::mousePressEvent(
        QGraphicsSceneMouseEvent *event)
{
    QGraphicsRectItem::mousePressEvent(event);
    if (event->button() == Qt::LeftButton) {
        event->accept();
        /*
        if (event->modifiers() & Qt::ControlModifier) {
            m_moving = true;
            m_resizing = false;
            m_pointMovingStarted = event->pos();
            setCursor(Qt::DragMoveCursor);
        } else {
        //*/
            m_moving = false;
            m_resizing = true;
            QPointF pos = event->scenePos();
            QRectF rect = boundingRect();
            QPointF topLeft = mapToScene(rect.topLeft());
            QPointF bottomRight = mapToScene(rect.bottomRight());
            double minX = qMin(qAbs(pos.x() - topLeft.x()),
                               qAbs(pos.x() - bottomRight.x()));
            double minY = qMin(qAbs(pos.y() - topLeft.y()),
                               qAbs(pos.y() - bottomRight.y()));
            //if (qAbs(pos.x() - topLeft.x()) < THICKNESS)
            _initMoveBools();
            if (minX <= THICKNESS_DOUBLE && minY <= THICKNESS_DOUBLE) {
                if (minX == qAbs(pos.x() - topLeft.x())) {
                    if (minY == qAbs(pos.y() - topLeft.y())) {
                        m_moveTopLeft = true;
                    } else {
                        m_moveBottomLeft = true;
                    }
                } else {
                    if (minY == qAbs(pos.y() - topLeft.y())) {
                        m_moveTopRight = true;
                    } else {
                        m_moveBottomRight = true;
                    }
                }
                setCursor(Qt::SizeFDiagCursor);
            } else if (minX <= THICKNESS_DOUBLE) {
                if (minX == qAbs(pos.x() - topLeft.x())) {
                    m_moveLeft = true;
                } else {
                    m_moveRight = true;
                }
                setCursor(Qt::SizeHorCursor);
            } else if (minY <= THICKNESS_DOUBLE) {
                if (minY == qAbs(pos.y() - topLeft.y())) {
                    m_moveTop = true;
                } else {
                    m_moveBottom = true;
                }
                setCursor(Qt::SizeVerCursor);
            } else {
                m_moving = true;
                m_resizing = false;
                m_pointMovingStarted = event->pos();
                setCursor(Qt::DragMoveCursor);
                //setCursor(Qt::ArrowCursor);
            }
        //}
    }
    //setCursor(Qt::SizeFDiagCursor);
}
//----------------------------------------
void ResizableRect::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_resizing) {
        QPointF newPos = event->pos();
        auto oldRect = rect();
        if (m_sizeRatio.isValid()) {
            if (m_moveBottomLeft) {
                oldRect.setBottomLeft(newPos);
                _adaptRatioRectWidth(oldRect);
            } else if (m_moveBottomRight) {
                oldRect.setBottomRight(newPos);
                _adaptRatioRectWidth(oldRect);
            } else if (m_moveTopLeft) {
                oldRect.setTopLeft(newPos);
                _adaptRatioRectWidth(oldRect);
            } else if (m_moveTopRight) {
                oldRect.setTopRight(newPos);
                _adaptRatioRectWidth(oldRect);
            } else if (m_moveBottom) {
                oldRect.setBottom(newPos.y());
                _adaptRatioRectWidth(oldRect);
            } else if (m_moveTop) {
                oldRect.setTop(newPos.y());
                _adaptRatioRectWidth(oldRect);
            } else if (m_moveLeft) {
                oldRect.setLeft(newPos.x());
                _adaptRatioRectHeight(oldRect);
            } else if (m_moveRight) {
                oldRect.setRight(newPos.x());
                _adaptRatioRectHeight(oldRect);
            }
        } else {
            if (m_moveBottomLeft) {
                oldRect.setBottomLeft(newPos);
            } else if (m_moveBottomRight) {
                oldRect.setBottomRight(newPos);
            } else if (m_moveTopLeft) {
                oldRect.setTopLeft(newPos);
            } else if (m_moveTopRight) {
                oldRect.setTopRight(newPos);
            } else if (m_moveBottom) {
                oldRect.setBottom(newPos.y());
            } else if (m_moveTop) {
                oldRect.setTop(newPos.y());
            } else if (m_moveLeft) {
                oldRect.setLeft(newPos.x());
            } else if (m_moveRight) {
                oldRect.setRight(newPos.x());
            }
        }
        setRect(oldRect);
        m_callbackOnResized(pos(), oldRect);
    } else if (m_moving) {
        QPointF newPos = event->pos();
        double newX = x() + newPos.x() - m_pointMovingStarted.x();
        double newY = y() + newPos.y() - m_pointMovingStarted.y();
        setPos(newX, newY);
        m_callbackOnResized(pos(), rect());
    }
    /*
    if (event->buttons() & Qt::LeftButton)
    {
    }
    //*/
    QGraphicsRectItem::mouseMoveEvent(event);
}
//----------------------------------------
void ResizableRect::_adaptRatioRectWidth(QRectF &rect)
{
    double width = m_sizeRatio.width() * rect.height() / m_sizeRatio.height();
    rect.setWidth(width);
}
//----------------------------------------
void ResizableRect::_adaptRatioRectHeight(QRectF &rect)
{
    double height = rect.width()
            * m_sizeRatio.height() / m_sizeRatio.width();
    rect.setHeight(height);
}
//----------------------------------------
void ResizableRect::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsRectItem::mouseReleaseEvent(event);
    _initMoveBools();
    m_resizing = false;
    m_moving = false;
    setCursor(Qt::ArrowCursor);
}
//----------------------------------------
void ResizableRect::setCallbackOnResized(
        std::function<void (const QPointF &, const QRectF &)> callback)
{
    m_callbackOnResized = callback;
}
//----------------------------------------
