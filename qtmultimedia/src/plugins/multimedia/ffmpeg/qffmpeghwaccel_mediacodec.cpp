// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qffmpeghwaccel_mediacodec_p.h"

#include <androidsurfacetexture_p.h>
#include <QtGui/private/qrhi_p.h>

extern "C" {
#include <libavcodec/mediacodec.h>
}

#if !defined(Q_OS_ANDROID)
#    error "Configuration error"
#endif

namespace QFFmpeg {

Q_GLOBAL_STATIC(AndroidSurfaceTexture, androidSurfaceTexture, 0);

class MediaCodecTextureSet : public TextureSet
{
public:
    MediaCodecTextureSet(qint64 textureHandle) : handle(textureHandle) { }

    qint64 textureHandle(int plane) override { return (plane == 0) ? handle : 0; }

private:
    qint64 handle;
};

void MediaCodecTextureConverter::setupDecoderSurface(AVCodecContext *avCodecContext)
{
    AVMediaCodecContext *mediacodecContext = av_mediacodec_alloc_context();
    av_mediacodec_default_init(avCodecContext, mediacodecContext, androidSurfaceTexture->surface());
}

TextureSet *MediaCodecTextureConverter::getTextures(AVFrame *frame)
{
    if (!androidSurfaceTexture->isValid())
        return {};

    if (!externalTexture) {
        androidSurfaceTexture->detachFromGLContext();
        externalTexture = std::unique_ptr<QRhiTexture>(
                rhi->newTexture(QRhiTexture::Format::RGBA8, { frame->width, frame->height }, 1,
                                QRhiTexture::ExternalOES));

        if (!externalTexture->create()) {
            qWarning() << "Failed to create the external texture!";
            return {};
        }

        quint64 textureHandle = externalTexture->nativeTexture().object;
        androidSurfaceTexture->attachToGLContext(textureHandle);
    }

    // release a MediaCodec buffer and render it to the surface
    AVMediaCodecBuffer *buffer = (AVMediaCodecBuffer *)frame->data[3];
    int result = av_mediacodec_release_buffer(buffer, 1);
    if (result < 0) {
        qWarning() << "Failed to render buffer to surface.";
        return {};
    }

    androidSurfaceTexture->updateTexImage();

    return new MediaCodecTextureSet(externalTexture->nativeTexture().object);
}
}
