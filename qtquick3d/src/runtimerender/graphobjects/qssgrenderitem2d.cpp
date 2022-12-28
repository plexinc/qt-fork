// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


#include <QtQuick3DRuntimeRender/private/qssgrenderitem2d_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderbuffermanager_p.h>
#include <QtQuick/QSGTexture>

QT_BEGIN_NAMESPACE

QSSGRenderItem2D::QSSGRenderItem2D()
    : QSSGRenderNode(QSSGRenderGraphObject::Type::Item2D)
{
}

QSSGRenderItem2D::~QSSGRenderItem2D()
{
    delete m_rp;
}

QT_END_NAMESPACE
