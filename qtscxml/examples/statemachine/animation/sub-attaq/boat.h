// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef BOAT_H
#define BOAT_H

#include "pixmapitem.h"

QT_BEGIN_NAMESPACE
class QVariantAnimation;
class QAbstractAnimation;
class QStateMachine;
QT_END_NAMESPACE

class Boat : public PixmapItem
{
Q_OBJECT
public:
    enum Movement {
       None = 0,
       Left,
       Right
    };
    enum { Type = UserType + 2 };
    Boat();
    void destroy();
    void run();
    void stop();

    int bombsLaunched() const;
    void setBombsLaunched(int number);

    int currentSpeed() const;
    void setCurrentSpeed(int speed);

    enum Movement currentDirection() const;
    void setCurrentDirection(Movement direction);

    void updateBoatMovement();

    int type() const override;

signals:
    void boatDestroyed();
    void boatExecutionFinished();

private:
    int speed;
    int bombsAlreadyLaunched;
    Movement direction;
    QVariantAnimation *movementAnimation;
    QAbstractAnimation *destroyAnimation;
    QStateMachine *machine;
};

#endif // BOAT_H
