// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QScxmlStateMachine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    /* Register QScxmlStateMachine as TrafficLightStateMachine. This is required to have a type
     * for the state machine and allows full code completion in the static case, since we
     * share the QML code. */
    qmlRegisterUncreatableType<QScxmlStateMachine>("TrafficLightStateMachine",
                                                   1, 0,
                                                   "TrafficLightStateMachine",
                                                   QLatin1String("TrafficLightStateMachine is not creatable."));

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:///trafficlight-qml-dynamic.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}

