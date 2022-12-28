// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "chat.h"
#include "remoteselector.h"
#include "chatserver.h"
#include "chatclient.h"

#include <QCoreApplication>
#include <QtCore/qdebug.h>

#include <QtBluetooth/qbluetoothdeviceinfo.h>
#include <QtBluetooth/qbluetoothlocaldevice.h>
#include <QtBluetooth/qbluetoothuuid.h>

static const QLatin1String serviceUuid("e8e10f95-1a70-4b27-9ccf-02010264e9c8");
#ifdef Q_OS_ANDROID
static const QLatin1String reverseUuid("c8e96402-0102-cf9c-274b-701a950fe1e8");
#endif

Chat::Chat(QWidget *parent)
    : QDialog(parent), ui(new Ui_Chat)
{
    //! [Construct UI]
    ui->setupUi(this);

    connect(ui->quitButton, &QPushButton::clicked, this, &Chat::accept);
    connect(ui->connectButton, &QPushButton::clicked, this, &Chat::connectClicked);
    connect(ui->sendButton, &QPushButton::clicked, this, &Chat::sendClicked);
    //! [Construct UI]

    localAdapters = QBluetoothLocalDevice::allDevices();
    if (localAdapters.size() < 2) {
        ui->localAdapterBox->setVisible(false);
    } else {
        //we ignore more than two adapters
        ui->localAdapterBox->setVisible(true);
        ui->firstAdapter->setText(tr("Default (%1)", "%1 = Bluetooth address").
                                  arg(localAdapters.at(0).address().toString()));
        ui->secondAdapter->setText(localAdapters.at(1).address().toString());
        ui->firstAdapter->setChecked(true);
        connect(ui->firstAdapter, &QRadioButton::clicked, this, &Chat::newAdapterSelected);
        connect(ui->secondAdapter, &QRadioButton::clicked, this, &Chat::newAdapterSelected);
    }

    // make discoverable
    if (!localAdapters.isEmpty()) {
        QBluetoothLocalDevice adapter(localAdapters.at(0).address());
        adapter.setHostMode(QBluetoothLocalDevice::HostDiscoverable);
    } else {
        qWarning("Local adapter is not found! The application might work incorrectly.");
#ifdef Q_OS_WIN
        // WinRT implementation does not support adapter information yet. So it
        // will always return an empty list.
        qWarning("If the adapter exists, make sure to pair the devices manually before launching"
                 " the chat.");
#endif
    }

    //! [Create Chat Server]
    server = new ChatServer(this);
    connect(server, QOverload<const QString &>::of(&ChatServer::clientConnected),
            this, &Chat::clientConnected);
    connect(server, QOverload<const QString &>::of(&ChatServer::clientDisconnected),
            this,  QOverload<const QString &>::of(&Chat::clientDisconnected));
    connect(server, &ChatServer::messageReceived,
            this,  &Chat::showMessage);
    connect(this, &Chat::sendMessage, server, &ChatServer::sendMessage);
    server->startServer();
    //! [Create Chat Server]

    //! [Get local device name]
    localName = QBluetoothLocalDevice().name();
    //! [Get local device name]
}

Chat::~Chat()
{
    qDeleteAll(clients);
    delete server;
}

//! [clientConnected clientDisconnected]
void Chat::clientConnected(const QString &name)
{
    ui->chat->insertPlainText(QString::fromLatin1("%1 has joined chat.\n").arg(name));
}

void Chat::clientDisconnected(const QString &name)
{
    ui->chat->insertPlainText(QString::fromLatin1("%1 has left.\n").arg(name));
}
//! [clientConnected clientDisconnected]

//! [connected]
void Chat::connected(const QString &name)
{
    ui->chat->insertPlainText(QString::fromLatin1("Joined chat with %1.\n").arg(name));
}
//! [connected]

void Chat::newAdapterSelected()
{
    const int newAdapterIndex = adapterFromUserSelection();
    if (currentAdapterIndex != newAdapterIndex) {
        server->stopServer();
        currentAdapterIndex = newAdapterIndex;
        const QBluetoothHostInfo info = localAdapters.at(currentAdapterIndex);
        QBluetoothLocalDevice adapter(info.address());
        adapter.setHostMode(QBluetoothLocalDevice::HostDiscoverable);
        server->startServer(info.address());
        localName = info.name();
    }
}

int Chat::adapterFromUserSelection() const
{
    int result = 0;
    QBluetoothAddress newAdapter = localAdapters.at(0).address();

    if (ui->secondAdapter->isChecked()) {
        newAdapter = localAdapters.at(1).address();
        result = 1;
    }
    return result;
}

void Chat::reactOnSocketError(const QString &error)
{
    ui->chat->insertPlainText(QString::fromLatin1("%1\n").arg(error));
}

//! [clientDisconnected]
void Chat::clientDisconnected()
{
    ChatClient *client = qobject_cast<ChatClient *>(sender());
    if (client) {
        clients.removeOne(client);
        client->deleteLater();
    }
}
//! [clientDisconnected]

//! [Connect to remote service]
void Chat::connectClicked()
{
    ui->connectButton->setEnabled(false);

    // scan for services
    const QBluetoothAddress adapter = localAdapters.isEmpty() ?
                                           QBluetoothAddress() :
                                           localAdapters.at(currentAdapterIndex).address();

    RemoteSelector remoteSelector(adapter);
#ifdef Q_OS_ANDROID
    // QTBUG-61392
    Q_UNUSED(serviceUuid);
    remoteSelector.startDiscovery(QBluetoothUuid(reverseUuid));
#else
    remoteSelector.startDiscovery(QBluetoothUuid(serviceUuid));
#endif
    if (remoteSelector.exec() == QDialog::Accepted) {
        QBluetoothServiceInfo service = remoteSelector.service();

        qDebug() << "Connecting to service 2" << service.serviceName()
                 << "on" << service.device().name();

        // Create client
        qDebug() << "Going to create client";
        ChatClient *client = new ChatClient(this);
qDebug() << "Connecting...";

        connect(client, &ChatClient::messageReceived,
                this, &Chat::showMessage);
        connect(client, &ChatClient::disconnected,
                this, QOverload<>::of(&Chat::clientDisconnected));
        connect(client, QOverload<const QString &>::of(&ChatClient::connected),
                this, &Chat::connected);
        connect(client, &ChatClient::socketErrorOccurred,
                this, &Chat::reactOnSocketError);
        connect(this, &Chat::sendMessage, client, &ChatClient::sendMessage);
qDebug() << "Start client";
        client->startClient(service);

        clients.append(client);
    }

    ui->connectButton->setEnabled(true);
}
//! [Connect to remote service]

//! [sendClicked]
void Chat::sendClicked()
{
    ui->sendButton->setEnabled(false);
    ui->sendText->setEnabled(false);

    showMessage(localName, ui->sendText->text());
    emit sendMessage(ui->sendText->text());

    ui->sendText->clear();

    ui->sendText->setEnabled(true);
    ui->sendButton->setEnabled(true);
    ui->sendText->setFocus();
}
//! [sendClicked]

//! [showMessage]
void Chat::showMessage(const QString &sender, const QString &message)
{
    ui->chat->insertPlainText(QString::fromLatin1("%1: %2\n").arg(sender, message));
    ui->chat->ensureCursorVisible();
}
//! [showMessage]
