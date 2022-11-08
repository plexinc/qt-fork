# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

qt_internal_add_test(tst_physicsscene
    GUI
    QMLTEST
    SOURCES
        tst_physicsscene.cpp
    LIBRARIES
        Qt::Core
        Qt::Qml
    TESTDATA
        data/tst_physicsscene.qml
        data/TestCube.qml
        data/hf.png
)

if(QT_BUILD_STANDALONE_TESTS)
    qt_import_qml_plugins(tst_physicsscene)
endif()
