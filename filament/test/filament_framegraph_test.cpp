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

using namespace filament;

TEST(FrameGraphTest, SimpleRenderPass) {

    FrameGraph fg;

    bool renderPassExecuted = false;

    struct RenderPassData {
        FrameGraphResource output;
    };

    auto& renderPass = fg.addPass<RenderPassData>("Render",
            [&](FrameGraphBuilder& builder, RenderPassData& data) {
                data.output = builder.createTexture("renderTarget", FrameGraphBuilder::WRITE, {});
                EXPECT_TRUE(data.output.isValid());
                EXPECT_TRUE(fg.isValid(data.output));
            },
            [&renderPassExecuted](FrameGraphPassResources const& resources, RenderPassData const& data) {
                renderPassExecuted = true;
            });

    fg.present(renderPass.getData().output);

    fg.compile().execute();

    EXPECT_TRUE(renderPassExecuted);
}


TEST(FrameGraphTest, SimpleRenderAndPostProcessPasses) {

    FrameGraph fg;

    bool renderPassExecuted = false;
    bool postProcessPassExecuted = false;

    struct RenderPassData {
        FrameGraphResource output;
    };

    auto& renderPass = fg.addPass<RenderPassData>("Render",
            [&](FrameGraphBuilder& builder, RenderPassData& data) {
                data.output = builder.createTexture("renderTarget", FrameGraphBuilder::WRITE, {});
                EXPECT_TRUE(data.output.isValid());
                EXPECT_TRUE(fg.isValid(data.output));
            },
            [&renderPassExecuted](FrameGraphPassResources const& resources, RenderPassData const& data) {
                renderPassExecuted = true;
            });


    struct PostProcessPassData {
        FrameGraphResource input;
        FrameGraphResource output;
    };

    auto& postProcessPass = fg.addPass<PostProcessPassData>("PostProcess",
            [&](FrameGraphBuilder& builder, PostProcessPassData& data) {
                data.input = builder.read(renderPass.getData().output);
                data.output = builder.write(data.input);
                EXPECT_TRUE(data.input.isValid());
                EXPECT_TRUE(data.output.isValid());
                EXPECT_FALSE(fg.isValid(data.input));
                EXPECT_TRUE(fg.isValid(data.output));
            },
            [&postProcessPassExecuted](FrameGraphPassResources const& resources, PostProcessPassData const& data) {
                postProcessPassExecuted = true;
            });

    fg.present(postProcessPass.getData().output);

    EXPECT_FALSE(fg.isValid(renderPass.getData().output));
    EXPECT_FALSE(fg.isValid(postProcessPass.getData().input));
    EXPECT_TRUE(fg.isValid(postProcessPass.getData().output));

    fg.compile();

    fg.export_graphviz(utils::slog.d);

    fg.execute();


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
            [&](FrameGraphBuilder& builder, RenderPassData& data) {
                data.output = builder.createTexture("renderTarget", FrameGraphBuilder::WRITE, {});
            },
            [&renderPassExecuted](FrameGraphPassResources const& resources, RenderPassData const& data) {
                renderPassExecuted = true;
            });


    struct PostProcessPassData {
        FrameGraphResource input;
        FrameGraphResource output;
    };

    auto& postProcessPass = fg.addPass<PostProcessPassData>("PostProcess",
            [&](FrameGraphBuilder& builder, PostProcessPassData& data) {
                data.input = builder.read(renderPass.getData().output);
                data.output = builder.createTexture("postprocess-renderTarget", FrameGraphBuilder::WRITE, {});
            },
            [&postProcessPassExecuted](FrameGraphPassResources const& resources, PostProcessPassData const& data) {
                postProcessPassExecuted = true;
            });


    struct CulledPassData {
        FrameGraphResource input;
        FrameGraphResource output;
    };

    auto& culledPass = fg.addPass<CulledPassData>("CulledPass",
            [&](FrameGraphBuilder& builder, CulledPassData& data) {
                data.input = builder.read(renderPass.getData().output);
                data.output = builder.createTexture("unused-rendertarget", FrameGraphBuilder::WRITE, {});
            },
            [&culledPassExecuted](FrameGraphPassResources const& resources, CulledPassData const& data) {
                culledPassExecuted = true;
            });

    fg.present(postProcessPass.getData().output);

    EXPECT_TRUE(fg.isValid(renderPass.getData().output));
    EXPECT_TRUE(fg.isValid(postProcessPass.getData().input));
    EXPECT_TRUE(fg.isValid(postProcessPass.getData().output));
    EXPECT_TRUE(fg.isValid(culledPass.getData().input));
    EXPECT_TRUE(fg.isValid(culledPass.getData().output));

    //fg.export_graphviz(utils::slog.d);

    fg.compile();

    //fg.export_graphviz(utils::slog.d);

    fg.execute();

    EXPECT_TRUE(renderPassExecuted);
    EXPECT_TRUE(postProcessPassExecuted);
    EXPECT_FALSE(culledPassExecuted);
}


