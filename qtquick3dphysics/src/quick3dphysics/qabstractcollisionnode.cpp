// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qabstractcollisionnode_p.h"
#include <QtQuick3D/private/qquick3dobject_p.h>
#include <foundation/PxTransform.h>

#include "qdynamicsworld_p.h"
QT_BEGIN_NAMESPACE

/*!
    \qmltype CollisionNode
    \inherits Node
    \inqmlmodule QtQuick3DPhysics
    \since 6.4
    \brief Base type for physics nodes.

    This is the base type for all physical (colliding) bodies.
*/

/*!
    \qmlproperty list<CollisionShape> CollisionNode::collisionShapes

    This property contains the list of collision shapes. These shapes will be combined and act as a
    single rigid body when colliding.
*/

/*!
    \qmlproperty bool CollisionNode::sendContactReports
    This property determines whether this body will send contact reports when colliding with other
    bodies.
*/

/*!
    \qmlproperty bool CollisionNode::receiveContactReports
    This property determines whether this body will receive contact reports when colliding with
    other bodies. If activated, this means that the bodyContact signal will be emitted on a
    collision with a body that has sendContactReports set to true.
*/

/*!
    \qmlproperty bool CollisionNode::enableTriggerReports
    This property determines whether this body will report when entering or leaving a trigger body.
*/

/*!
    \qmlsignal CollisionNode::bodyContact(CollisionNode *body, list<vector3D> positions,
   list<vector3D> impulses, list<vector3D> normals)

    This signal is emitted when there is a collision and \l {CollisionNode::}
    {receiveContactReports} is set to \c true in this body and \l {CollisionNode::}
    {sendContactReports} is set to \c true in the colliding \a body. The parameters \a positions, \a
    impulses and \a normals contain the position, impulse force and normal for each contact point at
    the same index.
*/

QAbstractCollisionNode::QAbstractCollisionNode()
{
    if (auto world = QDynamicsWorld::getWorld(); world != nullptr)
        world->registerNode(this);
}

QAbstractCollisionNode::~QAbstractCollisionNode()
{
    if (auto world = QDynamicsWorld::getWorld(); world != nullptr)
        world->deregisterNode(this);
}

QQmlListProperty<QAbstractCollisionShape> QAbstractCollisionNode::collisionShapes()
{
    return QQmlListProperty<QAbstractCollisionShape>(
            this, nullptr, QAbstractCollisionNode::qmlAppendShape,
            QAbstractCollisionNode::qmlShapeCount, QAbstractCollisionNode::qmlShapeAt,
            QAbstractCollisionNode::qmlClearShapes);
}

const QVector<QAbstractCollisionShape *> &QAbstractCollisionNode::getCollisionShapesList() const
{
    return m_collisionShapes;
}

void QAbstractCollisionNode::updateFromPhysicsTransform(const physx::PxTransform &transform)
{
    const auto pos = transform.p;
    const auto rotation = transform.q;
    const auto qtPosition = QVector3D(pos.x, pos.y, pos.z);
    const auto qtRotation = QQuaternion(rotation.w, rotation.x, rotation.y, rotation.z);

    // Get this nodes parent transform
    const QQuick3DNode *parentNode = static_cast<QQuick3DNode *>(parentItem());

    if (!parentNode) {
        // then it is the same space
        setRotation(qtRotation);
        setPosition(qtPosition);
    } else {
        setPosition(parentNode->mapPositionFromScene(qtPosition));
        const auto relativeRotation = parentNode->sceneRotation().inverted() * qtRotation;
        setRotation(relativeRotation);
    }
}

bool QAbstractCollisionNode::sendContactReports() const
{
    return m_sendContactReports;
}

void QAbstractCollisionNode::setSendContactReports(bool sendContactReports)
{
    if (m_sendContactReports == sendContactReports)
        return;

    m_sendContactReports = sendContactReports;
    emit sendContactReportsChanged(m_sendContactReports);
}

bool QAbstractCollisionNode::receiveContactReports() const
{
    return m_receiveContactReports;
}

