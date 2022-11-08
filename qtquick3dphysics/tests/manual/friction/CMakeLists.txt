# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

qt_internal_add_manual_test(test_friction
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::Quick3D
        Qt::Quick3DPhysics
)

# Resources:
set(qml_resource_files
  "main.qml"
)

qt6_add_resources(test_friction "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)
