// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QToolButton;
class QScxmlStateMachine;
class QLabel;
class QComboBox;
QT_END_NAMESPACE


class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QScxmlStateMachine *machine, QWidget *parent = nullptr);
    ~MainWindow();

private:
    QScxmlStateMachine *m_machine;
    QList<QList<QToolButton *>> m_buttons;
    QToolButton *m_startButton;
    QToolButton *m_undoButton;
    QLabel *m_label;
    QComboBox *m_chooser;
};

#endif // MAINWINDOW_H
