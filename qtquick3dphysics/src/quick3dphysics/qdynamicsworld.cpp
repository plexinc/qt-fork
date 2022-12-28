// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qcapsuleshape_p.h"
#include "qdebugdrawhelper_p.h"
#include "qdynamicsworld_p.h"

#include "qabstractcollisionnode_p.h"
#include "qphysicsutils_p.h"
#include "qtriggerbody_p.h"
#include "qrigidbody_p.h"
#include "qplaneshape_p.h"
#include "qphysicscommands_p.h"

#include "PxPhysicsAPI.h"
#include "qcharactercontroller_p.h"
#include "qheightfieldshape_p.h"

#include "cooking/PxCooking.h"

#include "extensions/PxDefaultCpuDispatcher.h"

#include <QtQuick3D/private/qquick3dnode_p.h>
#include <QtQuick3D/private/qquick3dmodel_p.h>
#include <QtQuick3D/private/qquick3ddefaultmaterial_p.h>

#define PHYSX_ENABLE_PVD 0

QT_BEGIN_NAMESPACE

/*!
    \qmltype DynamicsWorld
    \inqmlmodule QtQuick3DPhysics
    \since 6.4
    \brief The dynamics world.

    This is the dynamics world. This node is used to create an instance of the physics world as well
    as define its properties. There can only be one dynamics world. All collision nodes in the qml
    will get added automatically to the dynamics world.
*/

/*!
    \qmlproperty vector3d DynamicsWorld::gravity
    This property defines the gravity vector of the physics world.
    The default value is \c (0, -981, 0). Set the value to \c{Qt.vector3d(0, -9.81, 0)} if your
    unit of measurement is meters and you are simulating Earth gravity.
*/

/*!
    \qmlproperty bool DynamicsWorld::running
    This property starts or stops the physical simulation. The default value is \c true.
*/

/*!
    \qmlproperty bool DynamicsWorld::forceDebugView
    This property enables debug drawing of all active shapes in the physics world. The default value
    is \c false.
*/

/*!
    \qmlproperty bool DynamicsWorld::enableCCD
    This property enables continuous collision detection. This will reduce the risk of bodies going
    through other bodies at high velocities (also known as tunnelling). The default value is \c
    false.
*/

/*!
    \qmlproperty float DynamicsWorld::typicalLength
    This property defines the approximate size of objects in the simulation. This is used to
    estimate certain length-related tolerances. Objects much smaller or much larger than this
    size may not behave properly. The default value is \c 100.
*/

/*!
    \qmlproperty float DynamicsWorld::typicalSpeed
    This property defines the typical magnitude of velocities of objects in simulation. This is used
    to estimate whether a contact should be treated as bouncing or resting based on its impact
    velocity, and a kinetic energy threshold below which the simulation may put objects to sleep.

    For normal physical environments, a good choice is the approximate speed of an object falling
    under gravity for one second. The default value is \c 1000.
*/

/*!
    \qmlproperty float DynamicsWorld::defaultDensity
    This property defines the default density of dynamic objects, measured in kilograms per cubic
    unit. This is equal to the weight of a cube with side \c 1.

    The default value is \c 0.001, corresponding to 1 g/cm³: the density of water. If your unit of
    measurement is meters, a good value would be \c 1000. Note that only positive values are
   allowed.
*/

/*!
    \qmlproperty View3D DynamicsWorld::sceneView
    This property defines the viewport of the scene. If unset when the physical simulation is
    started a View3D will try to be located among the parents of this world. The first and top-most
    View3D found in any of this world's parents will be used.
*/

/*!
    \qmlproperty float DynamicsWorld::minTimestep
    This property defines the minimum simulation timestep in milliseconds. The default value is
    \c 16.667 which corresponds to \c 60 frames per second.
*/

/*!
    \qmlproperty float DynamicsWorld::maxTimestep
    This property defines the maximum simulation timestep in milliseconds. The default value is
    \c 33.333 which corresponds to \c 30 frames per second.
*/

Q_LOGGING_CATEGORY(lcQuick3dPhysics, "qt.quick3d.physics");

static const QQuaternion kMinus90YawRotation = QQuaternion::fromEulerAngles(0, -90, 0);

static inline bool fuzzyEquals(const physx::PxTransform &a, const physx::PxTransform &b)
{
    return qFuzzyCompare(a.p.x, b.p.x) && qFuzzyCompare(a.p.y, b.p.y) && qFuzzyCompare(a.p.z, b.p.z)
            && qFuzzyCompare(a.q.x, b.q.x) && qFuzzyCompare(a.q.y, b.q.y)
            && qFuzzyCompare(a.q.z, b.q.z) && qFuzzyCompare(a.q.w, b.q.w);
}

static physx::PxTransform getPhysXWorldTransform(const QQuick3DNode *node)
{
    const QQuaternion &rotation = node->sceneRotation();
    const QVector3D worldPosition = node->scenePosition();
    return physx::PxTransform(QPhysicsUtils::toPhysXType(worldPosition),
                              QPhysicsUtils::toPhysXType(rotation));
}

static physx::PxTransform getPhysXLocalTransform(const QQuick3DNode *node)
{
    // Modify transforms to make the PhysX shapes match the QtQuick3D conventions
    if (qobject_cast<const QPlaneShape *>(node) != nullptr) {
        // Rotate the plane to make it match the built-in rectangle
        const QQuaternion rotation = kMinus90YawRotation * node->rotation();
        return physx::PxTransform(QPhysicsUtils::toPhysXType(node->position()),
                                  QPhysicsUtils::toPhysXType(rotation));
    } else if (auto *hf = qobject_cast<const QHeightFieldShape *>(node)) {
        // Shift the height field so it's centered at the origin
        return physx::PxTransform(QPhysicsUtils::toPhysXType(node->position() + hf->hfOffset()),
                                  QPhysicsUtils::toPhysXType(node->rotation()));
    }

    const QQuaternion &rotation = node->rotation();
    const QVector3D &worldPosition = node->position();
    return physx::PxTransform(QPhysicsUtils::toPhysXType(worldPosition),
                              QPhysicsUtils::toPhysXType(rotation));
}

