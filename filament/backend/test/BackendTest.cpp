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

#include "ShaderGenerator.h"

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
    driver->terminate();
    delete driver;
}

void BackendTest::initializeDriver() {
    auto backend = static_cast<filament::backend::Backend>(sBackend);
    DefaultPlatform* platform = DefaultPlatform::create(&backend);
    assert(static_cast<uint8_t>(backend) == static_cast<uint8_t>(sBackend));
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

