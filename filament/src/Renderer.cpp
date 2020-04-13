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
#include "details/Texture.h"
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
#include <private/filament/SibGenerator.h>

// this helps visualize what dynamic-scaling is doing
#define DEBUG_DYNAMIC_SCALING false

using namespace filament::math;
using namespace utils;

namespace filament {

using namespace backend;

namespace details {

FRenderer::FRenderer(FEngine& engine) :
        mEngine(engine),
        mFrameSkipper(engine, 1u),
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
    mFrameInfoManager.terminate();
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

    if (mBeginFrameInternal) {
        mBeginFrameInternal();
        mBeginFrameInternal = {};
    }

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
    float2 scale = view.updateScale(mFrameInfoManager.getLastFrameInfo());
    const View::QualityLevel upscalingQuality = view.getDynamicResolutionOptions().quality;
    auto aoOptions = view.getAmbientOcclusionOptions();
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
    if (view.hasFog())                     renderFlags |= RenderPass::HAS_FOG;
    if (view.isFrontFaceWindingInverted()) renderFlags |= RenderPass::HAS_INVERSE_FRONT_FACES;
    pass.setRenderFlags(renderFlags);

    /*
     * Shadow pass
     */

    if (view.hasShadowing()) {
        // TODO: use the framegraph for the shadow passes
        RenderPass shadowMapPass = pass;
        view.renderShadowMaps(engine, driver, shadowMapPass);
        engine.flush(); // Wake-up the driver thread
    }

    /*
     * Frame graph
     */

    FrameGraph fg(engine.getResourceAllocator());

    const TargetBufferFlags discardedFlags = mDiscardedFlags;
    const TargetBufferFlags clearFlags = mClearFlags;
    const float4 clearColor = mClearOptions.clearColor;

    // Renderer's ClearOptions apply once at the beginning of the frame (not for each View),
    // however, it's implemented as part of executing a render pass on the current render target,
    // and that happens for each View. So we need to disable clearing after the 1st View has
    // been processed.
    mDiscardedFlags &= ~TargetBufferFlags::COLOR;
    mClearFlags &= ~TargetBufferFlags::COLOR;

    const Handle<HwRenderTarget> viewRenderTarget = getRenderTarget(view);
    FrameGraphRenderTargetHandle fgViewRenderTarget = fg.import<FrameGraphRenderTarget>(
            "viewRenderTarget", {
                    .viewport = DEBUG_DYNAMIC_SCALING ? svp : vp,
                    .clearColor = clearColor,
                    .clearFlags = clearFlags
            }, {
                    .target = viewRenderTarget,
                    .params = {
                            .flags = { .discardStart = discardedFlags },
                    }});


    const bool blending = view.getBlendMode() == View::BlendMode::TRANSLUCENT;
    // If the swapchain is transparent or if we blend into it, we need to allocate our intermediate
    // buffers with an alpha channel.
    // FIXME: this doesn't work when the target is a user provided rendertarget
    const bool translucent = mSwapChain->isTransparent() || blending;
    const TextureFormat hdrFormat = getHdrFormat(view, translucent);

    const ColorPassConfig config {
            .vp = vp,
            .svp = svp,
            .scale = scale,
            .hdrFormat = hdrFormat,
            .msaa = msaa,
            .clearFlags = clearFlags,
            .clearColor = clearColor,
            .hasContactShadows = scene.hasContactShadows()
    };

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
    // structure pass -- automatically culled if not used
    // Currently it consists of a simple depth pass.
    // This is normally used by SSAO and contact-shadows

    // TODO: this should be a FrameGraph pass to participate to automatic culling
    pass.newCommandBuffer();
    pass.appendCommands(RenderPass::CommandTypeFlags::SSAO);
    pass.sortCommands();

    ppm.structure(fg, pass, svp.width, svp.height, aoOptions.resolution);

    // --------------------------------------------------------------------------------------------
    // SSAO pass

    const bool useSSAO = view.getAmbientOcclusion() != View::AmbientOcclusion::NONE;
    if (useSSAO) {
        // we could rely on FrameGraph culling, but this creates unnecessary CPU work
        ppm.screenSpaceAmbientOclusion(fg, pass, svp, cameraInfo, aoOptions);
    }

    // --------------------------------------------------------------------------------------------
    // Color passes

    // TODO: ideally this should be a FrameGraph pass to participate to automatic culling
    pass.newCommandBuffer();
    pass.appendCommands(RenderPass::COLOR);
    pass.sortCommands();

