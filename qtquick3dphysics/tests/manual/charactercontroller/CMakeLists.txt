# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_manual_test(test_charactercontroller
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
  "Wasd.qml"
)

qt6_add_resources(test_charactercontroller "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)


qt6_add_resources(test_charactercontroller "assets"
    PREFIX
        "/"
    FILES
          "maps/OpenfootageNET_lowerAustria01-512.hdr"
          "meshes/room.mesh"
          "meshes/stairs.mesh"
)
