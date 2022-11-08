// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause


import QtQuick
import QtQuick.Window
import TrafficLightStateMachine 1.0

Image {
    id: lights

    property alias button: button
    property TrafficLightStateMachine stateMachine

    source: "background.png"

    Column {
        y: 40
        spacing: 27
        anchors.horizontalCenter: parent.horizontalCenter

        Image {
            id: redLight
            opacity: 0.2
            source: "red.png"
        }

        Image {
            id: yellowLight
            opacity: 0.2
            source: "yellow.png"
        }

        Image {
            id: greenLight
            opacity: 0.2
            source: "green.png"
        }
    }

    Button {
        id: button

        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 20
        source: "pause.png"
    }

    states: [
        State {
            name: "Red"
            when: stateMachine.red

            PropertyChanges {
                target: redLight
                opacity: 1
            }
        },
        State {
            name: "RedGoingGreen"
            when: stateMachine.redGoingGreen

            PropertyChanges {
                target: redLight
                opacity: 1
            }

            PropertyChanges {
                target: yellowLight
                opacity: 1
            }
        },
        State {
            name: "Yellow"
            when: stateMachine.yellow || stateMachine.blinking

            PropertyChanges {
                target: yellowLight
                opacity: 1
            }
        },
        State {
            name: "Green"
            when: stateMachine.green

            PropertyChanges {
                target: greenLight
                opacity: 1
            }
        }
    ]
}
