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

#include "vulkan/VulkanSamplerCache.h"
#include "vulkan/utils/Conversion.h"

#include <utils/Panic.h>

using namespace bluevk;

namespace filament::backend {

constexpr inline VkFilter getFilter(SamplerMinFilter filter) noexcept {
    switch (filter) {
        case SamplerMinFilter::NEAREST:
            return VK_FILTER_NEAREST;
        case SamplerMinFilter::LINEAR:
            return VK_FILTER_LINEAR;
        case SamplerMinFilter::NEAREST_MIPMAP_NEAREST:
            return VK_FILTER_NEAREST;
        case SamplerMinFilter::LINEAR_MIPMAP_NEAREST:
            return VK_FILTER_LINEAR;
        case SamplerMinFilter::NEAREST_MIPMAP_LINEAR:
            return VK_FILTER_NEAREST;
        case SamplerMinFilter::LINEAR_MIPMAP_LINEAR:
            return VK_FILTER_LINEAR;
    }
}

constexpr inline VkFilter getFilter(SamplerMagFilter filter) noexcept {
    switch (filter) {
        case SamplerMagFilter::NEAREST:
            return VK_FILTER_NEAREST;
        case SamplerMagFilter::LINEAR:
            return VK_FILTER_LINEAR;
    }
}

VulkanSamplerCache::VulkanSamplerCache(VkDevice device)
    : mDevice(device) {}

VkSampler VulkanSamplerCache::getSampler(SamplerParams params) noexcept {
    auto iter = mCache.find(params);
    if (UTILS_LIKELY(iter != mCache.end())) {
        return iter->second;
    }
    VkSamplerCreateInfo samplerInfo {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = getFilter(params.filterMag),
        .minFilter = getFilter(params.filterMin),
        .mipmapMode = fvkutils::getMipmapMode(params.filterMin),
        .addressModeU = fvkutils::getWrapMode(params.wrapS),
        .addressModeV = fvkutils::getWrapMode(params.wrapT),
        .addressModeW = fvkutils::getWrapMode(params.wrapR),
        .anisotropyEnable = params.anisotropyLog2 == 0 ? 0u : 1u,
        .maxAnisotropy = (float)(1u << params.anisotropyLog2),
        .compareEnable = fvkutils::getCompareEnable(params.compareMode),
        .compareOp = fvkutils::getCompareOp(params.compareFunc),
        .minLod = 0.0f,
        .maxLod = fvkutils::getMaxLod(params.filterMin),
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };
    VkSampler sampler;
    VkResult result = vkCreateSampler(mDevice, &samplerInfo, VKALLOC, &sampler);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS) << "Unable to create sampler."
                                                       << " error=" << static_cast<int32_t>(result);
    mCache.insert({params, sampler});
    return sampler;
}

void VulkanSamplerCache::terminate() noexcept {
    for (auto pair : mCache) {
        vkDestroySampler(mDevice, pair.second, VKALLOC);
    }
    mCache.clear();
}

} // namespace filament::backend
