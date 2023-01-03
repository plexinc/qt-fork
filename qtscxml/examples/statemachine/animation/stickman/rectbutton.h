// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef RECTBUTTON_H
#define RECTBUTTON_H

#include <QGraphicsObject>

class RectButton : public QGraphicsObject
{
    Q_OBJECT
public:
    RectButton(const QString &buttonText);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    void mousePressEvent (QGraphicsSceneMouseEvent *event) override;

signals:
    void clicked();

private:
    QString m_ButtonText;
};

#endif // RECTBUTTON_H
