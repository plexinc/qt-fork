// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtStateMachine>
#include <QtWidgets>

//! [0]
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QPushButton button;
    QStateMachine machine;
//! [0]

//! [1]
    QState *off = new QState();
    off->assignProperty(&button, "text", "Off");
    off->setObjectName("off");

    QState *on = new QState();
    on->setObjectName("on");
    on->assignProperty(&button, "text", "On");
//! [1]

//! [2]
    off->addTransition(&button, &QAbstractButton::clicked, on);
    on->addTransition(&button, &QAbstractButton::clicked, off);
//! [2]

//! [3]
    machine.addState(off);
    machine.addState(on);
//! [3]

//! [4]
    machine.setInitialState(off);
    machine.start();
//! [4]

//! [5]
    button.resize(100, 50);
    button.show();
    return app.exec();
}
//! [5]
