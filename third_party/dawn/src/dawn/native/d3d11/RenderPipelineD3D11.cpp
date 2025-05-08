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

#include "dawn/native/d3d11/RenderPipelineD3D11.h"

#include <d3dcompiler.h>

#include <array>
#include <memory>
#include <optional>
#include <utility>

#include "dawn/common/Range.h"
#include "dawn/native/CreatePipelineAsyncEvent.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d/ShaderUtils.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/Forward.h"
#include "dawn/native/d3d11/PipelineLayoutD3D11.h"
#include "dawn/native/d3d11/ShaderModuleD3D11.h"
#include "dawn/native/d3d11/UtilsD3D11.h"

namespace dawn::native::d3d11 {
namespace {

D3D11_INPUT_CLASSIFICATION VertexStepModeFunction(wgpu::VertexStepMode mode) {
    switch (mode) {
        case wgpu::VertexStepMode::Vertex:
            return D3D11_INPUT_PER_VERTEX_DATA;
        case wgpu::VertexStepMode::Instance:
            return D3D11_INPUT_PER_INSTANCE_DATA;
        case wgpu::VertexStepMode::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

D3D_PRIMITIVE_TOPOLOGY D3DPrimitiveTopology(wgpu::PrimitiveTopology topology) {
    switch (topology) {
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

D3D11_CULL_MODE D3DCullMode(wgpu::CullMode cullMode) {
    switch (cullMode) {
        case wgpu::CullMode::None:
            return D3D11_CULL_NONE;
        case wgpu::CullMode::Front:
            return D3D11_CULL_FRONT;
        case wgpu::CullMode::Back:
            return D3D11_CULL_BACK;
        case wgpu::CullMode::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

D3D11_BLEND D3DBlendFactor(wgpu::BlendFactor blendFactor) {
    switch (blendFactor) {
        case wgpu::BlendFactor::Zero:
            return D3D11_BLEND_ZERO;
        case wgpu::BlendFactor::One:
            return D3D11_BLEND_ONE;
        case wgpu::BlendFactor::Src:
            return D3D11_BLEND_SRC_COLOR;
        case wgpu::BlendFactor::OneMinusSrc:
            return D3D11_BLEND_INV_SRC_COLOR;
        case wgpu::BlendFactor::SrcAlpha:
            return D3D11_BLEND_SRC_ALPHA;
        case wgpu::BlendFactor::OneMinusSrcAlpha:
            return D3D11_BLEND_INV_SRC_ALPHA;
        case wgpu::BlendFactor::Dst:
            return D3D11_BLEND_DEST_COLOR;
        case wgpu::BlendFactor::OneMinusDst:
            return D3D11_BLEND_INV_DEST_COLOR;
        case wgpu::BlendFactor::DstAlpha:
            return D3D11_BLEND_DEST_ALPHA;
        case wgpu::BlendFactor::OneMinusDstAlpha:
            return D3D11_BLEND_INV_DEST_ALPHA;
        case wgpu::BlendFactor::SrcAlphaSaturated:
            return D3D11_BLEND_SRC_ALPHA_SAT;
        case wgpu::BlendFactor::Constant:
            return D3D11_BLEND_BLEND_FACTOR;
        case wgpu::BlendFactor::OneMinusConstant:
            return D3D11_BLEND_INV_BLEND_FACTOR;
        case wgpu::BlendFactor::Src1:
            return D3D11_BLEND_SRC1_COLOR;
        case wgpu::BlendFactor::OneMinusSrc1:
            return D3D11_BLEND_INV_SRC1_COLOR;
        case wgpu::BlendFactor::Src1Alpha:
            return D3D11_BLEND_SRC1_ALPHA;
        case wgpu::BlendFactor::OneMinusSrc1Alpha:
            return D3D11_BLEND_INV_SRC1_ALPHA;
        case wgpu::BlendFactor::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

// When a blend factor is defined for the alpha channel, any of the factors that don't
// explicitly state that they apply to alpha should be treated as their explicitly-alpha
// equivalents. See: https://github.com/gpuweb/gpuweb/issues/65
D3D11_BLEND D3DBlendAlphaFactor(wgpu::BlendFactor factor) {
    switch (factor) {
        case wgpu::BlendFactor::Src:
            return D3D11_BLEND_SRC_ALPHA;
        case wgpu::BlendFactor::OneMinusSrc:
            return D3D11_BLEND_INV_SRC_ALPHA;
        case wgpu::BlendFactor::Dst:
            return D3D11_BLEND_DEST_ALPHA;
        case wgpu::BlendFactor::OneMinusDst:
            return D3D11_BLEND_INV_DEST_ALPHA;
        case wgpu::BlendFactor::Src1:
            return D3D11_BLEND_SRC1_ALPHA;
        case wgpu::BlendFactor::OneMinusSrc1:
            return D3D11_BLEND_INV_SRC1_ALPHA;

        // Other blend factors translate to the same D3D11 enum as the color blend factors.
        default:
            return D3DBlendFactor(factor);
    }
}

D3D11_BLEND_OP D3DBlendOperation(wgpu::BlendOperation blendOperation) {
    switch (blendOperation) {
        case wgpu::BlendOperation::Add:
            return D3D11_BLEND_OP_ADD;
        case wgpu::BlendOperation::Subtract:
            return D3D11_BLEND_OP_SUBTRACT;
        case wgpu::BlendOperation::ReverseSubtract:
            return D3D11_BLEND_OP_REV_SUBTRACT;
        case wgpu::BlendOperation::Min:
            return D3D11_BLEND_OP_MIN;
        case wgpu::BlendOperation::Max:
            return D3D11_BLEND_OP_MAX;
        case wgpu::BlendOperation::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

UINT D3DColorWriteMask(wgpu::ColorWriteMask colorWriteMask) {
    static_assert(static_cast<UINT>(wgpu::ColorWriteMask::Red) == D3D11_COLOR_WRITE_ENABLE_RED);
    static_assert(static_cast<UINT>(wgpu::ColorWriteMask::Green) == D3D11_COLOR_WRITE_ENABLE_GREEN);
    static_assert(static_cast<UINT>(wgpu::ColorWriteMask::Blue) == D3D11_COLOR_WRITE_ENABLE_BLUE);
    static_assert(static_cast<UINT>(wgpu::ColorWriteMask::Alpha) == D3D11_COLOR_WRITE_ENABLE_ALPHA);

    return static_cast<UINT>(colorWriteMask);
}

D3D11_STENCIL_OP StencilOp(wgpu::StencilOperation op) {
    switch (op) {
        case wgpu::StencilOperation::Keep:
            return D3D11_STENCIL_OP_KEEP;
        case wgpu::StencilOperation::Zero:
            return D3D11_STENCIL_OP_ZERO;
        case wgpu::StencilOperation::Replace:
            return D3D11_STENCIL_OP_REPLACE;
        case wgpu::StencilOperation::IncrementClamp:
            return D3D11_STENCIL_OP_INCR_SAT;
        case wgpu::StencilOperation::DecrementClamp:
            return D3D11_STENCIL_OP_DECR_SAT;
        case wgpu::StencilOperation::Invert:
            return D3D11_STENCIL_OP_INVERT;
        case wgpu::StencilOperation::IncrementWrap:
            return D3D11_STENCIL_OP_INCR;
        case wgpu::StencilOperation::DecrementWrap:
            return D3D11_STENCIL_OP_DECR;
        case wgpu::StencilOperation::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

D3D11_DEPTH_STENCILOP_DESC StencilOpDesc(const StencilFaceState& descriptor) {
    D3D11_DEPTH_STENCILOP_DESC desc = {};

    desc.StencilFailOp = StencilOp(descriptor.failOp);
    desc.StencilDepthFailOp = StencilOp(descriptor.depthFailOp);
    desc.StencilPassOp = StencilOp(descriptor.passOp);
    desc.StencilFunc = ToD3D11ComparisonFunc(descriptor.compare);

    return desc;
}

}  // namespace

// static
Ref<RenderPipeline> RenderPipeline::CreateUninitialized(
    Device* device,
    const UnpackedPtr<RenderPipelineDescriptor>& descriptor) {
    return AcquireRef(new RenderPipeline(device, descriptor));
}

RenderPipeline::RenderPipeline(Device* device,
                               const UnpackedPtr<RenderPipelineDescriptor>& descriptor)
    : RenderPipelineBase(device, descriptor),
      mD3DPrimitiveTopology(D3DPrimitiveTopology(GetPrimitiveTopology())) {}

MaybeError RenderPipeline::InitializeImpl() {
    DAWN_TRY(InitializeRasterizerState());
    DAWN_TRY(InitializeBlendState());
    DAWN_TRY(InitializeShaders());
    DAWN_TRY(InitializeDepthStencilState());

    // RTVs and UAVs share the same resoure slots. Make sure here we are not going to run out of
    // slots.
    uint32_t colorAttachments =
        static_cast<uint8_t>(GetHighestBitIndexPlusOne(GetColorAttachmentsMask()));
    uint32_t unusedUAVs = ToBackend(GetLayout())->GetUnusedUAVBindingCount();
    uint32_t usedUAVs = ToBackend(GetLayout())->GetTotalUAVBindingCount() - unusedUAVs;
    // TODO(dawn:1814): Move the validation to the frontend, if we eventually regard it as a compat
    // restriction.
    DAWN_INVALID_IF(colorAttachments > unusedUAVs,
                    "The pipeline uses up to color attachment %u, but there are only %u remaining "
                    "slots because the pipeline uses %u UAVs",
                    colorAttachments, unusedUAVs, usedUAVs);

    SetLabelImpl();
    return {};
}

RenderPipeline::~RenderPipeline() = default;

void RenderPipeline::ApplyNow(const ScopedSwapStateCommandRecordingContext* commandContext,
                              const std::array<float, 4>& blendColor,
                              uint32_t stencilReference) {
    auto* d3d11DeviceContext = commandContext->GetD3D11DeviceContext3();
    d3d11DeviceContext->IASetPrimitiveTopology(mD3DPrimitiveTopology);
    // TODO(dawn:1753): deduplicate these objects in the backend eventually, and to avoid redundant
    // state setting.
    d3d11DeviceContext->IASetInputLayout(mInputLayout.Get());
    d3d11DeviceContext->RSSetState(mRasterizerState.Get());
    d3d11DeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
    d3d11DeviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

    ApplyBlendState(commandContext, blendColor);
    ApplyDepthStencilState(commandContext, stencilReference);
}

void RenderPipeline::ApplyBlendState(const ScopedSwapStateCommandRecordingContext* commandContext,
                                     const std::array<float, 4>& blendColor) {
    auto* d3d11DeviceContext = commandContext->GetD3D11DeviceContext3();
    d3d11DeviceContext->OMSetBlendState(mBlendState.Get(), blendColor.data(), GetSampleMask());
}

void RenderPipeline::ApplyDepthStencilState(
    const ScopedSwapStateCommandRecordingContext* commandContext,
    uint32_t stencilReference) {
    auto* d3d11DeviceContext = commandContext->GetD3D11DeviceContext3();
    d3d11DeviceContext->OMSetDepthStencilState(mDepthStencilState.Get(), stencilReference);
}

void RenderPipeline::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mRasterizerState.Get(), "Dawn_RenderPipeline", GetLabel());
    SetDebugName(ToBackend(GetDevice()), mInputLayout.Get(), "Dawn_RenderPipeline", GetLabel());
    SetDebugName(ToBackend(GetDevice()), mVertexShader.Get(), "Dawn_RenderPipeline", GetLabel());
    SetDebugName(ToBackend(GetDevice()), mPixelShader.Get(), "Dawn_RenderPipeline", GetLabel());
    SetDebugName(ToBackend(GetDevice()), mBlendState.Get(), "Dawn_RenderPipeline", GetLabel());
    SetDebugName(ToBackend(GetDevice()), mDepthStencilState.Get(), "Dawn_RenderPipeline",
                 GetLabel());
}

MaybeError RenderPipeline::InitializeRasterizerState() {
    Device* device = ToBackend(GetDevice());

    D3D11_RASTERIZER_DESC rasterizerDesc;
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3DCullMode(GetCullMode());
    rasterizerDesc.FrontCounterClockwise = (GetFrontFace() == wgpu::FrontFace::CCW) ? TRUE : FALSE;
    rasterizerDesc.DepthBias = GetDepthBias();
    rasterizerDesc.DepthBiasClamp = GetDepthBiasClamp();
    rasterizerDesc.SlopeScaledDepthBias = GetDepthBiasSlopeScale();
    rasterizerDesc.DepthClipEnable = !HasUnclippedDepth();
    rasterizerDesc.ScissorEnable = TRUE;
    rasterizerDesc.MultisampleEnable = (GetSampleCount() > 1) ? TRUE : FALSE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;

    DAWN_TRY(CheckHRESULT(
        device->GetD3D11Device()->CreateRasterizerState(&rasterizerDesc, &mRasterizerState),
        "ID3D11Device::CreateRasterizerState"));

    return {};
}

MaybeError RenderPipeline::InitializeInputLayout(const Blob& vertexShader) {
    if (!GetAttributeLocationsUsed().any()) {
        return {};
    }

    std::array<D3D11_INPUT_ELEMENT_DESC, kMaxVertexAttributes> inputElementDescriptors;
    UINT count = 0;
    for (VertexAttributeLocation loc : IterateBitSet(GetAttributeLocationsUsed())) {
        D3D11_INPUT_ELEMENT_DESC& inputElementDescriptor = inputElementDescriptors[count++];

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
        if (inputElementDescriptor.InputSlotClass == D3D11_INPUT_PER_VERTEX_DATA) {
            inputElementDescriptor.InstanceDataStepRate = 0;
        } else {
            inputElementDescriptor.InstanceDataStepRate = 1;
        }
    }

    ID3D11Device* d3d11Device = ToBackend(GetDevice())->GetD3D11Device();

    DAWN_TRY(CheckHRESULT(
        d3d11Device->CreateInputLayout(inputElementDescriptors.data(), count, vertexShader.Data(),
                                       vertexShader.Size(), &mInputLayout),
        "ID3D11Device::CreateInputLayout"));

    return {};
}

MaybeError RenderPipeline::InitializeBlendState() {
    Device* device = ToBackend(GetDevice());

    CD3D11_BLEND_DESC blendDesc(D3D11_DEFAULT);
    blendDesc.AlphaToCoverageEnable = IsAlphaToCoverageEnabled();
    blendDesc.IndependentBlendEnable = TRUE;

    static_assert(kMaxColorAttachments == std::size(blendDesc.RenderTarget));
    for (auto i : Range(kMaxColorAttachmentsTyped)) {
        D3D11_RENDER_TARGET_BLEND_DESC& rtBlendDesc =
            blendDesc.RenderTarget[static_cast<uint8_t>(i)];
        const ColorTargetState* descriptor = GetColorTargetState(i);
        rtBlendDesc.BlendEnable = descriptor->blend != nullptr;
        if (rtBlendDesc.BlendEnable) {
            rtBlendDesc.SrcBlend = D3DBlendFactor(descriptor->blend->color.srcFactor);
            if (device->GetValidInternalFormat(descriptor->format).componentCount < 4 &&
                rtBlendDesc.SrcBlend == D3D11_BLEND_DEST_ALPHA) {
                // According to the D3D SPEC, the default value for missing components in an element
                // format is "0" for any component except A, which gets "1". So here
                // D3D11_BLEND_DEST_ALPHA should have same effect as D3D11_BLEND_ONE.
                // Note that this replacement can be an optimization as using D3D11_BLEND_ONE means
                // the GPU hardware no longer needs to get pixels from the destination texture. It
                // can also be served as a workaround against an Intel driver issue about alpha
                // blending (see http://crbug.com/dawn/1579 for more details).
                rtBlendDesc.SrcBlend = D3D11_BLEND_ONE;
            }
            rtBlendDesc.DestBlend = D3DBlendFactor(descriptor->blend->color.dstFactor);
            rtBlendDesc.BlendOp = D3DBlendOperation(descriptor->blend->color.operation);
            rtBlendDesc.SrcBlendAlpha = D3DBlendAlphaFactor(descriptor->blend->alpha.srcFactor);
            rtBlendDesc.DestBlendAlpha = D3DBlendAlphaFactor(descriptor->blend->alpha.dstFactor);
            rtBlendDesc.BlendOpAlpha = D3DBlendOperation(descriptor->blend->alpha.operation);
        }
        rtBlendDesc.RenderTargetWriteMask = D3DColorWriteMask(descriptor->writeMask);
    }

    DAWN_TRY(CheckHRESULT(device->GetD3D11Device()->CreateBlendState(&blendDesc, &mBlendState),
                          "ID3D11Device::CreateBlendState"));
    return {};
}

MaybeError RenderPipeline::InitializeDepthStencilState() {
    Device* device = ToBackend(GetDevice());
    const DepthStencilState* state = GetDepthStencilState();

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = (state->depthCompare == wgpu::CompareFunction::Always &&
                                    state->depthWriteEnabled != wgpu::OptionalBool::True)
                                       ? FALSE
                                       : TRUE;
    depthStencilDesc.DepthWriteMask = state->depthWriteEnabled == wgpu::OptionalBool::True
                                          ? D3D11_DEPTH_WRITE_MASK_ALL
                                          : D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStencilDesc.DepthFunc = ToD3D11ComparisonFunc(state->depthCompare);

    depthStencilDesc.StencilEnable = UsesStencil() ? TRUE : FALSE;
    depthStencilDesc.StencilReadMask = static_cast<UINT8>(state->stencilReadMask);
    depthStencilDesc.StencilWriteMask = static_cast<UINT8>(state->stencilWriteMask);

    depthStencilDesc.FrontFace = StencilOpDesc(state->stencilFront);
    depthStencilDesc.BackFace = StencilOpDesc(state->stencilBack);

    DAWN_TRY(CheckHRESULT(
        device->GetD3D11Device()->CreateDepthStencilState(&depthStencilDesc, &mDepthStencilState),
        "ID3D11Device::CreateDepthStencilState"));
    return {};
}

MaybeError RenderPipeline::InitializeShaders() {
    Device* device = ToBackend(GetDevice());
    uint32_t compileFlags = 0;

    if (!device->IsToggleEnabled(Toggle::UseDXC) &&
        !device->IsToggleEnabled(Toggle::FxcOptimizations)) {
        compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL0;
    }

    if (device->IsToggleEnabled(Toggle::EmitHLSLDebugSymbols)) {
        compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    }

    // Tint does matrix multiplication expecting row major matrices
    compileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

    PerStage<d3d::CompiledShader> compiledShader;

    std::optional<dawn::native::d3d::InterStageShaderVariablesMask> usedInterstageVariables;
    dawn::native::EntryPointMetadata fragmentEntryPoint;
    if (GetStageMask() & wgpu::ShaderStage::Fragment) {
        // Now that only fragment shader can have inter-stage inputs.
        const ProgrammableStage& programmableStage = GetStage(SingleShaderStage::Fragment);
        fragmentEntryPoint = programmableStage.module->GetEntryPoint(programmableStage.entryPoint);
        usedInterstageVariables = dawn::native::d3d::ToInterStageShaderVariablesMask(
            fragmentEntryPoint.usedInterStageVariables);
    }

    if (GetStageMask() & wgpu::ShaderStage::Vertex) {
        const ProgrammableStage& programmableStage = GetStage(SingleShaderStage::Vertex);
        uint32_t additionalCompileFlags = 0;
        if (programmableStage.module->GetStrictMath().value_or(
                !device->IsToggleEnabled(Toggle::D3DDisableIEEEStrictness))) {
            additionalCompileFlags |= D3DCOMPILE_IEEE_STRICTNESS;
        }

        DAWN_TRY_ASSIGN(
            compiledShader[SingleShaderStage::Vertex],
            ToBackend(programmableStage.module)
                ->Compile(programmableStage, SingleShaderStage::Vertex, ToBackend(GetLayout()),
                          compileFlags | additionalCompileFlags, usedInterstageVariables));
        const Blob& shaderBlob = compiledShader[SingleShaderStage::Vertex].shaderBlob;
        DAWN_TRY(CheckHRESULT(device->GetD3D11Device()->CreateVertexShader(
                                  shaderBlob.Data(), shaderBlob.Size(), nullptr, &mVertexShader),
                              "D3D11 create vertex shader"));
        DAWN_TRY(InitializeInputLayout(shaderBlob));
        mUsesVertexIndex = compiledShader[SingleShaderStage::Vertex].usesVertexIndex;
        mUsesInstanceIndex = compiledShader[SingleShaderStage::Vertex].usesInstanceIndex;
    }

    std::optional<tint::hlsl::writer::PixelLocalOptions> pixelLocalOptions;
    if (GetStageMask() & wgpu::ShaderStage::Fragment) {
        pixelLocalOptions = tint::hlsl::writer::PixelLocalOptions();
        // HLSL SM5.0 doesn't support groups, so we set group index to 0.
        pixelLocalOptions->group_index = 0;

        if (GetAttachmentState()->HasPixelLocalStorage()) {
            const std::vector<wgpu::TextureFormat>& storageAttachmentSlots =
                GetAttachmentState()->GetStorageAttachmentSlots();
            DAWN_ASSERT(ToBackend(GetLayout())->GetTotalUAVBindingCount() >
                        storageAttachmentSlots.size());
            // Currently all the pixel local storage UAVs are allocated at the last several UAV
            // slots. For example, when there are 4 pixel local storage attachments, we will
            // allocate register u60 to u63 for them.
            uint32_t basePixelLocalAttachmentIndex =
                ToBackend(GetLayout())->GetTotalUAVBindingCount() -
                static_cast<uint32_t>(storageAttachmentSlots.size());
            for (size_t i = 0; i < storageAttachmentSlots.size(); i++) {
                auto& attachment = pixelLocalOptions->attachments[i];
                attachment.index = basePixelLocalAttachmentIndex + i;

                static_assert(
                    RenderPipelineBase::kImplicitPLSSlotFormat == wgpu::TextureFormat::R32Uint,
                    "The implicit Pixel Local Storage format should be R32Uint.");
                switch (storageAttachmentSlots[i]) {
                        // We use R32Uint as default pixel local storage attachment format
                    case wgpu::TextureFormat::Undefined:
                    case wgpu::TextureFormat::R32Uint:
                        attachment.format =
                            tint::hlsl::writer::PixelLocalAttachment::TexelFormat::kR32Uint;
                        break;
                    case wgpu::TextureFormat::R32Sint:
                        attachment.format =
                            tint::hlsl::writer::PixelLocalAttachment::TexelFormat::kR32Sint;
                        break;
                    case wgpu::TextureFormat::R32Float:
                        attachment.format =
                            tint::hlsl::writer::PixelLocalAttachment::TexelFormat::kR32Float;
                        break;
                    default:
                        DAWN_UNREACHABLE();
                        break;
                }
            }
        }

        const ProgrammableStage& programmableStage = GetStage(SingleShaderStage::Fragment);
        uint32_t additionalCompileFlags = 0;
        if (programmableStage.module->GetStrictMath().value_or(
                !device->IsToggleEnabled(Toggle::D3DDisableIEEEStrictness))) {
            additionalCompileFlags |= D3DCOMPILE_IEEE_STRICTNESS;
        }

        DAWN_TRY_ASSIGN(compiledShader[SingleShaderStage::Fragment],
                        ToBackend(programmableStage.module)
                            ->Compile(programmableStage, SingleShaderStage::Fragment,
                                      ToBackend(GetLayout()), compileFlags | additionalCompileFlags,
                                      usedInterstageVariables, pixelLocalOptions));
        DAWN_TRY(CheckHRESULT(device->GetD3D11Device()->CreatePixelShader(
                                  compiledShader[SingleShaderStage::Fragment].shaderBlob.Data(),
                                  compiledShader[SingleShaderStage::Fragment].shaderBlob.Size(),
                                  nullptr, &mPixelShader),
                              "D3D11 create pixel shader"));
    }

    return {};
}

}  // namespace dawn::native::d3d11
