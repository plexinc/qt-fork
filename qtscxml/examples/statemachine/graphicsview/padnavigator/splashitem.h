// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SPLASHITEM_H
#define SPLASHITEM_H

#include <QGraphicsObject>

//! [0]
class SplashItem : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit SplashItem(QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    QString text;
};
//! [0]

#endif // SPLASHITEM_H
