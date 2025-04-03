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

#include "dawn/native/RenderPipeline.h"

#include <algorithm>
#include <cmath>

#include "dawn/common/BitSetIterator.h"
#include "dawn/common/Enumerator.h"
#include "dawn/common/ityp_array.h"
#include "dawn/common/ityp_bitset.h"
#include "dawn/common/ityp_span.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/InternalPipelineStore.h"
#include "dawn/native/ObjectContentHasher.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/ValidationUtils.h"
#include "dawn/native/ValidationUtils_autogen.h"

namespace dawn::native {

static constexpr ityp::array<wgpu::VertexFormat, VertexFormatInfo, 42> sVertexFormatTable =
    []() constexpr {
        ityp::array<wgpu::VertexFormat, VertexFormatInfo, 42> table{};

        // clang-format off
        table[wgpu::VertexFormat::Uint8          ] = { 1, 1, VertexFormatBaseType::Uint };
        table[wgpu::VertexFormat::Uint8x2        ] = { 2, 2, VertexFormatBaseType::Uint };
        table[wgpu::VertexFormat::Uint8x4        ] = { 4, 4, VertexFormatBaseType::Uint };
        table[wgpu::VertexFormat::Sint8          ] = { 1, 1, VertexFormatBaseType::Sint };
        table[wgpu::VertexFormat::Sint8x2        ] = { 2, 2, VertexFormatBaseType::Sint };
        table[wgpu::VertexFormat::Sint8x4        ] = { 4, 4, VertexFormatBaseType::Sint };
        table[wgpu::VertexFormat::Unorm8         ] = { 1, 1, VertexFormatBaseType::Float};
        table[wgpu::VertexFormat::Unorm8x2       ] = { 2, 2, VertexFormatBaseType::Float};
        table[wgpu::VertexFormat::Unorm8x4       ] = { 4, 4, VertexFormatBaseType::Float};
        table[wgpu::VertexFormat::Snorm8         ] = { 1, 1, VertexFormatBaseType::Float};
        table[wgpu::VertexFormat::Snorm8x2       ] = { 2, 2, VertexFormatBaseType::Float};
        table[wgpu::VertexFormat::Snorm8x4       ] = { 4, 4, VertexFormatBaseType::Float};

        table[wgpu::VertexFormat::Uint16         ] = { 2, 1, VertexFormatBaseType::Uint };
        table[wgpu::VertexFormat::Uint16x2       ] = { 4, 2, VertexFormatBaseType::Uint };
        table[wgpu::VertexFormat::Uint16x4       ] = { 8, 4, VertexFormatBaseType::Uint };
        table[wgpu::VertexFormat::Sint16         ] = { 2, 1, VertexFormatBaseType::Sint };
        table[wgpu::VertexFormat::Sint16x2       ] = { 4, 2, VertexFormatBaseType::Sint };
        table[wgpu::VertexFormat::Sint16x4       ] = { 8, 4, VertexFormatBaseType::Sint };
        table[wgpu::VertexFormat::Unorm16        ] = { 2, 1, VertexFormatBaseType::Float};
        table[wgpu::VertexFormat::Unorm16x2      ] = { 4, 2, VertexFormatBaseType::Float};
        table[wgpu::VertexFormat::Unorm16x4      ] = { 8, 4, VertexFormatBaseType::Float};
        table[wgpu::VertexFormat::Snorm16        ] = { 2, 1, VertexFormatBaseType::Float};
        table[wgpu::VertexFormat::Snorm16x2      ] = { 4, 2, VertexFormatBaseType::Float};
        table[wgpu::VertexFormat::Snorm16x4      ] = { 8, 4, VertexFormatBaseType::Float};
        table[wgpu::VertexFormat::Float16        ] = { 2, 1, VertexFormatBaseType::Float};
        table[wgpu::VertexFormat::Float16x2      ] = { 4, 2, VertexFormatBaseType::Float};
        table[wgpu::VertexFormat::Float16x4      ] = { 8, 4, VertexFormatBaseType::Float};

        table[wgpu::VertexFormat::Float32        ] = { 4, 1, VertexFormatBaseType::Float};
        table[wgpu::VertexFormat::Float32x2      ] = { 8, 2, VertexFormatBaseType::Float};
        table[wgpu::VertexFormat::Float32x3      ] = {12, 3, VertexFormatBaseType::Float};
        table[wgpu::VertexFormat::Float32x4      ] = {16, 4, VertexFormatBaseType::Float};
        table[wgpu::VertexFormat::Uint32         ] = { 4, 1, VertexFormatBaseType::Uint };
        table[wgpu::VertexFormat::Uint32x2       ] = { 8, 2, VertexFormatBaseType::Uint };
        table[wgpu::VertexFormat::Uint32x3       ] = {12, 3, VertexFormatBaseType::Uint };
        table[wgpu::VertexFormat::Uint32x4       ] = {16, 4, VertexFormatBaseType::Uint };
        table[wgpu::VertexFormat::Sint32         ] = { 4, 1, VertexFormatBaseType::Sint };
        table[wgpu::VertexFormat::Sint32x2       ] = { 8, 2, VertexFormatBaseType::Sint };
        table[wgpu::VertexFormat::Sint32x3       ] = {12, 3, VertexFormatBaseType::Sint };
        table[wgpu::VertexFormat::Sint32x4       ] = {16, 4, VertexFormatBaseType::Sint };
        table[wgpu::VertexFormat::Unorm10_10_10_2] = { 4, 4, VertexFormatBaseType::Float};
        table[wgpu::VertexFormat::Unorm8x4BGRA   ] = { 4, 4, VertexFormatBaseType::Float};
        // clang-format on

        return table;
    }();

const VertexFormatInfo& GetVertexFormatInfo(wgpu::VertexFormat format) {
    DAWN_ASSERT(static_cast<uint32_t>(format) < static_cast<uint32_t>(sVertexFormatTable.size()));
    DAWN_ASSERT(static_cast<uint32_t>(format) != 0u);
    return sVertexFormatTable[format];
}

// Helper functions
namespace {
MaybeError ValidateVertexAttribute(DeviceBase* device,
                                   const VertexAttribute* attribute,
                                   const EntryPointMetadata& metadata,
                                   uint64_t vertexBufferStride,
                                   VertexAttributeMask* attributesSetMask) {
    DAWN_TRY(ValidateVertexFormat(attribute->format));
    const VertexFormatInfo& formatInfo = GetVertexFormatInfo(attribute->format);

    uint32_t maxVertexAttributes = device->GetLimits().v1.maxVertexAttributes;
    DAWN_INVALID_IF(
        attribute->shaderLocation >= maxVertexAttributes,
        "Attribute shader location (%u) exceeds the maximum number of vertex attributes "
        "(%u).",
        attribute->shaderLocation, maxVertexAttributes);

    VertexAttributeLocation location(static_cast<uint8_t>(attribute->shaderLocation));

    // No underflow is possible because the max vertex format size is smaller than
    // kMaxVertexBufferArrayStride.
    DAWN_ASSERT(kMaxVertexBufferArrayStride >= formatInfo.byteSize);
    DAWN_INVALID_IF(attribute->offset > kMaxVertexBufferArrayStride - formatInfo.byteSize,
                    "Attribute offset (%u) + format size (%u for %s) must be <= the maximum vertex "
                    "buffer stride (%u). Offsets larger than the maximum vertex buffer stride are "
                    "accomodated by setting buffer offsets when calling setVertexBuffer, which the "
                    "attribute offset is added to.",
                    attribute->offset, formatInfo.byteSize, attribute->format,
                    kMaxVertexBufferArrayStride);

    // No overflow is possible because the offset is already validated to be less
    // than kMaxVertexBufferArrayStride.
    DAWN_ASSERT(attribute->offset < kMaxVertexBufferArrayStride);
    DAWN_INVALID_IF(
        vertexBufferStride > 0 && attribute->offset + formatInfo.byteSize > vertexBufferStride,
        "Attribute offset (%u) + format size (%u for %s) must be <= the vertex buffer stride (%u). "
        "Offsets larger than the vertex buffer stride are accomodated by setting buffer offsets "
        "when calling setVertexBuffer, which the attribute offset is added to.",
        attribute->offset, formatInfo.byteSize, attribute->format, vertexBufferStride);

    DAWN_INVALID_IF(attribute->offset % std::min(4u, formatInfo.byteSize) != 0,
                    "Attribute offset (%u) in not a multiple of %u.", attribute->offset,
                    std::min(4u, formatInfo.byteSize));

    DAWN_INVALID_IF(metadata.usedVertexInputs[location] &&
                        formatInfo.baseType != metadata.vertexInputBaseTypes[location],
                    "Attribute base type (%s for %s) does not match the shader's base type (%s) in "
                    "location (%u).",
                    formatInfo.baseType, attribute->format, metadata.vertexInputBaseTypes[location],
                    attribute->shaderLocation);

    DAWN_INVALID_IF((*attributesSetMask)[location],
                    "Attribute shader location (%u) is used more than once.",
                    attribute->shaderLocation);

    attributesSetMask->set(location);
    return {};
}

MaybeError ValidateVertexBufferLayout(DeviceBase* device,
                                      const VertexBufferLayout* buffer,
                                      const EntryPointMetadata& metadata,
                                      VertexAttributeMask* attributesSetMask) {
    DAWN_TRY(ValidateVertexStepMode(buffer->stepMode));
    DAWN_INVALID_IF(buffer->arrayStride > kMaxVertexBufferArrayStride,
                    "Vertex buffer arrayStride (%u) is larger than the maximum array stride (%u).",
                    buffer->arrayStride, kMaxVertexBufferArrayStride);

    DAWN_INVALID_IF(buffer->arrayStride % 4 != 0,
                    "Vertex buffer arrayStride (%u) is not a multiple of 4.", buffer->arrayStride);

    for (uint32_t i = 0; i < buffer->attributeCount; ++i) {
        DAWN_TRY_CONTEXT(ValidateVertexAttribute(device, &buffer->attributes[i], metadata,
                                                 buffer->arrayStride, attributesSetMask),
                         "validating attributes[%u].", i);
    }

    return {};
}

ResultOrError<ShaderModuleEntryPoint> ValidateVertexState(
    DeviceBase* device,
    const VertexState* descriptor,
    const PipelineLayoutBase* layout,
    wgpu::PrimitiveTopology primitiveTopology) {
    DAWN_INVALID_IF(descriptor->nextInChain != nullptr, "nextInChain must be nullptr.");

    const CombinedLimits& limits = device->GetLimits();

    const uint32_t maxVertexBuffers = limits.v1.maxVertexBuffers;
    DAWN_INVALID_IF(descriptor->bufferCount > maxVertexBuffers,
                    "Vertex buffer count (%u) exceeds the maximum number of vertex buffers (%u).%s",
                    descriptor->bufferCount, maxVertexBuffers,
                    DAWN_INCREASE_LIMIT_MESSAGE(device->GetAdapter()->GetLimits().v1,
                                                maxVertexBuffers, descriptor->bufferCount));

    ShaderModuleEntryPoint entryPoint;
    DAWN_TRY_ASSIGN_CONTEXT(
        entryPoint,
        ValidateProgrammableStage(device, descriptor->module, descriptor->entryPoint,
                                  descriptor->constantCount, descriptor->constants, layout,
                                  SingleShaderStage::Vertex),
        "validating vertex stage (%s, entryPoint: %s).", descriptor->module,
        descriptor->entryPoint);
    const EntryPointMetadata& vertexMetadata = descriptor->module->GetEntryPoint(entryPoint.name);
    if (primitiveTopology == wgpu::PrimitiveTopology::PointList) {
        DAWN_INVALID_IF(
            vertexMetadata.totalInterStageShaderVariables + 1 >
                limits.v1.maxInterStageShaderVariables,
            "Total vertex output variables count (%u) exceeds the maximum (%u) when primitive "
            "topology is %s as another variable is implicitly used for the point size.",
            vertexMetadata.totalInterStageShaderVariables,
            limits.v1.maxInterStageShaderVariables - 1, primitiveTopology);
    }

    VertexAttributeMask attributesSetMask;
    uint32_t totalAttributesNum = 0;
    for (uint32_t i = 0; i < descriptor->bufferCount; ++i) {
        DAWN_TRY_CONTEXT(ValidateVertexBufferLayout(device, &descriptor->buffers[i], vertexMetadata,
                                                    &attributesSetMask),
                         "validating buffers[%u].", i);
        totalAttributesNum += descriptor->buffers[i].attributeCount;
    }

    if (device->IsCompatibilityMode() &&
        (vertexMetadata.usesVertexIndex || vertexMetadata.usesInstanceIndex)) {
        uint32_t totalEffectiveAttributesNum = totalAttributesNum +
                                               (vertexMetadata.usesVertexIndex ? 1 : 0) +
                                               (vertexMetadata.usesInstanceIndex ? 1 : 0);
        DAWN_INVALID_IF(totalEffectiveAttributesNum > limits.v1.maxVertexAttributes,
                        "Attribute count (%u) exceeds the maximum number of attributes (%u) as "
                        "@builtin(vertex_index) and @builtin(instance_index) each use an attribute "
                        "in compatibility mode.",
                        totalEffectiveAttributesNum, limits.v1.maxVertexAttributes);
    }

    // Every vertex attribute has a member called shaderLocation, and there are some
    // requirements for shaderLocation: 1) >=0, 2) values are different across different
    // attributes, 3) can't exceed kMaxVertexAttributes. So it can ensure that total
    // attribute number never exceed kMaxVertexAttributes.
    DAWN_ASSERT(totalAttributesNum <= kMaxVertexAttributes);

    // Validate that attributes used by the VertexState are in the shader using bitmask operations
    // but try to be helpful by finding one missing attribute to surface in the error message
    if (!IsSubset(vertexMetadata.usedVertexInputs, attributesSetMask)) {
        const VertexAttributeMask missingAttributes =
            vertexMetadata.usedVertexInputs & ~attributesSetMask;
        DAWN_ASSERT(missingAttributes.any());

        VertexAttributeLocation firstMissing = ityp::Sub(
            GetHighestBitIndexPlusOne(missingAttributes), VertexAttributeLocation(uint8_t(1)));
        return DAWN_VALIDATION_ERROR(
            "Vertex attribute slot %u used in (%s, %s) is not present in the "
            "VertexState.",
            uint8_t(firstMissing), descriptor->module, &entryPoint);
    }

    return entryPoint;
}

MaybeError ValidatePrimitiveState(const DeviceBase* device, const PrimitiveState* rawDescriptor) {
    UnpackedPtr<PrimitiveState> descriptor;
    DAWN_TRY_ASSIGN(descriptor, ValidateAndUnpack(rawDescriptor));
    DAWN_INVALID_IF(descriptor->unclippedDepth && !device->HasFeature(Feature::DepthClipControl),
                    "%s is not supported", wgpu::FeatureName::DepthClipControl);
    DAWN_TRY(ValidatePrimitiveTopology(descriptor->topology));
    DAWN_TRY(ValidateIndexFormat(descriptor->stripIndexFormat));
    DAWN_TRY(ValidateFrontFace(descriptor->frontFace));
    DAWN_TRY(ValidateCullMode(descriptor->cullMode));

    // Pipeline descriptors must have stripIndexFormat == undefined if they are using
    // non-strip topologies.
    if (!IsStripPrimitiveTopology(descriptor->topology)) {
        DAWN_INVALID_IF(descriptor->stripIndexFormat != wgpu::IndexFormat::Undefined,
                        "StripIndexFormat (%s) is not undefined when using a non-strip primitive "
                        "topology (%s).",
                        descriptor->stripIndexFormat, descriptor->topology);
    }

    return {};
}

MaybeError ValidateStencilFaceUnused(StencilFaceState face) {
    DAWN_INVALID_IF((face.compare != wgpu::CompareFunction::Always) &&
                        (face.compare != wgpu::CompareFunction::Undefined),
                    "compare (%s) is defined and not %s.", face.compare,
                    wgpu::CompareFunction::Always);
    DAWN_INVALID_IF((face.failOp != wgpu::StencilOperation::Keep) &&
                        (face.failOp != wgpu::StencilOperation::Undefined),
                    "failOp (%s) is defined and not %s.", face.failOp,
                    wgpu::StencilOperation::Keep);
    DAWN_INVALID_IF((face.depthFailOp != wgpu::StencilOperation::Keep) &&
                        (face.depthFailOp != wgpu::StencilOperation::Undefined),
                    "depthFailOp (%s) is defined and not %s.", face.depthFailOp,
                    wgpu::StencilOperation::Keep);
    DAWN_INVALID_IF((face.passOp != wgpu::StencilOperation::Keep) &&
                        (face.passOp != wgpu::StencilOperation::Undefined),
                    "passOp (%s) is defined and not %s.", face.passOp,
                    wgpu::StencilOperation::Keep);
    return {};
}

MaybeError ValidateDepthStencilState(const DeviceBase* device,
                                     const DepthStencilState* descriptor,
                                     const wgpu::PrimitiveTopology topology) {
    DAWN_TRY_CONTEXT(ValidateCompareFunction(descriptor->depthCompare),
                     "validating depth compare function");
    DAWN_TRY_CONTEXT(ValidateCompareFunction(descriptor->stencilFront.compare),
                     "validating stencil front compare function");
    DAWN_TRY_CONTEXT(ValidateStencilOperation(descriptor->stencilFront.failOp),
                     "validating stencil front fail operation");
    DAWN_TRY_CONTEXT(ValidateStencilOperation(descriptor->stencilFront.depthFailOp),
                     "validating stencil front depth fail operation");
    DAWN_TRY_CONTEXT(ValidateStencilOperation(descriptor->stencilFront.passOp),
                     "validating stencil front pass operation");
    DAWN_TRY_CONTEXT(ValidateCompareFunction(descriptor->stencilBack.compare),
                     "validating stencil back compare function");
    DAWN_TRY_CONTEXT(ValidateStencilOperation(descriptor->stencilBack.failOp),
                     "validating stencil back fail operation");
    DAWN_TRY_CONTEXT(ValidateStencilOperation(descriptor->stencilBack.depthFailOp),
                     "validating stencil back depth fail operation");
    DAWN_TRY_CONTEXT(ValidateStencilOperation(descriptor->stencilBack.passOp),
                     "validating stencil back pass operation");

    const Format* format;
    DAWN_TRY_ASSIGN(format, device->GetInternalFormat(descriptor->format));
    DAWN_INVALID_IF(!format->HasDepthOrStencil() || !format->isRenderable,
                    "Depth stencil format (%s) is not depth-stencil renderable.",
                    descriptor->format);

    DAWN_TRY(ValidateFloat("depthBiasSlopeScale", descriptor->depthBiasSlopeScale));
    DAWN_TRY(ValidateFloat("depthBiasClamp", descriptor->depthBiasClamp));

    DAWN_INVALID_IF(device->IsCompatibilityMode() && descriptor->depthBiasClamp != 0.0f,
                    "depthBiasClamp (%f) is not zero as required in compatibility mode.",
                    descriptor->depthBiasClamp);

    DAWN_INVALID_IF(
        format->HasDepth() && descriptor->depthCompare == wgpu::CompareFunction::Undefined &&
            (descriptor->depthWriteEnabled == wgpu::OptionalBool::True ||
             descriptor->stencilFront.depthFailOp != wgpu::StencilOperation::Keep ||
             descriptor->stencilBack.depthFailOp != wgpu::StencilOperation::Keep),
        "Depth stencil format (%s) has a depth aspect and depthCompare is %s while it's actually "
        "used by depthWriteEnabled (%s), or stencil front depth fail operation (%s), or "
        "stencil back depth fail operation (%s).",
        descriptor->format, wgpu::CompareFunction::Undefined, descriptor->depthWriteEnabled,
        descriptor->stencilFront.depthFailOp, descriptor->stencilBack.depthFailOp);

    DAWN_INVALID_IF(
        format->HasDepth() && descriptor->depthWriteEnabled == wgpu::OptionalBool::Undefined,
        "Depth stencil format (%s) has a depth aspect and depthWriteEnabled is undefined.",
        descriptor->format);

    DAWN_INVALID_IF(
        !format->HasDepth() && descriptor->depthCompare != wgpu::CompareFunction::Always &&
            descriptor->depthCompare != wgpu::CompareFunction::Undefined,
        "Depth stencil format (%s) doesn't have depth aspect while depthCompare (%s) is "
        "neither %s nor %s.",
        descriptor->format, descriptor->depthCompare, wgpu::CompareFunction::Always,
        wgpu::CompareFunction::Undefined);

    DAWN_INVALID_IF(
        !format->HasDepth() && descriptor->depthWriteEnabled == wgpu::OptionalBool::True,
        "Depth stencil format (%s) doesn't have depth aspect while depthWriteEnabled (%s) "
        "is true.",
        descriptor->format, descriptor->depthWriteEnabled);

    if (!format->HasStencil()) {
        DAWN_TRY_CONTEXT(ValidateStencilFaceUnused(descriptor->stencilFront),
                         "validating that stencilFront doesn't use stencil when the depth-stencil "
                         "format (%s) doesn't have a stencil aspect.",
                         descriptor->format);
        DAWN_TRY_CONTEXT(ValidateStencilFaceUnused(descriptor->stencilBack),
                         "validating that stencilBack doesn't use stencil when the depth-stencil "
                         "format (%s) doesn't have a stencil aspect.",
                         descriptor->format);
    }

    switch (topology) {
        case wgpu::PrimitiveTopology::PointList:
        case wgpu::PrimitiveTopology::LineList:
        case wgpu::PrimitiveTopology::LineStrip:
            DAWN_INVALID_IF(descriptor->depthBias != 0, "depthBias must be 0 when using %s.",
                            topology);
            DAWN_INVALID_IF(descriptor->depthBiasSlopeScale != 0,
                            "depthBiasSlopeScale must be 0 when using %s.", topology);
            DAWN_INVALID_IF(descriptor->depthBiasClamp != 0,
                            "depthBiasClamp must be 0 when using using %s.", topology);
            break;
        case wgpu::PrimitiveTopology::Undefined:
            // Default is TriangleList.
        case wgpu::PrimitiveTopology::TriangleList:
        case wgpu::PrimitiveTopology::TriangleStrip:
            break;
    }

    return {};
}

MaybeError ValidateMultisampleState(const DeviceBase* device, const MultisampleState* descriptor) {
    DAWN_INVALID_IF(!IsValidSampleCount(descriptor->count),
                    "Multisample count (%u) is not supported.", descriptor->count);

    DAWN_INVALID_IF(descriptor->alphaToCoverageEnabled && descriptor->count <= 1,
                    "Multisample count (%u) must be > 1 when alphaToCoverage is enabled.",
                    descriptor->count);

    return {};
}

MaybeError ValidateBlendComponent(BlendComponent blendComponent, bool dualSourceBlendingEnabled) {
    if (!dualSourceBlendingEnabled) {
        DAWN_INVALID_IF(blendComponent.srcFactor == wgpu::BlendFactor::Src1 ||
                            blendComponent.srcFactor == wgpu::BlendFactor::OneMinusSrc1 ||
                            blendComponent.srcFactor == wgpu::BlendFactor::Src1Alpha ||
                            blendComponent.srcFactor == wgpu::BlendFactor::OneMinusSrc1Alpha,
                        "Source blend factor is %s while dualSourceBlending is not enabled.",
                        blendComponent.srcFactor);

        DAWN_INVALID_IF(blendComponent.dstFactor == wgpu::BlendFactor::Src1 ||
                            blendComponent.dstFactor == wgpu::BlendFactor::OneMinusSrc1 ||
                            blendComponent.dstFactor == wgpu::BlendFactor::Src1Alpha ||
                            blendComponent.dstFactor == wgpu::BlendFactor::OneMinusSrc1Alpha,
                        "Destination blend factor is %s while dualSourceBlending is not enabled.",
                        blendComponent.dstFactor);
    }

    if (blendComponent.operation == wgpu::BlendOperation::Min ||
        blendComponent.operation == wgpu::BlendOperation::Max) {
        DAWN_INVALID_IF(
            (blendComponent.srcFactor != wgpu::BlendFactor::One) &&
                (blendComponent.srcFactor != wgpu::BlendFactor::Undefined),
            "Source blend factor (%s) is defined and not %s when blend operation is %s.",
            blendComponent.srcFactor, wgpu::BlendFactor::One, blendComponent.operation);
        DAWN_INVALID_IF(
            (blendComponent.dstFactor != wgpu::BlendFactor::One) &&
                (blendComponent.dstFactor != wgpu::BlendFactor::Undefined),
            "Destination blend factor (%s) is defined and not %s when blend operation is %s.",
            blendComponent.dstFactor, wgpu::BlendFactor::One, blendComponent.operation);
    }

    return {};
}

MaybeError ValidateBlendState(DeviceBase* device, const BlendState* descriptor) {
    DAWN_TRY(ValidateBlendOperation(descriptor->alpha.operation));
    DAWN_TRY(ValidateBlendFactor(descriptor->alpha.srcFactor));
    DAWN_TRY(ValidateBlendFactor(descriptor->alpha.dstFactor));
    DAWN_TRY(ValidateBlendOperation(descriptor->color.operation));
    DAWN_TRY(ValidateBlendFactor(descriptor->color.srcFactor));
    DAWN_TRY(ValidateBlendFactor(descriptor->color.dstFactor));

    bool dualSourceBlendingEnabled = device->HasFeature(Feature::DualSourceBlending);
    DAWN_TRY(ValidateBlendComponent(descriptor->alpha, dualSourceBlendingEnabled));
    DAWN_TRY(ValidateBlendComponent(descriptor->color, dualSourceBlendingEnabled));

    return {};
}

bool BlendFactorContainsSrcAlpha(wgpu::BlendFactor blendFactor) {
    return blendFactor == wgpu::BlendFactor::SrcAlpha ||
           blendFactor == wgpu::BlendFactor::OneMinusSrcAlpha ||
           blendFactor == wgpu::BlendFactor::SrcAlphaSaturated ||
           blendFactor == wgpu::BlendFactor::Src1Alpha ||
           blendFactor == wgpu::BlendFactor::OneMinusSrc1Alpha;
}

bool BlendFactorContainsSrc1(wgpu::BlendFactor blendFactor) {
    return blendFactor == wgpu::BlendFactor::Src1 ||
           blendFactor == wgpu::BlendFactor::OneMinusSrc1 ||
           blendFactor == wgpu::BlendFactor::Src1Alpha ||
           blendFactor == wgpu::BlendFactor::OneMinusSrc1Alpha;
}

bool BlendStateUsesBlendFactorSrc1(const BlendState& blend) {
    return BlendFactorContainsSrc1(blend.alpha.srcFactor) ||
           BlendFactorContainsSrc1(blend.alpha.dstFactor) ||
           BlendFactorContainsSrc1(blend.color.srcFactor) ||
           BlendFactorContainsSrc1(blend.color.dstFactor);
}

MaybeError ValidateColorTargetState(
    DeviceBase* device,
    const ColorTargetState& descriptor,
    const Format* format,
    bool fragmentWritten,
    const EntryPointMetadata::FragmentRenderAttachmentInfo& fragmentOutputVariable,
    const MultisampleState& multisample) {
    UnpackedPtr<ColorTargetState> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(&descriptor));
    if (unpacked.Get<ColorTargetStateExpandResolveTextureDawn>()) {
        DAWN_INVALID_IF(!device->HasFeature(Feature::DawnLoadResolveTexture),
                        "The ColorTargetStateExpandResolveTextureDawn struct is used while the "
                        "%s feature is not enabled.",
                        ToAPI(Feature::DawnLoadResolveTexture));

        DAWN_INVALID_IF(
            multisample.count <= 1,
            "The ColorTargetStateExpandResolveTextureDawn struct is used while multisample count "
            "(%u) is not > 1.",
            multisample.count);
    }

