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
#include "fg/ResourceAllocator.h"

#include <backend/Platform.h>

#include "private/backend/CommandStream.h"

using namespace filament;
using namespace backend;

static CircularBuffer buffer(8192);
static Backend gBackend = Backend::NOOP;
static DefaultPlatform* platform = DefaultPlatform::create(&gBackend);
static CommandStream driverApi(*platform->createDriver(nullptr), buffer);

class MockResourceAllocator : public fg::ResourceAllocatorInterface {
    uint32_t handle = 0;
public:
    backend::RenderTargetHandle createRenderTarget(const char* name,
            backend::TargetBufferFlags targetBufferFlags,
            uint32_t width,
            uint32_t height,
            uint8_t samples,
            backend::MRT color,
            backend::TargetBufferInfo depth,
            backend::TargetBufferInfo stencil) noexcept override {
        return backend::RenderTargetHandle(++handle);
    }

    void destroyRenderTarget(backend::RenderTargetHandle h) noexcept override {
    }

    backend::TextureHandle createTexture(const char* name, backend::SamplerType target,
            uint8_t levels,
            backend::TextureFormat format, uint8_t samples, uint32_t width, uint32_t height,
            uint32_t depth, backend::TextureUsage usage) noexcept override {
        return backend::TextureHandle(++handle);
    }

    void destroyTexture(backend::TextureHandle h) noexcept override {
    }
};

struct GenericResource {
    struct Descriptor {};
    void create(FrameGraph& fg, const char* name, Descriptor const& desc) noexcept {
        id = ++state;
    }
    void destroy(FrameGraph& fg) noexcept {}
    int id = 0;
private:
    static int state;
};

int GenericResource::state = 0;


TEST(FrameGraphTest, SimpleRenderPass) {

    fg::ResourceAllocator resourceAllocator(driverApi);
    FrameGraph fg(resourceAllocator);

    bool renderPassExecuted = false;

    struct RenderPassData {
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphRenderTargetHandle rt;
    };

    auto& renderPass = fg.addPass<RenderPassData>("Render",
            [&](FrameGraph::Builder& builder, auto& data) {
                FrameGraphTexture::Descriptor desc{ .format = TextureFormat::RGBA16F };
                data.output = builder.createTexture("color buffer", desc);
                data.output = builder.write(data.output);

                data.rt = builder.createRenderTarget("color buffer", {
                    .attachments = { data.output } });

                // rendertargets are weird...
                // A) reading from an attachment is equivalent to reading from the RT
                //    so any reads on any attachment should trigger a read on the RT,
                //    meaning the RT can't be culled.
                //    Or maybe, there should be a .use(RT) to declare it's going to be used
                //    in that pass.
                // B) writing to a RT, should propagate to its attachments, but then, how
                //    is the new attachment value returned??
                //    maybe it's a special type of write that doesn't change the version??
                //    or maybe we require writes to be set on the resource.


                EXPECT_TRUE(fg.isValid(data.output));
                EXPECT_TRUE(fg.isValid(data.rt));
            },
            [=, &renderPassExecuted](FrameGraphPassResources const& resources,
                    auto const& data, DriverApi& driver) {
                renderPassExecuted = true;
                auto const& rt = resources.get(data.rt);
                EXPECT_TRUE(rt.target);
                EXPECT_EQ(TargetBufferFlags::COLOR, rt.params.flags.discardStart);
                EXPECT_EQ(TargetBufferFlags::NONE, rt.params.flags.discardEnd);
            });

    fg.present(renderPass.getData().output);    // TODO: this should trigger a read() on the RT
    fg.compile();
    fg.execute(driverApi);

    EXPECT_TRUE(renderPassExecuted);

    resourceAllocator.terminate();
}

TEST(FrameGraphTest, SimpleRenderPass2) {

    fg::ResourceAllocator resourceAllocator(driverApi);
    FrameGraph fg(resourceAllocator);

    bool renderPassExecuted = false;

    struct RenderPassData {
        FrameGraphId<FrameGraphTexture> outColor;
        FrameGraphId<FrameGraphTexture> outDepth;
        FrameGraphRenderTargetHandle rt;
    };

    auto& renderPass = fg.addPass<RenderPassData>("Render",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.outColor = builder.createTexture("color buffer",
                        { .format = TextureFormat::RGBA16F });
                data.outDepth = builder.createTexture("depth buffer",
                        { .format = TextureFormat::DEPTH24 });
                data.outColor = builder.write(builder.read(data.outColor));
                data.outDepth = builder.write(builder.read(data.outDepth));
                data.rt = builder.createRenderTarget("rt", {
                        .attachments = { data.outColor, data.outDepth }
                });

                EXPECT_TRUE(fg.isValid(data.outColor));
                EXPECT_TRUE(fg.isValid(data.outDepth));
                EXPECT_TRUE(fg.isValid(data.rt));
            },
            [=, &renderPassExecuted](FrameGraphPassResources const& resources,
                    auto const& data, DriverApi& driver) {
                renderPassExecuted = true;
                auto const& rt = resources.get(data.rt);
                EXPECT_TRUE(rt.target);
                EXPECT_EQ(TargetBufferFlags::COLOR0 | TargetBufferFlags::DEPTH, rt.params.flags.discardStart);
                EXPECT_EQ(TargetBufferFlags::NONE, rt.params.flags.discardEnd);
            });

    fg.present(renderPass.getData().outColor);
    fg.present(renderPass.getData().outDepth);
    fg.compile();
    fg.execute(driverApi);

    EXPECT_TRUE(renderPassExecuted);

    resourceAllocator.terminate();
}

