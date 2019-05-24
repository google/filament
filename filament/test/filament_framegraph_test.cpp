/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <gtest/gtest.h>

#include "fg/FrameGraph.h"
#include "fg/FrameGraphPassResources.h"

#include <backend/Platform.h>

#include "private/backend/CommandStream.h"

using namespace filament;
using namespace backend;

static CircularBuffer buffer(8192);
static Backend gBackend = Backend::NOOP;
static DefaultPlatform* platform = DefaultPlatform::create(&gBackend);
static CommandStream driverApi(*platform->createDriver(nullptr), buffer);

TEST(FrameGraphTest, SimpleRenderPass) {

    FrameGraph fg;

    bool renderPassExecuted = false;

    struct RenderPassData {
        FrameGraphResource output;
    };

    auto& renderPass = fg.addPass<RenderPassData>("Render",
            [&](FrameGraph::Builder& builder, RenderPassData& data) {
                FrameGraphResource::Descriptor desc{
                        .format = TextureFormat::RGBA16F
                };
                data.output = builder.createTexture("color buffer", desc);
                data.output = builder.useRenderTarget(data.output);
                EXPECT_TRUE(fg.isValid(data.output));
            },
            [=, &renderPassExecuted](
                    FrameGraphPassResources const& resources,
                    RenderPassData const& data,
                    DriverApi& driver) {
                renderPassExecuted = true;
                auto const& rt = resources.getRenderTarget(data.output);
                EXPECT_TRUE(rt.target);
                EXPECT_EQ(TargetBufferFlags::ALL, rt.params.flags.discardStart);
                EXPECT_EQ(TargetBufferFlags::DEPTH_AND_STENCIL, rt.params.flags.discardEnd);

            });

    fg.present(renderPass.getData().output);
    fg.compile();
    fg.execute(driverApi);

    EXPECT_TRUE(renderPassExecuted);
}

TEST(FrameGraphTest, SimpleRenderPass2) {

    FrameGraph fg;

    bool renderPassExecuted = false;

    struct RenderPassData {
        FrameGraphResource outColor;
        FrameGraphResource outDepth;
    };

    auto& renderPass = fg.addPass<RenderPassData>("Render",
            [&](FrameGraph::Builder& builder, RenderPassData& data) {
                FrameGraphResource::Descriptor inputDesc{};
                inputDesc.format = TextureFormat::RGBA16F;
                data.outColor = builder.createTexture("color buffer", inputDesc);
                inputDesc.format = TextureFormat::DEPTH24;
                data.outDepth = builder.createTexture("depth buffer", inputDesc);
                FrameGraphRenderTarget::Descriptor outputDesc{
                        .attachments.color = data.outColor,
                        .attachments.depth = data.outDepth
                };
                auto rt = builder.useRenderTarget("rt", outputDesc);
                data.outColor = rt.textures[0];
                data.outDepth = rt.textures[1];

                EXPECT_TRUE(fg.isValid(data.outColor));
                EXPECT_TRUE(fg.isValid(data.outDepth));
            },
            [=, &renderPassExecuted](
                    FrameGraphPassResources const& resources,
                    RenderPassData const& data,
                    DriverApi& driver) {
                renderPassExecuted = true;
                auto const& rt = resources.getRenderTarget(data.outColor);
                EXPECT_TRUE(rt.target);
                EXPECT_EQ(TargetBufferFlags::ALL, rt.params.flags.discardStart);
                EXPECT_EQ(TargetBufferFlags::STENCIL, rt.params.flags.discardEnd);
            });

    fg.present(renderPass.getData().outColor);
    fg.present(renderPass.getData().outDepth);
    fg.compile();
    fg.execute(driverApi);

    EXPECT_TRUE(renderPassExecuted);
}

