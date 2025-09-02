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

using Bitmask = fvkutils::UniformBufferBitmask;
static_assert(sizeof(Bitmask) * 8 == fvkutils::MAX_DESCRIPTOR_SET_BITMASK_BITS);

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

void copySet(VkDevice device, VkDescriptorSet srcSet, VkDescriptorSet dstSet, Bitmask bindings) {
    // TODO: fix the size for better memory management
    std::vector<VkCopyDescriptorSet> copies;
    bindings.forEachSetBit([&](size_t index) {
        copies.push_back({
            .sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET,
            .srcSet = srcSet,
            .srcBinding = (uint32_t) index,
            .dstSet = dstSet,
            .dstBinding = (uint32_t) index,
            .descriptorCount = 1,
        });
    });
    vkUpdateDescriptorSets(device, 0, nullptr, copies.size(), copies.data());
}

Bitmask foldBitsInHalf(Bitmask bitset) {
    Bitmask outBitset;
    bitset.forEachSetBit([&](size_t index) {
        constexpr size_t BITMASK_LOWER_BITS_LEN = sizeof(outBitset) * 4;
        outBitset.set(index % BITMASK_LOWER_BITS_LEN);
    });
    return outBitset;
}

}// namespace

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
    mSetBindings.clear();
    mImages.clear();
}

void VulkanExternalImageManager::onBeginFrame() {
    std::for_each(mImages.begin(), mImages.end(), [](ImageData& image) {
        image.hasBeenValidated = false;
    });
    std::for_each(mSetBindings.begin(), mSetBindings.end(), [](SetBindingInfo& info) {
        info.bound = false;
    });
}

fvkutils::DescriptorSetMask VulkanExternalImageManager::prepareBindSets(LayoutArray const& layouts,
        SetArray const& sets) {
    fvkutils::DescriptorSetMask shouldUseExternalSampler{};
    for (uint8_t i = 0; i < sets.size(); i++) {
        auto set = sets[i];
        auto layout = layouts[i];
        if (!set || !layout) {
            continue;
        }
        if (hasExternalSampler(set)) {
            updateSetAndLayout(set, layout);
            shouldUseExternalSampler.set(i);
        }
    }
    return shouldUseExternalSampler;
}

bool VulkanExternalImageManager::hasExternalSampler(
        fvkmemory::resource_ptr<VulkanDescriptorSet> set) {
    auto itr = std::find_if(mSetBindings.begin(), mSetBindings.end(),
            [&](SetBindingInfo const& info) { return info.set == set; });
    return itr != mSetBindings.end();
}

void VulkanExternalImageManager::updateSetAndLayout(
        fvkmemory::resource_ptr<VulkanDescriptorSet> set,
        fvkmemory::resource_ptr<VulkanDescriptorSetLayout> layout) {
    utils::FixedCapacityVector<
            std::tuple<uint8_t, VkSampler, fvkmemory::resource_ptr<VulkanTexture>>>
            samplerAndBindings;
    samplerAndBindings.reserve(MAX_SAMPLER_COUNT);

    fvkutils::SamplerBitmask actualExternalSamplers;
    for (auto& bindingInfo : mSetBindings) {
        if (bindingInfo.set != set || bindingInfo.bound) {
            continue;
        }
        auto& imageData = findImage(mImages, bindingInfo.image);
        updateImage(&imageData);

        auto samplerParams = bindingInfo.samplerParams;
        // according to spec, these must match chromaFilter
        // https://registry.khronos.org/vulkan/specs/latest/man/html/VkSamplerCreateInfo.html#VUID-VkSamplerCreateInfo-minFilter-01645
        samplerParams.filterMag = SamplerMagFilter::NEAREST;
        samplerParams.filterMin = SamplerMinFilter::NEAREST;

        auto sampler = mSamplerCache->getSampler({
            .sampler = samplerParams,
            .conversion = imageData.conversion,
        });
        actualExternalSamplers.set(bindingInfo.binding);
        samplerAndBindings.push_back({ bindingInfo.binding, sampler, bindingInfo.image });
        bindingInfo.bound = true;
    }

    if (samplerAndBindings.empty()) {
        return;
    }

    // Sort by binding number
    std::sort(samplerAndBindings.begin(), samplerAndBindings.end(), [](auto const& a, auto const& b) {
        return std::get<0>(a) < std::get<0>(b);
    });

    utils::FixedCapacityVector<VkSampler> outSamplers;
    outSamplers.reserve(MAX_SAMPLER_COUNT);
    std::for_each(samplerAndBindings.begin(), samplerAndBindings.end(),
            [&](auto const& b) { outSamplers.push_back(std::get<1>(b)); });

    VkDescriptorSetLayout const oldLayout = layout->getExternalSamplerVkLayout();
    VkDescriptorSetLayout const newLayout = mDescriptorSetLayoutCache->getVkLayout(layout->bitmask,
            actualExternalSamplers, outSamplers);

    // Need to copy the set
    VkDescriptorSet const oldSet = set->getExternalSamplerVkSet();
    if (oldLayout != newLayout || oldSet == VK_NULL_HANDLE) {
        // Build a new descriptor set from the new layout
        VkDescriptorSet const newSet = mDescriptorSetCache->getVkSet(layout->count, newLayout);
        auto const ubo = layout->bitmask.ubo | layout->bitmask.dynamicUbo;
        auto const samplers = layout->bitmask.sampler & (~actualExternalSamplers);

        // Each bitmask denotes a binding index, and separated into two stages - vertex and buffer
        // We fold the two stages into just the lower half of the bits to denote a combined set of
        // bindings.
        Bitmask const copyBindings = foldBitsInHalf(ubo | samplers);
        VkDescriptorSet const srcSet = oldSet != VK_NULL_HANDLE ? oldSet : set->getVkSet();
        copySet(mPlatform->getDevice(), srcSet, newSet, copyBindings);

        set->setExternalSamplerVkSet(newSet,
                [&descriptorSetCache = mDescriptorSetCache, layoutCount = layout->count, newLayout,
                        newSet](VulkanDescriptorSet*) {
                    descriptorSetCache->manualRecycle(layoutCount, newLayout, newSet);
                });
        if (oldLayout != newLayout) {
            layout->setExternalSamplerVkLayout(newLayout);
        }
    }

    // Update the external samplers in the set
    for (auto& [binding, sampler, image]: samplerAndBindings) {
        mDescriptorSetCache->updateSamplerForExternalSamplerSet(set, binding, image);
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

void VulkanExternalImageManager::updateImage(ImageData* image) {
    if (image->hasBeenValidated) {
        return;
    }
    image->hasBeenValidated = true;

    auto metadata = mPlatform->extractExternalImageMetadata(image->platformHandle);
    auto vkYcbcr = getVkSamplerYcbcrConversion(metadata);
    if (vkYcbcr == image->conversion) {
        return;
    }

    image->image->setYcbcrConversion(vkYcbcr);
    image->conversion = vkYcbcr;
    return;
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
    mSetBindings.push_back({ bindingPoint, imageData.image, set, samplerParams });
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
