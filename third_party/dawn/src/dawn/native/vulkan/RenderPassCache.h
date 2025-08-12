// Copyright 2018 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_VULKAN_RENDERPASSCACHE_H_
#define SRC_DAWN_NATIVE_VULKAN_RENDERPASSCACHE_H_

#include <array>
#include <atomic>
#include <bitset>
#include <mutex>

#include "absl/container/flat_hash_map.h"
#include "dawn/common/Constants.h"
#include "dawn/common/ityp_array.h"
#include "dawn/common/ityp_bitset.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/Error.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/dawn_platform.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::vulkan {

class Device;

// This is a key to query the RenderPassCache, it can be sparse meaning that only the
// information for bits set in colorMask or hasDepthStencil need to be provided and the rest can
// be uninintialized.
struct RenderPassCacheQuery {
    // Use these helpers to build the query, they make sure all relevant data is initialized and
    // masks set.
    void SetColor(ColorAttachmentIndex index,
                  wgpu::TextureFormat format,
                  wgpu::LoadOp loadOp,
                  wgpu::StoreOp storeOp,
                  bool hasResolveTarget);
    void SetDepthStencil(wgpu::TextureFormat format,
                         wgpu::LoadOp depthLoadOp,
                         wgpu::StoreOp depthStoreOp,
                         bool depthReadOnly,
                         wgpu::LoadOp stencilLoadOp,
                         wgpu::StoreOp stencilStoreOp,
                         bool stencilRendOnly);
    void SetSampleCount(uint32_t sampleCount);

    ColorAttachmentMask colorMask;
    ColorAttachmentMask resolveTargetMask;
    PerColorAttachment<wgpu::TextureFormat> colorFormats;
    PerColorAttachment<wgpu::LoadOp> colorLoadOp;
    PerColorAttachment<wgpu::StoreOp> colorStoreOp;
    ColorAttachmentMask expandResolveMask;

    bool hasDepthStencil = false;
    wgpu::TextureFormat depthStencilFormat;
    wgpu::LoadOp depthLoadOp;
    wgpu::StoreOp depthStoreOp;
    bool depthReadOnly;
    wgpu::LoadOp stencilLoadOp;
    wgpu::StoreOp stencilStoreOp;
    bool stencilReadOnly;

    uint32_t sampleCount;
};

// Caches VkRenderPasses so that we don't create duplicate ones for every RenderPipeline or
// render pass. We always arrange the order of attachments in "color-depthstencil-resolve" order
// when creating render pass and framebuffer so that we can always make sure the order of
// attachments in the rendering pipeline matches the one of the framebuffer.
// All the operations on RenderPassCache are guaranteed to be thread-safe.
// TODO(cwallez@chromium.org): Make it an LRU cache somehow?
class RenderPassCache {
  public:
    explicit RenderPassCache(Device* device);
    ~RenderPassCache();

    struct RenderPassInfo {
        VkRenderPass renderPass = VK_NULL_HANDLE;
        uint32_t mainSubpass = 0;
        uint64_t uniqueId;
    };

    ResultOrError<RenderPassInfo> GetRenderPass(const RenderPassCacheQuery& query);

  private:
    // Does the actual VkRenderPass creation on a cache miss.
    ResultOrError<RenderPassInfo> CreateRenderPassForQuery(const RenderPassCacheQuery& query);

    // Implements the functors necessary for to use RenderPassCacheQueries as absl::flat_hash_map
    // keys.
    struct CacheFuncs {
        size_t operator()(const RenderPassCacheQuery& query) const;
        bool operator()(const RenderPassCacheQuery& a, const RenderPassCacheQuery& b) const;
    };
    using Cache = absl::flat_hash_map<RenderPassCacheQuery, RenderPassInfo, CacheFuncs, CacheFuncs>;

    raw_ptr<Device> mDevice = nullptr;

    std::atomic<uint64_t> nextRenderPassId = 1;

    std::mutex mMutex;
    Cache mCache;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_RENDERPASSCACHE_H_
