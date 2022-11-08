// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//Own
#include "pixmapitem.h"

//Qt
#include <QPainter>

PixmapItem::PixmapItem(const QString &fileName, GraphicsScene::Mode mode, QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    if (mode == GraphicsScene::Big)
        pix  = QPixmap(QStringLiteral(":/big/") + fileName);
    else
        pix = QPixmap(QStringLiteral(":/small/") + fileName);
}

PixmapItem::PixmapItem(const QString &fileName, QGraphicsScene *scene)
    : QGraphicsObject(), pix(fileName)
{
    scene->addItem(this);
}

QSizeF PixmapItem::size() const
{
    return pix.size();
}

QRectF PixmapItem::boundingRect() const
{
    return QRectF(QPointF(0, 0), pix.size());
}

void PixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->drawPixmap(0, 0, pix);
}


