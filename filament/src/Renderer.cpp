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
#include <filament/Renderer.h>

#include <backend/PixelBufferDescriptor.h>

#include "fg/FrameGraph.h"
#include "fg/FrameGraphHandle.h"
#include "fg/ResourceAllocator.h"


#include <utils/Panic.h>
#include <utils/Systrace.h>
#include <utils/vector.h>

#include <assert.h>


using namespace filament::math;
using namespace utils;

namespace filament {

using namespace backend;

namespace details {

FRenderer::FRenderer(FEngine& engine) :
        mEngine(engine),
        mFrameSkipper(engine, 2),
        mFrameInfoManager(engine),
        mIsRGB16FSupported(false),
        mIsRGB8Supported(false),
        mPerRenderPassArena(engine.getPerRenderPassAllocator())
{
    FDebugRegistry& debugRegistry = engine.getDebugRegistry();
    debugRegistry.registerProperty("d.ssao.enabled", &engine.debug.ssao.enabled);
}

void FRenderer::init() noexcept {
    DriverApi& driver = mEngine.getDriverApi();
    mUserEpoch = mEngine.getEngineEpoch();
    mRenderTarget = driver.createDefaultRenderTarget();
    mIsRGB16FSupported = driver.isRenderTargetFormatSupported(backend::TextureFormat::RGB16F);
    mIsRGB8Supported = driver.isRenderTargetFormatSupported(backend::TextureFormat::RGB8);
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

void FRenderer::resetUserTime() {
    mUserEpoch = std::chrono::steady_clock::now();
}

backend::TextureFormat FRenderer::getHdrFormat(const View& view) const noexcept {
    const bool translucent = mSwapChain->isTransparent();
    if (translucent) return backend::TextureFormat::RGBA16F;

    switch (view.getRenderQuality().hdrColorBuffer) {
        case View::QualityLevel::LOW:
        case View::QualityLevel::MEDIUM:
            return backend::TextureFormat::R11F_G11F_B10F;
        case View::QualityLevel::HIGH:
        case View::QualityLevel::ULTRA:
            return !mIsRGB16FSupported ? backend::TextureFormat::RGBA16F
                                       : backend::TextureFormat::RGB16F;
    }
}

backend::TextureFormat FRenderer::getLdrFormat() const noexcept {
    const bool translucent = mSwapChain->isTransparent();
    return (translucent || !mIsRGB8Supported) ? backend::TextureFormat::RGBA8
                                              : backend::TextureFormat::RGB8;
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
        renderJob(rootArena, const_cast<FView&>(*view));

        // make sure to flush the command buffer
        engine.flush();

        // and wait for all jobs to finish as a safety (this should be a no-op)
        js.runAndWait(masterJob);
    }
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
    bool toneMapping = view.getToneMapping() == View::ToneMapping::ACES;
    bool dithering = view.getDithering() == View::Dithering::TEMPORAL;
    bool fxaa = view.getAntiAliasing() == View::AntiAliasing::FXAA;
    uint8_t msaa = view.getSampleCount();
    float2 scale = view.updateScale(mFrameInfoManager.getLastFrameTime());
    if (!hasPostProcess) {
        // dynamic scaling and FXAA are part of the post-process phase and can't happen if
        // it's disabled.
        fxaa = false;
        dithering = false;
        scale = 1.0f;
        msaa = 1;
    }

    const bool scaled = any(notEqual(scale, float2(1.0f)));
    filament::Viewport svp = vp.scale(scale);
    if (svp.empty()) {
        return;
    }

    view.prepare(engine, driver, arena, svp, getShaderUserTime());

    // start froxelization immediately, it has no dependencies
    JobSystem::Job* jobFroxelize = js.runAndRetain(js.createJob(nullptr,
            [&engine, &view](JobSystem&, JobSystem::Job*) { view.froxelize(engine); }));

    /*
     * Allocate command buffer.
     */

    FScene& scene = *view.getScene();

    const size_t commandsSize = FEngine::CONFIG_PER_FRAME_COMMANDS_SIZE;
    const size_t commandsCount = commandsSize / sizeof(Command);
    GrowingSlice<Command> commands(
            arena.allocate<Command>(commandsCount, CACHELINE_SIZE), commandsCount);


    RenderPass pass(engine, commands);
    RenderPass::RenderFlags renderFlags = 0;
    if (view.hasShadowing())               renderFlags |= RenderPass::HAS_SHADOWING;
    if (view.hasDirectionalLight())        renderFlags |= RenderPass::HAS_DIRECTIONAL_LIGHT;
    if (view.hasDynamicLighting())         renderFlags |= RenderPass::HAS_DYNAMIC_LIGHTING;
    if (view.isFrontFaceWindingInverted()) renderFlags |= RenderPass::HAS_INVERSE_FRONT_FACES;
    pass.setRenderFlags(renderFlags);


    /*
     * Shadow pass
     */

    if (view.hasShadowing()) {
        view.getShadowMap().render(driver, pass, view);
        driver.flush(); // Kick the GPU since we're done with this render target
        engine.flush(); // Wake-up the driver thread
        commands.clear();
    }

    /*
     * Frame graph
     */

    FrameGraph fg(engine.getResourceAllocator());

    const TextureFormat hdrFormat = getHdrFormat(view);

    // FIXME: we use "hasPostProcess" as a proxy for deciding if we need a depth-buffer or not
    //        historically this has been true, but it's definitely wrong.
    //        This hack is needed because viewRenderTarget(output) doesn't have a depth-buffer,
    //        so when skipping post-process (which draws directly into it), we can't rely on it.
    const bool colorPassNeedsDepthBuffer = hasPostProcess;

    const backend::Handle<backend::HwRenderTarget> viewRenderTarget = getRenderTarget(view);
    FrameGraphRenderTargetHandle fgViewRenderTarget = fg.importRenderTarget("viewRenderTarget",
            { .viewport = vp }, viewRenderTarget, vp.width, vp.height,
            view.getDiscardedTargetBuffers());

    /*
     * Depth + Color passes
     */

    CameraInfo const& cameraInfo = view.getCameraInfo();
    pass.setCamera(cameraInfo);
    pass.setGeometry(scene, view.getVisibleRenderables());

    view.updatePrimitivesLod(engine, cameraInfo,
            scene.getRenderableData(), view.getVisibleRenderables());
    view.prepareCamera(cameraInfo, svp);
    view.commitUniforms(driver);


    // --------------------------------------------------------------------------------------------

    const bool useSSAO = view.getAmbientOcclusion() != View::AmbientOcclusion::NONE;

    // SSAO pass -- automatically culled if not used
    if (useSSAO) {
        pass.appendSortedCommands(RenderPass::CommandTypeFlags::DEPTH);
    }

    FrameGraphId<FrameGraphTexture> ssao = ppm.ssao(fg, pass, svp, cameraInfo, view.getAmbientOcclusionOptions());

    // --------------------------------------------------------------------------------------------

    // generate the normal commands
    RenderPass::CommandTypeFlags commandType = getCommandType(view.getDepthPrepass());
    Command const* colorPassBegin = commands.end();
    Command const* colorPassEnd = pass.appendSortedCommands(commandType);

    // We only honor the view's color buffer clear flags, depth/stencil are handled by the framefraph
    TargetBufferFlags clearFlags = view.getClearFlags() & TargetBufferFlags::COLOR | TargetBufferFlags::DEPTH;

    struct ColorPassData {
        FrameGraphId<FrameGraphTexture> color;
        FrameGraphId<FrameGraphTexture> depth;
        FrameGraphId<FrameGraphTexture> ssao;
        FrameGraphRenderTargetHandle rt;
    };

    auto& colorPass = fg.addPass<ColorPassData>("Color Pass",
            [&svp, hdrFormat, colorPassNeedsDepthBuffer, msaa, clearFlags, useSSAO, ssao]
            (FrameGraph::Builder& builder, ColorPassData& data) {

                if (useSSAO) {
                    data.ssao = builder.sample(ssao);
                }

                data.color = builder.createTexture("Color Buffer",
                        { .width = svp.width, .height = svp.height, .format = hdrFormat });

                if (colorPassNeedsDepthBuffer) {
                    data.depth = builder.createTexture("Depth Buffer", {
                            .width = svp.width, .height = svp.height,
                            .format = TextureFormat::DEPTH24
                    });
                    data.depth = builder.write(builder.read(data.depth));
                }

                data.color = builder.write(builder.read(data.color));
                data.rt = builder.createRenderTarget("Color Pass Target", {
                        .samples = msaa,
                        .attachments.color = data.color,
                        .attachments.depth = data.depth
                }, clearFlags);
            },
            [&pass, &ppm, colorPassBegin, colorPassEnd, jobFroxelize, &js, &view]
                    (FrameGraphPassResources const& resources,
                            ColorPassData const& data, DriverApi& driver) {
                auto out = resources.getRenderTarget(data.rt);
                Handle<HwTexture> ssao;
                if (data.ssao.isValid()) {
                    ssao = resources.getTexture(data.ssao);
                } else {
                    ssao = ppm.getNoSSAOTexture();
                }
                view.prepareSSAO(ssao);
                view.commitUniforms(driver);

                out.params.clearColor = view.getClearColor();

                if (jobFroxelize) {
                    auto sync = jobFroxelize;
                    js.waitAndRelease(sync);
                    view.commitFroxels(driver);
                }

                pass.execute(resources.getPassName(), out.target, out.params,
                        colorPassBegin, colorPassEnd);

                // Unbind the SSAO sampler, as the frame graph will delete the texture at the end of
                // the pass.
                view.cleanupSSAO();
            });

    jobFroxelize = nullptr;
    FrameGraphId<FrameGraphTexture> input = colorPass.getData().color;

    /*
     * Post Processing...
     */

    const bool translucent = mSwapChain->isTransparent();
    const TextureFormat ldrFormat = (toneMapping && fxaa) ?
            TextureFormat::RGBA8 : getLdrFormat(); // e.g. RGB8 or RGBA8

    if (hasPostProcess) {
        // FIXME: currently we can't render a view on top of another one (with transparency) if
        //        any post-processing is performed on that view -- this is because post processing
        //        uses intermediary buffers which are not blended back (they're blitted).

        if (toneMapping) {
            input = ppm.toneMapping(fg, input, ldrFormat, dithering, translucent);
        }
        if (fxaa) {
            input = ppm.fxaa(fg, input, ldrFormat, !toneMapping || translucent);
        }
        if (scaled) {
            input = ppm.dynamicScaling(fg, input, ldrFormat);
        }
    }

    // If we're rendering into the default RenderTarget (viewRenderTarget), we must take care
    // of a few things:
    // - since viewRenderTarget doesn't have depth or multi-sample buffer, we have to use an
    //   intermediate buffer which has one. We do this by forcing a blit.
    // - however a blit operation cannot move or scale the source, so we must additionally
    //   do a resolve pass in the multi-sample case.
    if (input == colorPass.getData().color) {
        if (msaa > 1) {
            input = ppm.resolve(fg, input);
            input = ppm.dynamicScaling(fg, input, ldrFormat);
        } else if (colorPassNeedsDepthBuffer) {
            input = ppm.dynamicScaling(fg, input, ldrFormat);
        }
    }

    fg.present(input);

    fg.moveResource(fgViewRenderTarget, input);

    fg.compile();
    //fg.export_graphviz(slog.d);

    fg.execute(engine, driver);

    commands.clear();

    recordHighWatermark(pass.getCommandsHighWatermark());
}

void FRenderer::copyFrame(FSwapChain* dstSwapChain, filament::Viewport const& dstViewport,
        filament::Viewport const& srcViewport, CopyFrameFlag flags) {
    SYSTRACE_CALL();

    assert(mSwapChain);
    assert(dstSwapChain);
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
        params.flags.clear = TargetBufferFlags::COLOR;
        params.clearColor = {0.f, 0.f, 0.f, 1.f};
        params.flags.discardStart = TargetBufferFlags::ALL;
        params.flags.discardEnd = TargetBufferFlags::NONE;
        params.viewport.left = 0;
        params.viewport.bottom = 0;
        params.viewport.width = std::numeric_limits<uint32_t>::max();
        params.viewport.height = std::numeric_limits<uint32_t>::max();
        params.flags.clear |= RenderPassFlags::IGNORE_SCISSOR;
    }
    driver.beginRenderPass(mRenderTarget, params);

