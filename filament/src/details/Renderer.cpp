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

#include "Allocators.h"
#include "DebugRegistry.h"
#include "FrameHistory.h"
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

#include <filament/Camera.h>
#include <filament/Fence.h>
#include <filament/Options.h>
#include <filament/Renderer.h>

#include <backend/DriverEnums.h>
#include <backend/DriverApiForward.h>
#include <backend/Handle.h>
#include <backend/PixelBufferDescriptor.h>

#include "fg/FrameGraph.h"
#include "fg/FrameGraphId.h"
#include "fg/FrameGraphResources.h"
#include "fg/FrameGraphTexture.h"

#include <math/vec2.h>
#include <math/vec3.h>
#include <math/mat4.h>

#include <utils/compiler.h>
#include <utils/JobSystem.h>
#include <utils/Log.h>
#include <utils/ostream.h>
#include <utils/Panic.h>
#include <utils/Systrace.h>
#include <utils/debug.h>

#include <chrono>
#include <limits>
#include <memory>
#include <utility>

#include <stddef.h>
#include <stdint.h>

#ifdef __ANDROID__
#include <sys/system_properties.h>
#endif

// this helps visualize what dynamic-scaling is doing
#define DEBUG_DYNAMIC_SCALING false

using namespace filament::math;
using namespace utils;

