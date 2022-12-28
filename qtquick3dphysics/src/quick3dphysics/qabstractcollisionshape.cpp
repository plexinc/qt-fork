// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qabstractcollisionshape_p.h"

#include <QtQml/QQmlListReference>

#include "qdynamicsworld_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype CollisionShape
    \inherits Node
    \inqmlmodule QtQuick3DPhysics
    \since 6.4
    \brief Base type for collision shapes.

    This is the base type for all collision shapes.
*/

/*!
    \qmlproperty bool CollisionNode::enableDebugView
    This property enables drawing the shape's debug view.
*/

QAbstractCollisionShape::QAbstractCollisionShape(QQuick3DNode *parent) : QQuick3DNode(parent)
{
    connect(this, &QQuick3DNode::sceneScaleChanged, this,
            &QAbstractCollisionShape::handleScaleChange);
}

QAbstractCollisionShape::~QAbstractCollisionShape() = default;

bool QAbstractCollisionShape::enableDebugView() const
{
    return m_enableDebugView;
}

void QAbstractCollisionShape::setEnableDebugView(bool enableDebugView)
{
    if (m_enableDebugView == enableDebugView)
        return;

    if (auto world = QDynamicsWorld::getWorld(); world != nullptr && enableDebugView)
        world->setHasIndividualDebugView();

    m_enableDebugView = enableDebugView;
    emit enableDebugViewChanged(m_enableDebugView);
}

void QAbstractCollisionShape::handleScaleChange()
{
    auto newScale = sceneScale();
    if (!qFuzzyCompare(newScale, m_prevScale)) {
        m_prevScale = newScale;
        m_scaleDirty = true;
        emit needsRebuild(this); // TODO: remove signal argument?
    }
}

QT_END_NAMESPACE
