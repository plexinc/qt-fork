// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qcharactercontroller_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype CharacterController
    \inqmlmodule QtQuick3DPhysics
    \inherits PhysicsBody
    \since 6.4
    \brief Defines a character controller.

    The CharacterController type controls the motion of a character.

    A character is an entity that moves under external control, but is still constrained
    by physical barriers and (optionally) subject to gravity.  This is in contrast to
    \l{DynamicRigidBody}{dynamic rigid bodies} which are either completely controlled by
    the physics simulation (for non-kinematic bodies); or move exactly where placed,
    regardless of barriers (for kinematic objects).

    For a first-person view, the camera is typically placed inside a character controller.
*/

/*!
    \qmlproperty vector3d CharacterController::speed

    This property defines the controlled speed of the character. This is the speed the character
    would move in the absence of gravity and without interacting with other physics objects.

    This property does not reflect the actual velocity of the character. If the character is stuck
    against terrain, the character can move slower than the defined \c speed. Conversely, if the
    character is in free fall, it may move much faster.

    The default value is \c{(0, 0, 0)}.
*/

/*!
    \qmlproperty vector3d CharacterController::gravity

    This property defines the gravitational acceleration that applies to the character.
    For a character that walks on the ground, it should typically be set to
    \l{DynamicsWorld::gravity}{DynamicsWorld.gravity}. A floating character that has movement
    controls in three dimensions will normally have gravity \c{(0, 0, 0)}. The default value is
    \c{(0, 0, 0)}.
*/

/*!
    \qmlproperty bool CharacterController::midAirControl

    This property defines whether the \l speed property has effect when the character is in free
    fall. This is only relevant if \l gravity in not null. A value of \c true means that the
    character will change direction in mid-air when \l speed changes. A value of \c false means that
    the character will continue on its current trajectory until it hits another object. The default
    value is \c true.
*/

/*!
    \qmlproperty Collisions CharacterController::collisions
    \readonly

    This property holds the current collision state of the character. It is either \c None for no
    collision, or an OR combination of \c Side, \c Up, and \c Down:

     \value   CharacterController.None
         The character is not colliding with anything. If gravity is non-null, this means that the
         character is in free fall.
     \value   CharacterController.Side
         The character is colliding with something from the side.
     \value   CharacterController.Up
         The character is colliding with something from above.
     \value   CharacterController.Down
         The character is colliding with something from below. In standard gravity, this means
         that the character is on the ground.
*/

/*!
    \qmlmethod CharacterController::teleport(vector3d position)
    Immediately move the character to \a position without checking for collisions.
    The caller is responsible for avoiding overlap with static objects.
*/

QCharacterController::QCharacterController() = default;

const QVector3D &QCharacterController::speed() const
{
    return m_speed;
}

void QCharacterController::setSpeed(const QVector3D &newSpeed)
{
    if (m_speed == newSpeed)
        return;
    m_speed = newSpeed;
    emit speedChanged();
}

const QVector3D &QCharacterController::gravity() const
{
    return m_gravity;
}

void QCharacterController::setGravity(const QVector3D &newGravity)
{
    if (m_gravity == newGravity)
        return;
    m_gravity = newGravity;
    emit gravityChanged();
}

// Calculate move based on speed/gravity (later: also implement teleport)

QVector3D QCharacterController::getMovement(float deltaTime)
{
    // movement based on speed()
    QVector3D movement = sceneRotation() * m_speed * deltaTime;

    // modified based on gravity
    const auto g = m_gravity;
    if (!g.isNull()) {
        bool freeFalling = m_collisions == Collision::None;

        if (freeFalling) {
            if (!m_midAirControl)
                movement = {}; // Ignore the speed() controls in true free fall

            movement += m_freeFallVelocity * deltaTime;
            m_freeFallVelocity += g * deltaTime;
        } else {
            m_freeFallVelocity = movement / deltaTime + g * deltaTime;
            if (m_midAirControl) // free fall only straight down
                m_freeFallVelocity =
                        QVector3D::dotProduct(m_freeFallVelocity, g.normalized()) * g.normalized();
        }
        const QVector3D gravityAcceleration = 0.5 * deltaTime * deltaTime * g;
        movement += gravityAcceleration; // always add gravitational acceleration, in case we start
                                         // to fall
    }

    return movement;
}

bool QCharacterController::midAirControl() const
{
    return m_midAirControl;
}

void QCharacterController::setMidAirControl(bool newMidAirControl)
{
    if (m_midAirControl == newMidAirControl)
        return;
    m_midAirControl = newMidAirControl;
    emit midAirControlChanged();
}

void QCharacterController::teleport(const QVector3D &position)
{
    m_teleport = true;
    m_teleportPosition = position;
    m_freeFallVelocity = {};
}

bool QCharacterController::getTeleport(QVector3D &position)
{
    if (m_teleport) {
        position = m_teleportPosition;
        m_teleport = false;
        return true;
    }
    return false;
}

const QCharacterController::Collisions &QCharacterController::collisions() const
{
    return m_collisions;
}

void QCharacterController::setCollisions(const Collisions &newCollisions)
{
    if (m_collisions == newCollisions)
        return;
    m_collisions = newCollisions;
    emit collisionsChanged();
}

QT_END_NAMESPACE
