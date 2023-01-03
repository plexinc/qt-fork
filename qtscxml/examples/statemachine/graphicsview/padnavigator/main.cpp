// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "padnavigator.h"

#include <QtWidgets/QtWidgets>

//! [0]
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Q_INIT_RESOURCE(padnavigator);

    PadNavigator navigator(QSize(3, 3));
    navigator.show();
    return app.exec();
}
//! [0]
