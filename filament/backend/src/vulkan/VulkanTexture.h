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

 #ifndef TNT_FILAMENT_DRIVER_VULKANTEXTURE_H
 #define TNT_FILAMENT_DRIVER_VULKANTEXTURE_H

#include "VulkanDriver.h"
#include "VulkanPipelineCache.h"
#include "VulkanBuffer.h"
#include "VulkanUtility.h"

namespace filament {
namespace backend {

struct VulkanTexture : public HwTexture {
    VulkanTexture(VulkanContext& context, SamplerType target, uint8_t levels,
            TextureFormat format, uint8_t samples, uint32_t w, uint32_t h, uint32_t depth,
            TextureUsage usage, VulkanStagePool& stagePool, VkComponentMapping swizzle = {});
    ~VulkanTexture();
    void update2DImage(const PixelBufferDescriptor& data, uint32_t width, uint32_t height,
            int miplevel);
    void update3DImage(const PixelBufferDescriptor& data, uint32_t width, uint32_t height,
            uint32_t depth, int miplevel);
    void updateCubeImage(const PixelBufferDescriptor& data, const FaceOffsets& faceOffsets,
            int miplevel);

    // Returns the primary image view, which is used for shader sampling.
    VkImageView getPrimaryImageView() const { return mCachedImageViews.at(mPrimaryViewRange); }

    // Sets the min/max range of miplevels in the primary image view.
    void setPrimaryRange(uint32_t minMiplevel, uint32_t maxMiplevel);

    // Gets or creates a cached VkImageView for a range of miplevels and array layers.
    // If force2D is true, this always returns an image view that has type = VK_IMAGE_VIEW_TYPE_2D,
    // regardless of the type of the primary image view.
    VkImageView getImageView(VkImageSubresourceRange range, bool force2D = false);

    // Convenient "single subresource" overload for the above method.
    VkImageView getImageView(int singleLevel, int singleLayer, VkImageAspectFlags aspect) {
        return getImageView({
            .aspectMask = aspect,
            .baseMipLevel = uint32_t(singleLevel),
            .levelCount = uint32_t(1),
            .baseArrayLayer = uint32_t(singleLayer),
            .layerCount = uint32_t(1),
        }, true);
    }

    VkFormat getVkFormat() const { return mVkFormat; }
    VkImage getVkImage() const { return mTextureImage; }

private:
    // Issues a copy from a VkBuffer to a specified miplevel in a VkImage. The given width and
    // height define a subregion within the miplevel.
    void copyBufferToImage(VkCommandBuffer cmdbuffer, VkBuffer buffer, VkImage image,
            uint32_t width, uint32_t height, uint32_t depth,
            FaceOffsets const* faceOffsets, uint32_t miplevel);

    const VkFormat mVkFormat;
    const VkComponentMapping mSwizzle;
    VkImageViewType mViewType;
    VkImage mTextureImage = VK_NULL_HANDLE;
    VkDeviceMemory mTextureImageMemory = VK_NULL_HANDLE;
    VkImageSubresourceRange mPrimaryViewRange;
    std::map<VkImageSubresourceRange, VkImageView> mCachedImageViews;
    VkImageAspectFlags mAspect;
    VulkanContext& mContext;
    VulkanStagePool& mStagePool;
};

} // namespace filament
} // namespace backend

#endif // TNT_FILAMENT_DRIVER_VULKANTEXTURE_H
