/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "details/Renderer.h"

#include "PostProcessManager.h"
#include "RenderPass.h"
#include "ResourceAllocator.h"

#include "details/Engine.h"
#include "details/Fence.h"
#include "details/Scene.h"
#include "details/SwapChain.h"
#include "details/Texture.h"
#include "details/View.h"

#include <filament/Scene.h>
#include <filament/Renderer.h>

#include <backend/PixelBufferDescriptor.h>

#include "fg2/FrameGraph.h"
#include "fg2/FrameGraphId.h"
#include "fg2/FrameGraphResources.h"

#include <utils/compiler.h>
#include <utils/Panic.h>
#include <utils/Systrace.h>
#include <utils/vector.h>
#include <utils/debug.h>

// this helps visualize what dynamic-scaling is doing
#define DEBUG_DYNAMIC_SCALING false

using namespace filament::math;
using namespace utils;

namespace filament {

using namespace backend;

FRenderer::FRenderer(FEngine& engine) :
        mEngine(engine),
        mFrameSkipper(1u),
        mFrameInfoManager(engine.getDriverApi()),
        mIsRGB8Supported(false),
        mPerRenderPassArena(engine.getPerRenderPassAllocator())
{
    FDebugRegistry& debugRegistry = engine.getDebugRegistry();
    debugRegistry.registerProperty("d.ssao.enabled", &engine.debug.ssao.enabled);

    debugRegistry.registerProperty("d.renderer.doFrameCapture",
            &engine.debug.renderer.doFrameCapture);
}

void FRenderer::init() noexcept {
    DriverApi& driver = mEngine.getDriverApi();
    mUserEpoch = mEngine.getEngineEpoch();
    mRenderTarget = driver.createDefaultRenderTarget();
    mIsRGB8Supported = driver.isRenderTargetFormatSupported(TextureFormat::RGB8);

    // our default HDR translucent format, fallback to LDR if not supported by the backend
    mHdrTranslucent = TextureFormat::RGBA16F;
    if (!driver.isRenderTargetFormatSupported(TextureFormat::RGBA16F)) {
        // this will clip all HDR data, but we don't have a choice
        mHdrTranslucent = TextureFormat::RGBA8;
    }

    // our default opaque low/medium quality HDR format, fallback to LDR if not supported
    mHdrQualityMedium = TextureFormat::R11F_G11F_B10F;
    if (!driver.isRenderTargetFormatSupported(mHdrQualityMedium)) {
        // this will clip all HDR data, but we don't have a choice
        mHdrQualityMedium = TextureFormat::RGB8;
    }

    // our default opaque high quality HDR format, fallback to RGBA, then medium, then LDR
    // if not supported
    mHdrQualityHigh = TextureFormat::RGB16F;
    if (!driver.isRenderTargetFormatSupported(mHdrQualityHigh)) {
        mHdrQualityHigh = TextureFormat::RGBA16F;
    }
    if (!driver.isRenderTargetFormatSupported(mHdrQualityHigh)) {
        mHdrQualityHigh = TextureFormat::R11F_G11F_B10F;
    }
    if (!driver.isRenderTargetFormatSupported(mHdrQualityHigh)) {
        // this will clip all HDR data, but we don't have a choice
        mHdrQualityHigh = TextureFormat::RGB8;
    }
}

FRenderer::~FRenderer() noexcept {
    // There shouldn't be any resource left when we get here, but if there is, make sure
    // to free what we can (it would probably mean something when wrong).
#ifndef NDEBUG
    size_t wm = getCommandsHighWatermark();
    size_t wmpct = wm / (CONFIG_PER_FRAME_COMMANDS_SIZE / 100);
    slog.d << "Renderer: Commands High watermark "
    << wm / 1024 << " KiB (" << wmpct << "%), "
    << wm / sizeof(Command) << " commands, " << sizeof(Command) << " bytes/command"
    << io::endl;
#endif
}

void FRenderer::terminate(FEngine& engine) {
    // Here we would cleanly free resources we've allocated or we own, in particular we would
    // shut down threads if we created any.
    DriverApi& driver = engine.getDriverApi();
    driver.destroyRenderTarget(mRenderTarget);

    // before we can destroy this Renderer's resources, we must make sure
    // that all pending commands have been executed (as they could reference data in this
    // instance, e.g. Fences, Callbacks, etc...)
    if (UTILS_HAS_THREADING) {
        Fence::waitAndDestroy(engine.createFence(FFence::Type::SOFT));
    } else {
        // In single threaded mode, allow recently-created objects (e.g. no-op fences in Skipper)
        // to initialize themselves, otherwise the engine tries to destroy invalid handles.
        engine.execute();
    }
    mFrameInfoManager.terminate(driver);
    mFrameSkipper.terminate(driver);
}

void FRenderer::resetUserTime() {
    mUserEpoch = std::chrono::steady_clock::now();
}

TextureFormat FRenderer::getHdrFormat(const View& view, bool translucent) const noexcept {
    if (translucent) {
        return mHdrTranslucent;
    }
    switch (view.getRenderQuality().hdrColorBuffer) {
        case QualityLevel::LOW:
        case QualityLevel::MEDIUM:
            return mHdrQualityMedium;
        case QualityLevel::HIGH:
        case QualityLevel::ULTRA: {
            return mHdrQualityHigh;
        }
    }
}

TextureFormat FRenderer::getLdrFormat(bool translucent) const noexcept {
    return (translucent || !mIsRGB8Supported) ? TextureFormat::RGBA8 : TextureFormat::RGB8;
}

void FRenderer::renderStandaloneView(FView const* view) {
    SYSTRACE_CALL();

    using namespace std::chrono;

    ASSERT_PRECONDITION(view->getRenderTarget(),
            "View \"%s\" must have a RenderTarget associated", view->getName());

    if (UTILS_LIKELY(view->getScene())) {
        mPreviousRenderTargets.clear();
        mFrameId++;

        // ask the engine to do what it needs to (e.g. updates light buffer, materials...)
        FEngine& engine = getEngine();
        engine.prepare();

        FEngine::DriverApi& driver = engine.getDriverApi();
        driver.beginFrame(steady_clock::now().time_since_epoch().count(), mFrameId);

        renderInternal(view);

        driver.endFrame(mFrameId);
    }
}

void FRenderer::render(FView const* view) {
    SYSTRACE_CALL();

    assert_invariant(mSwapChain);

    if (mBeginFrameInternal) {
        mBeginFrameInternal();
        mBeginFrameInternal = {};
    }

    if (UTILS_LIKELY(view && view->getScene())) {
        renderInternal(view);
    }
}

void FRenderer::renderInternal(FView const* view) {
    // per-renderpass data
    ArenaScope rootArena(mPerRenderPassArena);

    FEngine& engine = mEngine;
    JobSystem& js = engine.getJobSystem();

    // create a root job so no other job can escape
    auto *rootJob = js.setRootJob(js.createJob());

    // execute the render pass
    renderJob(rootArena, const_cast<FView&>(*view));

    // make sure to flush the command buffer
    engine.flush();

    // and wait for all jobs to finish as a safety (this should be a no-op)
    js.runAndWait(rootJob);
}

void FRenderer::renderJob(ArenaScope& arena, FView& view) {
    FEngine& engine = getEngine();
    JobSystem& js = engine.getJobSystem();
    FEngine::DriverApi& driver = engine.getDriverApi();
    PostProcessManager& ppm = engine.getPostProcessManager();

    // DEBUG: driver commands must all happen from the same thread. Enforce that on debug builds.
    engine.getDriverApi().debugThreading();

    filament::Viewport const& vp = view.getViewport();
    const bool hasPostProcess = view.hasPostProcessPass();
    bool hasColorGrading = hasPostProcess;
    bool hasDithering = view.getDithering() == Dithering::TEMPORAL;
    bool hasFXAA = view.getAntiAliasing() == AntiAliasing::FXAA;
    float2 scale = view.updateScale(engine, mFrameInfoManager.getLastFrameInfo(), mFrameRateOptions, mDisplayInfo);
    auto msaaOptions = view.getMultiSampleAntiAliasingOptions();
    auto dsrOptions = view.getDynamicResolutionOptions();
    auto bloomOptions = view.getBloomOptions();
    auto dofOptions = view.getDepthOfFieldOptions();
    auto aoOptions = view.getAmbientOcclusionOptions();
    auto taaOptions = view.getTemporalAntiAliasingOptions();
    auto vignetteOptions = view.getVignetteOptions();
    auto colorGrading = view.getColorGrading();
    auto ssReflectionsOptions = view.getScreenSpaceReflectionsOptions();
    if (!hasPostProcess) {
        // disable all effects that are part of post-processing
        dofOptions.enabled = false;
        bloomOptions.enabled = false;
        vignetteOptions.enabled = false;
        taaOptions.enabled = false;
        hasColorGrading = false;
        hasDithering = false;
        hasFXAA = false;
        scale = 1.0f;
    }

    const uint8_t msaaSampleCount = msaaOptions.enabled ? msaaOptions.sampleCount : 1u;

    const bool scaled = any(notEqual(scale, float2(1.0f)));
    filament::Viewport svp = vp.scale(scale);
    if (svp.empty()) {
        return;
    }

    view.prepare(engine, driver, arena, svp, getShaderUserTime());

    view.prepareUpscaler(scale);

    // start froxelization immediately, it has no dependencies
    JobSystem::Job* jobFroxelize = nullptr;
    if (view.hasDynamicLighting()) {
        jobFroxelize = js.runAndRetain(js.createJob(nullptr,
                [&engine, &view](JobSystem&, JobSystem::Job*) { view.froxelize(engine); }));
    }

    /*
     * Allocate command buffer
     */

    FScene& scene = *view.getScene();

    // Allocate some space for our commands in the per-frame Arena, and use that space as
    // an Arena for commands. All this space is released when we exit this method.
    void* const arenaBegin = arena.allocate(FEngine::CONFIG_PER_FRAME_COMMANDS_SIZE, CACHELINE_SIZE);
    void* const arenaEnd = pointermath::add(arenaBegin, FEngine::CONFIG_PER_FRAME_COMMANDS_SIZE);
    RenderPass::Arena commandArena("Command Arena", { arenaBegin, arenaEnd });

    RenderPass::RenderFlags renderFlags = 0;
    if (view.hasShadowing())                renderFlags |= RenderPass::HAS_SHADOWING;
    if (view.isFrontFaceWindingInverted())  renderFlags |= RenderPass::HAS_INVERSE_FRONT_FACES;

    RenderPass pass(engine, commandArena);
    pass.setRenderFlags(renderFlags);

    Variant variant;
    variant.setDirectionalLighting(view.hasDirectionalLight());
    variant.setDynamicLighting(view.hasDynamicLighting());
    variant.setFog(view.hasFog());
    variant.setVsm(view.hasShadowing() && view.getShadowType() != ShadowType::PCF);

    /*
     * Frame graph
     */

    FrameGraph fg(engine.getResourceAllocator());
    auto& blackboard = fg.getBlackboard();

    /*
     * Shadow pass
     */

    if (view.needsShadowMap()) {
        Variant shadowVariant(Variant::DEPTH_VARIANT);
        shadowVariant.setVsm(view.getShadowType() == ShadowType::VSM);

        RenderPass shadowPass(pass);
        shadowPass.setVariant(shadowVariant);
        view.renderShadowMaps(fg, engine, driver, shadowPass);
    }

    // When we don't have a custom RenderTarget, currentRenderTarget below is nullptr and is
    // recorded in the list of targets already rendered into -- this ensures that
    // initializeClearFlags() is called only once for the default RenderTarget.
    auto& previousRenderTargets = mPreviousRenderTargets;
    FRenderTarget* const currentRenderTarget = upcast(view.getRenderTarget());
    if (UTILS_LIKELY(
            previousRenderTargets.find(currentRenderTarget) == previousRenderTargets.end())) {
        previousRenderTargets.insert(currentRenderTarget);
        initializeClearFlags();
    }

    const float4 clearColor = mClearOptions.clearColor;
    const TargetBufferFlags clearFlags = mClearFlags;
    const TargetBufferFlags discardStartFlags = mDiscardStartFlags;
    TargetBufferFlags keepOverrideStartFlags = TargetBufferFlags::ALL & ~discardStartFlags;
    TargetBufferFlags keepOverrideEndFlags = TargetBufferFlags::NONE;

    if (currentRenderTarget) {
        // For custom RenderTarget, we look at each attachment flag and if they have their
        // SAMPLEABLE usage bit set, we assume they must not be discarded after the render pass.
        // "+1" to the count for the DEPTH attachment (we don't have stencil for the public RenderTarget)
        for (size_t i = 0; i < RenderTarget::MAX_SUPPORTED_COLOR_ATTACHMENTS_COUNT + 1; i++) {
            auto attachment = currentRenderTarget->getAttachment((RenderTarget::AttachmentPoint)i);
            if (attachment.texture && any(attachment.texture->getUsage() &
                    (TextureUsage::SAMPLEABLE | Texture::Usage::SUBPASS_INPUT))) {
                keepOverrideEndFlags |= backend::getTargetBufferFlagsAt(i);
            }
        }
    }

    // Renderer's ClearOptions apply once at the beginning of the frame (not for each View),
    // however, it's implemented as part of executing a render pass on the current render target,
    // and that happens for each View. So we need to disable clearing after the 1st View has
    // been processed.
    mDiscardStartFlags &= ~TargetBufferFlags::COLOR;
    mClearFlags &= ~TargetBufferFlags::COLOR;

    Handle<HwRenderTarget> viewRenderTarget;
    TargetBufferFlags attachmentMask;
    getRenderTarget(view, attachmentMask, viewRenderTarget);
    FrameGraphId<FrameGraphTexture> fgViewRenderTarget = fg.import("viewRenderTarget",
            {
                    .attachments = attachmentMask,
                    .viewport = DEBUG_DYNAMIC_SCALING ? svp : vp,
                    .clearColor = clearColor,
                    .samples = 0,
                    .clearFlags = clearFlags,
                    .keepOverrideStart = keepOverrideStartFlags,
                    .keepOverrideEnd = keepOverrideEndFlags
            }, viewRenderTarget);

    const bool blendModeTranslucent = view.getBlendMode() == BlendMode::TRANSLUCENT;
    const bool blending = blendModeTranslucent;
    // If the swapchain is transparent or if we blend into it, we need to allocate our intermediate
    // buffers with an alpha channel.
    const bool needsAlphaChannel = (mSwapChain ? mSwapChain->isTransparent() : false) || blendModeTranslucent;
    const TextureFormat hdrFormat = getHdrFormat(view, needsAlphaChannel);

    ColorPassConfig config{
            .vp = vp,
            .svp = svp,
            .scale = scale,
            .hdrFormat = hdrFormat,
            .msaa = msaaSampleCount,
            .clearFlags = clearFlags,
            .clearColor = clearColor,
            .ssrLodOffset = 0.0f,
            .hasContactShadows = scene.hasContactShadows(),
            .hasScreenSpaceReflections = ssReflectionsOptions.enabled
    };

    // asSubpass is disabled with TAA (although it's supported) because performance was degraded
    // on qualcomm hardware -- we might need a backend dependent toggle at some point
    const PostProcessManager::ColorGradingConfig colorGradingConfig{
            .asSubpass =
                    hasColorGrading &&
                    msaaSampleCount <= 1 &&
                    !bloomOptions.enabled && !dofOptions.enabled && !taaOptions.enabled &&
                    driver.isFrameBufferFetchSupported(),
            .customResolve =
                    msaaOptions.customResolve &&
                    msaaSampleCount > 1 &&
                    hasColorGrading &&
                    driver.isFrameBufferFetchMultiSampleSupported(),
            .translucent = needsAlphaChannel,
            .fxaa = hasFXAA,
            .dithering = hasDithering,
            .ldrFormat = (hasColorGrading && hasFXAA) ? TextureFormat::RGBA8 : getLdrFormat(needsAlphaChannel)
    };

    /*
     * Depth + Color passes
     */

    CameraInfo cameraInfo = view.getCameraInfo();

    // updatePrimitivesLod must be run before appendCommands and once for each set
    // of RenderPass::setCamera / RenderPass::setGeometry calls.
    view.updatePrimitivesLod(engine, cameraInfo,
            scene.getRenderableData(), view.getVisibleRenderables());

    pass.setCamera(cameraInfo);
    pass.setGeometry(scene.getRenderableData(), view.getVisibleRenderables(), scene.getRenderableUBO());

    // view set-ups that need to happen before rendering
    fg.addTrivialSideEffectPass("Prepare View Uniforms",
            [=, &uniforms = view.getPerViewUniforms()](DriverApi& driver) {
                uniforms.prepareCamera(cameraInfo);
                uniforms.prepareViewport(svp);
                uniforms.commit(driver);
            });

    // --------------------------------------------------------------------------------------------
    // structure pass -- automatically culled if not used
    // Currently it consists of a simple depth pass.
    // This is normally used by SSAO and contact-shadows

    // TODO: the scaling should depends on all passes that need the structure pass
    ppm.structure(fg, pass, renderFlags, svp.width, svp.height, {
            .scale = aoOptions.resolution,
            .picking = view.hasPicking()
    });

    if (view.hasPicking()) {
        struct PickingResolvePassData {
            FrameGraphId<FrameGraphTexture> picking;
        };
        auto picking = blackboard.get<FrameGraphTexture>("picking");
        fg.addPass<PickingResolvePassData>("Picking Resolve Pass",
                [&](FrameGraph::Builder& builder, auto& data) {
                    data.picking = builder.read(picking,
                            FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                    builder.declareRenderPass("Picking Resolve Target", {
                            .attachments = { .color = { data.picking }}
                    });
                    builder.sideEffect();
                },
                [=, &view](FrameGraphResources const& resources,
                        auto const& data, DriverApi& driver) mutable {
                    auto out = resources.getRenderPassInfo();
                    view.executePickingQueries(driver, out.target, aoOptions.resolution);
                });
    }

    // Store this frame's camera projection in the frame history.
    if (UTILS_UNLIKELY(taaOptions.enabled)) {
        auto& history = view.getFrameHistory();
        auto& current = history.getCurrent();
        auto const& previous = history[0];
        current.taa.projection = cameraInfo.projection * (cameraInfo.view * cameraInfo.worldOrigin);
        // update frame id
        current.frameId = previous.frameId + 1; // TODO: should probably be done somewhere else
    }

    // Apply the TAA jitter to everything after the structure pass, starting with the color pass.
    if (taaOptions.enabled) {
        auto& history = view.getFrameHistory();
        ppm.prepareTaa(history, cameraInfo, taaOptions);
        // convert the sample position to jitter in clip-space
        float2 jitterInClipSpace =
                history.getCurrent().taa.jitter * (2.0f / float2{ svp.width, svp.height });
        // update projection matrix
        cameraInfo.projection[2].xy -= jitterInClipSpace;

        fg.addTrivialSideEffectPass("Jitter Camera", [=, &view] (DriverApi& driver) {
            view.prepareCamera(cameraInfo);
            view.commitUniforms(driver);
        });
    }

    // --------------------------------------------------------------------------------------------
    // SSAO pass

    if (aoOptions.enabled) {
        // we could rely on FrameGraph culling, but this creates unnecessary CPU work
        ppm.screenSpaceAmbientOcclusion(fg, svp, cameraInfo, aoOptions);
    }

    // --------------------------------------------------------------------------------------------
    // screen-space reflections pass

    if (config.hasScreenSpaceReflections) {
        auto reflections = ppm.ssr(fg, pass,
                view.getFrameHistory(), cameraInfo,
                view.getPerViewUniforms(), view.getScreenSpaceReflectionsOptions(),
                { .width = svp.width, .height = svp.height });

        // generate the mipchain
        reflections = ppm.generateMipmapSSR(fg, reflections,
                view.getCameraUser().getFieldOfView(Camera::Fov::VERTICAL),
                config.svp, config.scale,
                TextureFormat::RGBA16F,
                &config.ssrLodOffset);

        blackboard["ssr"] = reflections;
    }

    // --------------------------------------------------------------------------------------------
    // Color passes

    // This one doesn't need to be a FrameGraph pass because it always happens by construction
    // (i.e. it won't be culled, unless everything is culled), so no need to complexify things.
    pass.setVariant(variant);
    pass.appendCommands(RenderPass::COLOR);
    pass.sortCommands();

    FrameGraphTexture::Descriptor desc = {
            .width = config.svp.width,
            .height = config.svp.height,
            .format = config.hdrFormat
    };

    // a non-drawing pass to prepare everything that need to be before the color passes execute
    fg.addTrivialSideEffectPass("Prepare Color Passes",
            [=, &js, &view, &ppm](DriverApi& driver) {
                // prepare color grading as subpass material
                if (colorGradingConfig.asSubpass) {
                    ppm.colorGradingPrepareSubpass(driver,
                            colorGrading, colorGradingConfig, vignetteOptions,
                            config.svp.width, config.svp.height);
                } else if (colorGradingConfig.customResolve) {
                    ppm.customResolvePrepareSubpass(driver,
                            PostProcessManager::CustomResolveOp::COMPRESS);
                }

                // We use a framegraph pass to wait for froxelization to finish (so it can be done
                // in parallel with .compile()
                if (jobFroxelize) {
                    auto *sync = jobFroxelize;
                    js.waitAndRelease(sync);
                    view.commitFroxels(driver);
                }
            }
    );

    // color-grading as subpass is done either by the color pass or the TAA pass if any
    auto colorGradingConfigForColor = colorGradingConfig;
    colorGradingConfigForColor.asSubpass = colorGradingConfigForColor.asSubpass && !taaOptions.enabled;

    if (colorGradingConfigForColor.asSubpass) {
        // append color grading subpass after all other passes
        pass.appendCustomCommand(
                RenderPass::Pass::BLENDED,
                RenderPass::CustomCommand::EPILOG,
                0, [&ppm, &driver, colorGradingConfigForColor]() {
                    ppm.colorGradingSubpass(driver, colorGradingConfigForColor);
                });
    } if (colorGradingConfig.customResolve) {
        // append custom resolve subpass after all other passes
        pass.appendCustomCommand(
                RenderPass::Pass::BLENDED,
                RenderPass::CustomCommand::EPILOG,
                0, [&ppm, &driver]() {
                    ppm.customResolveSubpass(driver);
                });
    }

    // the color pass itself + color-grading as subpass if needed
    FrameGraphId<FrameGraphTexture> colorPassOutput = colorPass(fg, "Color Pass",
            desc, config, colorGradingConfigForColor, pass.getExecutor(), view);

    if (view.isScreenSpaceRefractionEnabled() && !pass.empty()) {
        // this cancels the colorPass() call above if refraction is active.
        // the color pass + refraction + color-grading as subpass if needed
        colorPassOutput = refractionPass(fg, config, colorGradingConfigForColor, pass, view);
    }

    if (colorGradingConfig.customResolve) {
        // TODO: we have to "uncompress" (i.e. detonemap) the color buffer here because it's  used
        //       by many other passes (Bloom, TAA, DoF, etc...). We could make this more
        //       efficient by using ARM_shader_framebuffer_fetch. We use a load/store (i.e.
        //       subpass) here because it's more convenient.
        colorPassOutput = ppm.customResolveUncompressPass(fg, colorPassOutput);
    }

    // export the color buffer if screen-space reflections are enabled
    if (ssReflectionsOptions.enabled) {
        struct ExportSSRHistoryData {
            FrameGraphId<FrameGraphTexture> history;
        };
        fg.addPass<ExportSSRHistoryData>("Export SSR history",
                [&](FrameGraph::Builder& builder, auto& data) {
                    // We need to use sideEffect here to ensure this pass won't be culled.
                    // The "output" of this pass is going to be used during the next frame as
                    // an "import".
                    builder.sideEffect();
                    data.history = builder.sample(colorPassOutput); // FIXME: an access must be declared for detach(), why?
                }, [&view](FrameGraphResources const& resources, auto const& data,
                        backend::DriverApi&) {
                    const auto& cameraInfo = view.getCameraInfo();
                    auto& history = view.getFrameHistory();
                    auto& current = history.getCurrent();
                    current.ssr.projection = cameraInfo.projection * (cameraInfo.view * cameraInfo.worldOrigin);
                    resources.detach(data.history,
                            &current.ssr.color, &current.ssr.desc);
                });
    }

    FrameGraphId<FrameGraphTexture> input = colorPassOutput;
    fg.addTrivialSideEffectPass("Finish Color Passes", [&view](DriverApi& driver) {
        // Unbind SSAO sampler, b/c the FrameGraph will delete the texture at the end of the pass.
        view.cleanupRenderPasses();
        view.commitUniforms(driver);
    });

    // resolve depth -- which might be needed because of TAA or DoF. This pass will be culled
    // if the depth is not used below.
    auto depth = blackboard.get<FrameGraphTexture>("depth");
    depth = ppm.resolveBaseLevel(fg, "Resolved Depth Buffer", depth);
    blackboard.put("depth", depth);

    // TODO: DoF should be applied here, before TAA -- but if we do this it'll result in a lot of
    //       fireflies due to the instability of the highlights. This can be fixed with a
    //       dedicated TAA pass for the DoF, as explained in
    //       "Life of a Bokeh" by Guillaume Abadie, SIGGRAPH 2018

    // TAA for color pass
    if (taaOptions.enabled) {
        input = ppm.taa(fg, input, view.getFrameHistory(), taaOptions, colorGradingConfig);

        struct ExportColorHistoryData {
            FrameGraphId<FrameGraphTexture> color;
        };
        fg.addPass<ExportColorHistoryData>("Export TAA history",
                [&](FrameGraph::Builder& builder, auto& data) {
                    // We need to use sideEffect here to ensure this pass won't be culled.
                    // The "output" of this pass is going to be used during the next frame as
                    // an "import".
                    builder.sideEffect();
                    data.color = builder.sample(input); // FIXME: an access must be declared for detach(), why?
                }, [&view](FrameGraphResources const& resources, auto const& data,
                        backend::DriverApi&) {
                    FrameHistoryEntry& current = view.getFrameHistory().getCurrent();
                    resources.detach(data.color,
                            &current.taa.color, &current.taa.desc);
                });
    }

    // --------------------------------------------------------------------------------------------
    // Post Processing...

    bool mightNeedFinalBlit = true;
    if (hasPostProcess) {
        if (dofOptions.enabled) {
            input = ppm.dof(fg, input, dofOptions, needsAlphaChannel, cameraInfo, scale);
        }

        if (bloomOptions.enabled) {
            // generate the bloom buffer, which is stored in the blackboard as "bloom". This is
            // consumed by the colorGrading pass and will be culled if colorGrading is disabled.
            ppm.bloom(fg, input, bloomOptions, TextureFormat::R11F_G11F_B10F, scale);
        }

        if (hasColorGrading) {
            if (!colorGradingConfig.asSubpass) {
                input = ppm.colorGrading(fg, input,
                        colorGrading, colorGradingConfig,
                        bloomOptions, vignetteOptions, scale);
            }
        }
        if (hasFXAA) {
            input = ppm.fxaa(fg, input, colorGradingConfig.ldrFormat,
                    !hasColorGrading || needsAlphaChannel);
        }
        if (scaled) {
            mightNeedFinalBlit = false;
            auto viewport = DEBUG_DYNAMIC_SCALING ? svp : vp;
            if (UTILS_LIKELY(!blending && dsrOptions.quality == QualityLevel::LOW)) {
                input = ppm.opaqueBlit(fg, input, {
                        .width = viewport.width, .height = viewport.height,
                        .format = colorGradingConfig.ldrFormat }, SamplerMagFilter::LINEAR);
            } else {
                input = ppm.blendBlit(fg, blending, dsrOptions, input, {
                        .width = viewport.width, .height = viewport.height,
                        .format = colorGradingConfig.ldrFormat });
            }
        }
    }

    // We need to do special processing when rendering directly into the swap-chain, that is when
    // the viewRenderTarget is the default render target (mRenderTarget) and we're rendering into
    // it.
    // * This is because the default render target is not multi-sampled, so we need an
    //   intermediate buffer when MSAA is enabled.
    // * We also need an extra buffer for blending the result to the framebuffer if the view
    //   is translucent AND we've not already done it as part of upscaling.
    // * And we can't use the default rendertarget if MRT is required (e.g. with color grading
    //   as a subpass)
    // The intermediate buffer is accomplished with a "fake" opaqueBlit (i.e. blit) operation.

    const bool outputIsSwapChain = (input == colorPassOutput) && (viewRenderTarget == mRenderTarget);
    if (mightNeedFinalBlit &&
            ((outputIsSwapChain && (msaaSampleCount > 1 || colorGradingConfig.asSubpass)) ||
             blending)) {
        assert_invariant(!scaled);
        if (UTILS_LIKELY(!blending)) {
            input = ppm.opaqueBlit(fg, input, {
                    .width = vp.width, .height = vp.height,
                    .format = colorGradingConfig.ldrFormat }, SamplerMagFilter::NEAREST);
        } else {
            input = ppm.blendBlit(fg, blending, {
                    .quality = QualityLevel::LOW
            }, input, {
                    .width = vp.width, .height = vp.height,
                    .format = colorGradingConfig.ldrFormat });
        }
    }


//    auto debug = fg.getBlackboard().get<FrameGraphTexture>("structure");
//    fg.forwardResource(fgViewRenderTarget, debug ? debug : input);

    fg.forwardResource(fgViewRenderTarget, input);

    fg.present(fgViewRenderTarget);

    fg.compile();

    //fg.export_graphviz(slog.d, view.getName());

    fg.execute(driver);

    // save the current history entry and destroy the oldest entry
    view.commitFrameHistory(engine);

    //recordHighWatermark(pass.getCommandsHighWatermark());
}

FrameGraphId<FrameGraphTexture> FRenderer::refractionPass(FrameGraph& fg,
        ColorPassConfig config,
        PostProcessManager::ColorGradingConfig colorGradingConfig,
        RenderPass const& pass,
        FView const& view) const noexcept {

    auto& blackboard = fg.getBlackboard();
    auto input = blackboard.get<FrameGraphTexture>("color");
    FrameGraphId<FrameGraphTexture> output;

    // find the first refractive object
    Command const* const refraction = std::partition_point(pass.begin(), pass.end(),
            [](auto const& command) {
                return (command.key & RenderPass::PASS_MASK) < uint64_t(RenderPass::Pass::REFRACT);
            });

    const bool hasScreenSpaceRefraction =
            (refraction->key & RenderPass::PASS_MASK) == uint64_t(RenderPass::Pass::REFRACT);

    // if there wasn't any refractive object, just skip everything below.
    if (UTILS_UNLIKELY(hasScreenSpaceRefraction)) {
        PostProcessManager& ppm = mEngine.getPostProcessManager();
        float refractionLodOffset = 0.0f;

        // clear the color/depth buffers, which will orphan (and cull) the color pass
        input.clear();
        blackboard.remove("color");
        blackboard.remove("depth");

        input = colorPass(fg, "Color Pass (opaque)",
                {
                        // When rendering the opaques, we need to conserve the sample buffer,
                        // so create config that specifies the sample count.
                        .width = config.svp.width,
                        .height = config.svp.height,
                        .samples = config.msaa,
                        .format = config.hdrFormat
                },
                config,
                { .asSubpass = false },
                { pass.getExecutor(pass.begin(), refraction) },
                view);

        // generate the mipmap chain
        input = ppm.generateMipmapSSR(fg, input,
                view.getCameraUser().getFieldOfView(Camera::Fov::VERTICAL),
                config.svp, config.scale,
                TextureFormat::R11F_G11F_B10F,
                &refractionLodOffset);

        // and this becomes our SSR buffer
        blackboard["ssr"] = input;


        // Now we're doing the refraction pass proper.
        // This uses the same framebuffer (color and depth) used by the opaque pass. This happens
        // automatically because these are set in the Blackboard (they were set by the opaque
        // pass). For this reason, `desc` below is only used in colorPass() for the width and
        // height.
        config.clearFlags = TargetBufferFlags::NONE;
        config.hasScreenSpaceReflections = false; // FIXME: for now we can't have both
        config.ssrLodOffset = refractionLodOffset;
        output = colorPass(fg, "Color Pass (transparent)",
                {
                        .width = config.svp.width,
                        .height = config.svp.height
                },
                config,
                colorGradingConfig,
                { pass.getExecutor(refraction, pass.end()) },
                view);

        if (config.msaa > 1 && !colorGradingConfig.asSubpass) {
            // We need to do a resolve here because later passes (such as color grading or DoF) will need
            // to sample from 'output'. However, because we have MSAA, we know we're not sampleable.
            // And this is because in the SSR case, we had to use a renderbuffer to conserve the
            // multi-sample buffer.
            output = ppm.resolveBaseLevel(fg, "Resolved Color Buffer", output);
        }
    } else {
        output = input;
    }
    return output;
}

FrameGraphId<FrameGraphTexture> FRenderer::colorPass(FrameGraph& fg, const char* name,
        FrameGraphTexture::Descriptor const& colorBufferDesc,
        ColorPassConfig const& config, PostProcessManager::ColorGradingConfig colorGradingConfig,
        RenderPass::Executor const& passExecutor, FView const& view) const noexcept {

    struct ColorPassData {
        FrameGraphId<FrameGraphTexture> shadows;
        FrameGraphId<FrameGraphTexture> color;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphId<FrameGraphTexture> depth;
        FrameGraphId<FrameGraphTexture> ssao;
        FrameGraphId<FrameGraphTexture> ssr;    // either screen-space reflections or refractions
        FrameGraphId<FrameGraphTexture> structure;
        float4 clearColor{};
        TargetBufferFlags clearFlags{};
    };

    Blackboard& blackboard = fg.getBlackboard();

    auto& colorPass = fg.addPass<ColorPassData>(name,
            [&](FrameGraph::Builder& builder, ColorPassData& data) {

                TargetBufferFlags clearDepthFlags = config.clearFlags & TargetBufferFlags::DEPTH;
                TargetBufferFlags clearColorFlags = config.clearFlags & TargetBufferFlags::COLOR;

                data.shadows = blackboard.get<FrameGraphTexture>("shadows");
                data.ssao = blackboard.get<FrameGraphTexture>("ssao");
                data.color = blackboard.get<FrameGraphTexture>("color");
                data.depth = blackboard.get<FrameGraphTexture>("depth");

                // Screen-space reflection or refractions
                data.ssr = blackboard.get<FrameGraphTexture>("ssr");
                if (data.ssr) {
                    data.ssr = builder.sample(data.ssr);
                }

                if (config.hasContactShadows || config.hasScreenSpaceReflections) {
                    data.structure = blackboard.get<FrameGraphTexture>("structure");
                    assert_invariant(data.structure);
                    data.structure = builder.sample(data.structure);
                }

                if (data.shadows) {
                    data.shadows = builder.sample(data.shadows);
                }

                if (data.ssao) {
                    data.ssao = builder.sample(data.ssao);
                }

                if (!data.color) {
                    // FIXME: this works only when the viewport is full
                    //  if (view.isSkyboxVisible()) {
                    //      // if the skybox is visible, then we don't need to clear at all
                    //      clearColorFlags &= ~TargetBufferFlags::COLOR;
                    //  }
                    data.color = builder.createTexture("Color Buffer", colorBufferDesc);
                }

                if (!data.depth) {
                    // clear newly allocated depth buffers, regardless of given clear flags
                    clearDepthFlags = TargetBufferFlags::DEPTH;
                    data.depth = builder.createTexture("Depth Buffer", {
                            .width = colorBufferDesc.width,
                            .height = colorBufferDesc.height,
                            // If the color attachment requested MS, we assume this means the MS buffer
                            // must be kept, and for that reason we allocate the depth buffer with MS
                            // as well. On the other hand, if the color attachment was allocated without
                            // MS, no need to allocate the depth buffer with MS, if the RT is MS,
                            // the tile depth buffer will be MS, but it'll be resolved to single
                            // sample automatically -- which is what we want.
                            .samples = colorBufferDesc.samples,
                            .format = TextureFormat::DEPTH32F,
                    });
                }

                if (colorGradingConfig.asSubpass) {
                    data.output = builder.createTexture("Tonemapped Buffer", {
                            .width = colorBufferDesc.width,
                            .height = colorBufferDesc.height,
                            .format = colorGradingConfig.ldrFormat
                    });
                    data.color = builder.read(data.color, FrameGraphTexture::Usage::SUBPASS_INPUT);
                    data.output = builder.write(data.output, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                } else if (colorGradingConfig.customResolve) {
                    data.color = builder.read(data.color, FrameGraphTexture::Usage::SUBPASS_INPUT);
                }

                // We set a "read" constraint on these attachments here because we need to preserve them
                // when the color pass happens in several passes (e.g. with SSR)
                data.color = builder.read(data.color, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                data.depth = builder.read(data.depth, FrameGraphTexture::Usage::DEPTH_ATTACHMENT);

                data.color = builder.write(data.color, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                data.depth = builder.write(data.depth, FrameGraphTexture::Usage::DEPTH_ATTACHMENT);

                builder.declareRenderPass("Color Pass Target", {
                        .attachments = { .color = { data.color, data.output }, .depth = data.depth },
                        .samples = config.msaa,
                        .clearFlags = clearColorFlags | clearDepthFlags });

                data.clearColor = config.clearColor;
                data.clearFlags = clearColorFlags | clearDepthFlags;

                blackboard["depth"] = data.depth;
            },
            [=, &view](FrameGraphResources const& resources,
                            ColorPassData const& data, DriverApi& driver) {
                auto out = resources.getRenderPassInfo();

                // set samplers and uniforms
                PostProcessManager& ppm = getEngine().getPostProcessManager();
                view.prepareSSAO(data.ssao ?
                        resources.getTexture(data.ssao) : ppm.getOneTextureArray());

                // set shadow sampler
                view.prepareShadow(data.shadows ?
                        resources.getTexture(data.shadows) : ppm.getOneTextureArray());

                // set structure sampler
                view.prepareStructure(data.structure ?
                        resources.getTexture(data.structure) : ppm.getOneTexture());

                // set screen-space reflections and screen-space refractions
                TextureHandle ssr = data.ssr ?
                        resources.getTexture(data.ssr) : ppm.getOneTexture();

                view.prepareSSR(ssr, config.ssrLodOffset,
                        view.getScreenSpaceReflectionsOptions());

                view.prepareViewport(static_cast<filament::Viewport&>(out.params.viewport));
                view.commitUniforms(driver);

                out.params.clearColor = data.clearColor;
                out.params.flags.clear = data.clearFlags;
                if (view.getBlendMode() == BlendMode::TRANSLUCENT) {
                    if (any(out.params.flags.discardStart & TargetBufferFlags::COLOR0)) {
                        // if the buffer is discarded (e.g. it's new) and we're blending,
                        // then clear it to transparent
                        out.params.flags.clear |= TargetBufferFlags::COLOR;
                        out.params.clearColor = {};
                    }
                }

                if (colorGradingConfig.asSubpass || colorGradingConfig.customResolve) {
                    out.params.subpassMask = 1;
                }
                passExecutor.execute(resources.getPassName(), out.target, out.params);

                // color pass is typically heavy and we don't have much CPU work left after
                // this point, so flushing now allows us to start the GPU earlier and reduce
                // latency, without creating bubbles.
                driver.flush();
            }
    );

    // when color grading is done as a subpass, the output of the color-pass is the ldr buffer
    auto output = colorGradingConfig.asSubpass ? colorPass->output : colorPass->color;

    fg.getBlackboard()["color"] = output;
    return output;
}

void FRenderer::copyFrame(FSwapChain* dstSwapChain, filament::Viewport const& dstViewport,
        filament::Viewport const& srcViewport, CopyFrameFlag flags) {
    SYSTRACE_CALL();

    assert_invariant(mSwapChain);
    assert_invariant(dstSwapChain);
    FEngine& engine = getEngine();
    FEngine::DriverApi& driver = engine.getDriverApi();

    // Set the current swap chain as the read surface, and the destination
    // swap chain as the draw surface so that blitting between default render
    // targets results in a frame copy from the current frame to the
    // destination.
    driver.makeCurrent(dstSwapChain->getHwHandle(), mSwapChain->getHwHandle());

    RenderPassParams params = {};
    // Clear color to black if the CLEAR flag is set.
    if (flags & CLEAR) {
        params.clearColor = {0.f, 0.f, 0.f, 1.f};
        params.flags.clear = TargetBufferFlags::COLOR;
        params.flags.discardStart = TargetBufferFlags::ALL;
        params.flags.discardEnd = TargetBufferFlags::NONE;
        params.viewport.left = 0;
        params.viewport.bottom = 0;
        params.viewport.width = std::numeric_limits<uint32_t>::max();
        params.viewport.height = std::numeric_limits<uint32_t>::max();
    }
    driver.beginRenderPass(mRenderTarget, params);

    // Verify that the source swap chain is readable.
    assert_invariant(mSwapChain->isReadable());
    driver.blit(TargetBufferFlags::COLOR,
            mRenderTarget, dstViewport, mRenderTarget, srcViewport, SamplerMagFilter::LINEAR);
    if (flags & SET_PRESENTATION_TIME) {
        // TODO: Implement this properly, see https://github.com/google/filament/issues/633
    }

    driver.endRenderPass();

    if (flags & COMMIT) {
        dstSwapChain->commit(driver);
    }

    // Reset the context and read/draw surface to the current surface so that
    // frame rendering can continue or complete.
    mSwapChain->makeCurrent(driver);
}

bool FRenderer::beginFrame(FSwapChain* swapChain, uint64_t vsyncSteadyClockTimeNano) {
    assert_invariant(swapChain);

    SYSTRACE_CALL();

    // get the timestamp as soon as possible
    using namespace std::chrono;
    const steady_clock::time_point now{ steady_clock::now() };
    const steady_clock::time_point userVsync{ steady_clock::duration(vsyncSteadyClockTimeNano) };
    const time_point<steady_clock> appVsync(vsyncSteadyClockTimeNano ? userVsync : now);

    mFrameId++;

    { // scope for frame id trace
        char buf[64];
        snprintf(buf, 64, "frame %u", mFrameId);
        SYSTRACE_NAME(buf);
    }

    FEngine& engine = getEngine();
    FEngine::DriverApi& driver = engine.getDriverApi();

    // start a frame capture, if requested.
    if (UTILS_UNLIKELY(engine.debug.renderer.doFrameCapture)) {
        driver.startCapture();
    }

    // latch the frame time
    std::chrono::duration<double> time(appVsync - mUserEpoch);
    float h = float(time.count());
    float l = float(time.count() - h);
    mShaderUserTime = { h, l, 0, 0 };

    mPreviousRenderTargets.clear();

    mBeginFrameInternal = {};

    mSwapChain = swapChain;
    swapChain->makeCurrent(driver);

    // NOTE: this makes synchronous calls to the driver
    driver.updateStreams(&driver);

    // gives the backend a chance to execute periodic tasks
    driver.tick();

    /*
    * From this point, we can't do any more work in beginFrame() because the user could choose
    * to ignore the return value and render the frame anyways -- which is perfectly fine.
    * The remaining work will be done when the first render() call is made.
    */
    auto beginFrameInternal = [=]() {
        FEngine& engine = getEngine();
        FEngine::DriverApi& driver = engine.getDriverApi();

        driver.beginFrame(appVsync.time_since_epoch().count(), mFrameId);

        // This need to occur after the backend beginFrame() because some backends need to start
        // a command buffer before creating a fence.

        mFrameInfoManager.beginFrame(driver, {
                .historySize = mFrameRateOptions.history
        }, mFrameId);

        if (false && vsyncSteadyClockTimeNano) { // work in progress
            const size_t interval = mFrameRateOptions.interval; // user requested swap-interval;
            const steady_clock::duration refreshPeriod(uint64_t(1e9 / mDisplayInfo.refreshRate));
            const steady_clock::duration presentationDeadline(mDisplayInfo.presentationDeadlineNanos);
            const steady_clock::duration vsyncOffset(mDisplayInfo.vsyncOffsetNanos);

            // hardware vsync timestamp
            steady_clock::time_point hwVsync = appVsync - vsyncOffset;

            // compute our desired presentation time. We can't pick a desired presentation time
            // that's too far, or we won't be able to dequeue buffers.
            steady_clock::time_point desiredPresentationTime = hwVsync + 2 * interval * refreshPeriod;

            // Compute the deadline. This deadline is when the GPU must be finished.
            // The deadline has 1ms backed in it on Android.
            UTILS_UNUSED_IN_RELEASE
            steady_clock::time_point deadline = desiredPresentationTime - presentationDeadline;

            // one important thing is to make sure that the deadline is comfortably later than
            // when the gpu will finish, otherwise we'll have inconsistent latency/frames.

            // TODO: evaluate if we can make it in time, and if not why.
            //   If the problem is cpu+gpu latency we can try to push the desired presentation time
            //   further away, but this has limits, as only 2 buffers are dequeuable.
            //   If the problem is the gpu is overwhelmed, then we need to
            //    - see if there is more headroom in dynamic resolution
            //    - or start skipping frames. Ideally lower the framerate to
            // presentation time is set to the middle of the period we're interested in
            steady_clock::time_point presentationTime = desiredPresentationTime - refreshPeriod / 2;
            driver.setPresentationTime(presentationTime.time_since_epoch().count());
        }

        // ask the engine to do what it needs to (e.g. updates light buffer, materials...)
        engine.prepare();
    };

    if (mFrameSkipper.beginFrame(driver)) {
        // if beginFrame() returns true, we are expecting a call to endFrame(),
        // so do the beginFrame work right now, instead of requiring a call to render()
        beginFrameInternal();
        return true;
    }

    // however, if we return false, the user is allowed to ignore us and render a frame anyways,
    // so we need to delay this work until that happens.
    mBeginFrameInternal = beginFrameInternal;

    // we need to flush in this case, to make sure the tick() call is executed at some point
    engine.flush();

    return false;
}

void FRenderer::endFrame() {
    SYSTRACE_CALL();

    if (UTILS_UNLIKELY(mBeginFrameInternal)) {
        mBeginFrameInternal();
        mBeginFrameInternal = {};
    }

    FEngine& engine = getEngine();
    FEngine::DriverApi& driver = engine.getDriverApi();

    if (UTILS_HAS_THREADING) {
        // on debug builds this helps catching cases where we're writing to
        // the buffer form another thread, which is currently not allowed.
        driver.debugThreading();
    }

    mFrameInfoManager.endFrame(driver);
    mFrameSkipper.endFrame(driver);

    if (mSwapChain) {
        mSwapChain->commit(driver);
        mSwapChain = nullptr;
    }

    driver.endFrame(mFrameId);

    // gives the backend a chance to execute periodic tasks
    driver.tick();

    // stop the frame capture, if one was requested
    if (UTILS_UNLIKELY(engine.debug.renderer.doFrameCapture)) {
        driver.stopCapture();
        engine.debug.renderer.doFrameCapture = false;
    }

    // do this before engine.flush()
    engine.getResourceAllocator().gc();

    // Run the component managers' GC in parallel
    // WARNING: while doing this we can't access any component manager
    auto& js = engine.getJobSystem();

    auto *job = js.runAndRetain(jobs::createJob(js, nullptr, &FEngine::gc, &engine)); // gc all managers

    engine.flush();     // flush command stream

    // make sure we're done with the gcs
    js.waitAndRelease(job);
}

void FRenderer::readPixels(uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
        PixelBufferDescriptor&& buffer) {
#ifndef NDEBUG
    const bool withinFrame = mSwapChain != nullptr;
    ASSERT_PRECONDITION(withinFrame, "readPixels() on a SwapChain must be called after"
            " beginFrame() and before endFrame().");
#endif
    readPixels(mRenderTarget, xoffset, yoffset, width, height, std::move(buffer));
}

void FRenderer::readPixels(FRenderTarget* renderTarget,
        uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
        backend::PixelBufferDescriptor&& buffer) {
    readPixels(renderTarget->getHwHandle(), xoffset, yoffset, width, height, std::move(buffer));
}

void FRenderer::readPixels(Handle<HwRenderTarget> renderTargetHandle,
        uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
        backend::PixelBufferDescriptor&& buffer) {
    if (!ASSERT_POSTCONDITION_NON_FATAL(
            buffer.type != PixelDataType::COMPRESSED,
            "buffer.format cannot be COMPRESSED")) {
        return;
    }

    if (!ASSERT_POSTCONDITION_NON_FATAL(
            buffer.alignment > 0 && buffer.alignment <= 8 &&
            !(buffer.alignment & (buffer.alignment - 1u)),
            "buffer.alignment must be 1, 2, 4 or 8")) {
        return;
    }

    // It's not really possible to know here which formats will be supported because
    // it can vary depending on the RenderTarget, in GL the following are ALWAYS supported though:
    // format: RGBA, RGBA_INTEGER
    // type: UBYTE, UINT, INT, FLOAT

    const size_t sizeNeeded = PixelBufferDescriptor::computeDataSize(
            buffer.format, buffer.type,
            buffer.stride ? buffer.stride : width,
            buffer.top + height,
            buffer.alignment);

    if (!ASSERT_POSTCONDITION_NON_FATAL(buffer.size >= sizeNeeded,
            "Pixel buffer too small: has %u bytes, needs %u bytes", buffer.size, sizeNeeded)) {
        return;
    }

    FEngine& engine = getEngine();
    FEngine::DriverApi& driver = engine.getDriverApi();
    driver.readPixels(renderTargetHandle, xoffset, yoffset, width, height, std::move(buffer));
}

void FRenderer::getRenderTarget(FView const& view,
        TargetBufferFlags& outAttachementMask, Handle<HwRenderTarget>& outTarget) const noexcept {
    outTarget = view.getRenderTargetHandle();
    outAttachementMask = view.getRenderTargetAttachmentMask();
    if (!outTarget) {
        outTarget = mRenderTarget;
        outAttachementMask = TargetBufferFlags::COLOR0 | TargetBufferFlags::DEPTH;
    }
}

void FRenderer::initializeClearFlags() {
    // We always discard and clear the depth+stencil buffers -- we don't allow sharing these
    // across views (clear implies discard)
    mDiscardStartFlags = ((mClearOptions.discard || mClearOptions.clear) ?
                          TargetBufferFlags::COLOR : TargetBufferFlags::NONE)
                         | TargetBufferFlags::DEPTH_AND_STENCIL;

    mClearFlags = (mClearOptions.clear ? TargetBufferFlags::COLOR : TargetBufferFlags::NONE)
            | TargetBufferFlags::DEPTH_AND_STENCIL;
}


// ------------------------------------------------------------------------------------------------
// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

Engine* Renderer::getEngine() noexcept {
    return &upcast(this)->getEngine();
}

void Renderer::render(View const* view) {
    upcast(this)->render(upcast(view));
}

bool Renderer::beginFrame(SwapChain* swapChain, uint64_t vsyncSteadyClockTimeNano) {
    return upcast(this)->beginFrame(upcast(swapChain), vsyncSteadyClockTimeNano);
}

void Renderer::copyFrame(SwapChain* dstSwapChain, filament::Viewport const& dstViewport,
        filament::Viewport const& srcViewport, CopyFrameFlag flags) {
    upcast(this)->copyFrame(upcast(dstSwapChain), dstViewport, srcViewport, flags);
}

void Renderer::readPixels(uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
        PixelBufferDescriptor&& buffer) {
    upcast(this)->readPixels(xoffset, yoffset, width, height, std::move(buffer));
}

void Renderer::readPixels(RenderTarget* renderTarget,
        uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
        PixelBufferDescriptor&& buffer) {
    upcast(this)->readPixels(upcast(renderTarget),
            xoffset, yoffset, width, height, std::move(buffer));
}

void Renderer::endFrame() {
    upcast(this)->endFrame();
}

double Renderer::getUserTime() const {
    return upcast(this)->getUserTime().count();
}

void Renderer::resetUserTime() {
    upcast(this)->resetUserTime();
}

void Renderer::setDisplayInfo(const DisplayInfo& info) noexcept {
    upcast(this)->setDisplayInfo(info);
}

void Renderer::setFrameRateOptions(FrameRateOptions const& options) noexcept {
    upcast(this)->setFrameRateOptions(options);
}

void Renderer::setClearOptions(const ClearOptions& options) {
    upcast(this)->setClearOptions(options);
}

void Renderer::renderStandaloneView(View const* view) {
    upcast(this)->renderStandaloneView(upcast(view));
}

} // namespace filament