TEST(FrameGraphTest, ScenarioDepthPrePass) {

    FrameGraph fg;

    bool depthPrepassExecuted = false;
    bool colorPassExecuted = false;

    struct DepthPrepassData {
        FrameGraphResource outDepth;
    };

    auto& depthPrepass = fg.addPass<DepthPrepassData>("depth prepass",
            [&](FrameGraph::Builder& builder, DepthPrepassData& data) {
                FrameGraphResource::Descriptor inputDesc{};
                inputDesc.format = TextureFormat::DEPTH24;
                data.outDepth = builder.createTexture("depth buffer", inputDesc);
                FrameGraphRenderTarget::Descriptor outputDesc{
                        .attachments.depth = data.outDepth
                };
                data.outDepth = builder.useRenderTarget("rt depth", outputDesc).textures[1];
                EXPECT_TRUE(fg.isValid(data.outDepth));
            },
            [=, &depthPrepassExecuted](
                    FrameGraphPassResources const& resources,
                    DepthPrepassData const& data,
                    DriverApi& driver) {
                depthPrepassExecuted = true;
                auto const& rt = resources.getRenderTarget(data.outDepth);
                EXPECT_TRUE(rt.target);
                EXPECT_EQ(TargetBufferFlags::ALL, rt.params.flags.discardStart);
                EXPECT_EQ(TargetBufferFlags::COLOR_AND_STENCIL, rt.params.flags.discardEnd);
            });

    struct ColorPassData {
        FrameGraphResource outColor;
        FrameGraphResource outDepth;
    };

    auto& colorPass = fg.addPass<ColorPassData>("color pass",
            [&](FrameGraph::Builder& builder, ColorPassData& data) {
                FrameGraphResource::Descriptor inputDesc{};
                inputDesc.format = TextureFormat::RGBA16F;
                data.outColor = builder.createTexture("color buffer", inputDesc);

                // declare a read here, so a reference is added to the previous pass
                data.outDepth = depthPrepass.getData().outDepth;

                FrameGraphRenderTarget::Descriptor outputDesc{
                        .attachments.color = data.outColor,
                        .attachments.depth = data.outDepth
                };
                auto rt = builder.useRenderTarget("rt color+depth", outputDesc);
                data.outColor = rt.textures[0];
                data.outDepth = rt.textures[1];

                EXPECT_FALSE(fg.isValid(depthPrepass.getData().outDepth));
                EXPECT_TRUE(fg.isValid(data.outColor));
                EXPECT_TRUE(fg.isValid(data.outDepth));
            },
            [=, &colorPassExecuted](
                    FrameGraphPassResources const& resources,
                    ColorPassData const& data,
                    DriverApi& driver) {
                colorPassExecuted = true;
                auto const& rt = resources.getRenderTarget(data.outColor);
                EXPECT_TRUE(rt.target);
                EXPECT_EQ(TargetBufferFlags::COLOR_AND_STENCIL, rt.params.flags.discardStart);
                EXPECT_EQ(TargetBufferFlags::DEPTH_AND_STENCIL, rt.params.flags.discardEnd);
            });

    fg.present(colorPass.getData().outColor);
    fg.compile();
    fg.execute(driverApi);

    EXPECT_TRUE(depthPrepassExecuted);
    EXPECT_TRUE(colorPassExecuted);
}

TEST(FrameGraphTest, SimplePassCulling) {

    FrameGraph fg;

    bool renderPassExecuted = false;
    bool postProcessPassExecuted = false;
    bool culledPassExecuted = false;

    struct RenderPassData {
        FrameGraphResource output;
    };

    auto& renderPass = fg.addPass<RenderPassData>("Render",
            [&](FrameGraph::Builder& builder, RenderPassData& data) {
                data.output = builder.createTexture("renderTarget");
                data.output = builder.useRenderTarget(data.output);
            },
            [=, &renderPassExecuted](
                    FrameGraphPassResources const& resources,
                    RenderPassData const& data,
                    backend::DriverApi& driver) {
                renderPassExecuted = true;
                auto const& rt = resources.getRenderTarget(data.output);
                EXPECT_TRUE(rt.target);
                EXPECT_EQ(TargetBufferFlags::ALL, rt.params.flags.discardStart);
                EXPECT_EQ(TargetBufferFlags::DEPTH_AND_STENCIL, rt.params.flags.discardEnd);
            });


    struct PostProcessPassData {
        FrameGraphResource input;
        FrameGraphResource output;
    };

    auto& postProcessPass = fg.addPass<PostProcessPassData>("PostProcess",
            [&](FrameGraph::Builder& builder, PostProcessPassData& data) {
                data.input = builder.read(renderPass.getData().output);
                data.output = builder.createTexture("postprocess-renderTarget");
                data.output = builder.useRenderTarget(data.output);
            },
            [=, &postProcessPassExecuted](
                    FrameGraphPassResources const& resources,
                    PostProcessPassData const& data,
                    backend::DriverApi& driver) {
                postProcessPassExecuted = true;
                auto const& rt = resources.getRenderTarget(data.output);
                EXPECT_TRUE(rt.target);
                EXPECT_EQ(TargetBufferFlags::ALL, rt.params.flags.discardStart);
                EXPECT_EQ(TargetBufferFlags::DEPTH_AND_STENCIL, rt.params.flags.discardEnd);
            });


    struct CulledPassData {
        FrameGraphResource input;
        FrameGraphResource output;
    };

    auto& culledPass = fg.addPass<CulledPassData>("CulledPass",
            [&](FrameGraph::Builder& builder, CulledPassData& data) {
                data.input = builder.read(renderPass.getData().output);
                data.output = builder.createTexture("unused-rendertarget");
                data.output = builder.useRenderTarget(data.output);
            },
            [=, &culledPassExecuted](
                    FrameGraphPassResources const& resources,
                    CulledPassData const& data,
                    backend::DriverApi& driver) {
                culledPassExecuted = true;
            });

    fg.present(postProcessPass.getData().output);

    EXPECT_TRUE(fg.isValid(renderPass.getData().output));
    EXPECT_TRUE(fg.isValid(postProcessPass.getData().input));
    EXPECT_TRUE(fg.isValid(postProcessPass.getData().output));
    EXPECT_TRUE(fg.isValid(culledPass.getData().input));
    EXPECT_TRUE(fg.isValid(culledPass.getData().output));

    fg.compile();
    //fg.export_graphviz(utils::slog.d);
    fg.execute(driverApi);

    EXPECT_TRUE(renderPassExecuted);
    EXPECT_TRUE(postProcessPassExecuted);
    EXPECT_FALSE(culledPassExecuted);
}

