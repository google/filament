/*
 * Copyright (C) 2021 The Android Open Source Project
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

 #ifndef TNT_FILAMENT_BACKEND_VULKANTEXTURE_H
 #define TNT_FILAMENT_BACKEND_VULKANTEXTURE_H

#include "DriverBase.h"

#include "VulkanBuffer.h"
#include "VulkanResources.h"
#include "VulkanImageUtility.h"

#include <utils/RangeMap.h>

namespace filament::backend {

struct VulkanTexture : public HwTexture, VulkanResource {
    // Standard constructor for user-facing textures.
    VulkanTexture(VkDevice device, VkPhysicalDevice physicalDevice, VulkanContext const& context,
            VmaAllocator allocator, VulkanCommands* commands, SamplerType target, uint8_t levels,
            TextureFormat tformat, uint8_t samples, uint32_t w, uint32_t h, uint32_t depth,
            TextureUsage tusage, VulkanStagePool& stagePool, bool heapAllocated = false,
            VkComponentMapping swizzle = {});

    // Specialized constructor for internally created textures (e.g. from a swap chain)
    // The texture will never destroy the given VkImage, but it does manages its subresources.
    VulkanTexture(VkDevice device, VmaAllocator allocator, VulkanCommands* commands, VkImage image,
            VkFormat format, uint8_t samples, uint32_t width, uint32_t height, TextureUsage tusage,
            VulkanStagePool& stagePool, bool heapAllocated = false);

    ~VulkanTexture();

    // Uploads data into a subregion of a 2D or 3D texture.
    void updateImage(const PixelBufferDescriptor& data, uint32_t width, uint32_t height,
            uint32_t depth, uint32_t xoffset, uint32_t yoffset, uint32_t zoffset, uint32_t miplevel);

    // Returns the primary image view, which is used for shader sampling.
    VkImageView getPrimaryImageView() const { return mCachedImageViews.at(mPrimaryViewRange); }

    // Sets the min/max range of miplevels in the primary image view.
    void setPrimaryRange(uint32_t minMiplevel, uint32_t maxMiplevel);

    VkImageSubresourceRange getPrimaryRange() const { return mPrimaryViewRange; }

    VulkanLayout getPrimaryImageLayout() const {
        return getLayout(mPrimaryViewRange.baseArrayLayer, mPrimaryViewRange.baseMipLevel);
    }

    // Gets or creates a cached VkImageView for a single subresource that can be used as a render
    // target attachment.  Unlike the primary image view, this always has type VK_IMAGE_VIEW_TYPE_2D
    // and the identity swizzle.
    VkImageView getAttachmentView(VkImageSubresourceRange range);

    VkFormat getVkFormat() const { return mVkFormat; }
    VkImage getVkImage() const { return mTextureImage; }

    VulkanLayout getLayout(uint32_t layer, uint32_t level) const;

    void setSidecar(VulkanTexture* sidecar) {
        mSidecarMSAA.reset(sidecar);
    }

    VulkanTexture* getSidecar() const {
        return mSidecarMSAA.get();
    }

    void transitionLayout(VkCommandBuffer commands, const VkImageSubresourceRange& range,
            VulkanLayout newLayout);

    // Returns the preferred data plane of interest for all image views.
    // For now this always returns either DEPTH or COLOR.
    VkImageAspectFlags getImageAspect() const;

#if FVK_ENABLED(FVK_DEBUG_TEXTURE)
    void print() const;
#endif

private:
    // Gets or creates a cached VkImageView for a range of miplevels, array layers, viewType, and
    // swizzle (or not).
    VkImageView getImageView(VkImageSubresourceRange range, VkImageViewType viewType,
            VkComponentMapping swizzle);

    void updateImageWithBlit(const PixelBufferDescriptor& hostData, uint32_t width, uint32_t height,
            uint32_t depth, uint32_t miplevel);

    // The texture with the sidecar owns the sidecar.
    std::unique_ptr<VulkanTexture> mSidecarMSAA;
    const VkFormat mVkFormat;
    const VkImageViewType mViewType;
    const VkComponentMapping mSwizzle;
    VkImage mTextureImage = VK_NULL_HANDLE;
    VkDeviceMemory mTextureImageMemory = VK_NULL_HANDLE;

    // Track the image layout of each subresource using a sparse range map.
    utils::RangeMap<uint32_t, VulkanLayout> mSubresourceLayouts;

    // Track the range of subresources that define the "primary" image view, which is the special
    // image view that gets bound to an actual texture sampler.
    VkImageSubresourceRange mPrimaryViewRange;

    std::map<VkImageSubresourceRange, VkImageView> mCachedImageViews;
    VulkanStagePool& mStagePool;
    VkDevice mDevice;
    VmaAllocator mAllocator;
    VulkanCommands* mCommands;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANTEXTURE_H
