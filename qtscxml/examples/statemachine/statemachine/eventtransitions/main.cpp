// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QApplication>
#include <QEventTransition>
#include <QPushButton>
#include <QStateMachine>
#include <QVBoxLayout>
#include <QWidget>

//! [0]
class Window : public QWidget
{
public:
    Window(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        QPushButton *button = new QPushButton(this);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(button);
        layout->setContentsMargins(80, 80, 80, 80);
        setLayout(layout);
//! [0]

//! [1]
        QStateMachine *machine = new QStateMachine(this);

        QState *s1 = new QState();
        s1->assignProperty(button, "text", "Outside");

        QState *s2 = new QState();
        s2->assignProperty(button, "text", "Inside");
//! [1]

//! [2]
        QEventTransition *enterTransition = new QEventTransition(button, QEvent::Enter);
        enterTransition->setTargetState(s2);
        s1->addTransition(enterTransition);
//! [2]

//! [3]
        QEventTransition *leaveTransition = new QEventTransition(button, QEvent::Leave);
        leaveTransition->setTargetState(s1);
        s2->addTransition(leaveTransition);
//! [3]

//! [4]
        QState *s3 = new QState();
        s3->assignProperty(button, "text", "Pressing...");

        QEventTransition *pressTransition = new QEventTransition(button, QEvent::MouseButtonPress);
        pressTransition->setTargetState(s3);
        s2->addTransition(pressTransition);

        QEventTransition *releaseTransition = new QEventTransition(button, QEvent::MouseButtonRelease);
        releaseTransition->setTargetState(s2);
        s3->addTransition(releaseTransition);
//! [4]

//! [5]
        machine->addState(s1);
        machine->addState(s2);
        machine->addState(s3);

        machine->setInitialState(s1);
        machine->start();
    }
};
//! [5]

//! [6]
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Window window;
    window.resize(300, 300);
    window.show();

    return app.exec();
}
//! [6]
