// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QScxmlStateMachine>
#include <QStringListModel>

QT_USE_NAMESPACE

MainWindow::MainWindow(QScxmlStateMachine *stateMachine, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    auto model = new QStringListModel(QStringList() << QStringLiteral("song 1")
                                                    << QStringLiteral("song 2")
                                                    << QStringLiteral("song 3"),
                                      this);
    ui->mediaListView->setModel(model);

    connect(ui->mediaListView, &QAbstractItemView::clicked,
            [model, stateMachine](const QModelIndex &index) {
        QVariantMap data;
        data.insert(QStringLiteral("media"),
                    model->data(index, Qt::EditRole).toString());
        stateMachine->submitEvent("tap", data);
    });

    stateMachine->connectToEvent("playbackStarted", this, &MainWindow::started);
    stateMachine->connectToEvent("playbackStopped", this, &MainWindow::stopped);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::started(const QScxmlEvent &event)
{
    const QString media = event.data().toMap().value("media").toString();
    ui->logText->appendPlainText(QStringLiteral(
                 "call on slot started with media '%1'").arg(media));
    ui->statusLabel->setText(QStringLiteral("Playing %1").arg(media));
}

void MainWindow::stopped(const QScxmlEvent &event)
{
    const QString media = event.data().toMap().value("media").toString();
    ui->logText->appendPlainText(QStringLiteral(
                 "call on slot stopped with media '%1'").arg(media));
    ui->statusLabel->setText(QStringLiteral("Stopped"));
}
