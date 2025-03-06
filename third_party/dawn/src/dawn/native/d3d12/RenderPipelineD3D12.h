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

#ifndef SRC_DAWN_NATIVE_D3D12_RENDERPIPELINED3D12_H_
#define SRC_DAWN_NATIVE_D3D12_RENDERPIPELINED3D12_H_

#include "dawn/native/RenderPipeline.h"

#include "dawn/native/d3d12/ShaderModuleD3D12.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

class Device;

class RenderPipeline final : public RenderPipelineBase {
  public:
    static Ref<RenderPipeline> CreateUninitialized(
        Device* device,
        const UnpackedPtr<RenderPipelineDescriptor>& descriptor);
    RenderPipeline() = delete;

    MaybeError InitializeImpl() override;

    D3D12_PRIMITIVE_TOPOLOGY GetD3D12PrimitiveTopology() const;
    ID3D12PipelineState* GetPipelineState() const;

    bool UsesVertexOrInstanceIndex() const;

    // Dawn API
    void SetLabelImpl() override;

    ComPtr<ID3D12CommandSignature> GetDrawIndirectCommandSignature();

    ComPtr<ID3D12CommandSignature> GetDrawIndexedIndirectCommandSignature();

  private:
    ~RenderPipeline() override;

    void DestroyImpl() override;

    using RenderPipelineBase::RenderPipelineBase;
    D3D12_INPUT_LAYOUT_DESC ComputeInputLayout(
        std::array<D3D12_INPUT_ELEMENT_DESC, kMaxVertexAttributes>* inputElementDescriptors);
    D3D12_DEPTH_STENCIL_DESC ComputeDepthStencilDesc();

    D3D12_PRIMITIVE_TOPOLOGY mD3d12PrimitiveTopology;
    ComPtr<ID3D12PipelineState> mPipelineState;
    bool mUsesVertexOrInstanceIndex;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_RENDERPIPELINED3D12_H_
