// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "statemachine.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<Directions>("Directions", 1, 0, "Directions");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/invoke-static.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
