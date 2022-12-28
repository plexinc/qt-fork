# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

qt_internal_add_test(tst_cooked
    GUI
    QMLTEST
    SOURCES
        tst_cooked.cpp
    LIBRARIES
        Qt::Core
        Qt::Qml
    TESTDATA
        data/hf.cooked.hf
        data/tetrahedron.cooked.cvx
        data/tetrahedron.cooked.tri
        data/tst_cooked.qml
)

if(QT_BUILD_STANDALONE_TESTS)
    qt_import_qml_plugins(tst_cooked)
endif()
