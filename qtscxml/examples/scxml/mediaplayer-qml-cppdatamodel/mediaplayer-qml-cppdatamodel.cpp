// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "mediaplayer-cppdatamodel.h"
#include "thedatamodel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<TheDataModel>("MediaPlayerDataModel", 1, 0, "MediaPlayerDataModel");
    qmlRegisterType<MediaPlayerStateMachine>("MediaPlayerStateMachine", 1, 0, "MediaPlayerStateMachine");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:///mediaplayer-qml-cppdatamodel.qml")));

    return app.exec();
}
