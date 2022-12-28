// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qquick3dcubemaptexture_p.h"
#include "qquick3dscenerenderer_p.h"
#include "qquick3dsceneenvironment_p.h"
#include "qquick3dobject_p.h"
#include "qquick3dnode_p.h"
#include "qquick3dscenemanager_p.h"
#include "qquick3dtexture_p.h"
#include "qquick3dcamera_p.h"
#include "qquick3dpickresult_p.h"
#include "qquick3dmodel_p.h"
#include "qquick3drenderstats_p.h"
#include <QtQuick3DUtils/private/qquick3dprofiler_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrendererutil_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderer_p.h>

#include <QtQuick/private/qquickwindow_p.h>
#include <QtQuick/private/qsgdefaultrendercontext_p.h>
#include <QtQuick/private/qsgtexture_p.h>
#include <QtQuick/private/qsgplaintexture_p.h>
#include <QtQuick/private/qsgrendernode_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrendereffect_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrhieffectsystem_p.h>
#include <QtQuick3DRuntimeRender/private/qssglayerrenderdata_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrhiquadrenderer_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrhicontext_p.h>

#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

static bool dumpRenderTimes = false;

#if QT_CONFIG(qml_debug)

static quint64 statsDrawCalls(const QSSGRhiContextStats& stats)
{
    quint64 count = 0;
    for (auto pass : stats.renderPasses)
        count += pass.draws.callCount + pass.indexedDraws.callCount;
    count += stats.externalRenderPass.draws.callCount + stats.externalRenderPass.indexedDraws.callCount;
    return count;
}

#define STAT_PAYLOAD(stats) \
    (statsDrawCalls(stats) | (quint64(stats.renderPasses.size()) << 32))

#endif

SGFramebufferObjectNode::SGFramebufferObjectNode()
    : window(nullptr)
    , renderer(nullptr)
    , renderPending(true)
    , invalidatePending(false)
    , devicePixelRatio(1)
{
    qsgnode_set_description(this, QStringLiteral("fbonode"));
    setFlag(QSGNode::UsePreprocess, true);
}

SGFramebufferObjectNode::~SGFramebufferObjectNode()
{
    delete renderer;
    delete texture();
}

void SGFramebufferObjectNode::scheduleRender()
{
    renderPending = true;
    markDirty(DirtyMaterial);
}

QSGTexture *SGFramebufferObjectNode::texture() const
{
    return QSGSimpleTextureNode::texture();
}

void SGFramebufferObjectNode::preprocess()
{
    render();
}

// QQuickWindow::update() behaves differently depending on whether it's called from the GUI thread
// or the render thread.
// TODO: move this to QQuickWindow::fullUpdate(), if we can't change update()
static void requestFullUpdate(QQuickWindow *window)
{
    if (QThread::currentThread() == QCoreApplication::instance()->thread())
        window->update();
    else
        QCoreApplication::postEvent(window, new QEvent(QEvent::Type(QQuickWindowPrivate::FullUpdateRequest)));
}

void SGFramebufferObjectNode::render()
{
    if (renderPending) {
        if (renderer->renderStats())
            renderer->renderStats()->startRender();

        renderPending = false;

        if (renderer->m_sgContext->rhiContext()->isValid()) {
            QRhiTexture *rhiTexture = renderer->renderToRhiTexture(window);
            bool needsNewWrapper = false;
            if (!texture() || (texture()->textureSize() != renderer->surfaceSize()
                               || texture()->rhiTexture() != rhiTexture))
            {
                needsNewWrapper = true;
            }
            if (needsNewWrapper) {
                delete texture();
                QSGPlainTexture *t = new QSGPlainTexture;
                t->setOwnsTexture(false);
                t->setHasAlphaChannel(true);
                t->setTexture(rhiTexture);
                t->setTextureSize(renderer->surfaceSize());
                setTexture(t);
            }
        }

        markDirty(QSGNode::DirtyMaterial);
        emit textureChanged();

        if (renderer->renderStats())
            renderer->renderStats()->endRender(dumpRenderTimes);

        if (renderer->m_sgContext->renderer()->rendererRequestsFrames()
                || renderer->requestedFramesCount > 0) {
            scheduleRender();
            requestFullUpdate(window);
            if (renderer->requestedFramesCount > 0)
                renderer->requestedFramesCount--;
        }
    }
}

void SGFramebufferObjectNode::handleScreenChange()
{
    if (!qFuzzyCompare(window->effectiveDevicePixelRatio(), devicePixelRatio)) {
        renderer->invalidateFramebufferObject();
        quickFbo->update();
    }
}


QQuick3DSceneRenderer::QQuick3DSceneRenderer(const QSSGRef<QSSGRenderContextInterface> &rci)
    : m_sgContext(rci)
{
    dumpRenderTimes = (qEnvironmentVariableIntValue("QT_QUICK3D_DUMP_RENDERTIMES") > 0);
}

QQuick3DSceneRenderer::~QQuick3DSceneRenderer()
{
    delete m_layer;

    delete m_texture;

    releaseAaDependentRhiResources();
    delete m_effectSystem;
}

void QQuick3DSceneRenderer::releaseAaDependentRhiResources()
{
    QSSGRhiContext *rhiCtx = m_sgContext->rhiContext().data();
    if (!rhiCtx->isValid())
        return;

    delete m_textureRenderTarget;
    m_textureRenderTarget = nullptr;

    delete m_textureRenderPassDescriptor;
    m_textureRenderPassDescriptor = nullptr;

    delete m_depthStencilBuffer;
    m_depthStencilBuffer = nullptr;

    delete m_msaaRenderBuffer;
    m_msaaRenderBuffer = nullptr;

    delete m_ssaaTexture;
    m_ssaaTexture = nullptr;

    delete m_ssaaTextureToTextureRenderTarget;
    m_ssaaTextureToTextureRenderTarget = nullptr;

    delete m_ssaaTextureToTextureRenderPassDescriptor;
    m_ssaaTextureToTextureRenderPassDescriptor = nullptr;

    delete m_temporalAATexture;
    m_temporalAATexture = nullptr;
    delete m_temporalAARenderTarget;
    m_temporalAARenderTarget = nullptr;
    delete m_temporalAARenderPassDescriptor;
    m_temporalAARenderPassDescriptor = nullptr;

    delete m_prevTempAATexture;
    m_prevTempAATexture = nullptr;
}

// Blend factors are in the form of (frame blend factor, accumulator blend factor)
static const QVector2D s_ProgressiveAABlendFactors[QSSGLayerRenderData::MAX_AA_LEVELS] = {
    QVector2D(0.500000f, 0.500000f), // 1x
    QVector2D(0.333333f, 0.666667f), // 2x
    QVector2D(0.250000f, 0.750000f), // 3x
    QVector2D(0.200000f, 0.800000f), // 4x
    QVector2D(0.166667f, 0.833333f), // 5x
    QVector2D(0.142857f, 0.857143f), // 6x
    QVector2D(0.125000f, 0.875000f), // 7x
    QVector2D(0.111111f, 0.888889f), // 8x
};

