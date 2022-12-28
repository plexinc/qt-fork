// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "thedatamodel.h"

#include <QScxmlEvent>

bool TheDataModel::isValidMedia() const
{
    QString eventMedia = eventData().value(QStringLiteral("media")).toString();
    return eventMedia.size() > 0;
}

QVariantMap TheDataModel::eventData() const
{
    return scxmlEvent().data().value<QVariantMap>();
}
