// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtScxml

TrafficLight {
    StateMachineLoader {
        id: loader
        source: "qrc:///statemachine.scxml"
    }

    stateMachine: loader.stateMachine
}