static const QVector2D s_TemporalAABlendFactors = { 0.5f, 0.5f };

QRhiTexture *QQuick3DSceneRenderer::renderToRhiTexture(QQuickWindow *qw)
{
    if (!m_layer)
        return nullptr;

    QRhiTexture *currentTexture = m_texture; // the result so far

    if (qw) {
        if (m_renderStats)
            m_renderStats->startRenderPrepare();

        Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DPrepareFrame);

        QSSGRhiContext *rhiCtx = m_sgContext->rhiContext().data();

        rhiCtx->setMainRenderPassDescriptor(m_textureRenderPassDescriptor);
        rhiCtx->setRenderTarget(m_textureRenderTarget);

        QRhiCommandBuffer *cb = nullptr;
        QSGRendererInterface *rif = qw->rendererInterface();
        QRhiSwapChain *swapchain = static_cast<QRhiSwapChain *>(
            rif->getResource(qw, QSGRendererInterface::RhiSwapchainResource));
        if (swapchain) {
            cb = swapchain->currentFrameCommandBuffer();
            rhiCtx->setCommandBuffer(cb);
        } else {
            cb = static_cast<QRhiCommandBuffer *>(
                rif->getResource(qw, QSGRendererInterface::RhiRedirectCommandBuffer));
            if (cb)
                rhiCtx->setCommandBuffer(cb);
            else {
                qWarning("Neither swapchain nor redirected command buffer are available.");
                return currentTexture;
            }
        }

        // Graphics pipeline objects depend on the MSAA sample count, so the
        // renderer needs to know the value.
        rhiCtx->setMainPassSampleCount(m_msaaRenderBuffer ? m_msaaRenderBuffer->sampleCount() : 1);

        int ssaaAdjustedWidth = m_surfaceSize.width();
        int ssaaAdjustedHeight = m_surfaceSize.height();
        if (m_layer->antialiasingMode == QSSGRenderLayer::AAMode::SSAA) {
            ssaaAdjustedWidth *= m_ssaaMultiplier;
            ssaaAdjustedHeight *= m_ssaaMultiplier;
        }

        float dpr = m_sgContext->dpr();
        const QRect vp = QRect(0, 0, ssaaAdjustedWidth, ssaaAdjustedHeight);
        beginFrame();
        rhiPrepare(vp, dpr);

        if (m_renderStats)
            m_renderStats->endRenderPrepare();

        Q_QUICK3D_PROFILE_END(QQuick3DProfiler::Quick3DPrepareFrame);

        Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderFrame);

        // This is called from the node's preprocess() meaning Qt Quick has not
        // actually began recording a renderpass. Do our own.
        QColor clearColor = Qt::transparent;
        if (m_backgroundMode == QSSGRenderLayer::Background::Color
                || (m_backgroundMode == QSSGRenderLayer::Background::SkyBoxCubeMap && !m_layer->skyBoxCubeMap)
                || (m_backgroundMode == QSSGRenderLayer::Background::SkyBox && !m_layer->lightProbe)) {
            clearColor = m_backgroundColor;
        }
        cb->beginPass(m_textureRenderTarget, clearColor, { 1.0f, 0 }, nullptr, QSSGRhiContext::commonPassFlags());
        QSSGRHICTX_STAT(rhiCtx, beginRenderPass(m_textureRenderTarget));
        rhiRender();
        cb->endPass();
        QSSGRHICTX_STAT(rhiCtx, endRenderPass());

        const bool temporalAA = m_layer->temporalAAIsActive;
        const bool progressiveAA = m_layer->progressiveAAIsActive;
        const bool superSamplingAA = m_layer->antialiasingMode == QSSGRenderLayer::AAMode::SSAA;
        QRhi *rhi = rhiCtx->rhi();

        currentTexture = superSamplingAA ? m_ssaaTexture : m_texture;

        // Do effects before antialiasing
        if (m_effectSystem && m_layer->firstEffect) {
            const auto &renderer = m_sgContext->renderer();
            QSSGLayerRenderData *theRenderData = renderer->getOrCreateLayerRenderData(*m_layer);
            Q_ASSERT(theRenderData);
            QRhiTexture *theDepthTexture = theRenderData->rhiDepthTexture.texture;
            QVector2D cameraClipRange(m_layer->renderedCamera->clipNear, m_layer->renderedCamera->clipFar);

            currentTexture = m_effectSystem->process(m_sgContext->rhiContext(),
                                                     m_sgContext->renderer(),
                                                     *m_layer->firstEffect,
                                                     currentTexture,
                                                     theDepthTexture,
                                                     cameraClipRange);
        }

        // The only difference between temporal and progressive AA at this point is that tempAA always
        // uses blend factors of 0.5 and copies currentTexture to m_prevTempAATexture, while progAA uses blend
        // factors from a table and copies the blend result to m_prevTempAATexture

        if ((progressiveAA || temporalAA) && m_prevTempAATexture) {
            cb->debugMarkBegin(QByteArrayLiteral("Temporal AA"));

            QRhiTexture *blendResult;
            uint *aaIndex = progressiveAA ? &m_layer->progAAPassIndex : &m_layer->tempAAPassIndex; // TODO: can we use only one index?

            if (*aaIndex > 0) {
                if (temporalAA || *aaIndex < quint32(m_layer->antialiasingQuality)) {
                    const auto &renderer = m_sgContext->renderer();

                    // The fragment shader relies on per-target compilation and
                    // QSHADER_ macros of qsb, hence no need to communicate a flip
                    // flag from here.
                    QSSGRef<QSSGRhiShaderPipeline> shaderPipeline = renderer->getRhiProgressiveAAShader();
                    QRhiResourceUpdateBatch *rub = nullptr;

                    QSSGRhiDrawCallData &dcd(rhiCtx->drawCallData({ m_layer, nullptr, nullptr, 0, QSSGRhiDrawCallDataKey::ProgressiveAA }));
                    QRhiBuffer *&ubuf = dcd.ubuf;
                    const int ubufSize = 2 * sizeof(float);
                    if (!ubuf) {
                        ubuf = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, ubufSize);
                        ubuf->create();
                    }

                    rub = rhi->nextResourceUpdateBatch();
                    int idx = *aaIndex - 1;
                    const QVector2D *blendFactors = progressiveAA ? &s_ProgressiveAABlendFactors[idx] : &s_TemporalAABlendFactors;
                    rub->updateDynamicBuffer(ubuf, 0, 2 * sizeof(float), blendFactors);
                    renderer->rhiQuadRenderer()->prepareQuad(rhiCtx, rub);

                    QRhiSampler *sampler = rhiCtx->sampler({ QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                                             QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge, QRhiSampler::Repeat });
                    QSSGRhiShaderResourceBindingList bindings;
                    bindings.addUniformBuffer(0, QRhiShaderResourceBinding::FragmentStage, ubuf);
                    bindings.addTexture(1, QRhiShaderResourceBinding::FragmentStage, currentTexture, sampler);
                    bindings.addTexture(2, QRhiShaderResourceBinding::FragmentStage, m_prevTempAATexture, sampler);

                    QRhiShaderResourceBindings *srb = rhiCtx->srb(bindings);

                    QSSGRhiGraphicsPipelineState ps;
                    const QSize textureSize = currentTexture->pixelSize();
                    ps.viewport = QRhiViewport(0, 0, float(textureSize.width()), float(textureSize.height()));
                    ps.shaderPipeline = shaderPipeline.data();

                    renderer->rhiQuadRenderer()->recordRenderQuadPass(rhiCtx, &ps, srb, m_temporalAARenderTarget, QSSGRhiQuadRenderer::UvCoords);
                    blendResult = m_temporalAATexture;
                } else {
                    blendResult = m_prevTempAATexture;
                }
            } else {
                // For the first frame: no blend, only copy
                blendResult = currentTexture;
            }

            QRhiCommandBuffer *cb = rhiCtx->commandBuffer();

            if (temporalAA || (*aaIndex < quint32(m_layer->antialiasingQuality))) {
                auto *rub = rhi->nextResourceUpdateBatch();
                if (progressiveAA)
                    rub->copyTexture(m_prevTempAATexture, blendResult);
                else
                    rub->copyTexture(m_prevTempAATexture, currentTexture);
                cb->resourceUpdate(rub);
            }

            (*aaIndex)++;
            cb->debugMarkEnd();
            currentTexture = blendResult;
        }

        if (m_layer->antialiasingMode == QSSGRenderLayer::AAMode::SSAA) {
            // With supersampling antialiasing we at this point have the
            // content rendered at a larger size into m_ssaaTexture. Now scale
            // it down to the expected size into m_texture, using linear
            // filtering. Unlike in the OpenGL world, there is no
            // glBlitFramebuffer equivalent available, because APIs like D3D
            // and Metal have no such operation (the generally supported
            // texture copy operations are 1:1 copies, without support for
            // scaling, which is what we would need here). So draw a quad.

            QRhiCommandBuffer *cb = rhiCtx->commandBuffer();
            const auto &renderer = m_sgContext->renderer();

            cb->debugMarkBegin(QByteArrayLiteral("SSAA downsample"));
            renderer->rhiQuadRenderer()->prepareQuad(rhiCtx, nullptr);

            // Instead of passing in a flip flag we choose to rely on qsb's
            // per-target compilation mode in the fragment shader. (it does UV
            // flipping based on QSHADER_ macros) This is just better for
            // performance and the shaders are very simple so introducing a
            // uniform block and branching dynamically would be an overkill.
            QSSGRef<QSSGRhiShaderPipeline> shaderPipeline = renderer->getRhiSupersampleResolveShader();

            QRhiSampler *sampler = rhiCtx->sampler({ QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                                     QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge, QRhiSampler::Repeat });
            QSSGRhiShaderResourceBindingList bindings;
            bindings.addTexture(0, QRhiShaderResourceBinding::FragmentStage, currentTexture, sampler);
            QRhiShaderResourceBindings *srb = rhiCtx->srb(bindings);

            QSSGRhiGraphicsPipelineState ps;
            ps.viewport = QRhiViewport(0, 0, float(m_surfaceSize.width()), float(m_surfaceSize.height()));
            ps.shaderPipeline = shaderPipeline.data();

            renderer->rhiQuadRenderer()->recordRenderQuadPass(rhiCtx, &ps, srb, m_ssaaTextureToTextureRenderTarget, QSSGRhiQuadRenderer::UvCoords);
            cb->debugMarkEnd();
            currentTexture = m_texture;
        }
        endFrame();

        Q_QUICK3D_PROFILE_END_WITH_PAYLOAD(QQuick3DProfiler::Quick3DRenderFrame,
                                           STAT_PAYLOAD(m_sgContext->rhiContext()->stats()));
    }

    return currentTexture;
}

