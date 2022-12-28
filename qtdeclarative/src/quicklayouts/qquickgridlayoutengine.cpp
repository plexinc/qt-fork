// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickitem.h"
#include "qquickgridlayoutengine_p.h"
#include "qquicklayout_p.h"

QT_BEGIN_NAMESPACE

void QQuickGridLayoutEngine::setAlignment(QQuickItem *quickItem, Qt::Alignment alignment)
{
    if (QQuickGridLayoutItem *item = findLayoutItem(quickItem)) {
        item->setAlignment(alignment);
        invalidate();
    }
}

Qt::Alignment QQuickGridLayoutEngine::alignment(QQuickItem *quickItem) const
{
    if (QGridLayoutItem *item = findLayoutItem(quickItem))
        return item->alignment();
    return {};
}

QT_END_NAMESPACE
