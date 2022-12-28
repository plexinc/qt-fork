// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
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
    title: qsTr("PhysX Physics Demo")

    DynamicsWorld {
        id: physicsWorld
        gravity: Qt.vector3d(0, -9.81, 0)
        running: false
        forceDebugView: false
        typicalLength: 1
        typicalSpeed: 10
    }

    View3D {
        id: viewport
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#d6dbdf"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        focus: true

        property bool debugView: false

        Node {
            id: scene1
            PerspectiveCamera {
                id: camera1
                position: Qt.vector3d(1, 3, 7)
                eulerRotation.x: -30
                eulerRotation.y: 10
                clipFar: 100
                clipNear: 0.01
            }

            DirectionalLight {
                eulerRotation.x: -45
                eulerRotation.y: 45
                castsShadow: true
                brightness: 1
                shadowMapQuality: Light.ShadowMapQualityVeryHigh
            }

            StaticRigidBody {
                id: floor
                position: Qt.vector3d(0, -1, 0)
                eulerRotation: Qt.vector3d(-90, 0, 0)
                collisionShapes: PlaneShape {
                    enableDebugView: viewport.debugView
                }

                Model {
                    source: "#Rectangle"
                    scale: Qt.vector3d(0.3, 0.3, 0.3)
                    materials: DefaultMaterial {
                        diffuseColor: "green"
                    }
                    castsShadows: false
                    receivesShadows: true
                }
            }

            StaticRigidBody {
                id: wall
                position: Qt.vector3d(-4, 0, 0)
                eulerRotation: Qt.vector3d(0, 90, 0)

                collisionShapes: PlaneShape {
                    id: wallShape
                    enableDebugView: viewport.debugView
                }
                Model {
                    source: "#Rectangle"
                    scale: Qt.vector3d(0.3, 0.3, 0.3)
                    materials: PrincipledMaterial {
                        baseColor: "gray"
                        cullMode: Material.NoCulling

                    }
                    opacity: 0.3
                }
            }

            DynamicRigidBody {
                id: dynamicTorus
                position: Qt.vector3d(0.5, 3.5, 1.0)
                property bool inArea: false

                mass: 10 // kg
                scale: Qt.vector3d(0.01, 0.01, 0.01).times(scaleSlider.value)
                Model {
                    source: "meshes/newTorus.mesh"
                    materials: PrincipledMaterial {
                        baseColor: "orange"
                    }
                }
                collisionShapes: ConvexMeshShape {
                    id: torusShape
                    meshSource: "meshes/newConvexTorus.mesh"
                    enableDebugView: viewport.debugView
                }
            }

            DynamicRigidBody {
                id: kinematicTorus
                position: Qt.vector3d(-0.5, 2, 0.5)
                property bool inArea: false

                scale: Qt.vector3d(0.015, 0.015, 0.015).times(scaleSlider.value)
                mass: 10 // kg
                Model {
                    source: "meshes/newTorus.mesh"
                    materials: PrincipledMaterial {
                        baseColor: "lightGray"
                        metalness: 1.0
                        roughness: 0.5
                    }
                }
                collisionShapes: TriangleMeshShape {
                    id: torusShape2
                    meshSource: "meshes/newTorus.mesh"
                    enableDebugView: viewport.debugView
                }
                isKinematic: true
            }

            DynamicRigidBody {
                id: dynamicTorusConvexHullOfNonConvexMesh
                position: Qt.vector3d(2, 3, 1.5)
                property bool inArea: false
                scale: Qt.vector3d(0.01, 0.01, 0.01)

                mass: 10 // kg
                Model {
                    source: "meshes/newTorus.mesh"
                    materials: PrincipledMaterial {
                        baseColor: "#c07030"
                        metalness: 1.0
                        roughness: 0.5
                    }
                }
                collisionShapes: ConvexMeshShape {
                    id: torusShape3
                    meshSource: "meshes/newTorus.mesh"
                    enableDebugView: viewport.debugView
                }
            }



            // x-axis reference
            Model {
                materials: PrincipledMaterial {
                    baseColor: "red"
                    roughness: 1
                }
                source: "#Cube"
                scale: "0.002, 0.002, 0.002"
                position: "-0.5, 2, 4"
            }

            Model {
                materials: PrincipledMaterial {
                    baseColor: "green"
                    roughness: 1
                }
                source: "#Cube"
                scale: "0.002, 0.002, 0.002"
                position: "2, 2, 4"
            }

            Model {
                materials: PrincipledMaterial {
                    baseColor: "blue"
                    roughness: 1
                }
                source: "#Cube"
                scale: "0.002, 0.002, 0.002"
                position: "4.5, 2, 4"
            }


            StaticRigidBody {
                position: Qt.vector3d(2, 2, 1.5)
                id: hfTest

                Model {
                    id: hfModel
                    geometry: HeightFieldGeometry {
                        extents: Qt.vector3d(5, 2, 5).times(scaleSlider.value)
                        heightMap: "maps/heightfield.png"
                        smoothShading: false
                    }
                    materials: PrincipledMaterial {
                        baseColor: "#ff77ff"
                        roughness: 0.3
                    }
                    opacity: 0.6
                }

                collisionShapes: HeightFieldShape {
                    id: hfShape
                    extents: Qt.vector3d(5, 2, 5).times(scaleSlider.value)
                    heightMap: "maps/heightfield.png"
                    enableDebugView: viewport.debugView
                }
            }

            SequentialAnimation {
                running: animateCheckBox.checked
                loops: -1
                NumberAnimation {
                    target: hfShape
                    property: "position.x"
                    duration: 2000
                    from: 0
                    to: -2
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    target: hfShape
                    property: "position.x"
                    duration: 2000
                    from: -2
                    to: 0
                    easing.type: Easing.InOutQuad
                }
            }

            SequentialAnimation {
                running: animateCheckBox.checked
                loops: -1
                NumberAnimation {
                    target: hfModel
                    property: "position.x"
                    duration: 2000
                    from: 0
                    to: -2
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    target: hfModel
                    property: "position.x"
                    duration: 2000
                    from: -2
                    to: 0
                    easing.type: Easing.InOutQuad
                }
            }

            StaticRigidBody {
                position: Qt.vector3d(0, 0.6, 0)
                id: triangleMeshTest
                property bool inArea: false

                scale: Qt.vector3d(0.015, 0.01, 0.015).times(scaleSlider.value)
                Model {
                    id: testModel
                    source: "meshes/field.mesh"
                    materials: PrincipledMaterial {
                        baseColor: "#5070a0"
                        roughness: 0.3
                    }
                    opacity: 1
                }
                collisionShapes: TriangleMeshShape {
                    id: triShape
                    meshSource: "meshes/field.mesh"
                    enableDebugView: viewport.debugView
                }
            }


            SequentialAnimation {
                running: animateCheckBox.checked
                loops: -1
                NumberAnimation {
                    target: triangleMeshTest
                    property: "eulerRotation.y"
                    duration: 5000
                    from: 180
                    to: -180
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    target: triangleMeshTest
                    property: "eulerRotation.y"
                    duration: 5000
                    from: -180
                    to: 180
                    easing.type: Easing.InOutQuad
                }
            }


//            DynamicRigidBody {
//                id: table
//                y: -50

//                isKinematic: true

//                Model {
//                    id: tableModel
//                    source: "#Cube"
//                    materials: DefaultMaterial {
//                        diffuseColor: "gray"
//                    }
//                    scale: Qt.vector3d(4, 1, 4)
//                }
//                collisionShapes: BoxShape {
//                    id: tableShape
//                    extents: Qt.vector3d(200, 50, 200)
//                }
//            }

            Texture {
                id: qtLogoTexture
                source: "qt_logo_rect.png"
                scaleV: 0.5
                positionV: 0.2
            }
            Component {
                id: doughnutComponent

                Node {
                    id: thisNode
                    function randomInRange(min, max) {
                        return Math.random() * (max - min) + min;
                    }
                    eulerRotation: Qt.vector3d(randomInRange(0, 360),
                                          randomInRange(0, 360),
                                          randomInRange(0, 360))

                    position: Qt.vector3d(randomInRange(-1.5, 1.5),
                                          randomInRange( 4, 30),
                                          randomInRange(-1.5, 1.5))

                    property real sf: randomInRange(0.2, 0.45)
                    property color baseCol: Qt.hsla(randomInRange(0, 1), randomInRange(0.3, 0.7), randomInRange(0.4, 0.7), 1.0)
                    property bool grabbed: false
                    DynamicRigidBody {
                        id: thisBody
                        mass: 1 //heavy donut
                        isKinematic: thisNode.grabbed
                        Model {
                            id: thisModel
                            source: "#Sphere"//"meshes/newTorus.mesh"
                            materials: PrincipledMaterial {
                                metalness: 1.0
                                roughness: randomInRange(0.3, 0.6)
                                baseColor: thisNode.grabbed ? "white" : baseCol
                                emissiveMap: qtLogoTexture
                                emissiveFactor: "0.2, 0.2, 0.2"
                                heightAmount: 0.5
                            }

                            scale: Qt.vector3d(sf, sf, sf).times(0.01)
                            pickable: true
                            property Node proxyNode: thisNode
                        }
//                        collisionShapes: ConvexMeshShape {
//                            id: torusShape
//                            meshSource: "meshes/newConvexTorus.mesh"
//                            meshScale: Qt.vector3d(sf, sf, sf)
//                        }
                        collisionShapes: SphereShape {
                            diameter: sf
                            enableDebugView: viewport.debugView
                        }

                    }
                }
            }

            Repeater3D {
                id: rep
                model: 400
                delegate: doughnutComponent
            }

        } // scene1
    } // View3D


    WasdController {
        keysEnabled: true
        controlledObject: camera1
        speed: 0.02

        Keys.onPressed: (event)=> {
            if (keysEnabled) handleKeyPress(event);
            if (event.key === Qt.Key_Space) {
                physicsWorld.running = !physicsWorld.running
            } else if (event.key === Qt.Key_G) {
                viewport.debugView = !viewport.debugView
            } else if (event.key === Qt.Key_H) {
                physicsWorld.forceDebugView = !physicsWorld.forceDebugView
            } else if (event.key === Qt.Key_T) {
                tableShape.extents = "150, 100, 150"
                tableModel.scale = "3, 2, 3"
            }
        }
        Keys.onReleased: (event)=> { if (keysEnabled) handleKeyRelease(event) }
    }

    MouseArea {
         anchors.fill: viewport
         acceptedButtons: Qt.RightButton
         property Node grabbedNode: null
         onPressed: (mouse) =>
         {
             var result = viewport.pick(mouse.x, mouse.y);
             if (result.objectHit) {
                 var pickedNode = result.objectHit.proxyNode;
                 console.log("PRESSED:", pickedNode)

                 pickedNode.grabbed = true
                 //pickedNode.y += 100
                 grabbedNode = pickedNode
             }
         }
         onReleased:
         {
             if (grabbedNode) {
                 grabbedNode.grabbed = false
                 grabbedNode = null
             }
         }
    }


    ColumnLayout {
        Label {
            text: "Debug view: " + viewport.debugView
        }
        Label {
            text: "Debug view (forced): " + physicsWorld.forceDebugView
        }
        Label {
            text: "Scale: " + scaleSlider.value
        }
        Slider {
            id: scaleSlider
            focusPolicy: Qt.NoFocus
            from: 0.5
            to: 1.5
            value: 1
            stepSize: 0.1
        }
        CheckBox {
            id: animateCheckBox
            checked: false
            text: qsTr("Animate")
        }
    }


}
