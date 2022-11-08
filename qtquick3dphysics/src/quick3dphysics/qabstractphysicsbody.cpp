// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qabstractphysicsbody_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype PhysicsBody
    \inherits CollisionNode
    \inqmlmodule QtQuick3DPhysics
    \since 6.4
    \brief Base type for physics bodies.

    This is the base type for all physics bodies.
*/

/*!
    \qmlproperty PhysicsMaterial PhysicsBody::physicsMaterial
    This property defines the physical material of the body
*/

QAbstractPhysicsBody::QAbstractPhysicsBody()
{
    m_physicsMaterial = new QPhysicsMaterial(this);
}

QPhysicsMaterial *QAbstractPhysicsBody::physicsMaterial() const
{
    return m_physicsMaterial;
}

void QAbstractPhysicsBody::setPhysicsMaterial(QPhysicsMaterial *newPhysicsMaterial)
{
    if (m_physicsMaterial == newPhysicsMaterial)
        return;
    m_physicsMaterial = newPhysicsMaterial;
    emit physicsMaterialChanged();
}

QT_END_NAMESPACE
