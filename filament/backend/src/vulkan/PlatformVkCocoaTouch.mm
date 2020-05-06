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

#include "vulkan/PlatformVkCocoaTouch.h"

#include "VulkanDriverFactory.h"

// Metal is not available when building for the iOS simulator on Desktop.
#define METAL_AVAILABLE __has_include(<QuartzCore/CAMetalLayer.h>)

#if METAL_AVAILABLE
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#endif

#include <bluevk/BlueVK.h>
#include <utils/Panic.h>

#ifndef VK_MVK_ios_surface
#error VK_MVK_ios_surface is not defined
#endif

#define METALVIEW_TAG 255

namespace filament {

using namespace backend;

Driver* PlatformVkCocoaTouch::createDriver(void* const sharedContext) noexcept {
    ASSERT_PRECONDITION(sharedContext == nullptr, "Vulkan does not support shared contexts.");
    static const char* requestedExtensions[] = {"VK_KHR_surface", "VK_MVK_ios_surface"};
    return VulkanDriverFactory::create(this, requestedExtensions,
            sizeof(requestedExtensions) / sizeof(requestedExtensions[0]));
}

void* PlatformVkCocoaTouch::createVkSurfaceKHR(void* nativeWindow, void* instance) noexcept {
#if METAL_AVAILABLE
    CAMetalLayer* metalLayer = (CAMetalLayer*) nativeWindow;

    // Create the VkSurface.
    ASSERT_POSTCONDITION(vkCreateIOSSurfaceMVK, "Unable to load vkCreateIOSSurfaceMVK function.");
    VkSurfaceKHR surface = nullptr;
    VkIOSSurfaceCreateInfoMVK createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.pView = metalLayer;
    VkResult result = vkCreateIOSSurfaceMVK((VkInstance) instance, &createInfo, VKALLOC, &surface);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateIOSSurfaceMVK error.");

    return surface;
#else
    return nullptr;
#endif
}

void PlatformVkCocoaTouch::getClientExtent(void* window,  uint32_t* width, uint32_t* height) {
#if METAL_AVAILABLE
    CAMetalLayer* metalLayer = (CAMetalLayer*) nativeWindow;
    *width = metalLayer.drawableSize.width;
    *height = metalLayer.drawableSize.height;
#endif
}

} // namespace filament
