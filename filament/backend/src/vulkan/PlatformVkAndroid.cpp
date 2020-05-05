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

#include "vulkan/PlatformVkAndroid.h"

#include "VulkanDriverFactory.h"

#include <utils/Panic.h>

#include <bluevk/BlueVK.h>

#include <android/native_window.h>

namespace filament {

using namespace backend;

// All vkCreate* functions take an optional allocator. For now we select the default allocator by
// passing in a null pointer, and we highlight the argument by using the VKALLOC constant.
constexpr VkAllocationCallbacks* VKALLOC = nullptr;

Driver* PlatformVkAndroid::createDriver(void* const sharedContext) noexcept {
    ASSERT_PRECONDITION(sharedContext == nullptr, "Vulkan does not support shared contexts.");
    static const char* requestedExtensions[] = {
        "VK_KHR_surface",
        "VK_KHR_android_surface",
#if !defined(NDEBUG)
        "VK_EXT_debug_report",
#endif
    };
    return VulkanDriverFactory::create(this, requestedExtensions,
            sizeof(requestedExtensions) / sizeof(requestedExtensions[0]));
}

void* PlatformVkAndroid::createVkSurfaceKHR(void* nativeWindow, void* vkinstance) noexcept {
    const VkInstance instance = (VkInstance) vkinstance;
    ANativeWindow* aNativeWindow = (ANativeWindow*) nativeWindow;
    VkAndroidSurfaceCreateInfoKHR createInfo {
        .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
        .window = aNativeWindow
    };
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult result = vkCreateAndroidSurfaceKHR(instance, &createInfo, VKALLOC, &surface);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateAndroidSurfaceKHR error.");
    return (void*) surface;
}

void PlatformVkAndroid::getClientExtent(void* window, uint32_t* width, uint32_t* height) noexcept {
    ANativeWindow* aNativeWindow = (ANativeWindow*) window;
    *width = ANativeWindow_getWidth(aNativeWindow);
    *height = ANativeWindow_getHeight(aNativeWindow);
}

} // namespace filament