void QQuick3DSceneRenderer::beginFrame()
{
    m_sgContext->beginFrame(m_layer);
}

void QQuick3DSceneRenderer::endFrame()
{
    m_sgContext->endFrame(m_layer);
}

void QQuick3DSceneRenderer::rhiPrepare(const QRect &viewport, qreal displayPixelRatio)
{
    if (!m_layer)
        return;

    m_sgContext->setDpr(displayPixelRatio);

    m_sgContext->setViewport(viewport);
    m_sgContext->setScissorRect(viewport);

    m_sgContext->setSceneColor(QColor(Qt::black));

    m_sgContext->prepareLayerForRender(*m_layer);
    m_sgContext->rhiPrepare(*m_layer);

    m_prepared = true;
}

void QQuick3DSceneRenderer::rhiRender()
{
    Q_ASSERT(m_prepared);
    m_prepared = false;

    // There is no clearFirst flag - the rendering here does not record a
    // beginPass() so it never clears on its own.

    m_sgContext->rhiRender(*m_layer);
}

static QRhiTexture::Format toRhiTextureFormat(QQuickShaderEffectSource::Format format)
{
    switch (format) {
    case QQuickShaderEffectSource::RGBA8:
        return QRhiTexture::RGBA8;
    case QQuickShaderEffectSource::RGBA16F:
        return QRhiTexture::RGBA16F;
    case QQuickShaderEffectSource::RGBA32F:
        return QRhiTexture::RGBA32F;
    default:
        return QRhiTexture::RGBA8;
    }
}