    if (descriptor.blend) {
        DAWN_TRY_CONTEXT(ValidateBlendState(device, descriptor.blend), "validating blend state.");
    }

    DAWN_TRY(ValidateColorWriteMask(descriptor.writeMask));
    DAWN_INVALID_IF(!format->IsColor() || !format->isRenderable,
                    "Color format (%s) is not color renderable.", format->format);

    DAWN_INVALID_IF(descriptor.blend && !format->isBlendable,
                    "Blending is enabled but color format (%s) is not blendable.", format->format);

    if (!fragmentWritten) {
        DAWN_INVALID_IF(
            descriptor.writeMask != wgpu::ColorWriteMask::None,
            "Color target has no corresponding fragment stage output but writeMask (%s) is "
            "not zero.",
            descriptor.writeMask);
        return {};
    }

    DAWN_INVALID_IF(
        fragmentOutputVariable.baseType != format->GetAspectInfo(Aspect::Color).baseType,
        "Color format (%s) base type (%s) doesn't match the fragment "
        "module output type (%s).",
        format->format, format->GetAspectInfo(Aspect::Color).baseType,
        fragmentOutputVariable.baseType);

    DAWN_INVALID_IF(fragmentOutputVariable.componentCount < format->componentCount,
                    "The fragment stage has fewer output components (%u) than the color format "
                    "(%s) component count (%u).",
                    fragmentOutputVariable.componentCount, format->format, format->componentCount);

