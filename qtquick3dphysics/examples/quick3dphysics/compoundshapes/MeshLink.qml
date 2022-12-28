// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause


import QtQuick
import QtQuick3D
import QtQuick3D.Physics

//! [meshlink]
DynamicRigidBody {
    scale: Qt.vector3d(100, 100, 100)
    property color color: "white"
    PrincipledMaterial {
        id: _material
        baseColor: color
        metalness: 1.0
        roughness: 0.5
    }

    Model {
        source: "meshes/ring.mesh"
        materials: [
            _material
        ]
    }

    collisionShapes: [
        ConvexMeshShape {
            meshSource: "meshes/segmentedRing_001.mesh"
        },
        ConvexMeshShape {
            meshSource: "meshes/segmentedRing_002.mesh"
        },
        ConvexMeshShape {
            meshSource: "meshes/segmentedRing_003.mesh"
        },
        ConvexMeshShape {
            meshSource: "meshes/segmentedRing_004.mesh"
        },
        ConvexMeshShape {
            meshSource: "meshes/segmentedRing_005.mesh"
        },
        ConvexMeshShape {
            meshSource: "meshes/segmentedRing_006.mesh"
        },
        ConvexMeshShape {
            meshSource: "meshes/segmentedRing_007.mesh"
        },
        ConvexMeshShape {
            meshSource: "meshes/segmentedRing_008.mesh"
        },
        ConvexMeshShape {
            meshSource: "meshes/segmentedRing_009.mesh"
        },
        ConvexMeshShape {
            meshSource: "meshes/segmentedRing_010.mesh"
        },
        ConvexMeshShape {
            meshSource: "meshes/segmentedRing_011.mesh"
        },
        ConvexMeshShape {
            meshSource: "meshes/segmentedRing_012.mesh"
        }
    ]
}
//! [meshlink]