static physx::PxFilterFlags
contactReportFilterShader(physx::PxFilterObjectAttributes /*attributes0*/,
                          physx::PxFilterData /*filterData0*/,
                          physx::PxFilterObjectAttributes /*attributes1*/,
                          physx::PxFilterData /*filterData1*/, physx::PxPairFlags &pairFlags,
                          const void * /*constantBlock*/, physx::PxU32 /*constantBlockSize*/)
{
    // Makes objects collide
    const auto defaultCollisonFlags =
            physx::PxPairFlag::eSOLVE_CONTACT | physx::PxPairFlag::eDETECT_DISCRETE_CONTACT;

    // For trigger body detection
    const auto notifyTouchFlags =
            physx::PxPairFlag::eNOTIFY_TOUCH_FOUND | physx::PxPairFlag::eNOTIFY_TOUCH_LOST;

    // For contact detection
    const auto notifyContactFlags = physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;

    pairFlags = defaultCollisonFlags | notifyTouchFlags | notifyContactFlags;
    return physx::PxFilterFlag::eDEFAULT;
}

static physx::PxFilterFlags
contactReportFilterShaderCCD(physx::PxFilterObjectAttributes /*attributes0*/,
                             physx::PxFilterData /*filterData0*/,
                             physx::PxFilterObjectAttributes /*attributes1*/,
                             physx::PxFilterData /*filterData1*/, physx::PxPairFlags &pairFlags,
                             const void * /*constantBlock*/, physx::PxU32 /*constantBlockSize*/)
{
    // Makes objects collide
    const auto defaultCollisonFlags = physx::PxPairFlag::eSOLVE_CONTACT
            | physx::PxPairFlag::eDETECT_DISCRETE_CONTACT | physx::PxPairFlag::eDETECT_CCD_CONTACT;

    // For trigger body detection
    const auto notifyTouchFlags =
            physx::PxPairFlag::eNOTIFY_TOUCH_FOUND | physx::PxPairFlag::eNOTIFY_TOUCH_LOST;

    // For contact detection
    const auto notifyContactFlags = physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;

    pairFlags = defaultCollisonFlags | notifyTouchFlags | notifyContactFlags;
    return physx::PxFilterFlag::eDEFAULT;
}

class CallBackObject : public physx::PxSimulationEventCallback
{
public:
    CallBackObject(QDynamicsWorld *worldIn) : world(worldIn) {};
    virtual ~CallBackObject() = default;

