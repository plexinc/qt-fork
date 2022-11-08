// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "../mediaplayer-common/mainwindow.h"

#include <QApplication>
#include <QScxmlStateMachine>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    auto machine = QScxmlStateMachine::fromFile(
                QStringLiteral(":mediaplayer.scxml"));
    MainWindow mainWindow(machine);
    machine->setParent(&mainWindow);

    machine->start();
    mainWindow.show();
    return app.exec();
}
