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

#include "src/dawn/native/d3d12/PipelineLayoutHandle.h"

#include "src/dawn/native/d3d12/DeviceD3D12.h"
#include "src/dawn/native/d3d12/ImmediatesLayoutD3D12.h"

namespace dawn::native::d3d12 {

// static
Ref<PipelineLayoutHandle> PipelineLayoutHandle::Create(Device* device,
                                                       ComPtr<ID3D12RootSignature> rootSignature,
                                                       ComPtr<ID3DBlob> rootSignatureBlob,
                                                       uint32_t immediatesParameterIndex,
                                                       const ImmediateMask& pipelineImmediateMask) {
    return AcquireRef(new PipelineLayoutHandle(device, std::move(rootSignature),
                                               std::move(rootSignatureBlob),
                                               immediatesParameterIndex, pipelineImmediateMask));
}

PipelineLayoutHandle::~PipelineLayoutHandle() {
    mDevice->ReferenceUntilUnused(std::move(mRootSignature));

    if (mDispatchIndirectCommandSignatureWithNumWorkgroups.Get()) {
        mDevice->ReferenceUntilUnused(mDispatchIndirectCommandSignatureWithNumWorkgroups);
    }
    if (mDrawIndirectCommandSignatureWithInstanceVertexOffsets.Get()) {
        mDevice->ReferenceUntilUnused(mDrawIndirectCommandSignatureWithInstanceVertexOffsets);
    }
    if (mDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets.Get()) {
        mDevice->ReferenceUntilUnused(
            mDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets);
    }
}

ID3D12CommandSignature*
PipelineLayoutHandle::GetDispatchIndirectCommandSignatureWithNumWorkgroups() {
    // mDispatchIndirectCommandSignatureWithNumWorkgroups won't be created until it is needed.
    if (mDispatchIndirectCommandSignatureWithNumWorkgroups.Get() != nullptr) {
        return mDispatchIndirectCommandSignatureWithNumWorkgroups.Get();
    }

    D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[2] = {};
    argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
    argumentDescs[0].Constant.RootParameterIndex = mImmediatesParameterIndex;
    argumentDescs[0].Constant.Num32BitValuesToSet = 3;
    argumentDescs[0].Constant.DestOffsetIn32BitValues =
        GetImmediateByteOffsetInPipeline(&ComputeImmediates::numWorkgroups,
                                         mPipelineImmediateMask) /
        kImmediateElementByteSize;

    // A command signature must contain exactly 1 Draw / Dispatch / DispatchMesh / DispatchRays
    // command. That command must come last.
    argumentDescs[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

    D3D12_COMMAND_SIGNATURE_DESC programDesc = {};
    programDesc.ByteStride = 6 * sizeof(uint32_t);
    programDesc.NumArgumentDescs = 2;
    programDesc.pArgumentDescs = argumentDescs;

    // The root signature must be specified if and only if the command signature changes one of
    // the root arguments.
    mDevice->GetD3D12Device()->CreateCommandSignature(
        &programDesc, GetRootSignature(),
        IID_PPV_ARGS(&mDispatchIndirectCommandSignatureWithNumWorkgroups));
    return mDispatchIndirectCommandSignatureWithNumWorkgroups.Get();
}

ID3D12CommandSignature*
PipelineLayoutHandle::GetDrawIndirectCommandSignatureWithInstanceVertexOffsets() {
    // mDrawIndirectCommandSignatureWithInstanceVertexOffsets won't be created until it is
    // needed.
    if (mDrawIndirectCommandSignatureWithInstanceVertexOffsets.Get() != nullptr) {
        return mDrawIndirectCommandSignatureWithInstanceVertexOffsets.Get();
    }

    // First vertex and first instance are set in immediate mask together and first vertex is
    // always just before instance index in the immediate mask.
    D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[2] = {};
    argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
    argumentDescs[0].Constant.RootParameterIndex = mImmediatesParameterIndex;
    argumentDescs[0].Constant.Num32BitValuesToSet = 2;
    argumentDescs[0].Constant.DestOffsetIn32BitValues =
        GetImmediateByteOffsetInPipeline(&RenderImmediates::firstIndexOffset,
                                         mPipelineImmediateMask) /
        kImmediateElementByteSize;

    // A command signature must contain exactly 1 Draw / Dispatch / DispatchMesh / DispatchRays
    // command. That command must come last.
    argumentDescs[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;

    D3D12_COMMAND_SIGNATURE_DESC programDesc = {};
    programDesc.ByteStride = 6 * sizeof(uint32_t);
    programDesc.NumArgumentDescs = 2;
    programDesc.pArgumentDescs = argumentDescs;

    // The root signature must be specified if and only if the command signature changes one of
    // the root arguments.
    mDevice->GetD3D12Device()->CreateCommandSignature(
        &programDesc, GetRootSignature(),
        IID_PPV_ARGS(&mDrawIndirectCommandSignatureWithInstanceVertexOffsets));
    return mDrawIndirectCommandSignatureWithInstanceVertexOffsets.Get();
}

ID3D12CommandSignature*
PipelineLayoutHandle::GetDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets() {
    // mDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets won't be created until it
    // is needed.
    if (mDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets.Get() != nullptr) {
        return mDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets.Get();
    }

    // First vertex and first instance are set in immediate mask together and first vertex is
    // always just before instance index in the immediate mask.
    D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[2] = {};
    argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
    argumentDescs[0].Constant.RootParameterIndex = mImmediatesParameterIndex;
    argumentDescs[0].Constant.Num32BitValuesToSet = 2;
    argumentDescs[0].Constant.DestOffsetIn32BitValues =
        GetImmediateByteOffsetInPipeline(&RenderImmediates::firstIndexOffset,
                                         mPipelineImmediateMask) /
        kImmediateElementByteSize;

    // A command signature must contain exactly 1 Draw / Dispatch / DispatchMesh / DispatchRays
    // command. That command must come last.
    argumentDescs[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

    D3D12_COMMAND_SIGNATURE_DESC programDesc = {};
    programDesc.ByteStride = 7 * sizeof(uint32_t);
    programDesc.NumArgumentDescs = 2;
    programDesc.pArgumentDescs = argumentDescs;

    // The root signature must be specified if and only if the command signature changes one of
    // the root arguments.
    mDevice->GetD3D12Device()->CreateCommandSignature(
        &programDesc, GetRootSignature(),
        IID_PPV_ARGS(&mDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets));
    return mDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets.Get();
}

}  // namespace dawn::native::d3d12