    if (descriptor.blend && fragmentOutputVariable.componentCount < 4u) {
        // No alpha channel output, make sure there's no alpha involved in the blending operation.
        DAWN_INVALID_IF(BlendFactorContainsSrcAlpha(descriptor.blend->color.srcFactor) ||
                            BlendFactorContainsSrcAlpha(descriptor.blend->color.dstFactor),
                        "Color blending srcFactor (%s) or dstFactor (%s) is reading alpha "
                        "but it is missing from fragment output.",
                        descriptor.blend->color.srcFactor, descriptor.blend->color.dstFactor);
    }

    return {};
}

MaybeError ValidateFramebufferInput(
    DeviceBase* device,
    const Format* format,
    const EntryPointMetadata::FragmentRenderAttachmentInfo& inputVar) {
    DAWN_INVALID_IF(inputVar.baseType != format->GetAspectInfo(Aspect::Color).baseType,
                    "Color format (%s) base type (%s) doesn't match the fragment "
                    "module input type (%s).",
                    format->format, format->GetAspectInfo(Aspect::Color).baseType,
                    inputVar.baseType);
    DAWN_INVALID_IF(inputVar.componentCount != format->componentCount,
                    "The fragment stage number of input components (%u) doesn't match the color "
                    "format (%s) component count (%u).",
                    inputVar.componentCount, format->format, format->componentCount);
    return {};
}

MaybeError ValidateColorTargetStatesMatch(ColorAttachmentIndex firstColorTargetIndex,
                                          const ColorTargetState* const firstColorTargetState,
                                          ColorAttachmentIndex targetIndex,
                                          const ColorTargetState* target) {
    DAWN_INVALID_IF(firstColorTargetState->writeMask != target->writeMask,
                    "targets[%u].writeMask (%s) does not match targets[%u].writeMask (%s).",
                    targetIndex, target->writeMask, firstColorTargetIndex,
                    firstColorTargetState->writeMask);
    if (!firstColorTargetState->blend) {
        DAWN_INVALID_IF(target->blend,
                        "targets[%u].blend has a blend state but targets[%u].blend does not.",
                        targetIndex, firstColorTargetIndex);
    } else {
        DAWN_INVALID_IF(!target->blend,
                        "targets[%u].blend has a blend state but targets[%u].blend does not.",
                        firstColorTargetIndex, targetIndex);

        const BlendState& currBlendState = *target->blend;
        const BlendState& firstBlendState = *firstColorTargetState->blend;

        DAWN_INVALID_IF(
            firstBlendState.color.operation != currBlendState.color.operation,
            "targets[%u].color.operation (%s) does not match targets[%u].color.operation (%s).",
            firstColorTargetIndex, firstBlendState.color.operation, targetIndex,
            currBlendState.color.operation);
        DAWN_INVALID_IF(
            firstBlendState.color.srcFactor != currBlendState.color.srcFactor,
            "targets[%u].color.srcFactor (%s) does not match targets[%u].color.srcFactor (%s).",
            firstColorTargetIndex, firstBlendState.color.srcFactor, targetIndex,
            currBlendState.color.srcFactor);
        DAWN_INVALID_IF(
            firstBlendState.color.dstFactor != currBlendState.color.dstFactor,
            "targets[%u].color.dstFactor (%s) does not match targets[%u].color.dstFactor (%s).",
            firstColorTargetIndex, firstBlendState.color.dstFactor, targetIndex,
            currBlendState.color.dstFactor);
        DAWN_INVALID_IF(
            firstBlendState.alpha.operation != currBlendState.alpha.operation,
            "targets[%u].alpha.operation (%s) does not match targets[%u].alpha.operation (%s).",
            firstColorTargetIndex, firstBlendState.alpha.operation, targetIndex,
            currBlendState.alpha.operation);
        DAWN_INVALID_IF(
            firstBlendState.alpha.srcFactor != currBlendState.alpha.srcFactor,
            "targets[%u].alpha.srcFactor (%s) does not match targets[%u].alpha.srcFactor (%s).",
            firstColorTargetIndex, firstBlendState.alpha.srcFactor, targetIndex,
            currBlendState.alpha.srcFactor);
        DAWN_INVALID_IF(
            firstBlendState.alpha.dstFactor != currBlendState.alpha.dstFactor,
            "targets[%u].alpha.dstFactor (%s) does not match targets[%u].alpha.dstFactor (%s).",
            firstColorTargetIndex, firstBlendState.alpha.dstFactor, targetIndex,
            currBlendState.alpha.dstFactor);
    }
    return {};
}

ResultOrError<ShaderModuleEntryPoint> ValidateFragmentState(DeviceBase* device,
                                                            const FragmentState* descriptor,
                                                            const PipelineLayoutBase* layout,
                                                            const DepthStencilState* depthStencil,
                                                            const MultisampleState& multisample) {
    DAWN_INVALID_IF(descriptor->nextInChain != nullptr, "nextInChain must be nullptr.");

    ShaderModuleEntryPoint entryPoint;
    DAWN_TRY_ASSIGN_CONTEXT(
        entryPoint,
        ValidateProgrammableStage(device, descriptor->module, descriptor->entryPoint,
                                  descriptor->constantCount, descriptor->constants, layout,
                                  SingleShaderStage::Fragment),
        "validating fragment stage (%s, entryPoint: %s).", descriptor->module,
        descriptor->entryPoint);

    const EntryPointMetadata& fragmentMetadata = descriptor->module->GetEntryPoint(entryPoint.name);

    if (fragmentMetadata.usesFragDepth) {
        DAWN_INVALID_IF(depthStencil == nullptr,
                        "Depth stencil state is not present when fragment stage (%s, %s) is "
                        "writing to frag_depth.",
                        descriptor->module, &entryPoint);
        const Format* depthStencilFormat;
        DAWN_TRY_ASSIGN(depthStencilFormat, device->GetInternalFormat(depthStencil->format));
        DAWN_INVALID_IF(!depthStencilFormat->HasDepth(),
                        "Depth stencil state format (%s) has no depth aspect when fragment stage "
                        "(%s, %s) is "
                        "writing to frag_depth.",
                        depthStencil->format, descriptor->module, &entryPoint);
    }

    uint32_t maxColorAttachments = device->GetLimits().v1.maxColorAttachments;
    DAWN_INVALID_IF(descriptor->targetCount > maxColorAttachments,
                    "Number of targets (%u) exceeds the maximum (%u).%s", descriptor->targetCount,
                    maxColorAttachments,
                    DAWN_INCREASE_LIMIT_MESSAGE(device->GetAdapter()->GetLimits().v1,
                                                maxColorAttachments, descriptor->targetCount));

    auto targets =
        ityp::SpanFromUntyped<ColorAttachmentIndex>(descriptor->targets, descriptor->targetCount);

    ColorAttachmentMask targetMask;
    for (auto [i, target] : Enumerate(targets)) {
        if (target.format == wgpu::TextureFormat::Undefined) {
            DAWN_INVALID_IF(target.blend,
                            "Color target[%u] blend state is set when the format is undefined.", i);
        } else {
            targetMask.set(i);
        }
    }

    bool usesSrc1 = false;
    bool usesBlendSrc1 = false;
    ColorAttachmentFormats colorAttachmentFormats;
    for (auto i : IterateBitSet(targetMask)) {
        const Format* format;
        DAWN_TRY_ASSIGN(format, device->GetInternalFormat(targets[i].format));

        DAWN_TRY_CONTEXT(ValidateColorTargetState(
                             device, targets[i], format, fragmentMetadata.fragmentOutputMask[i],
                             fragmentMetadata.fragmentOutputVariables[i], multisample),
                         "validating targets[%u] framebuffer output.", i);
        colorAttachmentFormats.push_back(&device->GetValidInternalFormat(targets[i].format));

        if (fragmentMetadata.fragmentOutputVariables[i].blendSrc == 1u) {
            usesBlendSrc1 = true;
        }

        if (fragmentMetadata.fragmentInputMask[i]) {
            DAWN_TRY_CONTEXT(ValidateFramebufferInput(device, format,
                                                      fragmentMetadata.fragmentInputVariables[i]),
                             "validating targets[%u]'s framebuffer input.", i);
        }

        if (targets[i].blend != nullptr) {
            usesSrc1 |= BlendStateUsesBlendFactorSrc1(*targets[i].blend);
        }
    }

    if (usesSrc1) {
        DAWN_INVALID_IF(!usesBlendSrc1,
                        "One of the blend factor uses `blend_src(1)` while `blend_src(1)` is "
                        "missing from the fragment shader outputs.");
        DAWN_INVALID_IF(descriptor->targetCount != 1,
                        "One of the blend factor uses `blend_src(1)` but the color targets count "
                        "is not 1.");
    }

    auto extraFramebufferInputs = fragmentMetadata.fragmentInputMask & ~targetMask;
    DAWN_INVALID_IF(
        extraFramebufferInputs.any(),
        "Framebuffer input at index %u is used without a corresponding color target state.",
        uint8_t(ityp::Sub(GetHighestBitIndexPlusOne(extraFramebufferInputs),
                          ColorAttachmentIndex(uint8_t(1)))));

    DAWN_TRY(ValidateColorAttachmentBytesPerSample(device, colorAttachmentFormats));

    if (multisample.alphaToCoverageEnabled) {
        DAWN_INVALID_IF(fragmentMetadata.usesSampleMaskOutput,
                        "alphaToCoverageEnabled is true when the sample_mask builtin is a "
                        "pipeline output of fragment stage of %s.",
                        descriptor->module);

        DAWN_INVALID_IF(descriptor->targetCount == 0 ||
                            descriptor->targets[0].format == wgpu::TextureFormat::Undefined,
                        "alphaToCoverageEnabled is true when color target[0] is not present.");

        const Format* format;
        DAWN_TRY_ASSIGN(format, device->GetInternalFormat(descriptor->targets[0].format));
        DAWN_INVALID_IF(
            !format->HasAlphaChannel(),
            "alphaToCoverageEnabled is true when target[0].format (%s) has no alpha channel.",
            format->format);
    }

    if (device->IsCompatibilityMode()) {
        DAWN_INVALID_IF(
            fragmentMetadata.usesSampleMaskOutput,
            "sample_mask is not supported in compatibility mode in the fragment stage (%s, %s)",
            descriptor->module, &entryPoint);

        DAWN_INVALID_IF(
            fragmentMetadata.usesSampleIndex,
            "sample_index is not supported in compatibility mode in the fragment stage (%s, %s)",
            descriptor->module, &entryPoint);

        // Check that all the color target states match.
        ColorAttachmentIndex firstColorTargetIndex{};
        const ColorTargetState* firstColorTargetState = nullptr;
        for (auto i : IterateBitSet(targetMask)) {
            if (!firstColorTargetState) {
                firstColorTargetState = &targets[i];
                firstColorTargetIndex = i;
                continue;
            }

            DAWN_TRY_CONTEXT(ValidateColorTargetStatesMatch(firstColorTargetIndex,
                                                            firstColorTargetState, i, &targets[i]),
                             "validating targets in compatibility mode.");
        }
    }

    return entryPoint;
}

MaybeError ValidateInterStageMatching(DeviceBase* device,
                                      const VertexState& vertexState,
                                      const ShaderModuleEntryPoint& vertexEntryPoint,
                                      const FragmentState& fragmentState,
                                      const ShaderModuleEntryPoint& fragmentEntryPoint) {
    const EntryPointMetadata& vertexMetadata =
        vertexState.module->GetEntryPoint(vertexEntryPoint.name);
    const EntryPointMetadata& fragmentMetadata =
        fragmentState.module->GetEntryPoint(fragmentEntryPoint.name);

    size_t maxInterStageShaderVariables = device->GetLimits().v1.maxInterStageShaderVariables;
    DAWN_ASSERT(vertexMetadata.usedInterStageVariables.size() == maxInterStageShaderVariables);
    DAWN_ASSERT(fragmentMetadata.usedInterStageVariables.size() == maxInterStageShaderVariables);
    for (size_t i = 0; i < maxInterStageShaderVariables; ++i) {
        if (!vertexMetadata.usedInterStageVariables[i]) {
            if (fragmentMetadata.usedInterStageVariables[i]) {
                return DAWN_VALIDATION_ERROR(
                    "The fragment input at location %u doesn't have a corresponding vertex output.",
                    i);
            }
            continue;
        }

        // It is valid that fragment output is a subset of vertex input
        if (!fragmentMetadata.usedInterStageVariables[i]) {
            continue;
        }
        const auto& vertexOutputInfo = vertexMetadata.interStageVariables[i];
        const auto& fragmentInputInfo = fragmentMetadata.interStageVariables[i];
        DAWN_INVALID_IF(
            vertexOutputInfo.baseType != fragmentInputInfo.baseType,
            "The base type (%s) of the vertex output at location %u is different from the "
            "base type (%s) of the fragment input at location %u.",
            vertexOutputInfo.baseType, i, fragmentInputInfo.baseType, i);

        DAWN_INVALID_IF(vertexOutputInfo.componentCount != fragmentInputInfo.componentCount,
                        "The component count (%u) of the vertex output at location %u is different "
                        "from the component count (%u) of the fragment input at location %u.",
                        vertexOutputInfo.componentCount, i, fragmentInputInfo.componentCount, i);

        DAWN_INVALID_IF(
            vertexOutputInfo.interpolationType != fragmentInputInfo.interpolationType,
            "The interpolation type (%s) of the vertex output at location %u is different "
            "from the interpolation type (%s) of the fragment input at location %u.",
            vertexOutputInfo.interpolationType, i, fragmentInputInfo.interpolationType, i);

        DAWN_INVALID_IF(
            vertexOutputInfo.interpolationSampling != fragmentInputInfo.interpolationSampling,
            "The interpolation sampling (%s) of the vertex output at location %u is "
            "different from the interpolation sampling (%s) of the fragment input at "
            "location %u.",
            vertexOutputInfo.interpolationSampling, i, fragmentInputInfo.interpolationSampling, i);

        if (device->IsCompatibilityMode()) {
            DAWN_INVALID_IF(
                vertexOutputInfo.interpolationType == InterpolationType::Linear,
                "The interpolation type (%s) of the vertex output at location %u is not "
                "supported in compatibility mode",
                vertexOutputInfo.interpolationType, i);

            DAWN_INVALID_IF(
                vertexOutputInfo.interpolationSampling == InterpolationSampling::Sample ||
                    vertexOutputInfo.interpolationSampling == InterpolationSampling::First,
                "The interpolation sampling (%s) of the vertex output at location %u is "
                "not supported in compatibility mode",
                vertexOutputInfo.interpolationSampling, i);

            DAWN_INVALID_IF(
                vertexOutputInfo.interpolationType == InterpolationType::Flat &&
                    vertexOutputInfo.interpolationSampling == InterpolationSampling::None,
                "The interpolation sampling (%s) of the vertex output at location %u when "
                "interpolation type is (%s)"
                "not supported in compatibility mode",
                vertexOutputInfo.interpolationSampling, i, vertexOutputInfo.interpolationType);
        }
    }

    return {};
}
}  // anonymous namespace

