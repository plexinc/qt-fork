// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MOVEMENTTRANSITION_H
#define MOVEMENTTRANSITION_H

#include <QtStateMachine>
#include <QtWidgets>

#include "window.h"

//![0]
class MovementTransition : public QEventTransition
{
    Q_OBJECT

public:
    MovementTransition(Window *window) :
        QEventTransition(window, QEvent::KeyPress) {
        this->window = window;
    }
//![0]

//![1]
protected:
    bool eventTest(QEvent *event) override {
        if (event->type() == QEvent::StateMachineWrapped &&
            static_cast<QStateMachine::WrappedEvent *>(event)->event()->type() == QEvent::KeyPress) {
            QEvent *wrappedEvent = static_cast<QStateMachine::WrappedEvent *>(event)->event();

            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(wrappedEvent);
            int key = keyEvent->key();

            return key == Qt::Key_2 || key == Qt::Key_8 || key == Qt::Key_6 ||
                   key == Qt::Key_4 || key == Qt::Key_Down || key == Qt::Key_Up ||
                   key == Qt::Key_Right || key == Qt::Key_Left;
        }
        return false;
    }
//![1]

//![2]
    void onTransition(QEvent *event) override {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(
            static_cast<QStateMachine::WrappedEvent *>(event)->event());

        int key = keyEvent->key();
        switch (key) {
            case Qt::Key_Left:
            case Qt::Key_4:
                window->movePlayer(Window::Left);
                break;
            case Qt::Key_Up:
            case Qt::Key_8:
                window->movePlayer(Window::Up);
                break;
            case Qt::Key_Right:
            case Qt::Key_6:
                window->movePlayer(Window::Right);
                break;
            case Qt::Key_Down:
            case Qt::Key_2:
                window->movePlayer(Window::Down);
                break;
            default:
                ;
        }
    }
//![2]

private:
    Window *window;
};

#endif

