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

#include "VulkanDriverFactory.h"

#include <utils/Panic.h>

#include <bluevk/BlueVK.h>

namespace filament {

using namespace backend;

Driver* PlatformVkWindows::createDriver(void* const sharedContext) noexcept {
    ASSERT_PRECONDITION(sharedContext == nullptr, "Vulkan does not support shared contexts.");
    const char* requestedExtensions[] = {
        "VK_KHR_surface",
        "VK_KHR_win32_surface",
#if !defined(NDEBUG)
        "VK_EXT_debug_report",
#endif
    };
    return VulkanDriverFactory::create(this, requestedExtensions,
        sizeof(requestedExtensions) / sizeof(requestedExtensions[0]));
}

void* PlatformVkWindows::createVkSurfaceKHR(void* nativeWindow, void* instance) noexcept {
    VkSurfaceKHR surface = nullptr;

    HWND window = (HWND) nativeWindow;

    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = window;
    createInfo.hinstance = GetModuleHandle(nullptr);

    VkResult result = vkCreateWin32SurfaceKHR((VkInstance) instance, &createInfo, nullptr, &surface);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateWin32SurfaceKHR error.");

    return surface;
}

void PlatformVkWindows::getClientExtent(void* win, uint32_t* width, uint32_t* height) noexcept {
	HWND window = (HWND)win;
	RECT rect;
	BOOL success = GetClientRect(window, &rect);
	ASSERT_POSTCONDITION(success, "GetWindowRect error.");
	*width = rect.right - rect.left;
	*height = rect.bottom - rect.top;
}

} // namespace filament
