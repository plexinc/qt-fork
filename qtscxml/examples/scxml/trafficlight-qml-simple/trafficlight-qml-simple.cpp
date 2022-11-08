// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "statemachine.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<TrafficLightStateMachine>("TrafficLightStateMachine", 1, 0,
                                              "TrafficLightStateMachine");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/TrafficLight.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}

