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
using namespace filament::math;

#ifndef FILAMENT_IOS
#include <imageio/ImageEncoder.h>
#include <image/ColorTransform.h>

using namespace image;
#endif
#include <iostream>

namespace test {

Backend BackendTest::sBackend = Backend::NOOP;
OperatingSystem BackendTest::sOperatingSystem = OperatingSystem::OTHER;
bool BackendTest::sIsMobilePlatform = false;
int BackendTest::sArgc = 0;
char** BackendTest::sArgv = nullptr;
std::vector<std::string> BackendTest::sFailedImages;

void BackendTest::init(Backend backend, OperatingSystem operatingSystem, bool isMobilePlatform,
        int argc, char** argv) {
    sBackend = backend;
    sOperatingSystem = operatingSystem;
    sIsMobilePlatform = isMobilePlatform;
    sArgc = argc;
    sArgv = argv;
}

BackendTest::BackendTest() : commandBufferQueue(CONFIG_MIN_COMMAND_BUFFERS_SIZE,
        CONFIG_COMMAND_BUFFERS_SIZE, /*mPaused=*/false) {
    initializeDriver();
    mImageExpectations.emplace(getDriverApi());
    NativeView nativeView = getNativeView();
    mScreenSize = {nativeView.width, nativeView.height};
}

BackendTest::~BackendTest() {
    // Ensure all graphics commands and callbacks are finished.
    flushAndWait();
    mImageExpectations->evaluate();
    // Note: Don't terminate the driver for OpenGL, as it wipes away the context and removes the buffer from the screen.
    if (sBackend != Backend::OPENGL) {
        driver->terminate();
        delete driver;
    }

    recordFailedImages();
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

PipelineState BackendTest::getColorWritePipelineState() {
    PipelineState result;
    result.rasterState.colorWrite = true;
    result.rasterState.depthWrite = false;
    result.rasterState.depthFunc = RasterState::DepthFunc::A;
    return result;
}

filament::backend::Viewport BackendTest::getFullViewport() const {
   const NativeView& view = getNativeView();
   return Viewport {
       .left = 0,
       .bottom = 0,
       .width = static_cast<uint32_t>(view.width),
       .height = static_cast<uint32_t>(view.height)
   };
}

filament::backend::RenderPassParams BackendTest::getClearColorRenderPass(float4 color) {
    RenderPassParams params = {};
    params.flags.clear = TargetBufferFlags::COLOR;
    params.flags.discardStart = TargetBufferFlags::ALL;
    params.flags.discardEnd = TargetBufferFlags::NONE;
    params.clearColor = color;
    return params;
}

filament::backend::RenderPassParams BackendTest::getNoClearRenderPass() {
    return RenderPassParams{};
}

std::size_t BackendTest::screenWidth() const {
    return mScreenSize[0];
}

std::size_t BackendTest::screenHeight() const {
    return mScreenSize[1];
}

bool BackendTest::matchesEnvironment(Backend backend) {
    return sBackend == backend;
}

bool BackendTest::matchesEnvironment(OperatingSystem operatingSystem) {
    return sOperatingSystem == operatingSystem;
}

void BackendTest::markImageAsFailure(std::string failedImageName) {
    sFailedImages.emplace_back(std::move(failedImageName));
}

std::filesystem::path BackendTest::binaryDirectory() {
    assert(sArgc >= 1);
    return std::filesystem::path(sArgv[0]).remove_filename().string();
}

void BackendTest::recordFailedImages() {
    if (!sFailedImages.empty()) {
        std::string failedImages;
        for (auto& failedTestImageName: sFailedImages) {
            if (failedImages.empty()) {
                failedImages = failedTestImageName;
            } else {
                failedImages.append(",");
                failedImages.append(failedTestImageName);
            }
        }
        RecordProperty("FailedImages", failedImages);
    }
    sFailedImages.clear();
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

void initTests(Backend backend, OperatingSystem operatingSystem, bool isMobile, int& argc,
        char* argv[]) {
    BackendTest::init(backend, operatingSystem, isMobile, argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new Environment);
}

int runTests() {
    return RUN_ALL_TESTS();
}

} // namespace test
