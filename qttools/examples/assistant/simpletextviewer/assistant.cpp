// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "assistant.h"

#include <QApplication>
#include <QByteArray>
#include <QDir>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QStandardPaths>

Assistant::Assistant() = default;

//! [0]
Assistant::~Assistant()
{
    if (!m_process.isNull() && m_process->state() == QProcess::Running) {
        QObject::disconnect(m_process.data(), &QProcess::finished, nullptr, nullptr);
        m_process->terminate();
        m_process->waitForFinished(3000);
    }
}
//! [0]

//! [1]
void Assistant::showDocumentation(const QString &page)
{
    if (!startAssistant())
        return;

    QByteArray ba("SetSource ");
    ba.append("qthelp://org.qt-project.examples.simpletextviewer/doc/");

    m_process->write(ba + page.toLocal8Bit() + '\n');
}
//! [1]

QString documentationDirectory()
{
    QStringList paths;
#ifdef SRCDIR
    paths.append(QLatin1String(SRCDIR));
#endif
    paths.append(QLibraryInfo::path(QLibraryInfo::ExamplesPath));
    paths.append(QCoreApplication::applicationDirPath());
    paths.append(QStandardPaths::standardLocations(QStandardPaths::AppDataLocation));
    for (const auto &dir : qAsConst(paths)) {
        const QString path = dir + QLatin1String("/documentation");
        if (QFileInfo::exists(path))
            return path;
    }
    return QString();
}

//! [2]
bool Assistant::startAssistant()
{
    if (m_process.isNull()) {
        m_process.reset(new QProcess());
        QObject::connect(m_process.data(), &QProcess::finished,
                         m_process.data(), [this](int exitCode, QProcess::ExitStatus status) {
                             this->finished(exitCode, status);
                         });
    }

    if (m_process->state() != QProcess::Running) {
        QString app = QLibraryInfo::path(QLibraryInfo::BinariesPath);
#ifndef Q_OS_DARWIN
        app += QLatin1String("/assistant");
#else
        app += QLatin1String("/Assistant.app/Contents/MacOS/Assistant");
#endif

        const QString collectionDirectory = documentationDirectory();
        if (collectionDirectory.isEmpty()) {
            showError(tr("The documentation directory cannot be found"));
            return false;
        }

        QStringList args{QLatin1String("-collectionFile"),
                         collectionDirectory + QLatin1String("/simpletextviewer.qhc"),
                         QLatin1String("-enableRemoteControl")};

        m_process->start(app, args);

        if (!m_process->waitForStarted()) {
            showError(tr("Unable to launch Qt Assistant (%1): %2")
                              .arg(QDir::toNativeSeparators(app), m_process->errorString()));
            return false;
        }
    }
    return true;
}
//! [2]

void Assistant::showError(const QString &message)
{
    QMessageBox::critical(QApplication::activeWindow(),
                          tr("Simple Text Viewer"), message);
}

void Assistant::finished(int exitCode, QProcess::ExitStatus status)
{
    const QString stdErr = QString::fromLocal8Bit(m_process->readAllStandardError());
    if (status != QProcess::NormalExit) {
        showError(tr("Assistant crashed: ").arg(stdErr));
    } else if (exitCode != 0) {
        showError(tr("Assistant exited with %1: %2").arg(exitCode).arg(stdErr));
    }
}