// Helper functions
size_t IndexFormatSize(wgpu::IndexFormat format) {
    switch (format) {
        case wgpu::IndexFormat::Uint16:
            return sizeof(uint16_t);
        case wgpu::IndexFormat::Uint32:
            return sizeof(uint32_t);
        case wgpu::IndexFormat::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

bool IsStripPrimitiveTopology(wgpu::PrimitiveTopology primitiveTopology) {
    return primitiveTopology == wgpu::PrimitiveTopology::LineStrip ||
           primitiveTopology == wgpu::PrimitiveTopology::TriangleStrip;
}

MaybeError ValidateRenderPipelineDescriptor(DeviceBase* device,
                                            const RenderPipelineDescriptor* descriptor) {
    UnpackedPtr<RenderPipelineDescriptor> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(descriptor));

    if (descriptor->layout != nullptr) {
        DAWN_TRY(device->ValidateObject(descriptor->layout));
    }

    ShaderModuleEntryPoint vertexEntryPoint;
    DAWN_TRY_ASSIGN_CONTEXT(vertexEntryPoint,
                            ValidateVertexState(device, &descriptor->vertex, descriptor->layout,
                                                descriptor->primitive.topology),
                            "validating vertex state.");

    DAWN_TRY_CONTEXT(ValidatePrimitiveState(device, &descriptor->primitive),
                     "validating primitive state.");

    if (descriptor->depthStencil) {
        DAWN_TRY_CONTEXT(ValidateDepthStencilState(device, descriptor->depthStencil,
                                                   descriptor->primitive.topology),
                         "validating depthStencil state.");
    }

    DAWN_TRY_CONTEXT(ValidateMultisampleState(device, &descriptor->multisample),
                     "validating multisample state.");

    DAWN_INVALID_IF(
        descriptor->multisample.alphaToCoverageEnabled && descriptor->fragment == nullptr,
        "alphaToCoverageEnabled is true when fragment state is not present.");

    if (descriptor->fragment != nullptr) {
        ShaderModuleEntryPoint fragmentEntryPoint;
        DAWN_TRY_ASSIGN_CONTEXT(
            fragmentEntryPoint,
            ValidateFragmentState(device, descriptor->fragment, descriptor->layout,
                                  descriptor->depthStencil, descriptor->multisample),
            "validating fragment state.");

        DAWN_TRY(ValidateInterStageMatching(device, descriptor->vertex, vertexEntryPoint,
                                            *(descriptor->fragment), fragmentEntryPoint));
    }

    bool hasStorageAttachments =
        descriptor->layout != nullptr && descriptor->layout->HasAnyStorageAttachments();
    bool hasColorAttachments =
        descriptor->fragment != nullptr && descriptor->fragment->targetCount != 0;
    bool hasDepthStencilAttachment = descriptor->depthStencil != nullptr;
    DAWN_INVALID_IF(!hasColorAttachments && !hasDepthStencilAttachment && !hasStorageAttachments,
                    "No attachment was specified.");

    return {};
}

std::vector<StageAndDescriptor> GetRenderStagesAndSetPlaceholderShader(
    DeviceBase* device,
    const RenderPipelineDescriptor* descriptor) {
    std::vector<StageAndDescriptor> stages;
    stages.push_back({SingleShaderStage::Vertex, descriptor->vertex.module,
                      descriptor->vertex.entryPoint, descriptor->vertex.constantCount,
                      descriptor->vertex.constants});
    if (descriptor->fragment != nullptr) {
        stages.push_back({SingleShaderStage::Fragment, descriptor->fragment->module,
                          descriptor->fragment->entryPoint, descriptor->fragment->constantCount,
                          descriptor->fragment->constants});
    } else if (device->IsToggleEnabled(Toggle::UsePlaceholderFragmentInVertexOnlyPipeline)) {
        InternalPipelineStore* store = device->GetInternalPipelineStore();
        // The placeholder fragment shader module should already be initialized
        DAWN_ASSERT(store->placeholderFragmentShader != nullptr);
        ShaderModuleBase* placeholderFragmentShader = store->placeholderFragmentShader.Get();
        stages.push_back(
            {SingleShaderStage::Fragment, placeholderFragmentShader, "fs_empty_main", 0, nullptr});
    }
    return stages;
}

// RenderPipelineBase

RenderPipelineBase::RenderPipelineBase(DeviceBase* device,
                                       const UnpackedPtr<RenderPipelineDescriptor>& descriptor)
    : PipelineBase(device,
                   descriptor->layout,
                   descriptor->label,
                   GetRenderStagesAndSetPlaceholderShader(device, *descriptor)),
      mAttachmentState(device->GetOrCreateAttachmentState(descriptor, GetLayout())) {
    mVertexBufferCount = descriptor->vertex.bufferCount;

    auto buffers =
        ityp::SpanFromUntyped<VertexBufferSlot>(descriptor->vertex.buffers, mVertexBufferCount);
    for (auto [slot, buffer] : Enumerate(buffers)) {
        // Skip unused slots
        if (buffer.stepMode == wgpu::VertexStepMode::Undefined && buffer.attributeCount == 0) {
            continue;
        }

        mVertexBuffersUsed.set(slot);
        mVertexBufferInfos[slot].arrayStride = buffer.arrayStride;
        mVertexBufferInfos[slot].stepMode = (buffer.stepMode == wgpu::VertexStepMode::Undefined)
                                                ? wgpu::VertexStepMode::Vertex
                                                : buffer.stepMode;
        mVertexBufferInfos[slot].usedBytesInStride = 0;
        mVertexBufferInfos[slot].lastStride = 0;
        switch (mVertexBufferInfos[slot].stepMode) {
            case wgpu::VertexStepMode::Vertex:
                mVertexBuffersUsedAsVertexBuffer.set(slot);
                break;
            case wgpu::VertexStepMode::Instance:
                mVertexBuffersUsedAsInstanceBuffer.set(slot);
                break;
            case wgpu::VertexStepMode::Undefined:
                DAWN_UNREACHABLE();
        }

        auto attributes = ityp::SpanFromUntyped<size_t>(buffer.attributes, buffer.attributeCount);
        for (auto [i, attribute] : Enumerate(attributes)) {
            VertexAttributeLocation location =
                VertexAttributeLocation(static_cast<uint8_t>(attribute.shaderLocation));

            mAttributeLocationsUsed.set(location);
            mAttributeInfos[location].shaderLocation = location;
            mAttributeInfos[location].vertexBufferSlot = slot;
            mAttributeInfos[location].offset = attribute.offset;
            mAttributeInfos[location].format = attribute.format;
            // Compute the access boundary of this attribute by adding attribute format size to
            // attribute offset. Although offset is in uint64_t, such sum must be no larger than
            // maxVertexBufferArrayStride (2048), which is promised by the GPUVertexBufferLayout
            // validation of creating render pipeline. Therefore, calculating in uint16_t will
            // cause no overflow.
            uint32_t formatByteSize = GetVertexFormatInfo(attribute.format).byteSize;
            DAWN_ASSERT(attribute.offset <= 2048);
            uint16_t accessBoundary = uint16_t(attribute.offset) + uint16_t(formatByteSize);
            mVertexBufferInfos[slot].usedBytesInStride =
                std::max(mVertexBufferInfos[slot].usedBytesInStride, accessBoundary);
            mVertexBufferInfos[slot].lastStride =
                std::max(mVertexBufferInfos[slot].lastStride,
                         mAttributeInfos[location].offset + formatByteSize);
        }
    }

    mPrimitive = descriptor->primitive.WithTrivialFrontendDefaults();
    mMultisample = descriptor->multisample;

    if (mAttachmentState->HasDepthStencilAttachment()) {
        mDepthStencil = descriptor->depthStencil->WithTrivialFrontendDefaults();

        // Reify depth option for stencil-only formats
        const Format& format = device->GetValidInternalFormat(mDepthStencil.format);
        if (!format.HasDepth()) {
            mDepthStencil.depthWriteEnabled = wgpu::OptionalBool::False;
            mDepthStencil.depthCompare = wgpu::CompareFunction::Always;
        }
        if (format.HasDepth() && mDepthStencil.depthCompare == wgpu::CompareFunction::Undefined &&
            mDepthStencil.depthWriteEnabled != wgpu::OptionalBool::True &&
            mDepthStencil.stencilFront.depthFailOp == wgpu::StencilOperation::Keep &&
            mDepthStencil.stencilBack.depthFailOp == wgpu::StencilOperation::Keep) {
            mDepthStencil.depthCompare = wgpu::CompareFunction::Always;
        }
        mWritesDepth = mDepthStencil.depthWriteEnabled == wgpu::OptionalBool::True;
        if (mDepthStencil.stencilWriteMask) {
            if ((mPrimitive.cullMode != wgpu::CullMode::Front &&
                 (mDepthStencil.stencilFront.failOp != wgpu::StencilOperation::Keep ||
                  mDepthStencil.stencilFront.depthFailOp != wgpu::StencilOperation::Keep ||
                  mDepthStencil.stencilFront.passOp != wgpu::StencilOperation::Keep)) ||
                (mPrimitive.cullMode != wgpu::CullMode::Back &&
                 (mDepthStencil.stencilBack.failOp != wgpu::StencilOperation::Keep ||
                  mDepthStencil.stencilBack.depthFailOp != wgpu::StencilOperation::Keep ||
                  mDepthStencil.stencilBack.passOp != wgpu::StencilOperation::Keep))) {
                mWritesStencil = true;
            }
        }
    } else {
        // These default values below are useful for backends to fill information.
        // The values indicate that depth and stencil test are disabled when backends
        // set their own depth stencil states/descriptors according to the values in
        // mDepthStencil.
        // - Most defaults come from the dawn::native::DepthStencilState definition.
        mDepthStencil = {};
        // - depthCompare is nullable for validation purposes but should default to Always.
        mDepthStencil.depthCompare = wgpu::CompareFunction::Always;
    }

    for (auto i : IterateBitSet(mAttachmentState->GetColorAttachmentsMask())) {
        // Vertex-only render pipeline have no color attachment. For a render pipeline with
        // color attachments, there must be a valid FragmentState.
        DAWN_ASSERT(descriptor->fragment != nullptr);
        const ColorTargetState* target = &descriptor->fragment->targets[static_cast<uint8_t>(i)];
        mTargets[i] = *target;

        if (target->blend != nullptr) {
            mTargetBlend[i] = target->blend->WithTrivialFrontendDefaults();
            mTargets[i].blend = &mTargetBlend[i];
        }
    }

    if (HasStage(SingleShaderStage::Fragment)) {
        mUsesFragDepth = GetStage(SingleShaderStage::Fragment).metadata->usesFragDepth;
    }

    if (HasStage(SingleShaderStage::Vertex)) {
        mUsesVertexIndex = GetStage(SingleShaderStage::Vertex).metadata->usesVertexIndex;
        mUsesInstanceIndex = GetStage(SingleShaderStage::Vertex).metadata->usesInstanceIndex;
    }

    SetContentHash(ComputeContentHash());
    GetObjectTrackingList()->Track(this);

    // Initialize the cache key to include the cache type and device information.
    StreamIn(&mCacheKey, CacheKey::Type::RenderPipeline, device->GetCacheKey());
}

RenderPipelineBase::RenderPipelineBase(DeviceBase* device,
                                       ObjectBase::ErrorTag tag,
                                       StringView label)
    : PipelineBase(device, tag, label) {}

RenderPipelineBase::~RenderPipelineBase() = default;

void RenderPipelineBase::DestroyImpl() {
    Uncache();

    // Remove reference to the attachment state so that we don't have lingering references to
    // it preventing it from being uncached in the device.
    mAttachmentState = nullptr;
}

// static
Ref<RenderPipelineBase> RenderPipelineBase::MakeError(DeviceBase* device, StringView label) {
    class ErrorRenderPipeline final : public RenderPipelineBase {
      public:
        explicit ErrorRenderPipeline(DeviceBase* device, StringView label)
            : RenderPipelineBase(device, ObjectBase::kError, label) {}

        MaybeError InitializeImpl() override {
            DAWN_UNREACHABLE();
            return {};
        }
    };

    return AcquireRef(new ErrorRenderPipeline(device, label));
}

ObjectType RenderPipelineBase::GetType() const {
    return ObjectType::RenderPipeline;
}

const VertexAttributeMask& RenderPipelineBase::GetAttributeLocationsUsed() const {
    DAWN_ASSERT(!IsError());
    return mAttributeLocationsUsed;
}

const VertexAttributeInfo& RenderPipelineBase::GetAttribute(
    VertexAttributeLocation location) const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mAttributeLocationsUsed[location]);
    return mAttributeInfos[location];
}

