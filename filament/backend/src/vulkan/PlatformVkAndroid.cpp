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

#include "VulkanConstants.h"
#include "VulkanDriverFactory.h"

#include <utils/Panic.h>

#include <bluevk/BlueVK.h>

#include <android/native_window.h>

using namespace bluevk;

namespace filament::backend {

Driver* PlatformVkAndroid::createDriver(void* const sharedContext, const Platform::DriverConfig& driverConfig) noexcept {
    ASSERT_PRECONDITION(sharedContext == nullptr, "Vulkan does not support shared contexts.");
    static const char* requiredInstanceExtensions[] = { "VK_KHR_android_surface" };
    return VulkanDriverFactory::create(this, requiredInstanceExtensions, 1, driverConfig);
}

void* PlatformVkAndroid::createVkSurfaceKHR(void* nativeWindow, void* vkinstance, uint64_t flags) noexcept {
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

} // namespace filament::backend
