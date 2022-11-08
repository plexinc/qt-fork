// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SUBMARINE_P_H
#define SUBMARINE_P_H

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
#include "animationmanager.h"
#include "submarine.h"
#include "qanimationstate.h"

//Qt
#include <QPropertyAnimation>
#include <QRandomGenerator>

//This state is describing when the boat is moving right
class MovementState : public QAnimationState
{
Q_OBJECT
public:
    explicit MovementState(SubMarine *submarine, QState *parent = nullptr) : QAnimationState(parent)
    {
        movementAnimation = new QPropertyAnimation(submarine, "pos");
        connect(movementAnimation, &QPropertyAnimation::valueChanged,
                this, &MovementState::onAnimationMovementValueChanged);
        setAnimation(movementAnimation);
        AnimationManager::self()->registerAnimation(movementAnimation);
        this->submarine = submarine;
    }

protected slots:
    void onAnimationMovementValueChanged(const QVariant &)
    {
        if (QRandomGenerator::global()->bounded(200) + 1 == 3)
            submarine->launchTorpedo(QRandomGenerator::global()->bounded(3) + 1);
    }

protected:
    void onEntry(QEvent *e) override
    {
        if (submarine->currentDirection() == SubMarine::Left) {
            movementAnimation->setEndValue(QPointF(0,submarine->y()));
            movementAnimation->setDuration(submarine->x()/submarine->currentSpeed()*12);
        }
        else /*if (submarine->currentDirection() == SubMarine::Right)*/ {
            movementAnimation->setEndValue(QPointF(submarine->scene()->width()-submarine->size().width(),submarine->y()));
            movementAnimation->setDuration((submarine->scene()->width()-submarine->size().width()-submarine->x())/submarine->currentSpeed()*12);
        }
        QAnimationState::onEntry(e);
    }

private:
    SubMarine *submarine;
    QPropertyAnimation *movementAnimation;
};

//This state is describing when the boat is moving right
class ReturnState : public QAnimationState
{
public:
    explicit ReturnState(SubMarine *submarine, QState *parent = nullptr) : QAnimationState(parent)
    {
        returnAnimation = new QPropertyAnimation(submarine->rotation(), "angle");
        returnAnimation->setDuration(500);
        AnimationManager::self()->registerAnimation(returnAnimation);
        setAnimation(returnAnimation);
        this->submarine = submarine;
    }

protected:
    void onEntry(QEvent *e) override
    {
        returnAnimation->stop();
        returnAnimation->setEndValue(submarine->currentDirection() == SubMarine::Right ? 360. : 180.);
        QAnimationState::onEntry(e);
    }

    void onExit(QEvent *e) override
    {
        submarine->currentDirection() == SubMarine::Right ? submarine->setCurrentDirection(SubMarine::Left) : submarine->setCurrentDirection(SubMarine::Right);
        QAnimationState::onExit(e);
    }

private:
    SubMarine *submarine;
    QPropertyAnimation *returnAnimation;
};

#endif // SUBMARINE_P_H
