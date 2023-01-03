// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "roundrectitem.h"

#include <QGuiApplication>
#include <QPainter>
#include <QPalette>

//! [0]
RoundRectItem::RoundRectItem(const QRectF &bounds, const QColor &color,
                             QGraphicsItem *parent)
    : QGraphicsObject(parent), fillRect(false), bounds(bounds)
{
    gradient.setStart(bounds.topLeft());
    gradient.setFinalStop(bounds.bottomRight());
    gradient.setColorAt(0, color);
    gradient.setColorAt(1, color.darker(200));
    setCacheMode(ItemCoordinateCache);
}
//! [0]

//! [1]
QPixmap RoundRectItem::pixmap() const
{
    return pix;
}
void RoundRectItem::setPixmap(const QPixmap &pixmap)
{
    pix = pixmap;
    update();
}
//! [1]

//! [2]
QRectF RoundRectItem::boundingRect() const
{
    return bounds.adjusted(0, 0, 2, 2);
}
//! [2]

//! [3]
void RoundRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                          QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 0, 64));
    painter->drawRoundedRect(bounds.translated(2, 2), 25, 25, Qt::RelativeSize);
//! [3]
//! [4]
    if (fillRect)
        painter->setBrush(QGuiApplication::palette().brush(QPalette::Window));
    else
        painter->setBrush(gradient);
    painter->setPen(QPen(Qt::black, 1));
    painter->drawRoundedRect(bounds, 25,25, Qt::RelativeSize);
//! [4]
//! [5]
    if (!pix.isNull()) {
        painter->scale(1.95, 1.95);
        painter->drawPixmap(-pix.width() / 2, -pix.height() / 2, pix);
    }
}
//! [5]

//! [6]
bool RoundRectItem::fill() const
{
    return fillRect;
}
void RoundRectItem::setFill(bool fill)
{
    fillRect = fill;
    update();
}
//! [6]
