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

#include <private/backend/PlatformFactory.h>

#include "BackendTest.h"

#include <utils/Hash.h>

#include <fstream>

#include "ShaderGenerator.h"
#include "TrianglePrimitive.h"

static constexpr size_t CONFIG_MIN_COMMAND_BUFFERS_SIZE = 1 * 1024 * 1024;
static constexpr size_t CONFIG_COMMAND_BUFFERS_SIZE     = 3 * CONFIG_MIN_COMMAND_BUFFERS_SIZE;

using namespace filament;
using namespace filament::backend;

#ifndef FILAMENT_IOS
#include <imageio/ImageEncoder.h>
#include <image/ColorTransform.h>

using namespace image;
#endif

namespace test {

Backend BackendTest::sBackend = Backend::NOOP;
OperatingSystem BackendTest::sOperatingSystem = OperatingSystem::OTHER;
bool BackendTest::sIsMobilePlatform = false;

void BackendTest::init(Backend backend, OperatingSystem operatingSystem, bool isMobilePlatform) {
    sBackend = backend;
    sOperatingSystem = operatingSystem;
    sIsMobilePlatform = isMobilePlatform;
}

BackendTest::BackendTest() : commandBufferQueue(CONFIG_MIN_COMMAND_BUFFERS_SIZE,
        CONFIG_COMMAND_BUFFERS_SIZE, /*mPaused=*/false) {
    initializeDriver();
    mImageExpectations.emplace(getDriverApi());
}

BackendTest::~BackendTest() {
    // Ensure all graphics commands and callbacks are finished.
    flushAndWait();
    mImageExpectations->evaluate();
    // Note: Don't terminate the driver for OpenGL, as it wipes away the context and removes the buffer from the screen.
    if (sBackend == Backend::OPENGL) {
        return;
    }
    driver->terminate();
    delete driver;
}

void BackendTest::initializeDriver() {
    auto backend = static_cast<filament::backend::Backend>(sBackend);
    Platform* platform = PlatformFactory::create(&backend);
    assert_invariant(static_cast<uint8_t>(backend) == static_cast<uint8_t>(sBackend));
    Platform::DriverConfig const driverConfig;
    driver = platform->createDriver(nullptr, driverConfig);
    commandStream = std::make_unique<CommandStream>(*driver, commandBufferQueue.getCircularBuffer());
}

void BackendTest::executeCommands() {
    commandBufferQueue.flush();
    auto buffers = commandBufferQueue.waitForCommands();
    for (auto& item : buffers) {
        if (UTILS_LIKELY(item.begin)) {
            getDriverApi().execute(item.begin);
            commandBufferQueue.releaseBuffer(item);
        }
    }
}

void BackendTest::flushAndWait() {
    getDriverApi().finish();
    executeCommands();
    getDriver().purge();
}

Handle<HwSwapChain> BackendTest::createSwapChain() {
    const NativeView& view = getNativeView();
    if (!view.ptr) {
        return getDriverApi().createSwapChainHeadless(view.width, view.height, 0);
    }
    return getDriverApi().createSwapChain(view.ptr, 0);
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

void BackendTest::renderTriangle(
        PipelineLayout const& pipelineLayout,
        Handle<filament::backend::HwRenderTarget> renderTarget,
        Handle<filament::backend::HwSwapChain> swapChain,
        Handle<filament::backend::HwProgram> program) {
    RenderPassParams params = {};
    fullViewport(params);
    params.flags.clear = TargetBufferFlags::COLOR;
    params.clearColor = {0.f, 0.f, 1.f, 1.f};
    params.flags.discardStart = TargetBufferFlags::ALL;
    params.flags.discardEnd = TargetBufferFlags::NONE;
    params.viewport.height = 512;
    params.viewport.width = 512;
    renderTriangle(pipelineLayout, renderTarget, swapChain, program, params);
}

void BackendTest::renderTriangle(
        PipelineLayout const& pipelineLayout,
        Handle<HwRenderTarget> renderTarget,
        Handle<HwSwapChain> swapChain,
        Handle<HwProgram> program,
        const RenderPassParams& params) {
    auto& api = getDriverApi();

    TrianglePrimitive triangle(api);

    api.makeCurrent(swapChain, swapChain);

    api.beginRenderPass(renderTarget, params);

    PipelineState state;
    state.program = program;
    state.pipelineLayout = pipelineLayout;
    state.rasterState.colorWrite = true;
    state.rasterState.depthWrite = false;
    state.rasterState.depthFunc = RasterState::DepthFunc::A;
    state.rasterState.culling = CullingMode::NONE;

    api.draw(state, triangle.getRenderPrimitive(), 0, 3, 1);

    api.endRenderPass();
}

bool BackendTest::matchesEnvironment(Backend backend) {
    return sBackend == backend;
}

bool BackendTest::matchesEnvironment(OperatingSystem operatingSystem) {
    return sOperatingSystem == operatingSystem;
}

bool BackendTest::matchesEnvironment(OperatingSystem operatingSystem, Backend backend) {
    return matchesEnvironment(operatingSystem) && matchesEnvironment(backend);
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

void initTests(Backend backend, OperatingSystem operatingSystem, bool isMobile, int& argc, char* argv[]) {
    BackendTest::init(backend, operatingSystem, isMobile);
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new Environment);
}

int runTests() {
    return RUN_ALL_TESTS();
}

} // namespace test
