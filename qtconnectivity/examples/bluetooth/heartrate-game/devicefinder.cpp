// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "devicefinder.h"
#include "devicehandler.h"
#include "deviceinfo.h"
#include "heartrate-global.h"

DeviceFinder::DeviceFinder(DeviceHandler *handler, QObject *parent):
    BluetoothBaseClass(parent),
    m_deviceHandler(handler)
{
    //! [devicediscovery-1]
    m_deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    m_deviceDiscoveryAgent->setLowEnergyDiscoveryTimeout(15000);

    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &DeviceFinder::addDevice);
    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred, this,
            &DeviceFinder::scanError);

    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &DeviceFinder::scanFinished);
    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled, this, &DeviceFinder::scanFinished);
    //! [devicediscovery-1]


    if (simulator) {
        m_demoTimer.setSingleShot(true);
        m_demoTimer.setInterval(2000);
        connect(&m_demoTimer, &QTimer::timeout, this, &DeviceFinder::scanFinished);
    }
}

DeviceFinder::~DeviceFinder()
{
    qDeleteAll(m_devices);
    m_devices.clear();
}

void DeviceFinder::startSearch()
{
    clearMessages();
    m_deviceHandler->setDevice(nullptr);
    qDeleteAll(m_devices);
    m_devices.clear();

    emit devicesChanged();

    if (simulator) {
        m_demoTimer.start();
    }  else {
        //! [devicediscovery-2]
        m_deviceDiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
        //! [devicediscovery-2]
    }

    emit scanningChanged();
    setInfo(tr("Scanning for devices..."));
}

//! [devicediscovery-3]
void DeviceFinder::addDevice(const QBluetoothDeviceInfo &device)
{
    // If device is LowEnergy-device, add it to the list
    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        m_devices.append(new DeviceInfo(device));
        setInfo(tr("Low Energy device found. Scanning more..."));
//! [devicediscovery-3]
        emit devicesChanged();
//! [devicediscovery-4]
    }
    //...
}
//! [devicediscovery-4]

void DeviceFinder::scanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    if (error == QBluetoothDeviceDiscoveryAgent::PoweredOffError)
        setError(tr("The Bluetooth adaptor is powered off."));
    else if (error == QBluetoothDeviceDiscoveryAgent::InputOutputError)
        setError(tr("Writing or reading from the device resulted in an error."));
    else
        setError(tr("An unknown error has occurred."));
}

void DeviceFinder::scanFinished()
{
    if (simulator) {
        // Only for testing
        for (int i = 0; i < 4; i++)
            m_devices.append(new DeviceInfo(QBluetoothDeviceInfo()));
    }

    if (m_devices.isEmpty())
        setError(tr("No Low Energy devices found."));
    else
        setInfo(tr("Scanning done."));

    emit scanningChanged();
    emit devicesChanged();
}

void DeviceFinder::connectToService(const QString &address)
{
    m_deviceDiscoveryAgent->stop();

    DeviceInfo *currentDevice = nullptr;
    for (QObject *entry : qAsConst(m_devices)) {
        auto device = qobject_cast<DeviceInfo *>(entry);
        if (device && device->getAddress() == address ) {
            currentDevice = device;
            break;
        }
    }

    if (currentDevice)
        m_deviceHandler->setDevice(currentDevice);

    clearMessages();
}

bool DeviceFinder::scanning() const
{
    if (simulator)
        return m_demoTimer.isActive();
    return m_deviceDiscoveryAgent->isActive();
}

QVariant DeviceFinder::devices()
{
    return QVariant::fromValue(m_devices);
}
