// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef RIGIDBODY_H
#define RIGIDBODY_H

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

#include <QtQuick3DPhysics/private/qabstractphysicsbody_p.h>
#include <QtQml/QQmlEngine>

#include <QtCore/QQueue>

QT_BEGIN_NAMESPACE

class QPhysicsCommand;

class Q_QUICK3DPHYSICS_EXPORT QDynamicRigidBody : public QAbstractPhysicsBody
{
public:
    enum class MassMode {
        Density,
        Mass,
        MassAndInertiaTensor,
        MassAndInertiaMatrix,
    };
    Q_ENUM(MassMode)

    Q_OBJECT
    Q_PROPERTY(float mass READ mass WRITE setMass NOTIFY massChanged)
    Q_PROPERTY(float density READ density WRITE setDensity NOTIFY densityChanged)

    Q_PROPERTY(QVector3D linearVelocity READ linearVelocity WRITE setLinearVelocity NOTIFY
                       linearVelocityChanged)
    Q_PROPERTY(QVector3D angularVelocity READ angularVelocity WRITE setAngularVelocity NOTIFY
                       angularVelocityChanged)

    Q_PROPERTY(bool axisLockLinearX READ axisLockLinearX WRITE setAxisLockLinearX NOTIFY
                       axisLockLinearXChanged)
    Q_PROPERTY(bool axisLockLinearY READ axisLockLinearY WRITE setAxisLockLinearY NOTIFY
                       axisLockLinearYChanged)
    Q_PROPERTY(bool axisLockLinearZ READ axisLockLinearZ WRITE setAxisLockLinearZ NOTIFY
                       axisLockLinearZChanged)
    Q_PROPERTY(bool axisLockAngularX READ axisLockAngularX WRITE setAxisLockAngularX NOTIFY
                       axisLockAngularXChanged)
    Q_PROPERTY(bool axisLockAngularY READ axisLockAngularY WRITE setAxisLockAngularY NOTIFY
                       axisLockAngularYChanged)
    Q_PROPERTY(bool axisLockAngularZ READ axisLockAngularZ WRITE setAxisLockAngularZ NOTIFY
                       axisLockAngularZChanged)

    Q_PROPERTY(bool isKinematic READ isKinematic WRITE setIsKinematic NOTIFY isKinematicChanged)
    Q_PROPERTY(bool gravityEnabled READ gravityEnabled WRITE setGravityEnabled NOTIFY
                       gravityEnabledChanged)

    Q_PROPERTY(MassMode massMode READ massMode WRITE setMassMode NOTIFY massModeChanged)
    Q_PROPERTY(QVector3D inertiaTensor READ inertiaTensor WRITE setInertiaTensor NOTIFY
                       inertiaTensorChanged)
    Q_PROPERTY(QVector3D centerOfMassPosition READ centerOfMassPosition WRITE
                       setCenterOfMassPosition NOTIFY centerOfMassPositionChanged)
    Q_PROPERTY(QQuaternion centerOfMassRotation READ centerOfMassRotation WRITE
                       setCenterOfMassRotation NOTIFY centerOfMassRotationChanged)
    Q_PROPERTY(QList<float> inertiaMatrix READ readInertiaMatrix WRITE setInertiaMatrix NOTIFY
                       inertiaMatrixChanged);

    // clang-format off
//    // ??? separate simulation control object? --- some of these have default values in the engine, so we need tristate
//    Q_PROPERTY(float sleepThreshold READ sleepThreshold WRITE setSleepThreshold NOTIFY sleepThresholdChanged)
//    Q_PROPERTY(float stabilizationThreshold READ stabilizationThreshold WRITE setStabilizationThreshold NOTIFY stabilizationThresholdChanged)
//    Q_PROPERTY(float contactReportThreshold READ contactReportThreshold WRITE setContactReportThreshold NOTIFY contactReportThresholdChanged)
//    Q_PROPERTY(float maxContactImpulse READ maxContactImpulse WRITE setMaxContactImpulse NOTIFY maxContactImpulseChanged)
//    Q_PROPERTY(float maxDepenetrationVelocity READ maxDepenetrationVelocity WRITE setMaxDepenetrationVelocity NOTIFY maxDepenetrationVelocityChanged)
//    Q_PROPERTY(float maxAngularVelocity READ maxAngularVelocity WRITE setMaxAngularVelocity NOTIFY maxAngularVelocityChanged)
//    Q_PROPERTY(int minPositionIterationCount READ minPositionIterationCount WRITE setMinPositionIterationCount NOTIFY minPositionIterationCountChanged)
//    Q_PROPERTY(int minVelocityIterationCount READ minVelocityIterationCount WRITE setMinVelocityIterationCount NOTIFY minVelocityIterationCountChanged)
    // clang-format on
    QML_NAMED_ELEMENT(DynamicRigidBody)

public:
    QDynamicRigidBody();
    ~QDynamicRigidBody();