namespace filament {

using namespace backend;

FRenderer::FRenderer(FEngine& engine) :
        mEngine(engine),
        mFrameSkipper(),
        mRenderTargetHandle(engine.getDefaultRenderTarget()),
        mFrameInfoManager(engine.getDriverApi()),
        mHdrTranslucent(TextureFormat::RGBA16F),
        mHdrQualityMedium(TextureFormat::R11F_G11F_B10F),
        mHdrQualityHigh(TextureFormat::RGB16F),
        mIsRGB8Supported(false),
        mUserEpoch(engine.getEngineEpoch()),
        mResourceAllocator(std::make_unique<ResourceAllocator>(
                engine.getSharedResourceAllocatorDisposer(),
                engine.getConfig(),
                engine.getDriverApi()))
{
    FDebugRegistry& debugRegistry = engine.getDebugRegistry();
    debugRegistry.registerProperty("d.renderer.doFrameCapture",
            &engine.debug.renderer.doFrameCapture);
    debugRegistry.registerProperty("d.renderer.disable_buffer_padding",
            &engine.debug.renderer.disable_buffer_padding);
    debugRegistry.registerProperty("d.renderer.disable_subpasses",
            &engine.debug.renderer.disable_subpasses);
    debugRegistry.registerProperty("d.shadowmap.display_shadow_texture",
            &engine.debug.shadowmap.display_shadow_texture);
    debugRegistry.registerProperty("d.shadowmap.display_shadow_texture_scale",
            &engine.debug.shadowmap.display_shadow_texture_scale);
    debugRegistry.registerProperty("d.shadowmap.display_shadow_texture_layer",
            &engine.debug.shadowmap.display_shadow_texture_layer);
    debugRegistry.registerProperty("d.shadowmap.display_shadow_texture_level",
            &engine.debug.shadowmap.display_shadow_texture_level);
    debugRegistry.registerProperty("d.shadowmap.display_shadow_texture_channel",
            &engine.debug.shadowmap.display_shadow_texture_channel);
    debugRegistry.registerProperty("d.shadowmap.display_shadow_texture_layer_count",
            &engine.debug.shadowmap.display_shadow_texture_layer_count);
    debugRegistry.registerProperty("d.shadowmap.display_shadow_texture_level_count",
            &engine.debug.shadowmap.display_shadow_texture_level_count);
    debugRegistry.registerProperty("d.shadowmap.display_shadow_texture_power",
            &engine.debug.shadowmap.display_shadow_texture_power);
    debugRegistry.registerProperty("d.stereo.combine_multiview_images",
        &engine.debug.stereo.combine_multiview_images);

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
    size_t const wm = getCommandsHighWatermark();
    size_t const wmpct = wm / (mEngine.getPerFrameCommandsSize() / 100);
    slog.d << "Renderer: Commands High watermark "
    << wm / 1024 << " KiB (" << wmpct << "%), "
    << wm / sizeof(Command) << " commands, " << sizeof(Command) << " bytes/command"
    << io::endl;
#endif
}

void FRenderer::terminate(FEngine& engine) {
    // Here we would cleanly free resources we've allocated, or we own, in particular we would
    // shut down threads if we created any.
    DriverApi& driver = engine.getDriverApi();

    // Before we can destroy this Renderer's resources, we must make sure
    // that all pending commands have been executed (as they could reference data in this
    // instance, e.g. Fences, Callbacks, etc...)
    if (UTILS_HAS_THREADING) {
        Fence::waitAndDestroy(engine.createFence());
    } else {
        // In single threaded mode, allow recently-created objects (e.g. no-op fences in Skipper)
        // to initialize themselves, otherwise the engine tries to destroy invalid handles.
        engine.execute();
    }
    mFrameInfoManager.terminate(driver);
    mFrameSkipper.terminate(driver);
    mResourceAllocator->terminate();
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

std::pair<Handle<HwRenderTarget>, TargetBufferFlags>
        FRenderer::getRenderTarget(FView const& view) const noexcept {
    Handle<HwRenderTarget> outTarget = view.getRenderTargetHandle();
    TargetBufferFlags outAttachmentMask = view.getRenderTargetAttachmentMask();
    if (!outTarget) {
        outTarget = mRenderTargetHandle;
        outAttachmentMask = TargetBufferFlags::COLOR0 | TargetBufferFlags::DEPTH;
    }
    return { outTarget, outAttachmentMask };
}

backend::TargetBufferFlags FRenderer::getClearFlags() const noexcept {
    return (mClearOptions.clear ? TargetBufferFlags::COLOR : TargetBufferFlags::NONE)
           | TargetBufferFlags::DEPTH_AND_STENCIL;
}

void FRenderer::initializeClearFlags() noexcept {
    // We always discard and clear the depth+stencil buffers -- we don't allow sharing these
    // across views (clear implies discard)
    mDiscardStartFlags = ((mClearOptions.discard || mClearOptions.clear) ?
                          TargetBufferFlags::COLOR : TargetBufferFlags::NONE)
                         | TargetBufferFlags::DEPTH_AND_STENCIL;

    mClearFlags = getClearFlags();
}

void FRenderer::setPresentationTime(int64_t monotonic_clock_ns) {
    FEngine::DriverApi& driver = mEngine.getDriverApi();
    driver.setPresentationTime(monotonic_clock_ns);
}

void FRenderer::setVsyncTime(uint64_t steadyClockTimeNano) noexcept {
    mVsyncSteadyClockTimeNano = steadyClockTimeNano;
}

void FRenderer::skipFrame(uint64_t vsyncSteadyClockTimeNano) {
    SYSTRACE_CALL();

    FILAMENT_CHECK_PRECONDITION(!mSwapChain) <<
            "skipFrame() can't be called between beginFrame() and endFrame()";

    if (!vsyncSteadyClockTimeNano) {
        vsyncSteadyClockTimeNano = mVsyncSteadyClockTimeNano;
        mVsyncSteadyClockTimeNano = 0;
    }

    FEngine& engine = mEngine;
    FEngine::DriverApi& driver = engine.getDriverApi();

    // Gives the backend a chance to execute periodic tasks. This must be called before
    // the frame skipper.
    driver.tick();

    // do this before engine.flush()
    mResourceAllocator->gc(true);

    // Run the component managers' GC in parallel
    // WARNING: while doing this we can't access any component manager
    auto& js = engine.getJobSystem();

    auto *job = js.runAndRetain(jobs::createJob(js, nullptr, &FEngine::gc, &engine)); // gc all managers

    engine.flush();     // flush command stream

    // make sure we're done with the gcs
    js.waitAndRelease(job);
}

bool FRenderer::beginFrame(FSwapChain* swapChain, uint64_t vsyncSteadyClockTimeNano) {
    assert_invariant(swapChain);

    SYSTRACE_CALL();

#if 0 && defined(__ANDROID__)
    char scratch[PROP_VALUE_MAX + 1];
    int length = __system_property_get("debug.filament.protected", scratch);
    if (swapChain && length > 0) {
        uint64_t flags = swapChain->getFlags();
        bool value = bool(atoi(scratch));
        if (value) {
            flags |= SwapChain::CONFIG_PROTECTED_CONTENT;
        } else {
            flags &= ~SwapChain::CONFIG_PROTECTED_CONTENT;
        }
        swapChain->recreateWithNewFlags(mEngine, flags);
    }
#endif

    if (!vsyncSteadyClockTimeNano) {
        vsyncSteadyClockTimeNano = mVsyncSteadyClockTimeNano;
        mVsyncSteadyClockTimeNano = 0;
    }

    // get the timestamp as soon as possible
    using namespace std::chrono;
    const steady_clock::time_point now{ steady_clock::now() };
    const steady_clock::time_point userVsync{ steady_clock::duration(vsyncSteadyClockTimeNano) };
    const time_point<steady_clock> appVsync(vsyncSteadyClockTimeNano ? userVsync : now);

    mFrameId++;
    mViewRenderedCount = 0;

    SYSTRACE_FRAME_ID(mFrameId);

    FEngine& engine = mEngine;
    FEngine::DriverApi& driver = engine.getDriverApi();

    // start a frame capture, if requested.
    if (UTILS_UNLIKELY(engine.debug.renderer.doFrameCapture)) {
        driver.startCapture();
    }

    // latch the frame time
    std::chrono::duration<double> const time(appVsync - mUserEpoch);
    float const h = float(time.count());
    float const l = float(time.count() - h);
    mShaderUserTime = { h, l, 0, 0 };

    mPreviousRenderTargets.clear();

    mBeginFrameInternal = {};

    mSwapChain = swapChain;
    swapChain->makeCurrent(driver);

    // NOTE: this makes synchronous calls to the driver
    driver.updateStreams(&driver);

    // Gives the backend a chance to execute periodic tasks. This must be called before
    // the frame skipper.
    driver.tick();

    /*
    * From this point, we can't do any more work in beginFrame() because the user could choose
    * to ignore the return value and render the frame anyway -- which is perfectly fine.
    * The remaining work will be done when the first render() call is made.
    */
    auto beginFrameInternal = [this, appVsync, swapChain]() {
        FEngine& engine = mEngine;
        FEngine::DriverApi& driver = engine.getDriverApi();

        // we need to re-set mSwapChain here because if a frame was marked as "skip" but the
        // user ignored us, we'd still want mSwapChain to be set.
        mSwapChain = swapChain;

        driver.beginFrame(
                appVsync.time_since_epoch().count(),
                mDisplayInfo.refreshRate == 0.0 ? 0 : int64_t(
                        1'000'000'000.0 / mDisplayInfo.refreshRate),
                mFrameId);

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

    // however, if we return false, the user is allowed to ignore us and render a frame anyway,
    // so we need to delay this work until that happens.
    mBeginFrameInternal = beginFrameInternal;

    // clear mSwapChain because the frame should be skipped (it will be re-set if the user
    // ignores the skip)
    mSwapChain = nullptr;

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
        // on debug builds this helps to catch cases where we're writing to
        // the buffer form another thread, which is currently not allowed.
        driver.debugThreading();
    }

    if (mSwapChain) {
        mSwapChain->commit(driver);
        mSwapChain = nullptr;
    }

    mFrameInfoManager.endFrame(driver);
    mFrameSkipper.endFrame(driver);

    driver.endFrame(mFrameId);

    // gives the backend a chance to execute periodic tasks
    driver.tick();

    // stop the frame capture, if one was requested
    if (UTILS_UNLIKELY(engine.debug.renderer.doFrameCapture)) {
        driver.stopCapture();
        engine.debug.renderer.doFrameCapture = false;
    }

    // do this before engine.flush()
    mResourceAllocator->gc();

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
    FILAMENT_CHECK_PRECONDITION(withinFrame) << "readPixels() on a SwapChain must be called after"
                                                " beginFrame() and before endFrame().";
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
        driver.beginRenderPass(mRenderTargetHandle, params);
        driver.endRenderPass();
    }

    // Verify that the source swap chain is readable.
    assert_invariant(mSwapChain->isReadable());
    driver.blitDEPRECATED(TargetBufferFlags::COLOR, mRenderTargetHandle,
            dstViewport, mRenderTargetHandle, srcViewport, SamplerMagFilter::LINEAR);

    if (flags & SET_PRESENTATION_TIME) {
        // TODO: Implement this properly, see https://github.com/google/filament/issues/633
    }

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

    FILAMENT_CHECK_PRECONDITION(view->getRenderTarget())
            << "View \"" << view->getName() << "\" must have a RenderTarget associated";

    FILAMENT_CHECK_PRECONDITION(!mSwapChain)
            << "renderStandaloneView() must be called outside of beginFrame() / endFrame()";

    if (UTILS_LIKELY(view->getScene())) {
        mPreviousRenderTargets.clear();
        mFrameId++;

        // ask the engine to do what it needs to (e.g. updates light buffer, materials...)
        FEngine& engine = mEngine;
        engine.prepare();

        FEngine::DriverApi& driver = engine.getDriverApi();
        driver.beginFrame(
                steady_clock::now().time_since_epoch().count(),
                mDisplayInfo.refreshRate == 0.0 ? 0 : int64_t(
                        1'000'000'000.0 / mDisplayInfo.refreshRate),
                mFrameId);

        renderInternal(view);

        driver.endFrame(mFrameId);
    }
}

void FRenderer::render(FView const* view) {
    SYSTRACE_CALL();

    if (UTILS_UNLIKELY(mBeginFrameInternal)) {
        // this should not happen, the user should not call render() if we returned false from
        // beginFrame(). But because this is allowed, we handle it gracefully.
        mBeginFrameInternal();
        mBeginFrameInternal = {};
    }

    // after beginFrame() is called, mSwapChain should be true
    assert_invariant(mSwapChain);

    if (UTILS_LIKELY(view && view->getScene() && view->hasCamera())) {
        if (mViewRenderedCount) {
            // This is a good place to kick the GPU, since we've rendered a View before,
            // and we're about to render another one.
            mEngine.getDriverApi().flush();
        }
        renderInternal(view);
        mViewRenderedCount++;
    }
}

void FRenderer::renderInternal(FView const* view) {
    FEngine& engine = mEngine;

    // per-renderpass data
    RootArenaScope rootArenaScope(engine.getPerRenderPassArena());

    // create a root job so no other job can escape
    JobSystem& js = engine.getJobSystem();
    auto *rootJob = js.setRootJob(js.createJob());

    // execute the render pass
    renderJob(rootArenaScope, const_cast<FView&>(*view));

    // make sure to flush the command buffer
    engine.flush();

    // and wait for all jobs to finish as a safety (this should be a no-op)
    js.runAndWait(rootJob);
}

void FRenderer::renderJob(RootArenaScope& rootArenaScope, FView& view) {
    FEngine& engine = mEngine;
    JobSystem& js = engine.getJobSystem();
    FEngine::DriverApi& driver = engine.getDriverApi();
    PostProcessManager& ppm = engine.getPostProcessManager();

    // DEBUG: driver commands must all happen from the same thread. Enforce that on debug builds.
    driver.debugThreading();

    bool hasPostProcess = view.hasPostProcessPass();
    bool hasScreenSpaceRefraction = false;
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
    const bool isRenderingMultiview = view.hasStereo() &&
            engine.getConfig().stereoscopicType == backend::StereoscopicType::MULTIVIEW;
    // FIXME: This is to override some settings that are not supported for multiview at the moment.
    // Remove this when all features are supported.
    if (isRenderingMultiview) {
        hasPostProcess = false;
        msaaOptions.enabled = false;
    }
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
    } else {
        // This configures post-process materials by setting constant parameters
        if (taaOptions.enabled) {
            ppm.configureTemporalAntiAliasingMaterial(taaOptions);
            if (taaOptions.upscaling) {
                // for now TAA upscaling is incompatible with regular dsr
                dsrOptions.enabled = false;
                // also, upscaling doesn't work well with quater-resolution SSAO
                aoOptions.resolution = 1.0;
                // Currently we only support a fixed TAA upscaling ratio
                scale = 0.5f;
            }
        }
    }

    const bool blendModeTranslucent = view.getBlendMode() == BlendMode::TRANSLUCENT;
    // If the swap-chain is transparent or if we blend into it, we need to allocate our intermediate
    // buffers with an alpha channel.
    const bool needsAlphaChannel =
            (mSwapChain && mSwapChain->isTransparent()) || blendModeTranslucent;

    const bool isProtectedContent =  mSwapChain && mSwapChain->isProtected();

    // asSubpass is disabled with TAA (although it's supported) because performance was degraded
    // on qualcomm hardware -- we might need a backend dependent toggle at some point
    const PostProcessManager::ColorGradingConfig colorGradingConfig{
            .asSubpass =
                    msaaSampleCount <= 1 &&
                    driver.isFrameBufferFetchSupported() &&
                    hasColorGrading &&
                    !bloomOptions.enabled && !dofOptions.enabled && !taaOptions.enabled &&
                    !engine.debug.renderer.disable_subpasses,
            .customResolve =
                    msaaSampleCount > 1 &&
                    driver.isFrameBufferFetchMultiSampleSupported() &&
                    msaaOptions.customResolve &&
                    hasColorGrading &&
                    !engine.debug.renderer.disable_subpasses,
            .translucent = needsAlphaChannel,
            .fxaa = hasFXAA,
            .dithering = hasDithering,
            .ldrFormat = (hasColorGrading && hasFXAA) ?
                    TextureFormat::RGBA8 : getLdrFormat(needsAlphaChannel)
    };

    // by construction (msaaSampleCount) both asSubpass and customResolve can't be true
    assert_invariant(colorGradingConfig.asSubpass + colorGradingConfig.customResolve < 2);

    // whether we're scaled at all
     bool scaled = any(notEqual(scale, float2(1.0f)));

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
    const bool noBufferPadding = (colorGradingConfig.asSubpass && !hasFXAA && !scaled)
            || engine.debug.renderer.disable_buffer_padding;

    // guardBand must be a multiple of 16 to guarantee the same exact rendering up to 4 mip levels.
    float const guardBand = guardBandOptions.enabled ? 16.0f : 0.0f;

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
        cameraInfo.clipTransform = { ts[0][0], ts[1][1], ts[3].x, ts[3].y };

        // adjust svp to the new, larger, rendering dimensions
        svp.width  = uint32_t(width);
        svp.height = uint32_t(height);
        xvp.left   = int32_t(guardBand);
        xvp.bottom = int32_t(guardBand);
    }

    view.prepare(engine, driver, rootArenaScope, svp, cameraInfo, getShaderUserTime(), needsAlphaChannel);

    view.prepareUpscaler(scale, taaOptions, dsrOptions);

    /*
     * Allocate command buffer
     */

    FScene& scene = *view.getScene();

    // Allocate some space for our commands in the per-frame Arena, and use that space as
    // an Arena for commands. All this space is released when we exit this method.
    size_t const perFrameCommandsSize = engine.getPerFrameCommandsSize();
    void* const arenaBegin = rootArenaScope.allocate(perFrameCommandsSize, CACHELINE_SIZE);
    void* const arenaEnd = pointermath::add(arenaBegin, perFrameCommandsSize);

    // This arena *must* stay valid until all commands have been processed
    RenderPass::Arena commandArena("Command Arena", { arenaBegin, arenaEnd });

    RenderPass::RenderFlags renderFlags = 0;
    if (view.hasShadowing())                renderFlags |= RenderPass::HAS_SHADOWING;
    if (view.isFrontFaceWindingInverted())  renderFlags |= RenderPass::HAS_INVERSE_FRONT_FACES;

    RenderPassBuilder passBuilder(commandArena);
    passBuilder.renderFlags(renderFlags);

    Variant variant;
    variant.setDirectionalLighting(view.hasDirectionalLight());
    variant.setDynamicLighting(view.hasDynamicLighting());
    variant.setFog(view.hasFog());
    variant.setVsm(view.hasShadowing() && view.getShadowType() != ShadowType::PCF);
    variant.setStereo(view.hasStereo());

    /*
     * Frame graph
     */

    FrameGraph fg(*mResourceAllocator,
            isProtectedContent ? FrameGraph::Mode::PROTECTED : FrameGraph::Mode::UNPROTECTED);
    auto& blackboard = fg.getBlackboard();

    /*
     * Shadow pass
     */

    if (view.needsShadowMap()) {
        Variant shadowVariant(Variant::DEPTH_VARIANT);
        shadowVariant.setVsm(view.getShadowType() == ShadowType::VSM);
        auto shadows = view.renderShadowMaps(engine, fg, cameraInfo, mShaderUserTime,
                RenderPassBuilder{ commandArena }
                    .renderFlags(renderFlags)
                    .variant(shadowVariant));
        blackboard["shadows"] = shadows;
    }

    // When we don't have a custom RenderTarget, currentRenderTarget below is nullptr and is
    // recorded in the list of targets already rendered into -- this ensures that
    // initializeClearFlags() is called only once for the default RenderTarget.
    auto& previousRenderTargets = mPreviousRenderTargets;
    FRenderTarget* const currentRenderTarget = downcast(view.getRenderTarget());
    if (UTILS_LIKELY(
            previousRenderTargets.find(currentRenderTarget) == previousRenderTargets.end())) {
        previousRenderTargets.insert(currentRenderTarget);
        initializeClearFlags();
    }

    // Note:  it is not well-defined what colorspace this clearColor is defined into. This leads
    //        to inconsistent colors depending on enabled features. When the clear is performed
    //        into a temporary buffer (common case), the clearColor is color-graded. A problem
    //        arises when transparent views are used, in this case the clear color is not
    //        color-graded.
    const float4 clearColor = mClearOptions.clearColor;

    const uint8_t clearStencil = mClearOptions.clearStencil;
    const TargetBufferFlags clearFlags = mClearFlags;
    const TargetBufferFlags discardStartFlags = mDiscardStartFlags;
    const TargetBufferFlags keepOverrideStartFlags = TargetBufferFlags::ALL & ~discardStartFlags;
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

    // the clearFlags and clearColor set below are "sticky" to the imported target, meaning
    // they will apply anytime we render into this target, THIS INCLUDES when this target
    // is "replacing" another one. E.g. typically when the color pass ends-up drawing directly
    // here.
    auto [viewRenderTarget, attachmentMask] = getRenderTarget(view);
    FrameGraphId<FrameGraphTexture> const fgViewRenderTarget = fg.import("viewRenderTarget", {
            .attachments = attachmentMask,
            .viewport = DEBUG_DYNAMIC_SCALING ? svp : vp,
            .clearColor = clearColor,
            .samples = 0,
            .clearFlags = clearFlags,
            .keepOverrideStart = keepOverrideStartFlags,
            .keepOverrideEnd = keepOverrideEndFlags
    }, viewRenderTarget);

    const TextureFormat hdrFormat = getHdrFormat(view, needsAlphaChannel);

    // the clearFlags and clearColor specified below will only apply when rendering into the
    // temporary color buffer. In particular, they won't apply when rendering into the main
    // swapchain (imported render target above)
    RendererUtils::ColorPassConfig config{
            .physicalViewport = svp,
            .logicalViewport = xvp,
            .scale = scale,
            .hdrFormat = hdrFormat,
            .msaa = msaaSampleCount,
            .clearFlags = getClearFlags(),
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

    passBuilder.camera(cameraInfo);
    passBuilder.geometry(scene.getRenderableData(),
            view.getVisibleRenderables(),
            view.getRenderableUBO());

    // view set-ups that need to happen before rendering
    fg.addTrivialSideEffectPass("Prepare View Uniforms",
            [=, &view, &engine](DriverApi& driver) {
                view.prepareCamera(engine, cameraInfo);

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

                // note: aoOptions.resolution is either 1.0 or 0.5, and the result is then
                // guaranteed to be an integer (because xvp is a multiple of 16).
                view.prepareViewport(svp,
                        filament::Viewport{
                             int32_t(float(xvp.left  ) * aoOptions.resolution),
                             int32_t(float(xvp.bottom) * aoOptions.resolution),
                            uint32_t(float(xvp.width ) * aoOptions.resolution),
                            uint32_t(float(xvp.height) * aoOptions.resolution)});

                view.commitUniforms(driver);

                // set uniforms and samplers for the color passes
                view.bindPerViewUniformsAndSamplers(driver);
            });

    // --------------------------------------------------------------------------------------------
    // structure pass -- automatically culled if not used
    // Currently it consists of a simple depth pass.
    // This is normally used by SSAO and contact-shadows

    // TODO: the scaling should depends on all passes that need the structure pass
    const auto [structure, picking_] = ppm.structure(fg,
            passBuilder, renderFlags, svp.width, svp.height, {
            .scale = aoOptions.resolution,
            .picking = view.hasPicking()
    });
    const auto picking = picking_;


    if (view.hasPicking()) {
        struct PickingResolvePassData {
            FrameGraphId<FrameGraphTexture> picking;
        };
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
                        auto const&, DriverApi& driver) mutable {
                    auto out = resources.getRenderPassInfo();
                    view.executePickingQueries(driver, out.target, scale * aoOptions.resolution);
                });
    }

