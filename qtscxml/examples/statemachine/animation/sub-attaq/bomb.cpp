// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//Own
#include "bomb.h"
#include "submarine.h"
#include "animationmanager.h"
#include "qanimationstate.h"

//Qt
#include <QFinalState>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QStateMachine>

Bomb::Bomb() : PixmapItem(QString("bomb"), GraphicsScene::Big)
{
    setZValue(2);
}

void Bomb::launch(Bomb::Direction direction)
{
    QSequentialAnimationGroup *launchAnimation = new QSequentialAnimationGroup;
    AnimationManager::self()->registerAnimation(launchAnimation);
    qreal delta = direction == Right ? 20 : - 20;
    QPropertyAnimation *anim = new QPropertyAnimation(this, "pos");
    anim->setEndValue(QPointF(x() + delta,y() - 20));
    anim->setDuration(150);
    launchAnimation->addAnimation(anim);
    anim = new QPropertyAnimation(this, "pos");
    anim->setEndValue(QPointF(x() + delta*2, y() ));
    anim->setDuration(150);
    launchAnimation->addAnimation(anim);
    anim = new QPropertyAnimation(this, "pos");
    anim->setEndValue(QPointF(x() + delta*2,scene()->height()));
    anim->setDuration(y()/2*60);
    launchAnimation->addAnimation(anim);
    connect(anim, &QVariantAnimation::valueChanged, this, &Bomb::onAnimationLaunchValueChanged);
    connect(this, &Bomb::bombExploded, launchAnimation, &QAbstractAnimation::stop);
    //We setup the state machine of the bomb
    QStateMachine *machine = new QStateMachine(this);

    //This state is when the launch animation is playing
    QAnimationState *launched = new QAnimationState(machine);
    launched->setAnimation(launchAnimation);

    //End
    QFinalState *finalState = new QFinalState(machine);

    machine->setInitialState(launched);

    //### Add a nice animation when the bomb is destroyed
    launched->addTransition(this, &Bomb::bombExploded, finalState);

    //If the animation is finished, then we move to the final state
    launched->addTransition(launched, &QAnimationState::animationFinished, finalState);

    //The machine has finished to be executed, then the boat is dead
    connect(machine,&QState::finished, this, &Bomb::bombExecutionFinished);

    machine->start();

}

void Bomb::onAnimationLaunchValueChanged(const QVariant &)
{
    const QList<QGraphicsItem *> colItems =
            collidingItems(Qt::IntersectsItemBoundingRect);
    for (QGraphicsItem *item : colItems) {
        if (item->type() == SubMarine::Type) {
            SubMarine *s = static_cast<SubMarine *>(item);
            destroy();
            s->destroy();
        }
    }
}

void Bomb::destroy()
{
    emit bombExploded();
}