void QQuick3DSceneRenderer::synchronize(QQuick3DViewport *view3D, const QSize &size, float dpr)
{
    Q_ASSERT(view3D != nullptr); // This is not an option!
    if (!m_renderStats)
        m_renderStats = view3D->renderStats();

    if (m_renderStats)
        m_renderStats->startSync();

    Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DSynchronizeFrame);

    m_sgContext->setDpr(dpr);
    bool layerSizeIsDirty = m_surfaceSize != size;
    m_surfaceSize = size;

    QList<QSSGRenderGraphObject *> resourceLoaders;

    QQuick3DSceneManager *importSceneManager = nullptr;
    QQuick3DNode *importScene = view3D->importScene();
    if (importScene) {
        importSceneManager = QQuick3DObjectPrivate::get(importScene)->sceneManager;
        importSceneManager->rci = m_sgContext.data();
    }

    if (auto sceneManager = QQuick3DObjectPrivate::get(view3D->scene())->sceneManager) {
        // Cleanup
        sceneManager->rci = m_sgContext.data();
        sceneManager->cleanupNodes();
        if (importSceneManager)
            importSceneManager->cleanupNodes();
        // Resources
        if (importSceneManager)
            importSceneManager->updateDirtyResourceNodes();
        sceneManager->updateDirtyResourceNodes();
        // Spatial Nodes
        if (importSceneManager)
            importSceneManager->updateDirtySpatialNodes();
        sceneManager->updateDirtySpatialNodes();
        // Bounding Boxes
        if (importSceneManager)
            importSceneManager->updateBoundingBoxes(m_sgContext->bufferManager());
        sceneManager->updateBoundingBoxes(m_sgContext->bufferManager());
        // Resource Loaders
        resourceLoaders.append(sceneManager->resourceLoaders.values());
        if (importSceneManager)
            resourceLoaders.append(importSceneManager->resourceLoaders.values());
    }

    // Generate layer node
    if (!m_layer) {
        m_layer = new QSSGRenderLayer();
        m_layer->setState(QSSGRenderNode::LocalState::Active);
    }

    // Update the layer node properties
    updateLayerNode(view3D, resourceLoaders);

    bool postProcessingNeeded = m_layer->firstEffect;
    bool postProcessingWasActive = m_effectSystem;
    QSSGRenderTextureFormat::Format effectOutputFormatOverride = QSSGRenderTextureFormat::Unknown;
    if (postProcessingNeeded) {
        QSSGRenderEffect *lastEffect = m_layer->firstEffect;
        while (lastEffect->m_nextEffect)
            lastEffect = lastEffect->m_nextEffect;
        effectOutputFormatOverride = QSSGRhiEffectSystem::overriddenOutputFormat(lastEffect);
    }
    const auto layerTextureFormat = [effectOutputFormatOverride, view3D](QRhi *rhi, bool postProc) {
        if (effectOutputFormatOverride != QSSGRenderTextureFormat::Unknown)
            return QSSGBufferManager::toRhiFormat(effectOutputFormatOverride);

        // Our standard choice for the postprocessing input/output textures'
        // format is a floating point one. (unlike intermediate Buffers, which
        // default to RGBA8 unless the format is explicitly specified)
        // This is intentional since a float format allows passing in
        // non-tonemapped content without colors being clamped when written out
        // to the render target.
        //
        // When it comes to the output, this applies to that too due to
        // QSSGRhiEffectSystem picking it up unless overridden (with a Buffer
        // an empty 'name'). Here too a float format gives more flexibility:
        // the effect may or may not do its own tonemapping and this approach
        // is compatible with potential future on-screen HDR output support.

        const QRhiTexture::Format preferredPostProcFormat = QRhiTexture::RGBA16F;
        if (postProc && rhi->isTextureFormatSupported(preferredPostProcFormat))
            return preferredPostProcFormat;

        const QRhiTexture::Format preferredView3DFormat = toRhiTextureFormat(view3D->renderFormat());
        if (rhi->isTextureFormatSupported(preferredView3DFormat))
            return preferredView3DFormat;

        return QRhiTexture::RGBA8;
    };
    bool postProcessingStateDirty = postProcessingNeeded != postProcessingWasActive;

    // Store from the layer properties the ones we need to handle ourselves (with the RHI code path)
    m_backgroundMode = QSSGRenderLayer::Background(view3D->environment()->backgroundMode());
    m_backgroundColor = view3D->environment()->clearColor();

    // Set the root item for the scene to the layer
    auto rootNode = static_cast<QSSGRenderNode*>(QQuick3DObjectPrivate::get(view3D->scene())->spatialNode);
    if (rootNode != m_sceneRootNode) {
        if (m_sceneRootNode)
            removeNodeFromLayer(m_sceneRootNode);

        if (rootNode)
            addNodeToLayer(rootNode);

        m_sceneRootNode = rootNode;
    }

    // Add the referenced scene root node to the layer as well if available
    QSSGRenderNode* importRootNode = nullptr;
    if (importScene) {
        importRootNode = static_cast<QSSGRenderNode*>(
                                QQuick3DObjectPrivate::get(importScene)->spatialNode);
    }
    if (importRootNode != m_importRootNode) {
        if (m_importRootNode)
            m_layer->removeImportScene(*m_importRootNode);

        if (importRootNode) {
            // if importScene has the rendered viewport as ancestor, it probably means
            // "importScene: MyScene { }" type of inclusion.
            // In this case don't duplicate content by adding it again.
            QObject *sceneParent = importScene->parent();
            bool isEmbedded = false;
            while (sceneParent) {
                if (sceneParent == view3D) {
                    isEmbedded = true;
                    break;
                }
                sceneParent = sceneParent->parent();
            }
            if (!isEmbedded)
                m_layer->setImportScene(*importRootNode);
        }

        m_importRootNode = importRootNode;
    }

    const bool progressiveAA = m_layer->antialiasingMode == QSSGRenderLayer::AAMode::ProgressiveAA;
    const bool multiSamplingAA = m_layer->antialiasingMode == QSSGRenderLayer::AAMode::MSAA;
    const bool temporalAA = m_layer->temporalAAEnabled && !multiSamplingAA;
    const bool superSamplingAA = m_layer->antialiasingMode == QSSGRenderLayer::AAMode::SSAA;
    const bool timeBasedAA = progressiveAA || temporalAA;
    m_postProcessingStack = m_layer->firstEffect || timeBasedAA  || superSamplingAA;
    bool useFBO = view3D->renderMode() == QQuick3DViewport::RenderMode::Offscreen ||
                                          ((view3D->renderMode() == QQuick3DViewport::RenderMode::Underlay || view3D->renderMode() == QQuick3DViewport::RenderMode::Overlay)
                                           && m_postProcessingStack);
    if (useFBO) {
        QSSGRhiContext *rhiCtx = m_sgContext->rhiContext().data();
        if (rhiCtx->isValid()) {
            QRhi *rhi = rhiCtx->rhi();
            const QSize renderSize = superSamplingAA ? m_surfaceSize * m_ssaaMultiplier : m_surfaceSize;

            if (m_texture) {
                // the size changed, or the AA settings changed, or toggled between some effects - no effect
                if (layerSizeIsDirty || postProcessingStateDirty) {
                    m_texture->setPixelSize(m_surfaceSize);
                    m_texture->setFormat(layerTextureFormat(rhi, postProcessingNeeded));
                    m_texture->create();

                    // If AA settings changed, then we drop and recreate all
                    // resources, otherwise use a lighter path if just the size
                    // changed.
                    if (!m_aaIsDirty) {
                        // A special case: when toggling effects and AA is on,
                        // use the heavier AA path because the renderbuffer for
                        // MSAA and texture for SSAA may need a different
                        // format now since m_texture's format could have
                        // changed between RBGA8 and RGBA16F (due to layerTextureFormat()).
                        if (postProcessingStateDirty && (m_layer->antialiasingMode != QSSGRenderLayer::AAMode::NoAA || temporalAA)) {
                            releaseAaDependentRhiResources();
                        } else {
                            if (m_ssaaTexture) {
                                m_ssaaTexture->setPixelSize(renderSize);
                                m_ssaaTexture->create();
                            }
                            m_depthStencilBuffer->setPixelSize(renderSize);
                            m_depthStencilBuffer->create();
                            if (m_msaaRenderBuffer) {
                                m_msaaRenderBuffer->setPixelSize(renderSize);
                                m_msaaRenderBuffer->create();
                            }
                            // Toggling effects on and off will change the format
                            // (assuming effects default to a floating point
                            // format) and that needs on a different renderpass on
                            // Vulkan. Hence renewing m_textureRenderPassDescriptor as well.
                            if (postProcessingStateDirty) {
                                delete m_textureRenderPassDescriptor;
                                m_textureRenderPassDescriptor = m_textureRenderTarget->newCompatibleRenderPassDescriptor();
                                m_textureRenderTarget->setRenderPassDescriptor(m_textureRenderPassDescriptor);
                            }
                            m_textureRenderTarget->create();
                            if (m_ssaaTextureToTextureRenderTarget)
                                m_ssaaTextureToTextureRenderTarget->create();

                            if (m_temporalAATexture) {
                                m_temporalAATexture->setPixelSize(renderSize);
                                m_temporalAATexture->create();
                            }
                            if (m_prevTempAATexture) {
                                m_prevTempAATexture->setPixelSize(renderSize);
                                m_prevTempAATexture->create();
                            }
                            if (m_temporalAARenderTarget)
                                m_temporalAARenderTarget->create();
                        }
                    }
                } else if (m_aaIsDirty && rhi->backend() == QRhi::Metal) { // ### to avoid garbage upon enabling MSAA with macOS 10.14 (why is this needed?)
                    m_texture->create();
                }

                if (m_aaIsDirty)
                    releaseAaDependentRhiResources();
            }

            const QRhiTexture::Flags textureFlags = QRhiTexture::RenderTarget
                    | QRhiTexture::UsedAsTransferSource; // transfer source is for progressive/temporal AA
            const QRhiTexture::Format textureFormat = layerTextureFormat(rhi, postProcessingNeeded);

            if (!m_texture) {
                m_texture = rhi->newTexture(textureFormat, m_surfaceSize, 1, textureFlags);
                m_texture->create();
            }

            if (!m_ssaaTexture && superSamplingAA) {
                m_ssaaTexture = rhi->newTexture(textureFormat, renderSize, 1, textureFlags);
                m_ssaaTexture->create();
            }

            if (timeBasedAA && !m_temporalAATexture) {
                m_temporalAATexture = rhi->newTexture(textureFormat, renderSize, 1, textureFlags);
                m_temporalAATexture->create();
                m_prevTempAATexture = rhi->newTexture(textureFormat, renderSize, 1, textureFlags);
                m_prevTempAATexture->create();
            }

            // we need to re-render time-based AA not only when AA state changes, but also when resized
            if (m_aaIsDirty || layerSizeIsDirty)
                m_layer->tempAAPassIndex = m_layer->progAAPassIndex = 0;

            if (m_aaIsDirty) {
                m_samples = 1;
                if (m_layer->antialiasingMode == QSSGRenderLayer::AAMode::MSAA) {
                    if (rhi->isFeatureSupported(QRhi::MultisampleRenderBuffer)) {
                        m_samples = qMax(1, int(m_layer->antialiasingQuality));
                        // The Quick3D API exposes high level values such as
                        // Medium, High, VeryHigh instead of direct sample
                        // count values. Therefore, be nice and find a sample
                        // count that's actually supported in case the one
                        // associated by default is not.
                        const QVector<int> supported = rhi->supportedSampleCounts(); // assumed to be sorted
                        if (!supported.contains(m_samples)) {
                            if (!supported.isEmpty()) {
                                auto it = std::lower_bound(supported.cbegin(), supported.cend(), m_samples);
                                m_samples = it == supported.cend() ? supported.last() : *it;
                            } else {
                                m_samples = 1;
                            }
                        }
                    } else {
                        static bool warned = false;
                        if (!warned) {
                            warned = true;
                            qWarning("Multisample renderbuffers are not supported, disabling MSAA for Offscreen View3D");
                        }
                    }
                }
            }

            if (!m_depthStencilBuffer) {
                m_depthStencilBuffer = rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, renderSize, m_samples);
                m_depthStencilBuffer->create();
            }

            if (!m_textureRenderTarget) {
                QRhiTextureRenderTargetDescription rtDesc;
                if (m_samples > 1) {
                    // pass in the texture's format (which may be a floating point one!) as the preferred format hint
                    m_msaaRenderBuffer = rhi->newRenderBuffer(QRhiRenderBuffer::Color, renderSize, m_samples, {}, m_texture->format());
                    m_msaaRenderBuffer->create();
                    QRhiColorAttachment att;
                    att.setRenderBuffer(m_msaaRenderBuffer);
                    att.setResolveTexture(m_texture);
                    rtDesc.setColorAttachments({ att });
                } else {
                    if (m_layer->antialiasingMode == QSSGRenderLayer::AAMode::SSAA)
                        rtDesc.setColorAttachments({ m_ssaaTexture });
                    else
                        rtDesc.setColorAttachments({ m_texture });
                }
                rtDesc.setDepthStencilBuffer(m_depthStencilBuffer);

                m_textureRenderTarget = rhi->newTextureRenderTarget(rtDesc);
                m_textureRenderPassDescriptor = m_textureRenderTarget->newCompatibleRenderPassDescriptor();
                m_textureRenderTarget->setRenderPassDescriptor(m_textureRenderPassDescriptor);
                m_textureRenderTarget->create();
            }

            if (!m_ssaaTextureToTextureRenderTarget && m_layer->antialiasingMode == QSSGRenderLayer::AAMode::SSAA) {
                m_ssaaTextureToTextureRenderTarget = rhi->newTextureRenderTarget({ m_texture });
                m_ssaaTextureToTextureRenderPassDescriptor = m_ssaaTextureToTextureRenderTarget->newCompatibleRenderPassDescriptor();
                m_ssaaTextureToTextureRenderTarget->setRenderPassDescriptor(m_ssaaTextureToTextureRenderPassDescriptor);
                m_ssaaTextureToTextureRenderTarget->create();
            }

            if (m_layer->firstEffect) {
                if (!m_effectSystem)
                    m_effectSystem = new QSSGRhiEffectSystem(m_sgContext);
                m_effectSystem->setup(renderSize);
            } else if (m_effectSystem) {
                delete m_effectSystem;
                m_effectSystem = nullptr;
            }

            if (timeBasedAA && !m_temporalAARenderTarget) {
                m_temporalAARenderTarget = rhi->newTextureRenderTarget({ m_temporalAATexture });
                m_temporalAARenderPassDescriptor = m_temporalAARenderTarget->newCompatibleRenderPassDescriptor();
                m_temporalAARenderTarget->setRenderPassDescriptor(m_temporalAARenderPassDescriptor);
                m_temporalAARenderTarget->create();
            }

            m_textureNeedsFlip = rhi->isYUpInFramebuffer();
            m_aaIsDirty = false;
        }
    }

    if (m_renderStats)
        m_renderStats->endSync(dumpRenderTimes);

    Q_QUICK3D_PROFILE_END(QQuick3DProfiler::Quick3DSynchronizeFrame);
}