const VertexBufferMask& RenderPipelineBase::GetVertexBuffersUsed() const {
    DAWN_ASSERT(!IsError());
    return mVertexBuffersUsed;
}

const VertexBufferMask& RenderPipelineBase::GetVertexBuffersUsedAsVertexBuffer() const {
    DAWN_ASSERT(!IsError());
    return mVertexBuffersUsedAsVertexBuffer;
}

const VertexBufferMask& RenderPipelineBase::GetVertexBuffersUsedAsInstanceBuffer() const {
    DAWN_ASSERT(!IsError());
    return mVertexBuffersUsedAsInstanceBuffer;
}

const VertexBufferInfo& RenderPipelineBase::GetVertexBuffer(VertexBufferSlot slot) const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mVertexBuffersUsed[slot]);
    return mVertexBufferInfos[slot];
}

uint32_t RenderPipelineBase::GetVertexBufferCount() const {
    DAWN_ASSERT(!IsError());
    return mVertexBufferCount;
}

const ColorTargetState* RenderPipelineBase::GetColorTargetState(
    ColorAttachmentIndex attachmentSlot) const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(attachmentSlot < mTargets.size());
    return &mTargets[attachmentSlot];
}

const DepthStencilState* RenderPipelineBase::GetDepthStencilState() const {
    DAWN_ASSERT(!IsError());
    return &mDepthStencil;
}

