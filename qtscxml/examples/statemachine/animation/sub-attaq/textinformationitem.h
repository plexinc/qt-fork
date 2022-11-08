// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TEXTINFORMATIONITEM_H
#define TEXTINFORMATIONITEM_H

//Qt
#include <QGraphicsTextItem>

class TextInformationItem : public QGraphicsTextItem
{
public:
    TextInformationItem(QGraphicsItem *parent = nullptr);
    void setMessage(const QString &text, const bool centerPosition = true);
};

#endif // TEXTINFORMATIONITEM_H