void QQuick3DSceneRenderer::invalidateFramebufferObject()
{
    if (fboNode)
        fboNode->invalidatePending = true;
}

QSSGOption<QSSGRenderRay> QQuick3DSceneRenderer::getRayFromViewportPos(const QPointF &pos)
{
    if (!m_layer || !m_layer->renderedCamera)
        return QSSGEmpty();

    const QVector2D viewportSize(m_surfaceSize.width(), m_surfaceSize.height());
    const QVector2D position(float(pos.x()), float(pos.y()));
    const QRectF viewportRect(QPointF{}, QSizeF(m_surfaceSize));

    // First invert the y so we are dealing with numbers in a normal coordinate space.
    // Second, move into our layer's coordinate space
    QVector2D correctCoords(position.x(), viewportSize.y() - position.y());
    QVector2D theLocalMouse = toRectRelative(viewportRect, correctCoords);
    if ((theLocalMouse.x() < 0.0f || theLocalMouse.x() >= viewportSize.x() || theLocalMouse.y() < 0.0f
         || theLocalMouse.y() >= viewportSize.y()))
        return QSSGEmpty();

    return m_layer->renderedCamera->unproject(theLocalMouse, viewportRect);
}

QSSGRenderPickResult QQuick3DSceneRenderer::syncPick(const QSSGRenderRay &ray)
{
    if (!m_layer)
        return QSSGRenderPickResult();

    return m_sgContext->renderer()->syncPick(*m_layer,
                                             m_sgContext->bufferManager(),
                                             ray);
}

