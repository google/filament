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
#include "RendererUtils.h"
#include "RenderPass.h"
#include "ResourceAllocator.h"

#include "details/Engine.h"
#include "details/Fence.h"
#include "details/Scene.h"
#include "details/SwapChain.h"
#include "details/Texture.h"
#include "details/View.h"

#include <filament/Renderer.h>

#include <backend/PixelBufferDescriptor.h>

#include "fg/FrameGraph.h"
#include "fg/FrameGraphId.h"
#include "fg/FrameGraphResources.h"

#include <utils/compiler.h>
#include <utils/JobSystem.h>
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
        mRenderTargetHandle(engine.getDefaultRenderTarget()),
        mFrameInfoManager(engine.getDriverApi()),
        mHdrTranslucent(TextureFormat::RGBA16F),
        mHdrQualityMedium(TextureFormat::R11F_G11F_B10F),
        mHdrQualityHigh(TextureFormat::RGB16F),
        mIsRGB8Supported(false),
        mUserEpoch(engine.getEngineEpoch()),
        mPerRenderPassArena(engine.getPerRenderPassAllocator())
{
    FDebugRegistry& debugRegistry = engine.getDebugRegistry();
    debugRegistry.registerProperty("d.renderer.doFrameCapture",
            &engine.debug.renderer.doFrameCapture);

    DriverApi& driver = engine.getDriverApi();

    mIsRGB8Supported = driver.isRenderTargetFormatSupported(TextureFormat::RGB8);

    // our default HDR translucent format, fallback to LDR if not supported by the backend
    if (!driver.isRenderTargetFormatSupported(TextureFormat::RGBA16F)) {
        // this will clip all HDR data, but we don't have a choice
        mHdrTranslucent = TextureFormat::RGBA8;
    }

    // our default opaque low/medium quality HDR format, fallback to LDR if not supported
    if (!driver.isRenderTargetFormatSupported(mHdrQualityMedium)) {
        // this will clip all HDR data, but we don't have a choice
        mHdrQualityMedium = TextureFormat::RGB8;
    }

    // our default opaque high quality HDR format, fallback to RGBA, then medium, then LDR
    // if not supported
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
    size_t wmpct = wm / (mEngine.getPerFrameCommandsSize() / 100);
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

TextureFormat FRenderer::getHdrFormat(const FView& view, bool translucent) const noexcept {
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

void FRenderer::getRenderTarget(FView const& view,
        TargetBufferFlags& outAttachementMask, Handle<HwRenderTarget>& outTarget) const noexcept {
    outTarget = view.getRenderTargetHandle();
    outAttachementMask = view.getRenderTargetAttachmentMask();
    if (!outTarget) {
        outTarget = mRenderTargetHandle;
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

void FRenderer::setPresentationTime(int64_t monotonic_clock_ns) {
    FEngine::DriverApi& driver = mEngine.getDriverApi();
    driver.setPresentationTime(monotonic_clock_ns);
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
    mViewRenderedCount = 0;

    { // scope for frame id trace
        char buf[64];
        snprintf(buf, 64, "frame %u", mFrameId);
        SYSTRACE_NAME(buf);
    }

    FEngine& engine = mEngine;
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
    auto beginFrameInternal = [this, appVsync]() {
        FEngine& engine = mEngine;
        FEngine::DriverApi& driver = engine.getDriverApi();

        driver.beginFrame(appVsync.time_since_epoch().count(), mFrameId);

        // This need to occur after the backend beginFrame() because some backends need to start
        // a command buffer before creating a fence.

        mFrameInfoManager.beginFrame(driver, {
                .historySize = mFrameRateOptions.history
        }, mFrameId);

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

    FEngine& engine = mEngine;
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
    RendererUtils::readPixels(mEngine.getDriverApi(), mRenderTargetHandle,
            xoffset, yoffset, width, height, std::move(buffer));
}

void FRenderer::readPixels(FRenderTarget* renderTarget,
        uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
        backend::PixelBufferDescriptor&& buffer) {
    RendererUtils::readPixels(mEngine.getDriverApi(), renderTarget->getHwHandle(),
            xoffset, yoffset, width, height, std::move(buffer));
}

void FRenderer::copyFrame(FSwapChain* dstSwapChain, filament::Viewport const& dstViewport,
        filament::Viewport const& srcViewport, CopyFrameFlag flags) {
    SYSTRACE_CALL();

    assert_invariant(mSwapChain);
    assert_invariant(dstSwapChain);
    FEngine& engine = mEngine;
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
    driver.beginRenderPass(mRenderTargetHandle, params);

    // Verify that the source swap chain is readable.
    assert_invariant(mSwapChain->isReadable());
    driver.blit(TargetBufferFlags::COLOR,
            mRenderTargetHandle, dstViewport, mRenderTargetHandle, srcViewport, SamplerMagFilter::LINEAR);
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

void FRenderer::renderStandaloneView(FView const* view) {
    SYSTRACE_CALL();

    using namespace std::chrono;

    ASSERT_PRECONDITION(view->getRenderTarget(),
            "View \"%s\" must have a RenderTarget associated", view->getName());

    if (UTILS_LIKELY(view->getScene())) {
        mPreviousRenderTargets.clear();
        mFrameId++;

        // ask the engine to do what it needs to (e.g. updates light buffer, materials...)
        FEngine& engine = mEngine;
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

    if (UTILS_UNLIKELY(mBeginFrameInternal)) {
        // this should not happen, the user should not call render() if we returned false from
        // beginFrame(). But because this is allowed, we handle it gracefully.
        mBeginFrameInternal();
        mBeginFrameInternal = {};
    }

    if (UTILS_LIKELY(view && view->getScene())) {
        if (mViewRenderedCount) {
            // this is a good place to kick the GPU, since we've rendered a View before
            // and we're about to render another one.
            mEngine.getDriverApi().flush();
        }
        renderInternal(view);
        mViewRenderedCount++;
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
    FEngine& engine = mEngine;
    JobSystem& js = engine.getJobSystem();
    FEngine::DriverApi& driver = engine.getDriverApi();
    PostProcessManager& ppm = engine.getPostProcessManager();

    // DEBUG: driver commands must all happen from the same thread. Enforce that on debug builds.
    driver.debugThreading();

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
    auto guardBandOptions = view.getGuardBandOptions();
    const uint8_t msaaSampleCount = msaaOptions.enabled ? msaaOptions.sampleCount : 1u;
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

    const bool blendModeTranslucent = view.getBlendMode() == BlendMode::TRANSLUCENT;
    // If the swap-chain is transparent or if we blend into it, we need to allocate our intermediate
    // buffers with an alpha channel.
    const bool needsAlphaChannel =
            (mSwapChain && mSwapChain->isTransparent()) || blendModeTranslucent;

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
            .ldrFormat = (hasColorGrading && hasFXAA) ?
                    TextureFormat::RGBA8 : getLdrFormat(needsAlphaChannel)
    };

    // whether we're scaled at all
    const bool scaled = any(notEqual(scale, float2(1.0f)));

    // vp is the user defined viewport within the View
    filament::Viewport const& vp = view.getViewport();

    // svp is the "rendering" viewport. That is the viewport after dynamic-resolution is applied
    // as well as other adjustment, such as the guard band.
    filament::Viewport svp = {
            0, 0, // this is ignored
            uint32_t(float(vp.width ) * scale.x),
            uint32_t(float(vp.height) * scale.y)
    };
    if (svp.empty()) {
        return;
    }

    // xvp is the viewport relative to svp containing the "interesting" rendering
    filament::Viewport xvp = svp;

    CameraInfo cameraInfo = view.computeCameraInfo(engine);

    // when colorgrading-as-subpass is active, we know that many other effects are disabled
    // such as dof, bloom. Moreover, if fxaa and scaling are not enabled, we're essentially in
    // a very fast rendering path -- in this case, we would need an extra blit to "resolve" the
    // buffer padding (because there are no other pass that can do it as a side effect).
    // In this case, it is better to skip the padding, which won't be helping much.
    const bool noBufferPadding = colorGradingConfig.asSubpass && !hasFXAA && !scaled;

    // guardBand must be a multiple of 16 to guarantee the same exact rendering up to 4 mip levels.
    float guardBand = guardBandOptions.enabled ? 16.0f : 0.0f;

    if (hasPostProcess && !noBufferPadding) {
        // We always pad the rendering viewport to dimensions multiple of 16, this guarantees
        // that up to 4 mipmap levels are possible with an exact 1:2 scale. This also helps
        // with memory allocations and quad-shading when dynamic-resolution is enabled.
        // There is a small performance cost for dimensions that are not already multiple of 16.
        // But, this a no-op in common resolutions, in particular in 720p.
        // The origin of rendering is not modified, the padding is added to the right/top.
        //
        // TODO: Should we enable when we don't have post-processing?
        //       Without post-processing, we usually draw directly into
        //       the SwapChain, and we might want to keep it this way.

        auto round = [](uint32_t x) {
            constexpr uint32_t rounding = 16u;
            return (x + (rounding - 1u)) & ~(rounding - 1u);
        };

        // compute the new rendering width and height, multiple of 16.
        const float width  = float(round(svp.width )) + 2.0f * guardBand;
        const float height = float(round(svp.height)) + 2.0f * guardBand;

        // scale the field-of-view up, so it covers exactly the extra pixels
        const float3 clipSpaceScaling{
                float(svp.width)  / width,
                float(svp.height) / height,
                1.0f };

        // add the extra-pixels on the right/top of the viewport
        // the translation comes from (same for height): 2*((width - svp.width)/2) / width
        // i.e. we offset by half the width/height delta and the 2* comes from the fact that
        // clip-space has width/height of 2.
        // note: this creates an asymmetric frustum -- but we eventually copy only the
        // left/bottom part, which is a symmetric region.
        const float2 clipSpaceTranslation{
                1.0f - clipSpaceScaling.x - 2.0f * guardBand / width,
                1.0f - clipSpaceScaling.y - 2.0f * guardBand / height
        };

        mat4f ts = mat4f::scaling(clipSpaceScaling);
        ts[3].xy = -clipSpaceTranslation;

        // update the camera projection
        cameraInfo.projection = highPrecisionMultiply(ts, cameraInfo.projection);

        // VERTEX_DOMAIN_DEVICE doesn't apply the projection, but it still needs this
        // clip transform, so we apply it separately (see main.vs)
        cameraInfo.clipTransfrom = { ts[0][0], ts[1][1], ts[3].x, ts[3].y };

        // adjust svp to the new, larger, rendering dimensions
        svp.width  = uint32_t(width);
        svp.height = uint32_t(height);
        xvp.left   = int32_t(guardBand);
        xvp.bottom = int32_t(guardBand);
    }

    view.prepare(engine, driver, arena, svp, cameraInfo, getShaderUserTime(), needsAlphaChannel);

    view.prepareUpscaler(scale);

    // start froxelization immediately, it has no dependencies
    JobSystem::Job* jobFroxelize = nullptr;
    if (view.hasDynamicLighting()) {
        jobFroxelize = js.runAndRetain(js.createJob(nullptr,
                [&engine, &view, &viewMatrix = cameraInfo.view](JobSystem&, JobSystem::Job*) {
                    view.froxelize(engine, viewMatrix); }));
    }

    /*
     * Allocate command buffer
     */

    FScene& scene = *view.getScene();

    // Allocate some space for our commands in the per-frame Arena, and use that space as
    // an Arena for commands. All this space is released when we exit this method.
    size_t perFrameCommandsSize = engine.getPerFrameCommandsSize();
    void* const arenaBegin = arena.allocate(perFrameCommandsSize, CACHELINE_SIZE);
    void* const arenaEnd = pointermath::add(arenaBegin, perFrameCommandsSize);
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
        auto shadows = view.renderShadowMaps(fg, engine, driver, shadowPass);
        blackboard["shadows"] = shadows;
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
    const uint8_t clearStencil = mClearOptions.clearStencil;
    const TargetBufferFlags clearFlags = mClearFlags;
    const TargetBufferFlags discardStartFlags = mDiscardStartFlags;
    TargetBufferFlags keepOverrideStartFlags = TargetBufferFlags::ALL & ~discardStartFlags;
    TargetBufferFlags keepOverrideEndFlags = TargetBufferFlags::NONE;

    if (currentRenderTarget) {
        // For custom RenderTarget, we look at each attachment flag and if they have their
        // SAMPLEABLE usage bit set, we assume they must not be discarded after the render pass.
        keepOverrideEndFlags |= currentRenderTarget->getSampleableAttachmentsMask();
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

    const bool blending = blendModeTranslucent;
    const TextureFormat hdrFormat = getHdrFormat(view, needsAlphaChannel);

    RendererUtils::ColorPassConfig config{
            .width = svp.width,
            .height = svp.height,
            .xoffset = (uint32_t)xvp.left,
            .yoffset = (uint32_t)xvp.bottom,
            .scale = scale,
            .hdrFormat = hdrFormat,
            .msaa = msaaSampleCount,
            .clearFlags = clearFlags,
            .clearColor = clearColor,
            .clearStencil = clearStencil,
            .ssrLodOffset = 0.0f,
            .hasContactShadows = scene.hasContactShadows(),
            // at this point we don't know if we have refraction, but that's handled later
            .hasScreenSpaceReflectionsOrRefractions = ssReflectionsOptions.enabled,
            .enabledStencilBuffer = view.isStencilBufferEnabled()
    };

    /*
     * Depth + Color passes
     */

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

                // The code here is a little fragile. In theory, we need to call prepareViewport()
                // for each render pass, because the viewport parameters depend on the resolution.
                // However, in practice, we only have two resolutions: the color pass resolution,
                // and the structure pass which is governed by aoOptions.resolution (this could
                // change in the future).
                // So here we set the parameters for the structure pass and SSAO passes which
                // are always done first. The SSR pass will also use these parameters which
                // is wrong if it doesn't run at the same resolution as SSAO.
                // prepareViewport() is called again during the color pass, which resets the
                // values correctly for the Color pass, however, this will be again wrong
                // for passes that come after the Color pass, such as DoF.
                //
                // The solution is to call prepareViewport() for each pass, really (so we should
                // move it to its own UBO).
                //
                // The reason why this bug is acceptable is that the viewport parameters are
                // currently only used for generating noise, so it's not too bad.

                uniforms.prepareViewport(svp,
                        xvp.left   * aoOptions.resolution,
                        xvp.bottom * aoOptions.resolution);

                uniforms.commit(driver);
            });

    // --------------------------------------------------------------------------------------------
    // structure pass -- automatically culled if not used
    // Currently it consists of a simple depth pass.
    // This is normally used by SSAO and contact-shadows

    // TODO: the scaling should depends on all passes that need the structure pass
    const auto [structure, picking_] = ppm.structure(fg, pass, renderFlags, svp.width, svp.height, {
            .scale = aoOptions.resolution,
            .picking = view.hasPicking()
    });
    blackboard["structure"] = structure;
    const auto picking = picking_;


    if (view.hasPicking()) {
        struct PickingResolvePassData {
            FrameGraphId<FrameGraphTexture> picking;
        };
        fg.addPass<PickingResolvePassData>("Picking Resolve Pass",
                [&](FrameGraph::Builder& builder, auto& data) {
                    data.picking = builder.read(picking,
                            FrameGraphTexture::Usage::COLOR_ATTACHMENT);

                    FrameGraphRenderPass::Descriptor descr;
                    descr.attachments.content.color[0] = data.picking;
                    builder.declareRenderPass("Picking Resolve Target", descr);
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
        // Apply the TAA jitter to everything after the structure pass, starting with the color pass.
        ppm.prepareTaa(fg, svp, view.getFrameHistory(), &FrameHistoryEntry::taa,
                &cameraInfo, view.getPerViewUniforms());
    }

    // --------------------------------------------------------------------------------------------
    // SSAO pass

    if (aoOptions.enabled) {
        // we could rely on FrameGraph culling, but this creates unnecessary CPU work
        auto ssao = ppm.screenSpaceAmbientOcclusion(fg, svp, cameraInfo, structure, aoOptions);
        blackboard["ssao"] = ssao;
    }

    // --------------------------------------------------------------------------------------------
    // prepare screen-space reflection/refraction passes

    PostProcessManager::ScreenSpaceRefConfig ssrConfig = PostProcessManager::prepareMipmapSSR(
            fg, svp.width, svp.height,
            ssReflectionsOptions.enabled ? TextureFormat::RGBA16F : TextureFormat::R11F_G11F_B10F,
            view.getCameraUser().getFieldOfView(Camera::Fov::VERTICAL), config.scale);
    config.ssrLodOffset = ssrConfig.lodOffset;
    blackboard["ssr"] = ssrConfig.ssr;

    // --------------------------------------------------------------------------------------------
    // screen-space reflections pass

    if (ssReflectionsOptions.enabled) {
        auto reflections = ppm.ssr(fg, pass,
                view.getFrameHistory(), cameraInfo,
                view.getPerViewUniforms(),
                structure,
                ssReflectionsOptions,
                { .width = svp.width, .height = svp.height });

        // generate the mipchain
        reflections = PostProcessManager::generateMipmapSSR(ppm, fg,
                reflections, ssrConfig.reflection, false, ssrConfig);
    }

    // --------------------------------------------------------------------------------------------
    // Color passes

    // This one doesn't need to be a FrameGraph pass because it always happens by construction
    // (i.e. it won't be culled, unless everything is culled), so no need to complexify things.
    pass.setVariant(variant);
    pass.appendCommands(RenderPass::COLOR);
    pass.sortCommands();

    FrameGraphTexture::Descriptor desc = {
            .width = config.width,
            .height = config.height,
            .format = config.hdrFormat
    };

    // a non-drawing pass to prepare everything that need to be before the color passes execute
    fg.addTrivialSideEffectPass("Prepare Color Passes",
            [=, &js, &view, &ppm](DriverApi& driver) {
                // prepare color grading as subpass material
                if (colorGradingConfig.asSubpass) {
                    ppm.colorGradingPrepareSubpass(driver,
                            colorGrading, colorGradingConfig, vignetteOptions,
                            config.width, config.height);
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

    // this makes the viewport relative to xvp
    // FIXME: we should use 'vp' when rendering directly into the swapchain, but that's hard to
    //        know at this point. This will usually be the case when post-process is disabled.
    // FIXME: we probably should take the dynamic scaling into account too
    pass.setScissorViewport(hasPostProcess ? xvp : vp);

    // the color pass itself + color-grading as subpass if needed
    auto colorPassOutput = RendererUtils::colorPass(fg, "Color Pass", mEngine, view,
            desc, config, colorGradingConfigForColor, pass.getExecutor());

    if (view.isScreenSpaceRefractionEnabled() && !pass.empty()) {
        // this cancels the colorPass() call above if refraction is active.
        // the color pass + refraction + color-grading as subpass if needed
        colorPassOutput = RendererUtils::refractionPass(fg, mEngine, view,
                config, ssrConfig, colorGradingConfigForColor, pass);
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
        // FIXME: should we use the TAA-modified cameraInfo here or not? (we are).
        auto projection = mat4f{ cameraInfo.projection * cameraInfo.getUserViewMatrix() };
        fg.addPass<ExportSSRHistoryData>("Export SSR history",
                [&](FrameGraph::Builder& builder, auto& data) {
                    // We need to use sideEffect here to ensure this pass won't be culled.
                    // The "output" of this pass is going to be used during the next frame as
                    // an "import".
                    builder.sideEffect();
                    data.history = builder.sample(colorPassOutput); // FIXME: an access must be declared for detach(), why?
                }, [&view, projection](FrameGraphResources const& resources, auto const& data,
                        backend::DriverApi&) {
                    auto& history = view.getFrameHistory();
                    auto& current = history.getCurrent();
                    current.ssr.projection = projection;
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
    auto const depth = ppm.resolveBaseLevel(fg, "Resolved Depth Buffer",
            blackboard.get<FrameGraphTexture>("depth"));

    // TODO: DoF should be applied here, before TAA -- but if we do this it'll result in a lot of
    //       fireflies due to the instability of the highlights. This can be fixed with a
    //       dedicated TAA pass for the DoF, as explained in
    //       "Life of a Bokeh" by Guillaume Abadie, SIGGRAPH 2018

    // TAA for color pass
    if (taaOptions.enabled) {
        input = ppm.taa(fg, input, depth, view.getFrameHistory(), &FrameHistoryEntry::taa,
                taaOptions, colorGradingConfig);
    }

    // --------------------------------------------------------------------------------------------
    // Post Processing...

    UTILS_UNUSED_IN_RELEASE auto const& inputDesc = fg.getDescriptor(input);
    assert_invariant(inputDesc.width == svp.width);
    assert_invariant(inputDesc.height == svp.height);

    bool mightNeedFinalBlit = true;
    if (hasPostProcess) {
        if (dofOptions.enabled) {
            // The bokeh height is always correct regardless of the dynamic resolution scaling.
            // (because the CoC is calculated w.r.t. the height), so we only need to adjust
            // the width.
            float bokehAspectRatio = scale.x / scale.y;
            input = ppm.dof(fg, input, depth, cameraInfo, needsAlphaChannel,
                    bokehAspectRatio, dofOptions);
        }

        FrameGraphId<FrameGraphTexture> bloom, flare;
        if (bloomOptions.enabled) {
            // generate the bloom buffer, which is stored in the blackboard as "bloom". This is
            // consumed by the colorGrading pass and will be culled if colorGrading is disabled.
            auto [bloom_, flare_] = ppm.bloom(fg, input, bloomOptions, TextureFormat::R11F_G11F_B10F, scale);
            bloom = bloom_;
            flare = flare_;
        }

        if (hasColorGrading) {
            if (!colorGradingConfig.asSubpass) {
                input = ppm.colorGrading(fg, input, xvp,
                        bloom, flare,
                        colorGrading, colorGradingConfig,
                        bloomOptions, vignetteOptions);
                // the padded buffer is resolved now
                xvp.left = xvp.bottom = 0;
                svp = xvp;
            }
        }

        if (hasFXAA) {
            input = ppm.fxaa(fg, input, xvp, colorGradingConfig.ldrFormat,
                    !hasColorGrading || needsAlphaChannel);
            // the padded buffer is resolved now
            xvp.left = xvp.bottom = 0;
            svp = xvp;
        }
        if (scaled) {
            mightNeedFinalBlit = false;
            auto viewport = DEBUG_DYNAMIC_SCALING ? xvp : vp;
            input = ppm.upscale(fg, blending, dsrOptions, input, xvp, {
                    .width = viewport.width, .height = viewport.height,
                    .format = colorGradingConfig.ldrFormat });
            xvp.left = xvp.bottom = 0;
            svp = xvp;
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
    // * And we also can't use the default rendertarget if frame history is needed (e.g. with
    //   screen-space reflections)
    // * We also need an extra blit if we haven't yet handled "xvp"
    //   TODO: in that specific scenario it would be better to just not use xvp
    // The intermediate buffer is accomplished with a "fake" opaqueBlit (i.e. blit) operation.

    const bool outputIsSwapChain = (input == colorPassOutput) && (viewRenderTarget == mRenderTargetHandle);
    if (mightNeedFinalBlit) {
        if (blending ||
            xvp != svp ||
            (outputIsSwapChain &&
                    (msaaSampleCount > 1 ||
                    colorGradingConfig.asSubpass ||
                    ssReflectionsOptions.enabled))) {
            assert_invariant(!scaled);
            input = ppm.blit(fg, blending, input, xvp, {
                    .width = vp.width, .height = vp.height,
                    .format = colorGradingConfig.ldrFormat }, SamplerMagFilter::NEAREST);
        }
    }

//    auto debug = structure
//    fg.forwardResource(fgViewRenderTarget, debug ? debug : input);

    fg.forwardResource(fgViewRenderTarget, input);

    fg.present(fgViewRenderTarget);

    fg.compile();

    //fg.export_graphviz(slog.d, view.getName());

    fg.execute(driver);

    // save the current history entry and destroy the oldest entry
    view.commitFrameHistory(engine);

    recordHighWatermark(commandArena.getListener().getHighWatermark());
}

} // namespace filament
