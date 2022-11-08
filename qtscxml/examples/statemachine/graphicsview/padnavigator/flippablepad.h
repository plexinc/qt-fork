// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef FLIPPABLEPAD_H
#define FLIPPABLEPAD_H

#include "roundrectitem.h"

#include <QList>

//! [0]
class FlippablePad : public RoundRectItem
{
public:
    explicit FlippablePad(const QSize &size, QGraphicsItem *parent = nullptr);

    RoundRectItem *iconAt(int column, int row) const;

private:
    QList<QList<RoundRectItem *>> iconGrid;
};
//! [0]

#endif // FLIPPABLEPAD_H
