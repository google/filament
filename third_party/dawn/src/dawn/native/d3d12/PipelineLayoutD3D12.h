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

#ifndef SRC_DAWN_NATIVE_D3D12_PIPELINELAYOUTD3D12_H_
#define SRC_DAWN_NATIVE_D3D12_PIPELINELAYOUTD3D12_H_

#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/ityp_array.h"
#include "dawn/common/ityp_vector.h"
#include "dawn/native/BindingInfo.h"
#include "dawn/native/PipelineLayout.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

class Device;

class PipelineLayout final : public PipelineLayoutBase {
  public:
    static ResultOrError<Ref<PipelineLayout>> Create(
        Device* device,
        const UnpackedPtr<PipelineLayoutDescriptor>& descriptor);

    uint32_t GetResourceTableRootParameterIndex() const;
    uint32_t GetBaseResourceTableRegisterSpace() const;

    uint32_t GetCbvUavSrvRootParameterIndex(BindGroupIndex group) const;
    uint32_t GetSamplerRootParameterIndex(BindGroupIndex group) const;

    // Returns the index of the root parameter reserved for a dynamic uniform buffer binding
    uint32_t GetDynamicUniformRootParameterIndex(BindGroupIndex group,
                                                 BindingIndex bindingIndex) const;

    uint32_t GetFirstIndexOffsetRegisterSpace() const;
    uint32_t GetFirstIndexOffsetShaderRegister() const;
    uint32_t GetFirstIndexOffsetParameterIndex() const;

    uint32_t GetNumWorkgroupsRegisterSpace() const;
    uint32_t GetNumWorkgroupsShaderRegister() const;
    uint32_t GetNumWorkgroupsParameterIndex() const;

    uint32_t GetDynamicStorageBufferLengthsRegisterSpace() const;
    uint32_t GetDynamicStorageBufferLengthsShaderRegister() const;
    uint32_t GetDynamicStorageBufferLengthsParameterIndex() const;

    uint32_t GetDynamicStorageBufferOffsetsRegisterSpace() const;
    uint32_t GetDynamicStorageBufferOffsetsShaderRegister() const;
    uint32_t GetDynamicStorageBufferOffsetsParameterIndex() const;

    uint32_t GetImmediatesRegisterSpace() const;
    uint32_t GetImmediatesShaderRegister() const;
    uint32_t GetImmediatesParameterIndex() const;

    ID3D12RootSignature* GetRootSignature() const;

    ID3DBlob* GetRootSignatureBlob() const;

    ID3D12CommandSignature* GetDispatchIndirectCommandSignatureWithNumWorkgroups();

    ID3D12CommandSignature* GetDrawIndirectCommandSignatureWithInstanceVertexOffsets();

    ID3D12CommandSignature* GetDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets();

    struct BindGroupDynamicStorageBufferInfo {
        // First register offset for a bind group's dynamic storage buffer lengths or offsets.
        // This is the index into the array of root constants where this bind group's
        // lengths or offsets start.
        uint32_t firstRegisterOffset;

        struct BindingAndRegisterOffset {
            BindingNumber binding;
            uint32_t registerOffset;
        };
        // Associative list of (BindingNumber,registerOffset) pairs, which is passed into
        // the shader to map the BindingPoint(thisGroup, BindingNumber) to the registerOffset
        // into the root constant array which holds the dynamic storage buffer lengths and offsets.
        std::vector<BindingAndRegisterOffset> bindingAndRegisterOffsets;
    };

    // Flat map from bind group index to the list of (BindingNumber,Register) pairs.
    using DynamicStorageBufferInfo = PerBindGroup<BindGroupDynamicStorageBufferInfo>;

    const DynamicStorageBufferInfo& GetDynamicStorageBufferInfo() const;

  private:
    ~PipelineLayout() override = default;
    using PipelineLayoutBase::PipelineLayoutBase;
    MaybeError Initialize();
    void DestroyImpl(DestroyReason reason) override;

    PerBindGroup<uint32_t> mCbvUavSrvRootParameterIndices;
    PerBindGroup<uint32_t> mSamplerRootParameterIndices;
    PerBindGroup<ityp::vector<BindingIndex, uint32_t>> mDynamicUniformRootParameterIndices;
    DynamicStorageBufferInfo mDynamicStorageBufferInfo;
    uint32_t mResourceTableRootParameterIndex;
    uint32_t mFirstIndexOffsetParameterIndex;
    uint32_t mNumWorkgroupsParameterIndex;
    uint32_t mDynamicStorageBufferLengthsParameterIndex;
    uint32_t mDynamicStorageBufferOffsetsParameterIndex;
    uint32_t mImmediatesParameterIndex;
    ComPtr<ID3D12RootSignature> mRootSignature;
    // Store the root signature blob to put in pipeline cachekey
    ComPtr<ID3DBlob> mRootSignatureBlob;
    ComPtr<ID3D12CommandSignature> mDispatchIndirectCommandSignatureWithNumWorkgroups;
    ComPtr<ID3D12CommandSignature> mDrawIndirectCommandSignatureWithInstanceVertexOffsets;
    ComPtr<ID3D12CommandSignature> mDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_PIPELINELAYOUTD3D12_H_