TEST(FrameGraphTest, BadGraph) {

    /*
     * We invalidate and rename handles that are writen into, to avoid undefined order
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
            [&](FrameGraphBuilder& builder, R0Data& data) {
                data.output = builder.createTexture("A", FrameGraphBuilder::WRITE, {});
            },
            [&](FrameGraphPassResources const&, R0Data const&) {
                R0exec = true;
            });


    struct RData {
        FrameGraphResource input;
        FrameGraphResource output;
    };

    auto& R1 = fg.addPass<RData>("R1",
            [&](FrameGraphBuilder& builder, RData& data) {
                data.input = builder.read(R0.getData().output);
                data.output = builder.write(data.input);
            },
            [&](FrameGraphPassResources const&, RData const&) {
                R1exec = true;
            });

    fg.present(R1.getData().output);

    auto& R2 = fg.addPass<RData>("R2",
            [&](FrameGraphBuilder& builder, RData& data) {
                EXPECT_FALSE(fg.isValid(R0.getData().output));
            },
            [&](FrameGraphPassResources const&, RData const&) {
                R2exec = true;
            });

    EXPECT_FALSE(fg.isValid(R2.getData().output));

    fg.compile();

    fg.execute();

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
            [&](FrameGraphBuilder& builder, DepthPassData& data) {
                data.output = builder.createTexture("Depth Buffer", FrameGraphBuilder::WRITE, {});
            },
            [&](FrameGraphPassResources const&, DepthPassData const&) {
            });



    // buggy pass
    struct BuffyPassData {
        FrameGraphResource input;
        FrameGraphResource output;
    };
    auto& buggyPass = fg.addPass<BuffyPassData>("Bug",
            [&](FrameGraphBuilder& builder, BuffyPassData& data) {
                data.input = builder.read(depthPass.getData().output);
                data.output = builder.createTexture("Buggy output", FrameGraphBuilder::WRITE, {});
            },
            [&](FrameGraphPassResources const&, BuffyPassData const&) {
            });
    fg.present(buggyPass.getData().output);



    struct GBufferPassData {
        FrameGraphResource input;
        FrameGraphResource output;
        FrameGraphResource gbuffers[3];
    };
    auto& gbufferPass = fg.addPass<GBufferPassData>("Gbuffer pass",
            [&](FrameGraphBuilder& builder, GBufferPassData& data) {
                data.input = builder.read(depthPass.getData().output);
                data.output = builder.write(data.input);
                data.gbuffers[0] = builder.createTexture("Gbuffer 1", FrameGraphBuilder::WRITE, {});
                data.gbuffers[1] = builder.createTexture("Gbuffer 2", FrameGraphBuilder::WRITE, {});
                data.gbuffers[2] = builder.createTexture("Gbuffer 3", FrameGraphBuilder::WRITE, {});
            },
            [&](FrameGraphPassResources const&, GBufferPassData const&) {
            });


    struct LightingPassData {
        FrameGraphResource input[4];
        FrameGraphResource output;
    };
    auto& lightingPass = fg.addPass<LightingPassData>("Lighting pass",
            [&](FrameGraphBuilder& builder, LightingPassData& data) {
                data.input[0] = builder.read(gbufferPass.getData().output);
                data.input[1] = builder.read(gbufferPass.getData().gbuffers[0]);
                data.input[2] = builder.read(gbufferPass.getData().gbuffers[1]);
                data.input[3] = builder.read(gbufferPass.getData().gbuffers[2]);
                data.output = builder.createTexture("Lighting buffer", FrameGraphBuilder::WRITE, {});
            },
            [&](FrameGraphPassResources const&, LightingPassData const&) {
            });


    struct ConvolutionPassData {
        FrameGraphResource input;
        FrameGraphResource output;
    };
    auto& convolutionPass = fg.addPass<ConvolutionPassData>("Convolution",
            [&](FrameGraphBuilder& builder, ConvolutionPassData& data) {
                data.input = builder.read(gbufferPass.getData().gbuffers[2]); //builder.createTexture("Cubemap", FrameGraphBuilder::READ, {});
                data.output = builder.createTexture("Reflection probe", FrameGraphBuilder::WRITE, {});
            },
            [&](FrameGraphPassResources const&, ConvolutionPassData const&) {
            });

    fg.present(lightingPass.getData().output);

    fg.moveResource(convolutionPass.getData().output, lightingPass.getData().output);

    fg.compile();

    fg.export_graphviz(utils::slog.d);

    fg.execute();
}

TEST(FrameGraphTest, MoveResource) {

    FrameGraph fg;

    struct RenderPassData {
        FrameGraphResource input;
        FrameGraphResource output;
    };

    auto& renderPass = fg.addPass<RenderPassData>("Render",
            [&](FrameGraphBuilder& builder, RenderPassData& data) {
                data.input = builder.createTexture("render-inout", FrameGraphBuilder::READ, {});
                data.output = builder.write(data.input);
            },
            [](FrameGraphPassResources const& resources, RenderPassData const& data) {
            });
    fg.present(renderPass.getData().output);

    auto& debugPass = fg.addPass<RenderPassData>("Debug",
            [&](FrameGraphBuilder& builder, RenderPassData& data) {
                data.input = builder.createTexture("debug-inout", FrameGraphBuilder::READ, {});
                data.output = builder.write(data.input);
            },
            [](FrameGraphPassResources const& resources, RenderPassData const& data) {
            });
    fg.present(debugPass.getData().output);

    fg.moveResource(renderPass.getData().output, debugPass.getData().input);

    fg.compile();
    fg.export_graphviz(utils::slog.d);
    fg.execute();
}
