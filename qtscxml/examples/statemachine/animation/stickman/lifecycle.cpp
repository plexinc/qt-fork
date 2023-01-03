// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "lifecycle.h"
#include "stickman.h"
#include "node.h"
#include "animation.h"
#include "graphicsview.h"

#include <QEventTransition>
#include <QFile>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QRandomGenerator>
#include <QSignalTransition>
#include <QState>
#include <QStateMachine>
#include <QTimer>

class KeyPressTransition: public QSignalTransition
{
public:
    KeyPressTransition(GraphicsView *receiver, Qt::Key key)
        : QSignalTransition(receiver, &GraphicsView::keyPressed), m_key(key)
    {
    }
    KeyPressTransition(GraphicsView *receiver, Qt::Key key, QAbstractState *target)
        : QSignalTransition(receiver, &GraphicsView::keyPressed), m_key(key)
    {
        setTargetState(target);
    }

    bool eventTest(QEvent *e) override
    {
        if (QSignalTransition::eventTest(e)) {
            QVariant key = static_cast<QStateMachine::SignalEvent*>(e)->arguments().at(0);
            return (key.toInt() == int(m_key));
        }

        return false;
    }
private:
    Qt::Key m_key;
};

//! [4]
class LightningStrikesTransition: public QEventTransition
{
public:
    LightningStrikesTransition(QAbstractState *target)
        : QEventTransition(this, QEvent::Timer)
    {
        setTargetState(target);
        startTimer(1000);
    }

    bool eventTest(QEvent *e) override
    {
        return QEventTransition::eventTest(e) && QRandomGenerator::global()->bounded(50) == 0;
    }
};
//! [4]

LifeCycle::LifeCycle(StickMan *stickMan, GraphicsView *keyReceiver)
    : m_stickMan(stickMan), m_keyReceiver(keyReceiver)
{
    // Create animation group to be used for all transitions
    m_animationGroup = new QParallelAnimationGroup();
    const int stickManNodeCount = m_stickMan->nodeCount();
    for (int i = 0; i < stickManNodeCount; ++i) {
        QPropertyAnimation *pa = new QPropertyAnimation(m_stickMan->node(i), "pos");
        m_animationGroup->addAnimation(pa);
    }

    // Set up initial state graph
//! [3]
    m_machine = new QStateMachine();
    m_machine->addDefaultAnimation(m_animationGroup);
//! [3]

    m_alive = new QState(m_machine);
    m_alive->setObjectName("alive");

    // Make it blink when lightning strikes before entering dead animation
    QState *lightningBlink = new QState(m_machine);
    lightningBlink->assignProperty(m_stickMan->scene(), "backgroundBrush", QColor(Qt::white));
    lightningBlink->assignProperty(m_stickMan, "penColor", QColor(Qt::black));
    lightningBlink->assignProperty(m_stickMan, "fillColor", QColor(Qt::white));
    lightningBlink->assignProperty(m_stickMan, "isDead", true);

//! [5]
    QTimer *timer = new QTimer(lightningBlink);
    timer->setSingleShot(true);
    timer->setInterval(100);
    QObject::connect(lightningBlink, &QAbstractState::entered,
                     timer, QOverload<>::of(&QTimer::start));
    QObject::connect(lightningBlink, &QAbstractState::exited,
                     timer, &QTimer::stop);
//! [5]

    m_dead = new QState(m_machine);
    m_dead->assignProperty(m_stickMan->scene(), "backgroundBrush", QColor(Qt::black));
    m_dead->assignProperty(m_stickMan, "penColor", QColor(Qt::white));
    m_dead->assignProperty(m_stickMan, "fillColor", QColor(Qt::black));
    m_dead->setObjectName("dead");

    // Idle state (sets no properties)
    m_idle = new QState(m_alive);
    m_idle->setObjectName("idle");

    m_alive->setInitialState(m_idle);

    // Lightning strikes at random
    m_alive->addTransition(new LightningStrikesTransition(lightningBlink));
//! [0]
    lightningBlink->addTransition(timer, &QTimer::timeout, m_dead);
//! [0]

    m_machine->setInitialState(m_alive);
}

void LifeCycle::setDeathAnimation(const QString &fileName)
{
    QState *deathAnimation = makeState(m_dead, fileName);
    m_dead->setInitialState(deathAnimation);
}

void LifeCycle::start()
{
    m_machine->start();
}

void LifeCycle::addActivity(const QString &fileName, Qt::Key key, QObject *sender, const char *signal)
{
    QState *state = makeState(m_alive, fileName);
    m_alive->addTransition(new KeyPressTransition(m_keyReceiver, key, state));

    if (sender && signal)
        m_alive->addTransition(sender, signal, state);
}

QState *LifeCycle::makeState(QState *parentState, const QString &animationFileName)
{
    QState *topLevel = new QState(parentState);

    Animation animation;
    {
        QFile file(animationFileName);
        if (file.open(QIODevice::ReadOnly))
            animation.load(&file);
    }

    const int frameCount = animation.totalFrames();
    QState *previousState = nullptr;
    for (int i = 0; i < frameCount; ++i) {
        animation.setCurrentFrame(i);

//! [1]
        QState *frameState = new QState(topLevel);
        const int nodeCount = animation.nodeCount();
        for (int j = 0; j < nodeCount; ++j)
            frameState->assignProperty(m_stickMan->node(j), "pos", animation.nodePos(j));
//! [1]

        frameState->setObjectName(QString::fromLatin1("frame %0").arg(i));
        if (previousState == nullptr)
            topLevel->setInitialState(frameState);
        else
//! [2]
            previousState->addTransition(previousState, &QState::propertiesAssigned, frameState);
//! [2]

        previousState = frameState;
    }

    // Loop
    previousState->addTransition(previousState, &QState::propertiesAssigned, topLevel->initialState());

    return topLevel;

}

LifeCycle::~LifeCycle()
{
    delete m_machine;
    delete m_animationGroup;
}
