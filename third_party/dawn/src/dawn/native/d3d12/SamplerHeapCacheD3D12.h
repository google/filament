// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_D3D12_SAMPLERHEAPCACHED3D12_H_
#define SRC_DAWN_NATIVE_D3D12_SAMPLERHEAPCACHED3D12_H_

#include <vector>

#include "absl/container/flat_hash_set.h"
#include "dawn/common/MutexProtected.h"
#include "dawn/common/Ref.h"
#include "dawn/common/RefCounted.h"
#include "dawn/native/BindingInfo.h"
#include "dawn/native/d3d12/CPUDescriptorHeapAllocationD3D12.h"
#include "dawn/native/d3d12/GPUDescriptorHeapAllocationD3D12.h"
#include "partition_alloc/pointers/raw_ptr_exclusion.h"
#include "partition_alloc/pointers/raw_ref.h"

// |SamplerHeapCacheEntry| maintains a cache of sampler descriptor heap allocations.
// Each entry represents one or more sampler descriptors that co-exist in a CPU and
// GPU descriptor heap. The CPU-side allocation is deallocated once the final reference
// has been released while the GPU-side allocation is deallocated when the GPU is finished.
//
// The BindGroupLayout hands out these entries upon constructing the bindgroup. If the entry is not
// invalid, it will allocate and initialize so it may be reused by another bindgroup.
//
// The cache is primary needed for the GPU sampler heap, which is much smaller than the view heap
// and switches incur expensive pipeline flushes.
namespace dawn::native::d3d12 {

class BindGroup;
class Device;
class Sampler;
class SamplerHeapCache;
class StagingDescriptorAllocator;
class ShaderVisibleDescriptorAllocator;

// Wraps sampler descriptor heap allocations in a cache.
class SamplerHeapCacheEntry : public RefCounted {
  public:
    explicit SamplerHeapCacheEntry(std::vector<Sampler*> samplers);
    SamplerHeapCacheEntry(SamplerHeapCache* cache,
                          std::vector<Sampler*> samplers,
                          CPUDescriptorHeapAllocation allocation);
    ~SamplerHeapCacheEntry() override;

    D3D12_GPU_DESCRIPTOR_HANDLE GetBaseDescriptor() const;

    std::vector<Sampler*>&& AcquireSamplers();

    bool Populate(MutexProtected<ShaderVisibleDescriptorAllocator>& allocator);

    // Functors necessary for the unordered_map<SamplerHeapCacheEntry*>-based cache.
    struct HashFunc {
        size_t operator()(const SamplerHeapCacheEntry* entry) const;
    };

    struct EqualityFunc {
        bool operator()(const SamplerHeapCacheEntry* a, const SamplerHeapCacheEntry* b) const;
    };

  private:
    CPUDescriptorHeapAllocation mCPUAllocation;
    GPUDescriptorHeapAllocation mGPUAllocation;

    // Storing raw pointer because the sampler object will be already hashed
    // by the device and will already be unique.
    std::vector<Sampler*> mSamplers;

    raw_ptr<SamplerHeapCache> mCache = nullptr;
};

// Cache descriptor heap allocations so that we don't create duplicate ones for every
// BindGroup.
class SamplerHeapCache {
  public:
    explicit SamplerHeapCache(Device* device);
    ~SamplerHeapCache();

    ResultOrError<Ref<SamplerHeapCacheEntry>> GetOrCreate(const BindGroup* group);

    void RemoveCacheEntry(SamplerHeapCacheEntry* entry);

    Device* GetDevice() const;

  private:
    raw_ptr<Device> mDevice;

    using Cache = absl::flat_hash_set<SamplerHeapCacheEntry*,
                                      SamplerHeapCacheEntry::HashFunc,
                                      SamplerHeapCacheEntry::EqualityFunc>;

    Cache mCache;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_SAMPLERHEAPCACHED3D12_H_
