# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_manual_test(test_convexmesh
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

qt6_add_resources(test_convexmesh "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)


qt6_add_resources(test_convexmesh "assets"
    PREFIX
        "/"
    FILES
        "qt_logo_rect.png"
        "meshes/test.mesh"
        "meshes/field.mesh"
        "meshes/newTorus.mesh"
        "meshes/newConvexTorus.mesh"
        "maps/heightfield.png"
)