    void onTrigger(physx::PxTriggerPair *pairs, physx::PxU32 count) override
    {
        for (physx::PxU32 i = 0; i < count; i++) {
            // ignore pairs when shapes have been deleted
            if (pairs[i].flags
                & (physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER
                   | physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
                continue;

            QAbstractCollisionNode *otherNode =
                    static_cast<QAbstractCollisionNode *>(pairs[i].otherActor->userData);
            if (!otherNode->enableTriggerReports())
                continue;

            if (pairs->status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND) {
                world->registerOverlap(pairs[i].triggerActor, pairs[i].otherActor);
            } else if (pairs->status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST) {
                world->deregisterOverlap(pairs[i].triggerActor, pairs[i].otherActor);
            }
        }
    }

    void onConstraintBreak(physx::PxConstraintInfo * /*constraints*/,
                           physx::PxU32 /*count*/) override {};
    void onWake(physx::PxActor ** /*actors*/, physx::PxU32 /*count*/) override {};
    void onSleep(physx::PxActor ** /*actors*/, physx::PxU32 /*count*/) override {};
    void onContact(const physx::PxContactPairHeader &pairHeader, const physx::PxContactPair *pairs,
                   physx::PxU32 nbPairs) override
    {
        constexpr physx::PxU32 bufferSize = 64;
        physx::PxContactPairPoint contacts[bufferSize];

        for (physx::PxU32 i = 0; i < nbPairs; i++) {
            const physx::PxContactPair &contactPair = pairs[i];

            if (contactPair.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND) {
                QAbstractCollisionNode *trigger =
                        static_cast<QAbstractCollisionNode *>(pairHeader.actors[0]->userData);
                QAbstractCollisionNode *other =
                        static_cast<QAbstractCollisionNode *>(pairHeader.actors[1]->userData);

                if (!trigger || !other) //### TODO: handle character controllers
                    continue;

                const bool triggerReceive = world->hasReceiveContactReports(trigger)
                        && world->hasSendContactReports(other);
                const bool otherReceive = world->hasReceiveContactReports(other)
                        && world->hasSendContactReports(trigger);

                if (!triggerReceive && !otherReceive)
                    continue;

                physx::PxU32 nbContacts = pairs[i].extractContacts(contacts, bufferSize);

                QList<QVector3D> positions;
                QList<QVector3D> impulses;
                QList<QVector3D> normals;

                positions.reserve(nbContacts);
                impulses.reserve(nbContacts);
                normals.reserve(nbContacts);

                for (physx::PxU32 j = 0; j < nbContacts; j++) {
                    physx::PxVec3 position = contacts[j].position;
                    physx::PxVec3 impulse = contacts[j].impulse;
                    physx::PxVec3 normal = contacts[j].normal;

                    positions.push_back(QPhysicsUtils::toQtType(position));
                    impulses.push_back(QPhysicsUtils::toQtType(impulse));
                    normals.push_back(QPhysicsUtils::toQtType(normal));
                }

                QList<QVector3D> normalsInverted;
                normalsInverted.reserve(normals.size());
                for (const QVector3D &v : normals) {
                    normalsInverted.push_back(QVector3D(-v.x(), -v.y(), -v.z()));
                }

                if (triggerReceive)
                    trigger->registerContact(other, positions, impulses, normals);
                if (otherReceive)
                    other->registerContact(trigger, positions, impulses, normalsInverted);
            }
        }
    };
    void onAdvance(const physx::PxRigidBody *const * /*bodyBuffer*/,
                   const physx::PxTransform * /*poseBuffer*/,
                   const physx::PxU32 /*count*/) override {};

private:
    QDynamicsWorld *world = nullptr;
};

struct PhysXWorld
{
    physx::PxDefaultErrorCallback defaultErrorCallback;
    physx::PxDefaultAllocator defaultAllocatorCallback;
    physx::PxFoundation *foundation = nullptr;
    physx::PxPvd *pvd = nullptr;
    physx::PxPvdTransport *transport = nullptr;
    physx::PxPhysics *physics = nullptr;
    physx::PxScene *scene = nullptr;
    physx::PxDefaultCpuDispatcher *dispatcher = nullptr;

    physx::PxCooking *cooking = nullptr;
    physx::PxControllerManager *controllerManager = nullptr;

    bool recordMemoryAllocations = true;
    CallBackObject *callback = nullptr;
    bool isRunning = false;
};

#define PHYSX_RELEASE(x)                                                                           \
    if (x != nullptr) {                                                                            \
        x->release();                                                                              \
        x = nullptr;                                                                               \
    }

class QAbstractPhysXNode
{
public:
    QAbstractPhysXNode(QAbstractCollisionNode *node) : frontendNode(node)
    {
        node->m_backendObject = this;
    }
    virtual ~QAbstractPhysXNode() { }

    bool cleanupIfRemoved(PhysXWorld *physX); // TODO rename??

    virtual void init(QDynamicsWorld *world, PhysXWorld *physX) = 0;
    virtual void updateDefaultDensity(float /*density*/) { }
    virtual void createMaterial(PhysXWorld *physX);
    void createMaterialFromQtMaterial(PhysXWorld *physX, QPhysicsMaterial *qtMaterial);
    virtual void markDirtyShapes() { }
    virtual void rebuildDirtyShapes(QDynamicsWorld *, PhysXWorld *) { }

    virtual void sync(float deltaTime) = 0;
    virtual void cleanup(PhysXWorld *)
    {
        for (auto *shape : shapes)
            PHYSX_RELEASE(shape);
        if (material != defaultMaterial)
            PHYSX_RELEASE(material);
    }
    virtual bool debugGeometryCapability() { return false; }
    virtual physx::PxTransform getGlobalPose() { return {}; }

    virtual bool useTriggerFlag() { return false; }

    bool shapesDirty() const { return frontendNode && frontendNode->m_shapesDirty; }
    void setShapesDirty(bool dirty) { frontendNode->m_shapesDirty = dirty; }

    QVector<physx::PxShape *> shapes;
    physx::PxMaterial *material = nullptr;
    QAbstractCollisionNode *frontendNode = nullptr;
    bool isRemoved = false;
    static physx::PxMaterial *defaultMaterial;
};

physx::PxMaterial *QAbstractPhysXNode::defaultMaterial = nullptr;

bool QAbstractPhysXNode::cleanupIfRemoved(PhysXWorld *physX)
{
    if (isRemoved) {
        cleanup(physX);
        delete this;
        return true;
    }
    return false;
}

class QPhysXCharacterController : public QAbstractPhysXNode
{
public:
    QPhysXCharacterController(QCharacterController *frontEnd) : QAbstractPhysXNode(frontEnd) { }
    void init(QDynamicsWorld *world, PhysXWorld *physX) override;

    void sync(float deltaTime) override;
    void createMaterial(PhysXWorld *physX) override;

private:
    physx::PxController *controller = nullptr;
};

class QPhysXActorBody : public QAbstractPhysXNode
{
public:
    QPhysXActorBody(QAbstractCollisionNode *frontEnd) : QAbstractPhysXNode(frontEnd) { }
    void cleanup(PhysXWorld *physX) override
    {
        if (actor) {
            physX->scene->removeActor(*actor);
            PHYSX_RELEASE(actor);
        }
        QAbstractPhysXNode::cleanup(physX);
    }
    void init(QDynamicsWorld *world, PhysXWorld *physX) override;
    void sync(float) override;
    void markDirtyShapes() override;
    void rebuildDirtyShapes(QDynamicsWorld *world, PhysXWorld *physX) override;
    virtual void createActor(PhysXWorld *physX);

    bool debugGeometryCapability() override { return true; }
    physx::PxTransform getGlobalPose() override { return actor->getGlobalPose(); }
    void buildShapes(PhysXWorld *physX);

    physx::PxRigidActor *actor = nullptr;
};

class QPhysXRigidBody : public QPhysXActorBody
{
public:
    QPhysXRigidBody(QAbstractPhysicsBody *frontEnd) : QPhysXActorBody(frontEnd) { }
    void createMaterial(PhysXWorld *physX) override;
};

class QPhysXStaticBody : public QPhysXRigidBody
{
public:
    QPhysXStaticBody(QStaticRigidBody *frontEnd) : QPhysXRigidBody(frontEnd) { }

    void sync(float deltaTime) override;
    void createActor(PhysXWorld *physX) override;
};

class QPhysXDynamicBody : public QPhysXRigidBody
{
public:
    QPhysXDynamicBody(QDynamicRigidBody *frontEnd) : QPhysXRigidBody(frontEnd) { }

    void sync(float deltaTime) override;
    void rebuildDirtyShapes(QDynamicsWorld *world, PhysXWorld *physX) override;
    void updateDefaultDensity(float density) override;
};

class QPhysXTriggerBody : public QPhysXActorBody
{
public:
    QPhysXTriggerBody(QTriggerBody *frontEnd) : QPhysXActorBody(frontEnd) { }

    void sync(float deltaTime) override;
    bool useTriggerFlag() override { return true; }
};

class QPhysXFactory
{
public:
    static QAbstractPhysXNode *createBackend(QAbstractCollisionNode *node)
    { // TODO: virtual function in QAbstractCollisionNode??

        if (auto *rigidBody = qobject_cast<QDynamicRigidBody *>(node))
            return new QPhysXDynamicBody(rigidBody);
        if (auto *staticBody = qobject_cast<QStaticRigidBody *>(node))
            return new QPhysXStaticBody(staticBody);
        if (auto *triggerBody = qobject_cast<QTriggerBody *>(node))
            return new QPhysXTriggerBody(triggerBody);
        if (auto *controller = qobject_cast<QCharacterController *>(node))
            return new QPhysXCharacterController(controller);
        Q_UNREACHABLE();
    }
};

/*
   NOTE
   The inheritance hierarchy is not ideal, since both controller and rigid body have materials,
   but trigger doesn't. AND both trigger and rigid body have actors, but controller doesn't.

   TODO: defaultMaterial isn't used for rigid bodies, since they always create their own
   QPhysicsMaterial with default values. We should only have a qt material when set explicitly.
   */

void QAbstractPhysXNode::createMaterialFromQtMaterial(PhysXWorld *physX, QPhysicsMaterial *qtMaterial)
{
    if (qtMaterial) {
        material =  physX->physics->createMaterial(qtMaterial->staticFriction(),
                                                   qtMaterial->dynamicFriction(),
                                                   qtMaterial->restitution());
    } else {
        if (!defaultMaterial) {
            defaultMaterial = physX->physics->createMaterial(QPhysicsMaterial::defaultStaticFriction,
                                                             QPhysicsMaterial::defaultDynamicFriction,
                                                             QPhysicsMaterial::defaultRestitution);
        }
        material = defaultMaterial;
    }
}

void QAbstractPhysXNode::createMaterial(PhysXWorld *physX)
{
    createMaterialFromQtMaterial(physX, nullptr);
}

void QPhysXCharacterController::createMaterial(PhysXWorld *physX)
{
    createMaterialFromQtMaterial(physX, static_cast<QCharacterController *>(frontendNode)->physicsMaterial());
}

void QPhysXRigidBody::createMaterial(PhysXWorld *physX)
{
    createMaterialFromQtMaterial(physX, static_cast<QAbstractPhysicsBody *>(frontendNode)->physicsMaterial());
}

void QPhysXActorBody::createActor(PhysXWorld *physX)
{
    physx::PxTransform trf = getPhysXWorldTransform(frontendNode);
    actor = physX->physics->createRigidDynamic(trf);
}

void QPhysXStaticBody::createActor(PhysXWorld *physX)
{
    physx::PxTransform trf = getPhysXWorldTransform(frontendNode);
    actor = physX->physics->createRigidStatic(trf);
}

void QPhysXActorBody::init(QDynamicsWorld *, PhysXWorld *physX)
{
    Q_ASSERT(!actor);

    createMaterial(physX);
    createActor(physX);

    actor->userData = reinterpret_cast<void *>(frontendNode);
    physX->scene->addActor(*actor);
    setShapesDirty(true);
}

void QPhysXCharacterController::init(QDynamicsWorld *world, PhysXWorld *physX)
{
    Q_ASSERT(!controller);

    auto *characterController = static_cast<QCharacterController *>(frontendNode);

    auto shapes = characterController->getCollisionShapesList();
    if (shapes.empty())
        return;
    auto *capsule = qobject_cast<QCapsuleShape *>(shapes.first());
    if (!capsule)
        return;

    auto *mgr = world->controllerManager();
    if (!mgr)
        return;

    createMaterial(physX);

    const QVector3D s = characterController->sceneScale();
    const qreal hs = s.y();
    const qreal rs = s.x();
    physx::PxCapsuleControllerDesc desc;
    desc.radius = rs * capsule->diameter() / 2;
    desc.height = hs * capsule->height();
    desc.stepOffset = desc.height / 4; // TODO: API

    desc.material = material;
    const QVector3D pos = characterController->scenePosition();
    desc.position = { pos.x(), pos.y(), pos.z() };
    controller = mgr->createController(desc);
}

void QPhysXDynamicBody::updateDefaultDensity(float density)
{
    QDynamicRigidBody *rigidBody = static_cast<QDynamicRigidBody *>(frontendNode);
    rigidBody->updateDefaultDensity(density);
}

void QPhysXActorBody::markDirtyShapes()
{
    if (!frontendNode || !actor)
        return;

    // Go through the shapes and look for a change in pose (rotation, position)
    // TODO: it is likely cheaper to connect a signal for changes on the position and rotation
    // property and mark the node dirty then.
    if (!shapesDirty()) {
        const auto &collisionShapes = frontendNode->getCollisionShapesList();
        const auto &physXShapes = shapes;

        const int len = collisionShapes.size();
        if (physXShapes.size() != len) {
            // This should not really happen but check it anyway
            setShapesDirty(true);
        } else {

            for (int i = 0; i < len; i++) {
                auto poseNew = getPhysXLocalTransform(collisionShapes[i]);
                auto poseOld = physXShapes[i]->getLocalPose();

                if (!fuzzyEquals(poseNew, poseOld)) {
                    setShapesDirty(true);
                    break;
                }
            }
        }
    }
}

void QPhysXActorBody::buildShapes(PhysXWorld *physX)
{
    auto body = actor;
    for (auto *shape : shapes) {
        body->detachShape(*shape);
        PHYSX_RELEASE(shape);
    }

    // TODO: Only remove changed shapes?
    shapes.clear();

    for (const auto &collisionShape : frontendNode->getCollisionShapesList()) {
        // TODO: shapes can be shared between multiple actors.
        // Do we need to create new ones for every body?
        auto *geom = collisionShape->getPhysXGeometry();
        if (!geom || !material)
            continue;
        auto physXShape = physX->physics->createShape(*geom, *material);

        if (useTriggerFlag()) {
            physXShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
            physXShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
        }

        shapes.push_back(physXShape);
        physXShape->setLocalPose(getPhysXLocalTransform(collisionShape));
        body->attachShape(*physXShape);
    }
}

void QPhysXActorBody::rebuildDirtyShapes(QDynamicsWorld *, PhysXWorld *physX)
{
    if (!shapesDirty())
        return;
    buildShapes(physX);
    setShapesDirty(false);
}

void QPhysXDynamicBody::rebuildDirtyShapes(QDynamicsWorld *world, PhysXWorld *physX)
{
    if (!shapesDirty())
        return;

    buildShapes(physX);

    QDynamicRigidBody *drb = static_cast<QDynamicRigidBody *>(frontendNode);

    // Density must be set after shapes so the inertia tensor is set
    if (!drb->hasStaticShapes()) {
        // Body with only dynamic shapes, set/calculate mass
        QPhysicsCommand *command = nullptr;
        switch (drb->massMode()) {
        case QDynamicRigidBody::MassMode::Density: {
            const float density = drb->density() < 0.f ? world->defaultDensity() : drb->density();
            command = new QPhysicsCommandSetDensity(density);
            break;
        }
        case QDynamicRigidBody::MassMode::Mass: {
            const float mass = qMax(drb->mass(), 0.f);
            command = new QPhysicsCommandSetMass(mass);
            break;
        }
        case QDynamicRigidBody::MassMode::MassAndInertiaTensor: {
            const float mass = qMax(drb->mass(), 0.f);
            command = new QPhysicsCommandSetMassAndInertiaTensor(mass, drb->inertiaTensor());
            break;
        }
        case QDynamicRigidBody::MassMode::MassAndInertiaMatrix: {
            const float mass = qMax(drb->mass(), 0.f);
            command = new QPhysicsCommandSetMassAndInertiaMatrix(mass, drb->inertiaMatrix());
            break;
        }
        }

        drb->commandQueue().enqueue(command);
    } else if (!drb->isKinematic()) {
        // Body with static shapes that is not kinematic, this is disallowed
        qWarning() << "Cannot make body containing trimesh/heightfield/plane non-kinematic, "
                      "forcing kinematic.";
        drb->setIsKinematic(true);
    }

    auto *dynamicBody = static_cast<physx::PxRigidDynamic *>(actor);
    dynamicBody->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, drb->isKinematic());

    if (world->enableCCD() && !drb->isKinematic()) // CCD not supported for kinematic bodies
        dynamicBody->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, true);