    // We use a framegraph pass to wait for froxelization to finish (so it can be done
    // in parallel with .compile()
    fg.addTrivialSideEffectPass("Prepare Color Passes", [=, &js, &view](DriverApi& driver) {
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
    colorPass(fg, "Color Pass", desc, config, pass, view);

    // TODO: look for refraction draw calls only if screen-space refraction is enabled
    FrameGraphId<FrameGraphTexture> colorPassOutput = refractionPass(fg, config, pass, view);
    FrameGraphId<FrameGraphTexture> input = colorPassOutput;

    fg.addTrivialSideEffectPass("Finish Color Passes", [&view](DriverApi& driver) {
        // Unbind SSAO sampler, b/c the FrameGraph will delete the texture at the end of the pass.
        view.cleanupRenderPasses();
        view.commitUniforms(driver);
    });

    // --------------------------------------------------------------------------------------------
    // Post Processing...

    const TextureFormat ldrFormat = (toneMapping && fxaa) ?
            TextureFormat::RGBA8 : getLdrFormat(translucent); // e.g. RGB8 or RGBA8

    if (hasPostProcess) {
        if (toneMapping) {
            input = ppm.toneMapping(fg, input, ldrFormat, translucent, fxaa, scale,
                    view.getBloomOptions(), dithering);
        }
        if (fxaa) {
            input = ppm.fxaa(fg, input, ldrFormat, !toneMapping || translucent);
        }
        if (scaled) {
            if (UTILS_LIKELY(!blending && upscalingQuality == View::QualityLevel::LOW)) {
                input = ppm.opaqueBlit(fg, input, { .format = ldrFormat });
            } else {
                input = ppm.blendBlit(fg, true, upscalingQuality, input,
                        { .format = ldrFormat });
            }
        }
    }

    // We need to do special processing when rendering directly into the swap-chain, that is when
    // the viewRenderTarget is the default render target (mRenderTarget) and we're rendering into
    // it. This is because the default render target is not multi-sampled, so we need an
    // intermediate buffer when MSAA is enabled.
    // We also need an extra buffer for blending the result to the framebuffer if the view
    // is translucent.
    // The intermediate buffer is accomplished with a "fake" opaqueBlit (i.e. blit)
    // operation.

    const bool outputIsInput = fg.equal(input, colorPassOutput);
    if ((outputIsInput && viewRenderTarget == mRenderTarget && msaa > 1) ||
        (!outputIsInput && blending)) {
        if (UTILS_LIKELY(!blending && upscalingQuality == View::QualityLevel::LOW)) {
            input = ppm.opaqueBlit(fg, input, { .format = ldrFormat });
        } else {
            input = ppm.blendBlit(fg, true, upscalingQuality, input,
                    { .format = ldrFormat });
        }
    }

    auto output = input;
    fg.present(output);
    fg.moveResource(fgViewRenderTarget, output);
    fg.compile();
    //fg.export_graphviz(slog.d);
    fg.execute(engine, driver);

    recordHighWatermark(pass.getCommandsHighWatermark());
}

FrameGraphId<FrameGraphTexture> FRenderer::refractionPass(FrameGraph& fg,
        ColorPassConfig config, RenderPass const& pass,
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
        input = colorPass(fg, "Color Pass (opaque)", desc, config, opaquePass, view);

        // vvv the actual bloom pass starts below vvv

        // scale factor for the gaussian so it matches our resolution / FOV
        const float verticalFieldOfView = view.getCameraUser().getFieldOfView(Camera::Fov::VERTICAL);
        const float s = verticalFieldOfView / desc.height;

        // The kernel-size was determined empirically so that we don't get too many artifacts
        // due to the down-sampling with a box filter (which happens implicitly).
        // e.g.: size of 13 (4 stored coefficients)
        //      +-------+-------+-------*===*-------+-------+-------+
        //  ... | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | ...
        //      +-------+-------+-------*===*-------+-------+-------+
        const size_t kernelSize = 21;   // requires only 6 stored coefficients and 11 tap/pass
        static_assert(kernelSize & 1u, "kernel size must be odd");
        static_assert((((kernelSize - 1u) / 2u) & 1u) == 0, "kernel positive side size must be even");

        // The relation between n and sigma (variance) is 6*sigma - 1 = N
        const float sigma0 = (kernelSize + 1) / 6.0f;

        // The variance doubles each time we go one mip down, so the relation between LOD and
        // sigma is: lod = log2(sigma/sigma0).
        // sigma is deduced from the roughness: roughness = sqrt(2) * s * sigma
        // In the end we get: lod = 2 * log2(perceptualRoughness) - log2(sigma0 * s * sqrt2)
        const float refractionLodOffset = -std::log2(sigma0 * s * (float)F_SQRT2);
        const float maxPerceptualRoughness = 0.5f;
        const uint8_t maxLod = std::ceil(2.0f * std::log2(maxPerceptualRoughness) + refractionLodOffset);

        // Number of roughness levels we want.
        // TODO: If we want to limit the number of mip levels, we must reduce the initial
        //       resolution (if we want to keep the same filter, and still match the IBL somewhat).
        const uint8_t roughnessLodCount =
                std::min(maxLod, FTexture::maxLevelCount(desc.width, desc.height));

        // First we need to resolve the MSAA buffer if enabled
        input = ppm.resolve(fg, "Resolved Color Buffer", input);

        // Then copy the color buffer into a texture, and make sure to scale it back properly.
        uint32_t w = config.svp.width;
        uint32_t h = config.svp.height;
        if (config.scale.x < config.scale.y) {
            // we're downscaling more horizontally
            w = config.vp.width * config.scale.y;
        } else {
            // we're downscaling more vertically
            h = config.vp.height * config.scale.x;
        }

        input = ppm.opaqueBlit(fg, input, {
                .width = w,
                .height = h,
                .levels = roughnessLodCount,
                .format = TextureFormat::R11F_G11F_B10F,
        });

        input = ppm.generateGaussianMipmap(fg, input, roughnessLodCount, true, kernelSize);
        blackboard["ssr"] = input;

        // ^^^ the actual bloom pass ends above ^^^

        // set-up the refraction pass
        RenderPass translucentPass(pass);
        translucentPass.getCommands().set(
                const_cast<Command*>(refraction),
                const_cast<Command*>(pass.end()));

        config.refractionLodOffset = refractionLodOffset;
        config.clearFlags = TargetBufferFlags::NONE;
        output = colorPass(fg, "Color Pass (transparent)", desc, config, translucentPass, view);

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
        RenderPass const& pass, FView const& view) const noexcept {

    struct ColorPassData {
        FrameGraphId<FrameGraphTexture> color;
        FrameGraphId<FrameGraphTexture> depth;
        FrameGraphId<FrameGraphTexture> ssao;
        FrameGraphId<FrameGraphTexture> ssr;
        FrameGraphId<FrameGraphTexture> structure;
        FrameGraphRenderTargetHandle rt{};
        float4 clearColor;
    };

    auto& colorPass = fg.addPass<ColorPassData>(name,
            [&](FrameGraph::Builder& builder, ColorPassData& data) {
                data.clearColor = config.clearColor;

                TargetBufferFlags clearDepthFlags = TargetBufferFlags::NONE;
                TargetBufferFlags clearColorFlags = TargetBufferFlags::NONE;
                Blackboard& blackboard = fg.getBlackboard();

                data.ssr  = blackboard.get<FrameGraphTexture>("ssr");
                data.ssao = blackboard.get<FrameGraphTexture>("ssao");
                data.color = blackboard.get<FrameGraphTexture>("color");
                data.structure = blackboard.get<FrameGraphTexture>("structure");

                if (config.hasContactShadows) {
                    assert(data.structure.isValid());
                    data.structure = builder.sample(data.structure);
                }

                if (data.ssr.isValid()) {
                    data.ssr = builder.sample(data.ssr);
                }

                if (data.ssao.isValid()) {
                    data.ssao = builder.sample(data.ssao);
                }

                if (!data.color.isValid()) {
                    if (!mClearOptions.clear && !view.isSkyboxVisible()) {
                        clearColorFlags = TargetBufferFlags::COLOR;
                        data.clearColor = {};
                    }
                    data.color = builder.createTexture("Color Buffer", colorBufferDesc);
                }

                if (!data.depth.isValid()) {
                    // clear newly allocated depth buffers, regardless of given clear flags
                    clearDepthFlags = TargetBufferFlags::DEPTH;
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
                        .clearFlags = config.clearFlags | clearColorFlags | clearDepthFlags });
            },
            [=, &view](FrameGraphPassResources const& resources,
                            ColorPassData const& data, DriverApi& driver) {

                // set samplers and uniforms
                PostProcessManager& ppm = getEngine().getPostProcessManager();
                view.prepareSSAO(data.ssao.isValid() ? resources.getTexture(data.ssao) : ppm.getOneTexture());
                if (data.structure.isValid()) {
                    view.prepareStructure(resources.getTexture(data.structure));
                }
                if (data.ssr.isValid()) {
                    view.prepareSSR(resources.getTexture(data.ssr), config.refractionLodOffset);
                }
                view.commitUniforms(driver);

                auto out = resources.get(data.rt);
                out.params.clearColor = data.clearColor;

                pass.execute(resources.getPassName(), out.target, out.params);

                // color pass is typically heavy and we don't have much CPU work left after
                // this point, so flushing now allows us to start the GPU earlier and reduce
                // latency, without creating bubbles.
                driver.flush();
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

bool FRenderer::beginFrame(FSwapChain* swapChain, uint64_t vsyncSteadyClockTimeNano,
        backend::FrameFinishedCallback callback, void* user) {
    assert(swapChain);

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

    // latch the frame time
    std::chrono::duration<double> time(appVsync - mUserEpoch);
    float h = float(time.count());
    float l = float(time.count() - h);
    mShaderUserTime = { h, l, 0, 0 };

    // We always discard and clear the depth+stencil buffers -- we don't allow sharing these
    // across views (clear implies discard)
    mDiscardedFlags = ((mClearOptions.discard || mClearOptions.clear) ?
                       TargetBufferFlags::COLOR : TargetBufferFlags::NONE)
                      | TargetBufferFlags::DEPTH_AND_STENCIL;

    mClearFlags = (mClearOptions.clear ? TargetBufferFlags::COLOR : TargetBufferFlags::NONE)
                  | TargetBufferFlags::DEPTH_AND_STENCIL;

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

        driver.beginFrame(appVsync.time_since_epoch().count(), mFrameId, callback, user);

        // This need to occur after the backend beginFrame() because some backends need to start
        // a command buffer before creating a fence.
        mFrameInfoManager.beginFrame({
                .targetFrameTime = FrameInfo::duration{
                        float(mFrameRateOptions.interval) / mDisplayInfo.refreshRate },
                .headRoomRatio = mFrameRateOptions.headRoomRatio,
                .oneOverTau = mFrameRateOptions.scaleRate,
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
            steady_clock::time_point deadline = desiredPresentationTime - presentationDeadline;

            // one important thing is to make sure that the deadline is comfortably later than
            // when the gpu will finish, otherwise we'll have inconsistent latency/frames.

            // TODO: evaluate if we can make it in time, and if not why.
            // If the problem is cpu+gpu latency we can try to push the desired presentation time
            // further away, but this has limits, as only 2 buffers are dequeuable.
            // If the problem is the gpu is overwhelmed, then we need to
            //  - see if there is more headroom in dynamic resolution
            //  - or start skipping frames. Ideally lower the framerate too.

            // presentation time is set to the middle of the period we're interested in
            steady_clock::time_point presentationTime = desiredPresentationTime - refreshPeriod / 2;
            driver.setPresentationTime(presentationTime.time_since_epoch().count());
        }

        // ask the engine to do what it needs to (e.g. updates light buffer, materials...)
        engine.prepare();
    };

    if (mFrameSkipper.beginFrame()) {
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

    mFrameInfoManager.endFrame();
    mFrameSkipper.endFrame();

    if (mSwapChain) {
        mSwapChain->commit(driver);
        mSwapChain = nullptr;
    }

    driver.endFrame(mFrameId);

    // gives the backend a chance to execute periodic tasks
    driver.tick();

    // do this before engine.flush()
    engine.getResourceAllocator().gc();

    // Run the component managers' GC in parallel
    // WARNING: while doing this we can't access any component manager
    auto& js = engine.getJobSystem();

    auto job = js.runAndRetain(jobs::createJob(js, nullptr, &FEngine::gc, &engine)); // gc all managers

    engine.flush();     // flush command stream

    // make sure we're done with the gcs
    js.waitAndRelease(job);
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

bool Renderer::beginFrame(SwapChain* swapChain, uint64_t vsyncSteadyClockTimeNano,
        backend::FrameFinishedCallback callback, void* user) {
    return upcast(this)->beginFrame(upcast(swapChain), vsyncSteadyClockTimeNano, callback, user);
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

} // namespace filament
