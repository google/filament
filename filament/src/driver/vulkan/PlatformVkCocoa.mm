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

#include "driver/vulkan/PlatformVkCocoa.h"

#include "VulkanDriver.h"

#include <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include <bluevk/BlueVK.h>
#include <filament/SwapChain.h>
#include <utils/Panic.h>

#ifndef VK_MVK_macos_surface
#error VK_MVK_macos_surface is not defined
#endif

#define METALVIEW_TAG 255

namespace filament {

using namespace driver;

Driver* PlatformVkCocoa::createDriver(void* sharedContext) noexcept {
    ASSERT_PRECONDITION(sharedContext == nullptr, "Vulkan does not support shared contexts.");
    static const char* requestedExtensions[] = {"VK_KHR_surface", "VK_MVK_macos_surface"};
    return VulkanDriver::create(this, requestedExtensions,
            sizeof(requestedExtensions) / sizeof(requestedExtensions[0]));
}

void* PlatformVkCocoa::createVkSurfaceKHR(void* nativeWindow, void* instance,
        uint32_t* width, uint32_t* height) noexcept {
    // Obtain the CAMetalLayer-backed view.
    NSView* nsview = (NSView*) nativeWindow;
    nsview = [nsview viewWithTag:METALVIEW_TAG];
    ASSERT_POSTCONDITION(nsview, "Unable to obtain Metal-backed NSView.");
    CAMetalLayer* mlayer = (CAMetalLayer*) nsview.layer;
    ASSERT_POSTCONDITION(mlayer, "Unable to obtain CAMetalLayer from NSView.");

    // Create the VkSurface.
    ASSERT_POSTCONDITION(vkCreateMacOSSurfaceMVK, "Unable to load vkCreateMacOSSurfaceMVK function.");
    VkSurfaceKHR surface = nullptr;
    VkMacOSSurfaceCreateInfoMVK createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
    createInfo.pView = nsview;
    VkResult result = vkCreateMacOSSurfaceMVK((VkInstance) instance, &createInfo, VKALLOC, &surface);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateMacOSSurfaceMVK error.");

    // The size that we return to VulkanDriver is consistent with what the macOS client sees for the
    // view size, but it's not necessarily consistent with the surface caps currentExtent. We've
    // observed that if the window was initially created on a high DPI display, then dragged to a
    // low DPI display, the VkSurfaceKHR physical caps still have a high resolution, despite the
    // fact that we've recreated it.
    NSSize sz = [nsview convertSizeToBacking: nsview.frame.size];
    *width = sz.width;
    *height = sz.height;
    return surface;
}

} // namespace filament
