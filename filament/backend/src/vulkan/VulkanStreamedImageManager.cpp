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

#include "VulkanStreamedImageManager.h"

#include "VulkanDescriptorSetCache.h"
#include "VulkanExternalImageManager.h"
#include "VulkanDescriptorSetLayoutCache.h"
#include "VulkanSamplerCache.h"

namespace filament::backend {
    VulkanStreamedImageManager::VulkanStreamedImageManager(
        VulkanExternalImageManager* manager,
        VulkanDescriptorSetCache* descriptorSet,
        VulkanSamplerCache* samplerCache)
    : mExternalImageManager(manager),
      mDescriptorSetCache(descriptorSet),
      mSamplerCache(samplerCache) {

}

VulkanStreamedImageManager::~VulkanStreamedImageManager() = default;

void VulkanStreamedImageManager::terminate() { mStreamedTexturesBindings.clear(); }

void VulkanStreamedImageManager::bindStreamedTexture(
        fvkmemory::resource_ptr<VulkanDescriptorSet> set,
        uint8_t bindingPoint, fvkmemory::resource_ptr<VulkanTexture> image,
        SamplerParams samplerParams) {
    mStreamedTexturesBindings.push_back({ bindingPoint, image, set, samplerParams });
}

void VulkanStreamedImageManager::unbindStreamedTexture(
        fvkmemory::resource_ptr<VulkanDescriptorSet> set,
        uint8_t bindingPoint) {
    auto iter = std::remove_if(mStreamedTexturesBindings.begin(), mStreamedTexturesBindings.end(),
            [&](StreamedTextureBinding& binding) {
                return ((binding.set == set) && (binding.binding == bindingPoint));
            });
    mStreamedTexturesBindings.erase(iter, mStreamedTexturesBindings.end());
}

void VulkanStreamedImageManager::onStreamAcquireImage(fvkmemory::resource_ptr<VulkanTexture> image,
        fvkmemory::resource_ptr<VulkanStream> stream, bool newImage) {
    for (StreamedTextureBinding const& data: mStreamedTexturesBindings) {
        // Find the right stream
        if (data.image->getStream() == stream) {
            // For some reason, some of the frames coming to us, are on streams where the
            // descriptor set isn't external...
            if (data.set->getExternalSamplerVkSet()) {
                if (newImage) {
                    mExternalImageManager->bindExternallySampledTexture(data.set, data.binding,
                            image, data.samplerParams);
                }
            } else {
                //... In this case we just default to using the normal path and update the
                // sampler.
                VulkanSamplerCache::Params cacheParams = {
                    .sampler = data.samplerParams,
                };
                VkSampler const vksampler = mSamplerCache->getSampler(cacheParams);
                mDescriptorSetCache->updateSampler(data.set, data.binding, image, vksampler);
            }
        }
    }
}

} // namespace filament::backend
