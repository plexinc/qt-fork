// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCore>
#include <QtStateMachine>

#include <iostream>

//! [0]
class PingEvent : public QEvent
{
public:
    PingEvent() : QEvent(QEvent::Type(QEvent::User+2))
        {}
};

class PongEvent : public QEvent
{
public:
    PongEvent() : QEvent(QEvent::Type(QEvent::User+3))
        {}
};
//! [0]

//! [1]
class Pinger : public QState
{
public:
    Pinger(QState *parent)
        : QState(parent) {}

protected:
    void onEntry(QEvent *) override
    {
        machine()->postEvent(new PingEvent());
        std::cout << "ping?" << std::endl;
    }
};
//! [1]

//! [3]
class PongTransition : public QAbstractTransition
{
public:
    PongTransition() {}

protected:
    bool eventTest(QEvent *e) override {
        return (e->type() == QEvent::User+3);
    }
    void onTransition(QEvent *) override
    {
        machine()->postDelayedEvent(new PingEvent(), 500);
        std::cout << "ping?" << std::endl;
    }
};
//! [3]

//! [2]
class PingTransition : public QAbstractTransition
{
public:
    PingTransition() {}

protected:
    bool eventTest(QEvent *e) override {
        return (e->type() == QEvent::User+2);
    }
    void onTransition(QEvent *) override
    {
        machine()->postDelayedEvent(new PongEvent(), 500);
        std::cout << "pong!" << std::endl;
    }
};
//! [2]

//! [4]
int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QStateMachine machine;
    QState *group = new QState(QState::ParallelStates);
    group->setObjectName("group");
//! [4]

//! [5]
    Pinger *pinger = new Pinger(group);
    pinger->setObjectName("pinger");
    pinger->addTransition(new PongTransition());

    QState *ponger = new QState(group);
    ponger->setObjectName("ponger");
    ponger->addTransition(new PingTransition());
//! [5]

//! [6]
    machine.addState(group);
    machine.setInitialState(group);
    machine.start();

    return app.exec();
}
//! [6]
