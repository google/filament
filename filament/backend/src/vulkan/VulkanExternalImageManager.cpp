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

} // anonymous

VulkanExternalImageManager::VulkanExternalImageManager(VulkanPlatform* platform,
        VulkanSamplerCache* samplerCache, VulkanYcbcrConversionCache* ycbcrConversionCache,
        VulkanDescriptorSetCache* setCache, VulkanDescriptorSetLayoutCache* layoutCache)
    : mPlatform(platform),
      mSamplerCache(samplerCache),
      mYcbcrConversionCache(ycbcrConversionCache),
      mDescriptorSetCache(setCache),
      mDescriptorSetLayoutCache(layoutCache) {
}

VulkanExternalImageManager::~VulkanExternalImageManager() = default;

void VulkanExternalImageManager::terminate() {
    mSetAndLayouts.clear();
    mSetBindings.clear();
    mImages.clear();
}

void VulkanExternalImageManager::onBeginFrame() {
    std::for_each(mImages.begin(), mImages.end(), [](ImageData& image) {
        image.hasBeenValidated = false;
    });
}

bool VulkanExternalImageManager::prepareBindSets(SetArray const& sets) {
    bool hasUpdated = false;
    for (auto set: sets) {
        if (!set) {
            continue;
        }
        if (auto itr = std::find_if(mSetAndLayouts.begin(), mSetAndLayouts.end(),
                    [&](auto const& setAndLayout) { return setAndLayout.first == set; });
                itr != mSetAndLayouts.end()) {
            hasUpdated = updateSetAndLayout(itr->first, itr->second) || hasUpdated;
        }
    }
    return hasUpdated;
}

bool VulkanExternalImageManager::updateSetAndLayout(
        fvkmemory::resource_ptr<VulkanDescriptorSet> set,
        fvkmemory::resource_ptr<VulkanDescriptorSetLayout> layout) {
    auto findImage = [&](fvkmemory::resource_ptr<VulkanTexture> texture) -> ImageData* {
        auto itr = std::find_if(mImages.begin(), mImages.end(), [&](ImageData const& data) {
            return data.ptr == texture;
        });
        assert_invariant(itr != mImages.end());
        return &(*itr);
    };

    //std::vector<std::pair<uint8_t, ImageData*>> externalImages;
    utils::FixedCapacityVector<std::pair<uint8_t, VkSampler>> samplerAndBindings;
    samplerAndBindings.reserve(MAX_SAMPLER_COUNT);

    bool hasImageUpdates = false;
    for (auto& bindingInfo : mSetBindings) {
        if (bindingInfo.set != set) {
            continue;
        }
        auto imageData = findImage(bindingInfo.image);
        hasImageUpdates = updateImage(imageData) || hasImageUpdates;

        auto samplerParams = bindingInfo.samplerParams;
        // according to spec, these must match chromaFilter
        // https://registry.khronos.org/vulkan/specs/latest/man/html/VkSamplerCreateInfo.html#VUID-VkSamplerCreateInfo-minFilter-01645
        samplerParams.filterMag = SamplerMagFilter::NEAREST;
        samplerParams.filterMin = SamplerMinFilter::NEAREST;

        auto sampler = mSamplerCache->getSampler({
            .sampler = samplerParams,
            .conversion = imageData->conversion,
        });
        samplerAndBindings.push_back({ bindingInfo.binding, sampler });
    }

    // We need to sort by binding number
    std::sort(samplerAndBindings.begin(), samplerAndBindings.end());

    utils::FixedCapacityVector<VkSampler> outSamplers;
    outSamplers.reserve(MAX_SAMPLER_COUNT);
    std::for_each(samplerAndBindings.begin(), samplerAndBindings.end(),
            [&](auto const& b) { outSamplers.push_back(b.second); });

    VkDescriptorSetLayout const oldLayout = layout->getVkLayout();
    VkDescriptorSetLayout const newLayout =
            mDescriptorSetLayoutCache->getVkLayout(layout->bitmask, outSamplers);
    bool const hasLayoutUpdate = oldLayout != newLayout;
    layout->setVkLayout(newLayout);

    assert_invariant(
            (!hasImageUpdates && !hasLayoutUpdate) ||
            (hasImageUpdates && hasLayoutUpdate));

    if (!hasLayoutUpdate) {
        return false;
    }

    auto foldBitsInHalf = [](auto bitset) {
        constexpr size_t BITMASK_LOWER_BITS_LEN = sizeof(bitset) * 4;
        decltype(bitset) outBitset;
        bitset.forEachSetBit([&](size_t index) { outBitset.set(index % BITMASK_LOWER_BITS_LEN); });
        return outBitset;
    };
    // We need to build a new descriptor set from the new layout
    VkDescriptorSet oldSet = set->getVkSet();
    VkDescriptorSet newSet = mDescriptorSetCache->getVkSet(layout);

    using Bitmask = fvkutils::UniformBufferBitmask;
    static_assert(sizeof(Bitmask) * 8 == fvkutils::MAX_DESCRIPTOR_SET_BITMASK_BITS);

    auto const ubo = layout->bitmask.ubo | layout->bitmask.dynamicUbo;
    auto const samplers = layout->bitmask.sampler & (~layout->bitmask.externalSampler);

    // each bitmask denotes a binding index, and separated into two stages - vertex and buffer
    // We fold the two stages into just the lower half of the bits to denote a combined set of
    // bindings.
    Bitmask const copyBindings = foldBitsInHalf(ubo | samplers);

    // TODO: fix the size for better memory
    std::vector<VkCopyDescriptorSet> copies;
    copyBindings.forEachSetBit([&](size_t index) {
        copies.push_back({
            .sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET,
            .srcSet = oldSet,
            .srcBinding = (uint32_t) index,
            .dstSet = newSet,
            .dstBinding = (uint32_t) index,
            .descriptorCount = 1,
        });
    });
    vkUpdateDescriptorSets(mPlatform->getDevice(), 0, nullptr, copies.size(), copies.data());

    set->setVkSet(newSet);

    // We need to release the vkset, which is no longer used, back into the pool.
    mDescriptorSetCache->manualRecyle(layout->count, oldLayout, oldSet);

    // We need to update the external samplers in the set
    for (auto& bindingInfo: mSetBindings) {
        if (bindingInfo.set != set) {
            continue;
        }
        mDescriptorSetCache->updateSampler(set, bindingInfo.binding, bindingInfo.image,
                VK_NULL_HANDLE);
    }
    return true;
}

