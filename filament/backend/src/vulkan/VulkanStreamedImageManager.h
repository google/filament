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

#ifndef TNT_FILAMENT_BACKEND_CACHING_VULKANSTREAMEDIMAGEMANAGER_H
#define TNT_FILAMENT_BACKEND_CACHING_VULKANSTREAMEDIMAGEMANAGER_H

#include "VulkanHandles.h"

#include <backend/DriverEnums.h>

#include <array>
#include <vector>

namespace filament::backend {

class VulkanExternalImageManager;
class VulkanDescriptorSetCache;
class VulkanSamplerCache;

// Manages the logic of streamed and streamed images.
class VulkanStreamedImageManager {
public:
    VulkanStreamedImageManager(
            VulkanExternalImageManager* manager,
            VulkanDescriptorSetCache* descriptorSet,
            VulkanSamplerCache* samplerCache);
    ~VulkanStreamedImageManager();
    void terminate();

public:
    void bindStreamedTexture(fvkmemory::resource_ptr<VulkanDescriptorSet> set, uint8_t bindingPoint,
            fvkmemory::resource_ptr<VulkanTexture> image, SamplerParams samplerParams);
    void unbindStreamedTexture(fvkmemory::resource_ptr<VulkanDescriptorSet> set,
            uint8_t bindingPoint);
    void onStreamAcquireImage(fvkmemory::resource_ptr<VulkanTexture> image,
            fvkmemory::resource_ptr<VulkanStream> stream, bool newImage);

private:
    struct streamedTextureBinding {
        uint8_t binding = 0;
        fvkmemory::resource_ptr<VulkanTexture> image;
        fvkmemory::resource_ptr<VulkanDescriptorSet> set;
        SamplerParams samplerParams;
    };
    // keep track of all the stream bindings
    std::vector<streamedTextureBinding> mStreamedTexturesBindings;

    VulkanExternalImageManager* mExternalImageManager;
    VulkanDescriptorSetCache*   mDescriptorSetCache;
    VulkanSamplerCache*         mSamplerCache;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_CACHING_VULKANSTREAMEDIMAGEMANAGER_H
