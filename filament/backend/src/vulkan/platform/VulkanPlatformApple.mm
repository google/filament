/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include <backend/platforms/VulkanPlatform.h>

#include "vulkan/VulkanConstants.h"
#include "vulkan/VulkanDriverFactory.h"

#include <utils/Panic.h>

#include <bluevk/BlueVK.h>

// Platform specific includes and defines
#if defined(__APPLE__)
    #include <Cocoa/Cocoa.h>
    #import <Metal/Metal.h>
    #import <QuartzCore/CAMetalLayer.h>

    #ifndef VK_MVK_macos_surface
         #error VK_MVK_macos_surface is not defined
    #endif
#elif defined(IOS)
    // Metal is not available when building for the iOS simulator on Desktop.
    #define METAL_AVAILABLE __has_include(<QuartzCore/CAMetalLayer.h>)
    #if METAL_AVAILABLE
        #import <Metal/Metal.h>
        #import <QuartzCore/CAMetalLayer.h>
    #endif

    #ifndef VK_MVK_ios_surface
        #error VK_MVK_ios_surface is not defined
    #endif
    #define METALVIEW_TAG 255
#else
    #error Not a supported Apple + Vulkan platform
#endif

using namespace bluevk;

namespace filament::backend {

VulkanPlatform::ExtensionSet VulkanPlatform::getSwapchainInstanceExtensions() {
    ExtensionSet const ret = {
    #if defined(__APPLE__)
        VK_MVK_MACOS_SURFACE_EXTENSION_NAME,  // TODO: replace with VK_EXT_metal_surface
    #elif defined(IOS) && defined(METAL_AVAILABLE)
        VK_MVK_IOS_SURFACE_EXTENSION_NAME,
    #endif
    };
    return ret;
}

VulkanPlatform::SurfaceBundle VulkanPlatform::createVkSurfaceKHR(void* nativeWindow,
        VkInstance instance, uint64_t flags) noexcept {
    VkSurfaceKHR surface;
    #if defined(__APPLE__)
        NSView* nsview = (__bridge NSView*) nativeWindow;
        FILAMENT_CHECK_POSTCONDITION(nsview) << "Unable to obtain Metal-backed NSView.";

        // Create the VkSurface.
        FILAMENT_CHECK_POSTCONDITION(vkCreateMacOSSurfaceMVK)
                << "Unable to load vkCreateMacOSSurfaceMVK.";
        VkMacOSSurfaceCreateInfoMVK createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
        createInfo.pView = (__bridge void*) nsview;
        VkResult result = vkCreateMacOSSurfaceMVK((VkInstance) instance, &createInfo, VKALLOC,
                (VkSurfaceKHR*) &surface);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS) << "vkCreateMacOSSurfaceMVK error.";
    #elif defined(IOS) && defined(METAL_AVAILABLE)
        CAMetalLayer* metalLayer = (CAMetalLayer*) nativeWindow;
        // Create the VkSurface.
        FILAMENT_CHECK_POSTCONDITION(vkCreateIOSSurfaceMVK)
                << "Unable to load vkCreateIOSSurfaceMVK function.";
        VkIOSSurfaceCreateInfoMVK createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK;
        createInfo.pNext = NULL;
        createInfo.flags = 0;
        createInfo.pView = metalLayer;
        VkResult result = vkCreateIOSSurfaceMVK((VkInstance) instance, &createInfo, VKALLOC,
                (VkSurfaceKHR*) &surface);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS) << "vkCreateIOSSurfaceMVK error.";
    #endif
    return std::make_tuple(surface, VkExtent2D {});
}

} // namespace filament::backend
