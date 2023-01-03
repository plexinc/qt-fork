// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef BOAT_P_H
#define BOAT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

//Own
#include "bomb.h"
#include "graphicsscene.h"

// Qt
#include <QGraphicsRotation>
#include <QKeyEventTransition>
#include <QState>

static const int MAX_BOMB = 5;


//These transtion test if we have to stop the boat (i.e current speed is 1)
class KeyStopTransition : public QKeyEventTransition
{
public:
    KeyStopTransition(Boat *b, QEvent::Type t, int k)
    : QKeyEventTransition(b, t, k), boat(b)
    {
    }
protected:
    bool eventTest(QEvent *event) override
    {
        if (!QKeyEventTransition::eventTest(event))
            return false;
        return (boat->currentSpeed() == 1);
    }
private:
    Boat *boat;
};

//These transtion test if we have to move the boat (i.e current speed was 0 or another value)
 class KeyMoveTransition : public QKeyEventTransition
{
public:
    KeyMoveTransition(Boat *b, QEvent::Type t, int k)
    : QKeyEventTransition(b, t, k), boat(b), key(k)
    {
    }
protected:
    bool eventTest(QEvent *event) override
    {
        if (!QKeyEventTransition::eventTest(event))
            return false;
        return (boat->currentSpeed() >= 0);
    }
    void onTransition(QEvent *) override
    {
        //We decrease the speed if needed
        if (key == Qt::Key_Left && boat->currentDirection() == Boat::Right)
            boat->setCurrentSpeed(boat->currentSpeed() - 1);
        else if (key == Qt::Key_Right && boat->currentDirection() == Boat::Left)
            boat->setCurrentSpeed(boat->currentSpeed() - 1);
        else if (boat->currentSpeed() < 3)
            boat->setCurrentSpeed(boat->currentSpeed() + 1);
        boat->updateBoatMovement();
    }
private:
    Boat *boat;
    int key;
};

//This transition trigger the bombs launch
 class KeyLaunchTransition : public QKeyEventTransition
{
public:
    KeyLaunchTransition(Boat *boat, QEvent::Type type, int key)
    : QKeyEventTransition(boat, type, key), boat(boat)
    {
    }
protected:
    bool eventTest(QEvent *event) override
    {
        if (!QKeyEventTransition::eventTest(event))
            return false;
        //We have enough bomb?
        return (boat->bombsLaunched() < MAX_BOMB);
    }
private:
    Boat *boat;
};

//This state is describing when the boat is moving right
class MoveStateRight : public QState
{
public:
    explicit MoveStateRight(Boat *boat, QState *parent = nullptr)
        : QState(parent), boat(boat)
    {
    }
protected:
    void onEntry(QEvent *) override
    {
        boat->setCurrentDirection(Boat::Right);
        boat->updateBoatMovement();
    }
private:
    Boat *boat;
};

 //This state is describing when the boat is moving left
class MoveStateLeft : public QState
{
public:
    explicit MoveStateLeft(Boat *boat, QState *parent = nullptr)
        : QState(parent), boat(boat)
    {
    }
protected:
    void onEntry(QEvent *) override
    {
        boat->setCurrentDirection(Boat::Left);
        boat->updateBoatMovement();
    }
private:
    Boat *boat;
};

//This state is describing when the boat is in a stand by position
class StopState : public QState
{
public:
    explicit StopState(Boat *boat, QState *parent = nullptr)
        : QState(parent), boat(boat)
    {
    }
protected:
    void onEntry(QEvent *) override
    {
        boat->setCurrentSpeed(0);
        boat->setCurrentDirection(Boat::None);
        boat->updateBoatMovement();
    }
private:
    Boat *boat;
};

//This state is describing the launch of the torpedo on the right
class LaunchStateRight : public QState
{
public:
    explicit LaunchStateRight(Boat *boat, QState *parent = nullptr)
        : QState(parent), boat(boat)
    {
    }
protected:
    void onEntry(QEvent *) override
    {
        Bomb *b = new Bomb;
        b->setPos(boat->x()+boat->size().width(),boat->y());
        GraphicsScene *scene = static_cast<GraphicsScene *>(boat->scene());
        scene->addItem(b);
        b->launch(Bomb::Right);
        boat->setBombsLaunched(boat->bombsLaunched() + 1);
    }
private:
    Boat *boat;
};

//This state is describing the launch of the torpedo on the left
class LaunchStateLeft : public QState
{
public:
    explicit LaunchStateLeft(Boat *boat, QState *parent = nullptr)
        : QState(parent), boat(boat)
    {
    }
protected:
    void onEntry(QEvent *) override
    {
        Bomb *b = new Bomb;
        b->setPos(boat->x() - b->size().width(), boat->y());
        GraphicsScene *scene = static_cast<GraphicsScene *>(boat->scene());
        scene->addItem(b);
        b->launch(Bomb::Left);
        boat->setBombsLaunched(boat->bombsLaunched() + 1);
    }
private:
    Boat *boat;
};

#endif // BOAT_P_H
