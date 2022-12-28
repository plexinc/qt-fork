// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qplaneshape_p.h"

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
    \qmltype PlaneShape
    \inqmlmodule QtQuick3DPhysics
    \inherits CollisionShape
    \since 6.4
    \brief Plane shape.

    This is the plane shape. Planes divide space into "above" and "below" them. Everything "below"
    the plane will collide with it and be pushed above it. The Plane lies on the XY plane with
    "above" pointing towards positive Z.
*/

QPlaneShape::QPlaneShape() = default;

QPlaneShape::~QPlaneShape()
{
    delete m_planeGeometry;
}

physx::PxGeometry *QPlaneShape::getPhysXGeometry()
{
    if (!m_planeGeometry) {
        updatePhysXGeometry();
    }
    return m_planeGeometry;
}

void QPlaneShape::updatePhysXGeometry()
{
    delete m_planeGeometry;
    // TODO: we need only one plane geometry, and it should live in the backend
    m_planeGeometry = new physx::PxPlaneGeometry();
}

QT_END_NAMESPACE