    setShapesDirty(false);
}

static void processCommandQueue(QQueue<QPhysicsCommand *> &commandQueue,
                                const QDynamicRigidBody &rigidBody, physx::PxRigidBody &body)
{
    for (auto command : commandQueue) {
        command->execute(rigidBody, body);
        delete command;
    }

    commandQueue.clear();
}

static physx::PxRigidDynamicLockFlags getLockFlags(QDynamicRigidBody *body)
{
    const int flags =
            (body->axisLockAngularX() ? physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X : 0)
            | (body->axisLockAngularY() ? physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y : 0)
            | (body->axisLockAngularZ() ? physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z : 0)
            | (body->axisLockLinearX() ? physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X : 0)
            | (body->axisLockLinearY() ? physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y : 0)
            | (body->axisLockLinearZ() ? physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z : 0);
    return static_cast<physx::PxRigidDynamicLockFlags>(flags);
}

static void updatePhysXMaterial(const QPhysicsMaterial *qtMaterial,
                                physx::PxMaterial *physXMaterial)
{
    const float staticFriction = qtMaterial->staticFriction();
    const float dynamicFriction = qtMaterial->dynamicFriction();
    const float restitution = qtMaterial->restitution();
    if (physXMaterial->getStaticFriction() != staticFriction)
        physXMaterial->setStaticFriction(staticFriction);
    if (physXMaterial->getDynamicFriction() != dynamicFriction)
        physXMaterial->setDynamicFriction(dynamicFriction);
    if (physXMaterial->getRestitution() != restitution)
        physXMaterial->setRestitution(restitution);
}

