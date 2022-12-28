// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TRIGGERBODY_H
#define TRIGGERBODY_H

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
#include <QtQuick3DPhysics/private/qabstractcollisionnode_p.h>
#include <QtQml/QQmlEngine>

QT_BEGIN_NAMESPACE

class Q_QUICK3DPHYSICS_EXPORT QTriggerBody : public QAbstractCollisionNode
{
    Q_OBJECT
    Q_PROPERTY(int collisionCount READ collisionCount NOTIFY collisionCountChanged)
    QML_NAMED_ELEMENT(TriggerBody)
public:
    QTriggerBody();

    void registerCollision(QAbstractCollisionNode *collision);
    void deregisterCollision(QAbstractCollisionNode *collision);

    int collisionCount() const;

Q_SIGNALS:
    void bodyEntered(QAbstractCollisionNode *body);
    void bodyExited(QAbstractCollisionNode *body);
    void collisionCountChanged();

private:
    QSet<QAbstractCollisionNode *> m_collisions;
};

QT_END_NAMESPACE

#endif // TRIGGERBODY_H
