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
#include <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#ifndef VK_EXT_metal_surface
    #error VK_EXT_metal_surface is not defined
#endif

using namespace bluevk;

namespace filament::backend {

VulkanPlatform::ExtensionSet VulkanPlatform::getSwapchainInstanceExtensionsImpl() {
    ExtensionSet const ret = {
        VK_EXT_METAL_SURFACE_EXTENSION_NAME,
    };
    return ret;
}

VulkanPlatform::SurfaceBundle VulkanPlatform::createVkSurfaceKHRImpl(void* nativeWindow,
        VkInstance instance, uint64_t flags) noexcept {
    VkSurfaceKHR surface;
    CAMetalLayer* mlayer = (__bridge CAMetalLayer*) nativeWindow;
    FILAMENT_CHECK_POSTCONDITION(mlayer) << "Unable to obtain Metal-backed layer.";

    // Create the VkSurface.
    FILAMENT_CHECK_POSTCONDITION(vkCreateMetalSurfaceEXT)
            << "Unable to load vkCreateMetalSurfaceEXT.";
    VkMetalSurfaceCreateInfoEXT createInfo = {
        .sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT,
        .pLayer = mlayer,
    };
    VkResult result =
            vkCreateMetalSurfaceEXT(instance, &createInfo, VKALLOC, &surface);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
            << "vkCreateMacOSSurfaceMVK. error=" << static_cast<int32_t>(result);
    return std::make_tuple(surface, VkExtent2D{});
}

} // namespace filament::backend
