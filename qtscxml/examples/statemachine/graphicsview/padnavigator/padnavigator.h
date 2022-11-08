// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PADNAVIGATOR_H
#define PADNAVIGATOR_H

#include <QGraphicsView>
#include "ui_form.h"

//! [0]
class PadNavigator : public QGraphicsView
{
    Q_OBJECT
public:
    explicit PadNavigator(const QSize &size, QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::Form form;
};
//! [0]

#endif // PADNAVIGATOR_H
