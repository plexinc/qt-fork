// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef ROUNDRECTITEM_H
#define ROUNDRECTITEM_H

#include <QGraphicsObject>
#include <QLinearGradient>

//! [0]
class RoundRectItem : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(bool fill READ fill WRITE setFill)
public:
    RoundRectItem(const QRectF &bounds, const QColor &color,
                  QGraphicsItem *parent = nullptr);

    QPixmap pixmap() const;
    void setPixmap(const QPixmap &pixmap);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    bool fill() const;
    void setFill(bool fill);
//! [0]

//! [1]
private:
    QPixmap pix;
    bool fillRect;
    QRectF bounds;
    QLinearGradient gradient;
};
//! [1]

#endif // ROUNDRECTITEM_H
