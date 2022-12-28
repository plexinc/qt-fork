// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QPHYSICSUTILS_P_H
#define QPHYSICSUTILS_P_H

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

#include <QVector3D>
#include <QQuaternion>
#include <foundation/PxMat33.h>
#include <foundation/PxQuat.h>
#include <foundation/PxVec3.h>

namespace physx {
class PxRigidBody;
}

namespace QPhysicsUtils {

Q_ALWAYS_INLINE physx::PxVec3 toPhysXType(const QVector3D &qvec)
{
    return physx::PxVec3(qvec.x(), qvec.y(), qvec.z());
}

Q_ALWAYS_INLINE physx::PxQuat toPhysXType(const QQuaternion &qquat)
{
    return physx::PxQuat(qquat.x(), qquat.y(), qquat.z(), qquat.scalar());
}

Q_ALWAYS_INLINE physx::PxMat33 toPhysXType(const QMatrix3x3 &m)
{
    return physx::PxMat33(const_cast<float *>(m.constData()));
}

Q_ALWAYS_INLINE QVector3D toQtType(const physx::PxVec3 &vec)
{
    return QVector3D(vec.x, vec.y, vec.z);
}

Q_ALWAYS_INLINE QQuaternion toQtType(const physx::PxQuat &quat)
{
    return QQuaternion(quat.w, quat.x, quat.y, quat.z);
}
}

#endif // QPHYSICSUTILS_P_H