bool RenderPipelineBase::UsesStencil() const {
    return mDepthStencil.stencilBack.compare != wgpu::CompareFunction::Always ||
           mDepthStencil.stencilBack.failOp != wgpu::StencilOperation::Keep ||
           mDepthStencil.stencilBack.depthFailOp != wgpu::StencilOperation::Keep ||
           mDepthStencil.stencilBack.passOp != wgpu::StencilOperation::Keep ||
           mDepthStencil.stencilFront.compare != wgpu::CompareFunction::Always ||
           mDepthStencil.stencilFront.failOp != wgpu::StencilOperation::Keep ||
           mDepthStencil.stencilFront.depthFailOp != wgpu::StencilOperation::Keep ||
           mDepthStencil.stencilFront.passOp != wgpu::StencilOperation::Keep;
}

wgpu::PrimitiveTopology RenderPipelineBase::GetPrimitiveTopology() const {
    DAWN_ASSERT(!IsError());
    return mPrimitive.topology;
}

wgpu::IndexFormat RenderPipelineBase::GetStripIndexFormat() const {
    DAWN_ASSERT(!IsError());
    return mPrimitive.stripIndexFormat;
}

wgpu::CullMode RenderPipelineBase::GetCullMode() const {
    DAWN_ASSERT(!IsError());
    return mPrimitive.cullMode;
}

