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

#include "VulkanCommands.h"
#include "VulkanConstants.h"
#include "VulkanMemory.h"
#include "VulkanStagePool.h"
#include "vulkan/memory/Resource.h"
#include "vulkan/memory/ResourcePointer.h"
#include "vulkan/utils/Image.h"

#include <utils/Hash.h>
#include <utils/RangeMap.h>

#include <unordered_map>

namespace filament::backend {

struct VulkanTexture;

struct VulkanStream : public HwStream, fvkmemory::ThreadSafeResource {
    fvkmemory::resource_ptr<VulkanTexture> getTexture(void* ahb) {
        fvkmemory::resource_ptr<VulkanTexture> frame;
        const auto& it = mTextures.find(ahb);
        if (it != mTextures.end()) frame = it->second;
        return frame;
    }
    void pushImage(void* ahb, fvkmemory::resource_ptr<VulkanTexture> tex) { mTextures[ahb] = tex; }
    void acquire(const AcquiredImage& image) {
        mPrevious = mAcquired;
        mAcquired = image;
    }
    bool previousNeedsRelease() const { return (mPrevious.image != nullptr); }
    // this function will null the previous once the caller takes it.
    // It ensures we don't schedule for release twice.
    AcquiredImage takePrevious() {
        AcquiredImage previous = mPrevious;
        mPrevious = { nullptr, nullptr, nullptr, nullptr };
        return previous;
    }
    const AcquiredImage& getAcquired() const { return mAcquired; }

private:
    AcquiredImage mAcquired;
    AcquiredImage mPrevious;
    std::map<void*, fvkmemory::resource_ptr<VulkanTexture>> mTextures;
};

struct VulkanTextureState : public fvkmemory::Resource {
    VulkanTextureState(VulkanStagePool& stagePool, VulkanCommands* commands, VmaAllocator allocator,
            VkDevice device, VkImage image, VkDeviceMemory deviceMemory, VkFormat format,
            VkImageViewType viewType, uint8_t levels, uint8_t layerCount,
            VkSamplerYcbcrConversion ycbcrConversion, VkImageUsageFlags usage, bool isProtected);

    ~VulkanTextureState();

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

    VkImageView getImageView(VkImageSubresourceRange range, VkImageViewType viewType,
            VkComponentMapping swizzle);
private:
    void clearCachedImageViews() noexcept;
    VulkanStagePool& mStagePool;
    VulkanCommands* const mCommands;
    VmaAllocator const mAllocator;
    VkDevice const mDevice;

    // The texture with the sidecar owns the sidecar.
    fvkmemory::resource_ptr<VulkanTexture> mSidecarMSAA;
    // The stream this texture is associated with (I think cleaner than HwTexture::hwStream).
    fvkmemory::resource_ptr<VulkanStream> mStream;

    VkImage const mTextureImage;
    VkDeviceMemory const mTextureImageMemory;
    VkFormat const mVkFormat;
    VkImageViewType const mViewType;
    VkImageSubresourceRange const mFullViewRange;

    // Note that this parameter is not constant due to the fact that AHB can force a change in the
    // conversion matrix per-frame.
    struct Ycbcr {
        VkSamplerYcbcrConversion conversion;

        bool operator==(Ycbcr const& other) const {
            return conversion == other.conversion;
        }

        bool operator!=(Ycbcr const& other) const {
            return !((*this) == other);
        }

    } mYcbcr;

    VulkanLayout const mDefaultLayout;
    VkImageUsageFlags const mUsage;
    bool const mIsProtected;

    // Track the image layout of each subresource using a sparse range map.
    utils::RangeMap<uint32_t, VulkanLayout> mSubresourceLayouts;
    using ImageViewHash = utils::hash::MurmurHashFn<ImageViewKey>;
    std::unordered_map<ImageViewKey, VkImageView, ImageViewHash> mCachedImageViews;


    friend struct VulkanTexture;
};

struct VulkanTexture : public HwTexture, fvkmemory::Resource {
    // Standard constructor for user-facing textures.
    VulkanTexture(VkDevice device, VkPhysicalDevice physicalDevice, VulkanContext const& context,
            VmaAllocator allocator, fvkmemory::ResourceManager* resourceManager,
            VulkanCommands* commands, SamplerType target, uint8_t levels, TextureFormat tformat,
            uint8_t samples, uint32_t w, uint32_t h, uint32_t depth, TextureUsage tusage,
            VulkanStagePool& stagePool);

