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

#ifndef SRC_DAWN_NATIVE_VULKAN_PIPELINECACHEVK_H_
#define SRC_DAWN_NATIVE_VULKAN_PIPELINECACHEVK_H_

#include "dawn/native/ObjectBase.h"
#include "dawn/native/PipelineCache.h"
#include "partition_alloc/pointers/raw_ptr.h"

#include "dawn/common/vulkan_platform.h"

namespace dawn::native::vulkan {

class Device;

class PipelineCache final : public PipelineCacheBase {
  public:
    static Ref<PipelineCache> Create(Device* device, const CacheKey& key);

    // Creates a pipeline cache that is intended to be monolithic. The cache will only be serialized
    // and stored to BlobCache when StoreOnIdle() is called.
    static Ref<PipelineCache> CreateMonolithic(Device* device, const CacheKey& key);

    VkPipelineCache GetHandle() const;

  private:
    explicit PipelineCache(Device* device, const CacheKey& key, bool isMonolithicCache);
    ~PipelineCache() override;

    void Initialize();
    MaybeError SerializeToBlobImpl(Blob* blob) override;

    const raw_ptr<Device> mDevice;
    VkPipelineCache mHandle = VK_NULL_HANDLE;

    // Only a single thread should be inside SerializeToBlobImpl() at one time so this should never
    // be accessed concurrently on multiple threads.
    size_t mStoredDataSize = 0;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_PIPELINECACHEVK_H_
