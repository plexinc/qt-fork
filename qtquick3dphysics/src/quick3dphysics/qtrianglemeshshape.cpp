// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qtrianglemeshshape_p.h"

#include <QtQuick3D/QQuick3DGeometry>
#include <extensions/PxExtensionsAPI.h>

#include "qphysicsmeshutils_p_p.h"

//########################################################################################
// NOTE:
// Triangle mesh, heightfield or plane geometry shapes configured as eSIMULATION_SHAPE are
// not supported for non-kinematic PxRigidDynamic instances.
//########################################################################################

#include "qdynamicsworld_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype TriangleMeshShape
    \inqmlmodule QtQuick3DPhysics
    \inherits CollisionShape
    \since 6.4
    \brief Triangle mesh shape.

    This is the triangle mesh shape.
*/

/*!
    \qmlproperty url TriangleMeshShape::meshSource
    This property defines the location of the mesh file used to define the shape.
*/

QTriangleMeshShape::QTriangleMeshShape() = default;

QTriangleMeshShape::~QTriangleMeshShape()
{
    delete m_meshGeometry;
    if (m_mesh)
        QQuick3DPhysicsMeshManager::releaseMesh(m_mesh);
}

physx::PxGeometry *QTriangleMeshShape::getPhysXGeometry()
{
    if (m_dirtyPhysx || m_scaleDirty) {
        updatePhysXGeometry();
    }
    return m_meshGeometry;
}

void QTriangleMeshShape::updatePhysXGeometry()
{
    delete m_meshGeometry;
    m_meshGeometry = nullptr;

    if (!m_mesh)
        return;
    auto *triangleMesh = m_mesh->triangleMesh();
    if (!triangleMesh)
        return;

    auto meshScale = sceneScale();
    physx::PxMeshScale scale(physx::PxVec3(meshScale.x(), meshScale.y(), meshScale.z()),
                             physx::PxQuat(physx::PxIdentity));

    m_meshGeometry = new physx::PxTriangleMeshGeometry(triangleMesh, scale);
    m_dirtyPhysx = false;
}

const QUrl &QTriangleMeshShape::meshSource() const
{
    return m_meshSource;
}

void QTriangleMeshShape::setMeshSource(const QUrl &newMeshSource)
{
    if (m_meshSource == newMeshSource)
        return;
    m_meshSource = newMeshSource;
    m_mesh = QQuick3DPhysicsMeshManager::getMesh(m_meshSource, this);

    updatePhysXGeometry();

    emit needsRebuild(this);
    emit meshSourceChanged();
}

QT_END_NAMESPACE