TEST(FrameGraphTest, ScenarioDepthPrePass) {

    fg::ResourceAllocator resourceAllocator(driverApi);
    FrameGraph fg(resourceAllocator);

    bool depthPrepassExecuted = false;
    bool colorPassExecuted = false;

    struct DepthPrepassData {
        FrameGraphId<FrameGraphTexture> outDepth;
        FrameGraphRenderTargetHandle rt;
    };

    auto& depthPrepass = fg.addPass<DepthPrepassData>("depth prepass",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.outDepth = builder.createTexture("depth buffer",
                        { .format = TextureFormat::DEPTH24 });
                data.outDepth = builder.write(builder.read(data.outDepth));
                data.rt = builder.createRenderTarget("rt depth", {
                        .attachments = {{}, data.outDepth }
                });
                EXPECT_TRUE(fg.isValid(data.outDepth));
                EXPECT_TRUE(fg.isValid(data.rt));
            },
            [=, &depthPrepassExecuted](FrameGraphPassResources const& resources,
                    auto const& data, DriverApi& driver) {
                depthPrepassExecuted = true;
                auto const& rt = resources.get(data.rt);
                EXPECT_TRUE(rt.target);
                EXPECT_EQ(TargetBufferFlags::DEPTH, rt.params.flags.discardStart);
                EXPECT_EQ(TargetBufferFlags::NONE, rt.params.flags.discardEnd);
            });

    struct ColorPassData {
        FrameGraphId<FrameGraphTexture> outColor;
        FrameGraphId<FrameGraphTexture> outDepth;
        FrameGraphRenderTargetHandle rt;
    };

    auto& colorPass = fg.addPass<ColorPassData>("color pass",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.outColor = builder.createTexture("color buffer",
                        { .format = TextureFormat::RGBA16F });

                // declare a read here, so a reference is added to the previous pass
                data.outDepth = depthPrepass.getData().outDepth;
                data.outColor = builder.write(builder.read(data.outColor));
                data.outDepth = builder.write(builder.read(data.outDepth));
                data.rt = builder.createRenderTarget("rt color+depth", {
                        .attachments = { data.outColor, data.outDepth }
                });

                EXPECT_FALSE(fg.isValid(depthPrepass.getData().outDepth));
                EXPECT_TRUE(fg.isValid(data.outColor));
                EXPECT_TRUE(fg.isValid(data.outDepth));
                EXPECT_TRUE(fg.isValid(data.rt));
            },
            [=, &colorPassExecuted](FrameGraphPassResources const& resources,
                    auto const& data, DriverApi& driver) {
                colorPassExecuted = true;
                auto const& rt = resources.get(data.rt);
                EXPECT_TRUE(rt.target);
                EXPECT_EQ(TargetBufferFlags::COLOR, rt.params.flags.discardStart);
                EXPECT_EQ(TargetBufferFlags::DEPTH, rt.params.flags.discardEnd);
            });

    fg.present(colorPass.getData().outColor);
    fg.compile();
    fg.execute(driverApi);

    EXPECT_TRUE(depthPrepassExecuted);
    EXPECT_TRUE(colorPassExecuted);

    resourceAllocator.terminate();
}

