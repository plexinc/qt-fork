// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//Own
#include "animationmanager.h"

#include <QAbstractAnimation>

AnimationManager *AnimationManager::self()
{
    // the universe's only animation manager
    static AnimationManager s_instance;
    return &s_instance;
}

void AnimationManager::registerAnimation(QAbstractAnimation *anim)
{
    QObject::connect(anim, &QObject::destroyed, this, &AnimationManager::unregisterAnimation_helper);
    animations.append(anim);
}

void AnimationManager::unregisterAnimation_helper(QObject *obj)
{
    unregisterAnimation(static_cast<QAbstractAnimation*>(obj));
}

void AnimationManager::unregisterAnimation(QAbstractAnimation *anim)
{
    QObject::disconnect(anim, &QObject::destroyed, this, &AnimationManager::unregisterAnimation_helper);
    animations.removeAll(anim);
}

void AnimationManager::unregisterAllAnimations()
{
    animations.clear();
}

void AnimationManager::pauseAll()
{
    for (QAbstractAnimation *animation : qAsConst(animations)) {
        if (animation->state() == QAbstractAnimation::Running)
            animation->pause();
    }
}
void AnimationManager::resumeAll()
{
    for (QAbstractAnimation *animation : qAsConst(animations)) {
        if (animation->state() == QAbstractAnimation::Paused)
            animation->resume();
    }
}
