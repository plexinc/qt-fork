// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef ABSTRACTCOLLISIONNODE_H
#define ABSTRACTCOLLISIONNODE_H

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
#include <QtQuick3D/private/qquick3dnode_p.h>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlListProperty>
#include <QtQuick3DPhysics/private/qabstractcollisionshape_p.h>

namespace physx {
class PxTransform;
class PxShape;
}

QT_BEGIN_NAMESPACE

class QAbstractPhysXNode;

class Q_QUICK3DPHYSICS_EXPORT QAbstractCollisionNode : public QQuick3DNode
{
    Q_OBJECT
    Q_PROPERTY(
            QQmlListProperty<QAbstractCollisionShape> collisionShapes READ collisionShapes CONSTANT)
    Q_PROPERTY(bool sendContactReports READ sendContactReports WRITE setSendContactReports NOTIFY
                       sendContactReportsChanged)
    Q_PROPERTY(bool receiveContactReports READ receiveContactReports WRITE setReceiveContactReports
                       NOTIFY receiveContactReportsChanged)
    Q_PROPERTY(bool enableTriggerReports READ enableTriggerReports WRITE setEnableTriggerReports
                       NOTIFY enableTriggerReportsChanged)

    QML_NAMED_ELEMENT(CollisionNode)
    QML_UNCREATABLE("abstract interface")
public:
    QAbstractCollisionNode();
    ~QAbstractCollisionNode() override;

    QQmlListProperty<QAbstractCollisionShape> collisionShapes();
    const QVector<QAbstractCollisionShape *> &getCollisionShapesList() const;

    void updateFromPhysicsTransform(const physx::PxTransform &transform);

    void registerContact(QAbstractCollisionNode *body, const QVector<QVector3D> &positions,
                         const QVector<QVector3D> &impulses, const QVector<QVector3D> &normals);

    bool sendContactReports() const;
    void setSendContactReports(bool sendContactReports);

    bool receiveContactReports() const;
    void setReceiveContactReports(bool receiveContactReports);

    bool enableTriggerReports() const;
    void setEnableTriggerReports(bool enableTriggerReports);

    bool hasStaticShapes() const { return m_hasStaticShapes; }

private Q_SLOTS:
    void onShapeDestroyed(QObject *object);
    void onShapeNeedsRebuild(QObject *object);

Q_SIGNALS:
    void bodyContact(QAbstractCollisionNode *body, const QVector<QVector3D> &positions,
                     const QVector<QVector3D> &impulses, const QVector<QVector3D> &normals);
    void sendContactReportsChanged(float sendContactReports);
    void receiveContactReportsChanged(float receiveContactReports);
    void enableTriggerReportsChanged(float enableTriggerReports);

private:
    static void qmlAppendShape(QQmlListProperty<QAbstractCollisionShape> *list,
                               QAbstractCollisionShape *shape);
    static QAbstractCollisionShape *qmlShapeAt(QQmlListProperty<QAbstractCollisionShape> *list,
                                               qsizetype index);
    static qsizetype qmlShapeCount(QQmlListProperty<QAbstractCollisionShape> *list);
    static void qmlClearShapes(QQmlListProperty<QAbstractCollisionShape> *list);

    QVector<QAbstractCollisionShape *> m_collisionShapes;
    bool m_shapesDirty = false;
    bool m_sendContactReports = false;
    bool m_receiveContactReports = false;
    bool m_enableTriggerReports = false;
    bool m_hasStaticShapes = false;

    friend class QAbstractPhysXNode;
    friend class QDynamicsWorld; // for register/deregister TODO: cleaner mechanism
    QAbstractPhysXNode *m_backendObject = nullptr;
};

QT_END_NAMESPACE

#endif // ABSTRACTCOLLISIONNODE_H