TEST(FrameGraphTest, RenderTargetLifetime) {

    FrameGraph fg;

    bool renderPassExecuted1 = false;
    bool renderPassExecuted2 = false;
    Handle<HwRenderTarget> rt1;

    struct RenderPassData {
        FrameGraphResource output;
    };

    auto& renderPass1 = fg.addPass<RenderPassData>("Render1",
            [&](FrameGraph::Builder& builder, RenderPassData& data) {
                FrameGraphResource::Descriptor desc{
                        .format = TextureFormat::RGBA16F
                };
                data.output = builder.createTexture("color buffer", desc);
                data.output = builder.useRenderTarget(data.output, (TargetBufferFlags)0x80);
                EXPECT_TRUE(fg.isValid(data.output));
            },
            [=, &rt1, &renderPassExecuted1](
                    FrameGraphPassResources const& resources,
                    RenderPassData const& data,
                    DriverApi& driver) {
                renderPassExecuted1 = true;
                auto const& rt = resources.getRenderTarget(data.output);
                rt1 = rt.target;
                EXPECT_TRUE(rt.target);
                EXPECT_EQ(TargetBufferFlags::ALL, rt.params.flags.discardStart);
                EXPECT_EQ(TargetBufferFlags::DEPTH_AND_STENCIL, rt.params.flags.discardEnd);
            });

    auto& renderPass2 = fg.addPass<RenderPassData>("Render2",
            [&](FrameGraph::Builder& builder, RenderPassData& data) {
                data.output = builder.useRenderTarget("color", {
                                .attachments.color = {
                                        renderPass1.getData().output, FrameGraphRenderTarget::Attachments::READ_WRITE }},
                        (TargetBufferFlags)0x40).color;
                EXPECT_TRUE(fg.isValid(data.output));
            },
            [=, &rt1, &renderPassExecuted2](
                    FrameGraphPassResources const& resources,
                    RenderPassData const& data,
                    DriverApi& driver) {
                renderPassExecuted2 = true;
                auto const& rt = resources.getRenderTarget(data.output);
                EXPECT_TRUE(rt.target);
                EXPECT_EQ(0x40|0x80, rt.params.flags.clear);
                EXPECT_EQ(rt1.getId(), rt.target.getId()); // FIXME: this test is always true the NoopDriver
                EXPECT_EQ(TargetBufferFlags::DEPTH_AND_STENCIL, rt.params.flags.discardStart);
                EXPECT_EQ(TargetBufferFlags::DEPTH_AND_STENCIL, rt.params.flags.discardEnd);

            });

    fg.present(renderPass2.getData().output);
    fg.compile();
    fg.execute(driverApi);

    EXPECT_TRUE(renderPassExecuted1);
    EXPECT_TRUE(renderPassExecuted2);
}
