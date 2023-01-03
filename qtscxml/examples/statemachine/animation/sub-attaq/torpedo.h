// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TORPEDO_H
#define TORPEDO_H

#include "pixmapitem.h"

class Torpedo : public PixmapItem
{
Q_OBJECT
public:
    Torpedo();
    void launch();
    void setCurrentSpeed(int speed);
    void destroy();

signals:
    void torpedoExploded();
    void torpedoExecutionFinished();

private slots:
    void onAnimationLaunchValueChanged(const QVariant &);

private:
    int currentSpeed;
};

#endif // TORPEDO_H
