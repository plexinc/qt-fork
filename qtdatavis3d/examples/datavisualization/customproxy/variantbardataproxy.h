// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef VARIANTBARDATAPROXY_H
#define VARIANTBARDATAPROXY_H

#include <QtDataVisualization/qbardataproxy.h>
#include "variantdataset.h"
#include "variantbardatamapping.h"
#include <QtCore/QStringList>
#include <QtCore/QMap>
#include <QtCore/QPointer>

//! [0]
class VariantBarDataProxy : public QBarDataProxy
//! [0]
{
    Q_OBJECT

public:
    explicit VariantBarDataProxy();
    explicit VariantBarDataProxy(VariantDataSet *newSet, VariantBarDataMapping *mapping);
    virtual ~VariantBarDataProxy();

    //! [1]
    // Doesn't gain ownership of the dataset, but does connect to it to listen for data changes.
    void setDataSet(VariantDataSet *newSet);
    VariantDataSet *dataSet();

    // Map key (row, column, value) to value index in data item (VariantItem).
    // Doesn't gain ownership of mapping, but does connect to it to listen for mapping changes.
    // Modifying mapping that is set to proxy will trigger dataset re-resolving.
    void setMapping(VariantBarDataMapping *mapping);
    VariantBarDataMapping *mapping();
    //! [1]

public Q_SLOTS:
    void handleItemsAdded(int index, int count);
    void handleDataCleared();
    void handleMappingChanged();

private:
    void resolveDataSet();

    QPointer<VariantDataSet> m_dataSet;
    QPointer<VariantBarDataMapping> m_mapping;

    Q_DISABLE_COPY(VariantBarDataProxy)
};

#endif