    // Store this frame's camera projection in the frame history.
    if (UTILS_UNLIKELY(taaOptions.enabled)) {
        // Apply the TAA jitter to everything after the structure pass, starting with the color pass.
        ppm.prepareTaa(fg, svp, taaOptions, view.getFrameHistory(), &FrameHistoryEntry::taa,
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

    PostProcessManager::ScreenSpaceRefConfig const ssrConfig = PostProcessManager::prepareMipmapSSR(
            fg, svp.width, svp.height,
            ssReflectionsOptions.enabled ? TextureFormat::RGBA16F : TextureFormat::R11F_G11F_B10F,
            view.getCameraUser().getFieldOfView(Camera::Fov::VERTICAL), config.scale);
    config.ssrLodOffset = ssrConfig.lodOffset;

    // --------------------------------------------------------------------------------------------
    // screen-space reflections pass

    if (ssReflectionsOptions.enabled) {
        auto reflections = ppm.ssr(fg, passBuilder,
                view.getFrameHistory(), cameraInfo,
                view.getPerViewUniforms(),
                structure,
                ssReflectionsOptions,
                { .width = svp.width, .height = svp.height });

        if (UTILS_LIKELY(reflections)) {
            // generate the mipchain
            PostProcessManager::generateMipmapSSR(ppm, fg,
                    reflections, ssrConfig.reflection, false, ssrConfig);
        }
        config.screenSpaceReflectionHistoryNotReady = !reflections;
    }

    // --------------------------------------------------------------------------------------------
    // Color passes

    // this makes the viewport relative to xvp
    // FIXME: we should use 'vp' when rendering directly into the swapchain, but that's hard to
    //        know at this point. This will usually be the case when post-process is disabled.
    // FIXME: we probably should take the dynamic scaling into account too
    passBuilder.scissorViewport(hasPostProcess ? xvp : vp);

    // This one doesn't need to be a FrameGraph pass because it always happens by construction
    // (i.e. it won't be culled, unless everything is culled), so no need to complexify things.
    passBuilder.variant(variant);

    // color-grading as subpass is done either by the color pass or the TAA pass if any
    auto colorGradingConfigForColor = colorGradingConfig;
    colorGradingConfigForColor.asSubpass = colorGradingConfigForColor.asSubpass && !taaOptions.enabled;

    if (colorGradingConfigForColor.asSubpass) {
        // append color grading subpass after all other passes
        passBuilder.customCommand(engine, 3,
                RenderPass::Pass::BLENDED,
                RenderPass::CustomCommand::EPILOG,
                0, [&ppm, &driver, colorGradingConfigForColor]() {
                    ppm.colorGradingSubpass(driver, colorGradingConfigForColor);
                });
    } else if (colorGradingConfig.customResolve) {
        // append custom resolve subpass after all other passes
        passBuilder.customCommand(engine, 3,
                RenderPass::Pass::BLENDED,
                RenderPass::CustomCommand::EPILOG,
                0, [&ppm, &driver]() {
                    ppm.customResolveSubpass(driver);
                });
    }

    passBuilder.commandTypeFlags(RenderPass::CommandTypeFlags::COLOR);


    // RenderPass::IS_INSTANCED_STEREOSCOPIC only applies to the color pass
    if (view.hasStereo() &&
        engine.getConfig().stereoscopicType == backend::StereoscopicType::INSTANCED) {
        renderFlags |= RenderPass::IS_INSTANCED_STEREOSCOPIC;
        passBuilder.renderFlags(renderFlags);
    }

    RenderPass const pass{ passBuilder.build(engine) };

    FrameGraphTexture::Descriptor colorBufferDesc = {
            .width = config.physicalViewport.width,
            .height = config.physicalViewport.height,
            .format = config.hdrFormat
    };

    // Set the depth to the number of layers if we're rendering multiview.
    if (isRenderingMultiview) {
        colorBufferDesc.depth = engine.getConfig().stereoscopicEyeCount;
        colorBufferDesc.type = backend::SamplerType::SAMPLER_2D_ARRAY;
    }

    // a non-drawing pass to prepare everything that need to be before the color passes execute
    fg.addTrivialSideEffectPass("Prepare Color Passes",
            [=, &js, &view, &ppm](DriverApi& driver) {
                // prepare color grading as subpass material
                if (colorGradingConfig.asSubpass) {
                    ppm.colorGradingPrepareSubpass(driver,
                            colorGrading, colorGradingConfig, vignetteOptions,
                            colorBufferDesc.width, colorBufferDesc.height);
                } else if (colorGradingConfig.customResolve) {
                    ppm.customResolvePrepareSubpass(driver,
                            PostProcessManager::CustomResolveOp::COMPRESS);
                }

                // We use a framegraph pass to wait for froxelization to finish (so it can be done
                // in parallel with .compile()
                auto sync = view.getFroxelizerSync();
                if (sync) {
                    js.waitAndRelease(sync);
                    view.commitFroxels(driver);
                }
            }
    );

    // the color pass itself + color-grading as subpass if needed
    auto colorPassOutput = RendererUtils::colorPass(fg, "Color Pass", mEngine, view, {
                .shadows = blackboard.get<FrameGraphTexture>("shadows"),
                .ssao = blackboard.get<FrameGraphTexture>("ssao"),
                .ssr = ssrConfig.ssr,
                .structure = structure
            },
            colorBufferDesc, config, colorGradingConfigForColor, pass.getExecutor());

    if (view.isScreenSpaceRefractionEnabled() && !pass.empty()) {
        // This cancels the colorPass() call above if refraction is active.
        // The color pass + refraction + color-grading as subpass if needed
        auto const output = RendererUtils::refractionPass(fg, mEngine, view, {
                        .shadows = blackboard.get<FrameGraphTexture>("shadows"),
                        .ssao = blackboard.get<FrameGraphTexture>("ssao"),
                        .ssr = ssrConfig.ssr,
                        .structure = structure
                },
                config, ssrConfig, colorGradingConfigForColor, pass);

        hasScreenSpaceRefraction = output.has_value();
        if (hasScreenSpaceRefraction) {
            colorPassOutput = output.value();
        }
    }

    if (colorGradingConfig.customResolve) {
        assert_invariant(fg.getDescriptor(colorPassOutput.linearColor).samples <= 1);
        // TODO: we have to "uncompress" (i.e. detonemap) the color buffer here because it's used
        //       by many other passes (Bloom, TAA, DoF, etc...). We could make this more
        //       efficient by using ARM_shader_framebuffer_fetch. We use a load/store (i.e.
        //       subpass) here because it's more convenient.
        colorPassOutput.linearColor =
                ppm.customResolveUncompressPass(fg, colorPassOutput.linearColor);
    }

    // export the color buffer if screen-space reflections are enabled
    if (ssReflectionsOptions.enabled) {
        struct ExportSSRHistoryData {
            FrameGraphId<FrameGraphTexture> history;
        };
        // FIXME: should we use the TAA-modified cameraInfo here or not? (we are).
        mat4 const projection = cameraInfo.projection * cameraInfo.getUserViewMatrix();
        fg.addPass<ExportSSRHistoryData>("Export SSR history",
                [&](FrameGraph::Builder& builder, auto& data) {
                    // We need to use sideEffect here to ensure this pass won't be culled.
                    // The "output" of this pass is going to be used during the next frame as
                    // an "import".
                    builder.sideEffect();

                    // we can't use colorPassOutput here because it could be tonemapped
                    data.history = builder.sample(colorPassOutput.linearColor); // FIXME: an access must be declared for detach(), why?
                }, [&view, projection](FrameGraphResources const& resources, auto const& data,
                        backend::DriverApi&) {
                    auto& history = view.getFrameHistory();
                    auto& current = history.getCurrent();
                    current.ssr.projection = projection;
                    resources.detach(data.history, &current.ssr.color, &current.ssr.desc);
                });
    }

    // this is the output of the color pass / input to post processing,
    // this is only used later for comparing it with the output after post-processing
    FrameGraphId<FrameGraphTexture> const postProcessInput = colorGradingConfig.asSubpass ?
                                                           colorPassOutput.tonemappedColor :
                                                           colorPassOutput.linearColor;

    // input can change below
    FrameGraphId<FrameGraphTexture> input = postProcessInput;
    fg.addTrivialSideEffectPass("Finish Color Passes", [&view](DriverApi& driver) {
        // Unbind SSAO sampler, b/c the FrameGraph will delete the texture at the end of the pass.
        view.cleanupRenderPasses();
        view.commitUniforms(driver);
    });

    // Resolve depth -- which might be needed because of TAA or DoF. This pass will be culled
    // if the depth is not used below or if the depth is not MS (e.g. it could have been
    // auto-resolved).
    // In practice, this is used on Vulkan and older Metal devices.
    auto depth = ppm.resolve(fg, "Resolved Depth Buffer", colorPassOutput.depth, { .levels = 1 });

    // Debug: CSM visualisation
    if (UTILS_UNLIKELY(engine.debug.shadowmap.visualize_cascades &&
                       view.hasShadowing() && view.hasDirectionalLight())) {
        input = ppm.debugShadowCascades(fg, input, depth);
    }

    // TODO: DoF should be applied here, before TAA -- but if we do this it'll result in a lot of
    //       fireflies due to the instability of the highlights. This can be fixed with a
    //       dedicated TAA pass for the DoF, as explained in
    //       "Life of a Bokeh" by Guillaume Abadie, SIGGRAPH 2018

    // TAA for color pass
    if (taaOptions.enabled) {
        input = ppm.taa(fg, input, depth, view.getFrameHistory(), &FrameHistoryEntry::taa,
                taaOptions, colorGradingConfig);
        if (taaOptions.upscaling) {
            scale = 1.0f;
            scaled = false;
            UTILS_UNUSED_IN_RELEASE auto const& inputDesc = fg.getDescriptor(input);
            svp.width = inputDesc.width;
            svp.height = inputDesc.height;
            xvp.width *= 2;
            xvp.height *= 2;
        }
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
            float const aspect = (scale.x / scale.y) * dofOptions.cocAspectRatio;
            float2 const bokehScale{
                aspect < 1.0f ? aspect : 1.0f,
                aspect > 1.0f ? 1.0f / aspect : 1.0f
            };
            input = ppm.dof(fg, input, depth, cameraInfo, needsAlphaChannel,
                    bokehScale, dofOptions);
        }

        FrameGraphId<FrameGraphTexture> bloom, flare;
        if (bloomOptions.enabled) {
            // Generate the bloom buffer, which is stored in the blackboard as "bloom". This is
            // consumed by the colorGrading pass and will be culled if colorGrading is disabled.
            auto [bloom_, flare_] = ppm.bloom(fg, input, TextureFormat::R11F_G11F_B10F,
                    bloomOptions, taaOptions, scale);
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
            input = ppm.upscale(fg, needsAlphaChannel, dsrOptions, input, xvp, {
                    .width = viewport.width, .height = viewport.height,
                    .format = colorGradingConfig.ldrFormat }, SamplerMagFilter::LINEAR);
            xvp.left = xvp.bottom = 0;
            svp = xvp;
        }
    }

    // Debug: combine the array texture for multiview into a single image.
    if (UTILS_UNLIKELY(isRenderingMultiview && engine.debug.stereo.combine_multiview_images)) {
        input = ppm.debugCombineArrayTexture(fg, blendModeTranslucent, input, xvp, {
                        .width = vp.width, .height = vp.height,
                        .format = colorGradingConfig.ldrFormat },
                        SamplerMagFilter::NEAREST, SamplerMinFilter::NEAREST);
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
    // * And we also can't use the default rendertarget with refractions, which need to reuse
    //   the rendertarget due to how the clear flags work.
    // * We also need an extra blit if we haven't yet handled "xvp"
    //   TODO: in that specific scenario it would be better to just not use xvp
    // The intermediate buffer is accomplished with a "fake" opaqueBlit (i.e. blit) operation.

    const bool outputIsSwapChain =
            (input == postProcessInput) && (viewRenderTarget == mRenderTargetHandle);
    if (mightNeedFinalBlit) {
        if (blendModeTranslucent ||
            xvp != svp ||
            (outputIsSwapChain &&
                    (msaaSampleCount > 1 ||
                    colorGradingConfig.asSubpass ||
                    hasScreenSpaceRefraction ||
                    ssReflectionsOptions.enabled))) {
            assert_invariant(!scaled);
            input = ppm.blit(fg, blendModeTranslucent, input, xvp, {
                            .width = vp.width, .height = vp.height,
                            .format = colorGradingConfig.ldrFormat },
                    SamplerMagFilter::NEAREST, SamplerMinFilter::NEAREST);
        }
    }

    if (UTILS_UNLIKELY(outputIsSwapChain && view.isStencilBufferEnabled())) {
        assert_invariant(mSwapChain);
        FILAMENT_CHECK_PRECONDITION(mSwapChain->hasStencilBuffer())
                << "View has stencil buffer enabled, but SwapChain does not have "
                   "SwapChain::CONFIG_HAS_STENCIL_BUFFER flag set.";
    }

    if (UTILS_UNLIKELY(engine.debug.shadowmap.display_shadow_texture)) {
        auto shadowmap = blackboard.get<FrameGraphTexture>("shadowmap");
        input = ppm.debugDisplayShadowTexture(fg, input, shadowmap,
                engine.debug.shadowmap.display_shadow_texture_scale,
                engine.debug.shadowmap.display_shadow_texture_layer,
                engine.debug.shadowmap.display_shadow_texture_level,
                engine.debug.shadowmap.display_shadow_texture_channel,
                engine.debug.shadowmap.display_shadow_texture_power);
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
