// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef BOMB_H
#define BOMB_H

#include "pixmapitem.h"

class Bomb : public PixmapItem
{
Q_OBJECT
public:
    enum Direction {
        Left =  0,
        Right
    };
    Bomb();
    void launch(Direction direction);
    void destroy();

signals:
    void bombExploded();
    void bombExecutionFinished();

private slots:
    void onAnimationLaunchValueChanged(const QVariant &);
};

#endif // BOMB_H
