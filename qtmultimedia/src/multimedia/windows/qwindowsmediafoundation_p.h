// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWINDOWSMEDIAFOUNDATION_H
#define QWINDOWSMEDIAFOUNDATION_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/qtmultimediaglobal_p.h>
#include <QtCore/qt_windows.h>
#include <QtCore/private/qsystemlibrary_p.h>
#include <memory>
#include <mfapi.h>

struct IMFMediaType;

QT_BEGIN_NAMESPACE

class QWindowsMediaFoundation
{
public:
    ~QWindowsMediaFoundation();

    static QWindowsMediaFoundation *instance();

    decltype(&::MFCreateMediaType) mfCreateMediaType = nullptr;
    decltype(&::MFCreateMemoryBuffer) mfCreateMemoryBuffer = nullptr;
    decltype(&::MFCreateSample) mfCreateSample = nullptr;

private:
    QWindowsMediaFoundation() : m_mfplat(QStringLiteral("Mfplat.dll")) {}

    QSystemLibrary m_mfplat;
};

QT_END_NAMESPACE

#endif // QWINDOWSMEDIAFOUNDATION_H
