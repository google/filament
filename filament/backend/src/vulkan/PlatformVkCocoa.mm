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

#include "vulkan/PlatformVkCocoa.h"

#include "VulkanConstants.h"
#include "VulkanDriverFactory.h"

#include <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include <bluevk/BlueVK.h>
#include <utils/Panic.h>

#ifndef VK_MVK_macos_surface
#error VK_MVK_macos_surface is not defined
#endif

using namespace bluevk;

namespace filament {

using namespace backend;

Driver* PlatformVkCocoa::createDriver(void* sharedContext) noexcept {
    ASSERT_PRECONDITION(sharedContext == nullptr, "Vulkan does not support shared contexts.");
    static const char* requiredInstanceExtensions[] = {
        "VK_MVK_macos_surface", // TODO: replace with VK_EXT_metal_surface
    };
    return VulkanDriverFactory::create(this, requiredInstanceExtensions, 1);
}

void* PlatformVkCocoa::createVkSurfaceKHR(void* nativeWindow, void* instance, uint64_t flags) noexcept {
    // Obtain the CAMetalLayer-backed view.
    NSView* nsview = (__bridge NSView*) nativeWindow;
    ASSERT_POSTCONDITION(nsview, "Unable to obtain Metal-backed NSView.");

    // Create the VkSurface.
    ASSERT_POSTCONDITION(vkCreateMacOSSurfaceMVK, "Unable to load vkCreateMacOSSurfaceMVK.");
    VkSurfaceKHR surface = nullptr;
    VkMacOSSurfaceCreateInfoMVK createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
    createInfo.pView = (__bridge void*) nsview;
    VkResult result = vkCreateMacOSSurfaceMVK((VkInstance) instance, &createInfo, VKALLOC, &surface);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateMacOSSurfaceMVK error.");

    return surface;
}

} // namespace filament
