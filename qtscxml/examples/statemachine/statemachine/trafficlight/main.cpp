// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QApplication>
#include <QFinalState>
#include <QPainter>
#include <QStateMachine>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

//! [0]
class LightWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool on READ isOn WRITE setOn)
public:
    LightWidget(const QColor &color, QWidget *parent = nullptr)
        : QWidget(parent), m_color(color), m_on(false) {}

    bool isOn() const
        { return m_on; }
    void setOn(bool on)
    {
        if (on == m_on)
            return;
        m_on = on;
        update();
    }

public slots:
    void turnOff() { setOn(false); }
    void turnOn() { setOn(true); }

protected:
    void paintEvent(QPaintEvent *) override
    {
        if (!m_on)
            return;
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(m_color);
        painter.drawEllipse(0, 0, width(), height());
    }

private:
    QColor m_color;
    bool m_on;
};
//! [0]

//! [1]
class TrafficLightWidget : public QWidget
{
public:
    TrafficLightWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        QVBoxLayout *vbox = new QVBoxLayout(this);
        m_red = new LightWidget(Qt::red);
        vbox->addWidget(m_red);
        m_yellow = new LightWidget(Qt::yellow);
        vbox->addWidget(m_yellow);
        m_green = new LightWidget(Qt::green);
        vbox->addWidget(m_green);
        QPalette pal = palette();
        pal.setColor(QPalette::Window, Qt::black);
        setPalette(pal);
        setAutoFillBackground(true);
    }

    LightWidget *redLight() const
        { return m_red; }
    LightWidget *yellowLight() const
        { return m_yellow; }
    LightWidget *greenLight() const
        { return m_green; }

private:
    LightWidget *m_red;
    LightWidget *m_yellow;
    LightWidget *m_green;
};
//! [1]

//! [2]
QState *createLightState(LightWidget *light, int duration, QState *parent = nullptr)
{
    QState *lightState = new QState(parent);
    QTimer *timer = new QTimer(lightState);
    timer->setInterval(duration);
    timer->setSingleShot(true);
    QState *timing = new QState(lightState);
    QObject::connect(timing, &QAbstractState::entered, light, &LightWidget::turnOn);
    QObject::connect(timing, &QAbstractState::entered, timer, QOverload<>::of(&QTimer::start));
    QObject::connect(timing, &QAbstractState::exited, light, &LightWidget::turnOff);
    QFinalState *done = new QFinalState(lightState);
    timing->addTransition(timer, &QTimer::timeout, done);
    lightState->setInitialState(timing);
    return lightState;
}
//! [2]

//! [3]
class TrafficLight : public QWidget
{
public:
    TrafficLight(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        QVBoxLayout *vbox = new QVBoxLayout(this);
        TrafficLightWidget *widget = new TrafficLightWidget;
        vbox->addWidget(widget);
        vbox->setContentsMargins(QMargins());

        QStateMachine *machine = new QStateMachine(this);
        QState *redGoingYellow = createLightState(widget->redLight(), 3000);
        redGoingYellow->setObjectName("redGoingYellow");
        QState *yellowGoingGreen = createLightState(widget->yellowLight(), 1000);
        yellowGoingGreen->setObjectName("yellowGoingGreen");
        redGoingYellow->addTransition(redGoingYellow, &QState::finished, yellowGoingGreen);
        QState *greenGoingYellow = createLightState(widget->greenLight(), 3000);
        greenGoingYellow->setObjectName("greenGoingYellow");
        yellowGoingGreen->addTransition(yellowGoingGreen, &QState::finished, greenGoingYellow);
        QState *yellowGoingRed = createLightState(widget->yellowLight(), 1000);
        yellowGoingRed->setObjectName("yellowGoingRed");
        greenGoingYellow->addTransition(greenGoingYellow, &QState::finished, yellowGoingRed);
        yellowGoingRed->addTransition(yellowGoingRed, &QState::finished, redGoingYellow);

        machine->addState(redGoingYellow);
        machine->addState(yellowGoingGreen);
        machine->addState(greenGoingYellow);
        machine->addState(yellowGoingRed);
        machine->setInitialState(redGoingYellow);
        machine->start();
    }
};
//! [3]

//! [4]
int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    TrafficLight widget;
    widget.resize(110, 300);
    widget.show();

    return app.exec();
}
//! [4]

#include "main.moc"