TEST(FrameGraphTest, SimplePassCulling) {

    fg::ResourceAllocator resourceAllocator(driverApi);
    FrameGraph fg(resourceAllocator);

    bool renderPassExecuted = false;
    bool postProcessPassExecuted = false;
    bool culledPassExecuted = false;

    struct RenderPassData {
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphRenderTargetHandle rt;
    };

    auto& renderPass = fg.addPass<RenderPassData>("Render",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.output = builder.createTexture("renderTarget");
                data.output = builder.write(data.output);
                data.rt = builder.createRenderTarget("renderTarget", {
                        .attachments = { data.output }
                });
                EXPECT_TRUE(fg.isValid(data.output));
                EXPECT_TRUE(fg.isValid(data.rt));
            },
            [=, &renderPassExecuted](FrameGraphPassResources const& resources,
                    auto const& data, backend::DriverApi& driver) {
                renderPassExecuted = true;
                auto const& rt = resources.get(data.rt);
                EXPECT_TRUE(rt.target);
                EXPECT_EQ(TargetBufferFlags::COLOR, rt.params.flags.discardStart);
                EXPECT_EQ(TargetBufferFlags::NONE, rt.params.flags.discardEnd);
            });


    struct PostProcessPassData {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphRenderTargetHandle rt;
    };

    auto& postProcessPass = fg.addPass<PostProcessPassData>("PostProcess",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.input = builder.sample(renderPass.getData().output);
                data.output = builder.createTexture("postprocess-renderTarget");
                data.output = builder.write(data.output);
                data.rt = builder.createRenderTarget("postprocess-renderTarget",{
                    .attachments = { data.output }
                });
                EXPECT_TRUE(fg.isValid(data.input));
                EXPECT_TRUE(fg.isValid(data.output));
                EXPECT_TRUE(fg.isValid(data.rt));
            },
            [=, &postProcessPassExecuted](FrameGraphPassResources const& resources,
                    auto const& data, backend::DriverApi& driver) {
                postProcessPassExecuted = true;
                auto const& rt = resources.get(data.rt);
                EXPECT_TRUE(rt.target);
                EXPECT_EQ(TargetBufferFlags::COLOR, rt.params.flags.discardStart);
                EXPECT_EQ(TargetBufferFlags::NONE, rt.params.flags.discardEnd);
            });


    struct CulledPassData {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphRenderTargetHandle rt;
    };

    auto& culledPass = fg.addPass<CulledPassData>("CulledPass",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.input = builder.sample(renderPass.getData().output);
                data.output = builder.createTexture("unused-rendertarget");
                data.output = builder.write(data.output);
                data.rt = builder.createRenderTarget("unused-rendertarget", {
                        .attachments = { data.output }
                });
                EXPECT_TRUE(fg.isValid(data.input));
                EXPECT_TRUE(fg.isValid(data.output));
                EXPECT_TRUE(fg.isValid(data.rt));
            },
            [=, &culledPassExecuted](FrameGraphPassResources const& resources,
                    auto const& data, backend::DriverApi& driver) {
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

    resourceAllocator.terminate();
}

TEST(FrameGraphTest, MoveGenericResource) {
    // This checks that:
    // - two passes writing in the same resource, that is replaced (moved) by
    //   another resource, end-up both using the 'replacing' resource.
    // - these passes are not culled, even though they're not consumed by
    //   anyone initially -- but the 'replacing' resource was.
    // - passes writing to the moved resource no longer do (and are culled)

    MockResourceAllocator resourceAllocator;
    FrameGraph fg(resourceAllocator);

    struct RenderPassData {
        FrameGraphId<GenericResource> output;
    };

    bool p0Executed = false;
    bool p1Executed = false;
    bool p2Executed = false;
    bool p3Executed = false;
    int h[4];

    auto& p0 = fg.addPass<RenderPassData>("P0",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.output = builder.create<GenericResource>("r0");
                data.output = builder.write(data.output);
                EXPECT_TRUE(fg.isValid(data.output));
            },
            [&](FrameGraphPassResources const& resources,
                    auto const& data, backend::DriverApi& driver) {
                p0Executed = true;
                h[0] = resources.get(data.output).id;
                EXPECT_TRUE(h[0]);
            });


    auto& p1 = fg.addPass<RenderPassData>("P1",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.output = p0.getData().output;
                data.output = builder.read(data.output);
                data.output = builder.write(data.output);
                EXPECT_TRUE(fg.isValid(data.output));
            },
            [&](FrameGraphPassResources const& resources,
                    auto const& data, backend::DriverApi& driver) {
                p1Executed = true;
                h[1] = resources.get(data.output).id;
                EXPECT_TRUE(h[1]);
            });

    auto& p2 = fg.addPass<RenderPassData>("P2",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.output = builder.create<GenericResource>("r2");
                data.output = builder.write(data.output);
                EXPECT_TRUE(fg.isValid(data.output));
            },
            [&](FrameGraphPassResources const& resources,
                    auto const& data, backend::DriverApi& driver) {
                p2Executed = true;
                h[2] = resources.get(data.output).id;
                EXPECT_TRUE(h[2]);
            });

    auto& p3 = fg.addPass<RenderPassData>("P3",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.output = builder.read(p2.getData().output);
                builder.sideEffect();
                EXPECT_TRUE(fg.isValid(data.output));
            },
            [&](FrameGraphPassResources const& resources,
                    auto const& data, backend::DriverApi& driver) {
                p3Executed = true;
                h[3] = resources.get(data.output).id;
                EXPECT_TRUE(h[3]);
            });

    fg.moveResource(p3.getData().output, p1.getData().output);

    fg.compile();
    fg.execute(driverApi);

    EXPECT_TRUE(p0Executed);
    EXPECT_TRUE(p1Executed);
    EXPECT_FALSE(p2Executed);
    EXPECT_TRUE(p3Executed);
    EXPECT_EQ(h[0], h[1]);
    EXPECT_EQ(h[1], h[3]);
    EXPECT_EQ(h[3], h[0]);
}
