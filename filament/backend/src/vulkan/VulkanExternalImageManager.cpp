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

#include "VulkanExternalImageManager.h"

#include "VulkanDescriptorSetCache.h"
#include "VulkanDescriptorSetLayoutCache.h"
#include "VulkanSamplerCache.h"
#include "VulkanYcbcrConversionCache.h"
#include "vulkan/memory/ResourcePointer.h"
#include "vulkan/utils/Conversion.h"

#include <backend/platforms/VulkanPlatform.h>

#include <algorithm>

namespace filament::backend {

namespace {

template<typename T>
void erasep(std::vector<T>& v, std::function<bool(T const&)> f) {
    auto newEnd = std::remove_if(v.begin(), v.end(), f);
    v.erase(newEnd, v.end());
}

using ImageData = VulkanExternalImageManager::VulkanExternalImageManager::ImageData;
ImageData& findImage(std::vector<ImageData>& images,
        fvkmemory::resource_ptr<VulkanTexture> texture) {
    auto itr = std::find_if(images.begin(), images.end(), [&](ImageData const& data) {
        return data.image == texture;
    });
    assert_invariant(itr != images.end());
    return *itr;
}

}// namespace

VulkanExternalImageManager::VulkanExternalImageManager(VulkanSamplerCache* samplerCache,
        VulkanYcbcrConversionCache* ycbcrConversionCache, VulkanDescriptorSetCache* setCache,
        VulkanDescriptorSetLayoutCache* layoutCache)
        : mSamplerCache(samplerCache),
          mYcbcrConversionCache(ycbcrConversionCache),
          mDescriptorSetCache(setCache),
          mDescriptorSetLayoutCache(layoutCache) {}

VulkanExternalImageManager::~VulkanExternalImageManager() = default;

void VulkanExternalImageManager::terminate() {
    mSetBindings.clear();
    mImages.clear();
}

void VulkanExternalImageManager::updateSetAndLayout(
        fvkmemory::resource_ptr<VulkanDescriptorSet> set) {
    utils::FixedCapacityVector<
            std::tuple<uint8_t, VkSampler, fvkmemory::resource_ptr<VulkanTexture>>>
            samplerAndBindings;
    samplerAndBindings.reserve(MAX_SAMPLER_COUNT);

    fvkutils::SamplerBitmask actualExternalSamplers;
    for (auto& bindingInfo: mSetBindings) {
        if (bindingInfo.set != set) {
            continue;
        }
        actualExternalSamplers.set(bindingInfo.binding);
        samplerAndBindings.push_back(
                { bindingInfo.binding, bindingInfo.sampler, bindingInfo.image });
    }

    // Sort by binding number
    std::sort(samplerAndBindings.begin(), samplerAndBindings.end(), [](auto const& a, auto const& b) {
        return std::get<0>(a) < std::get<0>(b);
    });

    utils::FixedCapacityVector<std::pair<uint64_t,VkSampler>> outSamplers;
    outSamplers.reserve(MAX_SAMPLER_COUNT);
    std::for_each(samplerAndBindings.begin(), samplerAndBindings.end(),
            [&](auto const& b) { outSamplers.push_back({ static_cast<uint64_t>(std::get<0>(b)), std::get<1>(b) }); });

    fvkmemory::resource_ptr<VulkanDescriptorSetLayout> const& layout = set->getLayout();
    set->boundLayout = mDescriptorSetLayoutCache->getVkLayout(layout->bitmask,
            actualExternalSamplers, outSamplers);
    // Update the external samplers in the set
    for (auto& [binding, sampler, image]: samplerAndBindings) {
        // We cannot call updateSamplerForExternalSamplerSet because some samplers are non NULL
        // (RGB) and we cannot do a combined update with a NULL sampler.
        mDescriptorSetCache->updateSampler(set, binding, image, sampler, set->boundLayout);
    }
}

VkSamplerYcbcrConversion VulkanExternalImageManager::getVkSamplerYcbcrConversion(
        VulkanPlatform::ExternalImageMetadata const& metadata) {
    // This external image does not require external sampler (YUV conversion).
    if (metadata.externalFormat == 0 && !fvkutils::isVKYcbcrConversionFormat(metadata.format)) {
        return VK_NULL_HANDLE;
    }
    VulkanYcbcrConversionCache::Params ycbcrParams = {
        .conversion = {
            .ycbcrModel = fvkutils::getYcbcrModelConversionFilament(metadata.ycbcrModel),
            .r = fvkutils::getSwizzleFilament(metadata.ycbcrConversionComponents.r, 0),
            .g = fvkutils::getSwizzleFilament(metadata.ycbcrConversionComponents.g, 1),
            .b = fvkutils::getSwizzleFilament(metadata.ycbcrConversionComponents.b, 2),
            .a = fvkutils::getSwizzleFilament(metadata.ycbcrConversionComponents.a, 3),
            .ycbcrRange = fvkutils::getYcbcrRangeFilament(metadata.ycbcrRange),
            .xChromaOffset = fvkutils::getChromaLocationFilament(metadata.xChromaOffset),
            .yChromaOffset = fvkutils::getChromaLocationFilament(metadata.yChromaOffset),

            // Unclear where to get the chromaFilter, we just assume it's nearest.
            .chromaFilter = SamplerMagFilter::NEAREST,
        },
        .format = metadata.format,
        .externalFormat = metadata.externalFormat,
    };
    return mYcbcrConversionCache->getConversion(ycbcrParams);
}

void VulkanExternalImageManager::removeDescriptorSet(
        fvkmemory::resource_ptr<VulkanDescriptorSet> inSet) {
    erasep<SetBindingInfo>(mSetBindings,
            [&](auto const& bindingInfo) { return (bindingInfo.set == inSet); });
}

void VulkanExternalImageManager::bindExternallySampledTexture(
        fvkmemory::resource_ptr<VulkanDescriptorSet> set, uint8_t bindingPoint,
        fvkmemory::resource_ptr<VulkanTexture> image, SamplerParams samplerParams) {
    // Should we do duplicate validation here?
    auto& imageData = findImage(mImages, image);
    // according to spec, these must match chromaFilter
    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkSamplerCreateInfo.html#VUID-VkSamplerCreateInfo-minFilter-01645
    samplerParams.filterMag = SamplerMagFilter::NEAREST;
    samplerParams.filterMin = SamplerMinFilter::NEAREST;

    VkSampler const sampler = mSamplerCache->getSampler({
        .sampler = samplerParams,
        .conversion = imageData.conversion,
    });

    mSetBindings.push_back({ bindingPoint, imageData.image, set, sampler });
}

void VulkanExternalImageManager::addExternallySampledTexture(
        fvkmemory::resource_ptr<VulkanTexture> image, VkSamplerYcbcrConversion const conversion) {
    mImages.push_back({
        .image = image,
        .conversion = conversion,
    });
}

void VulkanExternalImageManager::removeExternallySampledTexture(
        fvkmemory::resource_ptr<VulkanTexture> image) {
    erasep<SetBindingInfo>(mSetBindings,
            [&](auto const& bindingInfo) { return (bindingInfo.image == image); });
    erasep<ImageData>(mImages, [&](auto const& imageData) {
        return imageData.image == image;
    });
}

bool VulkanExternalImageManager::isExternallySampledTexture(
        fvkmemory::resource_ptr<VulkanTexture> image) const {
    return std::find_if(mImages.begin(), mImages.end(), [&](auto const& imageData) {
        return imageData.image == image;
    }) != mImages.end();
}

void VulkanExternalImageManager::clearTextureBinding(
        fvkmemory::resource_ptr<VulkanDescriptorSet> set, uint8_t bindingPoint) {
    erasep<SetBindingInfo>(mSetBindings, [&](auto const& bindingInfo) {
        return (bindingInfo.set == set && bindingInfo.binding == bindingPoint);
    });
}

} // namesapce filament::backend
