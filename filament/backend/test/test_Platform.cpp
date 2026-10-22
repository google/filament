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

#include <private/backend/Driver.h>
#include <private/backend/PlatformFactory.h>

#include <backend/Platform.h>
#if defined(FILAMENT_DRIVER_SUPPORTS_VULKAN)
#include <backend/platforms/VulkanPlatform.h>
#endif

#include <utils/Panic.h>

#include <gtest/gtest.h>

#include <array>

namespace test {

using namespace filament;
using namespace filament::backend;

class PlatformTest : public BackendTest {
public:
    PlatformTest() = default;
};


TEST_F(PlatformTest, GetDeviceInfo) {
    Platform* platform = getPlatform();
    Driver* driver = &getDriver();
    auto backend = BackendTest::sBackend;

    ASSERT_NE(platform, nullptr);

    // Test valid queries for the current platform (will be either GL, Vulkan, or Metal depending on
    // host)
    if (backend == Backend::OPENGL) {
        platform->getDeviceInfo(Platform::DeviceInfoType::OPENGL_RENDERER, driver);
        platform->getDeviceInfo(Platform::DeviceInfoType::OPENGL_VENDOR, driver);
        platform->getDeviceInfo(Platform::DeviceInfoType::OPENGL_VERSION, driver);

        // Test that calling with nullptr driver on supported types returns empty CString
        EXPECT_TRUE(platform->getDeviceInfo(Platform::DeviceInfoType::OPENGL_RENDERER, nullptr)
                        .empty());
        EXPECT_TRUE(
                platform->getDeviceInfo(Platform::DeviceInfoType::OPENGL_VENDOR, nullptr).empty());
        EXPECT_TRUE(
                platform->getDeviceInfo(Platform::DeviceInfoType::OPENGL_VERSION, nullptr).empty());


        // Death tests for Vulkan info on OpenGL platform
#ifdef __EXCEPTIONS
        EXPECT_THROW(platform->getDeviceInfo(Platform::DeviceInfoType::VULKAN_DEVICE_NAME, nullptr), utils::PostconditionPanic);
#elif defined(GTEST_HAS_DEATH_TEST) && GTEST_HAS_DEATH_TEST
        // Death tests are unavailable on iOS-family platforms.
        EXPECT_DEATH(platform->getDeviceInfo(Platform::DeviceInfoType::VULKAN_DEVICE_NAME, nullptr),
                "Unsupported DeviceInfoType");
#endif
    } else if (backend == Backend::VULKAN) {
        platform->getDeviceInfo(Platform::DeviceInfoType::VULKAN_DEVICE_NAME, driver);
        platform->getDeviceInfo(Platform::DeviceInfoType::VULKAN_DRIVER_NAME, driver);
        platform->getDeviceInfo(Platform::DeviceInfoType::VULKAN_DRIVER_INFO, driver);

        // Death tests for OpenGL info on Vulkan platform
#ifdef __EXCEPTIONS
        EXPECT_THROW(platform->getDeviceInfo(Platform::DeviceInfoType::OPENGL_RENDERER, nullptr), utils::PostconditionPanic);
#elif defined(GTEST_HAS_DEATH_TEST) && GTEST_HAS_DEATH_TEST
        // Death tests are unavailable on iOS-family platforms.
        EXPECT_DEATH(platform->getDeviceInfo(Platform::DeviceInfoType::OPENGL_RENDERER, nullptr),
                "Unsupported DeviceInfoType");
#endif
    }
}

#if defined(FILAMENT_DRIVER_SUPPORTS_VULKAN)
TEST_F(PlatformTest, VulkanRenderTargetFormatSupport) {
    if (BackendTest::sBackend != Backend::VULKAN) {
        GTEST_SKIP() << "This test verifies Vulkan format feature queries.";
    }

    struct FormatFeature {
        TextureFormat textureFormat;
        VkFormat vulkanFormat;
        VkFormatFeatureFlagBits requiredFeature;
    };

    constexpr std::array FORMAT_FEATURES = {
        FormatFeature{ TextureFormat::RGBA8, VK_FORMAT_R8G8B8A8_UNORM,
                VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT },
        FormatFeature{ TextureFormat::DEPTH16, VK_FORMAT_D16_UNORM,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT },
        FormatFeature{ TextureFormat::DEPTH24, VK_FORMAT_X8_D24_UNORM_PACK32,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT },
        FormatFeature{ TextureFormat::DEPTH32F, VK_FORMAT_D32_SFLOAT,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT },
        FormatFeature{ TextureFormat::DEPTH24_STENCIL8, VK_FORMAT_D24_UNORM_S8_UINT,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT },
        FormatFeature{ TextureFormat::DEPTH32F_STENCIL8, VK_FORMAT_D32_SFLOAT_S8_UINT,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT },
        FormatFeature{ TextureFormat::STENCIL8, VK_FORMAT_S8_UINT,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT },
    };

    VkPhysicalDevice const physicalDevice =
            static_cast<VulkanPlatform*>(getPlatform())->getPhysicalDevice();
    for (FormatFeature const& format : FORMAT_FEATURES) {
        VkFormatProperties properties;
        bluevk::vkGetPhysicalDeviceFormatProperties(
                physicalDevice, format.vulkanFormat, &properties);
        bool const expected =
                (properties.optimalTilingFeatures & format.requiredFeature) != 0;
        EXPECT_EQ(expected, getDriver().isRenderTargetFormatSupported(format.textureFormat));
    }
    EXPECT_FALSE(getDriver().isTextureFormatMipmappable(TextureFormat::STENCIL8));
}
#endif

} // namespace test