wgpu::FrontFace RenderPipelineBase::GetFrontFace() const {
    DAWN_ASSERT(!IsError());
    return mPrimitive.frontFace;
}

bool RenderPipelineBase::IsDepthBiasEnabled() const {
    DAWN_ASSERT(!IsError());
    return mDepthStencil.depthBias != 0 || mDepthStencil.depthBiasSlopeScale != 0;
}

int32_t RenderPipelineBase::GetDepthBias() const {
    DAWN_ASSERT(!IsError());
    return mDepthStencil.depthBias;
}

float RenderPipelineBase::GetDepthBiasSlopeScale() const {
    DAWN_ASSERT(!IsError());
    return mDepthStencil.depthBiasSlopeScale;
}

float RenderPipelineBase::GetDepthBiasClamp() const {
    DAWN_ASSERT(!IsError());
    return mDepthStencil.depthBiasClamp;
}

bool RenderPipelineBase::HasUnclippedDepth() const {
    DAWN_ASSERT(!IsError());
    return mPrimitive.unclippedDepth;
}

ColorAttachmentMask RenderPipelineBase::GetColorAttachmentsMask() const {
    DAWN_ASSERT(!IsError());
    return mAttachmentState->GetColorAttachmentsMask();
}

bool RenderPipelineBase::HasDepthStencilAttachment() const {
    DAWN_ASSERT(!IsError());
    return mAttachmentState->HasDepthStencilAttachment();
}

wgpu::TextureFormat RenderPipelineBase::GetColorAttachmentFormat(
    ColorAttachmentIndex attachment) const {
    DAWN_ASSERT(!IsError());
    return mTargets[attachment].format;
}

wgpu::TextureFormat RenderPipelineBase::GetDepthStencilFormat() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mAttachmentState->HasDepthStencilAttachment());
    return mDepthStencil.format;
}

uint32_t RenderPipelineBase::GetSampleCount() const {
    DAWN_ASSERT(!IsError());
    return mAttachmentState->GetSampleCount();
}

uint32_t RenderPipelineBase::GetSampleMask() const {
    DAWN_ASSERT(!IsError());
    return mMultisample.mask;
}