QSSGRenderPickResult QQuick3DSceneRenderer::syncPickOne(const QSSGRenderRay &ray, QSSGRenderNode *node)
{
    if (!m_layer)
        return QSSGRenderPickResult();

    return m_sgContext->renderer()->syncPick(*m_layer,
                                             m_sgContext->bufferManager(),
                                             ray,
                                             node);
}

QQuick3DSceneRenderer::PickResultList QQuick3DSceneRenderer::syncPickAll(const QSSGRenderRay &ray)
{
    if (!m_layer)
        return QQuick3DSceneRenderer::PickResultList();

    return m_sgContext->renderer()->syncPickAll(*m_layer,
                                                m_sgContext->bufferManager(),
                                                ray);
}

void QQuick3DSceneRenderer::setGlobalPickingEnabled(bool isEnabled)
{
    m_sgContext->renderer()->setGlobalPickingEnabled(isEnabled);
}

QQuick3DRenderStats *QQuick3DSceneRenderer::renderStats()
{
    return m_renderStats;
}

void QQuick3DRenderLayerHelpers::updateLayerNodeHelper(const QQuick3DViewport &view3D, QSSGRenderLayer &layerNode, bool &aaIsDirty, bool &temporalIsDirty, float &ssaaMultiplier)
{
    QQuick3DSceneEnvironment *environment = view3D.environment();

    QSSGRenderLayer::AAMode aaMode = QSSGRenderLayer::AAMode(environment->antialiasingMode());
    if (aaMode != layerNode.antialiasingMode) {
        layerNode.antialiasingMode = aaMode;
        layerNode.progAAPassIndex = 0;
        aaIsDirty = true;
    }
    QSSGRenderLayer::AAQuality aaQuality = QSSGRenderLayer::AAQuality(environment->antialiasingQuality());
    if (aaQuality != layerNode.antialiasingQuality) {
        layerNode.antialiasingQuality = aaQuality;
        ssaaMultiplier = (aaQuality == QSSGRenderLayer::AAQuality::Normal) ? 1.2f :
                                                                               (aaQuality == QSSGRenderLayer::AAQuality::High) ? 1.5f :
                                                                                                                                 2.0f;
        layerNode.ssaaMultiplier = ssaaMultiplier;
        aaIsDirty = true;
    }

    bool temporalAAEnabled = environment->temporalAAEnabled();
    if (temporalAAEnabled != layerNode.temporalAAEnabled) {
        layerNode.temporalAAEnabled = environment->temporalAAEnabled();
        temporalIsDirty = true;

        layerNode.tempAAPassIndex = 0;
        aaIsDirty = true;
    }
    layerNode.ssaaEnabled = environment->antialiasingMode()
            == QQuick3DSceneEnvironment::QQuick3DEnvironmentAAModeValues::SSAA;

    layerNode.temporalAAStrength = environment->temporalAAStrength();

    layerNode.specularAAEnabled = environment->specularAAEnabled();

    layerNode.background = QSSGRenderLayer::Background(environment->backgroundMode());
    layerNode.clearColor = QVector3D(float(environment->clearColor().redF()),
                                      float(environment->clearColor().greenF()),
                                      float(environment->clearColor().blueF()));

    layerNode.m_width = 100.f;
    layerNode.m_height = 100.f;
    layerNode.widthUnits = QSSGRenderLayer::UnitType::Percent;
    layerNode.heightUnits = QSSGRenderLayer::UnitType::Percent;

    layerNode.aoStrength = environment->aoStrength();
    layerNode.aoDistance = environment->aoDistance();
    layerNode.aoSoftness = environment->aoSoftness();
    layerNode.aoBias = environment->aoBias();
    layerNode.aoSamplerate = environment->aoSampleRate();
    layerNode.aoDither = environment->aoDither();

    // ### These images will not be registered anywhere
    if (environment->lightProbe())
        layerNode.lightProbe = environment->lightProbe()->getRenderImage();
    else
        layerNode.lightProbe = nullptr;
    if (view3D.environment()->skyBoxCubeMap())
        layerNode.skyBoxCubeMap = view3D.environment()->skyBoxCubeMap()->getRenderImage();
    else
        layerNode.skyBoxCubeMap = nullptr;

    layerNode.probeExposure = environment->probeExposure();
    // Remap the probeHorizon to the expected Range
    layerNode.probeHorizon = qMin(environment->probeHorizon() - 1.0f, -0.001f);
    layerNode.setProbeOrientation(environment->probeOrientation());

    if (view3D.camera())
        layerNode.explicitCamera = static_cast<QSSGRenderCamera *>(QQuick3DObjectPrivate::get(view3D.camera())->spatialNode);

    layerNode.layerFlags.setFlag(QSSGRenderLayer::LayerFlag::EnableDepthTest, environment->depthTestEnabled());
    layerNode.layerFlags.setFlag(QSSGRenderLayer::LayerFlag::EnableDepthPrePass, environment->depthPrePassEnabled());

    layerNode.tonemapMode = QSSGRenderLayer::TonemapMode(environment->tonemapMode());
    layerNode.skyboxBlurAmount = environment->skyboxBlurAmount();

    if (environment->lightmapper()) {
        QQuick3DLightmapper *lightmapper = environment->lightmapper();
        layerNode.lmOptions.opacityThreshold = lightmapper->opacityThreshold();
        layerNode.lmOptions.bias = lightmapper->bias();
        layerNode.lmOptions.useAdaptiveBias = lightmapper->isAdaptiveBiasEnabled();
        layerNode.lmOptions.indirectLightEnabled = lightmapper->isIndirectLightEnabled();
        layerNode.lmOptions.indirectLightSamples = lightmapper->samples();
        layerNode.lmOptions.indirectLightWorkgroupSize = lightmapper->indirectLightWorkgroupSize();
        layerNode.lmOptions.indirectLightBounces = lightmapper->bounces();
        layerNode.lmOptions.indirectLightFactor = lightmapper->indirectLightFactor();
    } else {
        layerNode.lmOptions = {};
    }
}

