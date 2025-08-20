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

#ifndef TNT_FILAMENT_BACKEND_CACHING_VULKANEXTERNALIMAGEMANAGER_H
#define TNT_FILAMENT_BACKEND_CACHING_VULKANEXTERNALIMAGEMANAGER_H

#include "VulkanHandles.h"

#include <backend/DriverEnums.h>

#include <array>
#include <vector>

namespace filament::backend {

class VulkanYcbcrConversionCache;
class VulkanSamplerCache;
class VulkanDescriptorSetLayoutCache;
class VulkanDescriptorSetCache;

// Manages the logic of external images and their quirks wrt Vulikan.
class VulkanExternalImageManager {
public:

    VulkanExternalImageManager(
            VulkanPlatform* platform,
            VulkanSamplerCache* samplerCache,
            VulkanYcbcrConversionCache* ycbcrConversionCache,
            VulkanDescriptorSetCache* setCache,
            VulkanDescriptorSetLayoutCache* layoutCache);

    ~VulkanExternalImageManager();

    void terminate();

    void onBeginFrame();

    using SetArray = std::array<fvkmemory::resource_ptr<VulkanDescriptorSet>,
            VulkanDescriptorSetLayout::UNIQUE_DESCRIPTOR_SET_COUNT>;

    using LayoutArray = std::array<fvkmemory::resource_ptr<VulkanDescriptorSetLayout>,
            VulkanDescriptorSetLayout::UNIQUE_DESCRIPTOR_SET_COUNT>;

    using VkLayoutArray = VulkanDescriptorSetLayout::DescriptorSetLayoutArray;

    // Returns  bitmask to indicate whether or not to use the external sampler version of each
    // descriptor set.
    fvkutils::DescriptorSetMask prepareBindSets(LayoutArray const& layouts, SetArray const& sets);

    void removeDescriptorSet(fvkmemory::resource_ptr<VulkanDescriptorSet> set);

    void bindExternallySampledTexture(fvkmemory::resource_ptr<VulkanDescriptorSet> set,
            uint8_t bindingPoint, fvkmemory::resource_ptr<VulkanTexture> image,
            SamplerParams samplerParams);
    void bindStream(fvkmemory::resource_ptr<VulkanDescriptorSet> set, uint8_t bindingPoint,
            fvkmemory::resource_ptr<VulkanStream> stream, SamplerParams samplerParams);

    void clearTextureBinding(fvkmemory::resource_ptr<VulkanDescriptorSet> set,
                             uint8_t bindingPoint);

    void addExternallySampledTexture(fvkmemory::resource_ptr<VulkanTexture> external,
            Platform::ExternalImageHandleRef platformHandleRef);

    void removeExternallySampledTexture(fvkmemory::resource_ptr<VulkanTexture> image);

    bool isExternallySampledTexture(fvkmemory::resource_ptr<VulkanTexture> image) const;
    bool isStreamedTexture(fvkmemory::resource_ptr<VulkanTexture> image) const;

    // For a stream backed VulkanTexture, we are receiving new frames periodically, add them to the tracking system
    void bindStreamFrame(fvkmemory::resource_ptr<VulkanStream> stream,
            fvkmemory::resource_ptr<VulkanTexture> frame);

    VkSamplerYcbcrConversion getVkSamplerYcbcrConversion(
            VulkanPlatform::ExternalImageMetadata const& metadata);

    struct ImageData {
        fvkmemory::resource_ptr<VulkanTexture> image;
        Platform::ExternalImageHandle platformHandle;
        bool hasBeenValidated = false; // indicates whether the image has been validated *this frame*
        VkSamplerYcbcrConversion conversion = VK_NULL_HANDLE;
    };

private:
    bool hasExternalSampler(fvkmemory::resource_ptr<VulkanDescriptorSet> set);

    void updateSetAndLayout(fvkmemory::resource_ptr<VulkanDescriptorSet> set,
            fvkmemory::resource_ptr<VulkanDescriptorSetLayout> layout);

    void updateImage(ImageData* imageData);

    VulkanPlatform* mPlatform;
    VulkanSamplerCache* mSamplerCache;
    VulkanYcbcrConversionCache* mYcbcrConversionCache;
    VulkanDescriptorSetCache* mDescriptorSetCache;
    VulkanDescriptorSetLayoutCache* mDescriptorSetLayoutCache;

    using SetAndLayout = std::pair<fvkmemory::resource_ptr<VulkanDescriptorSet>,
            fvkmemory::resource_ptr<VulkanDescriptorSetLayout>>;

    struct SetBindingInfo {
        uint8_t binding = 0;
        fvkmemory::resource_ptr<VulkanTexture> image;
        fvkmemory::resource_ptr<VulkanDescriptorSet> set;
        SamplerParams samplerParams;
        bool bound = false;
    };
    struct SetStreamBindingInfo {
        uint8_t binding = 0;
        fvkmemory::resource_ptr<VulkanStream> stream;
        fvkmemory::resource_ptr<VulkanDescriptorSet> set;
        SamplerParams samplerParams;
    };

    // Use vectors instead of hash maps because we only expect small number of entries.
    std::vector<SetBindingInfo> mSetBindings;
    std::vector<SetStreamBindingInfo> mSetStreamBindings;
    std::vector<ImageData> mImages;
};

} // filament::backend

#endif // TNT_FILAMENT_BACKEND_CACHING_VULKANEXTERNALIMAGEMANAGER_H