void QPhysXActorBody::sync(float)
{
    auto *body = static_cast<QAbstractPhysicsBody *>(frontendNode);
    if (QPhysicsMaterial *qtMaterial = body->physicsMaterial()) {
        updatePhysXMaterial(qtMaterial, material);
    }
}

void QPhysXTriggerBody::sync(float)
{
    auto *triggerBody = static_cast<QTriggerBody *>(frontendNode);
    actor->setGlobalPose(getPhysXWorldTransform(triggerBody));
}

void QPhysXDynamicBody::sync(float deltaTime)
{
    auto *dynamicRigidBody = static_cast<QDynamicRigidBody *>(frontendNode);
    // first update front end node from physx simulation
    if (!dynamicRigidBody->isKinematic())
        dynamicRigidBody->updateFromPhysicsTransform(actor->getGlobalPose());

    auto *dynamicActor = static_cast<physx::PxRigidDynamic *>(actor);
    processCommandQueue(dynamicRigidBody->commandQueue(), *dynamicRigidBody, *dynamicActor);
    if (dynamicRigidBody->isKinematic())
        dynamicActor->setKinematicTarget(getPhysXWorldTransform(dynamicRigidBody));
    else
        dynamicActor->setRigidDynamicLockFlags(getLockFlags(dynamicRigidBody));

    QPhysXActorBody::sync(deltaTime);
}

void QPhysXCharacterController::sync(float deltaTime)
{
    Q_ASSERT(controller);
    auto pos = controller->getPosition();
    QVector3D qtPosition(pos.x, pos.y, pos.z);
    auto *characterController = static_cast<QCharacterController *>(frontendNode);

    // update node from physX
    const QQuick3DNode *parentNode = static_cast<QQuick3DNode *>(characterController->parentItem());
    if (!parentNode) {
        // then it is the same space
        characterController->setPosition(qtPosition);
    } else {
        characterController->setPosition(parentNode->mapPositionFromScene(qtPosition));
    }
    QVector3D teleportPos;
    bool teleport = characterController->getTeleport(teleportPos);
    if (teleport) {
        controller->setPosition({ teleportPos.x(), teleportPos.y(), teleportPos.z() });
    } else if (deltaTime > 0) {
        const auto movement = characterController->getMovement(deltaTime);
        physx::PxVec3 displacement(movement.x(), movement.y(), movement.z());
        auto collisions =
                controller->move(displacement, displacement.magnitude() / 100, deltaTime, {});
        characterController->setCollisions(QCharacterController::Collisions(uint(collisions)));
    }
    // QCharacterController has a material property, but we don't inherit from
    // QPhysXMaterialBody, so we create the material manually in init()
    // TODO: handle material changes
}

void QPhysXStaticBody::sync(float deltaTime)
{
    auto *staticBody = static_cast<QStaticRigidBody *>(frontendNode);
    const physx::PxTransform poseNew = getPhysXWorldTransform(staticBody);
    const physx::PxTransform poseOld = actor->getGlobalPose();

    // For performance we only update static objects if they have been moved
    if (!fuzzyEquals(poseNew, poseOld))
        actor->setGlobalPose(poseNew);
    QPhysXActorBody::sync(deltaTime);
}

QDynamicsWorld::QDynamicsWorld(QObject *parent) : QObject(parent)
{
    m_physx = new PhysXWorld;
    m_physx->callback = new CallBackObject(this);
    m_physx->foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_physx->defaultAllocatorCallback,
                                             m_physx->defaultErrorCallback);
    if (!m_physx->foundation)
        qFatal("PxCreateFoundation failed!");

#if PHYSX_ENABLE_PVD
    m_physx->pvd = PxCreatePvd(*m_physx->foundation);
    m_physx->transport = physx::PxDefaultPvdSocketTransportCreate("qt", 5425, 10);
    m_physx->pvd->connect(*m_physx->transport, physx::PxPvdInstrumentationFlag::eALL);
#endif

    self = this; // TODO: make a better internal access mechanism
    maintainTimer();
}

QDynamicsWorld::~QDynamicsWorld()
{
    if (m_physx->isRunning)
        m_physx->scene->fetchResults(true);

    PHYSX_RELEASE(m_physx->dispatcher);
    PHYSX_RELEASE(m_physx->controllerManager);
    PHYSX_RELEASE(m_physx->cooking);
    PHYSX_RELEASE(m_physx->physics);
    PHYSX_RELEASE(m_physx->transport);
    PHYSX_RELEASE(m_physx->pvd);
    PHYSX_RELEASE(m_physx->foundation);

    delete m_physx->callback;
    delete m_physx;
    self = nullptr;

    for (auto body : m_physXBodies) {
        delete body;
    }
}

QVector3D QDynamicsWorld::gravity() const
{
    return m_gravity;
}

bool QDynamicsWorld::running() const
{
    return m_running;
}

bool QDynamicsWorld::forceDebugView() const
{
    return m_forceDebugView;
}

bool QDynamicsWorld::enableCCD() const
{
    return m_enableCCD;
}

float QDynamicsWorld::typicalLength() const
{
    return m_typicalLength;
}

float QDynamicsWorld::typicalSpeed() const
{
    return m_typicalSpeed;
}

