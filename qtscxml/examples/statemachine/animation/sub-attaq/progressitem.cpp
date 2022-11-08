// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "progressitem.h"

#include <QFont>

ProgressItem::ProgressItem(QGraphicsItem *parent)
    : QGraphicsTextItem(parent)
{
    setFont(QFont(QStringList{"Comic Sans MS"}));
    setPos(parentItem()->boundingRect().topRight() - QPointF(180, -5));
}

void ProgressItem::setLevel(int level)
{
    currentLevel = level;
    updateProgress();
}

void ProgressItem::setScore(int score)
{
    currentScore = score;
    updateProgress();
}

void ProgressItem::updateProgress()
{
    setHtml(QString("Level : %1 Score : %2").arg(currentLevel).arg(currentScore));
}
