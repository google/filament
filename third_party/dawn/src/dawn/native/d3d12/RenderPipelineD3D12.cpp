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

#include "dawn/native/d3d12/RenderPipelineD3D12.h"

#include <d3dcompiler.h>

#include <memory>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/native/CreatePipelineAsyncEvent.h"
#include "dawn/native/Instance.h"
#include "dawn/native/d3d/BlobD3D.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/PipelineLayoutD3D12.h"
#include "dawn/native/d3d12/PlatformFunctionsD3D12.h"
#include "dawn/native/d3d12/ShaderModuleD3D12.h"
#include "dawn/native/d3d12/TextureD3D12.h"
#include "dawn/native/d3d12/UtilsD3D12.h"
#include "dawn/platform/metrics/HistogramMacros.h"

namespace dawn::native::d3d12 {
namespace {

D3D12_INPUT_CLASSIFICATION VertexStepModeFunction(wgpu::VertexStepMode mode) {
    switch (mode) {
        case wgpu::VertexStepMode::Vertex:
            return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        case wgpu::VertexStepMode::Instance:
            return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
        case wgpu::VertexStepMode::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

D3D12_PRIMITIVE_TOPOLOGY D3D12PrimitiveTopology(wgpu::PrimitiveTopology primitiveTopology) {
    switch (primitiveTopology) {
        case wgpu::PrimitiveTopology::PointList:
            return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
        case wgpu::PrimitiveTopology::LineList:
            return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
        case wgpu::PrimitiveTopology::LineStrip:
            return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
        case wgpu::PrimitiveTopology::TriangleList:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        case wgpu::PrimitiveTopology::TriangleStrip:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        case wgpu::PrimitiveTopology::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE D3D12PrimitiveTopologyType(
    wgpu::PrimitiveTopology primitiveTopology) {
    switch (primitiveTopology) {
        case wgpu::PrimitiveTopology::PointList:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
        case wgpu::PrimitiveTopology::LineList:
        case wgpu::PrimitiveTopology::LineStrip:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
        case wgpu::PrimitiveTopology::TriangleList:
        case wgpu::PrimitiveTopology::TriangleStrip:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        case wgpu::PrimitiveTopology::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

D3D12_CULL_MODE D3D12CullMode(wgpu::CullMode mode) {
    switch (mode) {
        case wgpu::CullMode::None:
            return D3D12_CULL_MODE_NONE;
        case wgpu::CullMode::Front:
            return D3D12_CULL_MODE_FRONT;
        case wgpu::CullMode::Back:
            return D3D12_CULL_MODE_BACK;
        case wgpu::CullMode::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

D3D12_BLEND D3D12Blend(wgpu::BlendFactor factor) {
    switch (factor) {
        case wgpu::BlendFactor::Zero:
            return D3D12_BLEND_ZERO;
        case wgpu::BlendFactor::One:
            return D3D12_BLEND_ONE;
        case wgpu::BlendFactor::Src:
            return D3D12_BLEND_SRC_COLOR;
        case wgpu::BlendFactor::OneMinusSrc:
            return D3D12_BLEND_INV_SRC_COLOR;
        case wgpu::BlendFactor::SrcAlpha:
            return D3D12_BLEND_SRC_ALPHA;
        case wgpu::BlendFactor::OneMinusSrcAlpha:
            return D3D12_BLEND_INV_SRC_ALPHA;
        case wgpu::BlendFactor::Dst:
            return D3D12_BLEND_DEST_COLOR;
        case wgpu::BlendFactor::OneMinusDst:
            return D3D12_BLEND_INV_DEST_COLOR;
        case wgpu::BlendFactor::DstAlpha:
            return D3D12_BLEND_DEST_ALPHA;
        case wgpu::BlendFactor::OneMinusDstAlpha:
            return D3D12_BLEND_INV_DEST_ALPHA;
        case wgpu::BlendFactor::SrcAlphaSaturated:
            return D3D12_BLEND_SRC_ALPHA_SAT;
        case wgpu::BlendFactor::Constant:
            return D3D12_BLEND_BLEND_FACTOR;
        case wgpu::BlendFactor::OneMinusConstant:
            return D3D12_BLEND_INV_BLEND_FACTOR;
        case wgpu::BlendFactor::Src1:
            return D3D12_BLEND_SRC1_COLOR;
        case wgpu::BlendFactor::OneMinusSrc1:
            return D3D12_BLEND_INV_SRC1_COLOR;
        case wgpu::BlendFactor::Src1Alpha:
            return D3D12_BLEND_SRC1_ALPHA;
        case wgpu::BlendFactor::OneMinusSrc1Alpha:
            return D3D12_BLEND_INV_SRC1_ALPHA;
        case wgpu::BlendFactor::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

// When a blend factor is defined for the alpha channel, any of the factors that don't
// explicitly state that they apply to alpha should be treated as their explicitly-alpha
// equivalents. See: https://github.com/gpuweb/gpuweb/issues/65
D3D12_BLEND D3D12AlphaBlend(wgpu::BlendFactor factor) {
    switch (factor) {
        case wgpu::BlendFactor::Src:
            return D3D12_BLEND_SRC_ALPHA;
        case wgpu::BlendFactor::OneMinusSrc:
            return D3D12_BLEND_INV_SRC_ALPHA;
        case wgpu::BlendFactor::Dst:
            return D3D12_BLEND_DEST_ALPHA;
        case wgpu::BlendFactor::OneMinusDst:
            return D3D12_BLEND_INV_DEST_ALPHA;
        case wgpu::BlendFactor::Src1:
            return D3D12_BLEND_SRC1_ALPHA;
        case wgpu::BlendFactor::OneMinusSrc1:
            return D3D12_BLEND_INV_SRC1_ALPHA;

        // Other blend factors translate to the same D3D12 enum as the color blend factors.
        default:
            return D3D12Blend(factor);
    }
}

D3D12_BLEND_OP D3D12BlendOperation(wgpu::BlendOperation operation) {
    switch (operation) {
        case wgpu::BlendOperation::Add:
            return D3D12_BLEND_OP_ADD;
        case wgpu::BlendOperation::Subtract:
            return D3D12_BLEND_OP_SUBTRACT;
        case wgpu::BlendOperation::ReverseSubtract:
            return D3D12_BLEND_OP_REV_SUBTRACT;
        case wgpu::BlendOperation::Min:
            return D3D12_BLEND_OP_MIN;
        case wgpu::BlendOperation::Max:
            return D3D12_BLEND_OP_MAX;
        case wgpu::BlendOperation::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

uint8_t D3D12RenderTargetWriteMask(wgpu::ColorWriteMask writeMask) {
    static_assert(static_cast<D3D12_COLOR_WRITE_ENABLE>(wgpu::ColorWriteMask::Red) ==
                      D3D12_COLOR_WRITE_ENABLE_RED,
                  "ColorWriteMask values must match");
    static_assert(static_cast<D3D12_COLOR_WRITE_ENABLE>(wgpu::ColorWriteMask::Green) ==
                      D3D12_COLOR_WRITE_ENABLE_GREEN,
                  "ColorWriteMask values must match");
    static_assert(static_cast<D3D12_COLOR_WRITE_ENABLE>(wgpu::ColorWriteMask::Blue) ==
                      D3D12_COLOR_WRITE_ENABLE_BLUE,
                  "ColorWriteMask values must match");
    static_assert(static_cast<D3D12_COLOR_WRITE_ENABLE>(wgpu::ColorWriteMask::Alpha) ==
                      D3D12_COLOR_WRITE_ENABLE_ALPHA,
                  "ColorWriteMask values must match");
    return static_cast<uint8_t>(writeMask);
}

D3D12_RENDER_TARGET_BLEND_DESC ComputeColorDesc(const DeviceBase* device,
                                                const ColorTargetState* state) {
    D3D12_RENDER_TARGET_BLEND_DESC blendDesc = {};
    blendDesc.BlendEnable = state->blend != nullptr;
    if (blendDesc.BlendEnable) {
        blendDesc.SrcBlend = D3D12Blend(state->blend->color.srcFactor);
        if (device->GetValidInternalFormat(state->format).componentCount < 4 &&
            blendDesc.SrcBlend == D3D12_BLEND_DEST_ALPHA) {
            // According to the D3D SPEC, the default value for missing components in an element
            // format is "0" for any component except A, which gets "1". So here
            // D3D12_BLEND_DEST_ALPHA should have same effect as D3D12_BLEND_ONE.
            // Note that this replacement can be an optimization as using D3D12_BLEND_ONE means the
            // GPU hardware no longer needs to get pixels from the destination texture. It can also
            // be served as a workaround against an Intel driver issue about alpha blending (see
            // http://crbug.com/dawn/1579 for more details).
            blendDesc.SrcBlend = D3D12_BLEND_ONE;
        }

        blendDesc.DestBlend = D3D12Blend(state->blend->color.dstFactor);
        blendDesc.BlendOp = D3D12BlendOperation(state->blend->color.operation);
        blendDesc.SrcBlendAlpha = D3D12AlphaBlend(state->blend->alpha.srcFactor);
        blendDesc.DestBlendAlpha = D3D12AlphaBlend(state->blend->alpha.dstFactor);
        blendDesc.BlendOpAlpha = D3D12BlendOperation(state->blend->alpha.operation);

        if (device->IsToggleEnabled(
                Toggle::D3D12ReplaceAddWithMinusWhenDstFactorIsZeroAndSrcFactorIsDstAlpha) &&
            blendDesc.SrcBlend == D3D12_BLEND_DEST_ALPHA &&
            blendDesc.SrcBlendAlpha == D3D12_BLEND_DEST_ALPHA &&
            blendDesc.BlendOp == D3D12_BLEND_OP_ADD &&
            blendDesc.BlendOpAlpha == D3D12_BLEND_OP_ADD &&
            blendDesc.DestBlend == D3D12_BLEND_ZERO &&
            blendDesc.DestBlendAlpha == D3D12_BLEND_ZERO) {
            blendDesc.BlendOp = D3D12_BLEND_OP_SUBTRACT;
            blendDesc.BlendOpAlpha = D3D12_BLEND_OP_SUBTRACT;
        }
    }
    blendDesc.RenderTargetWriteMask = D3D12RenderTargetWriteMask(state->writeMask);
    blendDesc.LogicOpEnable = false;
    blendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
    return blendDesc;
}

D3D12_STENCIL_OP StencilOp(wgpu::StencilOperation op) {
    switch (op) {
        case wgpu::StencilOperation::Keep:
            return D3D12_STENCIL_OP_KEEP;
        case wgpu::StencilOperation::Zero:
            return D3D12_STENCIL_OP_ZERO;
        case wgpu::StencilOperation::Replace:
            return D3D12_STENCIL_OP_REPLACE;
        case wgpu::StencilOperation::IncrementClamp:
            return D3D12_STENCIL_OP_INCR_SAT;
        case wgpu::StencilOperation::DecrementClamp:
            return D3D12_STENCIL_OP_DECR_SAT;
        case wgpu::StencilOperation::Invert:
            return D3D12_STENCIL_OP_INVERT;
        case wgpu::StencilOperation::IncrementWrap:
            return D3D12_STENCIL_OP_INCR;
        case wgpu::StencilOperation::DecrementWrap:
            return D3D12_STENCIL_OP_DECR;
        case wgpu::StencilOperation::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

D3D12_DEPTH_STENCILOP_DESC StencilOpDesc(const StencilFaceState& descriptor) {
    D3D12_DEPTH_STENCILOP_DESC desc = {};

    desc.StencilFailOp = StencilOp(descriptor.failOp);
    desc.StencilDepthFailOp = StencilOp(descriptor.depthFailOp);
    desc.StencilPassOp = StencilOp(descriptor.passOp);
    desc.StencilFunc = ToD3D12ComparisonFunc(descriptor.compare);

    return desc;
}

D3D12_INDEX_BUFFER_STRIP_CUT_VALUE ComputeIndexBufferStripCutValue(
    wgpu::PrimitiveTopology primitiveTopology,
    wgpu::IndexFormat indexFormat) {
    if (primitiveTopology != wgpu::PrimitiveTopology::TriangleStrip &&
        primitiveTopology != wgpu::PrimitiveTopology::LineStrip) {
        return D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    }

    switch (indexFormat) {
        case wgpu::IndexFormat::Uint16:
            return D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
        case wgpu::IndexFormat::Uint32:
            return D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF;
        case wgpu::IndexFormat::Undefined:
            return D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    }
}

}  // anonymous namespace

Ref<RenderPipeline> RenderPipeline::CreateUninitialized(
    Device* device,
    const UnpackedPtr<RenderPipelineDescriptor>& descriptor) {
    return AcquireRef(new RenderPipeline(device, descriptor));
}

MaybeError RenderPipeline::InitializeImpl() {
    Device* device = ToBackend(GetDevice());
    uint32_t compileFlags = 0;

    if (!device->IsToggleEnabled(Toggle::UseDXC) &&
        !device->IsToggleEnabled(Toggle::FxcOptimizations)) {
        compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL0;
    }

    if (device->IsToggleEnabled(Toggle::EmitHLSLDebugSymbols)) {
        compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    }

    if (device->IsToggleEnabled(Toggle::UseDXC) &&
        ((compileFlags & D3DCOMPILE_OPTIMIZATION_LEVEL2) == 0)) {
        // DXC's default opt level is /O3, unlike FXC's /O1. Set explicitly, otherwise there's no
        // way to tell if we want /O1 as D3DCOMPILE_OPTIMIZATION_LEVEL1 is defined to 0.
        compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    }

    // Tint does matrix multiplication expecting row major matrices
    compileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC descriptorD3D12 = {};

    PerStage<D3D12_SHADER_BYTECODE*> shaders;
    shaders[SingleShaderStage::Vertex] = &descriptorD3D12.VS;
    shaders[SingleShaderStage::Fragment] = &descriptorD3D12.PS;

    PerStage<d3d::CompiledShader> compiledShader;

    std::optional<dawn::native::d3d::InterStageShaderVariablesMask> usedInterstageVariables;
    dawn::native::EntryPointMetadata fragmentEntryPoint;
    if (GetStageMask() & wgpu::ShaderStage::Fragment) {
        // Now that only fragment shader can have interstage inputs.
        const ProgrammableStage& programmableStage = GetStage(SingleShaderStage::Fragment);
        fragmentEntryPoint = programmableStage.module->GetEntryPoint(programmableStage.entryPoint);
        usedInterstageVariables = dawn::native::d3d::ToInterStageShaderVariablesMask(
            fragmentEntryPoint.usedInterStageVariables);
    }

    for (auto stage : IterateStages(GetStageMask())) {
        const ProgrammableStage& programmableStage = GetStage(stage);
        uint32_t additionalCompileFlags = 0;
        if (programmableStage.module->GetStrictMath().value_or(
                !device->IsToggleEnabled(Toggle::D3DDisableIEEEStrictness))) {
            additionalCompileFlags |= D3DCOMPILE_IEEE_STRICTNESS;
        }
        DAWN_TRY_ASSIGN(
            compiledShader[stage],
            ToBackend(programmableStage.module)
                ->Compile(programmableStage, stage, ToBackend(GetLayout()),
                          compileFlags | additionalCompileFlags, usedInterstageVariables));
        *shaders[stage] = {compiledShader[stage].shaderBlob.Data(),
                           compiledShader[stage].shaderBlob.Size()};
    }

    mUsesVertexOrInstanceIndex = compiledShader[SingleShaderStage::Vertex].usesVertexIndex ||
                                 compiledShader[SingleShaderStage::Vertex].usesInstanceIndex;

    PipelineLayout* layout = ToBackend(GetLayout());

    descriptorD3D12.pRootSignature = layout->GetRootSignature();

    // D3D12 logs warnings if any empty input state is used
    std::array<D3D12_INPUT_ELEMENT_DESC, kMaxVertexAttributes> inputElementDescriptors;
    if (GetAttributeLocationsUsed().any()) {
        descriptorD3D12.InputLayout = ComputeInputLayout(&inputElementDescriptors);
    }

    descriptorD3D12.IBStripCutValue =
        ComputeIndexBufferStripCutValue(GetPrimitiveTopology(), GetStripIndexFormat());

    descriptorD3D12.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    descriptorD3D12.RasterizerState.CullMode = D3D12CullMode(GetCullMode());
    descriptorD3D12.RasterizerState.FrontCounterClockwise =
        (GetFrontFace() == wgpu::FrontFace::CCW) ? TRUE : FALSE;
    descriptorD3D12.RasterizerState.DepthBias = GetDepthBias();
    descriptorD3D12.RasterizerState.DepthBiasClamp = GetDepthBiasClamp();
    descriptorD3D12.RasterizerState.SlopeScaledDepthBias = GetDepthBiasSlopeScale();
    descriptorD3D12.RasterizerState.DepthClipEnable = !HasUnclippedDepth();
    descriptorD3D12.RasterizerState.MultisampleEnable = (GetSampleCount() > 1) ? TRUE : FALSE;
    descriptorD3D12.RasterizerState.AntialiasedLineEnable = FALSE;
    descriptorD3D12.RasterizerState.ForcedSampleCount = 0;
    descriptorD3D12.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    if (HasDepthStencilAttachment()) {
        descriptorD3D12.DSVFormat = d3d::DXGITextureFormat(device, GetDepthStencilFormat());
    }

    static_assert(kMaxColorAttachments == 8);
    auto highestColorAttachmentIndexPlusOne = GetHighestBitIndexPlusOne(GetColorAttachmentsMask());
    for (uint8_t i = 0; i < kMaxColorAttachments; i++) {
        if (i < static_cast<uint8_t>(highestColorAttachmentIndexPlusOne)) {
            descriptorD3D12.RTVFormats[i] = GetNullRTVDXGIFormatForD3D12RenderPass();
        } else {
            descriptorD3D12.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
        }
        descriptorD3D12.BlendState.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
    }
    for (auto i : IterateBitSet(GetColorAttachmentsMask())) {
        descriptorD3D12.RTVFormats[static_cast<uint8_t>(i)] =
            d3d::DXGITextureFormat(device, GetColorAttachmentFormat(i));
        descriptorD3D12.BlendState.RenderTarget[static_cast<uint8_t>(i)] =
            ComputeColorDesc(device, GetColorTargetState(i));
    }
    DAWN_ASSERT(highestColorAttachmentIndexPlusOne <= kMaxColorAttachmentsTyped);
    descriptorD3D12.NumRenderTargets = static_cast<uint8_t>(highestColorAttachmentIndexPlusOne);

    descriptorD3D12.BlendState.AlphaToCoverageEnable = IsAlphaToCoverageEnabled();
    descriptorD3D12.BlendState.IndependentBlendEnable = TRUE;

    descriptorD3D12.DepthStencilState = ComputeDepthStencilDesc();

    descriptorD3D12.SampleMask = GetSampleMask();
    descriptorD3D12.PrimitiveTopologyType = D3D12PrimitiveTopologyType(GetPrimitiveTopology());
    descriptorD3D12.SampleDesc.Count = GetSampleCount();
    descriptorD3D12.SampleDesc.Quality = 0;

    mD3d12PrimitiveTopology = D3D12PrimitiveTopology(GetPrimitiveTopology());

    StreamIn(&mCacheKey, descriptorD3D12, *layout->GetRootSignatureBlob());

    // Try to see if we have anything in the blob cache.
    Blob blob = device->LoadCachedBlob(GetCacheKey());
    bool cacheHit = !blob.Empty();
    if (cacheHit) {
        // Cache hits, attach cached blob to descriptor.
        descriptorD3D12.CachedPSO.pCachedBlob = blob.Data();
        descriptorD3D12.CachedPSO.CachedBlobSizeInBytes = blob.Size();
    }

    // We don't use the scoped cache histogram counters for the cache hit here so that we can
    // condition on whether it fails appropriately.
    auto* d3d12Device = device->GetD3D12Device();
    platform::metrics::DawnHistogramTimer cacheTimer(device->GetPlatform());
    HRESULT result =
        d3d12Device->CreateGraphicsPipelineState(&descriptorD3D12, IID_PPV_ARGS(&mPipelineState));
    if (cacheHit && result == D3D12_ERROR_DRIVER_VERSION_MISMATCH) {
        // See dawn:1878 where it is possible for the PSO creation to fail with this error.
        cacheHit = false;
        descriptorD3D12.CachedPSO.pCachedBlob = nullptr;
        descriptorD3D12.CachedPSO.CachedBlobSizeInBytes = 0;
        cacheTimer.Reset();
        result = d3d12Device->CreateGraphicsPipelineState(&descriptorD3D12,
                                                          IID_PPV_ARGS(&mPipelineState));
    }
    DAWN_TRY(CheckHRESULT(result, "D3D12 create graphics pipeline state"));

    if (!cacheHit) {
        // Cache misses, need to get pipeline cached blob and store.
        cacheTimer.RecordMicroseconds("D3D12.CreateGraphicsPipelineState.CacheMiss");
        ComPtr<ID3DBlob> d3dBlob;
        if (!device->GetInstance()->ConsumedError(
                CheckHRESULT(GetPipelineState()->GetCachedBlob(&d3dBlob),
                             "D3D12 render pipeline state get cached blob"))) {
            device->StoreCachedBlob(GetCacheKey(), CreateBlob(std::move(d3dBlob)));
        }
    } else {
        cacheTimer.RecordMicroseconds("D3D12.CreateGraphicsPipelineState.CacheHit");
    }

    SetLabelImpl();

    return {};
}

RenderPipeline::~RenderPipeline() = default;

void RenderPipeline::DestroyImpl() {
    RenderPipelineBase::DestroyImpl();
    ToBackend(GetDevice())->ReferenceUntilUnused(mPipelineState);
}

D3D12_PRIMITIVE_TOPOLOGY RenderPipeline::GetD3D12PrimitiveTopology() const {
    return mD3d12PrimitiveTopology;
}

ID3D12PipelineState* RenderPipeline::GetPipelineState() const {
    return mPipelineState.Get();
}

bool RenderPipeline::UsesVertexOrInstanceIndex() const {
    return mUsesVertexOrInstanceIndex;
}

void RenderPipeline::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), GetPipelineState(), "Dawn_RenderPipeline", GetLabel());
}

ComPtr<ID3D12CommandSignature> RenderPipeline::GetDrawIndirectCommandSignature() {
    if (mUsesVertexOrInstanceIndex) {
        return ToBackend(GetLayout())->GetDrawIndirectCommandSignatureWithInstanceVertexOffsets();
    }

    return ToBackend(GetDevice())->GetDrawIndirectSignature();
}

ComPtr<ID3D12CommandSignature> RenderPipeline::GetDrawIndexedIndirectCommandSignature() {
    if (mUsesVertexOrInstanceIndex) {
        return ToBackend(GetLayout())
            ->GetDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets();
    }

    return ToBackend(GetDevice())->GetDrawIndexedIndirectSignature();
}

D3D12_INPUT_LAYOUT_DESC RenderPipeline::ComputeInputLayout(
    std::array<D3D12_INPUT_ELEMENT_DESC, kMaxVertexAttributes>* inputElementDescriptors) {
    unsigned int count = 0;
    for (VertexAttributeLocation loc : IterateBitSet(GetAttributeLocationsUsed())) {
        D3D12_INPUT_ELEMENT_DESC& inputElementDescriptor = (*inputElementDescriptors)[count++];

        const VertexAttributeInfo& attribute = GetAttribute(loc);

        // If the HLSL semantic is TEXCOORDN the SemanticName should be "TEXCOORD" and the
        // SemanticIndex N
        inputElementDescriptor.SemanticName = "TEXCOORD";
        inputElementDescriptor.SemanticIndex = static_cast<uint8_t>(loc);
        inputElementDescriptor.Format = d3d::DXGIVertexFormat(attribute.format);
        inputElementDescriptor.InputSlot = static_cast<uint8_t>(attribute.vertexBufferSlot);

        const VertexBufferInfo& input = GetVertexBuffer(attribute.vertexBufferSlot);

        inputElementDescriptor.AlignedByteOffset = attribute.offset;
        inputElementDescriptor.InputSlotClass = VertexStepModeFunction(input.stepMode);
        if (inputElementDescriptor.InputSlotClass == D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA) {
            inputElementDescriptor.InstanceDataStepRate = 0;
        } else {
            inputElementDescriptor.InstanceDataStepRate = 1;
        }
    }

    D3D12_INPUT_LAYOUT_DESC inputLayoutDescriptor;
    inputLayoutDescriptor.pInputElementDescs = &(*inputElementDescriptors)[0];
    inputLayoutDescriptor.NumElements = count;
    return inputLayoutDescriptor;
}

D3D12_DEPTH_STENCIL_DESC RenderPipeline::ComputeDepthStencilDesc() {
    const DepthStencilState* descriptor = GetDepthStencilState();

    D3D12_DEPTH_STENCIL_DESC depthStencilDescriptor = {};
    depthStencilDescriptor.DepthEnable =
        (descriptor->depthCompare == wgpu::CompareFunction::Always &&
         descriptor->depthWriteEnabled != wgpu::OptionalBool::True)
            ? FALSE
            : TRUE;
    depthStencilDescriptor.DepthWriteMask =
        descriptor->depthWriteEnabled == wgpu::OptionalBool::True ? D3D12_DEPTH_WRITE_MASK_ALL
                                                                  : D3D12_DEPTH_WRITE_MASK_ZERO;
    depthStencilDescriptor.DepthFunc = ToD3D12ComparisonFunc(descriptor->depthCompare);

    depthStencilDescriptor.StencilEnable = UsesStencil() ? TRUE : FALSE;
    depthStencilDescriptor.StencilReadMask = static_cast<UINT8>(descriptor->stencilReadMask);
    depthStencilDescriptor.StencilWriteMask = static_cast<UINT8>(descriptor->stencilWriteMask);

    depthStencilDescriptor.FrontFace = StencilOpDesc(descriptor->stencilFront);
    depthStencilDescriptor.BackFace = StencilOpDesc(descriptor->stencilBack);
    return depthStencilDescriptor;
}

}  // namespace dawn::native::d3d12
