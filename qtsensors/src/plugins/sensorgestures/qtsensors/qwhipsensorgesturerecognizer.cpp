// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only


#include "qwhipsensorgesturerecognizer.h"
#include "qtsensorgesturesensorhandler.h"

#include <QtCore/qmath.h>

#define TIMER_TIMEOUT 850

QT_BEGIN_NAMESPACE

QWhipSensorGestureRecognizer::QWhipSensorGestureRecognizer(QObject *parent)
    : QSensorGestureRecognizer(parent),
    orientationReading(0),
    accelRange(0),
    active(0),
    lastX(0),
    lastY(0),
    lastZ(0),
    detecting(0),
    whipOk(0)
  , lastTimestamp(0)
  , timerActive(0)
  , lapsedTime(0)
{
}

QWhipSensorGestureRecognizer::~QWhipSensorGestureRecognizer()
{
}

void QWhipSensorGestureRecognizer::create()
{
}

QString QWhipSensorGestureRecognizer::id() const
{
    return QString("QtSensors.whip");
}

bool QWhipSensorGestureRecognizer::start()
{
    if (QtSensorGestureSensorHandler::instance()->startSensor(QtSensorGestureSensorHandler::Accel)) {
        if (QtSensorGestureSensorHandler::instance()->startSensor(QtSensorGestureSensorHandler::Orientation)) {
            accelRange = QtSensorGestureSensorHandler::instance()->accelRange;
            active = true;
            connect(QtSensorGestureSensorHandler::instance(),SIGNAL(orientationReadingChanged(QOrientationReading*)),
                    this,SLOT(orientationReadingChanged(QOrientationReading*)));

            connect(QtSensorGestureSensorHandler::instance(),SIGNAL(accelReadingChanged(QAccelerometerReading*)),
                    this,SLOT(accelChanged(QAccelerometerReading*)));
        } else {
            QtSensorGestureSensorHandler::instance()->stopSensor(QtSensorGestureSensorHandler::Accel);
            active = false;
        }
    } else {
        active = false;
    }
    lastTimestamp = 0;
    timerActive = false;
    lapsedTime = 0;
    return active;
}

bool QWhipSensorGestureRecognizer::stop()
{
    QtSensorGestureSensorHandler::instance()->stopSensor(QtSensorGestureSensorHandler::Accel);
    QtSensorGestureSensorHandler::instance()->stopSensor(QtSensorGestureSensorHandler::Orientation);
    disconnect(QtSensorGestureSensorHandler::instance(),SIGNAL(orientationReadingChanged(QOrientationReading*)),
            this,SLOT(orientationReadingChanged(QOrientationReading*)));

    disconnect(QtSensorGestureSensorHandler::instance(),SIGNAL(accelReadingChanged(QAccelerometerReading*)),
            this,SLOT(accelChanged(QAccelerometerReading*)));
    active = false;
    return active;
}

bool QWhipSensorGestureRecognizer::isActive()
{
    return active;
}

void QWhipSensorGestureRecognizer::orientationReadingChanged(QOrientationReading *reading)
{
    orientationReading = reading;
}

#define WHIP_FACTOR -11.0
#define WHIP_WIGGLE_FACTOR 0.35

void QWhipSensorGestureRecognizer::accelChanged(QAccelerometerReading *reading)
{
    const qreal x = reading->x();
    const qreal y = reading->y();
    qreal z = reading->z();

    quint64 timestamp = reading->timestamp();

    if (zList.count() > 4)
        zList.removeLast();

    qreal averageZ = 0;
    for (const qreal& az : qAsConst<QList<qreal>>(zList))
        averageZ += az;

    averageZ /= zList.count();

    zList.insert(0,z);

    if (orientationReading == 0)
        return;
    //// very hacky
    if (orientationReading->orientation() == QOrientationReading::FaceUp) {
        z = z - 9.8;
    }

    const qreal diffX = lastX - x;
    const qreal diffY = lastY - y;

    if (detecting && whipMap.count() > 5 && whipMap.at(5) == true) {
        checkForWhip();
    }

    if (whipMap.count() > 5)
        whipMap.removeLast();

    if (negativeList.count() > 5)
        negativeList.removeLast();

    if (z < WHIP_FACTOR
            && qAbs(diffX) > -(accelRange * .1285)//-5.0115
            && qAbs(lastX) < 7
            && qAbs(x) < 7) {
        whipMap.insert(0,true);
        if (!detecting && !timerActive) {
            timerActive = true;
            detecting = true;
        }
    } else {
        whipMap.insert(0,false);
    }

    // check if shaking
    if ((((x < 0 && lastX > 0) || (x > 0 && lastX < 0))
         && qAbs(diffX) > (accelRange   * 0.7)) //27.3
            || (((y < 0 && lastY > 0) || (y > 0 && lastY < 0))
            && qAbs(diffY) > (accelRange * 0.7))) {
        negativeList.insert(0,true);
    } else {
        negativeList.insert(0,false);
    }

    lastX = x;
    lastY = y;
    lastZ = z;

    if (timerActive && lastTimestamp > 0)
        lapsedTime += (timestamp - lastTimestamp )/1000;

    if (timerActive && lapsedTime >= TIMER_TIMEOUT) {
        timeout();
    }
}

void QWhipSensorGestureRecognizer::timeout()
{
    detecting = false;
}


void QWhipSensorGestureRecognizer::checkForWhip()
{
    whipOk = false;

    int check = 0;
    for (const qreal& az : qAsConst<QList<qreal>>(zList)) {
        if (az < -10)
            check++;
    }
    if (check >= 4)
        whipOk = true;
    else
        return;

    if (whipOk) {
        bool ok = true;
        for (int i = 0; i < negativeList.count() - 1; i++) {
            if (negativeList.at(i)) {
                ok = false;
            }
        }
        if (ok) {
            Q_EMIT whip();
            Q_EMIT detected("whip");
        }
        detecting = false;
        whipMap.clear();
        timerActive = false;
    }
}

QT_END_NAMESPACE
