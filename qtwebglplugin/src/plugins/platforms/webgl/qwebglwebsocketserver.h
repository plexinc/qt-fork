/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt WebGL module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWEBGLWEBSOCKETSERVER_H
#define QWEBGLWEBSOCKETSERVER_H

#include <QtCore/qobject.h>
#include <QtCore/qscopedpointer.h>

QT_BEGIN_NAMESPACE

class QMutex;
class QWebSocket;
class QWaitCondition;
class QWebGLWebSocketServerPrivate;

class QWebGLWebSocketServer : public QObject
{
    Q_OBJECT

public:
    enum class MessageType
    {
        Connect,
        GlCommand,
        CreateCanvas,
        DestroyCanvas,
        OpenUrl,
        ChangeTitle
    };

    QWebGLWebSocketServer(quint16 port, QObject *parent = nullptr);
    ~QWebGLWebSocketServer() override;

    quint16 port() const;

    QMutex *mutex();
    QWaitCondition *waitCondition();

    QVariant queryValue(int id);

public slots:
    void create();
    void sendMessage(QWebSocket *socket,
                     QWebGLWebSocketServer::MessageType type,
                     const QVariantMap &values);

protected:
    bool event(QEvent *event) override;

private slots:
    void onNewConnection();
    void onDisconnect();
    void onTextMessageReceived(const QString &message);

private:
    Q_DISABLE_COPY(QWebGLWebSocketServer)
    Q_DECLARE_PRIVATE(QWebGLWebSocketServer)
    QScopedPointer<QWebGLWebSocketServerPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QWEBGLWEBSOCKETSERVER_H
