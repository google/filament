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

#ifndef SRC_DAWN_NATIVE_D3D12_BINDGROUPLAYOUTD3D12_H_
#define SRC_DAWN_NATIVE_D3D12_BINDGROUPLAYOUTD3D12_H_

#include <vector>

#include "dawn/common/MutexProtected.h"
#include "dawn/common/SlabAllocator.h"
#include "dawn/common/ityp_stack_vec.h"
#include "dawn/native/BindGroupLayoutInternal.h"
#include "dawn/native/d3d12/BindGroupD3D12.h"
#include "dawn/native/d3d12/d3d12_platform.h"
#include "partition_alloc/pointers/raw_ptr_exclusion.h"

namespace dawn::native::d3d12 {

class CPUDescriptorHeapAllocation;
class Device;
class StagingDescriptorAllocator;

// A purposefully invalid register space.
//
// We use the bind group index as the register space, but don't know the bind group index until
// pipeline layout creation time. This value should be replaced in PipelineLayoutD3D12.
static constexpr uint32_t kRegisterSpacePlaceholder =
    D3D12_DRIVER_RESERVED_REGISTER_SPACE_VALUES_START;

class BindGroupLayout final : public BindGroupLayoutInternalBase {
  public:
    static Ref<BindGroupLayout> Create(Device* device, const BindGroupLayoutDescriptor* descriptor);

    ResultOrError<Ref<BindGroup>> AllocateBindGroup(Device* device,
                                                    const BindGroupDescriptor* descriptor);
    void DeallocateBindGroup(BindGroup* bindGroup);
    void DeallocateDescriptor(CPUDescriptorHeapAllocation* viewAllocation);
    void ReduceMemoryUsage() override;

    // The offset (in descriptor count) into the corresponding descriptor heap. Not valid for
    // dynamic binding indexes.
    ityp::span<BindingIndex, const uint32_t> GetDescriptorHeapOffsets() const;

    // The D3D shader register that the Dawn binding index is mapped to by this bind group
    // layout.
    uint32_t GetShaderRegister(BindingIndex bindingIndex) const;

    // Counts of descriptors in the descriptor tables.
    uint32_t GetCbvUavSrvDescriptorCount() const;
    uint32_t GetSamplerDescriptorCount() const;

    uint32_t GetViewSizeIncrement() const;

    const std::vector<D3D12_DESCRIPTOR_RANGE1>& GetCbvUavSrvDescriptorRanges() const;
    const std::vector<D3D12_DESCRIPTOR_RANGE1>& GetSamplerDescriptorRanges() const;
    const std::vector<D3D12_STATIC_SAMPLER_DESC>& GetStaticSamplers() const;

  private:
    BindGroupLayout(Device* device, const BindGroupLayoutDescriptor* descriptor);
    ~BindGroupLayout() override = default;

    // Contains the offset into the descriptor heap for the given resource view. Samplers and
    // non-samplers are stored in separate descriptor heaps, so the offsets should be unique
    // within each group and tightly packed.
    //
    // Dynamic resources are not used here since their descriptors are placed directly in root
    // parameters.
    ityp::stack_vec<BindingIndex, uint32_t, kMaxOptimalBindingsPerGroup> mDescriptorHeapOffsets;

    // Contains the shader register this binding is mapped to.
    ityp::stack_vec<BindingIndex, uint32_t, kMaxOptimalBindingsPerGroup> mShaderRegisters;

    uint32_t mCbvUavSrvDescriptorCount;
    uint32_t mSamplerDescriptorCount;
    uint32_t mViewSizeIncrement;

    std::vector<D3D12_DESCRIPTOR_RANGE1> mCbvUavSrvDescriptorRanges;
    std::vector<D3D12_DESCRIPTOR_RANGE1> mSamplerDescriptorRanges;

    std::vector<D3D12_STATIC_SAMPLER_DESC> mStaticSamplers;

    MutexProtected<SlabAllocator<BindGroup>> mBindGroupAllocator;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_BINDGROUPLAYOUTD3D12_H_
