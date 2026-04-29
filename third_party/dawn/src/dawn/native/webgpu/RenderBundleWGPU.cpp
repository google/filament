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

#include "dawn/native/webgpu/RenderBundleWGPU.h"

#include <utility>
#include <vector>

#include "dawn/native/CommandEncoder.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/webgpu/BindGroupWGPU.h"
#include "dawn/native/webgpu/BufferWGPU.h"
#include "dawn/native/webgpu/CaptureContext.h"
#include "dawn/native/webgpu/CommandBufferHelpers.h"
#include "dawn/native/webgpu/DeviceWGPU.h"
#include "dawn/native/webgpu/RenderPipelineWGPU.h"
#include "dawn/native/webgpu/TextureWGPU.h"
#include "dawn/native/webgpu/ToWGPU.h"

namespace dawn::native::webgpu {

namespace {

void EncodeRenderBundleCommand(const DawnProcTable& wgpu,
                               WGPURenderBundleEncoder encoder,
                               CommandIterator& commands,
                               Command type) {
    switch (type) {
        case Command::Draw: {
            auto cmd = commands.NextCommand<DrawCmd>();
            wgpu.renderBundleEncoderDraw(encoder, cmd->vertexCount, cmd->instanceCount,
                                         cmd->firstVertex, cmd->firstInstance);
            break;
        }

        case Command::DrawIndexed: {
            auto cmd = commands.NextCommand<DrawIndexedCmd>();
            wgpu.renderBundleEncoderDrawIndexed(encoder, cmd->indexCount, cmd->instanceCount,
                                                cmd->firstIndex, cmd->baseVertex,
                                                cmd->firstInstance);
            break;
        }

        case Command::DrawIndirect: {
            auto cmd = commands.NextCommand<DrawIndirectCmd>();
            wgpu.renderBundleEncoderDrawIndirect(
                encoder, ToBackend(cmd->indirectBuffer)->GetInnerHandle(), cmd->indirectOffset);
            break;
        }

        case Command::DrawIndexedIndirect: {
            auto cmd = commands.NextCommand<DrawIndexedIndirectCmd>();
            wgpu.renderBundleEncoderDrawIndexedIndirect(
                encoder, ToBackend(cmd->indirectBuffer)->GetInnerHandle(), cmd->indirectOffset);
            break;
        }

        case Command::MultiDrawIndirect: {
            DAWN_UNREACHABLE();
            break;
        }

        case Command::MultiDrawIndexedIndirect: {
            DAWN_UNREACHABLE();
            break;
        }

        case Command::InsertDebugMarker: {
            auto cmd = commands.NextCommand<InsertDebugMarkerCmd>();
            char* label = commands.NextData<char>(cmd->length + 1);
            wgpu.renderBundleEncoderInsertDebugMarker(encoder, {label, cmd->length});
            break;
        }

        case Command::PopDebugGroup: {
            commands.NextCommand<PopDebugGroupCmd>();
            wgpu.renderBundleEncoderPopDebugGroup(encoder);
            break;
        }

        case Command::PushDebugGroup: {
            auto cmd = commands.NextCommand<PushDebugGroupCmd>();
            char* label = commands.NextData<char>(cmd->length + 1);
            wgpu.renderBundleEncoderPushDebugGroup(encoder, {label, cmd->length});
            break;
        }

        case Command::SetBindGroup: {
            auto cmd = commands.NextCommand<SetBindGroupCmd>();
            uint32_t* dynamicOffsets = nullptr;
            if (cmd->dynamicOffsetCount > 0) {
                dynamicOffsets = commands.NextData<uint32_t>(cmd->dynamicOffsetCount);
            }
            wgpu.renderBundleEncoderSetBindGroup(encoder, static_cast<uint32_t>(cmd->index),
                                                 ToBackend(cmd->group)->GetInnerHandle(),
                                                 cmd->dynamicOffsetCount, dynamicOffsets);
            break;
        }

        case Command::SetIndexBuffer: {
            auto cmd = commands.NextCommand<SetIndexBufferCmd>();
            wgpu.renderBundleEncoderSetIndexBuffer(encoder,
                                                   ToBackend(cmd->buffer)->GetInnerHandle(),
                                                   ToWGPU(cmd->format), cmd->offset, cmd->size);
            break;
        }

        case Command::SetRenderPipeline: {
            auto cmd = commands.NextCommand<SetRenderPipelineCmd>();
            wgpu.renderBundleEncoderSetPipeline(encoder,
                                                ToBackend(cmd->pipeline)->GetInnerHandle());
            break;
        }

        case Command::SetVertexBuffer: {
            auto cmd = commands.NextCommand<SetVertexBufferCmd>();
            wgpu.renderBundleEncoderSetVertexBuffer(encoder, static_cast<uint8_t>(cmd->slot),
                                                    ToBackend(cmd->buffer)->GetInnerHandle(),
                                                    cmd->offset, cmd->size);
            break;
        }

        case Command::SetImmediates: {
            auto cmd = commands.NextCommand<SetImmediatesCmd>();
            DAWN_ASSERT(cmd->size > 0);
            uint8_t* value = nullptr;
            value = commands.NextData<uint8_t>(cmd->size);
            wgpu.renderBundleEncoderSetImmediates(encoder, cmd->offset, value, cmd->size);
            break;
        }

        default:
            DAWN_UNREACHABLE();
            break;
    }
}

}  // anonymous namespace

// static
Ref<RenderBundleBase> RenderBundle::Create(RenderBundleEncoderBase* encoder,
                                           const RenderBundleDescriptor* descriptor,
                                           RenderPassResourceUsage usages,
                                           IndirectDrawMetadata indirectDrawMetaData) {
    return AcquireRef(
        new RenderBundle(encoder, descriptor, std::move(usages), std::move(indirectDrawMetaData)));
}

RenderBundle::RenderBundle(RenderBundleEncoderBase* encoder,
                           const RenderBundleDescriptor* descriptor,
                           RenderPassResourceUsage usages,
                           IndirectDrawMetadata indirectDrawMetaData)
    : RenderBundleBase(encoder,
                       descriptor,
                       encoder->AcquireAttachmentState(),
                       encoder->IsDepthReadOnly(),
                       encoder->IsStencilReadOnly(),
                       std::move(usages),
                       std::move(indirectDrawMetaData)),
      RecordableObject(schema::ObjectType::RenderBundle),
      ObjectWGPU(ToBackend(GetDevice())->wgpu->renderBundleRelease) {
    Device* device = ToBackend(GetDevice());

    const AttachmentState* attachmentState = GetAttachmentState();

    PerColorAttachment<WGPUTextureFormat> colorFormats = {};
    size_t colorAttachmentCount = 0;
    for (ColorAttachmentIndex i : attachmentState->GetColorAttachmentsMask()) {
        colorFormats[i] = ToAPI(attachmentState->GetColorAttachmentFormat(i));
        colorAttachmentCount = static_cast<size_t>(i) + 1;
    }

    WGPURenderBundleEncoderDescriptor bundleEncoderDescriptor =
        WGPU_RENDER_BUNDLE_ENCODER_DESCRIPTOR_INIT;
    bundleEncoderDescriptor.colorFormatCount = colorAttachmentCount;
    bundleEncoderDescriptor.colorFormats = colorFormats.data();
    if (attachmentState->HasDepthStencilAttachment()) {
        bundleEncoderDescriptor.depthStencilFormat =
            ToAPI(attachmentState->GetDepthStencilFormat());
    }
    bundleEncoderDescriptor.sampleCount = attachmentState->GetSampleCount();
    bundleEncoderDescriptor.depthReadOnly = IsDepthReadOnly();
    bundleEncoderDescriptor.stencilReadOnly = IsStencilReadOnly();

    WGPURenderBundleEncoder innerRenderBundleEncoder =
        device->wgpu->deviceCreateRenderBundleEncoder(device->GetInnerHandle(),
                                                      &bundleEncoderDescriptor);

    CommandIterator* iter = GetCommands();
    Command bundleCommandType;
    while (iter->NextCommandId(&bundleCommandType)) {
        EncodeRenderBundleCommand(device->wgpu.get(), innerRenderBundleEncoder, *iter,
                                  bundleCommandType);
    }

    mInnerHandle = device->wgpu->renderBundleEncoderFinish(innerRenderBundleEncoder, nullptr);
    DAWN_ASSERT(mInnerHandle);

    device->wgpu->renderBundleEncoderRelease(innerRenderBundleEncoder);
}

void RenderBundle::DestroyImpl(DestroyReason reason) {
    RenderBundleBase::DestroyImpl(reason);
    ToBackend(GetDevice())->wgpu->renderBundleRelease(mInnerHandle);
    mInnerHandle = nullptr;
}

void RenderBundle::SetLabelImpl() {
    ToBackend(GetDevice())->CaptureSetLabel(this, GetLabel());
}

MaybeError RenderBundle::AddReferenced(CaptureContext& captureContext) {
    CommandBufferResourceUsages usedResources;
    CommandIterator& commands = *GetCommands();
    Command type;
    while (commands.NextCommandId(&type)) {
        DAWN_TRY(GatherReferencedResourcesFromRenderCommand(captureContext, commands, usedResources,
                                                            type));
    }

    DAWN_TRY(AddUsedResources(captureContext, usedResources));

    return {};
}

MaybeError RenderBundle::CaptureCreationParameters(CaptureContext& captureContext) {
    std::vector<wgpu::TextureFormat> colorFormats;

    const AttachmentState* attachmentState = GetAttachmentState();

    for (ColorAttachmentIndex i : attachmentState->GetColorAttachmentsMask()) {
        colorFormats.push_back(attachmentState->GetColorAttachmentFormat(i));
    }

    schema::RenderBundle bundle{{
        .colorFormats = colorFormats,
        .depthStencilFormat = attachmentState->HasDepthStencilAttachment()
                                  ? attachmentState->GetDepthStencilFormat()
                                  : wgpu::TextureFormat::Undefined,
        .sampleCount = attachmentState->GetSampleCount(),
        .depthReadOnly = IsDepthReadOnly(),
        .stencilReadOnly = IsStencilReadOnly(),
    }};
    Serialize(captureContext, bundle);

    CommandIterator& commands = *GetCommands();
    Command type;
    while (commands.NextCommandId(&type)) {
        DAWN_TRY(CaptureRenderCommand(captureContext, commands, type));
    }
    Serialize(captureContext, schema::CommandBufferCommand::End);

    return {};
}

}  // namespace dawn::native::webgpu
