// Copyright (C) 2014 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [document]
import QtQuick
import QtQml.StateMachine

Rectangle {
//![0]
    Row {
        anchors.fill: parent
        spacing: 2
        Button {
            id: button
            // change the button label to the active state id
            text: s11.active ? "s11" : s12.active ? "s12" :  s13.active ? "s13" : "s3"
        }
        Button {
            id: interruptButton
            text: s1.active ? "Interrupt" : "Resume"
        }
        Button {
            id: quitButton
            text: "quit"
        }
    }

    StateMachine {
        id: stateMachine
        // set the initial state
        initialState: s1

        // start the state machine
        running: true

        State {
            id: s1
            // set the initial state
            initialState: s11

            // create a transition from s1 to s2 when the button is clicked
            SignalTransition {
                targetState: s2
                signal: quitButton.clicked
            }
            // do something when the state enters/exits
            onEntered: console.log("s1 entered")
            onExited: console.log("s1 exited")
            State {
                id: s11
                // create a transition from s1 to s2 when the button is clicked
                SignalTransition {
                    targetState: s12
                    signal: button.clicked
                }
                // do something when the state enters/exits
                onEntered: console.log("s11 entered")
                onExited: console.log("s11 exited")
            }

            State {
                id: s12
                // create a transition from s2 to s3 when the button is clicked
                SignalTransition {
                    targetState: s13
                    signal: button.clicked
                }
                // do something when the state enters/exits
                onEntered: console.log("s12 entered")
                onExited: console.log("s12 exited")
            }
            State {
                id: s13
                // create a transition from s3 to s1 when the button is clicked
                SignalTransition {
                    targetState: s1
                    signal: button.clicked
                }
                // do something when the state enters/exits
                onEntered: console.log("s13 entered")
                onExited: console.log("s13 exited")
            }

            // create a transition from s1 to s3 when the button is clicked
            SignalTransition {
                targetState: s3
                signal: interruptButton.clicked
            }
            HistoryState {
                id: s1h
            }
        }
        FinalState {
            id: s2
            onEntered: console.log("s2 entered")
            onExited: console.log("s2 exited")
        }
        State {
            id: s3
            SignalTransition {
                targetState: s1h
                signal: interruptButton.clicked
            }
            // do something when the state enters/exits
            onEntered: console.log("s3 entered")
            onExited: console.log("s3 exited")
        }
        onFinished: Qt.quit()
    }
//![0]
}
//! [document]