    // Specialized constructor for internally created textures (e.g. from a swap chain)
    VulkanTexture(VulkanContext const& context, VkDevice device, VmaAllocator allocator,
            fvkmemory::ResourceManager* resourceManager, VulkanCommands* commands, VkImage image,
            VkDeviceMemory memory, VkFormat format, VkSamplerYcbcrConversion conversion,
            uint8_t samples, uint32_t width, uint32_t height, uint32_t depth,
            TextureUsage tusage, VulkanStagePool& stagePool);

    // Constructor for creating a texture view for wrt specific mip range
    VulkanTexture(VkDevice device, VkPhysicalDevice physicalDevice, VulkanContext const& context,
            VmaAllocator allocator, VulkanCommands* commands,
            fvkmemory::resource_ptr<VulkanTexture> src, uint8_t baseLevel, uint8_t levelCount);

    // Constructor for creating a texture view for swizzle.
    VulkanTexture(VkDevice device, VkPhysicalDevice physicalDevice, VulkanContext const& context,
            VmaAllocator allocator, VulkanCommands* commands,
            fvkmemory::resource_ptr<VulkanTexture> src, VkComponentMapping swizzle);

    ~VulkanTexture() = default;

    // Uploads data into a subregion of a 2D or 3D texture.
    void updateImage(const PixelBufferDescriptor& data, uint32_t width, uint32_t height,
            uint32_t depth, uint32_t xoffset, uint32_t yoffset, uint32_t zoffset, uint32_t miplevel);

    VkImageViewType getViewType() const {
        return mState->mViewType;
    }

    VkImageSubresourceRange const& getPrimaryViewRange() const { return mPrimaryViewRange; }

    VulkanLayout getSamplerLayout() const {
        if (!isSampleable()) {
            return VulkanLayout::UNDEFINED;
        }
        if (mState->mUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return VulkanLayout::DEPTH_SAMPLER;
        }
        return VulkanLayout::FRAG_READ;
    }

    // Returns the layout for the intended use of this texture (and not the expected layout at the
    // time of the call.
    VulkanLayout getDefaultLayout() const;

    // Gets or creates a cached VkImageView for a subresource that can be used as a render
    // target attachment.  Unlike the primary image view, this always the identity swizzle.
    VkImageView getAttachmentView(VkImageSubresourceRange const& range, VkImageViewType type);

    VkImageView getView(VkImageSubresourceRange const& range);

    VkFormat getVkFormat() const {
        return mState->mVkFormat;
    }
    VkImage getVkImage() const {
        return mState->mTextureImage;
    }

    VulkanLayout getLayout(uint32_t layer, uint32_t level) const;

    void setSidecar(fvkmemory::resource_ptr<VulkanTexture> sidecar) {
        mState->mSidecarMSAA = sidecar;
    }

    fvkmemory::resource_ptr<VulkanTexture> getSidecar() const {
        return mState->mSidecarMSAA;
    }

    void setStream(fvkmemory::resource_ptr<VulkanStream> stream) { mState->mStream = stream;
    }

    fvkmemory::resource_ptr<VulkanStream> getStream() const { return mState->mStream; }

    bool isTransientAttachment() const {
        return mState->mUsage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    }

    bool isSampleable() const {
        return mState->mUsage & VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    bool getIsProtected() const {
        return mState->mIsProtected;
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

    // This is used in the case of external images and external samplers. AHB might update the
    // conversion per-frame. This implies that we need to invalidate the view cache when that
    // happens.
    void setYcbcrConversion(VkSamplerYcbcrConversion conversion);

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

    fvkmemory::resource_ptr<VulkanTextureState> mState;

    // Track the range of subresources that define the "primary" image view, which is the special
    // image view that gets bound to an actual texture sampler.
    VkImageSubresourceRange mPrimaryViewRange;

    VkComponentMapping mSwizzle {};
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANTEXTURE_H
