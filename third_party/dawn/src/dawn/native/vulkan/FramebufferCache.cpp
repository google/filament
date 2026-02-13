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

#include "dawn/native/vulkan/FramebufferCache.h"

#include "absl/container/inlined_vector.h"
#include "dawn/common/HashUtils.h"
#include "dawn/common/Range.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/TextureVk.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

// FramebufferCacheQuery

void FramebufferCacheQuery::SetRenderPass(uint64_t passId,
                                          uint32_t passWidth,
                                          uint32_t passHeight) {
    renderPassId = passId;
    width = passWidth;
    height = passHeight;
}

MaybeError FramebufferCacheQuery::AddAttachment(TextureView* view,
                                                VkClearValue clearValue,
                                                uint32_t depthSlice) {
    textureViews[attachmentCount].textureViewId = view->GetTextureViewId();
    textureViews[attachmentCount].depthSlice = depthSlice;

    if (view->GetDimension() == wgpu::TextureViewDimension::e3D) {
        VkImageView handleFor2DViewOn3D;
        DAWN_TRY_ASSIGN(handleFor2DViewOn3D, view->GetOrCreate2DViewOn3D(depthSlice));
        attachments[attachmentCount] = handleFor2DViewOn3D;
    } else {
        attachments[attachmentCount] = view->GetHandle();
    }

    clearValues[attachmentCount] = clearValue;

    attachmentCount++;

    return {};
}

// FramebufferCache

FramebufferCache::FramebufferCache(Device* device, size_t capacity)
    : FramebufferCache::Base(capacity), mDevice(device) {}

FramebufferCache::~FramebufferCache() {
    mCache.Use([this](auto cache) {
        for (auto [_, framebuffer] : cache->list) {
            mDevice->fn.DestroyFramebuffer(mDevice->GetVkDevice(), framebuffer, nullptr);
        }
    });
}

void FramebufferCache::EvictedFromCache(const VkFramebuffer& framebuffer) {
    mDevice->GetFencedDeleter()->DeleteWhenUnused(framebuffer);
}

size_t FramebufferCacheFuncs::operator()(const FramebufferCacheQuery& query) const {
    size_t hash = Hash(query.renderPassId);

    HashCombine(&hash, query.width, query.height, query.attachmentCount);

    for (uint32_t i = 0; i < query.attachmentCount; ++i) {
        HashCombine(&hash, query.textureViews[i].textureViewId, query.textureViews[i].depthSlice);
    }

    return hash;
}

bool FramebufferCacheFuncs::operator()(const FramebufferCacheQuery& a,
                                       const FramebufferCacheQuery& b) const {
    if (a.renderPassId != b.renderPassId || a.width != b.width || a.height != b.height ||
        a.attachmentCount != b.attachmentCount) {
        return false;
    }

    for (uint32_t i = 0; i < a.attachmentCount; ++i) {
        if (a.textureViews[i].textureViewId != b.textureViews[i].textureViewId ||
            a.textureViews[i].depthSlice != b.textureViews[i].depthSlice) {
            return false;
        }
    }

    return true;
}

}  // namespace dawn::native::vulkan
