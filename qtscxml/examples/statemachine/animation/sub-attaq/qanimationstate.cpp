// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "qanimationstate.h"

#include <QAbstractAnimation>

QT_BEGIN_NAMESPACE

/*

The QAnimationState class provides state that handle an animation and emit
a signal when this animation is finished.

QAnimationState provides a state that handle an animation. It will start this animation
when the state is entered and stop it when it is leaved. When the animation has finished the
state emit animationFinished signal.

QStateMachine machine;
QAnimationState *s = new QAnimationState(machine->rootState());
QPropertyAnimation *animation = new QPropertyAnimation(obj, "pos");
s->setAnimation(animation);
QState *s2 = new QState(machine->rootState());
s->addTransition(s, &QAnimationState::animationFinished, s2);
machine.start();

*/

/*
  Constructs a new state with the given parent state.
*/
QAnimationState::QAnimationState(QState *parent)
    : QState(parent), m_animation(nullptr)
{
}

/*
  Destroys the animation state.
*/
QAnimationState::~QAnimationState()
{
}

/*
  Set an animation for this QAnimationState. If an animation was previously handle by this
  state then it won't emit animationFinished for the old animation. The QAnimationState doesn't
  take the ownership of the animation.
*/
void QAnimationState::setAnimation(QAbstractAnimation *animation)
{
    if (animation == m_animation)
        return;

    //Disconnect from the previous animation if exist
    if (m_animation)
        disconnect(m_animation, &QAbstractAnimation::finished, this, &QAnimationState::animationFinished);

    m_animation = animation;

    if (m_animation) {
        //connect the new animation
        connect(m_animation, &QAbstractAnimation::finished, this, &QAnimationState::animationFinished);
    }
}

/*
  Returns the animation handle by this animation state, or \nullptr if there is no animation.
*/
QAbstractAnimation* QAnimationState::animation() const
{
    return m_animation;
}

void QAnimationState::onEntry(QEvent *)
{
    if (m_animation)
        m_animation->start();
}

void QAnimationState::onExit(QEvent *)
{
    if (m_animation)
        m_animation->stop();
}

bool QAnimationState::event(QEvent *e)
{
    return QState::event(e);
}

QT_END_NAMESPACE
