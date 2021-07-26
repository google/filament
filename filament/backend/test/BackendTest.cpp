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

#include "BackendTest.h"

#include <utils/Hash.h>

#include "ShaderGenerator.h"
#include "TrianglePrimitive.h"

static constexpr size_t CONFIG_MIN_COMMAND_BUFFERS_SIZE = 1 * 1024 * 1024;
static constexpr size_t CONFIG_COMMAND_BUFFERS_SIZE     = 3 * CONFIG_MIN_COMMAND_BUFFERS_SIZE;

namespace test {

using namespace filament;
using namespace filament::backend;

Backend BackendTest::sBackend = Backend::NOOP;
bool BackendTest::sIsMobilePlatform = false;

void BackendTest::init(Backend backend, bool isMobilePlatform) {
    sBackend = backend;
    sIsMobilePlatform = isMobilePlatform;
}

BackendTest::BackendTest() : commandBufferQueue(CONFIG_MIN_COMMAND_BUFFERS_SIZE,
            CONFIG_COMMAND_BUFFERS_SIZE) {
    initializeDriver();
}

BackendTest::~BackendTest() {
    // Note: Don't terminate the driver for OpenGL, as it wipes away the context and removes the buffer from the screen.
    if (sBackend == Backend::OPENGL) {
        return;
    }
    flushAndWait();
    driver->terminate();
    delete driver;
}

void BackendTest::initializeDriver() {
    auto backend = static_cast<filament::backend::Backend>(sBackend);
    DefaultPlatform* platform = DefaultPlatform::create(&backend);
    assert_invariant(static_cast<uint8_t>(backend) == static_cast<uint8_t>(sBackend));
    driver = platform->createDriver(nullptr);
    commandStream = CommandStream(*driver, commandBufferQueue.getCircularBuffer());
}

void BackendTest::executeCommands() {
    commandBufferQueue.flush();
    auto buffers = commandBufferQueue.waitForCommands();
    for (auto& item : buffers) {
        if (UTILS_LIKELY(item.begin)) {
            commandStream.execute(item.begin);
            commandBufferQueue.releaseBuffer(item);
        }
    }
}

void BackendTest::flushAndWait(uint64_t timeout) {
    auto& api = getDriverApi();
    auto fence = api.createFence();
    api.finish();
    executeCommands();
    api.wait(fence, timeout);
    api.destroyFence(fence);
}

Handle<HwSwapChain> BackendTest::createSwapChain() {
    const NativeView& view = getNativeView();
    return commandStream.createSwapChain(view.ptr, 0);
}

void BackendTest::fullViewport(RenderPassParams& params) {
    fullViewport(params.viewport);
}

void BackendTest::fullViewport(Viewport& viewport) {
    const NativeView& view = getNativeView();
    viewport.left = 0;
    viewport.bottom = 0;
    viewport.width = view.width;
    viewport.height = view.height;
}

void BackendTest::renderTriangle(Handle<HwRenderTarget> renderTarget,
        Handle<HwSwapChain> swapChain, Handle<HwProgram> program) {
    auto& api = getDriverApi();

    TrianglePrimitive triangle(api);

    RenderPassParams params = {};
    fullViewport(params);
    params.flags.clear = TargetBufferFlags::COLOR;
    params.clearColor = {0.f, 0.f, 1.f, 1.f};
    params.flags.discardStart = TargetBufferFlags::ALL;
    params.flags.discardEnd = TargetBufferFlags::NONE;
    params.viewport.height = 512;
    params.viewport.width = 512;

    api.makeCurrent(swapChain, swapChain);

    api.beginRenderPass(renderTarget, params);

    PipelineState state;
    state.program = program;
    state.rasterState.colorWrite = true;
    state.rasterState.depthWrite = false;
    state.rasterState.depthFunc = RasterState::DepthFunc::A;
    state.rasterState.culling = CullingMode::NONE;

    api.draw(state, triangle.getRenderPrimitive());

    api.endRenderPass();
}

void BackendTest::readPixelsAndAssertHash(const char* testName, size_t width, size_t height,
        Handle<HwRenderTarget> rt, uint32_t expectedHash) {
    void* buffer = calloc(1, width * height * 4);

    struct Capture {
        uint32_t expectedHash;
        char* name;
    };
    Capture* c = new Capture();
    c->expectedHash = expectedHash;
    c->name = strdup(testName);

    PixelBufferDescriptor pbd(buffer, width * height * 4, PixelDataFormat::RGBA, PixelDataType::UBYTE,
            1, 0, 0, width, [](void* buffer, size_t size, void* user) {
                Capture* c = (Capture*)user;

                // Hash the contents of the buffer and check that they match.
                uint32_t hash = utils::hash::murmur3((const uint32_t*) buffer, size / 4, 0);
                ASSERT_EQ(hash, c->expectedHash) << c->name << " failed: hashes do not match." << std::endl;

                free(buffer);
                free(c->name);
                free(c);
            }, (void*)c);
    getDriverApi().readPixels(rt, 0, 0, 512, 512, std::move(pbd));
}

class Environment : public ::testing::Environment {
public:
    virtual void SetUp() override {
        ShaderGenerator::init();
    }

    virtual void TearDown() override {
        ShaderGenerator::shutdown();
    }
};

void initTests(Backend backend, bool isMobile, int& argc, char* argv[]) {
    BackendTest::init(backend, isMobile);
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new Environment);
}

int runTests() {
    return RUN_ALL_TESTS();
}

} // namespace test

