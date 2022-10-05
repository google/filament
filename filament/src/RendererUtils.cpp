/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include "RendererUtils.h"

#include "details/Engine.h"
#include "details/View.h"

#include "fg/FrameGraph.h"
#include "fg/FrameGraphId.h"
#include "fg/FrameGraphResources.h"

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Panic.h>

namespace filament {

using namespace backend;
using namespace math;

FrameGraphId<FrameGraphTexture> RendererUtils::colorPass(
        FrameGraph& fg, const char* name, FEngine& engine, FView const& view,
        FrameGraphTexture::Descriptor const& colorBufferDesc,
        ColorPassConfig const& config, PostProcessManager::ColorGradingConfig colorGradingConfig,
        RenderPass::Executor const& passExecutor) noexcept {

    struct ColorPassData {
        FrameGraphId<FrameGraphTexture> shadows;
        FrameGraphId<FrameGraphTexture> color;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphId<FrameGraphTexture> depth;
        FrameGraphId<FrameGraphTexture> stencil;
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
                TargetBufferFlags clearStencilFlags = config.clearFlags & TargetBufferFlags::STENCIL;

                data.shadows = blackboard.get<FrameGraphTexture>("shadows");
                data.ssao = blackboard.get<FrameGraphTexture>("ssao");
                data.color = blackboard.get<FrameGraphTexture>("color");
                data.depth = blackboard.get<FrameGraphTexture>("depth");

                // Screen-space reflection or refractions
                if (config.hasScreenSpaceReflectionsOrRefractions) {
                    data.ssr = blackboard.get<FrameGraphTexture>("ssr");
                    if (data.ssr) {
                        data.ssr = builder.sample(data.ssr);
                    }
                }

                if (config.hasContactShadows) {
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
                    data.color = builder.createTexture("Color Buffer", colorBufferDesc);
                }

                const bool canResolveDepth = engine.getDriverApi().isAutoDepthResolveSupported();

                if (!data.depth) {
                    // clear newly allocated depth/stencil buffers, regardless of given clear flags
                    clearDepthFlags = TargetBufferFlags::DEPTH;
                    clearStencilFlags = config.enabledStencilBuffer ?
                            TargetBufferFlags::STENCIL : TargetBufferFlags::NONE;
                    const char* const name = config.enabledStencilBuffer ?
                             "Depth/Stencil Buffer" : "Depth Buffer";
                    TextureFormat format = config.enabledStencilBuffer ?
                            TextureFormat::DEPTH32F_STENCIL8 : TextureFormat::DEPTH32F;
                    data.depth = builder.createTexture(name, {
                            .width = colorBufferDesc.width,
                            .height = colorBufferDesc.height,
                            // If the color attachment requested MS, we assume this means the MS buffer
                            // must be kept, and for that reason we allocate the depth buffer with MS
                            // as well. On the other hand, if the color attachment was allocated without
                            // MS, no need to allocate the depth buffer with MS, if the RT is MS,
                            // the tile depth buffer will be MS, but it'll be resolved to single
                            // sample automatically -- which is what we want.
                            .samples = canResolveDepth ? colorBufferDesc.samples : uint8_t(config.msaa),
                            .format = format,
                    });
                    if (config.enabledStencilBuffer) {
                        data.stencil = data.depth;
                    }
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

                /*
                 * There is a bit of magic happening here regarding the viewport used.
                 * We do not specify the viewport in declareRenderPass() below, so it will be
                 * deduced automatically to be { 0, 0, w, h }, with w,h the min width/height of
                 * all the attachments. This has the side effect of moving the viewport to the
                 * origin and ignore the left/bottom of 'svp'. The attachment sizes are set from
                 * svp's width/height, however.
                 * But that's not all! When we're rendering directly into the swap-chain (by way
                 * of calling forwardResource() later), the effective viewport comes from the
                 * imported resource (i.e. the swap-chain) and is set to 'vp' which has its
                 * left/bottom honored -- the view is therefore rendered directly where it should
                 * be (the imported resource viewport is set to 'vp', see  how 'fgViewRenderTarget'
                 * is initialized in this file).
                 */
                FrameGraphRenderPass::Descriptor descr;
                descr.attachments.content.color[0] = data.color;
                descr.attachments.content.color[1] = data.output;
                descr.attachments.content.depth = data.depth;
                descr.samples = config.msaa;
                descr.clearFlags = clearColorFlags | clearDepthFlags;
                builder.declareRenderPass("Color Pass Target", descr);

                data.clearColor = config.clearColor;
                data.clearFlags = clearColorFlags | clearDepthFlags | clearStencilFlags;

                blackboard["depth"] = data.depth;
            },
            [=, &view, &engine](FrameGraphResources const& resources,
                    ColorPassData const& data, DriverApi& driver) {
                auto out = resources.getRenderPassInfo();

                // set samplers and uniforms
                view.prepareSSAO(data.ssao ?
                        resources.getTexture(data.ssao) : engine.getOneTextureArray());

                view.prepareShadowMap(view.getVsmShadowOptions().highPrecision);

                // set shadow sampler
                view.prepareShadow(data.shadows ?
                        resources.getTexture(data.shadows) : engine.getOneTextureArray());

                // set structure sampler
                view.prepareStructure(data.structure ?
                        resources.getTexture(data.structure) : engine.getOneTexture());

                // set screen-space reflections and screen-space refractions
                TextureHandle ssr = data.ssr ?
                        resources.getTexture(data.ssr) : engine.getOneTextureArray();

                view.prepareSSR(ssr, config.ssrLodOffset,
                        view.getScreenSpaceReflectionsOptions());

                // Note: here we can't use data.color's descriptor for the viewport because
                // the actual viewport might be offset when the target is the swapchain.
                // However, the width/height should be the same.
                assert_invariant(
                        out.params.viewport.width == resources.getDescriptor(data.color).width);
                assert_invariant(
                        out.params.viewport.height == resources.getDescriptor(data.color).height);

                view.prepareViewport(static_cast<filament::Viewport&>(out.params.viewport),
                        config.xoffset, config.yoffset);
                view.commitUniforms(driver);

                out.params.clearColor = data.clearColor;
                out.params.clearStencil = config.clearStencil;
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

                // color pass is typically heavy, and we don't have much CPU work left after
                // this point, so flushing now allows us to start the GPU earlier and reduce
                // latency, without creating bubbles.
                driver.flush();
            }
    );

    // when color grading is done as a subpass, the output of the color-pass is the ldr buffer
    auto output = colorGradingConfig.asSubpass ? colorPass->output : colorPass->color;

    blackboard["color"] = output;
    return output;
}

FrameGraphId<FrameGraphTexture> RendererUtils::refractionPass(
        FrameGraph& fg, FEngine& engine, FView const& view,
        ColorPassConfig config,
        PostProcessManager::ScreenSpaceRefConfig const& ssrConfig,
        PostProcessManager::ColorGradingConfig colorGradingConfig,
        RenderPass const& pass) noexcept {

    auto& blackboard = fg.getBlackboard();
    auto input = blackboard.get<FrameGraphTexture>("color");
    FrameGraphId<FrameGraphTexture> output;

    // find the first refractive object
    RenderPass::Command const* const refraction = std::partition_point(pass.begin(), pass.end(),
            [](auto const& command) {
                return (command.key & RenderPass::PASS_MASK) < uint64_t(RenderPass::Pass::REFRACT);
            });

    const bool hasScreenSpaceRefraction =
            (refraction->key & RenderPass::PASS_MASK) == uint64_t(RenderPass::Pass::REFRACT);

    // if there wasn't any refractive object, just skip everything below.
    if (UTILS_UNLIKELY(hasScreenSpaceRefraction)) {
        PostProcessManager& ppm = engine.getPostProcessManager();

        // clear the color/depth buffers, which will orphan (and cull) the color pass
        input.clear();
        blackboard.remove("color");
        blackboard.remove("depth");

        config.hasScreenSpaceReflectionsOrRefractions = true;

        input = RendererUtils::colorPass(fg, "Color Pass (opaque)", engine, view, {
                        // When rendering the opaques, we need to conserve the sample buffer,
                        // so create config that specifies the sample count.
                        .width = config.width,
                        .height = config.height,
                        .samples = config.msaa,
                        .format = config.hdrFormat
                }, config, { .asSubpass = false },
                pass.getExecutor(pass.begin(), refraction));

        // generate the mipmap chain
        PostProcessManager::generateMipmapSSR(ppm, fg,
                input, ssrConfig.refraction, true, ssrConfig);

        // Now we're doing the refraction pass proper.
        // This uses the same framebuffer (color and depth) used by the opaque pass. This happens
        // automatically because these are set in the Blackboard (they were set by the opaque
        // pass). For this reason, `desc` below is only used in colorPass() for the width and
        // height.
        config.clearFlags = TargetBufferFlags::NONE;
        output = RendererUtils::colorPass(fg, "Color Pass (transparent)", engine, view,
                { .width = config.width, .height = config.height },
                config, colorGradingConfig, pass.getExecutor(refraction, pass.end()));

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

UTILS_NOINLINE
void RendererUtils::readPixels(backend::DriverApi& driver, Handle<HwRenderTarget> renderTargetHandle,
        uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
        backend::PixelBufferDescriptor&& buffer) {
    ASSERT_PRECONDITION(
            buffer.type != PixelDataType::COMPRESSED,
            "buffer.format cannot be COMPRESSED");

    ASSERT_PRECONDITION(
            buffer.alignment > 0 && buffer.alignment <= 8 &&
            !(buffer.alignment & (buffer.alignment - 1u)),
            "buffer.alignment must be 1, 2, 4 or 8");

    // It's not really possible to know here which formats will be supported because
    // it can vary depending on the RenderTarget, in GL the following are ALWAYS supported though:
    // format: RGBA, RGBA_INTEGER
    // type: UBYTE, UINT, INT, FLOAT

    const size_t sizeNeeded = PixelBufferDescriptor::computeDataSize(
            buffer.format, buffer.type,
            buffer.stride ? buffer.stride : width,
            buffer.top + height,
            buffer.alignment);

    ASSERT_PRECONDITION(buffer.size >= sizeNeeded,
            "Pixel buffer too small: has %u bytes, needs %u bytes", buffer.size, sizeNeeded);

    driver.readPixels(renderTargetHandle, xoffset, yoffset, width, height, std::move(buffer));
}

} // namespace filament
