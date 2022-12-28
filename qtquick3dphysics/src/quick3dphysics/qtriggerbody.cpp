// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qtriggerbody_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype TriggerBody
    \inherits CollisionNode
    \inqmlmodule QtQuick3DPhysics
    \since 6.4
    \brief Base type for a trigger body.

    This is the base type for a trigger body. A trigger body is a body that does not interact
    physically but is used to detect when objects intersect.
*/

/*!
    \qmlproperty int TriggerBody::collisionCount
    This property returns the number of bodies currently colliding with the trigger body.
*/

/*!
    \qmlsignal TriggerBody::bodyEntered(CollisionNode *body)
    This signal is emitted when the trigger body is penetrated by the specified \a body.
*/

/*!
    \qmlsignal TriggerBody::bodyExited(CollisionNode *body)
    This signal is emitted when the trigger body is no longer penetrated by the specified \a body.
*/

QTriggerBody::QTriggerBody() = default;

void QTriggerBody::registerCollision(QAbstractCollisionNode *collision)
{
    int size = m_collisions.size();
    m_collisions.insert(collision);

    if (size != m_collisions.size()) {
        emit bodyEntered(collision);
        emit collisionCountChanged();
    }
}

void QTriggerBody::deregisterCollision(QAbstractCollisionNode *collision)
{
    int size = m_collisions.size();
    m_collisions.remove(collision);

    if (size != m_collisions.size()) {
        emit bodyExited(collision);
        emit collisionCountChanged();
    }
}

int QTriggerBody::collisionCount() const
{
    return m_collisions.count();
}

QT_END_NAMESPACE
