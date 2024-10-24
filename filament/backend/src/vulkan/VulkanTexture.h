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

#include <utils/Hash.h>
#include <utils/RangeMap.h>

#include <unordered_map>

namespace filament::backend {

class VulkanResourceAllocator;

struct VulkanTextureState : public VulkanResource {
    VulkanTextureState(VkDevice device, VmaAllocator allocator, VulkanCommands* commands,
            VulkanStagePool& stagePool, VkFormat format, VkImageViewType viewType, uint8_t levels,
            uint8_t layerCount, VulkanLayout defaultLayout, bool isProtected);

    struct ImageViewKey {
        VkImageSubresourceRange range;  // 4 * 5 bytes
        VkImageViewType type;           // 4 bytes
        VkComponentMapping swizzle;     // 4 * 4 bytes

        bool operator==(ImageViewKey const& k2) const {
            auto const& k1 = *this;
            return k1.range.aspectMask == k2.range.aspectMask
                   && k1.range.baseMipLevel == k2.range.baseMipLevel
                   && k1.range.levelCount == k2.range.levelCount
                   && k1.range.baseArrayLayer == k2.range.baseArrayLayer
                   && k1.range.layerCount == k2.range.layerCount && k1.type == k2.type
                   && k1.swizzle.r == k2.swizzle.r && k1.swizzle.g == k2.swizzle.g
                   && k1.swizzle.b == k2.swizzle.b && k1.swizzle.a == k2.swizzle.a;
        }
    };
    // No implicit padding allowed due to it being a hash key.
    static_assert(sizeof(ImageViewKey) == 40);

    using ImageViewHash = utils::hash::MurmurHashFn<ImageViewKey>;

    uint32_t refs = 1;

    // The texture with the sidecar owns the sidecar.
    std::unique_ptr<VulkanTexture> mSidecarMSAA;
    VkDeviceMemory mTextureImageMemory = VK_NULL_HANDLE;

    VkFormat const mVkFormat;
    VkImageViewType const mViewType;
    VkImageSubresourceRange const mFullViewRange;
    VkImage mTextureImage = VK_NULL_HANDLE;
    VulkanLayout mDefaultLayout;

    bool mIsProtected = false;

    // Track the image layout of each subresource using a sparse range map.
    utils::RangeMap<uint32_t, VulkanLayout> mSubresourceLayouts;

    std::unordered_map<ImageViewKey, VkImageView, ImageViewHash> mCachedImageViews;
    VulkanStagePool& mStagePool;
    VkDevice mDevice;
    VmaAllocator mAllocator;
    VulkanCommands* mCommands;
    bool mIsTransientAttachment;
};


struct VulkanTexture : public HwTexture, VulkanResource {
    // Standard constructor for user-facing textures.
    VulkanTexture(VkDevice device, VkPhysicalDevice physicalDevice, VulkanContext const& context,
            VmaAllocator allocator, VulkanCommands* commands,
            VulkanResourceAllocator* handleAllocator,
            SamplerType target, uint8_t levels,
            TextureFormat tformat, uint8_t samples, uint32_t w, uint32_t h, uint32_t depth,
            TextureUsage tusage, VulkanStagePool& stagePool, bool heapAllocated = false);

    // Specialized constructor for internally created textures (e.g. from a swap chain)
    // The texture will never destroy the given VkImage, but it does manages its subresources.
    VulkanTexture(VkDevice device, VmaAllocator allocator, VulkanCommands* commands,
            VulkanResourceAllocator* handleAllocator,
            VkImage image,
            VkFormat format, uint8_t samples, uint32_t width, uint32_t height, TextureUsage tusage,
            VulkanStagePool& stagePool, bool heapAllocated = false);

    // Constructor for creating a texture view for wrt specific mip range
    VulkanTexture(VkDevice device, VkPhysicalDevice physicalDevice, VulkanContext const& context,
            VmaAllocator allocator, VulkanCommands* commands,
            VulkanResourceAllocator* handleAllocator,
            VulkanTexture const* src, uint8_t baseLevel, uint8_t levelCount);

    // Constructor for creating a texture view for swizzle.
    VulkanTexture(VkDevice device, VkPhysicalDevice physicalDevice, VulkanContext const& context,
            VmaAllocator allocator, VulkanCommands* commands,
            VulkanResourceAllocator* handleAllocator,
            VulkanTexture const* src, VkComponentMapping swizzle);

    ~VulkanTexture();

