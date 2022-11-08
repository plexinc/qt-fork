// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef LIFECYCLE_H
#define LIFECYCLE_H

#include <Qt>

QT_BEGIN_NAMESPACE
class QAbstractState;
class QAbstractTransition;
class QAnimationGroup;
class QObject;
class QState;
class QStateMachine;
QT_END_NAMESPACE
class GraphicsView;
class StickMan;
class LifeCycle
{
public:
    LifeCycle(StickMan *stickMan, GraphicsView *keyEventReceiver);
    ~LifeCycle();

    void setDeathAnimation(const QString &fileName);
    void addActivity(const QString &fileName, Qt::Key key,
                     QObject *sender = nullptr, const char *signal = nullptr);

    void start();

private:
    QState *makeState(QState *parentState, const QString &animationFileName);

    StickMan *m_stickMan;
    QStateMachine *m_machine;
    QAnimationGroup *m_animationGroup;
    GraphicsView *m_keyReceiver;

    QState *m_alive;
    QState *m_dead;
    QState *m_idle;
};

#endif
