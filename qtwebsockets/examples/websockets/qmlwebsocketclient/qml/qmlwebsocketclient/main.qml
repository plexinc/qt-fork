// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick 2.0
import QtWebSockets 1.0

Rectangle {
    width: 360
    height: 360

    WebSocket {
        id: socket
        url: "ws://echo.websocket.org"
        onTextMessageReceived: {
            messageBox.text = messageBox.text + "\nReceived message: " + message
        }
        onStatusChanged: if (socket.status == WebSocket.Error) {
                             console.log("Error: " + socket.errorString)
                         } else if (socket.status == WebSocket.Open) {
                             socket.sendTextMessage("Hello World")
                         } else if (socket.status == WebSocket.Closed) {
                             messageBox.text += "\nSocket closed"
                         }
        active: false
    }

    WebSocket {
        id: secureWebSocket
        url: "wss://echo.websocket.org"
        onTextMessageReceived: {
            messageBox.text = messageBox.text + "\nReceived secure message: " + message
        }
        onStatusChanged: if (secureWebSocket.status == WebSocket.Error) {
                             console.log("Error: " + secureWebSocket.errorString)
                         } else if (secureWebSocket.status == WebSocket.Open) {
                             secureWebSocket.sendTextMessage("Hello Secure World")
                         } else if (secureWebSocket.status == WebSocket.Closed) {
                             messageBox.text += "\nSecure socket closed"
                         }
        active: false
    }
    Text {
        id: messageBox
        text: socket.status == WebSocket.Open ? qsTr("Sending...") : qsTr("Welcome!")
        anchors.centerIn: parent
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            socket.active = !socket.active
            secureWebSocket.active =  !secureWebSocket.active;
            //Qt.quit();
        }
    }
}
