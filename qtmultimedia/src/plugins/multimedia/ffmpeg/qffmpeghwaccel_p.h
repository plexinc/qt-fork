// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef QFFMPEGHWACCEL_P_H
#define QFFMPEGHWACCEL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qffmpeg_p.h"
#include "qvideoframeformat.h"
#include <private/qabstractvideobuffer_p.h>
#include <qshareddata.h>
#include <memory>

QT_BEGIN_NAMESPACE

class QRhi;
class QRhiTexture;
class QFFmpegVideoBuffer;

namespace QFFmpeg {

// used for the get_format callback for the decoder
enum AVPixelFormat getFormat(struct AVCodecContext *s, const enum AVPixelFormat * fmt);

class HWAccel;

class TextureSet {
public:
    // ### Should add QVideoFrameFormat::PixelFormat here
    virtual ~TextureSet() {}
    virtual qint64 textureHandle(int /*plane*/) { return 0; }
};

class TextureConverterBackend
{
public:
    TextureConverterBackend(QRhi *rhi)
        : rhi(rhi)
    {}
    virtual ~TextureConverterBackend() {}
    virtual TextureSet *getTextures(AVFrame * /*frame*/) { return nullptr; }

    QRhi *rhi = nullptr;
};

class TextureConverter
{
    class Data final
    {
    public:
        ~Data();
        QAtomicInt ref = 0;
        QRhi *rhi = nullptr;
        AVPixelFormat format = AV_PIX_FMT_NONE;
        TextureConverterBackend *backend = nullptr;
    };
public:
    TextureConverter(QRhi *rhi = nullptr);

    void init(AVFrame *frame) {
        AVPixelFormat fmt = frame ? AVPixelFormat(frame->format) : AV_PIX_FMT_NONE;
        if (fmt != d->format)
            updateBackend(fmt);
    }
    TextureSet *getTextures(AVFrame *frame);
    bool isNull() const { return !d->backend || !d->backend->rhi; }

private:
    void updateBackend(AVPixelFormat format);

    QExplicitlySharedDataPointer<Data> d;
};

class HWAccel
{
    AVBufferRef *m_hwDeviceContext = nullptr;
    AVBufferRef *m_hwFramesContext = nullptr;

public:
    ~HWAccel();

    static std::unique_ptr<HWAccel> create(const AVCodec *decoder);
    static std::unique_ptr<HWAccel> create(AVHWDeviceType deviceType);
    static std::unique_ptr<HWAccel> findHardwareAccelForCodecID(AVCodecID id);

    static const AVCodec *hardwareDecoderForCodecId(AVCodecID id);
    const AVCodec *hardwareEncoderForCodecId(AVCodecID id) const;

    AVHWDeviceType deviceType() const;

    AVBufferRef *hwDeviceContextAsBuffer() const { return m_hwDeviceContext; }
    AVHWDeviceContext *hwDeviceContext() const;
    AVPixelFormat hwFormat() const;

    void createFramesContext(AVPixelFormat swFormat, const QSize &size);
    AVBufferRef *hwFramesContextAsBuffer() const { return m_hwFramesContext; }
    AVHWFramesContext *hwFramesContext() const;

    static AVPixelFormat format(AVFrame *frame);
    static std::pair<const AVHWDeviceType*, qsizetype> preferredDeviceTypes();

private:
    HWAccel(AVBufferRef *hwDeviceContext, AVBufferRef *hwFrameContext = nullptr)
        : m_hwDeviceContext(hwDeviceContext), m_hwFramesContext(hwFrameContext)
    {}
};

}

QT_END_NAMESPACE

#endif