void QQuick3DSceneRenderer::updateLayerNode(QQuick3DViewport *view3D, const QList<QSSGRenderGraphObject *> &resourceLoaders)
{
    QSSGRenderLayer *layerNode = m_layer;

    bool temporalIsDirty = false;
    QQuick3DRenderLayerHelpers::updateLayerNodeHelper(*view3D, *m_layer, m_aaIsDirty, temporalIsDirty, m_ssaaMultiplier);

    int extraFramesToRender = 0;

    if (layerNode->antialiasingMode == QSSGRenderLayer::AAMode::ProgressiveAA) {
        // with progressive AA, we need a number of extra frames after the last dirty one
        // if we always reset requestedFramesCount when dirty, we will get the extra frames eventually
        extraFramesToRender = int(layerNode->antialiasingQuality);

        // RHI: +1 since we need a normal frame to start with, and we're not copying that from the screen
        if (m_sgContext->rhiContext()->isValid())
            extraFramesToRender += 1;
    } else if (layerNode->temporalAAEnabled) {
        // When temporalAA is on and antialiasing mode changes,
        // layer needs to be re-rendered (at least) MAX_TEMPORAL_AA_LEVELS times
        // to generate temporal antialiasing.
        // Also, we need to do an extra render when animation stops
        extraFramesToRender = (m_aaIsDirty || temporalIsDirty) ? QSSGLayerRenderData::MAX_TEMPORAL_AA_LEVELS : 1;
    }

    requestedFramesCount = extraFramesToRender;
    // Effects need to be rendered in reverse order as described in the file.
    layerNode->firstEffect = nullptr; // We reset the linked list
    const auto &effects = view3D->environment()->m_effects;
    auto rit = effects.crbegin();
    const auto rend = effects.crend();
    for (; rit != rend; ++rit) {
        QQuick3DObjectPrivate *p = QQuick3DObjectPrivate::get(*rit);
        QSSGRenderEffect *effectNode = static_cast<QSSGRenderEffect *>(p->spatialNode);
        if (effectNode) {
            if (layerNode->hasEffect(effectNode)) {
                qWarning() << "Duplicate effect found, skipping!";
            } else {
                effectNode->className = (*rit)->metaObject()->className(); //### persistent, but still icky to store a const char* returned from a function
                layerNode->addEffect(*effectNode);
            }
        }
    }

    // ResourceLoaders
    layerNode->resourceLoaders.clear();
    layerNode->resourceLoaders = resourceLoaders;
}

void QQuick3DSceneRenderer::removeNodeFromLayer(QSSGRenderNode *node)
{
    if (!m_layer)
        return;

    m_layer->removeChild(*node);
}

void QQuick3DSceneRenderer::addNodeToLayer(QSSGRenderNode *node)
{
    if (!m_layer)
        return;

    m_layer->addChild(*node);
}

QSGRenderNode::StateFlags QQuick3DSGRenderNode::changedStates() const
{
    return BlendState | StencilState | DepthState | ScissorState | ColorState | CullState | ViewportState | RenderTargetState;
}

namespace {
inline QRect convertQtRectToGLViewport(const QRectF &rect, const QSize surfaceSize)
{
    const int x = int(rect.x());
    const int y = surfaceSize.height() - (int(rect.y()) + int(rect.height()));
    const int width = int(rect.width());
    const int height = int(rect.height());
    return QRect(x, y, width, height);
}

inline void queryMainRenderPassDescriptorAndCommandBuffer(QQuickWindow *window, QSSGRhiContext *rhiCtx)
{
    if (rhiCtx->isValid()) {
        // Query from the rif because that is available in the sync
        // phase (updatePaintNode) already.  QSGDefaultRenderContext's
        // copies of the rp and cb are not there until the render
        // phase of the scenegraph.
        int sampleCount = 1;
        QSGRendererInterface *rif = window->rendererInterface();
        QRhiSwapChain *swapchain = static_cast<QRhiSwapChain *>(
            rif->getResource(window, QSGRendererInterface::RhiSwapchainResource));
        if (swapchain) {
            rhiCtx->setMainRenderPassDescriptor(swapchain->renderPassDescriptor());
            rhiCtx->setCommandBuffer(swapchain->currentFrameCommandBuffer());
            rhiCtx->setRenderTarget(swapchain->currentFrameRenderTarget());
            sampleCount = swapchain->sampleCount();
        } else {
            // no swapchain when using a QQuickRenderControl (redirecting to a texture etc.)
            QRhiCommandBuffer *cb = static_cast<QRhiCommandBuffer *>(
                rif->getResource(window, QSGRendererInterface::RhiRedirectCommandBuffer));
            QRhiTextureRenderTarget *rt = static_cast<QRhiTextureRenderTarget *>(
                rif->getResource(window, QSGRendererInterface::RhiRedirectRenderTarget));
            if (cb && rt) {
                rhiCtx->setMainRenderPassDescriptor(rt->renderPassDescriptor());
                rhiCtx->setCommandBuffer(cb);
                rhiCtx->setRenderTarget(rt);
                const QRhiColorAttachment *color0 = rt->description().cbeginColorAttachments();
                if (color0 && color0->texture())
                    sampleCount = color0->texture()->sampleCount();
            } else {
                qWarning("Neither swapchain nor redirected command buffer and render target are available.");
            }
        }

        // MSAA is out of our control on this path: it is up to the
        // QQuickWindow and the scenegraph to set up the swapchain based on the
        // QSurfaceFormat's samples(). The only thing we need to do here is to
        // pass the sample count to the renderer because it is needed when
        // creating graphics pipelines.
        rhiCtx->setMainPassSampleCount(sampleCount);
    }
}

// The alternative to queryMainRenderPassDescriptorAndCommandBuffer()
// specifically for the Inline render mode when there is a QSGRenderNode.
inline void queryInlineRenderPassDescriptorAndCommandBuffer(QSGRenderNode *node, QSSGRhiContext *rhiCtx)
{
    QSGRenderNodePrivate *d = QSGRenderNodePrivate::get(node);
    rhiCtx->setMainRenderPassDescriptor(d->m_rt.rpDesc);
    rhiCtx->setCommandBuffer(d->m_rt.cb);
    rhiCtx->setRenderTarget(d->m_rt.rt);
    rhiCtx->setMainPassSampleCount(d->m_rt.rt->sampleCount());
}
}

QQuick3DSGRenderNode::~QQuick3DSGRenderNode()
{
    delete renderer;
}

void QQuick3DSGRenderNode::prepare()
{
    // this is outside the main renderpass

    if (!renderer->m_sgContext->rhiContext()->isValid())
        return;
    Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DPrepareFrame);

    queryInlineRenderPassDescriptorAndCommandBuffer(this, renderer->m_sgContext->rhiContext().data());

    qreal dpr = window->devicePixelRatio();
    const QSizeF itemSize = renderer->surfaceSize() / dpr;
    QRectF viewport = matrix()->mapRect(QRectF(QPoint(0, 0), itemSize));
    viewport = QRectF(viewport.topLeft() * dpr, viewport.size() * dpr);
    const QRect vp = convertQtRectToGLViewport(viewport, window->size() * dpr);
    renderer->beginFrame();
    renderer->rhiPrepare(vp, dpr);
    Q_QUICK3D_PROFILE_END(QQuick3DProfiler::Quick3DPrepareFrame);
}

