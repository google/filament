// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NATIVE_VULKAN_FRAMEBUFFERCACHE_H_
#define SRC_DAWN_NATIVE_VULKAN_FRAMEBUFFERCACHE_H_

#include <array>
#include <list>
#include <mutex>
#include <utility>

#include "absl/container/flat_hash_map.h"
#include "dawn/common/Constants.h"
#include "dawn/common/LRUCache.h"
#include "dawn/common/WeakRef.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/Error.h"
#include "dawn/native/dawn_platform.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::vulkan {

class Device;
class TextureView;

struct FramebufferCacheTextureView {
    // TODO(crbug.com/436274255): Uses a unique ID assigned at view creation rather than a WeakRef
    // due to the overhead of WeakRef promotion. Could switch back to using WeakRef if that was
    // optimized.
    uint64_t textureViewId;
    uint32_t depthSlice;
};

// A key to query the FramebufferCache
struct FramebufferCacheQuery {
    // Use these helpers to build the query, they make sure all relevant data is initialized and
    // masks set.
    void SetRenderPass(uint64_t passId, uint32_t passWidth, uint32_t passHeight);
    MaybeError AddAttachment(TextureView* attachment,
                             VkClearValue clearValue = {},
                             uint32_t depthSlice = 0);

    // A unique ID for the render pass is used for cache lookup rather than the VkRenderPass
    // because Vulkan handles may be reused, making them unreliable as cache keys.
    uint64_t renderPassId;
    uint32_t width;
    uint32_t height;

    std::array<FramebufferCacheTextureView, kMaxColorAttachments * 2 + 1> textureViews;

    // Attachments and clearValues are not used as part of the query hash, but are stored here
    // anyway because it's natural to build them up at the same time as the query criteria.
    std::array<VkImageView, kMaxColorAttachments * 2 + 1> attachments;
    std::array<VkClearValue, kMaxColorAttachments + 1> clearValues;

    uint32_t attachmentCount = 0;
};

// Implements the functors necessary for to use RenderPassCacheQueries as absl::flat_hash_map keys.
struct FramebufferCacheFuncs {
    size_t operator()(const FramebufferCacheQuery& query) const;
    bool operator()(const FramebufferCacheQuery& a, const FramebufferCacheQuery& b) const;
};

// A LRU Cache of VkFramebuffers so that we reduce the need to re-create framebuffers for every
// render pass. We always arrange the order of attachments in "color-depthstencil-resolve" order
// when creating render pass and framebuffer so that we can always make sure the order of
// attachments in the rendering pipeline matches the one of the framebuffer.
// All the operations on FramebufferCache are guaranteed to be thread-safe.
class FramebufferCache final
    : public LRUCache<FramebufferCacheQuery, VkFramebuffer, FramebufferCacheFuncs> {
    using Base = LRUCache<FramebufferCacheQuery, VkFramebuffer, FramebufferCacheFuncs>;

  public:
    static const size_t kDefaultCapacity = 32;
    explicit FramebufferCache(Device* device, size_t capacity = kDefaultCapacity);
    ~FramebufferCache() override;

    void EvictedFromCache(const VkFramebuffer& value) override;

  private:
    // We use a raw pointer to the device here because the cache is always owned by the device
    // and hence should always be valid.
    raw_ptr<Device> mDevice;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_FRAMEBUFFERCACHE_H_
