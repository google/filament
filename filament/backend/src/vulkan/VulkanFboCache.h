/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DRIVER_VULKANFBOCACHE_H
#define TNT_FILAMENT_DRIVER_VULKANFBOCACHE_H

#include "VulkanContext.h"

#include <utils/Hash.h>

#include <tsl/robin_map.h>

namespace filament {
namespace backend {

// Simple manager for VkFramebuffer and VkRenderPass objects.
//
// Note that a VkFramebuffer is just a binding between a render pass and a set of image views. So,
// this is NOT a cache of actual offscreen rendering surfaces. The Vulkan objects that it manages
// do not consume any GPU memory.
//
class VulkanFboCache {
public:
    // RenderPassKey is a small POD representing the immutable state that is used to construct
    // a VkRenderPass. It is hashed and used as a lookup key. Portions of this are extracted
    // from backend::RenderPassParams.
    struct alignas(8) RenderPassKey {
        VkImageLayout finalColorLayout;  // 4 bytes
        VkImageLayout finalDepthLayout;  // 4 bytes
        VkFormat colorFormat; // 4 bytes
        VkFormat depthFormat; // 4 bytes
        union {
            struct {
                TargetBufferFlags clear;
                TargetBufferFlags discardStart;
                TargetBufferFlags discardEnd;
                uint8_t padding0;
            };
            uint32_t value; // 4 bytes
        } flags;
        uint32_t padding1; // 4 bytes
    };
    struct RenderPassVal {
        VkRenderPass handle;
        uint32_t timestamp;
    };
    static_assert(sizeof(VkFormat) == 4, "VkFormat has unexpected size.");
    static_assert(sizeof(RenderPassKey) == 24, "RenderPassKey has unexpected size.");
    using RenderPassHash = utils::hash::MurmurHashFn<RenderPassKey>;
    struct RenderPassEq {
        bool operator()(const RenderPassKey& k1, const RenderPassKey& k2) const;
    };

    // FboKey is a small POD representing the immutable state that we wish to configure
    // in VkFramebuffer. It is hashed and used as a lookup key. There are 1-3 attachments, but
    // rather than storing a count, we simply zero out the unused slots. We do not bother storing
    // width and height in the key since they are immutable aspects of the image views.
    struct alignas(8) FboKey {
        VkRenderPass renderPass; // 8 bytes
        VkImageView attachments[3]; // 24 bytes
    };
    struct FboVal {
        VkFramebuffer handle;
        uint32_t timestamp;
    };
    static_assert(sizeof(VkRenderPass) == 8, "VkRenderPass has unexpected size.");
    static_assert(sizeof(VkImageView) == 8, "VkImageView has unexpected size.");
    static_assert(sizeof(FboKey) == 32, "FboKey has unexpected size.");
    using FboKeyHashFn = utils::hash::MurmurHashFn<FboKey>;
    struct FboKeyEqualFn {
        bool operator()(const FboKey& k1, const FboKey& k2) const;
    };

    explicit VulkanFboCache(VulkanContext&);
    ~VulkanFboCache();

    // Retrieves or creates a VkFramebuffer handle. Width and height are used only when
    // creating the framebuffer (they are not used for lookup),
    VkFramebuffer getFramebuffer(FboKey config, uint32_t width, uint32_t height) noexcept;

    // Retrieves or creates a VkRenderPass handle.
    VkRenderPass getRenderPass(RenderPassKey config) noexcept;

    // Evicts old unused Vulkan objects. Call this once per frame.
    void gc() noexcept;

    // Frees all Vulkan objects. Call this during shutdown before the device is destroyed.
    void reset() noexcept;

private:
    VulkanContext& mContext;
    tsl::robin_map<FboKey, FboVal, FboKeyHashFn, FboKeyEqualFn> mFramebufferCache;
    tsl::robin_map<RenderPassKey, RenderPassVal, RenderPassHash, RenderPassEq> mRenderPassCache;
    tsl::robin_map<VkRenderPass, uint32_t> mRenderPassRefCount;
    uint32_t mCurrentTime = 0;
    static constexpr uint32_t TIME_BEFORE_EVICTION = 2;
};

} // namespace filament
} // namespace backend

#endif // TNT_FILAMENT_DRIVER_VULKANFBOCACHE_H
