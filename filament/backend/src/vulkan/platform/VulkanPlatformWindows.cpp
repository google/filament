/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include <backend/platforms/VulkanPlatformWindows.h>
#include <backend/DriverEnums.h>

#include "vulkan/VulkanConstants.h"

#include <utils/Panic.h>

#include <bluevk/BlueVK.h>

#include <tuple>

#include <stdint.h>
#include <stddef.h>

#if defined(WIN32)
    #include <windows.h>
#endif

using namespace bluevk;

namespace filament::backend {

VulkanPlatform::ExtensionSet VulkanPlatformWindows::getSwapchainInstanceExtensions() const {
    VulkanPlatform::ExtensionSet const ret = {
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    };
    return ret;
}

VulkanPlatform::SurfaceBundle VulkanPlatformWindows::createVkSurfaceKHR(void* nativeWindow,
        VkInstance instance, uint64_t flags) const noexcept {
    VkSurfaceKHR surface;
    VkExtent2D extent;

    // On (at least) NVIDIA drivers, the Vulkan implementation (specifically the call to
    // vkGetPhysicalDeviceSurfaceCapabilitiesKHR()) does not correctly handle the fact that
    // each native window has its own DPI_AWARENESS_CONTEXT, and erroneously uses the context
    // of the calling thread. As a workaround, we set the current thread's DPI_AWARENESS_CONTEXT
    // to that of the native window we've been given. This isn't a perfect solution, because an
    // application could create swap chains on multiple native windows with varying DPI-awareness,
    // but even then, at least one of the windows would be guaranteed to work correctly.
    SetThreadDpiAwarenessContext(GetWindowDpiAwarenessContext((HWND) nativeWindow));

    VkWin32SurfaceCreateInfoKHR const createInfo = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hinstance = GetModuleHandle(nullptr),
        .hwnd = (HWND) nativeWindow,
    };
    VkResult const result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr,
            (VkSurfaceKHR*) &surface);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
            << "vkCreateWin32SurfaceKHR failed."
            << " error=" << static_cast<int32_t>(result);
    return std::make_tuple(surface, extent);
}

} // namespace filament::backend
