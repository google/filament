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

#include "driver/CommandStream.h"
#include "driver/noop/NoopDriver.h"

using namespace filament;

static CircularBuffer buffer(8192);
static CommandStream driverApi(*NoopDriver::create(), buffer);

TEST(FrameGraphTest, SimpleRenderPass) {

    FrameGraph fg;

    bool renderPassExecuted = false;

    struct RenderPassData {
        FrameGraphResource output;
        FrameGraphRenderTarget outRenderTarget;
    };

    auto& renderPass = fg.addPass<RenderPassData>("Render",
            [&](FrameGraph::Builder& builder, RenderPassData& data) {
                FrameGraphResource::Descriptor desc {
                    .format = driver::TextureFormat::RGBA16F
                };
                data.output = builder.write(builder.declareTexture("renderTarget", desc));
                data.outRenderTarget = builder.declareRenderTarget(data.output);
                EXPECT_TRUE(data.output.isValid());
                EXPECT_TRUE(fg.isValid(data.output));
            },
            [=, &renderPassExecuted](
                    FrameGraphPassResources const& resources,
                    RenderPassData const& data,
                    driver::DriverApi& driver) {
                renderPassExecuted = true;
                EXPECT_TRUE(resources.getRenderTarget(data.outRenderTarget).target);

            });

    fg.present(renderPass.getData().output);

    fg.compile();
    fg.execute(driverApi);

    EXPECT_TRUE(renderPassExecuted);
}

TEST(FrameGraphTest, SimpleRenderPassReadWrite) {

    FrameGraph fg;

    struct RenderPassData {
        FrameGraphResource input;
        FrameGraphResource output;
    };

    auto& renderPass = fg.addPass<RenderPassData>("Render",
            [&](FrameGraph::Builder& builder, RenderPassData& data) {
                auto r = builder.declareTexture("renderTarget", {});
                data.output = builder.write(r);
                data.input = builder.read(data.output);
            },
            [=](FrameGraphPassResources const& resources,
                    RenderPassData const& data,
                    driver::DriverApi& driver) {
            });

    fg.present(renderPass.getData().output);
    fg.compile();
    fg.export_graphviz(utils::slog.d);
    fg.execute(driverApi);
}


