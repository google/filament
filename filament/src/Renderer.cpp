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
#include "fg/FrameGraphPassResources.h"
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
        Fence::waitAndDestroy(engine.createFence(FFence::Type::SOFT));
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

TextureFormat FRenderer::getHdrFormat(const View& view, bool translucent) const noexcept {
    if (translucent) {
        return mHdrTranslucent;
    }
    switch (view.getRenderQuality().hdrColorBuffer) {
        case View::QualityLevel::LOW:
        case View::QualityLevel::MEDIUM:
            return mHdrQualityMedium;
        case View::QualityLevel::HIGH:
        case View::QualityLevel::ULTRA: {
            return mHdrQualityHigh;
        }
    }
}

TextureFormat FRenderer::getLdrFormat(bool translucent) const noexcept {
    return (translucent || !mIsRGB8Supported) ? TextureFormat::RGBA8 : TextureFormat::RGB8;
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
     * Allocate command buffer
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
        // TODO: use the framegraph for the shadow passes
        RenderPass shadowMapPass = pass;
        view.getShadowMap().render(driver, shadowMapPass, view);
        driver.flush(); // Kick the GPU since we're done with this render target
        engine.flush(); // Wake-up the driver thread
    }

    /*
     * Frame graph
     */

    FrameGraph fg(engine.getResourceAllocator());

    // FIXME: when the view doesn't ask for a clear, but it's drawn in an intermediate buffer
    //        that buffer needs to be cleared with transparent pixels if blending is enabled
    const TargetBufferFlags clearFlags = (view.getClearFlags() & TargetBufferFlags::COLOR)
                                   | TargetBufferFlags::DEPTH;

    const TargetBufferFlags discardedFlags = view.getDiscardedTargetBuffers();

    const float4 clearColor = view.getClearColor();

    // Figure out if we need to blend this view into the framebuffer. Maybe this should be
    // explicit, but since we don't have an API right now, we use heuristics:
    // - we are keeping the color buffer before rendering, and
    // - we are not clearing or clearing with alpha
    // FIXME: make this an explicit API
    const bool blending = !(discardedFlags & TargetBufferFlags::COLOR)
            && (!(clearFlags & TargetBufferFlags::COLOR) || clearColor.a < 1.0);

    // If the swapchain is transparent or if we blend into it, we need to allocate our intermediate
    // buffers with an alpha channel.
    // FIXME: this doesn't work when the target is a user provided rendertarget
    const bool translucent = mSwapChain->isTransparent() || blending;

    const TextureFormat hdrFormat = getHdrFormat(view, translucent);

    const Handle<HwRenderTarget> viewRenderTarget = getRenderTarget(view);
    FrameGraphRenderTargetHandle fgViewRenderTarget = fg.importRenderTarget("viewRenderTarget",
            { .viewport = vp }, viewRenderTarget, vp.width, vp.height, discardedFlags);

    /*
     * Depth + Color passes
     */

    CameraInfo const& cameraInfo = view.getCameraInfo();
    pass.setCamera(cameraInfo);
    pass.setGeometry(scene.getRenderableData(), view.getVisibleRenderables(), scene.getRenderableUBO());

    view.updatePrimitivesLod(engine, cameraInfo,scene.getRenderableData(), view.getVisibleRenderables());
    view.prepareCamera(cameraInfo, svp);
    view.commitUniforms(driver);

    // --------------------------------------------------------------------------------------------
    // SSAO pass

    const bool useSSAO = view.getAmbientOcclusion() != View::AmbientOcclusion::NONE;
    if (useSSAO) {
        // don't generate commands if we don't have SSAO
        // TODO: ideally this should be a FrameGraph pass to participate to automatic culling
        pass.newCommandBuffer();
        pass.appendCommands(RenderPass::CommandTypeFlags::SSAO);
        pass.sortCommands();
    }

    // SSAO pass -- automatically culled if not used
    FrameGraphId<FrameGraphTexture> ssao = ppm.ssao(fg, pass, svp, cameraInfo,
            view.getAmbientOcclusionOptions());

    // --------------------------------------------------------------------------------------------
    // Color passes

    // TODO: ideally this should be a FrameGraph pass to participate to automatic culling
    pass.newCommandBuffer();
    pass.appendCommands(RenderPass::COLOR);
    pass.sortCommands();

    const ColorPassConfig config {
            .svp = svp,
            .hdrFormat = hdrFormat,
            .msaa = msaa
    };

    // We use a framegraph pass to commit the View's uniforms and wait for froxelization to finish
    struct PrepareColorPassesData {
        FrameGraphId<FrameGraphTexture> ssao;
    };
    fg.addPass<PrepareColorPassesData>("Prepare Color Passes",
            [&fg, useSSAO, ssao](FrameGraph::Builder& builder, auto& data) {
                if (useSSAO) {
                    data.ssao = builder.sample(ssao);
                }
                fg.getBlackboard().put("ssao", data.ssao);
                builder.sideEffect();
            },
            [&ppm, &js, &view, jobFroxelize]
                    (FrameGraphPassResources const& resources, auto const& data, DriverApi& driver) {
                view.prepareSSAO(data.ssao.isValid() ? resources.getTexture(data.ssao)
                                                     : ppm.getNoSSAOTexture());
                view.commitUniforms(driver);
                if (jobFroxelize) {
                    auto sync = jobFroxelize;
                    js.waitAndRelease(sync);
                    view.commitFroxels(driver);
                }
            });

    FrameGraphTexture::Descriptor desc = {
            .width = config.svp.width,
            .height = config.svp.height,
            .format = config.hdrFormat
    };
    colorPass(fg, "Color Pass", desc, config, pass, clearFlags, clearColor);

    // TODO: look for refraction draw calls only if screen-space refraction is enabled
    FrameGraphId<FrameGraphTexture> colorPassOutput =
            refractionPass(fg, config, pass, view, clearFlags);

    FrameGraphId<FrameGraphTexture> input = colorPassOutput;

    fg.addTrivialSideEffectPass("Finish Color Passes", [&view]() {
        // Unbind SSAO sampler, b/c the FrameGraph will delete the texture at the end of the pass.
        view.cleanupSSAO();
        view.cleanupSSR();
    });

    // --------------------------------------------------------------------------------------------
    // Post Processing...

    const TextureFormat ldrFormat = (toneMapping && fxaa) ?
            TextureFormat::RGBA8 : getLdrFormat(translucent); // e.g. RGB8 or RGBA8

    if (hasPostProcess) {
        if (toneMapping) {
            input = ppm.toneMapping(fg, input, ldrFormat, dithering, translucent, fxaa);
        }
        if (fxaa) {
            input = ppm.fxaa(fg, input, ldrFormat, !toneMapping || translucent);
        }
        if (scaled) {
            input = ppm.dynamicScaling(fg, msaa, scaled, blending, input, ldrFormat);
        }
    }

    // We need to do special processing when rendering directly into the swap-chain, that is when
    // the viewRenderTarget is the default render target (mRenderTarget) and we're rendering into
    // it. This is because the default render target is not multi-sampled, so we need an
    // intermediate buffer when MSAA is enabled.
    // We also need an extra buffer for blending the result to the framebuffer if the view
    // is translucent.
    // The intermediate buffer is accomplished with a "fake" dynamicScaling (i.e. blit)
    // operation.

    const bool outputIsInput = fg.equal(input, colorPassOutput);
    if ((outputIsInput && viewRenderTarget == mRenderTarget && msaa > 1) ||
        (!outputIsInput && blending)) {
        input = ppm.dynamicScaling(fg, msaa, scaled, blending, input, ldrFormat);
    }

    fg.present(input);
    fg.moveResource(fgViewRenderTarget, input);
    fg.compile();
    //fg.export_graphviz(slog.d);
    fg.execute(engine, driver);

    recordHighWatermark(pass.getCommandsHighWatermark());
}

