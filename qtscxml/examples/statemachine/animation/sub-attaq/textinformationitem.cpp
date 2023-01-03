// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#include "textinformationitem.h"
#include "pixmapitem.h"

TextInformationItem::TextInformationItem (QGraphicsItem *parent)
    : QGraphicsTextItem(parent)
{
    setFont(QFont(QStringList{"Comic Sans MS"}, 15));
}

void TextInformationItem::setMessage(const QString &text, bool centerPosition)
{
    setHtml(text);
    if (centerPosition) {
        setPos(parentItem()->boundingRect().center().x() - boundingRect().size().width() / 2,
               parentItem()->boundingRect().center().y());
    }
}
