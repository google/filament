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

#include <utils/Log.h>
#include <utils/Hash.h>

#include <fstream>

#include "ShaderGenerator.h"
#include "TrianglePrimitive.h"

static constexpr size_t CONFIG_MIN_COMMAND_BUFFERS_SIZE = 1 * 1024 * 1024;
static constexpr size_t CONFIG_COMMAND_BUFFERS_SIZE     = 3 * CONFIG_MIN_COMMAND_BUFFERS_SIZE;

using namespace filament;
using namespace filament::backend;

#ifndef IOS
#include <imageio/ImageEncoder.h>
#include <image/ColorTransform.h>

using namespace image;
#endif

namespace test {

Backend BackendTest::sBackend = Backend::NOOP;
bool BackendTest::sIsMobilePlatform = false;

void BackendTest::init(Backend backend, bool isMobilePlatform) {
    sBackend = backend;
    sIsMobilePlatform = isMobilePlatform;
}

BackendTest::BackendTest() : commandBufferQueue(CONFIG_MIN_COMMAND_BUFFERS_SIZE,
        CONFIG_COMMAND_BUFFERS_SIZE, /*mPaused=*/false) {
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
        filament::backend::Handle<filament::backend::HwRenderTarget> renderTarget,
        filament::backend::Handle<filament::backend::HwSwapChain> swapChain,
        filament::backend::Handle<filament::backend::HwProgram> program) {
    RenderPassParams params = {};
    fullViewport(params);
    params.flags.clear = TargetBufferFlags::COLOR;
    params.clearColor = {0.f, 0.f, 1.f, 1.f};
    params.flags.discardStart = TargetBufferFlags::ALL;
    params.flags.discardEnd = TargetBufferFlags::NONE;
    params.viewport.height = 512;
    params.viewport.width = 512;
    renderTriangle(renderTarget, swapChain, program, params);
}

void BackendTest::renderTriangle(Handle<HwRenderTarget> renderTarget,
        Handle<HwSwapChain> swapChain, Handle<HwProgram> program, const RenderPassParams& params) {
    auto& api = getDriverApi();

    TrianglePrimitive triangle(api);

    api.makeCurrent(swapChain, swapChain);

    api.beginRenderPass(renderTarget, params);

    PipelineState state;
    state.program = program;
    state.rasterState.colorWrite = true;
    state.rasterState.depthWrite = false;
    state.rasterState.depthFunc = RasterState::DepthFunc::A;
    state.rasterState.culling = CullingMode::NONE;

    api.draw(state, triangle.getRenderPrimitive(), 0, 3, 1);

    api.endRenderPass();
}

void BackendTest::readPixelsAndAssertHash(const char* testName, size_t width, size_t height,
        Handle<HwRenderTarget> rt, uint32_t const expectedHash, bool const exportScreenshot) {
    auto img = getRenderTargetRGB(rt, width, height);
    uint32_t hash = computeImageHash(img);

    if (exportScreenshot) {
        writeImage(img, width, height, utils::CString(testName));
    }
    ASSERT_EQ(hash, expectedHash) << testName << " failed: hashes do not match." << std::endl;
}

void BackendTest::writeImage(utils::FixedCapacityVector<uint8_t> img, size_t w, size_t h,
        utils::CString const& fname) {
    constexpr size_t MAX_FNAME_LEN = 50;
    constexpr size_t FNAME_EXT_LEN = 4;
    constexpr size_t TOTAL_LEN = MAX_FNAME_LEN + FNAME_EXT_LEN + 1;
    char buf[TOTAL_LEN];

    ASSERT_PRECONDITION(fname.size() <= MAX_FNAME_LEN, "File name %s is too long", fname.c_str());

    snprintf(buf, TOTAL_LEN, "%s.png", fname.c_str());

#ifndef IOS
    LinearImage image(w, h, 4);
    image = toLinearWithAlpha<uint8_t>(w, h, w * 4, img.data());
    std::ofstream pngstrm(buf, std::ios::binary | std::ios::trunc);
    ImageEncoder::encode(pngstrm, ImageEncoder::Format::PNG, image, "", buf);
#endif
}

utils::FixedCapacityVector<uint8_t> BackendTest::getRenderTarget(
        Handle<HwRenderTarget> rt, size_t width, size_t height, PixelDataFormat format,
        PixelDataType dataType) {
    auto& api = getDriverApi();
    size_t const size = width * height * 4;
    utils::FixedCapacityVector<uint8_t> result(size);
    PixelBufferDescriptor pb(result.data(), size, format, dataType);
    api.readPixels(rt, 0, 0, width, height, std::move(pb));
    flushAndWait();

    return result;
}

uint32_t BackendTest::computeImageHash(utils::FixedCapacityVector<uint8_t> const& img) {
    return utils::hash::murmur3((uint32_t*) img.data(), img.size() / 4, 0);
}

utils::FixedCapacityVector<uint8_t> BackendTest::getRenderTargetRGB(
        Handle<HwRenderTarget> rt, size_t width, size_t height) {
    return getRenderTarget(rt, width, height, PixelDataFormat::RGBA,
            PixelDataType::UBYTE);
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
