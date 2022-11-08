// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PIXMAPITEM_H
#define PIXMAPITEM_H

//Own
#include "graphicsscene.h"

//Qt
#include <QGraphicsObject>

class PixmapItem : public QGraphicsObject
{
public:
    PixmapItem(const QString &fileName, GraphicsScene::Mode mode, QGraphicsItem *parent = nullptr);
    PixmapItem(const QString &fileName, QGraphicsScene *scene);
    QSizeF size() const;
    QRectF boundingRect() const override;
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;
private:
    QPixmap pix;
};

#endif // PIXMAPITEM_H
