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

#include <utils/Hash.h>

#include <backend/TargetBufferInfo.h>

#include <tsl/robin_map.h>

namespace filament::backend {

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
        // For each target, we need to know three image layouts: the layout BEFORE the pass, the
        // layout DURING the pass, and the layout AFTER the pass. Here are the rules:
        // - For depth, we explicitly specify all three layouts.
        // - Color targets have their initial image layout specified with a bitmask.
        // - For each color target, the pre-existing layout is either UNDEFINED (0) or GENERAL (1).
        // - The render pass and final images layout for color buffers is always
        //   VulkanLayout::COLOR_ATTACHMENT.
        uint8_t initialColorLayoutMask;

        // Note that if VulkanLayout grows beyond 16, we'd need to up this.
        VulkanLayout initialDepthLayout : 8;
        uint8_t padding0;
        uint8_t padding1;

        VkFormat colorFormat[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT]; // 32 bytes
        VkFormat depthFormat; // 4 bytes
        TargetBufferFlags clear; // 4 bytes
        TargetBufferFlags discardStart; // 4 bytes
        TargetBufferFlags discardEnd; // 4 bytes
        uint8_t samples; // 1 byte
        uint8_t needsResolveMask; // 1 byte
        uint8_t subpassMask; // 1 byte
        uint8_t viewCount; // 1 byte
    };
    struct RenderPassVal {
        VkRenderPass handle;
        uint32_t timestamp;
    };
    static_assert(0 == MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT % 8);
    static_assert(sizeof(RenderPassKey::initialColorLayoutMask) == MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT / 8);
    static_assert(sizeof(TargetBufferFlags) == 4, "TargetBufferFlags has unexpected size.");
    static_assert(sizeof(VkFormat) == 4, "VkFormat has unexpected size.");
    static_assert(sizeof(RenderPassKey) == 56, "RenderPassKey has unexpected size.");
    using RenderPassHash = utils::hash::MurmurHashFn<RenderPassKey>;
    struct RenderPassEq {
        bool operator()(const RenderPassKey& k1, const RenderPassKey& k2) const;
    };

    // FboKey is a small POD representing the immutable state that we wish to configure
    // in VkFramebuffer. It is hashed and used as a lookup key. There are several attachments, but
    // rather than storing a count, we simply zero out the unused slots.
    struct alignas(8) FboKey {
        VkRenderPass renderPass; // 8 bytes
        uint16_t width; // 2 bytes
        uint16_t height; // 2 bytes
        uint16_t layers; // 2 bytes
        uint16_t samples; // 2 bytes
        VkImageView color[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT]; // 64 bytes
        VkImageView resolve[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT]; // 64 bytes
        VkImageView depth; // 8 bytes
    };
    struct FboVal {
        VkFramebuffer handle;
        uint32_t timestamp;
    };
    static_assert(sizeof(VkRenderPass) == 8, "VkRenderPass has unexpected size.");
    static_assert(sizeof(VkImageView) == 8, "VkImageView has unexpected size.");
    static_assert(sizeof(FboKey) == 152, "FboKey has unexpected size.");
    using FboKeyHashFn = utils::hash::MurmurHashFn<FboKey>;
    struct FboKeyEqualFn {
        bool operator()(const FboKey& k1, const FboKey& k2) const;
    };

    explicit VulkanFboCache(VkDevice device);
    ~VulkanFboCache();

    // Retrieves or creates a VkFramebuffer handle.
    VkFramebuffer getFramebuffer(FboKey config) noexcept;

    // Retrieves or creates a VkRenderPass handle.
    VkRenderPass getRenderPass(RenderPassKey config) noexcept;

    // Evicts old unused Vulkan objects. Call this once per frame.
    void gc() noexcept;

    // Frees all Vulkan objects. Call this during shutdown before the device is destroyed.
    void reset() noexcept;

private:
    VkDevice mDevice;
    tsl::robin_map<FboKey, FboVal, FboKeyHashFn, FboKeyEqualFn> mFramebufferCache;
    tsl::robin_map<RenderPassKey, RenderPassVal, RenderPassHash, RenderPassEq> mRenderPassCache;
    tsl::robin_map<VkRenderPass, uint32_t> mRenderPassRefCount;
    uint32_t mCurrentTime = 0;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANFBOCACHE_H
