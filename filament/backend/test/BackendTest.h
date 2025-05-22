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
#include "ImageExpectations.h"

namespace test {

class BackendTest : public ::testing::Test {
public:

    static void init(Backend backend, OperatingSystem operatingSystem, bool isMobilePlatform);

    static Backend sBackend;
    static OperatingSystem sOperatingSystem;
    static bool sIsMobilePlatform;

    // Takes the name of the image that wasn't correct, without the .png suffix
    static void markImageAsFailure(std::string failedImageName);

protected:

    BackendTest();
    ~BackendTest() override;

    void initializeDriver();
    void executeCommands();
    void flushAndWait();

    filament::backend::Handle<filament::backend::HwSwapChain> createSwapChain();

    static filament::backend::PipelineState getColorWritePipelineState();

    // Gets the full back buffer's viewport
    filament::backend::Viewport getFullViewport() const;
    // If color is unset this defaults to using opaque cyan
    static filament::backend::RenderPassParams getClearColorRenderPass(
            filament::math::float4 color = filament::math::float4(0, 1, 1, 1));
    static filament::backend::RenderPassParams getNoClearRenderPass();

    filament::backend::DriverApi& getDriverApi() { return *commandStream; }
    filament::backend::Driver& getDriver() { return *driver; }

    ImageExpectations& getExpectations() { return *mImageExpectations; }

    static bool matchesEnvironment(Backend backend);
    static bool matchesEnvironment(OperatingSystem operatingSystem);
private:
    // Adds all the images that failed an ImageExpectation to the XML metadata for the current tests
    // case. Add --gtest_output=xml as a command line argument to generate a test_detail.xml file in
    // the directory where the tests are run.
    static void recordFailedImages();

    static std::vector<std::string> sFailedImages;

    filament::backend::Driver* driver = nullptr;
    filament::backend::CommandBufferQueue commandBufferQueue;
    std::unique_ptr<filament::backend::DriverApi> commandStream;

    filament::backend::Handle<filament::backend::HwBufferObject> uniform;

    // This isn't truly optional, it just needs to delay construction until after the driver has
    // been initialized
    std::optional<ImageExpectations> mImageExpectations;
};

} // namespace test

#endif
