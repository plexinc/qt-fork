// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "flippablepad.h"

#include <QDirIterator>

//! [0]
static QRectF boundsFromSize(const QSize &size)
{
    return QRectF((-size.width() / 2.0) * 150, (-size.height() / 2.0) * 150,
                  size.width() * 150, size.height() * 150);
}
//! [0]

//! [1]
static QPointF posForLocation(int column, int row, const QSize &size)
{
    return QPointF(column * 150, row * 150)
        - QPointF((size.width() - 1) * 75, (size.height() - 1) * 75);
}
//! [1]

//! [2]
FlippablePad::FlippablePad(const QSize &size, QGraphicsItem *parent)
    : RoundRectItem(boundsFromSize(size), QColor(226, 255, 92, 64), parent)
{
//! [2]
//! [3]
    int numIcons = size.width() * size.height();
    QList<QPixmap> pixmaps;
    QDirIterator it(":/images", {"*.png"});
    while (it.hasNext() && pixmaps.size() < numIcons)
        pixmaps << it.next();
//! [3]

//! [4]
    const QRectF iconRect(-54, -54, 108, 108);
    const QColor iconColor(214, 240, 110, 128);
    iconGrid.resize(size.height());
    int n = 0;

    for (int y = 0; y < size.height(); ++y) {
        iconGrid[y].resize(size.width());
        for (int x = 0; x < size.width(); ++x) {
            RoundRectItem *rect = new RoundRectItem(iconRect, iconColor, this);
            rect->setZValue(1);
            rect->setPos(posForLocation(x, y, size));
            rect->setPixmap(pixmaps.at(n++ % pixmaps.size()));
            iconGrid[y][x] = rect;
        }
    }
}
//! [4]

//! [5]
RoundRectItem *FlippablePad::iconAt(int column, int row) const
{
    return iconGrid[row][column];
}
//! [5]
