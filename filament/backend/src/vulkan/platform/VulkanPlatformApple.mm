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

#ifndef VK_MVK_macos_surface
    #error VK_MVK_macos_surface is not defined
#endif

using namespace bluevk;

namespace filament::backend {

VulkanPlatform::ExtensionSet VulkanPlatform::getSwapchainInstanceExtensionsImpl() {
    ExtensionSet const ret = {
        VK_MVK_MACOS_SURFACE_EXTENSION_NAME,  // TODO: replace with VK_EXT_metal_surface
    };
    return ret;
}

VulkanPlatform::ExternalImageMetadata VulkanPlatform::getExternalImageMetadataImpl(
        ExternalImageHandleRef externalImage, VkDevice device) {
    return {};
}

VulkanPlatform::ImageData VulkanPlatform::createExternalImageDataImpl(
        ExternalImageHandleRef externalImage, VkDevice device,
        const ExternalImageMetadata& metadata, uint32_t memoryTypeIndex, VkImageUsageFlags usage) {
    return {};
}

VkSampler VulkanPlatform::createExternalSamplerImpl(VkDevice device,
    SamplerYcbcrConversion chroma,
    SamplerParams sampler,
    uint32_t internalFormat) {
    return VK_NULL_HANDLE;
}

VkImageView VulkanPlatform::createExternalImageViewImpl(VkDevice device,
        SamplerYcbcrConversion chroma, uint32_t internalFormat, VkImage image,
        VkImageSubresourceRange range, VkImageViewType viewType, VkComponentMapping swizzle) {
    return VK_NULL_HANDLE;
}

VulkanPlatform::SurfaceBundle VulkanPlatform::createVkSurfaceKHRImpl(void* nativeWindow,
        VkInstance instance, uint64_t flags) noexcept {
    VkSurfaceKHR surface;
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
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
            << "vkCreateMacOSSurfaceMVK. error=" << static_cast<int32_t>(result);
    return std::make_tuple(surface, VkExtent2D{});
}

} // namespace filament::backend
