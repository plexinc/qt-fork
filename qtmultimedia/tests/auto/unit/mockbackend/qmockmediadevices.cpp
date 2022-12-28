// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qmockmediadevices_p.h"
#include "private/qcameradevice_p.h"

QT_BEGIN_NAMESPACE

QMockMediaDevices::QMockMediaDevices()
    : QPlatformMediaDevices()
{
    setDevices(this);
}

QMockMediaDevices::~QMockMediaDevices() = default;

QList<QAudioDevice> QMockMediaDevices::audioInputs() const
{
    return m_inputDevices;
}

QList<QAudioDevice> QMockMediaDevices::audioOutputs() const
{
    return m_outputDevices;
}

QPlatformAudioSource *QMockMediaDevices::createAudioSource(const QAudioDevice &info)
{
    Q_UNUSED(info);
    return nullptr;// ###
}

QPlatformAudioSink *QMockMediaDevices::createAudioSink(const QAudioDevice &info)
{
    Q_UNUSED(info);
    return nullptr; //###
}


QT_END_NAMESPACE
