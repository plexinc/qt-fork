// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef CHARACTERCONTROLLER_H
#define CHARACTERCONTROLLER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtQuick3DPhysics/qtquick3dphysicsglobal.h>
#include <QtQuick3DPhysics/private/qabstractphysicsbody_p.h>
#include <QtQml/QQmlEngine>
#include <QVector3D>

QT_BEGIN_NAMESPACE

class Q_QUICK3DPHYSICS_EXPORT QCharacterController : public QAbstractPhysicsBody
{
    Q_OBJECT
    Q_PROPERTY(QVector3D speed READ speed WRITE setSpeed NOTIFY speedChanged)
    Q_PROPERTY(QVector3D gravity READ gravity WRITE setGravity NOTIFY gravityChanged)
    Q_PROPERTY(bool midAirControl READ midAirControl WRITE setMidAirControl NOTIFY
                       midAirControlChanged)
    Q_PROPERTY(Collisions collisions READ collisions NOTIFY collisionsChanged)
    QML_NAMED_ELEMENT(CharacterController)
public:
    QCharacterController();

    enum class Collision {
        None = 0,
        Side = 1 << 0,
        Up = 1 << 1,
        Down = 1 << 2,
    };
    Q_DECLARE_FLAGS(Collisions, Collision)
    Q_FLAG(Collisions)

    const QVector3D &speed() const;
    void setSpeed(const QVector3D &newSpeed);
    const QVector3D &gravity() const;
    void setGravity(const QVector3D &newGravity);
    QVector3D getMovement(float deltaTime);
    bool getTeleport(QVector3D &position);

    bool midAirControl() const;
    void setMidAirControl(bool newMidAirControl);

    Q_INVOKABLE void teleport(const QVector3D &position);

    const Collisions &collisions() const;
    void setCollisions(const Collisions &newCollisions);

signals:
    void speedChanged();
    void gravityChanged();

    void midAirControlChanged();

    void impulseChanged();

    void collisionsChanged();

private:
    QVector3D m_speed;
    QVector3D m_gravity;
    bool m_midAirControl = true;

    QVector3D m_freeFallVelocity; // actual speed at start of next tick, if free fall

    QVector3D m_teleportPosition;
    bool m_teleport = false;
    Collisions m_collisions;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QCharacterController::Collisions)

QT_END_NAMESPACE

#endif // CHARACTERCONTROLLER_H
