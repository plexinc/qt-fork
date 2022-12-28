// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QState;
class QStateMachine;
class QTransition;
QT_END_NAMESPACE

#define WIDTH 35
#define HEIGHT 20

//![0]
class Window : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString status READ status WRITE setStatus)

public:
    enum Direction { Up, Down, Left, Right };

    Window();

    void movePlayer(Direction direction);
    void setStatus(const QString &status);
    QString status() const;

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
//![0]

//![1]
private:
    void buildMachine();
    void setupMap();

    QChar map[WIDTH][HEIGHT];
    int pX, pY;

    QStateMachine *machine;
    QString myStatus;
};
//![1]

#endif