void QAbstractCollisionNode::setReceiveContactReports(bool receiveContactReports)
{
    if (m_receiveContactReports == receiveContactReports)
        return;

    m_receiveContactReports = receiveContactReports;
    emit receiveContactReportsChanged(m_receiveContactReports);
}

bool QAbstractCollisionNode::enableTriggerReports() const
{
    return m_enableTriggerReports;
}

void QAbstractCollisionNode::setEnableTriggerReports(bool enableTriggerReports)
{
    if (m_enableTriggerReports == enableTriggerReports)
        return;

    m_enableTriggerReports = enableTriggerReports;
    emit enableTriggerReportsChanged(m_enableTriggerReports);
}

void QAbstractCollisionNode::registerContact(QAbstractCollisionNode *body,
                                             const QVector<QVector3D> &positions,
                                             const QVector<QVector3D> &impulses,
                                             const QVector<QVector3D> &normals)
{
    emit bodyContact(body, positions, impulses, normals);
}

void QAbstractCollisionNode::onShapeDestroyed(QObject *object)
{
    m_collisionShapes.removeAll(static_cast<QAbstractCollisionShape *>(object));
}

void QAbstractCollisionNode::onShapeNeedsRebuild(QObject * /*object*/)
{
    m_shapesDirty = true;
}

void QAbstractCollisionNode::qmlAppendShape(QQmlListProperty<QAbstractCollisionShape> *list,
                                            QAbstractCollisionShape *shape)
{
    if (shape == nullptr)
        return;
    QAbstractCollisionNode *self = static_cast<QAbstractCollisionNode *>(list->object);
    self->m_collisionShapes.push_back(shape);
    self->m_hasStaticShapes = self->m_hasStaticShapes || shape->isStaticShape();

    if (shape->parentItem() == nullptr) {
        // If the material has no parent, check if it has a hierarchical parent that's a
        // QQuick3DObject and re-parent it to that, e.g., inline materials
        QQuick3DObject *parentItem = qobject_cast<QQuick3DObject *>(shape->parent());
        if (parentItem) {
            shape->setParentItem(parentItem);
        } else { // If no valid parent was found, make sure the material refs our scene manager
            const auto &scenManager = QQuick3DObjectPrivate::get(self)->sceneManager;
            if (scenManager)
                QQuick3DObjectPrivate::refSceneManager(shape, *scenManager);
            // else: If there's no scene manager, defer until one is set, see itemChange()
        }
    }

    // Make sure materials are removed when destroyed
    connect(shape, &QAbstractCollisionShape::destroyed, self,
            &QAbstractCollisionNode::onShapeDestroyed);

    // Connect to rebuild signal
    connect(shape, &QAbstractCollisionShape::needsRebuild, self,
            &QAbstractCollisionNode::onShapeNeedsRebuild);
}

QAbstractCollisionShape *
QAbstractCollisionNode::qmlShapeAt(QQmlListProperty<QAbstractCollisionShape> *list, qsizetype index)
{
    QAbstractCollisionNode *self = static_cast<QAbstractCollisionNode *>(list->object);
    return self->m_collisionShapes.at(index);
}

qsizetype QAbstractCollisionNode::qmlShapeCount(QQmlListProperty<QAbstractCollisionShape> *list)
{
    QAbstractCollisionNode *self = static_cast<QAbstractCollisionNode *>(list->object);
    return self->m_collisionShapes.count();
}

void QAbstractCollisionNode::qmlClearShapes(QQmlListProperty<QAbstractCollisionShape> *list)
{
    QAbstractCollisionNode *self = static_cast<QAbstractCollisionNode *>(list->object);
    for (const auto &shape : qAsConst(self->m_collisionShapes)) {
        if (shape->parentItem() == nullptr)
            QQuick3DObjectPrivate::get(shape)->derefSceneManager();
        shape->disconnect(self, SLOT(onMaterialDestroyed(QObject *)));
    }
    self->m_hasStaticShapes = false;
    self->m_collisionShapes.clear();
}

QT_END_NAMESPACE
