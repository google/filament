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

#ifndef TNT_FILAMENT_BACKEND_VULKANFBOCACHE_H
#define TNT_FILAMENT_BACKEND_VULKANFBOCACHE_H

#include "VulkanContext.h"
#include "vulkan/memory/Resource.h"
#include "vulkan/memory/ResourceManager.h"
#include "vulkan/memory/ResourcePointer.h"

#include <utils/Hash.h>

#include <backend/TargetBufferInfo.h>

#include <tsl/robin_map.h>

namespace filament::backend {

struct VulkanRenderPass;

// Simple manager for VkFramebuffer and VkRenderPass objects.
//
// Note that a VkFramebuffer is just a binding between a render pass and a set of image views. So,
// this is NOT a cache of actual offscreen rendering surfaces. The Vulkan objects that it manages
// do not consume any GPU memory.
//
class VulkanFboCache {
public:
    constexpr static VulkanLayout FINAL_COLOR_ATTACHMENT_LAYOUT = VulkanLayout::COLOR_ATTACHMENT;
    constexpr static VulkanLayout FINAL_RESOLVE_ATTACHMENT_LAYOUT = VulkanLayout::COLOR_ATTACHMENT;
    constexpr static VulkanLayout FINAL_DEPTH_ATTACHMENT_LAYOUT = VulkanLayout::DEPTH_ATTACHMENT;

    // RenderPassKey is a small POD representing the immutable state that is used to construct
    // a VkRenderPass. It is hashed and used as a lookup key.
    struct alignas(8) RenderPassKey {
        VkFormat colorFormat[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT]; // 32 bytes
        VkFormat depthFormat; // 4 bytes
        TargetBufferFlags clear; // 4 bytes
        TargetBufferFlags discardStart; // 4 bytes
        TargetBufferFlags discardEnd; // 4 bytes

        VulkanLayout initialDepthLayout; // 1 byte
        uint8_t samples; // 1 byte
        uint8_t needsResolveMask; // 1 byte
        uint8_t usesLazilyAllocatedMemory; // 1 byte
        uint8_t subpassMask; // 1 byte
        uint8_t viewCount; // 1 byte
        uint8_t padding[2];
    };
    struct RenderPassVal {
        fvkmemory::resource_ptr<VulkanRenderPass> handle;
        uint32_t timestamp;
    };
    static_assert(0 == MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT % 8);
    static_assert(sizeof(TargetBufferFlags) == 4, "TargetBufferFlags has unexpected size.");
    static_assert(sizeof(VkFormat) == 4, "VkFormat has unexpected size.");
    static_assert(sizeof(RenderPassKey) == 56, "RenderPassKey has unexpected size.");
    using RenderPassHash = utils::hash::MurmurHashFn<RenderPassKey>;
    struct RenderPassEq {
        bool operator()(const RenderPassKey& k1, const RenderPassKey& k2) const;
    };

    static_assert(sizeof(VkRenderPass) == 8, "VkRenderPass has unexpected size.");
    static_assert(sizeof(VkImageView) == 8, "VkImageView has unexpected size.");

    explicit VulkanFboCache(VkDevice device);
    ~VulkanFboCache();

    // Retrieves or creates a VkRenderPass handle.
    fvkmemory::resource_ptr<VulkanRenderPass> getRenderPass(
        RenderPassKey const& config, fvkmemory::ResourceManager* resManager) noexcept;

    // Evicts old unused Vulkan objects. Call this once per frame.
    void gc() noexcept;

    // Frees all Vulkan objects. Call this during shutdown before the device is destroyed.
    void terminate() noexcept;

private:
    VkDevice mDevice;
    using RenderPassMap = tsl::robin_map<RenderPassKey, RenderPassVal, RenderPassHash, RenderPassEq>;
    RenderPassMap mRenderPassCache;
    tsl::robin_map<VkRenderPass, uint32_t> mRenderPassRefCount;
    uint32_t mCurrentTime = 0;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANFBOCACHE_H
