// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PROGRESSITEM_H
#define PROGRESSITEM_H

//Qt
#include <QGraphicsTextItem>

class ProgressItem : public QGraphicsTextItem
{
public:
    ProgressItem(QGraphicsItem *parent = nullptr);
    void setLevel(int level);
    void setScore(int score);

private:
    void updateProgress();
    int currentLevel = 1;
    int currentScore = 0;
};

#endif // PROGRESSITEM_H