VkSamplerYcbcrConversion VulkanExternalImageManager::getVkSamplerYcbcrConversion(
        VulkanPlatform::ExternalImageMetadata const& metadata) {
    // This external image does not require external sampler (YUV conversion).
    if (metadata.externalFormat == 0) {
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
        .format = metadata.filamentFormat,
        .externalFormat = metadata.externalFormat,
    };
    return mYcbcrConversionCache->getConversion(ycbcrParams);
}

bool VulkanExternalImageManager::updateImage(ImageData* image) {
    if (image->hasBeenValidated) {
        return false;
    }
    image->hasBeenValidated = true;

    auto metadata = mPlatform->extractExternalImageMetadata(image->platformHandle);
    auto vkYcbcr = getVkSamplerYcbcrConversion(metadata);
    if (vkYcbcr == image->conversion) {
        return false;
    }

    image->ptr->setYcbcrConversion(vkYcbcr, metadata.externalFormat != 0);
    image->conversion = vkYcbcr;
    return true;
}

void VulkanExternalImageManager::addDescriptorSet(
        fvkmemory::resource_ptr<VulkanDescriptorSetLayout> layout,
        fvkmemory::resource_ptr<VulkanDescriptorSet> set) {
    mSetAndLayouts.push_back({set, layout});
}

void VulkanExternalImageManager::removeDescriptorSet(
        fvkmemory::resource_ptr<VulkanDescriptorSet> inSet) {
    erasep<SetAndLayout>(mSetAndLayouts,
            [&](auto const& setLayout) { return (setLayout.first == inSet); });
    erasep<SetBindingInfo>(mSetBindings,
            [&](auto const& bindingInfo) { return (bindingInfo.set == inSet); });
}

void VulkanExternalImageManager::removeDescriptorSetLayout(
        fvkmemory::resource_ptr<VulkanDescriptorSetLayout> inLayout) {
    erasep<SetAndLayout>(mSetAndLayouts,
            [&](auto const& setLayout) { return (setLayout.second == inLayout); });
}

void VulkanExternalImageManager::bindExternallySampledTexture(
        fvkmemory::resource_ptr<VulkanDescriptorSet> set, uint8_t bindingPoint,
        fvkmemory::resource_ptr<VulkanTexture> image, SamplerParams samplerParams) {
    // Should we do duplicate validation here?
    mSetBindings.push_back({ bindingPoint, image, set, samplerParams });
}

void VulkanExternalImageManager::addExternallySampledTexture(
        fvkmemory::resource_ptr<VulkanTexture> image,
        Platform::ExternalImageHandleRef platformHandleRef) {
    mImages.push_back({ image, platformHandleRef, false });
}

void VulkanExternalImageManager::removeExternallySampledTexture(
        fvkmemory::resource_ptr<VulkanTexture> image) {
    erasep<SetBindingInfo>(mSetBindings,
            [&](auto const& bindingInfo) { return (bindingInfo.image == image); });
    erasep<ImageData>(mImages, [&](auto const& imageData) { return imageData.ptr == image; });
}

bool VulkanExternalImageManager::isExternallySampledTexture(
        fvkmemory::resource_ptr<VulkanTexture> image) const {
    return std::find_if(mImages.begin(), mImages.end(),
                   [&](auto const& imageData) { return imageData.ptr == image; }) != mImages.end();
}


} // namesapce filament::backend
