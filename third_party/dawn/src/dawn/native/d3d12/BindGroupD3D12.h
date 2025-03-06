// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_D3D12_BINDGROUPD3D12_H_
#define SRC_DAWN_NATIVE_D3D12_BINDGROUPD3D12_H_

#include "dawn/common/MutexProtected.h"
#include "dawn/common/PlacementAllocated.h"
#include "dawn/common/ityp_span.h"
#include "dawn/common/ityp_stack_vec.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/d3d12/CPUDescriptorHeapAllocationD3D12.h"
#include "dawn/native/d3d12/GPUDescriptorHeapAllocationD3D12.h"

namespace dawn::native::d3d12 {

class Device;
class SamplerHeapCacheEntry;
class ShaderVisibleDescriptorAllocator;

class BindGroup final : public BindGroupBase, public PlacementAllocated {
  public:
    static ResultOrError<Ref<BindGroup>> Create(Device* device,
                                                const BindGroupDescriptor* descriptor);

    BindGroup(Device* device,
              const BindGroupDescriptor* descriptor,
              uint32_t viewSizeIncrement,
              const CPUDescriptorHeapAllocation& viewAllocation);

    // Returns true if the BindGroup was successfully populated.
    bool PopulateViews(MutexProtected<ShaderVisibleDescriptorAllocator>& viewAllocator);
    bool PopulateSamplers(MutexProtected<ShaderVisibleDescriptorAllocator>& samplerAllocator);

    D3D12_GPU_DESCRIPTOR_HANDLE GetBaseViewDescriptor() const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetBaseSamplerDescriptor() const;

    void SetSamplerAllocationEntry(Ref<SamplerHeapCacheEntry> entry);

    using DynamicStorageBufferLengths = ityp::stack_vec<uint32_t, uint32_t, 4u>;
    const DynamicStorageBufferLengths& GetDynamicStorageBufferLengths() const;

  private:
    ~BindGroup() override;

    void DestroyImpl() override;

    Ref<SamplerHeapCacheEntry> mSamplerAllocationEntry;

    GPUDescriptorHeapAllocation mGPUViewAllocation;
    CPUDescriptorHeapAllocation mCPUViewAllocation;

    DynamicStorageBufferLengths mDynamicStorageBufferLengths;
};
}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_BINDGROUPD3D12_H_
