// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCore>
#include <QtStateMachine>

//! [0]
class Factorial : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int x READ x WRITE setX)
    Q_PROPERTY(int fac READ fac WRITE setFac)
public:
    using QObject::QObject;

    int x() const
    {
        return m_x;
    }

    void setX(int x)
    {
        if (x == m_x)
            return;
        m_x = x;
        emit xChanged(x);
    }

    int fac() const
    {
        return m_fac;
    }

    void setFac(int fac)
    {
        m_fac = fac;
    }

Q_SIGNALS:
    void xChanged(int value);

private:
    int m_x = -1;
    int m_fac = 1;
};
//! [0]

//! [1]
class FactorialLoopTransition : public QSignalTransition
{
public:
    FactorialLoopTransition(Factorial *fact)
        : QSignalTransition(fact, &Factorial::xChanged), m_fact(fact)
    {}

    bool eventTest(QEvent *e) override
    {
        if (!QSignalTransition::eventTest(e))
            return false;
        QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(e);
        return se->arguments().at(0).toInt() > 1;
    }

    void onTransition(QEvent *e) override
    {
        QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(e);
        int x = se->arguments().at(0).toInt();
        int fac = m_fact->property("fac").toInt();
        m_fact->setProperty("fac",  x * fac);
        m_fact->setProperty("x",  x - 1);
    }

private:
    Factorial *m_fact;
};
//! [1]

//! [2]
class FactorialDoneTransition : public QSignalTransition
{
public:
    FactorialDoneTransition(Factorial *fact)
        : QSignalTransition(fact, &Factorial::xChanged), m_fact(fact)
    {}

    bool eventTest(QEvent *e) override
    {
        if (!QSignalTransition::eventTest(e))
            return false;
        QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(e);
        return se->arguments().at(0).toInt() <= 1;
    }

    void onTransition(QEvent *) override
    {
        qInfo() << m_fact->property("fac").toInt();
    }

private:
    Factorial *m_fact;
};
//! [2]

//! [3]
int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    Factorial factorial;
    QStateMachine machine;
//! [3]

//! [4]
    QState *compute = new QState(&machine);
    compute->assignProperty(&factorial, "fac", 1);
    compute->assignProperty(&factorial, "x", 6);
    compute->addTransition(new FactorialLoopTransition(&factorial));
//! [4]

//! [5]
    QFinalState *done = new QFinalState(&machine);
    FactorialDoneTransition *doneTransition = new FactorialDoneTransition(&factorial);
    doneTransition->setTargetState(done);
    compute->addTransition(doneTransition);
//! [5]

//! [6]
    machine.setInitialState(compute);
    QObject::connect(&machine, &QStateMachine::finished, &app, QCoreApplication::quit);
    machine.start();

    return app.exec();
}
//! [6]

#include "main.moc"
