// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//Own
#include "torpedo.h"
#include "boat.h"
#include "graphicsscene.h"
#include "animationmanager.h"
#include "qanimationstate.h"

#include <QPropertyAnimation>
#include <QStateMachine>
#include <QFinalState>

Torpedo::Torpedo() : PixmapItem(QString::fromLatin1("torpedo"),GraphicsScene::Big),
    currentSpeed(0)
{
    setZValue(2);
}

void Torpedo::launch()
{
    QPropertyAnimation *launchAnimation = new QPropertyAnimation(this, "pos");
    AnimationManager::self()->registerAnimation(launchAnimation);
    launchAnimation->setEndValue(QPointF(x(), qobject_cast<GraphicsScene *>(scene())->sealLevel() - 15));
    launchAnimation->setEasingCurve(QEasingCurve::InQuad);
    launchAnimation->setDuration(y() / currentSpeed * 10);
    connect(launchAnimation, &QVariantAnimation::valueChanged, this, &Torpedo::onAnimationLaunchValueChanged);
    connect(this, &Torpedo::torpedoExploded, launchAnimation, &QAbstractAnimation::stop);

    //We setup the state machine of the torpedo
    QStateMachine *machine = new QStateMachine(this);

    //This state is when the launch animation is playing
    QAnimationState *launched = new QAnimationState(machine);
    launched->setAnimation(launchAnimation);

    //End
    QFinalState *finalState = new QFinalState(machine);

    machine->setInitialState(launched);

    //### Add a nice animation when the torpedo is destroyed
    launched->addTransition(this, &Torpedo::torpedoExploded, finalState);

    //If the animation is finished, then we move to the final state
    launched->addTransition(launched, &QAnimationState::animationFinished, finalState);

    //The machine has finished to be executed, then the boat is dead
    connect(machine, &QState::finished, this, &Torpedo::torpedoExecutionFinished);

    machine->start();
}

void Torpedo::setCurrentSpeed(int speed)
{
    if (speed < 0) {
        qWarning("Torpedo::setCurrentSpeed : The speed is invalid");
        return;
    }
    currentSpeed = speed;
}

void Torpedo::onAnimationLaunchValueChanged(const QVariant &)
{
    const QList<QGraphicsItem *> colItems =
            collidingItems(Qt::IntersectsItemBoundingRect);
    for (QGraphicsItem *item : colItems) {
        if (Boat *b = qgraphicsitem_cast<Boat*>(item))
            b->destroy();
    }
}

void Torpedo::destroy()
{
    emit torpedoExploded();
}