void QDynamicsWorld::registerOverlap(physx::PxRigidActor *triggerActor,
                                     physx::PxRigidActor *otherActor)
{
    QTriggerBody *trigger = static_cast<QTriggerBody *>(triggerActor->userData);
    QAbstractCollisionNode *other = static_cast<QAbstractCollisionNode *>(otherActor->userData);

    if (!m_removedCollisionNodes.contains(other) && !m_removedCollisionNodes.contains(trigger))
        trigger->registerCollision(other);
}

void QDynamicsWorld::deregisterOverlap(physx::PxRigidActor *triggerActor,
                                       physx::PxRigidActor *otherActor)
{
    QTriggerBody *trigger = static_cast<QTriggerBody *>(triggerActor->userData);
    QAbstractCollisionNode *other = static_cast<QAbstractCollisionNode *>(otherActor->userData);
    if (!m_removedCollisionNodes.contains(other) && !m_removedCollisionNodes.contains(trigger))
        trigger->deregisterCollision(other);
}

bool QDynamicsWorld::hasSendContactReports(QAbstractCollisionNode *object) const
{
    return !m_removedCollisionNodes.contains(object) && object->m_backendObject
            && object->sendContactReports();
}

bool QDynamicsWorld::hasReceiveContactReports(QAbstractCollisionNode *object) const
{
    return !m_removedCollisionNodes.contains(object) && object->m_backendObject
            && object->receiveContactReports();
}

void QDynamicsWorld::registerNode(QAbstractCollisionNode *collisionNode)
{
    m_newCollisionNodes.push_back(collisionNode);
}

void QDynamicsWorld::deregisterNode(QAbstractCollisionNode *collisionNode)
{
    m_newCollisionNodes.removeAll(collisionNode);
    if (collisionNode->m_backendObject)
        collisionNode->m_backendObject->isRemoved = true;

    for (auto shape : collisionNode->getCollisionShapesList()) {
        m_collisionShapeDebugModels.remove(shape);
    }

    m_removedCollisionNodes.insert(collisionNode);
}

void QDynamicsWorld::setGravity(QVector3D gravity)
{
    if (m_gravity == gravity)
        return;

    m_gravity = gravity;
    if (m_physx->scene) {
        m_physx->scene->setGravity(QPhysicsUtils::toPhysXType(m_gravity));
    }
    emit gravityChanged(m_gravity);
}

void QDynamicsWorld::setRunning(bool running)
{
    if (m_running == running)
        return;

    m_running = running;
    maintainTimer();
    emit runningChanged(m_running);
}

void QDynamicsWorld::setForceDebugView(bool forceDebugView)
{
    if (m_forceDebugView == forceDebugView)
        return;

    m_forceDebugView = forceDebugView;
    if (!m_forceDebugView)
        disableDebugDraw();
    else
        updateDebugDraw();
    emit forceDebugViewChanged(m_forceDebugView);
}

QQuick3DViewport *QDynamicsWorld::sceneView() const
{
    return m_sceneView;
}

void QDynamicsWorld::setHasIndividualDebugView()
{
    m_hasIndividualDebugView = true;
}

void QDynamicsWorld::setSceneView(QQuick3DViewport *sceneView)
{
    if (m_sceneView == sceneView)
        return;

    m_sceneView = sceneView;

    // TODO: test this
    delete m_debugMaterial;
    m_debugMaterial = nullptr;
    for (auto &holder : m_collisionShapeDebugModels) {
        delete holder.model;
    }
    m_collisionShapeDebugModels.clear();

    emit sceneViewChanged(m_sceneView);
}

void QDynamicsWorld::setMinTimestep(float minTimestep)
{
    if (qFuzzyCompare(m_minTimestep, minTimestep))
        return;

    m_minTimestep = minTimestep;

    // Change timer refresh rate if running
    if (m_running && m_updateTimer.isActive()) {
        m_updateTimer.stop();
        const int freq = qMax(1, int(m_minTimestep));
        m_updateTimer.start(freq, this);
    }

    emit minTimestepChanged(minTimestep);
}

void QDynamicsWorld::setMaxTimestep(float maxTimestep)
{
    if (qFuzzyCompare(m_maxTimestep, maxTimestep))
        return;

    m_maxTimestep = maxTimestep;
    emit maxTimestepChanged(maxTimestep);
}