bool RenderPipelineBase::IsAlphaToCoverageEnabled() const {
    DAWN_ASSERT(!IsError());
    return mMultisample.alphaToCoverageEnabled;
}

const AttachmentState* RenderPipelineBase::GetAttachmentState() const {
    DAWN_ASSERT(!IsError());
    return mAttachmentState.Get();
}

bool RenderPipelineBase::WritesDepth() const {
    DAWN_ASSERT(!IsError());
    return mWritesDepth;
}

bool RenderPipelineBase::WritesStencil() const {
    DAWN_ASSERT(!IsError());
    return mWritesStencil;
}

bool RenderPipelineBase::UsesFragDepth() const {
    DAWN_ASSERT(!IsError());
    return mUsesFragDepth;
}

bool RenderPipelineBase::UsesVertexIndex() const {
    DAWN_ASSERT(!IsError());
    return mUsesVertexIndex;
}

bool RenderPipelineBase::UsesInstanceIndex() const {
    DAWN_ASSERT(!IsError());
    return mUsesInstanceIndex;
}

size_t RenderPipelineBase::ComputeContentHash() {
    ObjectContentHasher recorder;

    // Record modules and layout
    recorder.Record(PipelineBase::ComputeContentHash());

    // Hierarchically record the attachment state.
    // It contains the attachments set, texture formats, and sample count.
    recorder.Record(mAttachmentState->GetContentHash());

    // Record attachments
    for (auto i : IterateBitSet(mAttachmentState->GetColorAttachmentsMask())) {
        const ColorTargetState& desc = *GetColorTargetState(i);
        recorder.Record(desc.writeMask);
        if (desc.blend != nullptr) {
            recorder.Record(desc.blend->color.operation, desc.blend->color.srcFactor,
                            desc.blend->color.dstFactor);
            recorder.Record(desc.blend->alpha.operation, desc.blend->alpha.srcFactor,
                            desc.blend->alpha.dstFactor);
        }
    }

    if (mAttachmentState->HasDepthStencilAttachment()) {
        const DepthStencilState& desc = mDepthStencil;
        recorder.Record(desc.depthWriteEnabled, desc.depthCompare);
        recorder.Record(desc.stencilReadMask, desc.stencilWriteMask);
        recorder.Record(desc.stencilFront.compare, desc.stencilFront.failOp,
                        desc.stencilFront.depthFailOp, desc.stencilFront.passOp);
        recorder.Record(desc.stencilBack.compare, desc.stencilBack.failOp,
                        desc.stencilBack.depthFailOp, desc.stencilBack.passOp);
        recorder.Record(desc.depthBias, desc.depthBiasSlopeScale, desc.depthBiasClamp);
    }

    // Record vertex state
    recorder.Record(mAttributeLocationsUsed);
    for (VertexAttributeLocation location : IterateBitSet(mAttributeLocationsUsed)) {
        const VertexAttributeInfo& desc = GetAttribute(location);
        recorder.Record(desc.shaderLocation, desc.vertexBufferSlot, desc.offset, desc.format);
    }

    recorder.Record(mVertexBuffersUsed);
    for (VertexBufferSlot slot : IterateBitSet(mVertexBuffersUsed)) {
        const VertexBufferInfo& desc = GetVertexBuffer(slot);
        recorder.Record(desc.arrayStride, desc.stepMode);
    }

    // Record primitive state
    recorder.Record(mPrimitive.topology, mPrimitive.stripIndexFormat, mPrimitive.frontFace,
                    mPrimitive.cullMode, mPrimitive.unclippedDepth);

    // Record multisample state
    // Sample count hashed as part of the attachment state
    recorder.Record(mMultisample.mask, mMultisample.alphaToCoverageEnabled);

    return recorder.GetContentHash();
}

bool RenderPipelineBase::EqualityFunc::operator()(const RenderPipelineBase* a,
                                                  const RenderPipelineBase* b) const {
    // Check the layout and shader stages.
    if (!PipelineBase::EqualForCache(a, b)) {
        return false;
    }

    // Check the attachment state.
    // It contains the attachments set, texture formats, and sample count.
    if (a->mAttachmentState.Get() != b->mAttachmentState.Get()) {
        return false;
    }

    if (a->mAttachmentState.Get() != nullptr) {
        for (auto i : IterateBitSet(a->mAttachmentState->GetColorAttachmentsMask())) {
            const ColorTargetState& descA = *a->GetColorTargetState(i);
            const ColorTargetState& descB = *b->GetColorTargetState(i);
            if (descA.writeMask != descB.writeMask) {
                return false;
            }
            if ((descA.blend == nullptr) != (descB.blend == nullptr)) {
                return false;
            }
            if (descA.blend != nullptr) {
                if (descA.blend->color.operation != descB.blend->color.operation ||
                    descA.blend->color.srcFactor != descB.blend->color.srcFactor ||
                    descA.blend->color.dstFactor != descB.blend->color.dstFactor) {
                    return false;
                }
                if (descA.blend->alpha.operation != descB.blend->alpha.operation ||
                    descA.blend->alpha.srcFactor != descB.blend->alpha.srcFactor ||
                    descA.blend->alpha.dstFactor != descB.blend->alpha.dstFactor) {
                    return false;
                }
            }
        }

        // Check depth/stencil state
        if (a->mAttachmentState->HasDepthStencilAttachment()) {
            const DepthStencilState& stateA = a->mDepthStencil;
            const DepthStencilState& stateB = b->mDepthStencil;

            DAWN_ASSERT(!std::isnan(stateA.depthBiasSlopeScale));
            DAWN_ASSERT(!std::isnan(stateB.depthBiasSlopeScale));
            DAWN_ASSERT(!std::isnan(stateA.depthBiasClamp));
            DAWN_ASSERT(!std::isnan(stateB.depthBiasClamp));

            if (stateA.depthWriteEnabled != stateB.depthWriteEnabled ||
                stateA.depthCompare != stateB.depthCompare ||
                stateA.depthBias != stateB.depthBias ||
                stateA.depthBiasSlopeScale != stateB.depthBiasSlopeScale ||
                stateA.depthBiasClamp != stateB.depthBiasClamp) {
                return false;
            }
            if (stateA.stencilFront.compare != stateB.stencilFront.compare ||
                stateA.stencilFront.failOp != stateB.stencilFront.failOp ||
                stateA.stencilFront.depthFailOp != stateB.stencilFront.depthFailOp ||
                stateA.stencilFront.passOp != stateB.stencilFront.passOp) {
                return false;
            }
            if (stateA.stencilBack.compare != stateB.stencilBack.compare ||
                stateA.stencilBack.failOp != stateB.stencilBack.failOp ||
                stateA.stencilBack.depthFailOp != stateB.stencilBack.depthFailOp ||
                stateA.stencilBack.passOp != stateB.stencilBack.passOp) {
                return false;
            }
            if (stateA.stencilReadMask != stateB.stencilReadMask ||
                stateA.stencilWriteMask != stateB.stencilWriteMask) {
                return false;
            }
        }
    }

    // Check vertex state
    if (a->mAttributeLocationsUsed != b->mAttributeLocationsUsed) {
        return false;
    }

    for (VertexAttributeLocation loc : IterateBitSet(a->mAttributeLocationsUsed)) {
        const VertexAttributeInfo& descA = a->GetAttribute(loc);
        const VertexAttributeInfo& descB = b->GetAttribute(loc);
        if (descA.shaderLocation != descB.shaderLocation ||
            descA.vertexBufferSlot != descB.vertexBufferSlot || descA.offset != descB.offset ||
            descA.format != descB.format) {
            return false;
        }
    }

    if (a->mVertexBuffersUsed != b->mVertexBuffersUsed) {
        return false;
    }

    for (VertexBufferSlot slot : IterateBitSet(a->mVertexBuffersUsed)) {
        const VertexBufferInfo& descA = a->GetVertexBuffer(slot);
        const VertexBufferInfo& descB = b->GetVertexBuffer(slot);
        if (descA.arrayStride != descB.arrayStride || descA.stepMode != descB.stepMode) {
            return false;
        }
    }

    // Check primitive state
    {
        const PrimitiveState& stateA = a->mPrimitive;
        const PrimitiveState& stateB = b->mPrimitive;
        if (stateA.topology != stateB.topology ||
            stateA.stripIndexFormat != stateB.stripIndexFormat ||
            stateA.frontFace != stateB.frontFace || stateA.cullMode != stateB.cullMode ||
            stateA.unclippedDepth != stateB.unclippedDepth) {
            return false;
        }
    }

    // Check multisample state
    {
        const MultisampleState& stateA = a->mMultisample;
        const MultisampleState& stateB = b->mMultisample;
        // Sample count already checked as part of the attachment state.
        if (stateA.mask != stateB.mask ||
            stateA.alphaToCoverageEnabled != stateB.alphaToCoverageEnabled) {
            return false;
        }
    }

    return true;
}

}  // namespace dawn::native
