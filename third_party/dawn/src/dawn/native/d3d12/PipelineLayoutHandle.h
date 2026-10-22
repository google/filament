// Copyright 2026 The Dawn & Tint Authors
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

// Defines PipelineLayoutHandle, which bundles together a root signature specialized for a specific
// pipeline (with a tight bound on the number of internal root constants), its serialized blob, the
// reserved root parameter indices, and the lazily-created indirect command signatures that depend
// on that root signature.

#ifndef SRC_DAWN_NATIVE_D3D12_PIPELINELAYOUTHANDLE_H_
#define SRC_DAWN_NATIVE_D3D12_PIPELINELAYOUTHANDLE_H_

#include <cstdint>
#include <utility>

#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/common/Ref.h"
#include "src/dawn/common/RefCounted.h"
#include "src/dawn/common/ityp_bitset.h"
#include "src/dawn/native/IntegerTypes.h"
#include "src/dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

class Device;

// The pipeline layouts are specialized per-pipeline to use a tight bound on the number of root
// constants used, so store all related root signatures and command signatures together.
class PipelineLayoutHandle final : public RefCounted {
  public:
    static Ref<PipelineLayoutHandle> Create(Device* device,
                                            ComPtr<ID3D12RootSignature> rootSignature,
                                            ComPtr<ID3DBlob> rootSignatureBlob,
                                            uint32_t immediatesParameterIndex,
                                            const ImmediateMask& pipelineImmediateMask);

    ID3D12RootSignature* GetRootSignature() const { return mRootSignature.Get(); }
    ID3DBlob* GetRootSignatureBlob() const { return mRootSignatureBlob.Get(); }
    uint32_t GetImmediatesParameterIndex() const { return mImmediatesParameterIndex; }

    // Lazy-created later.
    ID3D12CommandSignature* GetDispatchIndirectCommandSignatureWithNumWorkgroups();
    ID3D12CommandSignature* GetDrawIndirectCommandSignatureWithInstanceVertexOffsets();
    ID3D12CommandSignature* GetDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets();

  private:
    PipelineLayoutHandle(Device* device,
                         ComPtr<ID3D12RootSignature> rootSignature,
                         ComPtr<ID3DBlob> rootSignatureBlob,
                         uint32_t immediatesParameterIndex,
                         const ImmediateMask& pipelineImmediateMask)
        : mDevice(device),
          mRootSignature(std::move(rootSignature)),
          mRootSignatureBlob(std::move(rootSignatureBlob)),
          mImmediatesParameterIndex(immediatesParameterIndex),
          mPipelineImmediateMask(pipelineImmediateMask) {}

    ~PipelineLayoutHandle() override;

    // A handle is transitively owned by the pipelines and pipeline layout that reference it, all of
    // which are owned by the Device and destroyed before it. The handle therefore never outlives
    // the Device, so dereferencing mDevice in the destructor is safe.
    raw_ptr<Device> mDevice;
    ComPtr<ID3D12RootSignature> mRootSignature;
    ComPtr<ID3DBlob> mRootSignatureBlob;
    uint32_t mImmediatesParameterIndex;
    ComPtr<ID3D12CommandSignature> mDispatchIndirectCommandSignatureWithNumWorkgroups;
    ComPtr<ID3D12CommandSignature> mDrawIndirectCommandSignatureWithInstanceVertexOffsets;
    ComPtr<ID3D12CommandSignature> mDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets;
    ImmediateMask mPipelineImmediateMask = ImmediateMask(0);
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_PIPELINELAYOUTHANDLE_H_
