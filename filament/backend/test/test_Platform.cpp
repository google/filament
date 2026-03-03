/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include "BackendTest.h"

#include <backend/Platform.h>
#include <private/backend/PlatformFactory.h>

#include <gtest/gtest.h>

namespace test {

using namespace filament;
using namespace filament::backend;

TEST_F(BackendTest, PlatformGetDeviceInfo) {
    Platform* platform = &getPlatform();
    Driver* driver = &getDriver();
    ASSERT_NE(platform, nullptr);
    ASSERT_NE(driver, nullptr);

    // Test valid queries for the current platform (will be either GL, Vulkan, or Metal depending on
    // host)
    if (matchesEnvironment(Backend::OPENGL)) {
        platform->getDeviceInfo(Platform::DeviceInfoType::OPENGL_RENDERER, driver);
        platform->getDeviceInfo(Platform::DeviceInfoType::OPENGL_VENDOR, driver);

        // Death tests for Vulkan info on OpenGL platform
        EXPECT_DEATH(platform->getDeviceInfo(Platform::DeviceInfoType::VULKAN_DEVICE_NAME, driver),
                "Unsupported DeviceInfoType");
    } else if (matchesEnvironment(Backend::VULKAN)) {
        platform->getDeviceInfo(Platform::DeviceInfoType::VULKAN_DEVICE_NAME, driver);
        platform->getDeviceInfo(Platform::DeviceInfoType::VULKAN_DRIVER_NAME, driver);
        platform->getDeviceInfo(Platform::DeviceInfoType::VULKAN_DRIVER_INFO, driver);

        // Death tests for OpenGL info on Vulkan platform
        EXPECT_DEATH(platform->getDeviceInfo(Platform::DeviceInfoType::OPENGL_RENDERER, driver),
                "Unsupported DeviceInfoType");
    }
}

} // namespace test
