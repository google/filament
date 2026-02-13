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

#include "dawn/native/d3d12/SamplerHeapCacheD3D12.h"

#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/HashUtils.h"
#include "dawn/native/Queue.h"
#include "dawn/native/d3d12/BindGroupD3D12.h"
#include "dawn/native/d3d12/BindGroupLayoutD3D12.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/Forward.h"
#include "dawn/native/d3d12/SamplerD3D12.h"
#include "dawn/native/d3d12/ShaderVisibleDescriptorAllocatorD3D12.h"
#include "dawn/native/d3d12/StagingDescriptorAllocatorD3D12.h"

namespace dawn::native::d3d12 {

SamplerHeapCacheEntry::SamplerHeapCacheEntry(std::vector<Sampler*> samplers)
    : mSamplers(std::move(samplers)) {}

SamplerHeapCacheEntry::SamplerHeapCacheEntry(SamplerHeapCache* cache,
                                             std::vector<Sampler*> samplers,
                                             CPUDescriptorHeapAllocation allocation)
    : mCPUAllocation(std::move(allocation)),
      mSamplers(std::move(samplers)),
      mCache(cache) {
    DAWN_ASSERT(mCache != nullptr);
    DAWN_ASSERT(mCPUAllocation.IsValid());
    DAWN_ASSERT(!mSamplers.empty());
}

std::vector<Sampler*>&& SamplerHeapCacheEntry::AcquireSamplers() {
    // This function should only be called when SamplerHeapCacheEntry is created for blueprint.
    DAWN_ASSERT(!mCPUAllocation.IsValid());
    return std::move(mSamplers);
}

SamplerHeapCacheEntry::~SamplerHeapCacheEntry() {
    // If this is a blueprint then the CPU allocation cannot exist and has no entry to remove.
    if (mCPUAllocation.IsValid()) {
        mCache->RemoveCacheEntry(this);
        DAWN_ASSERT(!mSamplers.empty());
        auto* allocator = mCache->GetDevice()->GetSamplerStagingDescriptorAllocator(
            static_cast<uint32_t>(mSamplers.size()));
        DAWN_ASSERT(allocator != nullptr);
        (*allocator)->Deallocate(&mCPUAllocation);
    }

    DAWN_ASSERT(!mCPUAllocation.IsValid());
}

bool SamplerHeapCacheEntry::Populate(MutexProtected<ShaderVisibleDescriptorAllocator>& allocator) {
    if (allocator->IsAllocationStillValid(mGPUAllocation)) {
        return true;
    }

    DAWN_ASSERT(!mSamplers.empty());

    Device* device = allocator->GetDevice();

    // Attempt to allocate descriptors for the currently bound shader-visible heaps.
    // If either failed, return early to re-allocate and switch the heaps.
    const uint32_t descriptorCount = static_cast<uint32_t>(mSamplers.size());
    D3D12_CPU_DESCRIPTOR_HANDLE baseCPUDescriptor;
    if (!allocator->AllocateGPUDescriptors(descriptorCount,
                                           device->GetQueue()->GetPendingCommandSerial(),
                                           &baseCPUDescriptor, &mGPUAllocation)) {
        return false;
    }

    // CPU bindgroups are sparsely allocated across CPU heaps. Instead of doing
    // simple copies per bindgroup, a single non-simple copy could be issued.
    // TODO(dawn:155): Consider doing this optimization.
    device->GetD3D12Device()->CopyDescriptorsSimple(descriptorCount, baseCPUDescriptor,
                                                    mCPUAllocation.GetBaseDescriptor(),
                                                    D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

    return true;
}

D3D12_GPU_DESCRIPTOR_HANDLE SamplerHeapCacheEntry::GetBaseDescriptor() const {
    return mGPUAllocation.GetBaseDescriptor();
}

ResultOrError<Ref<SamplerHeapCacheEntry>> SamplerHeapCache::GetOrCreate(const BindGroup* group) {
    const BindGroupLayout* bgl = ToBackend(group->GetLayout());

    // If a previously created bindgroup used the same samplers, the backing sampler heap
    // allocation can be reused. The packed list of samplers acts as the key to lookup the
    // allocation in a cache.
    // TODO(dawn:155): Avoid re-allocating the vector each lookup.
    uint32_t samplerCount = bgl->GetSamplerDescriptorCount();
    std::vector<Sampler*> samplers;
    samplers.reserve(samplerCount);

    for (BindingIndex bindingIndex = bgl->GetDynamicBufferCount();
         bindingIndex < bgl->GetBindingCount(); ++bindingIndex) {
        const BindingInfo& bindingInfo = bgl->GetBindingInfo(bindingIndex);
        if (std::holds_alternative<SamplerBindingInfo>(bindingInfo.bindingLayout)) {
            samplers.push_back(ToBackend(group->GetBindingAsSampler(bindingIndex)));
        }
    }

    // Check the cache if there exists a sampler heap allocation that corresponds to the
    // samplers.
    SamplerHeapCacheEntry blueprint(std::move(samplers));
    auto iter = mCache.find(&blueprint);
    if (iter != mCache.end()) {
        return Ref<SamplerHeapCacheEntry>(*iter);
    }

    // Steal the sampler vector back from the blueprint to avoid creating a new copy for the
    // real entry below.
    samplers = std::move(blueprint.AcquireSamplers());

    uint32_t samplerSizeIncrement = 0;
    CPUDescriptorHeapAllocation allocation;
    Device* device = ToBackend(group->GetDevice());
    auto* samplerAllocator = device->GetSamplerStagingDescriptorAllocator(samplerCount);
    DAWN_TRY(samplerAllocator->Use([&](auto allocator) -> MaybeError {
        DAWN_TRY_ASSIGN(allocation, allocator->AllocateCPUDescriptors());
        samplerSizeIncrement = allocator->GetSizeIncrement();
        return {};
    }));

    ID3D12Device* d3d12Device = mDevice->GetD3D12Device();

    for (uint32_t i = 0; i < samplers.size(); ++i) {
        const auto& samplerDesc = samplers[i]->GetSamplerDescriptor();
        d3d12Device->CreateSampler(&samplerDesc, allocation.OffsetFrom(samplerSizeIncrement, i));
    }

    Ref<SamplerHeapCacheEntry> entry =
        AcquireRef(new SamplerHeapCacheEntry(this, std::move(samplers), std::move(allocation)));
    mCache.insert(entry.Get());
    return std::move(entry);
}

SamplerHeapCache::SamplerHeapCache(Device* device) : mDevice(device) {}

SamplerHeapCache::~SamplerHeapCache() {
    DAWN_ASSERT(mCache.empty());
}

Device* SamplerHeapCache::GetDevice() const {
    return mDevice;
}

void SamplerHeapCache::RemoveCacheEntry(SamplerHeapCacheEntry* entry) {
    DAWN_ASSERT(entry->GetRefCountForTesting() == 0);
    size_t removedCount = mCache.erase(entry);
    DAWN_ASSERT(removedCount == 1);
}

size_t SamplerHeapCacheEntry::HashFunc::operator()(const SamplerHeapCacheEntry* entry) const {
    size_t hash = 0;
    for (const Sampler* sampler : entry->mSamplers) {
        HashCombine(&hash, sampler);
    }
    return hash;
}

bool SamplerHeapCacheEntry::EqualityFunc::operator()(const SamplerHeapCacheEntry* a,
                                                     const SamplerHeapCacheEntry* b) const {
    return a->mSamplers == b->mSamplers;
}
}  // namespace dawn::native::d3d12
