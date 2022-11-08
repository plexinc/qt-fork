// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SUBMARINE_H
#define SUBMARINE_H

#include "pixmapitem.h"

#include <QGraphicsRotation>

class SubMarine : public PixmapItem
{
Q_OBJECT
public:
    enum Movement {
       None = 0,
       Left,
       Right
    };
    enum { Type = UserType + 1 };
    SubMarine(int type, const QString &name, int points);

    int points() const;

    void setCurrentDirection(Movement direction);
    enum Movement currentDirection() const;

    void setCurrentSpeed(int speed);
    int currentSpeed() const;

    void launchTorpedo(int speed);
    void destroy();

    int type() const override;

    QGraphicsRotation *rotation() const { return graphicsRotation; }

signals:
    void subMarineDestroyed();
    void subMarineExecutionFinished();
    void subMarineStateChanged();

private:
    int subType;
    QString subName;
    int subPoints;
    int speed;
    Movement direction;
    QGraphicsRotation *graphicsRotation;
};

#endif // SUBMARINE_H