void QQuick3DSGRenderNode::render(const QSGRenderNode::RenderState *state)
{
    Q_UNUSED(state);

    if (renderer->m_sgContext->rhiContext()->isValid()) {
        Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderFrame);

        queryInlineRenderPassDescriptorAndCommandBuffer(this, renderer->m_sgContext->rhiContext().data());

        renderer->rhiRender();
        renderer->endFrame();
        Q_QUICK3D_PROFILE_END_WITH_PAYLOAD(QQuick3DProfiler::Quick3DRenderFrame,
                                           STAT_PAYLOAD(renderer->m_sgContext->rhiContext()->stats()));
    }
}

void QQuick3DSGRenderNode::releaseResources()
{
}

QSGRenderNode::RenderingFlags QQuick3DSGRenderNode::flags() const
{
    // don't want begin/endExternal() to be called by Quick
    return NoExternalRendering;
}

QQuick3DSGDirectRenderer::QQuick3DSGDirectRenderer(QQuick3DSceneRenderer *renderer, QQuickWindow *window, QQuick3DSGDirectRenderer::QQuick3DSGDirectRendererMode mode)
    : m_renderer(renderer)
    , m_window(window)
    , m_mode(mode)
{
    if (QSGRendererInterface::isApiRhiBased(window->rendererInterface()->graphicsApi())) {
        connect(window, &QQuickWindow::beforeRendering, this, &QQuick3DSGDirectRenderer::prepare, Qt::DirectConnection);
        if (mode == Underlay)
            connect(window, &QQuickWindow::beforeRenderPassRecording, this, &QQuick3DSGDirectRenderer::render, Qt::DirectConnection);
        else
            connect(window, &QQuickWindow::afterRenderPassRecording, this, &QQuick3DSGDirectRenderer::render, Qt::DirectConnection);
    }
}

QQuick3DSGDirectRenderer::~QQuick3DSGDirectRenderer()
{
    delete m_renderer;
}

void QQuick3DSGDirectRenderer::setViewport(const QRectF &viewport)
{
    m_viewport = viewport;
}

void QQuick3DSGDirectRenderer::setVisibility(bool visible)
{
    if (m_isVisible == visible)
        return;
    m_isVisible = visible;
    m_window->update();
}

void QQuick3DSGDirectRenderer::requestRender()
{
    renderPending = true;
    requestFullUpdate(m_window);
}

void QQuick3DSGDirectRenderer::prepare()
{
    if (!m_isVisible)
        return;

    if (m_renderer->m_sgContext->rhiContext()->isValid()) {
        // this is outside the main renderpass
        if (m_renderer->m_postProcessingStack) {
            if (renderPending) {
                renderPending = false;
                m_rhiTexture = m_renderer->renderToRhiTexture(m_window);
                queryMainRenderPassDescriptorAndCommandBuffer(m_window, m_renderer->m_sgContext->rhiContext().data());
                auto quadRenderer = m_renderer->m_sgContext->renderer()->rhiQuadRenderer();
                quadRenderer->prepareQuad(m_renderer->m_sgContext->rhiContext().data(), nullptr);
                if (m_renderer->m_sgContext->renderer()->rendererRequestsFrames()
                        || m_renderer->requestedFramesCount > 0) {
                    requestRender();
                    if (m_renderer->requestedFramesCount > 0)
                        m_renderer->requestedFramesCount--;
                }
            }
        }
        else
        {
            QQuick3DRenderStats *renderStats = m_renderer->renderStats();
            if (renderStats) {
                renderStats->startRender();
                renderStats->startRenderPrepare();
            }

            Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DPrepareFrame);
            queryMainRenderPassDescriptorAndCommandBuffer(m_window, m_renderer->m_sgContext->rhiContext().data());
            const QRect vp = convertQtRectToGLViewport(m_viewport, m_window->size() * m_window->devicePixelRatio());
            m_renderer->beginFrame();
            m_renderer->rhiPrepare(vp, m_window->devicePixelRatio());
            Q_QUICK3D_PROFILE_END(QQuick3DProfiler::Quick3DPrepareFrame);

            if (renderStats)
                renderStats->endRenderPrepare();
        }
    }
}

void QQuick3DSGDirectRenderer::render()
{
    if (!m_isVisible)
        return;

    if (m_renderer->m_sgContext->rhiContext()->isValid()) {
        // the command buffer is recording the main renderpass at this point

        // No m_window->beginExternalCommands() must be done here. When the
        // renderer is using the same
        // QRhi/QRhiCommandBuffer/QRhiRenderPassDescriptor as the Qt Quick
        // scenegraph, there is no difference from the RHI's perspective. There are
        // no external (native) commands here.

        // Requery the command buffer and co. since Offscreen mode View3Ds may
        // have altered these on the context.
        if (m_renderer->m_postProcessingStack) {
            if (m_rhiTexture) {
                queryMainRenderPassDescriptorAndCommandBuffer(m_window, m_renderer->m_sgContext->rhiContext().data());
                auto rhiCtx = m_renderer->m_sgContext->rhiContext().data();
                const auto &renderer = m_renderer->m_sgContext->renderer();

                // Instead of passing in a flip flag we choose to rely on qsb's
                // per-target compilation mode in the fragment shader. (it does UV
                // flipping based on QSHADER_ macros) This is just better for
                // performance and the shaders are very simple so introducing a
                // uniform block and branching dynamically would be an overkill.
                QRect vp = convertQtRectToGLViewport(m_viewport, m_window->size() * m_window->devicePixelRatio());

                QSSGRef<QSSGRhiShaderPipeline> shaderPipeline = renderer->getRhiSimpleQuadShader();

                QRhiSampler *sampler = rhiCtx->sampler({ QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                                         QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge });
                QSSGRhiShaderResourceBindingList bindings;
                bindings.addTexture(0, QRhiShaderResourceBinding::FragmentStage, m_rhiTexture, sampler);
                QRhiShaderResourceBindings *srb = rhiCtx->srb(bindings);

                QSSGRhiGraphicsPipelineState ps;
                ps.viewport = QRhiViewport(float(vp.x()), float(vp.y()), float(vp.width()), float(vp.height()));
                ps.shaderPipeline = shaderPipeline.data();
                renderer->rhiQuadRenderer()->recordRenderQuad(rhiCtx, &ps, srb, rhiCtx->mainRenderPassDescriptor(), QSSGRhiQuadRenderer::UvCoords | QSSGRhiQuadRenderer::PremulBlend);
            }
        }
        else
        {
            Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderFrame);

            queryMainRenderPassDescriptorAndCommandBuffer(m_window, m_renderer->m_sgContext->rhiContext().data());

            m_renderer->rhiRender();
            m_renderer->endFrame();

            Q_QUICK3D_PROFILE_END_WITH_PAYLOAD(QQuick3DProfiler::Quick3DRenderFrame,
                                               STAT_PAYLOAD(m_renderer->m_sgContext->rhiContext()->stats()));

            if (m_renderer->renderStats())
                m_renderer->renderStats()->endRender(dumpRenderTimes);
        }
    }
}

QT_END_NAMESPACE