    // Uploads data into a subregion of a 2D or 3D texture.
    void updateImage(const PixelBufferDescriptor& data, uint32_t width, uint32_t height,
            uint32_t depth, uint32_t xoffset, uint32_t yoffset, uint32_t zoffset, uint32_t miplevel);

    // Returns the primary image view, which is used for shader sampling.
    VkImageView getPrimaryImageView() {
        VulkanTextureState* state = getSharedState();
        return getImageView(mPrimaryViewRange, state->mViewType, mSwizzle);
    }

    VkImageViewType getViewType() const {
        VulkanTextureState const* state = getSharedState();
        return state->mViewType;
    }

    VkImageSubresourceRange getPrimaryViewRange() const { return mPrimaryViewRange; }

    VulkanLayout getPrimaryImageLayout() const {
        return getLayout(mPrimaryViewRange.baseArrayLayer, mPrimaryViewRange.baseMipLevel);
    }

    // Returns the layout for the intended use of this texture (and not the expected layout at the
    // time of the call.
    VulkanLayout getDefaultLayout() const;

    // Gets or creates a cached VkImageView for a single subresource that can be used as a render
    // target attachment.  Unlike the primary image view, this always has type VK_IMAGE_VIEW_TYPE_2D
    // and the identity swizzle.
    VkImageView getAttachmentView(VkImageSubresourceRange range);

    // Gets or creates a cached VkImageView for a single subresource that can be used as a render
    // target attachment when rendering with multiview.
    VkImageView getMultiviewAttachmentView(VkImageSubresourceRange range);

    // This is a workaround for the first few frames where we're waiting for the texture to actually
    // be uploaded.  In that case, we bind the sampler to an empty texture, but the corresponding
    // imageView needs to be of the right type. Hence, we provide an option to indicate the
    // view type. Swizzle option does not matter in this case.
    VkImageView getViewForType(VkImageSubresourceRange const& range, VkImageViewType type);

    VkFormat getVkFormat() const {
        VulkanTextureState const* state = getSharedState();
        return state->mVkFormat;
    }
    VkImage getVkImage() const {
        VulkanTextureState const* state = getSharedState();
        return state->mTextureImage;
    }

    VulkanLayout getLayout(uint32_t layer, uint32_t level) const;

    void setSidecar(VulkanTexture* sidecar) {
        VulkanTextureState* state = getSharedState();
        state->mSidecarMSAA.reset(sidecar);
    }

    VulkanTexture* getSidecar() const {
        VulkanTextureState const* state = getSharedState();
        return state->mSidecarMSAA.get();
    }

    bool isTransientAttachment() const {
        VulkanTextureState const* state = getSharedState();
        return state->mIsTransientAttachment;
    }

    bool getIsProtected() const {
        VulkanTextureState const* state = getSharedState();
        return state->mIsProtected;
    }

    bool transitionLayout(VulkanCommandBuffer* commands, VkImageSubresourceRange const& range,
            VulkanLayout newLayout);

    bool transitionLayout(VkCommandBuffer cmdbuf, VkImageSubresourceRange const& range,
            VulkanLayout newLayout);

    void attachmentToSamplerBarrier(VulkanCommandBuffer* commands,
            VkImageSubresourceRange const& range);

    void samplerToAttachmentBarrier(VulkanCommandBuffer* commands,
            VkImageSubresourceRange const& range);

    // Returns the preferred data plane of interest for all image views.
    // For now this always returns either DEPTH or COLOR.
    VkImageAspectFlags getImageAspect() const;

    // For implicit transition like the end of a render pass, we need to be able to set the layout
    // manually (outside of calls to transitionLayout).
    void setLayout(VkImageSubresourceRange const& range, VulkanLayout newLayout);

#if FVK_ENABLED(FVK_DEBUG_TEXTURE)
    void print() const;
#endif

private:
    VulkanTextureState* getSharedState();
    VulkanTextureState const* getSharedState() const;

    // Gets or creates a cached VkImageView for a range of miplevels, array layers, viewType, and
    // swizzle (or not).
    VkImageView getImageView(VkImageSubresourceRange range, VkImageViewType viewType,
            VkComponentMapping swizzle);

    void updateImageWithBlit(const PixelBufferDescriptor& hostData, uint32_t width, uint32_t height,
            uint32_t depth, uint32_t miplevel);

    VulkanResourceAllocator* const mAllocator;

    Handle<VulkanTextureState> mState;

    // Track the range of subresources that define the "primary" image view, which is the special
    // image view that gets bound to an actual texture sampler.
    VkImageSubresourceRange mPrimaryViewRange;

    VkComponentMapping mSwizzle {};
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANTEXTURE_H
