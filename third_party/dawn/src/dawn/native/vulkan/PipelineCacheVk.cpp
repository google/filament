// Copyright 2022 The Dawn & Tint Authors
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

#include "dawn/native/vulkan/PipelineCacheVk.h"

#include <memory>

#include "dawn/native/Device.h"
#include "dawn/native/Error.h"
#include "dawn/native/Toggles.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

// static
Ref<PipelineCache> PipelineCache::Create(Device* device, const CacheKey& key) {
    Ref<PipelineCache> cache =
        AcquireRef(new PipelineCache(device, key, /*isMonolithicCache=*/false));
    cache->Initialize();
    return cache;
}

// static
Ref<PipelineCache> PipelineCache::CreateMonolithic(Device* device, const CacheKey& key) {
    Ref<PipelineCache> cache =
        AcquireRef(new PipelineCache(device, key, /*isMonolithicCache=*/true));
    cache->Initialize();
    return cache;
}

PipelineCache::PipelineCache(Device* device, const CacheKey& key, bool isMonolithicCache)
    : PipelineCacheBase(device->GetBlobCache(), key, isMonolithicCache), mDevice(device) {}

PipelineCache::~PipelineCache() {
    if (mHandle == VK_NULL_HANDLE) {
        return;
    }
    mDevice->fn.DestroyPipelineCache(mDevice->GetVkDevice(), mHandle, nullptr);
    mHandle = VK_NULL_HANDLE;
}

VkPipelineCache PipelineCache::GetHandle() const {
    return mHandle;
}

MaybeError PipelineCache::SerializeToBlobImpl(Blob* blob) {
    if (mHandle == VK_NULL_HANDLE) {
        // Pipeline cache isn't created successfully
        return {};
    }

    size_t bufferSize;
    DAWN_TRY(CheckVkSuccess(
        mDevice->fn.GetPipelineCacheData(mDevice->GetVkDevice(), mHandle, &bufferSize, nullptr),
        "GetPipelineCacheData"));

    if (bufferSize == 0 || bufferSize == mStoredDataSize) {
        // If current VkPipelineCache data size is same as `mStoredDataSize` assume nothing has
        // changed vs what is stored in the BlobCache.
        return {};
    }
    *blob = CreateBlob(bufferSize);
    DAWN_TRY(CheckVkSuccess(mDevice->fn.GetPipelineCacheData(mDevice->GetVkDevice(), mHandle,
                                                             &bufferSize, blob->Data()),
                            "GetPipelineCacheData"));
    mStoredDataSize = bufferSize;

    return {};
}

void PipelineCache::Initialize() {
    Blob blob = PipelineCacheBase::Initialize();

    VkPipelineCacheCreateInfo createInfo;
    createInfo.flags = 0;
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.initialDataSize = blob.Size();
    createInfo.pInitialData = blob.Data();

    mHandle = VK_NULL_HANDLE;

    // Attempts to create the pipeline cache but does not bubble the error, instead only logging.
    // This should be fine because the handle will be left as null and pipeline creation should
    // continue as if there was no cache.
    MaybeError maybeError = CheckVkSuccess(
        mDevice->fn.CreatePipelineCache(mDevice->GetVkDevice(), &createInfo, nullptr, &*mHandle),
        "CreatePipelineCache");
    if (maybeError.IsError()) {
        std::unique_ptr<ErrorData> error = maybeError.AcquireError();
        mDevice->EmitLog(wgpu::LoggingType::Info, error->GetFormattedMessage().c_str());
        return;
    }

    mStoredDataSize = blob.Size();
}

}  // namespace dawn::native::vulkan
