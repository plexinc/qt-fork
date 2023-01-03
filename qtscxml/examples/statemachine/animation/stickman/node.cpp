// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "node.h"
#include "stickman.h"

#include <QRectF>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

Node::Node(const QPointF &pos, QGraphicsItem *parent)
  : QGraphicsObject(parent), m_dragging(false)
{
    setPos(pos);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
}

Node::~Node()
{
}

QRectF Node::boundingRect() const
{
    return QRectF(-6.0, -6.0, 12.0, 12.0);
}

void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(Qt::white);
    painter->drawEllipse(QPointF(0.0, 0.0), 5.0, 5.0);
}

QVariant Node::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionChange)
        emit positionChanged();

    return QGraphicsObject::itemChange(change, value);
}

void Node::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    m_dragging = true;
}

void Node::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_dragging)
        setPos(mapToParent(event->pos()));
}

void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
    m_dragging = false;
}
