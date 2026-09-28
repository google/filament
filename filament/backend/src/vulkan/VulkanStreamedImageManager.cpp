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
#include "VulkanDescriptorSetLayoutCache.h"
#include "VulkanExternalImageManager.h"
#include "VulkanSamplerCache.h"

#include <algorithm>

namespace filament::backend {
VulkanStreamedImageManager::VulkanStreamedImageManager(VulkanExternalImageManager* manager)
        : mExternalImageManager(manager) {}

VulkanStreamedImageManager::~VulkanStreamedImageManager() = default;

void VulkanStreamedImageManager::terminate() {
    mStreamedTexturesBindings.clear();
    mStreamTextures.clear();
}

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
        fvkmemory::resource_ptr<VulkanStream> stream) {
    for (StreamedTextureBinding& data: mStreamedTexturesBindings) {
        // Find the right stream
        if (data.image->getStream() == stream) {
            data.set->isAnExternalSamplerBound = true;
            data.set->isLayoutDirty = true;
            mExternalImageManager->bindExternallySampledTexture(data.set, data.binding, image,
                    data.samplerParams);
        }
    }
}

fvkmemory::resource_ptr<VulkanTexture> VulkanStreamedImageManager::getTexture(
        fvkmemory::resource_ptr<VulkanStream> stream, void* ahb) const {
    auto const id = stream.id();
    auto iter = std::find_if(mStreamTextures.begin(), mStreamTextures.end(),
            [id, ahb](StreamTexture const& entry) {
                return entry.matches(id, ahb);
            });
    if (iter == mStreamTextures.end()) {
        return {};
    }
    return iter->texture;
}

void VulkanStreamedImageManager::pushImage(fvkmemory::resource_ptr<VulkanStream> stream, void* ahb,
        fvkmemory::resource_ptr<VulkanTexture> tex) {
    // removeStream() has already run for this stream and will never run again, so anything we
    // cache here would be retained until terminate(). Note that the entry would also be keyed on a
    // dead stream, which a future stream could alias.
    if (stream->isDestroyed()) {
        return;
    }
    auto const id = stream.id();
    auto iter = std::find_if(mStreamTextures.begin(), mStreamTextures.end(),
            [id, ahb](StreamTexture const& entry) {
                return entry.matches(id, ahb);
            });
    if (iter != mStreamTextures.end()) {
        iter->texture = tex;
        return;
    }
    mStreamTextures.push_back({ id, ahb, tex });
}

void VulkanStreamedImageManager::removeStream(fvkmemory::resource_ptr<VulkanStream> stream) {
    auto const id = stream.id();
    auto iter = std::remove_if(mStreamTextures.begin(), mStreamTextures.end(),
            [id](StreamTexture const& entry) {
                return entry.stream == id;
            });
    mStreamTextures.erase(iter, mStreamTextures.end());
}

} // namespace filament::backend
