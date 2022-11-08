// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Item {
    // "anywhere" is a context property, so we always have to check if it's null
    Button {
        id: here
        enabled: anywhere ? anywhere.here : false
        text: "Go There"
        width: parent.width / 2
        height: parent.height
        onClicked: anywhere.submitEvent("goThere")
    }

    Button {
        id: there
        enabled: anywhere ? anywhere.there : false
        text: "Go Here"
        width: parent.width / 2
        height: parent.height
        x: width
        onClicked: anywhere.submitEvent("goHere")
    }
}
