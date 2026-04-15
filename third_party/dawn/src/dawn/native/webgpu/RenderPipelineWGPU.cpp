// Copyright 2025 The Dawn & Tint Authors
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

#include "src/dawn/native/webgpu/RenderPipelineWGPU.h"

#include <string>
#include <vector>

#include "dawn/common/StringViewUtils.h"
#include "dawn/native/webgpu/BindGroupLayoutWGPU.h"
#include "dawn/native/webgpu/CaptureContext.h"
#include "dawn/native/webgpu/DeviceWGPU.h"
#include "dawn/native/webgpu/PipelineLayoutWGPU.h"
#include "dawn/native/webgpu/ShaderModuleWGPU.h"
#include "dawn/native/webgpu/ToWGPU.h"

namespace dawn::native::webgpu {

// static
Ref<RenderPipeline> RenderPipeline::CreateUninitialized(
    Device* device,
    const UnpackedPtr<RenderPipelineDescriptor>& descriptor) {
    return AcquireRef(new RenderPipeline(device, descriptor));
}

RenderPipeline::RenderPipeline(Device* device,
                               const UnpackedPtr<RenderPipelineDescriptor>& descriptor)
    : RenderPipelineBase(device, descriptor),
      RecordableObject(schema::ObjectType::RenderPipeline),
      ObjectWGPU(device->wgpu->renderPipelineRelease) {}

MaybeError RenderPipeline::InitializeImpl() {
    auto device = ToBackend(GetDevice());

    WGPURenderPipelineDescriptor desc;
    std::vector<WGPUConstantEntry> vertexConstants;
    std::vector<std::string> vertexConstantsKeys;
    PerVertexBuffer<WGPUVertexBufferLayout> vertexBuffers = {};
    PerVertexBuffer<absl::InlinedVector<WGPUVertexAttribute, kMaxVertexAttributes>>
        vertexAttributes = {};
    WGPUDepthStencilState depthStencil;
    WGPUFragmentState fragmentState;
    std::vector<WGPUConstantEntry> fragmentConstants;
    std::vector<std::string> fragmentConstantsKeys;
    PerColorAttachment<WGPUColorTargetState> colorTargets = {};
    PerColorAttachment<WGPUBlendState> blends = {};
    PerColorAttachment<WGPUColorTargetStateExpandResolveTextureDawn>
        colorTargetStateExpandResolveTextureDawnExtensions = {};

    std::string label = GetLabel();
    desc.nextInChain = nullptr;
    desc.label = ToOutputStringView(label);
    auto layout = GetLayout();
    DAWN_ASSERT(layout != nullptr);
    desc.layout = ToBackend(layout)->GetInnerHandle();

    // Vertex State
    const ProgrammableStage& vertex = GetStage(SingleShaderStage::Vertex);
    desc.vertex.nextInChain = nullptr;
    desc.vertex.module = ToBackend(vertex.module.Get())->GetInnerHandle();
    desc.vertex.entryPoint = ToOutputStringView(vertex.entryPoint);
    PopulateWGPUConstants(&vertexConstants, &vertexConstantsKeys, vertex.constants);
    desc.vertex.constants = vertexConstants.data();
    desc.vertex.constantCount = vertexConstants.size();

    // Vertex Buffers
    for (VertexAttributeLocation location : GetAttributeLocationsUsed()) {
        const VertexAttributeInfo& dawnAttr = GetAttribute(location);
        vertexAttributes[dawnAttr.vertexBufferSlot].push_back({
            .nextInChain = nullptr,
            .format = ToAPI(dawnAttr.format),
            .offset = dawnAttr.offset,
            .shaderLocation = static_cast<uint32_t>(dawnAttr.shaderLocation),
        });
    }

    size_t bufferCount = 0;
    for (VertexBufferSlot slot : GetVertexBuffersUsed()) {
        const VertexBufferInfo& dawnBuffer = GetVertexBuffer(slot);
        WGPUVertexBufferLayout* wgpuBuffer = &vertexBuffers[slot];
        wgpuBuffer->arrayStride = dawnBuffer.arrayStride;
        wgpuBuffer->stepMode = ToAPI(dawnBuffer.stepMode);

        auto& wgpuAttributes = vertexAttributes[slot];
        wgpuBuffer->attributes = wgpuAttributes.data();
        wgpuBuffer->attributeCount = wgpuAttributes.size();
        bufferCount = static_cast<size_t>(slot) + 1;
    }
    desc.vertex.bufferCount = bufferCount;
    desc.vertex.buffers = vertexBuffers.data();

    // Primitive State
    desc.primitive.nextInChain = nullptr;
    desc.primitive.topology = ToAPI(GetPrimitiveTopology());
    if (IsStripPrimitiveTopology(GetPrimitiveTopology())) {
        desc.primitive.stripIndexFormat = ToAPI(GetStripIndexFormat());
    } else {
        desc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
    }
    desc.primitive.frontFace = ToAPI(GetFrontFace());
    desc.primitive.cullMode = ToAPI(GetCullMode());
    desc.primitive.unclippedDepth = HasUnclippedDepth();

    // Depth Stencil State
    if (HasDepthStencilAttachment()) {
        depthStencil = ToWGPU(GetDepthStencilState());
        desc.depthStencil = &depthStencil;
    } else {
        desc.depthStencil = nullptr;
    }

    // Multisample State
    desc.multisample.nextInChain = nullptr;
    desc.multisample.count = GetSampleCount();
    desc.multisample.mask = GetSampleMask();
    desc.multisample.alphaToCoverageEnabled = IsAlphaToCoverageEnabled();

    // Fragment State
    if (HasStage(SingleShaderStage::Fragment)) {
        const ProgrammableStage& fragment = GetStage(SingleShaderStage::Fragment);
        fragmentState.nextInChain = nullptr;
        fragmentState.module = ToBackend(fragment.module.Get())->GetInnerHandle();
        fragmentState.entryPoint = ToOutputStringView(fragment.entryPoint);
        PopulateWGPUConstants(&fragmentConstants, &fragmentConstantsKeys, fragment.constants);
        fragmentState.constants = fragmentConstants.data();
        fragmentState.constantCount = fragmentConstants.size();

        uint32_t targetCount = 0;
        for (auto i : GetColorAttachmentsMask()) {
            const ColorTargetState* dawnTarget = GetColorTargetState(i);
            WGPUColorTargetState* wgpuTarget = &colorTargets[i];
            wgpuTarget->nextInChain = nullptr;
            wgpuTarget->format = ToAPI(dawnTarget->format);

            if (dawnTarget->blend != nullptr) {
                blends[i] = ToWGPU(dawnTarget->blend);
                wgpuTarget->blend = &blends[i];
            } else {
                wgpuTarget->blend = nullptr;
            }
            wgpuTarget->writeMask = ToAPI(dawnTarget->writeMask);

            if (GetAttachmentState()->GetExpandResolveInfo().resolveTargetsMask.test(i)) {
                auto& e = colorTargetStateExpandResolveTextureDawnExtensions[i];
                e = WGPU_COLOR_TARGET_STATE_EXPAND_RESOLVE_TEXTURE_DAWN_INIT;
                e.enabled =
                    GetAttachmentState()->GetExpandResolveInfo().attachmentsToExpandResolve.test(i);
                e.chain.next = wgpuTarget->nextInChain;
                wgpuTarget->nextInChain = &(e.chain);
            }

            targetCount = static_cast<size_t>(i) + 1;
        }
        fragmentState.targetCount = targetCount;
        fragmentState.targets = colorTargets.data();

        desc.fragment = &fragmentState;
    } else {
        desc.fragment = nullptr;
    }

    mInnerHandle = device->wgpu->deviceCreateRenderPipeline(device->GetInnerHandle(), &desc);
    DAWN_ASSERT(mInnerHandle);
    return {};
}

void RenderPipeline::SetLabelImpl() {
    ToBackend(GetDevice())->CaptureSetLabel(this, GetLabel());
}

MaybeError RenderPipeline::AddReferenced(CaptureContext& captureContext) {
    DAWN_TRY(
        captureContext.AddResource(ToBackend(GetStage(SingleShaderStage::Vertex).module.Get())));
    if (HasStage(SingleShaderStage::Fragment)) {
        DAWN_TRY(captureContext.AddResource(
            ToBackend(GetStage(SingleShaderStage::Fragment).module.Get())));
    }
    DAWN_TRY(captureContext.AddResource(ToBackend(GetLayout())));
    return {};
}

schema::BlendComponent ToSchema(const BlendComponent* component) {
    const BlendComponent& c = component ? *component : BlendComponent();
    return {{
        .operation = c.operation,
        .srcFactor = c.srcFactor,
        .dstFactor = c.dstFactor,
    }};
}

schema::StencilFaceState ToSchema(const StencilFaceState& state) {
    return {{
        .compare = state.compare,
        .failOp = state.failOp,
        .depthFailOp = state.depthFailOp,
        .passOp = state.passOp,
    }};
}

MaybeError RenderPipeline::CaptureCreationParameters(CaptureContext& captureContext) {
    std::vector<schema::VertexBufferLayout> buffers;
    for (VertexBufferSlot slot : GetVertexBuffersUsed()) {
        const auto& info = GetVertexBuffer(slot);

        std::vector<schema::VertexAttribute> attributes;

        for (VertexAttributeLocation loc : GetAttributeLocationsUsed()) {
            const VertexAttributeInfo& attrib = GetAttribute(loc);
            // Only use the attributes that use the current input
            if (attrib.vertexBufferSlot != slot) {
                continue;
            }
            attributes.push_back({{
                .format = attrib.format,
                .offset = attrib.offset,
                .shaderLocation = uint32_t(attrib.shaderLocation),
            }});
        }

        buffers.push_back({{
            .arrayStride = info.arrayStride,
            .stepMode = info.stepMode,
            .attributes = attributes,
        }});
    }

    const DepthStencilState defaultDepthStencilState;
    const DepthStencilState* depthStencilState = GetDepthStencilState();
    if (!depthStencilState) {
        depthStencilState = &defaultDepthStencilState;
    }
    ProgrammableStage empty;
    const ProgrammableStage& fragment =
        HasStage(SingleShaderStage::Fragment) ? GetStage(SingleShaderStage::Fragment) : empty;

    static const schema::BlendComponent kDefaultBlendComponent{{
        .operation = wgpu::BlendOperation::Add,
        .srcFactor = wgpu::BlendFactor::One,
        .dstFactor = wgpu::BlendFactor::Zero,
    }};
    static const schema::ColorTargetState kDefaultColorTargetState{{
        .format = wgpu::TextureFormat::Undefined,
        .blend{{
            .color = kDefaultBlendComponent,
            .alpha = kDefaultBlendComponent,
        }},
        .writeMask = wgpu::ColorWriteMask::None,
    }};

    // The front end does not store the number of attachments but the API requires that we
    // provide them for sparse attachments so we initialize targets with enough slots
    // to cover all used slots and fill them with a state that will be set to unused
    // on replay.
    ColorAttachmentMask attachmentMask = GetColorAttachmentsMask();
    ColorAttachmentIndex attachmentCount = GetHighestBitIndexPlusOne(attachmentMask);
    std::vector<schema::ColorTargetState> targets(size_t(attachmentCount),
                                                  kDefaultColorTargetState);

    if (fragment.module != nullptr) {
        for (auto slot : attachmentMask) {
            const auto& target = *GetColorTargetState(slot);

            schema::ExpandResolveMode expandResolveMode = schema::ExpandResolveMode::Unused;
            if (GetAttachmentState()->GetExpandResolveInfo().resolveTargetsMask.test(slot)) {
                expandResolveMode =
                    GetAttachmentState()->GetExpandResolveInfo().attachmentsToExpandResolve.test(
                        slot)
                        ? schema::ExpandResolveMode::Enabled
                        : schema::ExpandResolveMode::Disabled;
            }

            targets[size_t(slot)] = {{
                .format = target.format,
                .blend{{
                    .color = ToSchema(target.blend ? &target.blend->color : nullptr),
                    .alpha = ToSchema(target.blend ? &target.blend->alpha : nullptr),
                }},
                .writeMask = target.writeMask,
                .expandResolveMode = expandResolveMode,
            }};
        }
    }

    schema::RenderPipeline data{{
        .layoutId = captureContext.GetId(GetLayout()),
        .vertex{{
            .program = ToSchema(captureContext, GetStage(SingleShaderStage::Vertex)),
            .buffers = buffers,
        }},
        .primitive{{
            .topology = GetPrimitiveTopology(),
            .stripIndexFormat = GetStripIndexFormat(),
            .frontFace = GetFrontFace(),
            .cullMode = GetCullMode(),
            .unclippedDepth = HasUnclippedDepth(),
        }},
        .depthStencil{{
            .format = depthStencilState->format,
            .depthWriteEnabled = depthStencilState->depthWriteEnabled == wgpu::OptionalBool(true),
            .depthCompare = depthStencilState->depthCompare,
            .stencilFront = ToSchema(depthStencilState->stencilFront),
            .stencilBack = ToSchema(depthStencilState->stencilBack),
            .stencilReadMask = depthStencilState->stencilReadMask,
            .stencilWriteMask = depthStencilState->stencilWriteMask,
            .depthBias = depthStencilState->depthBias,
            .depthBiasSlopeScale = depthStencilState->depthBiasSlopeScale,
            .depthBiasClamp = depthStencilState->depthBiasClamp,
        }},
        .multisample{{
            .count = GetSampleCount(),
            .mask = GetSampleMask(),
            .alphaToCoverageEnabled = IsAlphaToCoverageEnabled(),
        }},
        .fragment{{
            .program = ToSchema(captureContext, fragment),
            .targets = targets,
        }},
    }};
    Serialize(captureContext, data);
    return {};
}

}  // namespace dawn::native::webgpu
