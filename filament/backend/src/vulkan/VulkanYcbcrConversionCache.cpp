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

#include "VulkanYcbcrConversionCache.h"

#include "vulkan/VulkanConstants.h"
#include "vulkan/utils/Conversion.h"

#include <utils/Panic.h>

#if defined(__ANDROID__)
#include <vulkan/vulkan_android.h>  // for VkExternalFormatANDROID
#endif

using namespace bluevk;

namespace filament::backend {

VulkanYcbcrConversionCache::VulkanYcbcrConversionCache(VkDevice device)
    : mDevice(device) {}

VkSamplerYcbcrConversion VulkanYcbcrConversionCache::getConversion(
        VulkanYcbcrConversionCache::Params params) noexcept {
    auto iter = mCache.find(params);
    if (UTILS_LIKELY(iter != mCache.end())) {
        return iter->second;
    }

    auto const& chroma = params.conversion;
    TextureSwizzle const swizzleArray[] = { chroma.r, chroma.g, chroma.b, chroma.a };
    VkSamplerYcbcrConversionCreateInfo conversionInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO,
        .format = params.format,
        .ycbcrModel = fvkutils::getYcbcrModelConversion(chroma.ycbcrModel),
        .ycbcrRange = fvkutils::getYcbcrRange(chroma.ycbcrRange),
        .components = fvkutils::getSwizzleMap(swizzleArray),
        .xChromaOffset = fvkutils::getChromaLocation(chroma.xChromaOffset),
        .yChromaOffset = fvkutils::getChromaLocation(chroma.yChromaOffset),
        .chromaFilter = fvkutils::getFilter(chroma.chromaFilter),
    };

    // We could put this in the platform class, but that seems like a bit of an overkill
#if defined(__ANDROID__)
    VkExternalFormatANDROID externalFormat = {
        .sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID,
        .externalFormat = params.externalFormat,
    };
    if (params.externalFormat) {
        conversionInfo.pNext = &externalFormat;
        conversionInfo.format = VK_FORMAT_UNDEFINED;
    }
#endif

    VkSamplerYcbcrConversion conversion;
    VkResult result =
            vkCreateSamplerYcbcrConversion(mDevice, &conversionInfo, nullptr, &conversion);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS) << "Unable to create Ycbcr Conversion."
                                                       << " error=" << static_cast<int32_t>(result);

    mCache.insert({ params, conversion });
    return conversion;
}

void VulkanYcbcrConversionCache::terminate() noexcept {
    for (auto& [param, conv]: mCache) {
        vkDestroySamplerYcbcrConversion(mDevice, conv, VKALLOC);
    }
    mCache.clear();
}

} // namespace filament::backend
