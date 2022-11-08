// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QScxmlStateMachine>
#include <QStringListModel>

QT_USE_NAMESPACE

MainWindow::MainWindow(QScxmlStateMachine *machine, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow),
    m_machine(machine)
{
    ui->setupUi(this);

    connect(ui->digit0, &QAbstractButton::clicked, [this] {
        m_machine->submitEvent("DIGIT.0");
    });
    connect(ui->digit1, &QAbstractButton::clicked, [this] {
        m_machine->submitEvent("DIGIT.1");
    });
    connect(ui->digit2, &QAbstractButton::clicked, [this] {
        m_machine->submitEvent("DIGIT.2");
    });
    connect(ui->digit3, &QAbstractButton::clicked, [this] {
        m_machine->submitEvent("DIGIT.3");
    });
    connect(ui->digit4, &QAbstractButton::clicked, [this] {
        m_machine->submitEvent("DIGIT.4");
    });
    connect(ui->digit5, &QAbstractButton::clicked, [this] {
        m_machine->submitEvent("DIGIT.5");
    });
    connect(ui->digit6, &QAbstractButton::clicked, [this] {
        m_machine->submitEvent("DIGIT.6");
    });
    connect(ui->digit7, &QAbstractButton::clicked, [this] {
        m_machine->submitEvent("DIGIT.7");
    });
    connect(ui->digit8, &QAbstractButton::clicked, [this] {
        m_machine->submitEvent("DIGIT.8");
    });
    connect(ui->digit9, &QAbstractButton::clicked, [this] {
        m_machine->submitEvent("DIGIT.9");
    });
    connect(ui->point, &QAbstractButton::clicked, [this] {
        m_machine->submitEvent("POINT");
    });
    connect(ui->operPlus, &QAbstractButton::clicked, [this] {
        m_machine->submitEvent("OPER.PLUS");
    });
    connect(ui->operMinus, &QAbstractButton::clicked, [this] {
        m_machine->submitEvent("OPER.MINUS");
    });
    connect(ui->operStar, &QAbstractButton::clicked, [this] {
        m_machine->submitEvent("OPER.STAR");
    });
    connect(ui->operDiv, &QAbstractButton::clicked, [this] {
        m_machine->submitEvent("OPER.DIV");
    });
    connect(ui->equals, &QAbstractButton::clicked, [this] {
        m_machine->submitEvent("EQUALS");
    });
    connect(ui->c, &QAbstractButton::clicked, [this] {
        m_machine->submitEvent("C");
    });

    m_machine->connectToEvent(QLatin1String("updateDisplay"), this,
                              [this](const QScxmlEvent &event) {
        const QString display = event.data().toMap()
                .value("display").toString();
        ui->display->setText(display);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

