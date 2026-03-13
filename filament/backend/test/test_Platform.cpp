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

#include <backend/Platform.h>
#include <gtest/gtest.h>
#include <private/backend/PlatformFactory.h>

namespace filament::backend {

TEST(PlatformTest, GetDeviceInfo) {
    Backend backend = Backend::DEFAULT;
    Platform* platform = PlatformFactory::create(&backend);
    ASSERT_NE(platform, nullptr);

    // Test valid queries for the current platform (will be either GL, Vulkan, or Metal depending on
    // host)
    if (backend == Backend::OPENGL) {
        platform->getDeviceInfo(Platform::DeviceInfoType::OPENGL_RENDERER, nullptr);
        platform->getDeviceInfo(Platform::DeviceInfoType::OPENGL_VENDOR, nullptr);

        // Death tests for Vulkan info on OpenGL platform
        EXPECT_DEATH(platform->getDeviceInfo(Platform::DeviceInfoType::VULKAN_DEVICE_NAME, nullptr),
                "Unsupported DeviceInfoType");
    } else if (backend == Backend::VULKAN) {
        platform->getDeviceInfo(Platform::DeviceInfoType::VULKAN_DEVICE_NAME, nullptr);
        platform->getDeviceInfo(Platform::DeviceInfoType::VULKAN_DRIVER_NAME, nullptr);
        platform->getDeviceInfo(Platform::DeviceInfoType::VULKAN_DRIVER_INFO, nullptr);

        // Death tests for OpenGL info on Vulkan platform
        EXPECT_DEATH(platform->getDeviceInfo(Platform::DeviceInfoType::OPENGL_RENDERER, nullptr),
                "Unsupported DeviceInfoType");
    }

    PlatformFactory::destroy(&platform);
}

} // namespace filament::backend
