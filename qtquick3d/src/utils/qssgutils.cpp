// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qssgutils_p.h"

#include <QtCore/QDir>

#include <cmath>

QT_BEGIN_NAMESPACE

float vec2::magnitude(const QVector2D &v)
{
    return ::sqrtf(v.x() * v.x() + v.y() * v.y());
}

bool vec3::isFinite(const QVector3D &v)
{
    return qIsFinite(v.x()) && qIsFinite(v.y()) && qIsFinite(v.z());
}

float vec3::magnitude(const QVector3D &v)
{
    return sqrtf(v.x() * v.x() + v.y() * v.y() + v.z() * v.z());
}

float vec3::magnitudeSquared(const QVector3D &v)
{
    return v.x() * v.x() + v.y() * v.y() + v.z() * v.z();
}

// This special normalize function normalizes a vector in place
// and returns the magnnitude (needed for compatiblity)
float vec3::normalize(QVector3D &v)
{
    const float m = vec3::magnitude(v);
    if (m > 0)
        v /= m;
    return m;
}

QVector3D mat33::transform(const QMatrix3x3 &m, const QVector3D &v)
{
    const QVector3D c0 = QVector3D(m(0, 0), m(1, 0), m(2, 0));
    const QVector3D c1 = QVector3D(m(0, 1), m(1, 1), m(2, 1));
    const QVector3D c2 = QVector3D(m(0, 2), m(1, 2), m(2, 2));
    return c0 * v.x() + c1 * v.y() + c2 * v.z();
}

QMatrix3x3 mat44::getUpper3x3(const QMatrix4x4 &m)
{
    const float values[9] = { m(0, 0), m(0, 1), m(0, 2), m(1, 0), m(1, 1), m(1, 2), m(2, 0), m(2, 1), m(2, 2) };
    return QMatrix3x3(values);
}

void mat44::normalize(QMatrix4x4 &m)
{
    QVector4D c0 = m.column(0);
    QVector4D c1 = m.column(1);
    QVector4D c2 = m.column(2);
    QVector4D c3 = m.column(3);

    c0.normalize();
    c1.normalize();
    c2.normalize();
    c3.normalize();

    m.setColumn(0, c0);
    m.setColumn(1, c1);
    m.setColumn(2, c2);
    m.setColumn(3, c3);
}

QVector3D mat44::rotate(const QMatrix4x4 &m, const QVector3D &v)
{
    const QVector4D tmp = mat44::rotate(m, QVector4D(v.x(), v.y(), v.z(), 1.0f));
    return QVector3D(tmp.x(), tmp.y(), tmp.z());
}

QVector4D mat44::rotate(const QMatrix4x4 &m, const QVector4D &v)
{  
    return m.column(0) * v.x() + m.column(1) * v.y() + m.column(2) * v.z();
}

QVector3D mat44::transform(const QMatrix4x4 &m, const QVector3D &v)
{
    const QVector4D tmp = mat44::transform(m, QVector4D(v.x(), v.y(), v.z(), 1.0f));
    return QVector3D(tmp.x(), tmp.y(), tmp.z());
}

QVector4D mat44::transform(const QMatrix4x4 &m, const QVector4D &v)
{
    return m.column(0) * v.x() + m.column(1) * v.y() + m.column(2) * v.z() + m.column(3) * v.w();
}

QVector3D mat44::getPosition(const QMatrix4x4 &m)
{
    return QVector3D(m(0, 3), m(1, 3), m(2, 3));
}

QVector3D mat44::getScale(const QMatrix4x4 &m)
{
    const float scaleX = m.column(0).length();
    const float scaleY = m.column(1).length();
    const float scaleZ = m.column(2).length();
    return QVector3D(scaleX, scaleY, scaleZ);
}

bool quant::isFinite(const QQuaternion &q)
{
    return qIsFinite(q.x()) && qIsFinite(q.y()) && qIsFinite(q.z()) && qIsFinite(q.scalar());
}

float quant::magnitude(const QQuaternion &q)
{
    return std::sqrt(q.x() * q.x() + q.y() * q.y() + q.z() * q.z() + q.scalar() * q.scalar());
}

bool quant::isSane(const QQuaternion &q)
{
    const float unitTolerance = float(1e-2);
    return isFinite(q) && qAbs(magnitude(q) - 1) < unitTolerance;
}

bool quant::isUnit(const QQuaternion &q)
{
    const float unitTolerance = float(1e-4);
    return isFinite(q) && qAbs(magnitude(q) - 1) < unitTolerance;
}

QVector3D quant::rotated(const QQuaternion &q, const QVector3D &v)
{
    const float vx = 2.0f * v.x();
    const float vy = 2.0f * v.y();
    const float vz = 2.0f * v.z();
    const float w2 = q.scalar() * q.scalar() - 0.5f;
    const float dot2 = (q.x() * vx + q.y() * vy + q.z() * vz);
    return QVector3D((vx * w2 + (q.y() * vz - q.z() * vy) * q.scalar() + q.x() * dot2),
                     (vy * w2 + (q.z() * vx - q.x() * vz) * q.scalar() + q.y() * dot2),
                     (vz * w2 + (q.x() * vy - q.y() * vx) * q.scalar() + q.z() * dot2));
}

QVector3D quant::inverseRotated(const QQuaternion &q, const QVector3D &v)
{
    const float vx = 2.0f * v.x();
    const float vy = 2.0f * v.y();
    const float vz = 2.0f * v.z();
    const float w2 = q.scalar() * q.scalar() - 0.5f;
    const float dot2 = (q.x() * vx + q.y() * vy + q.z() * vz);
    return QVector3D((vx * w2 - (q.y() * vz - q.z() * vy) * q.scalar() + q.x() * dot2),
                     (vy * w2 - (q.z() * vx - q.x() * vz) * q.scalar() + q.y() * dot2),
                     (vz * w2 - (q.x() * vy - q.y() * vx) * q.scalar() + q.z() * dot2));
}

const char *nonNull(const char *src)
{
    return src == nullptr ? "" : src;
}

QVector4D color::sRGBToLinear(const QColor &color)
{
    const QVector3D rgb(color.redF(), color.greenF(), color.blueF());
    const float C1 = 0.305306011f;
    const QVector3D C2(0.682171111f, 0.682171111f, 0.682171111f);
    const QVector3D C3(0.012522878f, 0.012522878f, 0.012522878f);
    return QVector4D(rgb * (rgb * (rgb * C1 + C2) + C3), color.alphaF());
}

QT_END_NAMESPACE

