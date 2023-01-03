// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef STICKMAN_H
#define STICKMAN_H

#include <QGraphicsObject>

static const int NodeCount = 16;
static const int BoneCount = 24;

class Node;
class StickMan: public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(QColor penColor WRITE setPenColor READ penColor)
    Q_PROPERTY(QColor fillColor WRITE setFillColor READ fillColor)
    Q_PROPERTY(bool isDead WRITE setIsDead READ isDead)
public:
    StickMan();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    int nodeCount() const;
    Node *node(int idx) const;

    void setDrawSticks(bool on);
    bool drawSticks() const { return m_sticks; }

    QColor penColor() const { return m_penColor; }
    void setPenColor(const QColor &color) { m_penColor = color; }

    QColor fillColor() const { return m_fillColor; }
    void setFillColor(const QColor &color) { m_fillColor = color; }

    bool isDead() const { return m_isDead; }
    void setIsDead(bool isDead) { m_isDead = isDead; }

public slots:
    void stabilize();
    void childPositionChanged();

protected:
    void timerEvent(QTimerEvent *e) override;

private:

    QPointF posFor(int idx) const;

    Node *m_nodes[NodeCount];
    qreal m_perfectBoneLengths[BoneCount];

    bool m_sticks = true;
    bool m_isDead = false;

    QColor m_penColor = Qt::white;
    QColor m_fillColor = Qt::black;
};

#endif // STICKMAN_H
