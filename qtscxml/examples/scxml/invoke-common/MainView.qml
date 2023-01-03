// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Window
import QtScxml

Window {
    id: window
    visible: true
    property StateMachine stateMachine

    color: "black"
    width: 400
    height: 300

    Item {
        width: parent.width / 2
        height: parent.height

        Button {
            id: nowhere
            text: "Go Nowhere"
            width: parent.width
            height: parent.height / 2
            onClicked: stateMachine.submitEvent("goNowhere")
            enabled: stateMachine.somewhere
        }

        Button {
            id: somewhere
            text: "Go Somewhere"
            width: parent.width
            height: parent.height / 2
            y: parent.height / 2
            onClicked: stateMachine.submitEvent("goSomewhere")
            enabled: stateMachine.nowhere
        }
    }

    Loader {
        source: "SubView.qml"
        active: stateMachine.somewhere

        x: parent.width / 2
        width: parent.width / 2
        height: parent.height

        InvokedServices {
            id: services
            stateMachine: window.stateMachine
        }

        property var anywhere: services.children.anywhere ? services.children.anywhere.stateMachine
                                                          : undefined
    }
}

