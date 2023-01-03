// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef THEDATAMODEL_H
#define THEDATAMODEL_H

#include "qscxmlcppdatamodel.h"

class TheDataModel: public QScxmlCppDataModel
{
    Q_OBJECT
    Q_SCXML_DATAMODEL

private:
    bool isValidMedia() const;
    QVariantMap eventData() const;

    QString media;
};

#endif // THEDATAMODEL_H
