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

#ifndef TNT_BACKEND_TEST_H
#define TNT_BACKEND_TEST_H

#include <gtest/gtest.h>

#include <backend/Platform.h>

#include "private/backend/CommandBufferQueue.h"
#include "private/backend/DriverApi.h"

#include "PlatformRunner.h"

namespace test {

class BackendTest : public ::testing::Test {
public:

    static void init(Backend backend, bool isMobilePlatform);

    static Backend sBackend;
    static bool sIsMobilePlatform;

protected:

    BackendTest();
    ~BackendTest() override;

    void initializeDriver();
    void executeCommands();
    void flushAndWait();

    filament::backend::Handle<filament::backend::HwSwapChain> createSwapChain();

    // Helper methods to set the viewport to the full extent of the swap chain.
    static void fullViewport(filament::backend::RenderPassParams& params);
    static void fullViewport(filament::backend::Viewport& viewport);

    void renderTriangle(filament::backend::Handle<filament::backend::HwRenderTarget> renderTarget,
            filament::backend::Handle<filament::backend::HwSwapChain> swapChain,
            filament::backend::Handle<filament::backend::HwProgram> program);
    void renderTriangle(filament::backend::Handle<filament::backend::HwRenderTarget> renderTarget,
            filament::backend::Handle<filament::backend::HwSwapChain> swapChain,
            filament::backend::Handle<filament::backend::HwProgram> program,
            const filament::backend::RenderPassParams& params);

    void readPixelsAndAssertHash(const char* testName, size_t width, size_t height,
            filament::backend::Handle<filament::backend::HwRenderTarget> rt, uint32_t expectedHash,
            bool exportScreenshot = false);

    filament::backend::DriverApi& getDriverApi() { return *commandStream; }
    filament::backend::Driver& getDriver() { return *driver; }

private:

    filament::backend::Driver* driver = nullptr;
    filament::backend::CommandBufferQueue commandBufferQueue;
    std::unique_ptr<filament::backend::DriverApi> commandStream;

    filament::backend::Handle<filament::backend::HwBufferObject> uniform;
};

} // namespace test

#endif
