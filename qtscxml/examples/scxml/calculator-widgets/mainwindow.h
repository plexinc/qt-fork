// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
class QScxmlStateMachine;
QT_END_NAMESPACE


class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QScxmlStateMachine *machine, QWidget *parent = nullptr);
    ~MainWindow();

private:
    QT_PREPEND_NAMESPACE(Ui::MainWindow) *ui;
    QScxmlStateMachine *m_machine;
};

#endif // MAINWINDOW_H
