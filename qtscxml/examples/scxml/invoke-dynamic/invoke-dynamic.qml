// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtScxml

MainView {
    stateMachine: directions.stateMachine

    StateMachineLoader {
        id: directions
        source: "qrc:///statemachine.scxml"
    }
}
