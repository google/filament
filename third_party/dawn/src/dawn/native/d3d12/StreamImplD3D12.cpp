// Copyright 2022 The Dawn & Tint Authors
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

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/native/d3d12/d3d12_platform.h"
#include "dawn/native/stream/Stream.h"

namespace dawn::native {

template <>
void stream::Stream<D3D12_RENDER_TARGET_BLEND_DESC>::Write(
    stream::Sink* sink,
    const D3D12_RENDER_TARGET_BLEND_DESC& t) {
    StreamIn(sink, t.BlendEnable, t.LogicOpEnable, t.SrcBlend, t.DestBlend, t.BlendOp,
             t.SrcBlendAlpha, t.DestBlendAlpha, t.BlendOpAlpha, t.LogicOp, t.RenderTargetWriteMask);
}

template <>
void stream::Stream<D3D12_BLEND_DESC>::Write(stream::Sink* sink, const D3D12_BLEND_DESC& t) {
    StreamIn(sink, t.AlphaToCoverageEnable, t.IndependentBlendEnable, t.RenderTarget);
}

template <>
void stream::Stream<D3D12_DEPTH_STENCILOP_DESC>::Write(stream::Sink* sink,
                                                       const D3D12_DEPTH_STENCILOP_DESC& t) {
    StreamIn(sink, t.StencilFailOp, t.StencilDepthFailOp, t.StencilPassOp, t.StencilFunc);
}

template <>
void stream::Stream<D3D12_DEPTH_STENCIL_DESC>::Write(stream::Sink* sink,
                                                     const D3D12_DEPTH_STENCIL_DESC& t) {
    StreamIn(sink, t.DepthEnable, t.DepthWriteMask, t.DepthFunc, t.StencilEnable, t.StencilReadMask,
             t.StencilWriteMask, t.FrontFace, t.BackFace);
}

template <>
void stream::Stream<D3D12_RASTERIZER_DESC>::Write(stream::Sink* sink,
                                                  const D3D12_RASTERIZER_DESC& t) {
    StreamIn(sink, t.FillMode, t.CullMode, t.FrontCounterClockwise, t.DepthBias, t.DepthBiasClamp,
             t.SlopeScaledDepthBias, t.DepthClipEnable, t.MultisampleEnable,
             t.AntialiasedLineEnable, t.ForcedSampleCount, t.ConservativeRaster);
}

template <>
void stream::Stream<D3D12_INPUT_ELEMENT_DESC>::Write(stream::Sink* sink,
                                                     const D3D12_INPUT_ELEMENT_DESC& t) {
    StreamIn(sink, std::string_view(t.SemanticName), t.SemanticIndex, t.Format, t.InputSlot,
             t.AlignedByteOffset, t.InputSlotClass, t.InstanceDataStepRate);
}

template <>
void stream::Stream<D3D12_INPUT_LAYOUT_DESC>::Write(stream::Sink* sink,
                                                    const D3D12_INPUT_LAYOUT_DESC& t) {
    StreamIn(sink, Iterable(t.pInputElementDescs, t.NumElements));
}

template <>
void stream::Stream<D3D12_SO_DECLARATION_ENTRY>::Write(stream::Sink* sink,
                                                       const D3D12_SO_DECLARATION_ENTRY& t) {
    StreamIn(sink, t.Stream, std::string_view(t.SemanticName), t.SemanticIndex, t.StartComponent,
             t.ComponentCount, t.OutputSlot);
}

template <>
void stream::Stream<D3D12_STREAM_OUTPUT_DESC>::Write(stream::Sink* sink,
                                                     const D3D12_STREAM_OUTPUT_DESC& t) {
    StreamIn(sink, Iterable(t.pSODeclaration, t.NumEntries),
             Iterable(t.pBufferStrides, t.NumStrides), t.RasterizedStream);
}

template <>
void stream::Stream<DXGI_SAMPLE_DESC>::Write(stream::Sink* sink, const DXGI_SAMPLE_DESC& t) {
    StreamIn(sink, t.Count, t.Quality);
}

template <>
void stream::Stream<D3D12_SHADER_BYTECODE>::Write(stream::Sink* sink,
                                                  const D3D12_SHADER_BYTECODE& t) {
    StreamIn(sink, Iterable(reinterpret_cast<const uint8_t*>(t.pShaderBytecode), t.BytecodeLength));
}

template <>
void stream::Stream<D3D12_GRAPHICS_PIPELINE_STATE_DESC>::Write(
    stream::Sink* sink,
    const D3D12_GRAPHICS_PIPELINE_STATE_DESC& t) {
    // Don't Serialize pRootSignature as we already serialize the signature blob in pipline layout.
    // Don't Serialize CachedPSO as it is in the cached blob.
    StreamIn(sink, t.VS, t.PS, t.DS, t.HS, t.GS, t.StreamOutput, t.BlendState, t.SampleMask,
             t.RasterizerState, t.DepthStencilState, t.InputLayout, t.IBStripCutValue,
             t.PrimitiveTopologyType, Iterable(t.RTVFormats, t.NumRenderTargets), t.DSVFormat,
             t.SampleDesc, t.NodeMask, t.Flags);
}

template <>
void stream::Stream<D3D12_COMPUTE_PIPELINE_STATE_DESC>::Write(
    stream::Sink* sink,
    const D3D12_COMPUTE_PIPELINE_STATE_DESC& t) {
    // Don't Serialize pRootSignature as we already serialize the signature blob in pipline layout.
    StreamIn(sink, t.CS, t.NodeMask, t.Flags);
}

template <>
void stream::Stream<ID3DBlob>::Write(stream::Sink* sink, const ID3DBlob& t) {
    // Workaround: GetBufferPointer and GetbufferSize are not marked as const
    ID3DBlob* pBlob = const_cast<ID3DBlob*>(&t);
    StreamIn(sink, Iterable(reinterpret_cast<uint8_t*>(pBlob->GetBufferPointer()),
                            pBlob->GetBufferSize()));
}

}  // namespace dawn::native
