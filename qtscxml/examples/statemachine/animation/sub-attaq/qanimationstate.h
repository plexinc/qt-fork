// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef QANIMATIONSTATE_H
#define QANIMATIONSTATE_H

#include <QState>

QT_BEGIN_NAMESPACE

class QAbstractAnimation;

class QAnimationState : public QState
{
    Q_OBJECT
public:
    QAnimationState(QState *parent = nullptr);
    ~QAnimationState();

    void setAnimation(QAbstractAnimation *animation);
    QAbstractAnimation* animation() const;

signals:
    void animationFinished();

protected:
    void onEntry(QEvent *) override;
    void onExit(QEvent *) override;
    bool event(QEvent *e) override;

private:
    Q_DISABLE_COPY(QAnimationState)
    QAbstractAnimation *m_animation;
};

QT_END_NAMESPACE

#endif // QANIMATIONSTATE_H
