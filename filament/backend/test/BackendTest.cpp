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
    auto& api = getDriverApi();
    api.finish();
    executeCommands();
}

Handle<HwSwapChain> BackendTest::createSwapChain() {
    const NativeView& view = getNativeView();
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

    api.draw(state, triangle.getRenderPrimitive(), 1);

    api.endRenderPass();
}

void BackendTest::readPixelsAndAssertHash(const char* testName, size_t width, size_t height,
        Handle<HwRenderTarget> rt, uint32_t expectedHash, bool exportScreenshot) {
    void* buffer = calloc(1, width * height * 4);

    struct Capture {
        uint32_t expectedHash;
        char* name;
        bool exportScreenshot;
        size_t width, height;
    };
    auto* c = new Capture();
    c->expectedHash = expectedHash;
    c->name = strdup(testName);
    c->exportScreenshot = exportScreenshot;
    c->width = width;
    c->height = height;

    PixelBufferDescriptor pbd(buffer, width * height * 4, PixelDataFormat::RGBA, PixelDataType::UBYTE,
            1, 0, 0, width, [](void* buffer, size_t size, void* user) {
                auto* c = (Capture*)user;

                // Export a screenshot, if requested.
                if (c->exportScreenshot) {
#ifndef IOS
                    LinearImage image(c->width, c->height, 4);
                    image = toLinearWithAlpha<uint8_t>(c->width, c->height, c->width * 4,
                            (uint8_t*) buffer);
                    const std::string png = std::string(c->name) + ".png";
                    std::ofstream outputStream(png.c_str(), std::ios::binary | std::ios::trunc);
                    ImageEncoder::encode(outputStream, ImageEncoder::Format::PNG, image, "",
                            png);
#endif
                }

                // Hash the contents of the buffer and check that they match.
                uint32_t hash = utils::hash::murmur3((const uint32_t*) buffer, size / 4, 0);
                ASSERT_EQ(hash, c->expectedHash) << c->name << " failed: hashes do not match." << std::endl;

                free(buffer);
                free(c->name);
                free(c);
            }, (void*)c);
    getDriverApi().readPixels(rt, 0, 0, width, height, std::move(pbd));
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

void getPixelInfo(PixelDataFormat format, PixelDataType type, size_t& outComponents, int& outBpp) {
    assert_invariant(type != PixelDataType::COMPRESSED);
    switch (format) {
        case PixelDataFormat::UNUSED:
        case PixelDataFormat::R:
        case PixelDataFormat::R_INTEGER:
        case PixelDataFormat::DEPTH_COMPONENT:
        case PixelDataFormat::ALPHA:
            outComponents = 1;
            break;
        case PixelDataFormat::RG:
        case PixelDataFormat::RG_INTEGER:
        case PixelDataFormat::DEPTH_STENCIL:
            outComponents = 2;
            break;
        case PixelDataFormat::RGB:
        case PixelDataFormat::RGB_INTEGER:
            outComponents = 3;
            break;
        case PixelDataFormat::RGBA:
        case PixelDataFormat::RGBA_INTEGER:
            outComponents = 4;
            break;
    }

    outBpp = outComponents;
    switch (type) {
        case PixelDataType::COMPRESSED: // Impossible -- to squash the IDE warnings
        case PixelDataType::UBYTE:
        case PixelDataType::BYTE:
            // nothing to do
            break;
        case PixelDataType::USHORT:
        case PixelDataType::SHORT:
        case PixelDataType::HALF:
            outBpp *= 2;
            break;
        case PixelDataType::UINT:
        case PixelDataType::INT:
        case PixelDataType::FLOAT:
            outBpp *= 4;
            break;
        case PixelDataType::UINT_10F_11F_11F_REV:
            // Special case, format must be RGB and uses 4 bytes
            assert_invariant(format == PixelDataFormat::RGB);
            outBpp = 4;
            break;
        case PixelDataType::UINT_2_10_10_10_REV:
            // Special case, format must be RGBA and uses 4 bytes
            assert_invariant(format == PixelDataFormat::RGBA);
            outBpp = 4;
            break;
        case PixelDataType::USHORT_565:
            // Special case, format must be RGB and uses 2 bytes
            assert_invariant(format == PixelDataFormat::RGB);
            outBpp = 2;
            break;
    }
}


} // namespace test

