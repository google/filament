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

#include "RenderPass.h"

#include "details/Engine.h"
#include "details/Fence.h"
#include "details/Scene.h"
#include "details/SwapChain.h"
#include "details/View.h"

#include <filament/Scene.h>

#include <filament/driver/PixelBufferDescriptor.h>

#include <utils/Panic.h>
#include <utils/Systrace.h>
#include <utils/vector.h>

#include <assert.h>

using namespace math;
using namespace utils;

namespace filament {

using namespace driver;

namespace details {

FRenderer::FRenderer(FEngine& engine) :
        mEngine(engine),
        mFrameSkipper(engine, 2),
        mFrameInfoManager(engine),
        mIsRGB16FSupported(false),
        mIsRGB8Supported(false),
        mPerRenderPassArena(engine.getPerRenderPassAllocator())
{
}

void FRenderer::init() noexcept {
    DriverApi& driver = mEngine.getDriverApi();
    mRenderTarget = driver.createDefaultRenderTarget();
    mIsRGB16FSupported = driver.isRenderTargetFormatSupported(driver::TextureFormat::RGB16F);
    mIsRGB8Supported = driver.isRenderTargetFormatSupported(driver::TextureFormat::RGB8);
    if (UTILS_HAS_THREADING) {
        mFrameInfoManager.run();
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
        Fence::waitAndDestroy(engine.createFence());
        mFrameInfoManager.terminate();
    } else {
        // In single threaded mode, allow recently-created objects (e.g. no-op fences in Skipper)
        // to initialize themselves, otherwise the engine tries to destroy invalid handles.
        engine.execute();
    }
}

void FRenderer::render(FView const* view) {
    SYSTRACE_CALL();

    assert(mSwapChain);

    if (UTILS_LIKELY(view && view->getScene())) {
        // per-renderpass data
        ArenaScope rootArena(mPerRenderPassArena);

        FEngine& engine = mEngine;
        JobSystem& js = engine.getJobSystem();

        // create a master job so no other job can escape
        auto masterJob = js.setMasterJob(js.createJob());

        // execute the render pass
        renderJob(rootArena, const_cast<FView*>(view));

        // make sure to flush the command buffer
        engine.flush();

        // and wait for all jobs to finish as a safety (this should be a no-op)
        js.runAndWait(masterJob);
        js.reset();
    }
}

void FRenderer::renderJob(ArenaScope& arena, FView* view) {
    FEngine& engine = getEngine();
    JobSystem& js = engine.getJobSystem();
    FEngine::DriverApi& driver = engine.getDriverApi();
    PostProcessManager& ppm = engine.getPostProcessManager();
    RenderTargetPool& rtp = engine.getRenderTargetPool();

    // DEBUG: driver commands must all happen from the same thread. Enforce that on debug builds.
    engine.getDriverApi().debugThreading();

    Viewport const& vp = view->getViewport();
    const bool hasPostProcess = view->hasPostProcessPass();
    float2 scale = view->updateScale(mFrameInfoManager.getLastFrameTime());
    bool mUseFXAA = view->getAntiAliasing() == View::AntiAliasing::FXAA;
    if (!hasPostProcess) {
        // dynamic scaling and FXAA are part of the post-process phase and can't happen if
        // it's disabled.
        mUseFXAA = false;
        scale = 1.0f;
    }

    const bool scaled = any(notEqual(scale, float2(1.0f)));
    Viewport svp = vp.scale(scale);
    if (svp.empty()) {
        return;
    }

    view->prepare(engine, driver, arena, svp);
    // TODO: froxelization could actually start now, instead of in ColorPass::renderColorPass()

    /*
     * Allocate command buffer.
     */

    const size_t commandsSize = FEngine::CONFIG_PER_FRAME_COMMANDS_SIZE;
    const size_t commandsCount = commandsSize / sizeof(Command);
    GrowingSlice<Command> commands(
            arena.allocate<Command>(commandsCount, CACHELINE_SIZE), commandsCount);

    /*
     * Shadow pass
     */

    if (view->hasShadowing()) {
        ShadowPass::renderShadowMap(engine, js, view, commands);
        recordHighWatermark(commands); // for debugging
        // reset the command buffer
        commands.clear();
    }

    /*
     * Depth + Color passes
     */

    const uint8_t useMSAA = view->getSampleCount();
    const TextureFormat hdrFormat = getHdrFormat();
    const TextureFormat ldrFormat = getLdrFormat();
    RenderTargetPool::Target const* colorTarget = nullptr;

    if (UTILS_LIKELY(hasPostProcess)) {
        // allocate the target we need for rendering the scene
        colorTarget = rtp.get(TargetBufferFlags::COLOR_AND_DEPTH,
                svp.width, svp.height, useMSAA, hdrFormat);
        svp.left = svp.bottom = 0;
    }

    // FIXME: viewRenderTarget doesn't have a depth-buffer, so when skipping post-process, don't rely on it
    const Handle<HwRenderTarget> viewRenderTarget = getRenderTarget();
    ColorPass::renderColorPass(engine, js,
            colorTarget ? colorTarget->target : viewRenderTarget, view, svp, commands);

    /*
     * Post Processing...
     */

    if (UTILS_LIKELY(hasPostProcess)) {
        driver.pushGroupMarker("Post Processing");

        ppm.start();

        if (useMSAA > 1) {
            // Note: MSAA, when used is applied before tone-mapping (which is not ideal)
            // (tone mapping currently only works without multi-sampling)
            // this blit does a MSAA resolve
            ppm.blit(hdrFormat);
        }

        const bool translucent = mSwapChain->isTransparent();
        Handle<HwProgram> toneMappingProgram = engine.getPostProcessProgram(
                translucent ? PostProcessStage::TONE_MAPPING_TRANSLUCENT
                            : PostProcessStage::TONE_MAPPING_OPAQUE);
        ppm.pass(mUseFXAA ? TextureFormat::RGBA8 : ldrFormat, toneMappingProgram);

        if (mUseFXAA) {
            Handle<HwProgram> antiAliasingProgram = engine.getPostProcessProgram(
                    translucent ? PostProcessStage::ANTI_ALIASING_TRANSLUCENT
                                : PostProcessStage::ANTI_ALIASING_OPAQUE);
            ppm.pass(ldrFormat, antiAliasingProgram);
        }

        if (scaled) {
            // because it's the last command, the TextureFormat is not relevant
            ppm.blit();
        }
        ppm.finish(view->getDiscardedTargetBuffers(), viewRenderTarget, vp, colorTarget, svp);

        driver.popGroupMarker();
    }

    // for debugging
    recordHighWatermark(commands);
}

bool FRenderer::beginFrame(FSwapChain* swapChain) {
    SYSTRACE_CALL();

    assert(swapChain);

    mFrameId++;
    if (UTILS_HAS_THREADING) {
        mFrameInfoManager.beginFrame(mFrameId);
    }

    { // scope for frame id trace
        char buf[64];
        snprintf(buf, 64, "frame %u", mFrameId);
        SYSTRACE_NAME(buf);
    }

    FEngine& engine = getEngine();
    FEngine::DriverApi& driver = engine.getDriverApi();

    // NOTE: this makes synchronous calls to the driver
    driver.updateStreams(&driver);

    mSwapChain = swapChain;
    swapChain->makeCurrent(driver);

    driver.beginFrame(
            uint64_t(std::chrono::steady_clock::now().time_since_epoch().count()), mFrameId);

    if (mFrameSkipper.skipFrameNeeded()) {
        mFrameInfoManager.cancelFrame();
        driver.endFrame(mFrameId);
        engine.flush();
        return false;
    }

    // ask the engine to do what it needs to (e.g. updates light buffer, materials...)
    engine.prepare();

    return true;
}

void FRenderer::endFrame() {
    SYSTRACE_CALL();

    FEngine& engine = getEngine();
    FEngine::DriverApi& driver = engine.getDriverApi();
    RenderTargetPool& rtp = engine.getRenderTargetPool();

    FrameInfoManager& frameInfoManager = mFrameInfoManager;

    if (UTILS_HAS_THREADING) {

        // on debug builds this helps catching cases where we're writing to
        // the buffer form another thread, which is currently not allowed.
        driver.debugThreading();

        frameInfoManager.endFrame();
    }
    mFrameSkipper.endFrame();

    driver.endFrame(mFrameId);

    if (mSwapChain) {
        mSwapChain->commit(driver);
        mSwapChain = nullptr;
    }

    // Run the component managers' GC in parallel
    // WARNING: while doing this we can't access any component manager
    auto& js = engine.getJobSystem();

    auto job = jobs::createJob(js, nullptr, &FEngine::gc, &engine); // gc all managers
    js.run(job);

    rtp.gc();           // gc post-processing targets (this can generate driver commands)
    engine.flush();     // flush command stream

    // make sure we're done with the gcs
    js.wait(job);


#if EXTRA_TIMING_INFO
    if (UTILS_UNLIKELY(frameInfoManager.isLapRecordsEnabled())) {
        auto history = frameInfoManager.getHistory();
        FrameInfo const& info = history.back();
        FrameInfo::duration rendering   = info.laps[FrameInfo::LAP_0]  - info.laps[FrameInfo::START];
        FrameInfo::duration postprocess = info.laps[FrameInfo::FINISH] - info.laps[FrameInfo::LAP_0];
        mRendering.push(rendering.count());
        mPostProcess.push(postprocess.count());
        slog.d << mRendering.latest() << ", "
               << mPostProcess.latest() << io::endl;
    }
#endif
}

void FRenderer::readPixels(uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
        driver::PixelBufferDescriptor&& buffer) {

    if (!ASSERT_POSTCONDITION_NON_FATAL(
            buffer.type != PixelDataType::COMPRESSED,
            "buffer.format cannot be COMPRESSED")) {
        return;
    }

    if (!ASSERT_POSTCONDITION_NON_FATAL(
            buffer.alignment > 0 && buffer.alignment <= 8 &&
            !(buffer.alignment & (buffer.alignment - 1)),
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
    driver.readPixels(mRenderTarget, xoffset, yoffset, width, height, std::move(buffer));
}

} // namespace details

// ------------------------------------------------------------------------------------------------
// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

using namespace details;

Engine* Renderer::getEngine() noexcept {
    return &upcast(this)->getEngine();
}

void Renderer::render(View const* view) {
    upcast(this)->render(upcast(view));
}

bool Renderer::beginFrame(SwapChain* swapChain) {
    return upcast(this)->beginFrame(upcast(swapChain));
}

void Renderer::readPixels(uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
        driver::PixelBufferDescriptor&& buffer) {
    upcast(this)->readPixels(xoffset, yoffset, width, height, std::move(buffer));
}

void Renderer::endFrame() {
    upcast(this)->endFrame();
}

} // namespace filament
