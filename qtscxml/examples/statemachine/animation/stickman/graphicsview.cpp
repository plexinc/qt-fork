// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "graphicsview.h"
#include "stickman.h"

#include <QKeyEvent>
#include <QGraphicsScene>

void GraphicsView::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape)
        close();
    emit keyPressed(Qt::Key(e->key()));
}

void GraphicsView::resizeEvent(QResizeEvent *e)
{
    fitInView(scene()->sceneRect());
    QGraphicsView::resizeEvent(e);
}