    float mass() const;
    void setMass(float mass);

    float density() const;
    void setDensity(float density);

    QVector3D linearVelocity() const;
    void setLinearVelocity(QVector3D linearVelocity);

    bool isKinematic() const;
    void setIsKinematic(bool isKinematic);

    const QVector3D &angularVelocity() const;
    void setAngularVelocity(const QVector3D &newAngularVelocity);

    bool axisLockLinearX() const;
    void setAxisLockLinearX(bool newAxisLockLinearX);

    bool axisLockLinearY() const;
    void setAxisLockLinearY(bool newAxisLockLinearY);

    bool axisLockLinearZ() const;
    void setAxisLockLinearZ(bool newAxisLockLinearZ);

    bool axisLockAngularX() const;
    void setAxisLockAngularX(bool newAxisLockAngularX);

    bool axisLockAngularY() const;
    void setAxisLockAngularY(bool newAxisLockAngularY);

    bool axisLockAngularZ() const;
    void setAxisLockAngularZ(bool newAxisLockAngularZ);

    bool gravityEnabled() const;
    void setGravityEnabled(bool gravityEnabled);

    Q_INVOKABLE void applyCentralForce(const QVector3D &force);
    Q_INVOKABLE void applyForce(const QVector3D &force, const QVector3D &position);
    Q_INVOKABLE void applyTorque(const QVector3D &torque);
    Q_INVOKABLE void applyCentralImpulse(const QVector3D &impulse);
    Q_INVOKABLE void applyImpulse(const QVector3D &impulse, const QVector3D &position);
    Q_INVOKABLE void applyTorqueImpulse(const QVector3D &impulse);
    Q_INVOKABLE void reset(const QVector3D &position, const QVector3D &eulerRotation);

    // Internal
    QQueue<QPhysicsCommand *> &commandQueue();

    void updateDefaultDensity(float defaultDensity);

    MassMode massMode() const;
    void setMassMode(const MassMode newMassMode);

    const QVector3D &inertiaTensor() const;
    void setInertiaTensor(const QVector3D &newInertiaTensor);

    const QVector3D &centerOfMassPosition() const;
    void setCenterOfMassPosition(const QVector3D &newCenterOfMassPosition);

    const QQuaternion &centerOfMassRotation() const;
    void setCenterOfMassRotation(const QQuaternion &newCenterOfMassRotation);

    const QList<float> &readInertiaMatrix() const;
    void setInertiaMatrix(const QList<float> &newInertiaMatrix);
    const QMatrix3x3 &inertiaMatrix() const;

Q_SIGNALS:
    void massChanged(float mass);
    void densityChanged(float density);
    void linearVelocityChanged(QVector3D linearVelocity);
    void isKinematicChanged(bool isKinematic);
    void angularVelocityChanged();
    void axisLockLinearXChanged();
    void axisLockLinearYChanged();
    void axisLockLinearZChanged();
    void axisLockAngularXChanged();
    void axisLockAngularYChanged();
    void axisLockAngularZChanged();
    void gravityEnabledChanged();
    void massModeChanged();
    void inertiaTensorChanged();
    void centerOfMassPositionChanged();
    void centerOfMassRotationChanged();
    void inertiaMatrixChanged();

private:
    float m_mass = 1.f;
    float m_density = -1.f;
    QVector3D m_centerOfMassPosition;
    QQuaternion m_centerOfMassRotation;
    QList<float> m_inertiaMatrixList;
    QMatrix3x3 m_inertiaMatrix;
    QVector3D m_inertiaTensor;

    QVector3D m_linearVelocity;
    bool m_isKinematic = false;
    QVector3D m_angularVelocity;
    bool m_axisLockLinearX = false;
    bool m_axisLockLinearY = false;
    bool m_axisLockLinearZ = false;
    bool m_axisLockAngularX = false;
    bool m_axisLockAngularY = false;
    bool m_axisLockAngularZ = false;
    QQueue<QPhysicsCommand *> m_commandQueue;
    bool m_gravityEnabled = true;
    MassMode m_massMode = MassMode::Density;
};

class Q_QUICK3DPHYSICS_EXPORT QStaticRigidBody : public QAbstractPhysicsBody
{
    Q_OBJECT
    QML_NAMED_ELEMENT(StaticRigidBody)
public:
    QStaticRigidBody();
};

QT_END_NAMESPACE

#endif // RIGIDBODY_H