FrameGraphId<FrameGraphTexture> FRenderer::refractionPass(FrameGraph& fg,
        ColorPassConfig const& config, RenderPass const& pass,
        FView const& view, TargetBufferFlags clearFlags) const noexcept {

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

    if (UTILS_UNLIKELY(hasScreenSpaceRefraction)) {
        PostProcessManager& ppm = mEngine.getPostProcessManager();
        // clear the color/depth buffers, which will orphan (and cull) the color pass
        input.clear();
        blackboard.remove("color");
        blackboard.remove("depth");

        RenderPass opaquePass(pass);
        opaquePass.getCommands().set(
                const_cast<Command*>(pass.begin()),
                const_cast<Command*>(refraction));

        FrameGraphTexture::Descriptor desc = {
                .width = config.svp.width,
                .height = config.svp.height,
                .samples = config.msaa,  // we need to conserve the sample buffer
                .format = config.hdrFormat
        };
        input = colorPass(fg, "Color Pass (opaque)", desc,
                config, opaquePass, clearFlags, view.getClearColor());

        // scale factor for the gaussian so it matches our resolution / FOV
        const float verticalFieldOfView = view.getCameraUser().getFieldOfView(Camera::Fov::VERTICAL);
        const float s = verticalFieldOfView / desc.height;

        // The kernel-size was determined empirically so that we don't get too many artifacts
        // due to the down-sampling with a box filter (which happens implicitly).
        // e.g.: size of 13 (4 stored coefficients)
        //      +-------+-------+-------*===*-------+-------+-------+
        //  ... | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | ...
        //      +-------+-------+-------*===*-------+-------+-------+
        const size_t kernelSize = 17;   // requires only 5 stored coefficients and 9 tap/pass
        static_assert(kernelSize & 1, "kernel size must be odd");
        static_assert((((kernelSize - 1) / 2) & 1) == 0, "kernel positive side size must be even");

        // The relation between n and sigma (variance) should 6*sigma - 1 = N, however here we
        // use 4*sigma - 1 = N, which gives a stronger blur, without bringing too many artifacts.
        const float sigma0 = (kernelSize + 1) / 4.0f;

        // The variance doubles each time we go one mip down, so the relation between LOD and
        // sigma is: lod = log2(sigma/sigma0).
        // sigma is deduced from the roughness: roughness = sqrt(2) * s * sigma
        // In the end we get: lod = 2 * log2(perceptualRoughness) - log2(sigma0 * s * sqrt2)
        const float refractionLodOffset = -std::log2(sigma0 * s * (float)F_SQRT2);
        const float maxPerceptualRoughness = 0.5f;
        const uint32_t maxLod = std::ceil(2.0f * std::log2(maxPerceptualRoughness) + refractionLodOffset);

        // Number of roughness levels we want.
        // TODO: If we want to limit the number of mip levels, we must reduce the initial
        //       resolution (if we want to keep the same filter, and still match the IBL somewhat).
        size_t roughnessLodCount =
                std::min(maxLod, (uint32_t)std::ilogbf(std::max(desc.width, desc.height))) + 1u;

        // Copy the color buffer into a texture, we use resolve() because in case of a multi-sample
        // buffer, it'll also resolve it.
        input = ppm.resolve(fg, "Refraction Buffer",
                roughnessLodCount, TextureFormat::R11F_G11F_B10F, input);

        input = ppm.generateGaussianMipmap(fg, input, roughnessLodCount, kernelSize, sigma0);

        struct PrepareSSRData {
            FrameGraphId<FrameGraphTexture> ssr;
        };
        fg.addPass<PrepareSSRData>("Prepare SSR",
                [&](FrameGraph::Builder& builder, auto& data) {
                    data.ssr = builder.sample(input);
                    blackboard["ssr"] = data.ssr;
                    builder.sideEffect();
                },
                [&view, refractionLodOffset]
                (FrameGraphPassResources const& resources, auto const& data, DriverApi& driver) {
                    view.prepareSSR(resources.getTexture(data.ssr), refractionLodOffset);
                    view.commitUniforms(driver);
                });

        // set-up the refraction pass
        RenderPass translucentPass(pass);
        translucentPass.getCommands().set(
                const_cast<Command*>(refraction),
                const_cast<Command*>(pass.end()));

        output = colorPass(fg, "Color Pass (transparent)", desc,
                config, translucentPass, TargetBufferFlags::NONE);

        if (config.msaa > 1) {
            // We need to do a resolve here because later passes (such as tonemapping) will need
            // to sample from 'output'. However, because we have MSAA, we know we're not sampleable.
            // And this is because in the SSR case, we had to use a renderbuffer to conserve the
            // multi-sample buffer.
            output = ppm.resolve(fg, "Resolved Color Buffer", output);
        }
    } else {
        output = input;
    }
    return output;
}

