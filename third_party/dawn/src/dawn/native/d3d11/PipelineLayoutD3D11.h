// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_D3D11_PIPELINELAYOUTD3D11_H_
#define SRC_DAWN_NATIVE_D3D11_PIPELINELAYOUTD3D11_H_

#include "dawn/native/PipelineLayout.h"

#include "dawn/common/ityp_array.h"
#include "dawn/common/ityp_vector.h"
#include "dawn/native/BindingInfo.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d11 {

class Device;

// For D3D11, uniform buffers, samplers, sampled textures, and storage buffers are bind to
// different kind of slots. The number of slots for each type is limited by the D3D11 spec.
// So we need to pack the bindings by type into the slots tightly. UAV slots are a little
// different. They are assigned to storage buffers and textures decreasingly from the end,
// as color attachments also use them increasingly from the begin.
// And D3D11 uses SM 5.0 which doesn't support spaces(binding groups). so we need to flatten
// the binding groups into a single array.
class PipelineLayout final : public PipelineLayoutBase {
  public:
    // constant buffer slot reserved for index offsets and num workgroups.
    static constexpr uint32_t kReservedConstantBufferSlot =
        D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1;
    static constexpr uint32_t kFirstIndexOffsetConstantBufferSlot = kReservedConstantBufferSlot;
    static constexpr uint32_t kNumWorkgroupsConstantBufferSlot = kReservedConstantBufferSlot;

    // The reserved groups and bindings are for internal use only. They must not conflict with
    // external clients.
    static constexpr uint32_t kReservedConstantsBindGroupIndex = kMaxBindGroups;
    static constexpr uint32_t kFirstIndexOffsetBindingNumber = 0u;

    static ResultOrError<Ref<PipelineLayout>> Create(
        Device* device,
        const UnpackedPtr<PipelineLayoutDescriptor>& descriptor);

    using BindingIndexInfo = PerBindGroup<ityp::vector<BindingIndex, uint32_t>>;
    const BindingIndexInfo& GetBindingIndexInfo() const;

    uint32_t GetUnusedUAVBindingCount() const { return mUnusedUAVBindingCount; }
    uint32_t GetTotalUAVBindingCount() const { return mTotalUAVBindingCount; }
    uint32_t GetPLSSlotCount() const {
        return static_cast<uint32_t>(GetStorageAttachmentSlots().size());
    }

    // Get the bind groups that use one or more UAV slots.
    const BindGroupMask& GetUAVBindGroupLayoutsMask() const;

  private:
    using PipelineLayoutBase::PipelineLayoutBase;

    ~PipelineLayout() override = default;

    MaybeError Initialize(Device* device);

    BindingIndexInfo mIndexInfo;

    uint32_t mUnusedUAVBindingCount = 0u;
    uint32_t mTotalUAVBindingCount = 0u;
    BindGroupMask mUAVBindGroups = 0;
};

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_PIPELINELAYOUTD3D11_H_
