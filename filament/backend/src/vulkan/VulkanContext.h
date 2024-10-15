/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_VULKANCONTEXT_H
#define TNT_FILAMENT_BACKEND_VULKANCONTEXT_H

#include "VulkanConstants.h"
#include "VulkanImageUtility.h"
#include "VulkanUtility.h"

#include "vulkan/memory/ResourcePointer.h"

#include <utils/bitset.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Mutex.h>
#include <utils/Slice.h>

#include <memory>

VK_DEFINE_HANDLE(VmaAllocator)
VK_DEFINE_HANDLE(VmaPool)

namespace filament::backend {

struct VulkanRenderTarget;
struct VulkanSwapChain;
struct VulkanTexture;
class VulkanStagePool;
struct VulkanTimerQuery;
struct VulkanCommandBuffer;

struct VulkanAttachment {
    fvkmemory::resource_ptr<VulkanTexture> texture;
    uint8_t level = 0;
    uint8_t layerCount = 1;
    uint8_t layer = 0;

    bool isDepth() const;
    VkImage getImage() const;
    VkFormat getFormat() const;
    VulkanLayout getLayout() const;
    VkExtent2D getExtent2D() const;
    VkImageView getImageView();
    // TODO: maybe embed aspect into the attachment or texture itself.
    VkImageSubresourceRange getSubresourceRange() const;
};

class VulkanTimestamps {
public:
    using QueryResult = std::array<uint64_t, 4>;

    VulkanTimestamps(VkDevice device);
    ~VulkanTimestamps();

    // Not copy-able.
    VulkanTimestamps(VulkanTimestamps const&) = delete;
    VulkanTimestamps& operator=(VulkanTimestamps const&) = delete;

    std::tuple<uint32_t, uint32_t> getNextQuery();
    void clearQuery(uint32_t queryIndex);

    void beginQuery(VulkanCommandBuffer const* commands,
            fvkmemory::resource_ptr<VulkanTimerQuery> query);
    void endQuery(VulkanCommandBuffer const* commands,
            fvkmemory::resource_ptr<VulkanTimerQuery> query);
    QueryResult getResult(fvkmemory::resource_ptr<VulkanTimerQuery> query);

private:
    VkDevice mDevice;
    VkQueryPool mPool;
    utils::bitset32 mUsed;
    utils::Mutex mMutex;
};

struct VulkanRenderPass {
    fvkmemory::resource_ptr<VulkanRenderTarget> renderTarget;
    VkRenderPass renderPass;
    RenderPassParams params;
    int currentSubpass;
};

// This is a collection of immutable data about the vulkan context. This actual handles to the
// context are stored in VulkanPlatform.
struct VulkanContext {
public:
    inline uint32_t selectMemoryType(uint32_t flags, VkFlags reqs) const {
        for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
            if (flags & 1) {
                if ((mMemoryProperties.memoryTypes[i].propertyFlags & reqs) == reqs) {
                    return i;
                }
            }
            flags >>= 1;
        }
        return (uint32_t) VK_MAX_MEMORY_TYPES;
    }

    inline VkFormatList const& getAttachmentDepthStencilFormats() const {
        return mDepthStencilFormats;
    }

    inline VkFormatList const& getBlittableDepthStencilFormats() const {
        return mBlittableDepthStencilFormats;
    }

    inline VkPhysicalDeviceLimits const& getPhysicalDeviceLimits() const noexcept {
        return mPhysicalDeviceProperties.limits;
    }

    inline uint32_t getPhysicalDeviceVendorId() const noexcept {
        return mPhysicalDeviceProperties.vendorID;
    }

    inline bool isImageCubeArraySupported() const noexcept {
        return mPhysicalDeviceFeatures.imageCubeArray == VK_TRUE;
    }

    inline bool isDepthClampSupported() const noexcept {
        return mPhysicalDeviceFeatures.depthClamp == VK_TRUE;
    }

    inline bool isDebugMarkersSupported() const noexcept {
        return mDebugMarkersSupported;
    }

    inline bool isDebugUtilsSupported() const noexcept {
        return mDebugUtilsSupported;
    }

    inline bool isMultiviewEnabled() const noexcept {
        return mMultiviewEnabled;
    }

    inline bool isClipDistanceSupported() const noexcept {
        return mPhysicalDeviceFeatures.shaderClipDistance == VK_TRUE;
    }

    inline bool isLazilyAllocatedMemorySupported() const noexcept {
        return mLazilyAllocatedMemorySupported;
    }

private:
    VkPhysicalDeviceMemoryProperties mMemoryProperties = {};
    VkPhysicalDeviceProperties mPhysicalDeviceProperties = {};
    VkPhysicalDeviceFeatures mPhysicalDeviceFeatures = {};
    bool mDebugMarkersSupported = false;
    bool mDebugUtilsSupported = false;
    bool mMultiviewEnabled = false;
    bool mLazilyAllocatedMemorySupported = false;

    VkFormatList mDepthStencilFormats;
    VkFormatList mBlittableDepthStencilFormats;

    // For convenience so that VulkanPlatform can initialize the private fields.
    friend class VulkanPlatform;
};

}// namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_VULKANCONTEXT_H
