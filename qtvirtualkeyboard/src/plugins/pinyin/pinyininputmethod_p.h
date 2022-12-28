// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef PINYININPUTMETHOD_P_H
#define PINYININPUTMETHOD_P_H

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

#include <QtVirtualKeyboard/qvirtualkeyboardabstractinputmethod.h>

QT_BEGIN_NAMESPACE
namespace QtVirtualKeyboard {

class PinyinInputMethodPrivate;

class PinyinInputMethod : public QVirtualKeyboardAbstractInputMethod
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(PinyinInputMethod)
    QML_ELEMENT

public:
    explicit PinyinInputMethod(QObject *parent = nullptr);
    ~PinyinInputMethod();

    QList<QVirtualKeyboardInputEngine::InputMode> inputModes(const QString &locale) override;
    bool setInputMode(const QString &locale, QVirtualKeyboardInputEngine::InputMode inputMode) override;
    bool setTextCase(QVirtualKeyboardInputEngine::TextCase textCase) override;

    bool keyEvent(Qt::Key key, const QString &text, Qt::KeyboardModifiers modifiers) override;

    QList<QVirtualKeyboardSelectionListModel::Type> selectionLists() override;
    int selectionListItemCount(QVirtualKeyboardSelectionListModel::Type type) override;
    QVariant selectionListData(QVirtualKeyboardSelectionListModel::Type type, int index,
                               QVirtualKeyboardSelectionListModel::Role role) override;
    void selectionListItemSelected(QVirtualKeyboardSelectionListModel::Type type, int index) override;

    void reset() override;
    void update() override;

private:
    QScopedPointer<PinyinInputMethodPrivate> d_ptr;
};

} // namespace QtVirtualKeyboard
QT_END_NAMESPACE

#endif // PINYININPUTMETHOD_P_H