FrameGraphId<FrameGraphTexture> FRenderer::colorPass(FrameGraph& fg, const char* name,
        FrameGraphTexture::Descriptor const& colorBufferDesc, ColorPassConfig const& config,
        RenderPass const& pass, backend::TargetBufferFlags clearFlags,
        math::float4 clearColor) noexcept {

    struct ColorPassData {
        FrameGraphId<FrameGraphTexture> color;
        FrameGraphId<FrameGraphTexture> depth;
        FrameGraphId<FrameGraphTexture> ssao;
        FrameGraphId<FrameGraphTexture> ssr;
        FrameGraphRenderTargetHandle rt{};
    };

    auto& colorPass = fg.addPass<ColorPassData>(name,
            [&](FrameGraph::Builder& builder, ColorPassData& data) {

                Blackboard& blackboard = fg.getBlackboard();

                data.ssr  = blackboard.get<FrameGraphTexture>("ssr");
                data.ssao = blackboard.get<FrameGraphTexture>("ssao");
                data.color = blackboard.get<FrameGraphTexture>("color");
                data.depth = blackboard.get<FrameGraphTexture>("depth");

                if (data.ssr.isValid()) {
                    data.ssr = builder.sample(data.ssr);
                }

                if (data.ssao.isValid()) {
                    data.ssao = builder.sample(data.ssao);
                }

                if (!data.color.isValid()) {
                    data.color = builder.createTexture("Color Buffer", colorBufferDesc);
                }

                if (!data.depth.isValid()) {
                    data.depth = builder.createTexture("Depth Buffer", {
                            .width = colorBufferDesc.width,
                            .height = colorBufferDesc.height,
                            .format = TextureFormat::DEPTH24
                    });
                }

                data.color = builder.write(builder.read(data.color));
                data.depth = builder.write(builder.read(data.depth));

                blackboard["color"] = data.color;
                blackboard["depth"] = data.depth;

                data.rt = builder.createRenderTarget("Color Pass Target", {
                        .attachments = { data.color, data.depth },
                        .samples = config.msaa,
                }, clearFlags);
            },
            [pass, clearColor]
                    (FrameGraphPassResources const& resources,
                            ColorPassData const& data, DriverApi& driver) {
                auto out = resources.getRenderTarget(data.rt);
                out.params.clearColor = clearColor;

                pass.execute(resources.getPassName(), out.target, out.params);
            });

    return colorPass.getData().color;
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
        params.clearColor = {0.f, 0.f, 0.f, 1.f};
        params.flags.clear = TargetBufferFlags::COLOR;
        params.flags.ignoreScissor = true;
        params.flags.discardStart = TargetBufferFlags::ALL;
        params.flags.discardEnd = TargetBufferFlags::NONE;
        params.viewport.left = 0;
        params.viewport.bottom = 0;
        params.viewport.width = std::numeric_limits<uint32_t>::max();
        params.viewport.height = std::numeric_limits<uint32_t>::max();
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

bool FRenderer::beginFrame(FSwapChain* swapChain, backend::FrameFinishedCallback callback,
        void* user) {
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

    mSwapChain = swapChain;
    swapChain->makeCurrent(driver);

    // NOTE: this makes synchronous calls to the driver
    driver.updateStreams(&driver);

    int64_t monotonic_clock_ns (std::chrono::steady_clock::now().time_since_epoch().count());
    driver.beginFrame(monotonic_clock_ns, mFrameId, callback, user);

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
        PixelBufferDescriptor&& buffer) {
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

Handle<HwRenderTarget> FRenderer::getRenderTarget(FView& view) const noexcept {
    Handle<HwRenderTarget> viewRenderTarget = view.getRenderTargetHandle();
    return viewRenderTarget ? viewRenderTarget : mRenderTarget;
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

bool Renderer::beginFrame(SwapChain* swapChain, backend::FrameFinishedCallback callback,
        void* user) {
    return upcast(this)->beginFrame(upcast(swapChain), callback, user);
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

} // namespace filament