TEST(FrameGraphTest, SimpleRenderAndPostProcessPasses) {

    FrameGraph fg;

    bool renderPassExecuted = false;
    bool postProcessPassExecuted = false;

    struct RenderPassData {
        FrameGraphResource output;
        FrameGraphRenderTarget outRenderTarget;
    };

    auto& renderPass = fg.addPass<RenderPassData>("Render",
            [&](FrameGraph::Builder& builder, RenderPassData& data) {
                FrameGraphResource::Descriptor desc {
                        .format = driver::TextureFormat::RGBA16F
                };
                data.output = builder.write(builder.declareTexture("renderTarget", desc));
                data.outRenderTarget = builder.declareRenderTarget(data.output);
                EXPECT_TRUE(data.output.isValid());
                EXPECT_TRUE(fg.isValid(data.output));
            },
            [=, &renderPassExecuted](
                    FrameGraphPassResources const& resources,
                    RenderPassData const& data,
                    driver::DriverApi& driver) {
                renderPassExecuted = true;
                EXPECT_TRUE(resources.getRenderTarget(data.outRenderTarget).target);
            });


    struct PostProcessPassData {
        FrameGraphResource input;
        FrameGraphResource output;
    };

    auto& postProcessPass = fg.addPass<PostProcessPassData>("PostProcess",
            [&](FrameGraph::Builder& builder, PostProcessPassData& data) {
                data.input = builder.read(renderPass.getData().output);
                data.output = builder.write(data.input);

                auto* desc = fg.getDescriptor(data.output);

                EXPECT_EQ(desc->format, driver::TextureFormat::RGBA16F);

                EXPECT_TRUE(data.input.isValid());
                EXPECT_TRUE(data.output.isValid());
                EXPECT_FALSE(fg.isValid(data.input));
                EXPECT_TRUE(fg.isValid(data.output));
            },
            [=, &postProcessPassExecuted](
                    FrameGraphPassResources const& resources,
                    PostProcessPassData const& data,
                    driver::DriverApi& driver) {
                postProcessPassExecuted = true;
            });

    fg.present(postProcessPass.getData().output);

    EXPECT_FALSE(fg.isValid(renderPass.getData().output));
    EXPECT_FALSE(fg.isValid(postProcessPass.getData().input));
    EXPECT_TRUE(fg.isValid(postProcessPass.getData().output));

    fg.compile();
    //fg.export_graphviz(utils::slog.d);
    fg.execute(driverApi);


    EXPECT_TRUE(renderPassExecuted);
    EXPECT_TRUE(postProcessPassExecuted);
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
                data.output = builder.write(builder.declareTexture("renderTarget"));
            },
            [=, &renderPassExecuted](
                    FrameGraphPassResources const& resources,
                    RenderPassData const& data,
                    driver::DriverApi& driver) {
                renderPassExecuted = true;
            });


    struct PostProcessPassData {
        FrameGraphResource input;
        FrameGraphResource output;
    };

    auto& postProcessPass = fg.addPass<PostProcessPassData>("PostProcess",
            [&](FrameGraph::Builder& builder, PostProcessPassData& data) {
                data.input = builder.read(renderPass.getData().output);
                data.output = builder.write(builder.declareTexture("postprocess-renderTarget"));
            },
            [=, &postProcessPassExecuted](
                    FrameGraphPassResources const& resources,
                    PostProcessPassData const& data,
                    driver::DriverApi& driver) {
                postProcessPassExecuted = true;
            });


    struct CulledPassData {
        FrameGraphResource input;
        FrameGraphResource output;
    };

    auto& culledPass = fg.addPass<CulledPassData>("CulledPass",
            [&](FrameGraph::Builder& builder, CulledPassData& data) {
                data.input = builder.read(renderPass.getData().output);
                data.output = builder.write(builder.declareTexture("unused-rendertarget"));
            },
            [=, &culledPassExecuted](
                    FrameGraphPassResources const& resources,
                    CulledPassData const& data,
                    driver::DriverApi& driver) {
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


TEST(FrameGraphTest, BadGraph) {

    /*
     * We invalidate and rename handles that are written into, to avoid undefined order
     * access to the resources.
     *
     * e.g. forbidden graphs
     *
     *              +->[R1]-+
     *             /         \
     * [R0]->(A)--+           +-> (A)
     *             \         /
     *              +->[R2]-+        // failure when setting R2 from (A)
     *
     */

    FrameGraph fg;

    bool R0exec = false;
    bool R1exec = false;
    bool R2exec = false;

    struct R0Data {
        FrameGraphResource output;
    };

    auto& R0 = fg.addPass<R0Data>("R1",
            [&](FrameGraph::Builder& builder, R0Data& data) {
                data.output = builder.write(builder.declareTexture("A"));
            },
            [=, &R0exec](FrameGraphPassResources const&, R0Data const&, driver::DriverApi&) {
                R0exec = true;
            });


    struct RData {
        FrameGraphResource input;
        FrameGraphResource output;
    };

    auto& R1 = fg.addPass<RData>("R1",
            [&](FrameGraph::Builder& builder, RData& data) {
                data.input = builder.read(R0.getData().output);
                data.output = builder.write(data.input);
            },
            [=, &R1exec](FrameGraphPassResources const&, RData const&, driver::DriverApi&) {
                R1exec = true;
            });

    fg.present(R1.getData().output);

    auto& R2 = fg.addPass<RData>("R2",
            [&](FrameGraph::Builder& builder, RData& data) {
                EXPECT_FALSE(fg.isValid(R0.getData().output));
            },
            [=, &R2exec](FrameGraphPassResources const&, RData const&, driver::DriverApi&) {
                R2exec = true;
            });

    EXPECT_FALSE(fg.isValid(R2.getData().output));

    fg.compile();

    fg.execute(driverApi);

    EXPECT_TRUE(R0exec);
    EXPECT_TRUE(R1exec);
    EXPECT_FALSE(R2exec);
}

TEST(FrameGraphTest, ComplexGraph) {

    FrameGraph fg;

    struct DepthPassData {
        FrameGraphResource output;
    };
    auto& depthPass = fg.addPass<DepthPassData>("Depth pass",
            [&](FrameGraph::Builder& builder, DepthPassData& data) {
                data.output = builder.write(builder.declareTexture("Depth Buffer"));
            },
            [=](FrameGraphPassResources const&, DepthPassData const&, driver::DriverApi&) {
            });



    // buggy pass
    struct BuffyPassData {
        FrameGraphResource input;
        FrameGraphResource output;
    };
    auto& buggyPass = fg.addPass<BuffyPassData>("Bug",
            [&](FrameGraph::Builder& builder, BuffyPassData& data) {
                data.input = builder.read(depthPass.getData().output);
                data.output = builder.write(builder.declareTexture("Buggy output"));
            },
            [&](FrameGraphPassResources const&, BuffyPassData const&, driver::DriverApi&) {
            });
    fg.present(buggyPass.getData().output);



    struct GBufferPassData {
        FrameGraphResource input;
        FrameGraphResource output;
        FrameGraphResource gbuffers[3];
    };
    auto& gbufferPass = fg.addPass<GBufferPassData>("Gbuffer pass",
            [&](FrameGraph::Builder& builder, GBufferPassData& data) {
                data.input = builder.read(depthPass.getData().output);
                data.output = builder.write(data.input);
                data.gbuffers[0] = builder.write(builder.declareTexture("Gbuffer 1"));
                data.gbuffers[1] = builder.write(builder.declareTexture("Gbuffer 2"));
                data.gbuffers[2] = builder.write(builder.declareTexture("Gbuffer 3"));
            },
            [=](FrameGraphPassResources const&, GBufferPassData const&, driver::DriverApi&) {
            });


    struct LightingPassData {
        FrameGraphResource input[4];
        FrameGraphResource output;
    };
    auto& lightingPass = fg.addPass<LightingPassData>("Lighting pass",
            [&](FrameGraph::Builder& builder, LightingPassData& data) {
                data.input[0] = builder.read(gbufferPass.getData().output);
                data.input[1] = builder.read(gbufferPass.getData().gbuffers[0]);
                data.input[2] = builder.read(gbufferPass.getData().gbuffers[1]);
                data.input[3] = builder.read(gbufferPass.getData().gbuffers[2]);
                data.output = builder.write(builder.declareTexture("Lighting buffer"));
            },
            [=](FrameGraphPassResources const&, LightingPassData const&, driver::DriverApi&) {
            });


    struct ConvolutionPassData {
        FrameGraphResource input;
        FrameGraphResource output;
    };
    auto& convolutionPass = fg.addPass<ConvolutionPassData>("Convolution",
            [&](FrameGraph::Builder& builder, ConvolutionPassData& data) {
                data.input = builder.read(gbufferPass.getData().gbuffers[2]); //builder.createTexture("Cubemap", Builder::READ, {});
                data.output = builder.write(builder.declareTexture("Reflection probe"));
            },
            [=](FrameGraphPassResources const&, ConvolutionPassData const&, driver::DriverApi&) {
            });

    fg.present(lightingPass.getData().output);

    //fg.moveResource(convolutionPass.getData().output, lightingPass.getData().output);

    fg.compile();
    //fg.export_graphviz(utils::slog.d);
    fg.execute(driverApi);
}

TEST(FrameGraphTest, MoveResource) {

    FrameGraph fg;

    struct RenderPassData {
        FrameGraphResource input;
        FrameGraphResource output;
    };

    auto& renderPass = fg.addPass<RenderPassData>("Render",
            [&](FrameGraph::Builder& builder, RenderPassData& data) {
                data.input = builder.read(builder.declareTexture("render-inout"));
                data.output = builder.write(data.input);
            },
            [=](FrameGraphPassResources const& resources, RenderPassData const& data, driver::DriverApi&) {
            });
    fg.present(renderPass.getData().output);

    auto& debugPass = fg.addPass<RenderPassData>("Debug",
            [&](FrameGraph::Builder& builder, RenderPassData& data) {
                data.input = builder.read(builder.declareTexture("debug-inout"));
                data.output = builder.write(data.input);
            },
            [=](FrameGraphPassResources const& resources, RenderPassData const& data, driver::DriverApi&) {
            });
    fg.present(debugPass.getData().output);

    fg.moveResource(renderPass.getData().output, debugPass.getData().input);

    fg.compile();
    //fg.export_graphviz(utils::slog.d);
    fg.execute(driverApi);
}
