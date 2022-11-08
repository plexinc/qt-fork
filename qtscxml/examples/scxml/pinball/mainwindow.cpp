// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QScxmlStateMachine>
#include <QStringListModel>

QT_USE_NAMESPACE

MainWindow::MainWindow(QScxmlStateMachine *machine, QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::MainWindow),
    m_machine(machine)
{
    m_ui->setupUi(this);

    // lights
    initAndConnect(QLatin1String("cLightOn"), m_ui->cLabel);
    initAndConnect(QLatin1String("rLightOn"), m_ui->rLabel);
    initAndConnect(QLatin1String("aLightOn"), m_ui->aLabel);
    initAndConnect(QLatin1String("zLightOn"), m_ui->zLabel);
    initAndConnect(QLatin1String("yLightOn"), m_ui->yLabel);
    initAndConnect(QLatin1String("hurryLightOn"), m_ui->hurryLabel);
    initAndConnect(QLatin1String("jackpotLightOn"), m_ui->jackpotLabel);
    initAndConnect(QLatin1String("gameOverLightOn"), m_ui->gameOverLabel);

    // help labels
    initAndConnect(QLatin1String("offState"), m_ui->offStateLabel);
    initAndConnect(QLatin1String("hurryStateOff"), m_ui->normalStateLabel);
    initAndConnect(QLatin1String("hurryStateOn"), m_ui->hurryStateLabel);
    initAndConnect(QLatin1String("jackpotStateOn"), m_ui->jackpotStateLabel);

    // context enablement
    initAndConnect(QLatin1String("offState"), m_ui->startButton);
    initAndConnect(QLatin1String("onState"), m_ui->cButton);
    initAndConnect(QLatin1String("onState"), m_ui->rButton);
    initAndConnect(QLatin1String("onState"), m_ui->aButton);
    initAndConnect(QLatin1String("onState"), m_ui->zButton);
    initAndConnect(QLatin1String("onState"), m_ui->yButton);
    initAndConnect(QLatin1String("onState"), m_ui->ballOutButton);

    // datamodel update
    m_machine->connectToEvent("updateScore", [this] (const QScxmlEvent &event) {
        const QVariant data = event.data();
        const QString highScore = data.toMap().value("highScore").toString();
        m_ui->highScoreLabel->setText(highScore);
        const QString score = data.toMap().value("score").toString();
        m_ui->scoreLabel->setText(score);
    });

    // gui interaction
    connect(m_ui->cButton, &QAbstractButton::clicked,
            [this] { m_machine->submitEvent("cLetterTriggered");
            });
    connect(m_ui->rButton, &QAbstractButton::clicked,
            [this] { m_machine->submitEvent("rLetterTriggered");
            });
    connect(m_ui->aButton, &QAbstractButton::clicked,
            [this] { m_machine->submitEvent("aLetterTriggered");
            });
    connect(m_ui->zButton, &QAbstractButton::clicked,
            [this] { m_machine->submitEvent("zLetterTriggered");
            });
    connect(m_ui->yButton, &QAbstractButton::clicked,
            [this] { m_machine->submitEvent("yLetterTriggered");
            });
    connect(m_ui->startButton, &QAbstractButton::clicked,
            [this] { m_machine->submitEvent("startTriggered");
            });
    connect(m_ui->ballOutButton, &QAbstractButton::clicked,
            [this] { m_machine->submitEvent("ballOutTriggered");
            });
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::initAndConnect(const QString &state, QWidget *widget)
{
    widget->setEnabled(m_machine->isActive(state));
    m_machine->connectToState(state, widget, &QWidget::setEnabled);
}
