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

#ifndef SRC_DAWN_NATIVE_D3D11_RENDERPIPELINED3D11_H_
#define SRC_DAWN_NATIVE_D3D11_RENDERPIPELINED3D11_H_

#include <array>
#include <vector>

#include "dawn/native/RenderPipeline.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d11 {

class Device;
class PersistentPipelineState;
class ScopedSwapStateCommandRecordingContext;

class RenderPipeline final : public RenderPipelineBase {
  public:
    static Ref<RenderPipeline> CreateUninitialized(
        Device* device,
        const UnpackedPtr<RenderPipelineDescriptor>& descriptor);

    void ApplyNow(const ScopedSwapStateCommandRecordingContext* commandContext,
                  const std::array<float, 4>& blendColor,
                  uint32_t stencilReference);
    void ApplyBlendState(const ScopedSwapStateCommandRecordingContext* commandContext,
                         const std::array<float, 4>& blendColor);
    void ApplyDepthStencilState(const ScopedSwapStateCommandRecordingContext* commandContext,
                                uint32_t stencilReference);

    bool UsesVertexIndex() const { return mUsesVertexIndex; }
    bool UsesInstanceIndex() const { return mUsesInstanceIndex; }

  private:
    RenderPipeline(Device* device, const UnpackedPtr<RenderPipelineDescriptor>& descriptor);
    ~RenderPipeline() override;

    MaybeError InitializeImpl() override;
    void SetLabelImpl() override;

    MaybeError InitializeRasterizerState();
    MaybeError InitializeInputLayout(const Blob& vertexShader);
    MaybeError InitializeShaders();
    MaybeError InitializeBlendState();
    MaybeError InitializeDepthStencilState();

    const D3D_PRIMITIVE_TOPOLOGY mD3DPrimitiveTopology;
    ComPtr<ID3D11RasterizerState> mRasterizerState;
    ComPtr<ID3D11InputLayout> mInputLayout;
    ComPtr<ID3D11VertexShader> mVertexShader;
    ComPtr<ID3D11PixelShader> mPixelShader;
    ComPtr<ID3D11BlendState> mBlendState;
    ComPtr<ID3D11DepthStencilState> mDepthStencilState;
    bool mUsesVertexIndex = false;
    bool mUsesInstanceIndex = false;
};

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_RENDERPIPELINED3D11_H_
