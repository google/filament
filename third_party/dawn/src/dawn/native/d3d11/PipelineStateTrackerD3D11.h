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

#ifndef SRC_DAWN_NATIVE_D3D11_PIPELINESTATETRACKERD3D11_H_
#define SRC_DAWN_NATIVE_D3D11_PIPELINESTATETRACKERD3D11_H_

#include <array>
#include <optional>

#include "partition_alloc/pointers/raw_ptr.h"
#include "partition_alloc/pointers/raw_ptr_exclusion.h"
#include "src/dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d11 {

class ScopedSwapStateCommandRecordingContext;

// PipelineStateTracker caches current shaders and states to avoid redundant state changes.
// The public methods are similar to ID3D11DeviceContext.
class PipelineStateTracker {
  public:
    explicit PipelineStateTracker(const ScopedSwapStateCommandRecordingContext* commandContext);

    void IASetInputLayout(ID3D11InputLayout* inputLayout);
    void IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology);
    void VSSetShader(ID3D11VertexShader* vertexShader);
    void PSSetShader(ID3D11PixelShader* pixelShader);
    void CSSetShader(ID3D11ComputeShader* computeShader);
    void RSSetState(ID3D11RasterizerState* rasterizerState);
    void OMSetBlendState(ID3D11BlendState* blendState, const FLOAT blendFactor[4], UINT sampleMask);
    void OMSetDepthStencilState(ID3D11DepthStencilState* depthStencilState, UINT stencilRef);

  private:
    raw_ptr<const ScopedSwapStateCommandRecordingContext> mCommandContext;

    // This tracker is only used during Queue::Submit, where the pipelines and their associated
    // states are already kept alive. Therefore, storing cached states as raw pointers is safe
    // and won't result in Use-After-Free (UAF).
    std::optional<ID3D11InputLayout*> mInputLayout;
    D3D_PRIMITIVE_TOPOLOGY mPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    std::optional<ID3D11VertexShader*> mVertexShader;
    std::optional<ID3D11PixelShader*> mPixelShader;
    std::optional<ID3D11ComputeShader*> mComputeShader;
    std::optional<ID3D11RasterizerState*> mRasterizerState;
    std::optional<ID3D11BlendState*> mBlendState;
    std::array<float, 4> mBlendFactor = {0.0f, 0.0f, 0.0f, 0.0f};
    uint32_t mSampleMask = 0xFFFFFFFF;
    std::optional<ID3D11DepthStencilState*> mDepthStencilState;
    uint32_t mStencilReference = 0;
};

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_PIPELINESTATETRACKERD3D11_H_
