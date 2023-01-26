/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include "vulkan/PlatformVkWindows.h"

#include "VulkanConstants.h"
#include "VulkanDriverFactory.h"

#include <utils/Panic.h>

#include <bluevk/BlueVK.h>

using namespace bluevk;

namespace filament::backend {

using SurfaceBundle = VulkanPlatform::SurfaceBundle;

Driver* PlatformVkWindows::createDriver(void* const sharedContext,
        const Platform::DriverConfig& driverConfig) noexcept {
    ASSERT_PRECONDITION(sharedContext == nullptr, "Vulkan does not support shared contexts.");
    const char* requiredInstanceExtensions[] = { "VK_KHR_win32_surface" };
    return VulkanDriverFactory::create(this, requiredInstanceExtensions, 1, driverConfig);
}

SurfaceBundle PlatformVkWindows::createVkSurfaceKHR(void* nativeWindow, void* instance,
        uint64_t flags) noexcept {
    SurfaceBundle bundle {
        .surface = nullptr,
        .width = 0,
        .height = 0
    };
    HWND window = (HWND) nativeWindow;

    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = window;
    createInfo.hinstance = GetModuleHandle(nullptr);

    VkResult result = vkCreateWin32SurfaceKHR((VkInstance) instance, &createInfo, nullptr,
            (VkSurfaceKHR*) &bundle.surface);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateWin32SurfaceKHR error.");

    return bundle;
}

} // namespace filament::backend
