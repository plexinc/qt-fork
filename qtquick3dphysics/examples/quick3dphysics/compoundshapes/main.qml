// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Physics
import QtQuick3D.Helpers
import QtQuick.Controls
import QtQuick.Layouts

Window {
    width: 1280
    height: 720
    visible: true
    title: qsTr("Compound Shapes Example")

    //! [world]
    DynamicsWorld {
        id: physicsWorld
        running: false
        enableCCD: true
    }
    //! [world]

    Timer {
        running: true
        interval: 5000
        onTriggered: physicsWorld.running = true
    }

    View3D {
        id: viewport
        property real ringY : 900
        property real ringDistance : 135
        anchors.fill: parent

        //! [environment]
        environment: SceneEnvironment {
            antialiasingMode: SceneEnvironment.MSAA
            backgroundMode: SceneEnvironment.Color
            clearColor: "lightblue"
        }

        focus: true

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(-200, 900, 1300)
            eulerRotation: Qt.vector3d(-10, 0, 0)
            clipFar: 15500
            clipNear: 1
        }

        DirectionalLight {
            eulerRotation.x: -45
            eulerRotation.y: 45
            castsShadow: true
            brightness: 1.5
            shadowFactor: 15
            shadowFilter: 10
            shadowMapFar: 100
            shadowBias: -0.01
            shadowMapQuality: Light.ShadowMapQualityVeryHigh
        }
        //! [environment]

        //! [plane]
        StaticRigidBody {
            position: Qt.vector3d(0, -100, 0)
            eulerRotation: Qt.vector3d(-90, 0, 0)
            collisionShapes: PlaneShape {}
            Model {
                source: "#Rectangle"
                scale: Qt.vector3d(500, 500, 0)
                materials: DefaultMaterial {
                    diffuseColor: "green"
                }
                castsShadows: false
                receivesShadows: true
            }
        }
        //! [plane]

        //! [link]
        MeshLink {
            isKinematic: true
            position: Qt.vector3d(-6 * viewport.ringDistance, viewport.ringY, 0)
            eulerRotation: Qt.vector3d(90, 0, 0)
            color: "red"

            SequentialAnimation on x {
                running: physicsWorld.running
                PauseAnimation {
                    duration: 7000
                }
                NumberAnimation {
                    from: -6 * viewport.ringDistance
                    to: -9 * viewport.ringDistance
                    duration: 3000
                    easing.type: Easing.InOutQuad
                }

                SequentialAnimation {
                    NumberAnimation {
                        from: -9 * viewport.ringDistance
                        to: 3 * viewport.ringDistance
                        easing.type: Easing.InOutCubic
                        duration: 8000
                    }
                    NumberAnimation {
                        to: -9 * viewport.ringDistance
                        from: 3 * viewport.ringDistance
                        easing.type: Easing.InOutCubic
                        duration: 8000
                    }
                    loops: Animation.Infinite
                }
            }
        }

        CapsuleLink {
            position: Qt.vector3d(-5 * viewport.ringDistance, viewport.ringY, 0)
            eulerRotation: Qt.vector3d(90, 0, 0)
        }

        MeshLink {
            position: Qt.vector3d(-4 * viewport.ringDistance, viewport.ringY, 0)
            eulerRotation: Qt.vector3d(90, 0, 0)
        }

        MeshLink {
            position: Qt.vector3d(-3 * viewport.ringDistance, viewport.ringY, 0)
            eulerRotation: Qt.vector3d(0, 90, 0)
        }

        MeshLink {
            position: Qt.vector3d(-2 * viewport.ringDistance, viewport.ringY, 0)
            eulerRotation: Qt.vector3d(90, 0, 0)
        }

        MeshLink {
            position: Qt.vector3d(-1 * viewport.ringDistance, viewport.ringY, 0)
            eulerRotation: Qt.vector3d(0, 90, 0)
        }

        CapsuleLink {
            position: Qt.vector3d(0, viewport.ringY, 0)
        }

        MeshLink {
            position: Qt.vector3d(1 * viewport.ringDistance, viewport.ringY, 0)
            eulerRotation: Qt.vector3d(0, 90, 0)
        }

        MeshLink {
            position: Qt.vector3d(2 * viewport.ringDistance, viewport.ringY, 0)
            eulerRotation: Qt.vector3d(90, 0, 0)
        }

        MeshLink {
            position: Qt.vector3d(3 * viewport.ringDistance, viewport.ringY, 0)
            eulerRotation: Qt.vector3d(0, 90, 0)
        }

        MeshLink {
            position: Qt.vector3d(4 * viewport.ringDistance, viewport.ringY, 0)
            eulerRotation: Qt.vector3d(90, 0, 0)
        }

        CapsuleLink {
            position: Qt.vector3d(5 * viewport.ringDistance, viewport.ringY, 0)
            eulerRotation: Qt.vector3d(90, 0, 0)
        }

        MeshLink {
            isKinematic: true
            position: Qt.vector3d(6 * viewport.ringDistance, viewport.ringY, 0)
            eulerRotation: Qt.vector3d(90, 0, 0)
            color: "red"
        }
        //! [link]
    }

    WasdController {
        controlledObject: camera
        speed: 0.02
        keysEnabled: true
        Keys.onPressed: (event)=> {
            if (keysEnabled) {handleKeyPress(event);}
            if (event.key === Qt.Key_Space) {
                physicsWorld.running = true
            } else if (event.key === Qt.Key_G) {
                physicsWorld.forceDebugView = !physicsWorld.forceDebugView
            }
        }
        Keys.onReleased: (event) => { if (keysEnabled) handleKeyRelease(event) }

    }
}
