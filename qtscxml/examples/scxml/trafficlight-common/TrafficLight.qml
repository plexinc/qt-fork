// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Window
import TrafficLightStateMachine 1.0

Window {
    id: root

    property TrafficLightStateMachine stateMachine

    visible: true
    width: lights.width
    height: lights.height
    maximumWidth: lights.implicitWidth
    maximumHeight: lights.implicitHeight

    Lights {
        id: lights

        stateMachine: root.stateMachine
        button.source: stateMachine.working ? "pause.png" : "play.png"

        button.onClicked: stateMachine.submitEvent(stateMachine.working ? "smash" : "repair");
    }
}
