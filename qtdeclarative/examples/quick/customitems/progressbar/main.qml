// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import "content"

Rectangle {
    id: main

    width: 600; height: 405
    color: "#edecec"

    Flickable {
        anchors.fill: parent
        contentHeight: column.height + 20

        Column {
            id: column
            x: 10; y: 10
            spacing: 10

            Repeater {
                model: 25

                ProgressBar {
                    property int r: Math.floor(Math.random() * 5000 + 1000)
                    width: main.width - 20

                    NumberAnimation on value { duration: r; from: 0; to: 100; loops: Animation.Infinite }
                    ColorAnimation on color { duration: r; from: "lightsteelblue"; to: "thistle"; loops: Animation.Infinite }
                    ColorAnimation on secondColor { duration: r; from: "steelblue"; to: "#CD96CD"; loops: Animation.Infinite }
                }
            }
        }
    }
}