void QDynamicsWorld::updateDebugDraw()
{
    if (m_sceneView == nullptr || !(m_forceDebugView || m_hasIndividualDebugView))
        return;

    auto sceneRoot = m_sceneView->scene();

    if (sceneRoot == nullptr)
        return;

    if (!m_debugMaterial) {
        m_debugMaterial = new QQuick3DDefaultMaterial();
        m_debugMaterial->setParentItem(sceneRoot);
        m_debugMaterial->setParent(sceneRoot);
        m_debugMaterial->setDiffuseColor(QColor(3, 252, 219));
        m_debugMaterial->setLighting(QQuick3DDefaultMaterial::NoLighting);
        m_debugMaterial->setCullMode(QQuick3DMaterial::NoCulling);
    }

    m_hasIndividualDebugView = false;

    for (QAbstractPhysXNode *node : m_physXBodies) {
        if (!node->debugGeometryCapability())
            continue;

        const auto &collisionShapes = node->frontendNode->getCollisionShapesList();
        const int length = collisionShapes.length();
        if (node->shapes.length() < length)
            continue; // CharacterController has shapes, but not PhysX shapes
        for (int idx = 0; idx < length; idx++) {
            const auto collisionShape = collisionShapes[idx];
            const auto physXShape = node->shapes[idx];

            DebugModelHolder &holder = m_collisionShapeDebugModels[collisionShape];
            auto &model = holder.model;

            if (!m_forceDebugView && !collisionShape->enableDebugView()) {
                if (model) {
                    model->setVisible(false);
                }
                continue;
            }

            m_hasIndividualDebugView =
                    m_hasIndividualDebugView || collisionShape->enableDebugView();

            auto localPose = physXShape->getLocalPose();

            // Create/Update debug view infrastructure
            if (!model) {
                model = new QQuick3DModel();
                model->setParentItem(sceneRoot);
                model->setParent(sceneRoot);
                model->setCastsShadows(false);
                model->setReceivesShadows(false);
                model->setCastsReflections(false);
                QQmlListReference materialsRef(model, "materials");
                materialsRef.append(m_debugMaterial);
            }

            switch (physXShape->getGeometryType()) {
            case physx::PxGeometryType::eBOX: {
                physx::PxBoxGeometry boxGeometry;
                physXShape->getBoxGeometry(boxGeometry);
                const auto &halfExtentsOld = holder.halfExtents();
                const auto halfExtents = QPhysicsUtils::toQtType(boxGeometry.halfExtents);
                if (!qFuzzyCompare(halfExtentsOld, halfExtents)) {
                    auto geom = QDebugDrawHelper::generateBoxGeometry(halfExtents);
                    model->setGeometry(geom);
                    holder.setHalfExtents(halfExtents);
                }

            }
                break;

            case physx::PxGeometryType::eSPHERE: {
                physx::PxSphereGeometry sphereGeometry;
                physXShape->getSphereGeometry(sphereGeometry);
                const float radius = holder.radius();
                if (!qFuzzyCompare(sphereGeometry.radius, radius)) {
                    auto geom = QDebugDrawHelper::generateSphereGeometry(sphereGeometry.radius);
                    model->setGeometry(geom);
                    holder.setRadius(sphereGeometry.radius);
                }
            }
                break;

            case physx::PxGeometryType::eCAPSULE: {
                physx::PxCapsuleGeometry capsuleGeometry;
                physXShape->getCapsuleGeometry(capsuleGeometry);
                const float radius = holder.radius();
                const float halfHeight = holder.halfHeight();

                if (!qFuzzyCompare(capsuleGeometry.radius, radius)
                    || !qFuzzyCompare(capsuleGeometry.halfHeight, halfHeight)) {
                    auto geom = QDebugDrawHelper::generateCapsuleGeometry(
                            capsuleGeometry.radius, capsuleGeometry.halfHeight);
                    model->setGeometry(geom);
                    holder.setRadius(capsuleGeometry.radius);
                    holder.setHalfHeight(capsuleGeometry.halfHeight);
                }
            }
                break;

            case physx::PxGeometryType::ePLANE:{
                physx::PxPlaneGeometry planeGeometry;
                physXShape->getPlaneGeometry(planeGeometry);
                // Special rotation
                const QQuaternion rotation =
                        kMinus90YawRotation * QPhysicsUtils::toQtType(localPose.q);
                localPose = physx::PxTransform(localPose.p, QPhysicsUtils::toPhysXType(rotation));

                if (model->geometry() == nullptr) {
                    auto geom = QDebugDrawHelper::generatePlaneGeometry();
                    model->setGeometry(geom);
                }
            }
                break;

            case physx::PxGeometryType::eHEIGHTFIELD: {
                physx::PxHeightFieldGeometry heightFieldGeometry;
                physXShape->getHeightFieldGeometry(heightFieldGeometry);
                const float heightScale = holder.heightScale();
                const float rowScale = holder.rowScale();
                const float columnScale = holder.columnScale();

                if (!qFuzzyCompare(heightFieldGeometry.heightScale, heightScale)
                    || !qFuzzyCompare(heightFieldGeometry.rowScale, rowScale)
                    || !qFuzzyCompare(heightFieldGeometry.columnScale, columnScale)) {

                    auto geom = QDebugDrawHelper::generateHeightFieldGeometry(
                            heightFieldGeometry.heightField, heightFieldGeometry.heightScale,
                            heightFieldGeometry.rowScale, heightFieldGeometry.columnScale);
                    model->setGeometry(geom);
                    holder.setHeightScale(heightFieldGeometry.heightScale);
                    holder.setRowScale(heightFieldGeometry.rowScale);
                    holder.setColumnScale(heightFieldGeometry.columnScale);
                }
            }
                break;

            case physx::PxGeometryType::eCONVEXMESH: {
                physx::PxConvexMeshGeometry convexMeshGeometry;
                physXShape->getConvexMeshGeometry(convexMeshGeometry);
                const auto rotation = convexMeshGeometry.scale.rotation * localPose.q;
                localPose = physx::PxTransform(localPose.p, rotation);
                model->setScale(QPhysicsUtils::toQtType(convexMeshGeometry.scale.scale));

                if (model->geometry() == nullptr) {
                    auto geom = QDebugDrawHelper::generateConvexMeshGeometry(
                            convexMeshGeometry.convexMesh);
                    model->setGeometry(geom);
                }
            }
                break;

            case physx::PxGeometryType::eTRIANGLEMESH: {
                physx::PxTriangleMeshGeometry triangleMeshGeometry;
                physXShape->getTriangleMeshGeometry(triangleMeshGeometry);
                const auto rotation = triangleMeshGeometry.scale.rotation * localPose.q;
                localPose = physx::PxTransform(localPose.p, rotation);
                model->setScale(QPhysicsUtils::toQtType(triangleMeshGeometry.scale.scale));

                if (model->geometry() == nullptr) {
                    auto geom = QDebugDrawHelper::generateTriangleMeshGeometry(
                            triangleMeshGeometry.triangleMesh);
                    model->setGeometry(geom);
                }
            }
                break;

            case physx::PxGeometryType::eINVALID:
            case physx::PxGeometryType::eGEOMETRY_COUNT:
                // should not happen
                Q_UNREACHABLE();
            }

            model->setParent(collisionShape);
            model->setVisible(true);

            auto globalPose = node->getGlobalPose();
            auto finalPose = globalPose.transform(localPose);

            model->setRotation(QPhysicsUtils::toQtType(finalPose.q));
            model->setPosition(QPhysicsUtils::toQtType(finalPose.p));
        }
    }
}

void QDynamicsWorld::disableDebugDraw()
{
    if (m_sceneView == nullptr || m_sceneView->scene() == nullptr)
        return;

    m_hasIndividualDebugView = false;

    for (QAbstractPhysXNode *body : m_physXBodies) {
        // TODO: refactor debug geometry handling as well
        const auto &collisionShapes = body->frontendNode->getCollisionShapesList();
        const int length = collisionShapes.length();
        for (int idx = 0; idx < length; idx++) {
            const auto collisionShape = collisionShapes[idx];
            DebugModelHolder &holder = m_collisionShapeDebugModels[collisionShape];

            if (!collisionShape->enableDebugView()) {
                if (holder.model) {
                    holder.model->setVisible(false);
                }
            } else {
                m_hasIndividualDebugView = true;
            }
        }
    }
}

void QDynamicsWorld::setEnableCCD(bool enableCCD)
{
    if (m_enableCCD == enableCCD)
        return;

    if (m_physicsInitialized) {
        qWarning()
                << "Warning: Changing 'enableCCD' after physics is initialized will have no effect";
        return;
    }

    m_enableCCD = enableCCD;
    emit enableCCDChanged(m_enableCCD);
}

