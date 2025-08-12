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

#include "dawn/native/metal/RenderPipelineMTL.h"

#include "dawn/native/Adapter.h"
#include "dawn/native/CreatePipelineAsyncEvent.h"
#include "dawn/native/Instance.h"
#include "dawn/native/metal/BackendMTL.h"
#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/native/metal/PipelineLayoutMTL.h"
#include "dawn/native/metal/ShaderModuleMTL.h"
#include "dawn/native/metal/TextureMTL.h"
#include "dawn/native/metal/UtilsMetal.h"
#include "dawn/platform/metrics/HistogramMacros.h"

namespace dawn::native::metal {

namespace {
MTLVertexFormat VertexFormatType(wgpu::VertexFormat format) {
    switch (format) {
        case wgpu::VertexFormat::Uint8:
            return MTLVertexFormatUChar;
        case wgpu::VertexFormat::Uint8x2:
            return MTLVertexFormatUChar2;
        case wgpu::VertexFormat::Uint8x4:
            return MTLVertexFormatUChar4;
        case wgpu::VertexFormat::Sint8:
            return MTLVertexFormatChar;
        case wgpu::VertexFormat::Sint8x2:
            return MTLVertexFormatChar2;
        case wgpu::VertexFormat::Sint8x4:
            return MTLVertexFormatChar4;
        case wgpu::VertexFormat::Unorm8:
            return MTLVertexFormatUCharNormalized;
        case wgpu::VertexFormat::Unorm8x2:
            return MTLVertexFormatUChar2Normalized;
        case wgpu::VertexFormat::Unorm8x4:
            return MTLVertexFormatUChar4Normalized;
        case wgpu::VertexFormat::Snorm8:
            return MTLVertexFormatCharNormalized;
        case wgpu::VertexFormat::Snorm8x2:
            return MTLVertexFormatChar2Normalized;
        case wgpu::VertexFormat::Snorm8x4:
            return MTLVertexFormatChar4Normalized;
        case wgpu::VertexFormat::Uint16:
            return MTLVertexFormatUShort;
        case wgpu::VertexFormat::Uint16x2:
            return MTLVertexFormatUShort2;
        case wgpu::VertexFormat::Uint16x4:
            return MTLVertexFormatUShort4;
        case wgpu::VertexFormat::Sint16:
            return MTLVertexFormatShort;
        case wgpu::VertexFormat::Sint16x2:
            return MTLVertexFormatShort2;
        case wgpu::VertexFormat::Sint16x4:
            return MTLVertexFormatShort4;
        case wgpu::VertexFormat::Unorm16:
            return MTLVertexFormatUShortNormalized;
        case wgpu::VertexFormat::Unorm16x2:
            return MTLVertexFormatUShort2Normalized;
        case wgpu::VertexFormat::Unorm16x4:
            return MTLVertexFormatUShort4Normalized;
        case wgpu::VertexFormat::Snorm16:
            return MTLVertexFormatShortNormalized;
        case wgpu::VertexFormat::Snorm16x2:
            return MTLVertexFormatShort2Normalized;
        case wgpu::VertexFormat::Snorm16x4:
            return MTLVertexFormatShort4Normalized;
        case wgpu::VertexFormat::Float16:
            return MTLVertexFormatHalf;
        case wgpu::VertexFormat::Float16x2:
            return MTLVertexFormatHalf2;
        case wgpu::VertexFormat::Float16x4:
            return MTLVertexFormatHalf4;
        case wgpu::VertexFormat::Float32:
            return MTLVertexFormatFloat;
        case wgpu::VertexFormat::Float32x2:
            return MTLVertexFormatFloat2;
        case wgpu::VertexFormat::Float32x3:
            return MTLVertexFormatFloat3;
        case wgpu::VertexFormat::Float32x4:
            return MTLVertexFormatFloat4;
        case wgpu::VertexFormat::Uint32:
            return MTLVertexFormatUInt;
        case wgpu::VertexFormat::Uint32x2:
            return MTLVertexFormatUInt2;
        case wgpu::VertexFormat::Uint32x3:
            return MTLVertexFormatUInt3;
        case wgpu::VertexFormat::Uint32x4:
            return MTLVertexFormatUInt4;
        case wgpu::VertexFormat::Sint32:
            return MTLVertexFormatInt;
        case wgpu::VertexFormat::Sint32x2:
            return MTLVertexFormatInt2;
        case wgpu::VertexFormat::Sint32x3:
            return MTLVertexFormatInt3;
        case wgpu::VertexFormat::Sint32x4:
            return MTLVertexFormatInt4;
        case wgpu::VertexFormat::Unorm10_10_10_2:
            return MTLVertexFormatUInt1010102Normalized;
        case wgpu::VertexFormat::Unorm8x4BGRA:
            return MTLVertexFormatUChar4Normalized_BGRA;
        default:
            DAWN_UNREACHABLE();
    }
}

MTLVertexStepFunction VertexStepModeFunction(wgpu::VertexStepMode mode) {
    switch (mode) {
        case wgpu::VertexStepMode::Vertex:
            return MTLVertexStepFunctionPerVertex;
        case wgpu::VertexStepMode::Instance:
            return MTLVertexStepFunctionPerInstance;
        case wgpu::VertexStepMode::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

MTLPrimitiveType MTLPrimitiveTopology(wgpu::PrimitiveTopology primitiveTopology) {
    switch (primitiveTopology) {
        case wgpu::PrimitiveTopology::PointList:
            return MTLPrimitiveTypePoint;
        case wgpu::PrimitiveTopology::LineList:
            return MTLPrimitiveTypeLine;
        case wgpu::PrimitiveTopology::LineStrip:
            return MTLPrimitiveTypeLineStrip;
        case wgpu::PrimitiveTopology::TriangleList:
            return MTLPrimitiveTypeTriangle;
        case wgpu::PrimitiveTopology::TriangleStrip:
            return MTLPrimitiveTypeTriangleStrip;
        case wgpu::PrimitiveTopology::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

MTLPrimitiveTopologyClass MTLInputPrimitiveTopology(wgpu::PrimitiveTopology primitiveTopology) {
    switch (primitiveTopology) {
        case wgpu::PrimitiveTopology::PointList:
            return MTLPrimitiveTopologyClassPoint;
        case wgpu::PrimitiveTopology::LineList:
        case wgpu::PrimitiveTopology::LineStrip:
            return MTLPrimitiveTopologyClassLine;
        case wgpu::PrimitiveTopology::TriangleList:
        case wgpu::PrimitiveTopology::TriangleStrip:
            return MTLPrimitiveTopologyClassTriangle;
        case wgpu::PrimitiveTopology::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

MTLBlendFactor MetalBlendFactor(wgpu::BlendFactor factor, bool alpha) {
    switch (factor) {
        case wgpu::BlendFactor::Zero:
            return MTLBlendFactorZero;
        case wgpu::BlendFactor::One:
            return MTLBlendFactorOne;
        case wgpu::BlendFactor::Src:
            return MTLBlendFactorSourceColor;
        case wgpu::BlendFactor::OneMinusSrc:
            return MTLBlendFactorOneMinusSourceColor;
        case wgpu::BlendFactor::SrcAlpha:
            return MTLBlendFactorSourceAlpha;
        case wgpu::BlendFactor::OneMinusSrcAlpha:
            return MTLBlendFactorOneMinusSourceAlpha;
        case wgpu::BlendFactor::Dst:
            return MTLBlendFactorDestinationColor;
        case wgpu::BlendFactor::OneMinusDst:
            return MTLBlendFactorOneMinusDestinationColor;
        case wgpu::BlendFactor::DstAlpha:
            return MTLBlendFactorDestinationAlpha;
        case wgpu::BlendFactor::OneMinusDstAlpha:
            return MTLBlendFactorOneMinusDestinationAlpha;
        case wgpu::BlendFactor::SrcAlphaSaturated:
            return MTLBlendFactorSourceAlphaSaturated;
        case wgpu::BlendFactor::Constant:
            return alpha ? MTLBlendFactorBlendAlpha : MTLBlendFactorBlendColor;
        case wgpu::BlendFactor::OneMinusConstant:
            return alpha ? MTLBlendFactorOneMinusBlendAlpha : MTLBlendFactorOneMinusBlendColor;
        case wgpu::BlendFactor::Src1:
            return MTLBlendFactorSource1Color;
        case wgpu::BlendFactor::OneMinusSrc1:
            return MTLBlendFactorOneMinusSource1Color;
        case wgpu::BlendFactor::Src1Alpha:
            return MTLBlendFactorSource1Alpha;
        case wgpu::BlendFactor::OneMinusSrc1Alpha:
            return MTLBlendFactorOneMinusSource1Alpha;
        case wgpu::BlendFactor::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

MTLBlendOperation MetalBlendOperation(wgpu::BlendOperation operation) {
    switch (operation) {
        case wgpu::BlendOperation::Add:
            return MTLBlendOperationAdd;
        case wgpu::BlendOperation::Subtract:
            return MTLBlendOperationSubtract;
        case wgpu::BlendOperation::ReverseSubtract:
            return MTLBlendOperationReverseSubtract;
        case wgpu::BlendOperation::Min:
            return MTLBlendOperationMin;
        case wgpu::BlendOperation::Max:
            return MTLBlendOperationMax;
        case wgpu::BlendOperation::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

MTLColorWriteMask MetalColorWriteMask(wgpu::ColorWriteMask writeMask,
                                      bool isDeclaredInFragmentShader) {
    if (!isDeclaredInFragmentShader) {
        return MTLColorWriteMaskNone;
    }

    MTLColorWriteMask mask = MTLColorWriteMaskNone;

    if (writeMask & wgpu::ColorWriteMask::Red) {
        mask |= MTLColorWriteMaskRed;
    }
    if (writeMask & wgpu::ColorWriteMask::Green) {
        mask |= MTLColorWriteMaskGreen;
    }
    if (writeMask & wgpu::ColorWriteMask::Blue) {
        mask |= MTLColorWriteMaskBlue;
    }
    if (writeMask & wgpu::ColorWriteMask::Alpha) {
        mask |= MTLColorWriteMaskAlpha;
    }

    return mask;
}

void ComputeBlendDesc(MTLRenderPipelineColorAttachmentDescriptor* attachment,
                      const ColorTargetState* state,
                      bool isDeclaredInFragmentShader) {
    attachment.blendingEnabled = state->blend != nullptr;
    if (attachment.blendingEnabled) {
        attachment.sourceRGBBlendFactor = MetalBlendFactor(state->blend->color.srcFactor, false);
        attachment.destinationRGBBlendFactor =
            MetalBlendFactor(state->blend->color.dstFactor, false);
        attachment.rgbBlendOperation = MetalBlendOperation(state->blend->color.operation);
        attachment.sourceAlphaBlendFactor = MetalBlendFactor(state->blend->alpha.srcFactor, true);
        attachment.destinationAlphaBlendFactor =
            MetalBlendFactor(state->blend->alpha.dstFactor, true);
        attachment.alphaBlendOperation = MetalBlendOperation(state->blend->alpha.operation);
    }
    attachment.writeMask = MetalColorWriteMask(state->writeMask, isDeclaredInFragmentShader);
}

MTLStencilOperation MetalStencilOperation(wgpu::StencilOperation stencilOperation) {
    switch (stencilOperation) {
        case wgpu::StencilOperation::Keep:
            return MTLStencilOperationKeep;
        case wgpu::StencilOperation::Zero:
            return MTLStencilOperationZero;
        case wgpu::StencilOperation::Replace:
            return MTLStencilOperationReplace;
        case wgpu::StencilOperation::Invert:
            return MTLStencilOperationInvert;
        case wgpu::StencilOperation::IncrementClamp:
            return MTLStencilOperationIncrementClamp;
        case wgpu::StencilOperation::DecrementClamp:
            return MTLStencilOperationDecrementClamp;
        case wgpu::StencilOperation::IncrementWrap:
            return MTLStencilOperationIncrementWrap;
        case wgpu::StencilOperation::DecrementWrap:
            return MTLStencilOperationDecrementWrap;
        case wgpu::StencilOperation::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

MTLWinding MTLFrontFace(wgpu::FrontFace face) {
    switch (face) {
        case wgpu::FrontFace::CW:
            return MTLWindingClockwise;
        case wgpu::FrontFace::CCW:
            return MTLWindingCounterClockwise;
        case wgpu::FrontFace::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

MTLCullMode ToMTLCullMode(wgpu::CullMode mode) {
    switch (mode) {
        case wgpu::CullMode::None:
            return MTLCullModeNone;
        case wgpu::CullMode::Front:
            return MTLCullModeFront;
        case wgpu::CullMode::Back:
            return MTLCullModeBack;
        case wgpu::CullMode::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

}  // anonymous namespace

// static
Ref<RenderPipelineBase> RenderPipeline::CreateUninitialized(
    Device* device,
    const UnpackedPtr<RenderPipelineDescriptor>& descriptor) {
    return AcquireRef(new RenderPipeline(device, descriptor));
}

RenderPipeline::RenderPipeline(DeviceBase* dev, const UnpackedPtr<RenderPipelineDescriptor>& desc)
    : RenderPipelineBase(dev, desc) {}

RenderPipeline::~RenderPipeline() = default;

MaybeError RenderPipeline::InitializeImpl() {
    mMtlPrimitiveTopology = MTLPrimitiveTopology(GetPrimitiveTopology());
    mMtlFrontFace = MTLFrontFace(GetFrontFace());
    mMtlCullMode = ToMTLCullMode(GetCullMode());
    // Build a mapping of vertex buffer slots to packed indices
    {
        // Vertex buffers are placed after all the buffers for the bind groups.
        uint32_t mtlVertexBufferIndex =
            ToBackend(GetLayout())->GetBufferBindingCount(SingleShaderStage::Vertex);

        for (VertexBufferSlot slot : GetVertexBuffersUsed()) {
            mMtlVertexBufferIndices[slot] = mtlVertexBufferIndex;
            mtlVertexBufferIndex++;
        }
    }

    auto mtlDevice = ToBackend(GetDevice())->GetMTLDevice();

    NSRef<MTLRenderPipelineDescriptor> descriptorMTLRef =
        AcquireNSRef([MTLRenderPipelineDescriptor new]);
    MTLRenderPipelineDescriptor* descriptorMTL = descriptorMTLRef.Get();

    NSRef<NSString> label = MakeDebugName(GetDevice(), "Dawn_RenderPipeline", GetLabel());
    descriptorMTL.label = label.Get();

    // Only put this flag on if the feature is enabled because it may have a performance cost.
    descriptorMTL.supportIndirectCommandBuffers =
        GetDevice()->HasFeature(Feature::MultiDrawIndirect);

    NSRef<MTLVertexDescriptor> vertexDesc;
    if (GetDevice()->IsToggleEnabled(Toggle::MetalEnableVertexPulling)) {
        vertexDesc = AcquireNSRef([MTLVertexDescriptor new]);
    } else {
        vertexDesc = MakeVertexDesc();
    }
    descriptorMTL.vertexDescriptor = vertexDesc.Get();

    const PerStage<ProgrammableStage>& allStages = GetAllStages();
    const ProgrammableStage& vertexStage = allStages[wgpu::ShaderStage::Vertex];
    ShaderModule::MetalFunctionData vertexData;
    DAWN_TRY_CONTEXT(ToBackend(vertexStage.module.Get())
                         ->CreateFunction(SingleShaderStage::Vertex, vertexStage,
                                          ToBackend(GetLayout()), &vertexData, 0xFFFFFFFF, this),
                     " getting vertex MTLFunction for %s", this);

    descriptorMTL.vertexFunction = vertexData.function.Get();
    if (vertexData.needsStorageBufferLength) {
        mStagesRequiringStorageBufferLength |= wgpu::ShaderStage::Vertex;
    }

    ShaderModule::MetalFunctionData fragmentData;
    if (GetStageMask() & wgpu::ShaderStage::Fragment) {
        const ProgrammableStage& fragmentStage = allStages[wgpu::ShaderStage::Fragment];
        DAWN_TRY_CONTEXT(
            ToBackend(fragmentStage.module.Get())
                ->CreateFunction(SingleShaderStage::Fragment, fragmentStage, ToBackend(GetLayout()),
                                 &fragmentData, GetSampleMask(), this),
            " getting fragment MTLFunction for %s", this);

        descriptorMTL.fragmentFunction = fragmentData.function.Get();
        if (fragmentData.needsStorageBufferLength) {
            mStagesRequiringStorageBufferLength |= wgpu::ShaderStage::Fragment;
        }

        const auto& fragmentOutputMask = fragmentStage.metadata->fragmentOutputMask;
        for (auto i : GetColorAttachmentsMask()) {
            descriptorMTL.colorAttachments[static_cast<uint8_t>(i)].pixelFormat =
                MetalPixelFormat(GetDevice(), GetColorAttachmentFormat(i));
            const ColorTargetState* descriptor = GetColorTargetState(i);
            ComputeBlendDesc(descriptorMTL.colorAttachments[static_cast<uint8_t>(i)], descriptor,
                             fragmentOutputMask[i]);
        }

        if (GetAttachmentState()->HasPixelLocalStorage()) {
            const std::vector<wgpu::TextureFormat>& storageAttachmentSlots =
                GetAttachmentState()->GetStorageAttachmentSlots();
            std::vector<ColorAttachmentIndex> storageAttachmentPacking =
                GetAttachmentState()->ComputeStorageAttachmentPackingInColorAttachments();

            for (size_t i = 0; i < storageAttachmentSlots.size(); i++) {
                uint8_t index = static_cast<uint8_t>(storageAttachmentPacking[i]);

                if (storageAttachmentSlots[i] == wgpu::TextureFormat::Undefined) {
                    descriptorMTL.colorAttachments[index].pixelFormat =
                        MetalPixelFormat(GetDevice(), kImplicitPLSSlotFormat);
                } else {
                    descriptorMTL.colorAttachments[index].pixelFormat =
                        MetalPixelFormat(GetDevice(), storageAttachmentSlots[i]);
                }
            }
        }
    }

    if (HasDepthStencilAttachment()) {
        wgpu::TextureFormat depthStencilFormat = GetDepthStencilFormat();
        MTLPixelFormat metalFormat = MetalPixelFormat(GetDevice(), depthStencilFormat);

        if (GetDevice()->IsToggleEnabled(
                Toggle::MetalUseBothDepthAndStencilAttachmentsForCombinedDepthStencilFormats)) {
            if (GetDepthStencilAspects(metalFormat) & Aspect::Depth) {
                descriptorMTL.depthAttachmentPixelFormat = metalFormat;
            }
            if (GetDepthStencilAspects(metalFormat) & Aspect::Stencil) {
                descriptorMTL.stencilAttachmentPixelFormat = metalFormat;
            }
        } else {
            const Format& internalFormat = GetDevice()->GetValidInternalFormat(depthStencilFormat);
            if (internalFormat.HasDepth()) {
                descriptorMTL.depthAttachmentPixelFormat = metalFormat;
            }
            if (internalFormat.HasStencil()) {
                descriptorMTL.stencilAttachmentPixelFormat = metalFormat;
            }
        }
    }

    descriptorMTL.inputPrimitiveTopology = MTLInputPrimitiveTopology(GetPrimitiveTopology());
    descriptorMTL.rasterSampleCount = GetSampleCount();
    descriptorMTL.alphaToCoverageEnabled = IsAlphaToCoverageEnabled();

    platform::metrics::DawnHistogramTimer timer(GetDevice()->GetPlatform());
    NSError* error = nullptr;
    mMtlRenderPipelineState =
        AcquireNSPRef([mtlDevice newRenderPipelineStateWithDescriptor:descriptorMTL error:&error]);
    if (error != nullptr) {
        std::string errorMessage = absl::StrFormat(
            "RenderPipelineMTL: error creating pipeline state: %s from vertex MSL:\n\n%s",
            [error.localizedDescription UTF8String], vertexData.msl);
        if (GetStageMask() & wgpu::ShaderStage::Fragment) {
            absl::StrAppendFormat(&errorMessage, "\n\nand fragment MSL:\n\n%s", fragmentData.msl);
        }
        return DAWN_INTERNAL_ERROR(errorMessage);
    }
    DAWN_ASSERT(mMtlRenderPipelineState != nil);
    timer.RecordMicroseconds("Metal.newRenderPipelineStateWithDescriptor.CacheMiss");

    // Create depth stencil state and cache it, fetch the cached depth stencil state when we
    // call setDepthStencilState() for a given render pipeline in CommandEncoder, in order
    // to improve performance.
    NSRef<MTLDepthStencilDescriptor> depthStencilDesc = MakeDepthStencilDesc();
    mMtlDepthStencilState =
        AcquireNSPRef([mtlDevice newDepthStencilStateWithDescriptor:depthStencilDesc.Get()]);

    return {};
}

MTLPrimitiveType RenderPipeline::GetMTLPrimitiveTopology() const {
    return mMtlPrimitiveTopology;
}

MTLWinding RenderPipeline::GetMTLFrontFace() const {
    return mMtlFrontFace;
}

MTLCullMode RenderPipeline::GetMTLCullMode() const {
    return mMtlCullMode;
}

void RenderPipeline::Encode(id<MTLRenderCommandEncoder> encoder) {
    [encoder setRenderPipelineState:mMtlRenderPipelineState.Get()];
}

id<MTLDepthStencilState> RenderPipeline::GetMTLDepthStencilState() {
    return mMtlDepthStencilState.Get();
}

uint32_t RenderPipeline::GetMtlVertexBufferIndex(VertexBufferSlot slot) const {
    DAWN_ASSERT(slot < kMaxVertexBuffersTyped);
    return mMtlVertexBufferIndices[slot];
}

wgpu::ShaderStage RenderPipeline::GetStagesRequiringStorageBufferLength() const {
    return mStagesRequiringStorageBufferLength;
}

NSRef<MTLVertexDescriptor> RenderPipeline::MakeVertexDesc() const {
    MTLVertexDescriptor* mtlVertexDescriptor = [MTLVertexDescriptor new];

    for (VertexBufferSlot slot : GetVertexBuffersUsed()) {
        const VertexBufferInfo& info = GetVertexBuffer(slot);

        MTLVertexBufferLayoutDescriptor* layoutDesc = [MTLVertexBufferLayoutDescriptor new];
        if (info.arrayStride == 0) {
            // For MTLVertexStepFunctionConstant, the stepRate must be 0,
            // but the arrayStride must NOT be 0, so we made up it with
            // max(attrib.offset + sizeof(attrib) for each attrib)
            size_t maxArrayStride = 0;
            for (VertexAttributeLocation loc : GetAttributeLocationsUsed()) {
                const VertexAttributeInfo& attrib = GetAttribute(loc);
                // Only use the attributes that use the current input
                if (attrib.vertexBufferSlot != slot) {
                    continue;
                }
                maxArrayStride =
                    std::max(maxArrayStride,
                             GetVertexFormatInfo(attrib.format).byteSize + size_t(attrib.offset));
            }
            layoutDesc.stepFunction = MTLVertexStepFunctionConstant;
            layoutDesc.stepRate = 0;
            // Metal requires the stride must be a multiple of 4 bytes, align it with next
            // multiple of 4 if it's not.
            layoutDesc.stride = Align(maxArrayStride, 4);
        } else {
            layoutDesc.stepFunction = VertexStepModeFunction(info.stepMode);
            layoutDesc.stepRate = 1;
            layoutDesc.stride = info.arrayStride;
        }

        mtlVertexDescriptor.layouts[GetMtlVertexBufferIndex(slot)] = layoutDesc;
        [layoutDesc release];
    }

    for (VertexAttributeLocation loc : GetAttributeLocationsUsed()) {
        const VertexAttributeInfo& info = GetAttribute(loc);

        auto attribDesc = [MTLVertexAttributeDescriptor new];
        attribDesc.format = VertexFormatType(info.format);
        attribDesc.offset = info.offset;
        attribDesc.bufferIndex = GetMtlVertexBufferIndex(info.vertexBufferSlot);
        mtlVertexDescriptor.attributes[static_cast<uint8_t>(loc)] = attribDesc;
        [attribDesc release];
    }

    return AcquireNSRef(mtlVertexDescriptor);
}

NSRef<MTLDepthStencilDescriptor> RenderPipeline::MakeDepthStencilDesc() {
    const DepthStencilState* descriptor = GetDepthStencilState();

    NSRef<MTLDepthStencilDescriptor> mtlDepthStencilDescRef =
        AcquireNSRef([MTLDepthStencilDescriptor new]);
    MTLDepthStencilDescriptor* mtlDepthStencilDescriptor = mtlDepthStencilDescRef.Get();

    mtlDepthStencilDescriptor.depthCompareFunction =
        ToMetalCompareFunction(descriptor->depthCompare);
    mtlDepthStencilDescriptor.depthWriteEnabled =
        descriptor->depthWriteEnabled == wgpu::OptionalBool::True;

    if (UsesStencil()) {
        NSRef<MTLStencilDescriptor> backFaceStencilRef = AcquireNSRef([MTLStencilDescriptor new]);
        MTLStencilDescriptor* backFaceStencil = backFaceStencilRef.Get();
        NSRef<MTLStencilDescriptor> frontFaceStencilRef = AcquireNSRef([MTLStencilDescriptor new]);
        MTLStencilDescriptor* frontFaceStencil = frontFaceStencilRef.Get();

        backFaceStencil.stencilCompareFunction =
            ToMetalCompareFunction(descriptor->stencilBack.compare);
        backFaceStencil.stencilFailureOperation =
            MetalStencilOperation(descriptor->stencilBack.failOp);
        backFaceStencil.depthFailureOperation =
            MetalStencilOperation(descriptor->stencilBack.depthFailOp);
        backFaceStencil.depthStencilPassOperation =
            MetalStencilOperation(descriptor->stencilBack.passOp);
        backFaceStencil.readMask = descriptor->stencilReadMask;
        backFaceStencil.writeMask = descriptor->stencilWriteMask;

        frontFaceStencil.stencilCompareFunction =
            ToMetalCompareFunction(descriptor->stencilFront.compare);
        frontFaceStencil.stencilFailureOperation =
            MetalStencilOperation(descriptor->stencilFront.failOp);
        frontFaceStencil.depthFailureOperation =
            MetalStencilOperation(descriptor->stencilFront.depthFailOp);
        frontFaceStencil.depthStencilPassOperation =
            MetalStencilOperation(descriptor->stencilFront.passOp);
        frontFaceStencil.readMask = descriptor->stencilReadMask;
        frontFaceStencil.writeMask = descriptor->stencilWriteMask;

        mtlDepthStencilDescriptor.backFaceStencil = backFaceStencil;
        mtlDepthStencilDescriptor.frontFaceStencil = frontFaceStencil;
    }

    return mtlDepthStencilDescRef;
}

}  // namespace dawn::native::metal
