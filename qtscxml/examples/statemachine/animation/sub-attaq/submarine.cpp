// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//Own
#include "submarine.h"
#include "submarine_p.h"
#include "torpedo.h"
#include "graphicsscene.h"
#include "animationmanager.h"
#include "qanimationstate.h"

#include <QFinalState>
#include <QPropertyAnimation>
#include <QStateMachine>
#include <QSequentialAnimationGroup>

static QAbstractAnimation *setupDestroyAnimation(SubMarine *sub)
{
    QSequentialAnimationGroup *group = new QSequentialAnimationGroup(sub);
    for (int i = 1; i <= 4; ++i) {
        PixmapItem *step = new PixmapItem(QString::fromLatin1("explosion/submarine/step%1").arg(i), GraphicsScene::Big, sub);
        step->setZValue(6);
        step->setOpacity(0);
        QPropertyAnimation *anim = new QPropertyAnimation(step, "opacity", group);
        anim->setDuration(100);
        anim->setEndValue(1);
    }
    AnimationManager::self()->registerAnimation(group);
    return group;
}


SubMarine::SubMarine(int type, const QString &name, int points) : PixmapItem(QString("submarine"), GraphicsScene::Big),
    subType(type), subName(name), subPoints(points), speed(0), direction(SubMarine::None)
{
    setZValue(5);
    setTransformOriginPoint(boundingRect().center());

    graphicsRotation = new QGraphicsRotation(this);
    graphicsRotation->setAxis(Qt::YAxis);
    graphicsRotation->setOrigin(QVector3D(size().width() / 2, size().height() / 2, 0));
    QList<QGraphicsTransform *> r({graphicsRotation});
    setTransformations(r);

    //We setup the state machine of the submarine
    QStateMachine *machine = new QStateMachine(this);

    //This state is when the boat is moving/rotating
    QState *moving = new QState(machine);

    //This state is when the boat is moving from left to right
    MovementState *movement = new MovementState(this, moving);

    //This state is when the boat is moving from left to right
    ReturnState *rotation = new ReturnState(this, moving);

    //This is the initial state of the moving root state
    moving->setInitialState(movement);

    movement->addTransition(this, &SubMarine::subMarineStateChanged, moving);

    //This is the initial state of the machine
    machine->setInitialState(moving);

    //End
    QFinalState *finalState = new QFinalState(machine);

    //If the moving animation is finished we move to the return state
    movement->addTransition(movement, &QAnimationState::animationFinished, rotation);

    //If the return animation is finished we move to the moving state
    rotation->addTransition(rotation, &QAnimationState::animationFinished, movement);

    //This state play the destroyed animation
    QAnimationState *destroyedState = new QAnimationState(machine);
    destroyedState->setAnimation(setupDestroyAnimation(this));

    //Play a nice animation when the submarine is destroyed
    moving->addTransition(this, &SubMarine::subMarineDestroyed, destroyedState);

    //Transition to final state when the destroyed animation is finished
    destroyedState->addTransition(destroyedState, &QAnimationState::animationFinished, finalState);

    //The machine has finished to be executed, then the submarine is dead
    connect(machine,&QState::finished,this, &SubMarine::subMarineExecutionFinished);

    machine->start();
}

int SubMarine::points() const
{
    return subPoints;
}

void SubMarine::setCurrentDirection(SubMarine::Movement direction)
{
    if (this->direction == direction)
        return;
    if (direction == SubMarine::Right && this->direction == SubMarine::None)
          graphicsRotation->setAngle(180);
    this->direction = direction;
}

enum SubMarine::Movement SubMarine::currentDirection() const
{
    return direction;
}

void SubMarine::setCurrentSpeed(int speed)
{
    if (speed < 0 || speed > 3)
        qWarning("SubMarine::setCurrentSpeed : The speed is invalid");
    this->speed = speed;
    emit subMarineStateChanged();
}

int SubMarine::currentSpeed() const
{
    return speed;
}

void SubMarine::launchTorpedo(int speed)
{
    Torpedo *torp = new Torpedo;
    GraphicsScene *scene = static_cast<GraphicsScene *>(this->scene());
    scene->addItem(torp);
    torp->setPos(pos());
    torp->setCurrentSpeed(speed);
    torp->launch();
}

void SubMarine::destroy()
{
    emit subMarineDestroyed();
}

int SubMarine::type() const
{
    return Type;
}