void QDynamicsWorld::setTypicalLength(float typicalLength)
{
    if (qFuzzyCompare(typicalLength, m_typicalLength))
        return;

    if (typicalLength <= 0.f) {
        qWarning() << "Warning: 'typicalLength' value less than zero, ignored";
        return;
    }

    if (m_physicsInitialized) {
        qWarning() << "Warning: Changing 'typicalLength' after physics is initialized will have "
                      "no effect";
        return;
    }

    m_typicalLength = typicalLength;

    emit typicalLengthChanged(typicalLength);
}

void QDynamicsWorld::setTypicalSpeed(float typicalSpeed)
{
    if (qFuzzyCompare(typicalSpeed, m_typicalSpeed))
        return;

    if (m_physicsInitialized) {
        qWarning() << "Warning: Changing 'typicalSpeed' after physics is initialized will have "
                      "no effect";
        return;
    }

    m_typicalSpeed = typicalSpeed;

    emit typicalSpeedChanged(typicalSpeed);
}

float QDynamicsWorld::defaultDensity() const
{
    return m_defaultDensity;
}

float QDynamicsWorld::minTimestep() const
{
    return m_minTimestep;
}

float QDynamicsWorld::maxTimestep() const
{
    return m_maxTimestep;
}

void QDynamicsWorld::setDefaultDensity(float defaultDensity)
{
    // Make sure the default density is not too small
    defaultDensity = qMax(0.0000001, defaultDensity);
    if (qFuzzyCompare(m_defaultDensity, defaultDensity))
        return;
    m_defaultDensity = defaultDensity;

    // Go through all dynamic rigid bodies and update the default density
    for (QAbstractPhysXNode *body : m_physXBodies)
        body->updateDefaultDensity(m_defaultDensity);

    emit defaultDensityChanged(defaultDensity);
}

void QDynamicsWorld::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_updateTimer.timerId())
        updatePhysics();
}

void QDynamicsWorld::findSceneView()
{
    // If we have not specified a scene view we find the first available one
    if (m_sceneView != nullptr)
        return;

    QObject *parent = this;
    while (parent->parent() != nullptr) {
        parent = parent->parent();
    }

    // Breath-first search through children
    QList<QObject *> children = parent->children();
    while (!children.empty()) {
        auto child = children.takeFirst();
        if (auto converted = qobject_cast<QQuick3DViewport *>(child); converted != nullptr) {
            m_sceneView = converted;
            break;
        }
        children.append(child->children());
    }
}

// Remove physics world items that no longer exist

void QDynamicsWorld::cleanupRemovedNodes()
{
    m_physXBodies.removeIf([this](QAbstractPhysXNode *body) {
                               return body->cleanupIfRemoved(m_physx);
                           });
    m_removedCollisionNodes.clear();
}

void QDynamicsWorld::initPhysics()
{
    Q_ASSERT(!m_physicsInitialized);

    physx::PxTolerancesScale scale;
    scale.length = m_typicalLength;
    scale.speed = m_typicalSpeed;

    m_physx->physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_physx->foundation, scale,
                                       m_physx->recordMemoryAllocations, m_physx->pvd);

    if (!m_physx->physics)
        qFatal("PxCreatePhysics failed!");

    physx::PxSceneDesc sceneDesc(scale);
    sceneDesc.gravity = QPhysicsUtils::toPhysXType(m_gravity);
    m_physx->dispatcher = physx::PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = m_physx->dispatcher;

    if (m_enableCCD) {
        sceneDesc.filterShader = contactReportFilterShaderCCD;
        sceneDesc.flags |= physx::PxSceneFlag::eENABLE_CCD;
    } else {
        sceneDesc.filterShader = contactReportFilterShader;
    }
    sceneDesc.solverType = physx::PxSolverType::eTGS;
    sceneDesc.simulationEventCallback = m_physx->callback;

    m_physx->scene = m_physx->physics->createScene(sceneDesc);
    m_physx->scene->setGravity(QPhysicsUtils::toPhysXType(m_gravity));

    findSceneView();

    m_physicsInitialized = true;
}

void QDynamicsWorld::updatePhysics()
{
    if (!m_physicsInitialized)
        initPhysics();

    // If not enough time has elapsed we return
    if (m_deltaTime.elapsed() < m_minTimestep)
        return;

    // Check if simulation is done
    if (m_physx->isRunning && !m_physx->scene->fetchResults())
        return;

    cleanupRemovedNodes();
    for (auto *node : qAsConst(m_newCollisionNodes)) {
        auto *body = QPhysXFactory::createBackend(node);
        body->init(this, m_physx);
        m_physXBodies.push_back(body);
    }
    m_newCollisionNodes.clear();

    // Calculate time step
    const auto deltaMS = m_deltaTime.restart();
    const auto deltaTime = qMin(float(deltaMS), m_maxTimestep) * 0.001f;

    // TODO: Use dirty flag/dirty list to avoid redoing things that didn't change
    for (auto *physXBody : qAsConst(m_physXBodies)) {
        physXBody->markDirtyShapes();
        physXBody->rebuildDirtyShapes(this, m_physx);

        // Sync the physics world and the scene
        physXBody->sync(deltaTime);
    }

    updateDebugDraw();

    // Start simulating next frame

    m_physx->scene->simulate(deltaTime);
    m_physx->isRunning = true;
}

void QDynamicsWorld::maintainTimer()
{
    if (m_running == m_updateTimer.isActive())
        return;

    if (m_running) {
        const int freq = qMax(1, int(m_minTimestep));
        m_updateTimer.start(freq, this);
        m_deltaTime.start();
    } else {
        m_updateTimer.stop();
    }
}

QDynamicsWorld *QDynamicsWorld::self = nullptr;

physx::PxPhysics *QDynamicsWorld::getPhysics()
{
    return self ? self->m_physx->physics : nullptr;
}

physx::PxCooking *QDynamicsWorld::getCooking()
{
    return self ? self->cooking() : nullptr;
}

physx::PxCooking *QDynamicsWorld::cooking()
{
    if (!m_physx->cooking) {
        m_physx->cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_physx->foundation,
                                           physx::PxCookingParams(physx::PxTolerancesScale()));
        qCDebug(lcQuick3dPhysics) << "Initialized cooking" << m_physx->cooking;
    }
    return m_physx->cooking;
}

physx::PxControllerManager *QDynamicsWorld::controllerManager()
{
    if (m_physx->scene && !m_physx->controllerManager) {
        m_physx->controllerManager = PxCreateControllerManager(*m_physx->scene);
        qCDebug(lcQuick3dPhysics) << "Created controller manager" << m_physx->controllerManager;
    }
    return m_physx->controllerManager;
}
QT_END_NAMESPACE
