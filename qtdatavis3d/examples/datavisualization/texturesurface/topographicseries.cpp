// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "topographicseries.h"

//! [0]
// Value used to encode height data as RGB value on PNG file
const float packingFactor = 11983.0f;
//! [0]

TopographicSeries::TopographicSeries()
{
    setDrawMode(QSurface3DSeries::DrawSurface);
    setFlatShadingEnabled(true);
    setBaseColor(Qt::white);
}

TopographicSeries::~TopographicSeries()
{
}

void TopographicSeries::setTopographyFile(const QString file, float width, float height)
{
//! [1]
    QImage heightMapImage(file);
    uchar *bits = heightMapImage.bits();
    int imageHeight = heightMapImage.height();
    int imageWidth = heightMapImage.width();
    int widthBits = imageWidth * 4;
    float stepX = width / float(imageWidth);
    float stepZ = height / float(imageHeight);

    QSurfaceDataArray *dataArray = new QSurfaceDataArray;
    dataArray->reserve(imageHeight);
    for (int i = 0; i < imageHeight; i++) {
        int p = i * widthBits;
        float z = height - float(i) * stepZ;
        QSurfaceDataRow *newRow = new QSurfaceDataRow(imageWidth);
        for (int j = 0; j < imageWidth; j++) {
            uchar aa = bits[p + 0];
            uchar rr = bits[p + 1];
            uchar gg = bits[p + 2];
            uint color = uint((gg << 16) + (rr << 8) + aa);
            float y = float(color) / packingFactor;
            (*newRow)[j].setPosition(QVector3D(float(j) * stepX, y, z));
            p = p + 4;
        }
        *dataArray << newRow;
    }

    dataProxy()->resetArray(dataArray);
//! [1]

    m_sampleCountX = float(imageWidth);
    m_sampleCountZ = float(imageHeight);
}
