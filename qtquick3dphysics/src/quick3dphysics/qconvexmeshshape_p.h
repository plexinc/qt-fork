// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef CONVEXMESHSHAPE_H
#define CONVEXMESHSHAPE_H

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
#include <QtQuick3DPhysics/private/qabstractcollisionshape_p.h>
#include <QtCore/QObject>
#include <QtGui/QVector3D>
#include <QtQml/QQmlEngine>

namespace physx {
class PxBoxGeometry;
class PxConvexMesh;
class PxConvexMeshGeometry;
}

QT_BEGIN_NAMESPACE
class QQuick3DPhysicsMesh;

class Q_QUICK3DPHYSICS_EXPORT QConvexMeshShape : public QAbstractCollisionShape
{
    Q_OBJECT
    Q_PROPERTY(QUrl meshSource READ meshSource WRITE setMeshSource NOTIFY meshSourceChanged)
    QML_NAMED_ELEMENT(ConvexMeshShape)
public:
    QConvexMeshShape();
    ~QConvexMeshShape();

    physx::PxGeometry *getPhysXGeometry() override;

    const QUrl &meshSource() const;
    void setMeshSource(const QUrl &newMeshSource);
    bool isStaticShape() const override { return false; }

signals:
    void meshSourceChanged();

private:
    void updatePhysXGeometry();

    bool m_dirtyPhysx = false;
    physx::PxConvexMeshGeometry *m_meshGeometry = nullptr;
    QUrl m_meshSource;
    QQuick3DPhysicsMesh *m_mesh = nullptr;
};

QT_END_NAMESPACE

#endif // CONVEXMESHSHAPE_H
