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

#include "src/dawn/native/d3d11/PipelineStateTrackerD3D11.h"

#include <cstring>

#include "src/dawn/native/d3d11/CommandRecordingContextD3D11.h"
#include "src/utils/compiler.h"

namespace dawn::native::d3d11 {

PipelineStateTracker::PipelineStateTracker(
    const ScopedSwapStateCommandRecordingContext* commandContext)
    : mCommandContext(commandContext) {}

void PipelineStateTracker::IASetInputLayout(ID3D11InputLayout* inputLayout) {
    if (mInputLayout == inputLayout) {
        return;
    }
    mInputLayout = inputLayout;
    mCommandContext->GetD3D11DeviceContext3()->IASetInputLayout(inputLayout);
}

void PipelineStateTracker::IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology) {
    if (mPrimitiveTopology == topology) {
        return;
    }
    mPrimitiveTopology = topology;
    mCommandContext->GetD3D11DeviceContext3()->IASetPrimitiveTopology(topology);
}

void PipelineStateTracker::VSSetShader(ID3D11VertexShader* vertexShader) {
    if (mVertexShader == vertexShader) {
        return;
    }
    mVertexShader = vertexShader;
    mCommandContext->GetD3D11DeviceContext3()->VSSetShader(vertexShader, nullptr, 0);
}

void PipelineStateTracker::PSSetShader(ID3D11PixelShader* pixelShader) {
    if (mPixelShader == pixelShader) {
        return;
    }
    mPixelShader = pixelShader;
    mCommandContext->GetD3D11DeviceContext3()->PSSetShader(pixelShader, nullptr, 0);
}

void PipelineStateTracker::CSSetShader(ID3D11ComputeShader* computeShader) {
    if (mComputeShader == computeShader) {
        return;
    }
    mComputeShader = computeShader;
    mCommandContext->GetD3D11DeviceContext3()->CSSetShader(computeShader, nullptr, 0);
}

void PipelineStateTracker::RSSetState(ID3D11RasterizerState* rasterizerState) {
    if (mRasterizerState == rasterizerState) {
        return;
    }
    mRasterizerState = rasterizerState;
    mCommandContext->GetD3D11DeviceContext3()->RSSetState(rasterizerState);
}

void PipelineStateTracker::OMSetBlendState(ID3D11BlendState* blendState,
                                           const FLOAT blendFactor[4],
                                           UINT sampleMask) {
    if (mBlendState == blendState && mSampleMask == sampleMask &&
        DAWN_UNSAFE_TODO(std::memcmp(mBlendFactor.data(), blendFactor, sizeof(mBlendFactor))) ==
            0) {
        return;
    }
    mBlendState = blendState;
    mSampleMask = sampleMask;
    DAWN_UNSAFE_TODO(std::memcpy(mBlendFactor.data(), blendFactor, sizeof(mBlendFactor)));
    mCommandContext->GetD3D11DeviceContext3()->OMSetBlendState(blendState, blendFactor, sampleMask);
}

void PipelineStateTracker::OMSetDepthStencilState(ID3D11DepthStencilState* depthStencilState,
                                                  UINT stencilRef) {
    if (mDepthStencilState == depthStencilState && mStencilReference == stencilRef) {
        return;
    }
    mDepthStencilState = depthStencilState;
    mStencilReference = stencilRef;
    mCommandContext->GetD3D11DeviceContext3()->OMSetDepthStencilState(depthStencilState,
                                                                      stencilRef);
}

}  // namespace dawn::native::d3d11
