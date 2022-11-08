// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef ANIMATIONMANAGER_H
#define ANIMATIONMANAGER_H

#include <QObject>

QT_BEGIN_NAMESPACE
class QAbstractAnimation;
QT_END_NAMESPACE

class AnimationManager : public QObject
{
    Q_OBJECT
    AnimationManager() = default;
    ~AnimationManager() = default;
public:
    void registerAnimation(QAbstractAnimation *anim);
    void unregisterAnimation(QAbstractAnimation *anim);
    void unregisterAllAnimations();
    static AnimationManager *self();

public slots:
    void pauseAll();
    void resumeAll();

private slots:
    void unregisterAnimation_helper(QObject *obj);

private:
    QList<QAbstractAnimation *> animations;
};

#endif // ANIMATIONMANAGER_H