    // Verify that the source swap chain is readable.
    assert(mSwapChain->isReadable());
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

bool FRenderer::beginFrame(FSwapChain* swapChain) {
    SYSTRACE_CALL();

    assert(swapChain);

    mFrameId++;

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

    int64_t monotonic_clock_ns (std::chrono::steady_clock::now().time_since_epoch().count());
    driver.beginFrame(monotonic_clock_ns, mFrameId);

    // This need to occur after the backend beginFrame() because some backends need to start
    // a command buffer before creating a fence.
    if (UTILS_HAS_THREADING) {
        mFrameInfoManager.beginFrame(mFrameId);
    }

    if (!mFrameSkipper.beginFrame()) {
        mFrameInfoManager.cancelFrame();
        driver.endFrame(mFrameId);
        engine.flush();
        return false;
    }

    // latch the frame time
    std::chrono::duration<double> time{ getUserTime() };
    float h = float(time.count());
    float l = float(time.count() - h);
    mShaderUserTime = { h, l, 0, 0 };

    // ask the engine to do what it needs to (e.g. updates light buffer, materials...)
    engine.prepare();

    return true;
}

void FRenderer::endFrame() {
    SYSTRACE_CALL();

    FEngine& engine = getEngine();
    FEngine::DriverApi& driver = engine.getDriverApi();

    FrameInfoManager& frameInfoManager = mFrameInfoManager;

    if (UTILS_HAS_THREADING) {

        // on debug builds this helps catching cases where we're writing to
        // the buffer form another thread, which is currently not allowed.
        driver.debugThreading();

        frameInfoManager.endFrame();
    }
    mFrameSkipper.endFrame();

    if (mSwapChain) {
        mSwapChain->commit(driver);
        mSwapChain = nullptr;
    }

    driver.endFrame(mFrameId);

    // do this before engine.flush()
    engine.getResourceAllocator().gc();

    // Run the component managers' GC in parallel
    // WARNING: while doing this we can't access any component manager
    auto& js = engine.getJobSystem();

    auto job = js.runAndRetain(jobs::createJob(js, nullptr, &FEngine::gc, &engine)); // gc all managers

    engine.flush();     // flush command stream

    // make sure we're done with the gcs
    js.waitAndRelease(job);

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
        backend::PixelBufferDescriptor&& buffer) {

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

backend::Handle<backend::HwRenderTarget> FRenderer::getRenderTarget(FView& view) const noexcept {
    backend::Handle<backend::HwRenderTarget> viewRenderTarget = view.getRenderTarget();
    return viewRenderTarget ? viewRenderTarget : mRenderTarget;
}

RenderPass::CommandTypeFlags FRenderer::getCommandType(View::DepthPrepass prepass) const noexcept {
    RenderPass::CommandTypeFlags commandType;
    switch (prepass) {
        case View::DepthPrepass::DEFAULT:
            // TODO: better default strategy (can even change on a per-frame basis)
            commandType = RenderPass::COLOR_WITH_DEPTH_PREPASS;
#if defined(ANDROID) || defined(__EMSCRIPTEN__)
            commandType = RenderPass::COLOR;
#endif
            break;
        case View::DepthPrepass::DISABLED:
            commandType = RenderPass::COLOR;
            break;
        case View::DepthPrepass::ENABLED:
            commandType = RenderPass::COLOR_WITH_DEPTH_PREPASS;
            break;
    }
    return commandType;
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

void Renderer::copyFrame(SwapChain* dstSwapChain, filament::Viewport const& dstViewport,
        filament::Viewport const& srcViewport, CopyFrameFlag flags) {
    upcast(this)->copyFrame(upcast(dstSwapChain), dstViewport, srcViewport, flags);
}

void Renderer::readPixels(uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
        backend::PixelBufferDescriptor&& buffer) {
    upcast(this)->readPixels(xoffset, yoffset, width, height, std::move(buffer));
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

} // namespace filament
