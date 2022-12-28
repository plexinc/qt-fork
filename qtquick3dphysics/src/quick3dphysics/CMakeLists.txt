# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## Quick3DPhysics Module:
#####################################################################

qt_internal_add_qml_module(Quick3DPhysics
    URI "QtQuick3D.Physics"
        VERSION "${PROJECT_VERSION}"
        CLASS_NAME QtQuick3DPhysicsPlugin
        PLUGIN_TARGET qquick3dphysicsplugin
        NO_PLUGIN_OPTIONAL
        NO_GENERATE_PLUGIN_SOURCE
    SOURCES
        qabstractcollisionnode.cpp qabstractcollisionnode_p.h
        qabstractcollisionshape.cpp qabstractcollisionshape_p.h
        qboxshape.cpp qboxshape_p.h
        qcapsuleshape.cpp qcapsuleshape_p.h
        qtriggerbody.cpp qtriggerbody_p.h
        qcharactercontroller.cpp qcharactercontroller_p.h
        qcollisiondebugmeshbuilder.cpp qcollisiondebugmeshbuilder_p.h
        qphysicsmeshutils_p_p.h
        qconvexmeshshape.cpp qconvexmeshshape_p.h
        qtrianglemeshshape.cpp qtrianglemeshshape_p.h
        qheightfieldshape.cpp qheightfieldshape_p.h
        qplaneshape.cpp qplaneshape_p.h
        qdynamicsworld.cpp qdynamicsworld_p.h
        qrigidbody.cpp qrigidbody_p.h
        qsphereshape.cpp qsphereshape_p.h
        qcacheutils.cpp qcacheutils_p.h
        qphysicsmaterial.cpp qphysicsmaterial_p.h
        qabstractphysicsbody.cpp qabstractphysicsbody_p.h
        qphysicscommands.cpp qphysicscommands_p.h
        qphysicsutils_p.h
        qdebugdrawhelper.cpp qdebugdrawhelper_p.h
        qtquick3dphysicsglobal.h
        qtquick3dphysicsglobal_p.h
    DEFINES
        QT_BUILD_QUICK3DPHYSICS_LIB
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::QmlPrivate
        Qt::Quick3DPrivate
        Qt::QuickPrivate
        Qt::BundledPhysX
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Qml
        Qt::Quick
        Qt::Quick3D
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::QmlPrivate
        Qt::Quick3D
        Qt::QuickPrivate
)

qt_internal_extend_target(qquick3dphysicsplugin
    SOURCES
        plugin.cpp
    LIBRARIES
        Qt::Quick3DPhysicsPrivate
)

target_include_directories(Quick3DPhysics SYSTEM
    PRIVATE
    ../3rdparty/PhysX/include
    ../3rdparty/PhysX/pxshared/include
)

if (UNIX)
    # Needed for PxPreprocessor.h error
    if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        qt_internal_extend_target(Quick3DPhysics DEFINES _DEBUG)
    else()
        qt_internal_extend_target(Quick3DPhysics DEFINES NDEBUG)
    endif()
endif()

qt_internal_extend_target(Quick3DPhysics DEFINES PX_PHYSX_STATIC_LIB)

qt_internal_add_docs(Quick3DPhysics
    doc/qtquick3dphysics.qdocconf
)
