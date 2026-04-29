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

#include "dawn/native/webgpu/CommandBufferWGPU.h"

#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/StringViewUtils.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/RenderBundle.h"
#include "dawn/native/webgpu/BindGroupWGPU.h"
#include "dawn/native/webgpu/BufferWGPU.h"
#include "dawn/native/webgpu/CaptureContext.h"
#include "dawn/native/webgpu/CommandBufferHelpers.h"
#include "dawn/native/webgpu/ComputePipelineWGPU.h"
#include "dawn/native/webgpu/DeviceWGPU.h"
#include "dawn/native/webgpu/QuerySetWGPU.h"
#include "dawn/native/webgpu/RenderBundleWGPU.h"
#include "dawn/native/webgpu/RenderPipelineWGPU.h"
#include "dawn/native/webgpu/Serialization.h"
#include "dawn/native/webgpu/TextureWGPU.h"
#include "dawn/native/webgpu/ToWGPU.h"

namespace dawn::native::webgpu {

// static
Ref<CommandBuffer> CommandBuffer::Create(CommandEncoder* encoder,
                                         const CommandBufferDescriptor* descriptor) {
    return AcquireRef(new CommandBuffer(encoder, descriptor));
}

CommandBuffer::CommandBuffer(CommandEncoder* encoder, const CommandBufferDescriptor* descriptor)
    : CommandBufferBase(encoder, descriptor), RecordableObject(schema::ObjectType::CommandBuffer) {}

void CommandBuffer::SetLabelImpl() {
    ToBackend(GetDevice())->CaptureSetLabel(this, GetLabel());
}

namespace {

void PrepareResourcesForSyncScope(const SyncScopeResourceUsage& scope) {
    for (size_t i = 0; i < scope.textures.size(); i++) {
        Texture* texture = ToBackend(scope.textures[i]);

        texture->SynchronizeTextureBeforeUse();

        // Mark subresources as cleared that are not render attachments as initialized.
        // Render attachments will be marked as cleared in Encode BeginRenderPass.
        // The actual clearing relies on the inner device.
        scope.textureSyncInfos[i].Iterate(
            [&](const SubresourceRange& range, const TextureSyncInfo& syncInfo) -> void {
                if (syncInfo.usage & ~wgpu::TextureUsage::RenderAttachment) {
                    texture->SetIsSubresourceContentInitialized(true, range);
                }
            });
    }

    for (BufferBase* bufferBase : scope.buffers) {
        bufferBase->SetInitialized(true);
    }
}

void EncodeComputePass(const DawnProcTable& wgpu,
                       WGPUCommandEncoder innerEncoder,
                       CommandIterator& commands,
                       BeginComputePassCmd* computePassCmd,
                       const ComputePassResourceUsage& resourceUsages) {
    WGPUComputePassDescriptor passDescriptor{
        .nextInChain = nullptr,
        .label = ToOutputStringView(computePassCmd->label),
        .timestampWrites = nullptr,
    };
    WGPUPassTimestampWrites timestampWrites;
    if (computePassCmd->timestampWrites.querySet) {
        timestampWrites = ToWGPU(computePassCmd->timestampWrites);
        passDescriptor.timestampWrites = &timestampWrites;
    }

    for (auto texture : resourceUsages.referencedTextures) {
        ToBackend(texture)->SynchronizeTextureBeforeUse();
    }

    WGPUComputePassEncoder passEncoder =
        wgpu.commandEncoderBeginComputePass(innerEncoder, &passDescriptor);

    size_t nextDispatchNumber = 0;
    Command type;
    while (commands.NextCommandId(&type)) {
        switch (type) {
            case Command::EndComputePass: {
                commands.NextCommand<EndComputePassCmd>();
                wgpu.computePassEncoderEnd(passEncoder);
                return;
            }

            case Command::Dispatch: {
                auto cmd = commands.NextCommand<DispatchCmd>();
                PrepareResourcesForSyncScope(resourceUsages.dispatchUsages[nextDispatchNumber]);
                ++nextDispatchNumber;
                wgpu.computePassEncoderDispatchWorkgroups(passEncoder, cmd->x, cmd->y, cmd->z);
                break;
            }

            case Command::DispatchIndirect: {
                auto cmd = commands.NextCommand<DispatchIndirectCmd>();
                PrepareResourcesForSyncScope(resourceUsages.dispatchUsages[nextDispatchNumber]);
                ++nextDispatchNumber;
                wgpu.computePassEncoderDispatchWorkgroupsIndirect(
                    passEncoder, ToBackend(cmd->indirectBuffer)->GetInnerHandle(),
                    cmd->indirectOffset);
                break;
            }

            case Command::SetComputePipeline: {
                auto cmd = commands.NextCommand<SetComputePipelineCmd>();
                wgpu.computePassEncoderSetPipeline(passEncoder,
                                                   ToBackend(cmd->pipeline)->GetInnerHandle());
                break;
            }

            case Command::SetBindGroup: {
                auto cmd = commands.NextCommand<SetBindGroupCmd>();
                uint32_t* dynamicOffsets = nullptr;
                if (cmd->dynamicOffsetCount > 0) {
                    dynamicOffsets = commands.NextData<uint32_t>(cmd->dynamicOffsetCount);
                }
                wgpu.computePassEncoderSetBindGroup(passEncoder, static_cast<uint32_t>(cmd->index),
                                                    ToBackend(cmd->group)->GetInnerHandle(),
                                                    cmd->dynamicOffsetCount, dynamicOffsets);
                break;
            }
            case Command::InsertDebugMarker: {
                auto cmd = commands.NextCommand<InsertDebugMarkerCmd>();
                char* label = commands.NextData<char>(cmd->length + 1);
                wgpu.computePassEncoderInsertDebugMarker(passEncoder, {label, cmd->length});
                break;
            }

            case Command::PopDebugGroup: {
                commands.NextCommand<PopDebugGroupCmd>();
                wgpu.computePassEncoderPopDebugGroup(passEncoder);
                break;
            }

            case Command::PushDebugGroup: {
                auto cmd = commands.NextCommand<PushDebugGroupCmd>();
                char* label = commands.NextData<char>(cmd->length + 1);
                wgpu.computePassEncoderPushDebugGroup(passEncoder, {label, cmd->length});
                break;
            }

            case Command::WriteTimestamp: {
                auto cmd = commands.NextCommand<WriteTimestampCmd>();
                wgpu.computePassEncoderWriteTimestamp(
                    passEncoder, ToBackend(cmd->querySet)->GetInnerHandle(), cmd->queryIndex);
                break;
            }

            case Command::SetImmediates: {
                auto cmd = commands.NextCommand<SetImmediatesCmd>();
                DAWN_ASSERT(cmd->size > 0);
                uint8_t* value = nullptr;
                value = commands.NextData<uint8_t>(cmd->size);
                wgpu.computePassEncoderSetImmediates(passEncoder, cmd->offset, value, cmd->size);
                break;
            }

            default: {
                DAWN_UNREACHABLE();
                break;
            }
        }
    }

    // EndComputePass should have been called
    DAWN_UNREACHABLE();
}

void EncodeRenderPass(const Device* device,
                      WGPUCommandEncoder innerEncoder,
                      CommandIterator& commands,
                      BeginRenderPassCmd* renderPassCmd) {
    const DawnProcTable& wgpu = device->wgpu.get();

    PerColorAttachment<WGPURenderPassColorAttachment> colorAttachments = {};

    size_t colorAttachmentCount = 0;
    for (auto i : renderPassCmd->attachmentState->GetColorAttachmentsMask()) {
        auto& colorAttachment = renderPassCmd->colorAttachments[i];
        colorAttachments[i] = ToWGPU(colorAttachment);
        colorAttachmentCount = static_cast<size_t>(i) + 1;
    }

    WGPURenderPassDescriptor passDescriptor{
        .nextInChain = nullptr,
        .label = ToOutputStringView(renderPassCmd->label),
        .colorAttachmentCount = colorAttachmentCount,
        .colorAttachments = colorAttachments.data(),
        .depthStencilAttachment = nullptr,
        .occlusionQuerySet = renderPassCmd->occlusionQuerySet
                                 ? ToBackend(renderPassCmd->occlusionQuerySet)->GetInnerHandle()
                                 : nullptr,
        .timestampWrites = nullptr,
    };
    WGPURenderPassDepthStencilAttachment depthStencilAttachment;
    if (renderPassCmd->attachmentState->HasDepthStencilAttachment()) {
        depthStencilAttachment = ToWGPU(renderPassCmd->depthStencilAttachment);
        passDescriptor.depthStencilAttachment = &depthStencilAttachment;
    }
    WGPUPassTimestampWrites timestampWrites;
    if (renderPassCmd->timestampWrites.querySet) {
        timestampWrites = ToWGPU(renderPassCmd->timestampWrites);
        passDescriptor.timestampWrites = &timestampWrites;
    }
    WGPURenderPassDescriptorResolveRect resolveRect;
    if (renderPassCmd->resolveRect.HasValue()) {
        resolveRect = WGPU_RENDER_PASS_DESCRIPTOR_RESOLVE_RECT_INIT;
        resolveRect.colorOffsetX = renderPassCmd->resolveRect.colorOffsetX;
        resolveRect.colorOffsetY = renderPassCmd->resolveRect.colorOffsetY;
        resolveRect.resolveOffsetX = renderPassCmd->resolveRect.resolveOffsetX;
        resolveRect.resolveOffsetY = renderPassCmd->resolveRect.resolveOffsetY;
        resolveRect.width = renderPassCmd->resolveRect.updateWidth;
        resolveRect.height = renderPassCmd->resolveRect.updateHeight;
        passDescriptor.nextInChain = &(resolveRect.chain);
    }
    WGPURenderPassEncoder passEncoder =
        wgpu.commandEncoderBeginRenderPass(innerEncoder, &passDescriptor);

    Command type;
    while (commands.NextCommandId(&type)) {
        switch (type) {
            case Command::EndRenderPass: {
                commands.NextCommand<EndRenderPassCmd>();
                wgpu.renderPassEncoderEnd(passEncoder);
                return;
            }

            case Command::SetBlendConstant: {
                auto cmd = commands.NextCommand<SetBlendConstantCmd>();
                WGPUColor color = ToWGPU(cmd->color);
                wgpu.renderPassEncoderSetBlendConstant(passEncoder, &color);
                break;
            }

            case Command::SetStencilReference: {
                auto cmd = commands.NextCommand<SetStencilReferenceCmd>();
                wgpu.renderPassEncoderSetStencilReference(passEncoder, cmd->reference);
                break;
            }

            case Command::SetViewport: {
                auto cmd = commands.NextCommand<SetViewportCmd>();
                wgpu.renderPassEncoderSetViewport(passEncoder, cmd->x, cmd->y, cmd->width,
                                                  cmd->height, cmd->minDepth, cmd->maxDepth);
                break;
            }

            case Command::SetScissorRect: {
                auto cmd = commands.NextCommand<SetScissorRectCmd>();
                wgpu.renderPassEncoderSetScissorRect(passEncoder, cmd->x, cmd->y, cmd->width,
                                                     cmd->height);
                break;
            }

            case Command::ExecuteBundles: {
                auto* cmd = commands.NextCommand<ExecuteBundlesCmd>();
                auto bundles = commands.NextData<Ref<RenderBundleBase>>(cmd->count);
                std::vector<WGPURenderBundle> wgpuBundles;
                wgpuBundles.reserve(cmd->count);

                for (uint32_t i = 0; i < cmd->count; ++i) {
                    wgpuBundles.push_back(ToBackend(bundles[i].Get())->GetInnerHandle());
                }
                wgpu.renderPassEncoderExecuteBundles(passEncoder, wgpuBundles.size(),
                                                     wgpuBundles.data());
                break;
            }

            case Command::BeginOcclusionQuery: {
                auto cmd = commands.NextCommand<BeginOcclusionQueryCmd>();
                wgpu.renderPassEncoderBeginOcclusionQuery(passEncoder, cmd->queryIndex);
                break;
            }

            case Command::EndOcclusionQuery: {
                commands.NextCommand<EndOcclusionQueryCmd>();
                wgpu.renderPassEncoderEndOcclusionQuery(passEncoder);
                break;
            }

            case Command::WriteTimestamp: {
                auto cmd = commands.NextCommand<WriteTimestampCmd>();
                wgpu.renderPassEncoderWriteTimestamp(
                    passEncoder, ToBackend(cmd->querySet)->GetInnerHandle(), cmd->queryIndex);
                break;
            }

            // The followings are commands shared with RenderBundleEncoder, but it's a bit hard to
            // share code with EncodeRenderBundleCommand since we are using webgpu C header.
            case Command::Draw: {
                auto cmd = commands.NextCommand<DrawCmd>();
                wgpu.renderPassEncoderDraw(passEncoder, cmd->vertexCount, cmd->instanceCount,
                                           cmd->firstVertex, cmd->firstInstance);
                break;
            }

            case Command::DrawIndexed: {
                auto cmd = commands.NextCommand<DrawIndexedCmd>();
                wgpu.renderPassEncoderDrawIndexed(passEncoder, cmd->indexCount, cmd->instanceCount,
                                                  cmd->firstIndex, cmd->baseVertex,
                                                  cmd->firstInstance);
                break;
            }

            case Command::DrawIndirect: {
                auto cmd = commands.NextCommand<DrawIndirectCmd>();
                wgpu.renderPassEncoderDrawIndirect(passEncoder,
                                                   ToBackend(cmd->indirectBuffer)->GetInnerHandle(),
                                                   cmd->indirectOffset);
                break;
            }

            case Command::DrawIndexedIndirect: {
                auto cmd = commands.NextCommand<DrawIndexedIndirectCmd>();
                wgpu.renderPassEncoderDrawIndexedIndirect(
                    passEncoder, ToBackend(cmd->indirectBuffer)->GetInnerHandle(),
                    cmd->indirectOffset);
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
                wgpu.renderPassEncoderInsertDebugMarker(passEncoder, {label, cmd->length});
                break;
            }

            case Command::PopDebugGroup: {
                commands.NextCommand<PopDebugGroupCmd>();
                wgpu.renderPassEncoderPopDebugGroup(passEncoder);
                break;
            }

            case Command::PushDebugGroup: {
                auto cmd = commands.NextCommand<PushDebugGroupCmd>();
                char* label = commands.NextData<char>(cmd->length + 1);
                wgpu.renderPassEncoderPushDebugGroup(passEncoder, {label, cmd->length});
                break;
            }

            case Command::SetBindGroup: {
                auto cmd = commands.NextCommand<SetBindGroupCmd>();
                uint32_t* dynamicOffsets = nullptr;
                if (cmd->dynamicOffsetCount > 0) {
                    dynamicOffsets = commands.NextData<uint32_t>(cmd->dynamicOffsetCount);
                }
                wgpu.renderPassEncoderSetBindGroup(passEncoder, static_cast<uint32_t>(cmd->index),
                                                   ToBackend(cmd->group)->GetInnerHandle(),
                                                   cmd->dynamicOffsetCount, dynamicOffsets);
                break;
            }

            case Command::SetIndexBuffer: {
                auto cmd = commands.NextCommand<SetIndexBufferCmd>();
                wgpu.renderPassEncoderSetIndexBuffer(passEncoder,
                                                     ToBackend(cmd->buffer)->GetInnerHandle(),
                                                     ToWGPU(cmd->format), cmd->offset, cmd->size);
                break;
            }

            case Command::SetRenderPipeline: {
                auto cmd = commands.NextCommand<SetRenderPipelineCmd>();
                wgpu.renderPassEncoderSetPipeline(passEncoder,
                                                  ToBackend(cmd->pipeline)->GetInnerHandle());
                break;
            }

            case Command::SetVertexBuffer: {
                auto cmd = commands.NextCommand<SetVertexBufferCmd>();
                wgpu.renderPassEncoderSetVertexBuffer(passEncoder, static_cast<uint8_t>(cmd->slot),
                                                      ToBackend(cmd->buffer)->GetInnerHandle(),
                                                      cmd->offset, cmd->size);
                break;
            }

            case Command::SetImmediates: {
                auto cmd = commands.NextCommand<SetImmediatesCmd>();
                DAWN_ASSERT(cmd->size > 0);
                uint8_t* value = nullptr;
                value = commands.NextData<uint8_t>(cmd->size);
                wgpu.renderPassEncoderSetImmediates(passEncoder, cmd->offset, value, cmd->size);
                break;
            }

            default: {
                DAWN_UNREACHABLE();
                break;
            }
        }
    }

    // EndRenderPass should have been called
    DAWN_UNREACHABLE();
}

MaybeError GatherReferencedResourcesFromComputePass(CaptureContext& captureContext,
                                                    CommandIterator& commands,
                                                    CommandBufferResourceUsages& usedResources) {
    Command type;
    while (commands.NextCommandId(&type)) {
        switch (type) {
            case Command::EndComputePass: {
                commands.NextCommand<EndComputePassCmd>();
                return {};
            }
            case Command::SetComputePipeline: {
                auto cmd = commands.NextCommand<SetComputePipelineCmd>();
                usedResources.computePipelines.push_back(cmd->pipeline.Get());
                break;
            }
            case Command::SetBindGroup: {
                auto cmd = commands.NextCommand<SetBindGroupCmd>();
                if (cmd->dynamicOffsetCount > 0) {
                    commands.NextData<uint32_t>(cmd->dynamicOffsetCount);
                }
                usedResources.bindGroups.push_back(cmd->group.Get());
                break;
            }
            case Command::Dispatch:
            case Command::DispatchIndirect:
            case Command::WriteTimestamp:
            case Command::SetImmediates:
            case Command::PushDebugGroup:
            case Command::InsertDebugMarker:
            case Command::PopDebugGroup:
                SkipCommand(&commands, type);
                break;
            default: {
                return DAWN_UNIMPLEMENTED_ERROR("Unimplemented command");
            }
        }
    }

    // EndComputePass should have been called
    DAWN_UNREACHABLE();
    return {};
}

MaybeError GatherReferencedResourcesFromRenderPass(CaptureContext& captureContext,
                                                   CommandIterator& commands,
                                                   CommandBufferResourceUsages& usedResources) {
    Command type;
    while (commands.NextCommandId(&type)) {
        switch (type) {
            case Command::EndRenderPass: {
                commands.NextCommand<EndRenderPassCmd>();
                return {};
            }
            case Command::ExecuteBundles: {
                auto cmd = commands.NextCommand<ExecuteBundlesCmd>();
                auto bundles = commands.NextData<Ref<RenderBundleBase>>(cmd->count);
                for (uint32_t i = 0; i < cmd->count; ++i) {
                    usedResources.renderBundles.push_back(bundles[i].Get());
                }
                break;
            }
            default:
                DAWN_TRY(GatherReferencedResourcesFromRenderCommand(captureContext, commands,
                                                                    usedResources, type));
                break;
        }
    }

    // EndComputePass should have been called
    DAWN_UNREACHABLE();
    return {};
}

void CaptureTimestampWriteCommand(CaptureContext& captureContext, CommandIterator& commands) {
    const auto& cmd = *commands.NextCommand<WriteTimestampCmd>();
    schema::CommandBufferCommandWriteTimestampCmd data{{
        .data = {{
            .querySetId = captureContext.GetId(cmd.querySet),
            .queryIndex = cmd.queryIndex,
        }},
    }};
    Serialize(captureContext, data);
}

MaybeError CaptureComputePass(CaptureContext& captureContext, CommandIterator& commands) {
    Command type;
    while (commands.NextCommandId(&type)) {
        switch (type) {
            case Command::EndComputePass: {
                commands.NextCommand<EndComputePassCmd>();
                Serialize(captureContext, schema::CommandBufferCommand::End);
                return {};
            }
            case Command::SetComputePipeline: {
                const auto& cmd = *commands.NextCommand<SetComputePipelineCmd>();
                schema::CommandBufferCommandSetComputePipelineCmd data{{
                    .data = {{
                        .pipelineId = captureContext.GetId(cmd.pipeline.Get()),
                    }},
                }};
                Serialize(captureContext, data);
                break;
            }
            case Command::Dispatch: {
                const auto& cmd = *commands.NextCommand<DispatchCmd>();
                schema::CommandBufferCommandDispatchCmd data{{
                    .data = {{
                        .x = cmd.x,
                        .y = cmd.y,
                        .z = cmd.z,
                    }},
                }};
                Serialize(captureContext, data);
                break;
            }
            case Command::DispatchIndirect: {
                const auto& cmd = *commands.NextCommand<DispatchIndirectCmd>();
                schema::CommandBufferCommandDispatchIndirectCmd data{{
                    .data = {{
                        .bufferId = captureContext.GetId(cmd.indirectBuffer),
                        .offset = cmd.indirectOffset,
                    }},
                }};
                Serialize(captureContext, data);
                break;
            }
            case Command::WriteTimestamp:
                CaptureTimestampWriteCommand(captureContext, commands);
                break;
            case Command::SetBindGroup:
            case Command::SetImmediates:
                CaptureSharedCommand(captureContext, commands, type);
                break;
            case Command::PushDebugGroup:
            case Command::PopDebugGroup:
            case Command::InsertDebugMarker:
                CaptureDebugCommand(captureContext, commands, type);
                break;
            default:
                return DAWN_UNIMPLEMENTED_ERROR("Unimplemented command");
        }
    }
    return {};
}

MaybeError CaptureRenderPass(CaptureContext& captureContext, CommandIterator& commands) {
    Command type;
    while (commands.NextCommandId(&type)) {
        switch (type) {
            case Command::EndRenderPass: {
                commands.NextCommand<EndRenderPassCmd>();
                Serialize(captureContext, schema::CommandBufferCommand::End);
                return {};
            }
            case Command::ExecuteBundles: {
                const auto& cmd = *commands.NextCommand<ExecuteBundlesCmd>();
                auto bundles = commands.NextData<Ref<RenderBundleBase>>(cmd.count);
                std::vector<schema::ObjectId> bundleIds;
                for (uint32_t i = 0; i < cmd.count; ++i) {
                    bundleIds.push_back(captureContext.GetId(bundles[i].Get()));
                }
                schema::CommandBufferCommandExecuteBundlesCmd data{{
                    .data = {{
                        .bundleIds = bundleIds,
                    }},
                }};
                Serialize(captureContext, data);
                break;
            }
            case Command::BeginOcclusionQuery: {
                const auto& cmd = *commands.NextCommand<BeginOcclusionQueryCmd>();
                schema::CommandBufferCommandBeginOcclusionQueryCmd data{{
                    .data = {{
                        .queryIndex = cmd.queryIndex,
                    }},
                }};
                Serialize(captureContext, data);
                break;
            }
            case Command::EndOcclusionQuery: {
                commands.NextCommand<EndOcclusionQueryCmd>();
                Serialize(captureContext, schema::CommandBufferCommand::EndOcclusionQuery);
                break;
            }
            case Command::SetBlendConstant: {
                const auto& cmd = *commands.NextCommand<SetBlendConstantCmd>();
                schema::CommandBufferCommandSetBlendConstantCmd data{{
                    .data = {{
                        .color = ToSchema(cmd.color),
                    }},
                }};
                Serialize(captureContext, data);
                break;
            }
            case Command::SetScissorRect: {
                const auto& cmd = *commands.NextCommand<SetScissorRectCmd>();
                schema::CommandBufferCommandSetScissorRectCmd data{{
                    .data = {{
                        .x = cmd.x,
                        .y = cmd.y,
                        .width = cmd.width,
                        .height = cmd.height,
                    }},
                }};
                Serialize(captureContext, data);
                break;
            }
            case Command::SetStencilReference: {
                const auto& cmd = *commands.NextCommand<SetStencilReferenceCmd>();
                schema::CommandBufferCommandSetStencilReferenceCmd data{{
                    .data = {{
                        .reference = cmd.reference,
                    }},
                }};
                Serialize(captureContext, data);
                break;
            }
            case Command::SetViewport: {
                const auto& cmd = *commands.NextCommand<SetViewportCmd>();
                schema::CommandBufferCommandSetViewportCmd data{{
                    .data = {{
                        .x = cmd.x,
                        .y = cmd.y,
                        .width = cmd.width,
                        .height = cmd.height,
                        .minDepth = cmd.minDepth,
                        .maxDepth = cmd.maxDepth,
                    }},
                }};
                Serialize(captureContext, data);
                break;
            }
            case Command::WriteTimestamp:
                CaptureTimestampWriteCommand(captureContext, commands);
                break;
            default:
                DAWN_TRY(CaptureRenderCommand(captureContext, commands, type));
                break;
        }
    }
    return {};
}

template <typename T>
MaybeError AddReferencedPassResourceUsages(CaptureContext& captureContext,
                                           const std::vector<T>& syncScopeResourceUsages) {
    for (const auto& usages : syncScopeResourceUsages) {
        for (auto buffer : usages.buffers) {
            DAWN_TRY(captureContext.AddResource(ToBackend(buffer)));
        }
        for (auto texture : usages.textures) {
            DAWN_TRY(captureContext.AddResource(ToBackend(texture)));
        }
    }
    return {};
}

}  // anonymous namespace

MaybeError CommandBuffer::AddReferenced(CaptureContext& captureContext) {
    const auto& resourceUsages = GetResourceUsages();
    for (auto buffer : resourceUsages.topLevelBuffers) {
        DAWN_TRY(captureContext.AddResource(ToBackend(buffer)));
    }
    for (auto texture : resourceUsages.topLevelTextures) {
        DAWN_TRY(captureContext.AddResource(ToBackend(texture)));
    }
    for (auto querySet : resourceUsages.usedQuerySets) {
        DAWN_TRY(captureContext.AddResource(ToBackend(querySet)));
    }
    DAWN_TRY(AddReferencedPassResourceUsages(captureContext, resourceUsages.renderPasses));
    for (const auto& pass : resourceUsages.computePasses) {
        DAWN_TRY(AddReferencedPassResourceUsages(captureContext, pass.dispatchUsages));
    }

    CommandBufferResourceUsages usedResources;

    CommandIterator& commands = mCommands;
    Command type;
    while (commands.NextCommandId(&type)) {
        switch (type) {
            case Command::BeginComputePass: {
                const auto& cmd = *commands.NextCommand<BeginComputePassCmd>();
                if (cmd.timestampWrites.querySet != nullptr) {
                    DAWN_TRY(
                        captureContext.AddResource(ToBackend(cmd.timestampWrites.querySet.Get())));
                }
                DAWN_TRY(GatherReferencedResourcesFromComputePass(captureContext, commands,
                                                                  usedResources));
                break;
            }
            case Command::BeginRenderPass: {
                const auto& cmd = *commands.NextCommand<BeginRenderPassCmd>();
                for (const auto& attachment : cmd.colorAttachments) {
                    if (attachment.view != nullptr) {
                        DAWN_TRY(captureContext.AddResource(ToBackend(attachment.view.Get())));
                    }
                    if (attachment.resolveTarget != nullptr) {
                        DAWN_TRY(
                            captureContext.AddResource(ToBackend(attachment.resolveTarget.Get())));
                    }
                }
                if (cmd.depthStencilAttachment.view != nullptr) {
                    DAWN_TRY(captureContext.AddResource(
                        ToBackend(cmd.depthStencilAttachment.view.Get())));
                }
                if (cmd.timestampWrites.querySet != nullptr) {
                    DAWN_TRY(
                        captureContext.AddResource(ToBackend(cmd.timestampWrites.querySet.Get())));
                }
                if (cmd.occlusionQuerySet != nullptr) {
                    DAWN_TRY(captureContext.AddResource(ToBackend(cmd.occlusionQuerySet.Get())));
                }
                DAWN_TRY(GatherReferencedResourcesFromRenderPass(captureContext, commands,
                                                                 usedResources));
                break;
            }
            case Command::WriteBuffer:
            case Command::ClearBuffer:
            case Command::CopyBufferToBuffer:
            case Command::CopyBufferToTexture:
            case Command::CopyTextureToBuffer:
            case Command::CopyTextureToTexture:
            case Command::ResolveQuerySet:
            case Command::WriteTimestamp:
            case Command::PushDebugGroup:
            case Command::InsertDebugMarker:
            case Command::PopDebugGroup:
                SkipCommand(&commands, type);
                break;
            default: {
                return DAWN_UNIMPLEMENTED_ERROR("Unimplemented command");
            }
        }
    }

    DAWN_TRY(AddUsedResources(captureContext, usedResources));

    return {};
}

schema::ColorAttachment ToSchema(CaptureContext& captureContext,
                                 const RenderPassColorAttachmentInfo& info) {
    return {{
        .viewId = captureContext.GetId(info.view),
        .depthSlice = info.view->GetDimension() == wgpu::TextureViewDimension::e3D
                          ? info.depthSlice
                          : wgpu::kDepthSliceUndefined,
        .resolveTargetId = captureContext.GetId(info.resolveTarget),
        .loadOp = info.loadOp,
        .storeOp = info.storeOp,
        .clearValue = ToSchema(info.clearColor),
    }};
}

schema::RenderPassDepthStencilAttachment ToSchema(
    CaptureContext& captureContext,
    const RenderPassDepthStencilAttachmentInfo& info) {
    // The front end does not save the user's actual loadOp/storeOp settings so we derive what they
    // were.
    // TODO(460491958): Save the actual user's loadOp/storeOp settings and adjust the backends.
    wgpu::LoadOp depthLoadOp = info.depthLoadOp;
    wgpu::StoreOp depthStoreOp = info.depthStoreOp;
    wgpu::LoadOp stencilLoadOp = info.stencilLoadOp;
    wgpu::StoreOp stencilStoreOp = info.stencilStoreOp;

    bool haveAttachment = info.view != nullptr;
    bool haveDepth = haveAttachment && info.view->GetFormat().HasDepth();
    bool haveStencil = haveAttachment && info.view->GetFormat().HasStencil();

    if (!haveAttachment || !haveDepth || info.depthReadOnly) {
        depthLoadOp = wgpu::LoadOp::Undefined;
        depthStoreOp = wgpu::StoreOp::Undefined;
    }

    if (!haveAttachment || !haveStencil || info.stencilReadOnly) {
        stencilLoadOp = wgpu::LoadOp::Undefined;
        stencilStoreOp = wgpu::StoreOp::Undefined;
    }

    return {{
        .viewId = captureContext.GetId(info.view),
        .depthLoadOp = depthLoadOp,
        .depthStoreOp = depthStoreOp,
        .depthClearValue = info.clearDepth,
        .depthReadOnly = info.depthReadOnly,
        .stencilLoadOp = stencilLoadOp,
        .stencilStoreOp = stencilStoreOp,
        .stencilClearValue = info.clearStencil,
        .stencilReadOnly = info.stencilReadOnly,
    }};
}

MaybeError CommandBuffer::CaptureCreationParameters(CaptureContext& captureContext) {
    CommandIterator& commands = mCommands;
    Command type;
    while (commands.NextCommandId(&type)) {
        switch (type) {
            case Command::CopyBufferToBuffer: {
                const auto& cmd = *commands.NextCommand<CopyBufferToBufferCmd>();
                schema::CommandBufferCommandCopyBufferToBufferCmd data{{
                    .data = {{
                        .srcBufferId = captureContext.GetId(cmd.source.Get()),
                        .srcOffset = cmd.sourceOffset,
                        .dstBufferId = captureContext.GetId(cmd.destination.Get()),
                        .dstOffset = cmd.destinationOffset,
                        .size = cmd.size,
                    }},
                }};
                Serialize(captureContext, data);
                break;
            }
            case Command::CopyBufferToTexture: {
                const auto& cmd = *commands.NextCommand<CopyBufferToTextureCmd>();
                const TypedTexelBlockInfo& blockInfo = GetBlockInfo(cmd.destination);
                schema::CommandBufferCommandCopyBufferToTextureCmd data{{
                    .data = {{
                        .source = ToSchema(captureContext, cmd.source, blockInfo),
                        .destination = ToSchema(captureContext, cmd.destination),
                        .copySize = ToSchema(cmd.copySize),
                    }},
                }};
                Serialize(captureContext, data);
                break;
            }
            case Command::CopyTextureToBuffer: {
                const auto& cmd = *commands.NextCommand<CopyTextureToBufferCmd>();
                const TypedTexelBlockInfo& blockInfo = GetBlockInfo(cmd.source);
                schema::CommandBufferCommandCopyTextureToBufferCmd data{{
                    .data = {{
                        .source = ToSchema(captureContext, cmd.source),
                        .destination = ToSchema(captureContext, cmd.destination, blockInfo),
                        .copySize = ToSchema(cmd.copySize),
                    }},
                }};
                Serialize(captureContext, data);
                break;
            }
            case Command::CopyTextureToTexture: {
                const auto& cmd = *commands.NextCommand<CopyTextureToTextureCmd>();
                schema::CommandBufferCommandCopyTextureToTextureCmd data{{
                    .data = {{
                        .source = ToSchema(captureContext, cmd.source),
                        .destination = ToSchema(captureContext, cmd.destination),
                        .copySize = ToSchema(cmd.copySize),
                    }},
                }};
                Serialize(captureContext, data);
                break;
            }
            case Command::ClearBuffer: {
                const auto& cmd = *commands.NextCommand<ClearBufferCmd>();
                schema::CommandBufferCommandClearBufferCmd data{{
                    .data = {{
                        .bufferId = captureContext.GetId(cmd.buffer.Get()),
                        .offset = cmd.offset,
                        .size = cmd.size,
                    }},
                }};
                Serialize(captureContext, data);
                break;
            }
            case Command::WriteBuffer: {
                const auto& cmd = *commands.NextCommand<WriteBufferCmd>();
                auto values = mCommands.NextData<uint8_t>(cmd.size);
                schema::CommandBufferCommandWriteBufferCmd data{{
                    .data = {{
                        .bufferId = captureContext.GetId(cmd.buffer.Get()),
                        .bufferOffset = cmd.offset,
                        .data = std::vector<uint8_t>(values, values + cmd.size),
                    }},
                }};
                Serialize(captureContext, data);
                break;
            }
            case Command::BeginComputePass: {
                const auto& cmd = *commands.NextCommand<BeginComputePassCmd>();
                schema::CommandBufferCommandBeginComputePassCmd data{{
                    .data = {{
                        .label = cmd.label,
                        .timestampWrites = ToSchema(captureContext, cmd.timestampWrites),
                    }},
                }};
                Serialize(captureContext, data);
                // Capture commands inside the compute pass
                DAWN_TRY(CaptureComputePass(captureContext, commands));
                break;
            }
            case Command::BeginRenderPass: {
                const auto& cmd = *commands.NextCommand<BeginRenderPassCmd>();

                // The front end does not store the number of attachments but the API requires that
                // we provide them for sparse attachments so we initialize colorAttachments with
                // enough slots to cover all used slots and fill them with a state that will be set
                // to unused on replay.
                ColorAttachmentMask attachmentMask = cmd.attachmentState->GetColorAttachmentsMask();
                ColorAttachmentIndex attachmentCount = GetHighestBitIndexPlusOne(attachmentMask);

                std::vector<schema::ColorAttachment> colorAttachments(size_t(attachmentCount),
                                                                      schema::ColorAttachment{});
                for (ColorAttachmentIndex slot : attachmentMask) {
                    colorAttachments[size_t(slot)] =
                        ToSchema(captureContext, cmd.colorAttachments[slot]);
                }

                schema::ResolveRect resolveRect = {};
                if (cmd.resolveRect.HasValue()) {
                    resolveRect.colorOffsetX = cmd.resolveRect.colorOffsetX;
                    resolveRect.colorOffsetY = cmd.resolveRect.colorOffsetY;
                    resolveRect.resolveOffsetX = cmd.resolveRect.resolveOffsetX;
                    resolveRect.resolveOffsetY = cmd.resolveRect.resolveOffsetY;
                    resolveRect.width = cmd.resolveRect.updateWidth;
                    resolveRect.height = cmd.resolveRect.updateHeight;
                }

                schema::CommandBufferCommandBeginRenderPassCmd data{{
                    .data = {{
                        .label = cmd.label,
                        .colorAttachments = colorAttachments,
                        .depthStencilAttachment =
                            ToSchema(captureContext, cmd.depthStencilAttachment),
                        .occlusionQuerySetId = captureContext.GetId(cmd.occlusionQuerySet.Get()),
                        .timestampWrites = ToSchema(captureContext, cmd.timestampWrites),
                        .resolveRect = resolveRect,
                    }},
                }};
                Serialize(captureContext, data);
                // Capture commands inside the compute pass
                DAWN_TRY(CaptureRenderPass(captureContext, commands));
                break;
            }
            case Command::ResolveQuerySet: {
                const auto& cmd = *commands.NextCommand<ResolveQuerySetCmd>();
                schema::CommandBufferCommandResolveQuerySetCmd data{{
                    .data = {{
                        .querySetId = captureContext.GetId(cmd.querySet.Get()),
                        .firstQuery = cmd.firstQuery,
                        .queryCount = cmd.queryCount,
                        .destinationId = captureContext.GetId(cmd.destination.Get()),
                        .destinationOffset = cmd.destinationOffset,
                    }},
                }};
                Serialize(captureContext, data);
                break;
            }
            case Command::WriteTimestamp:
                CaptureTimestampWriteCommand(captureContext, commands);
                break;
            case Command::PushDebugGroup:
            case Command::PopDebugGroup:
            case Command::InsertDebugMarker:
                CaptureDebugCommand(captureContext, commands, type);
                break;
            default:
                return DAWN_UNIMPLEMENTED_ERROR("Unimplemented command");
        }
    }
    Serialize(captureContext, schema::CommandBufferCommand::End);
    return {};
}

ResultOrError<WGPUCommandBuffer> CommandBuffer::Encode() {
    auto& wgpu = ToBackend(GetDevice())->wgpu.get();

    // TODO(crbug.com/413053623): Use stored command encoder descriptor
    WGPUCommandEncoder innerEncoder =
        wgpu.deviceCreateCommandEncoder(ToBackend(GetDevice())->GetInnerHandle(), nullptr);

    size_t nextComputePassNumber = 0;
    size_t nextRenderPassNumber = 0;

    Command type;
    while (mCommands.NextCommandId(&type)) {
        switch (type) {
            case Command::BeginComputePass: {
                BeginComputePassCmd* cmd = mCommands.NextCommand<BeginComputePassCmd>();
                EncodeComputePass(wgpu, innerEncoder, mCommands, cmd,
                                  GetResourceUsages().computePasses[nextComputePassNumber]);
                ++nextComputePassNumber;
                break;
            }
            case Command::BeginRenderPass: {
                auto cmd = mCommands.NextCommand<BeginRenderPassCmd>();

                // Call PrepareResourcesForSyncScope instead of manually set every texture with
                // SetInitialized(true) to properly manage initialization state especially for
                // imported shared texture.
                PrepareResourcesForSyncScope(
                    GetResourceUsages().renderPasses[nextRenderPassNumber]);

                DAWN_TRY(LazyClearRenderPassAttachments(
                    GetDevice(), cmd,
                    [](TextureBase* texture, const SubresourceRange& range) -> MaybeError {
                        // The WGPU backend doesn't clear directly, it relies on the the inner
                        // device so we just mark the subresource as initialized.
                        ToBackend(texture)->SetIsSubresourceContentInitialized(true, range);
                        return {};
                    }));
                EncodeRenderPass(ToBackend(GetDevice()), innerEncoder, mCommands, cmd);

                ++nextRenderPassNumber;
                break;
            }
            case Command::CopyBufferToBuffer: {
                auto copy = mCommands.NextCommand<CopyBufferToBufferCmd>();
                wgpu.commandEncoderCopyBufferToBuffer(
                    innerEncoder, ToBackend(copy->source)->GetInnerHandle(), copy->sourceOffset,
                    ToBackend(copy->destination)->GetInnerHandle(), copy->destinationOffset,
                    copy->size);
                break;
            }
            case Command::CopyBufferToTexture: {
                auto cmd = mCommands.NextCommand<CopyBufferToTextureCmd>();
                const TypedTexelBlockInfo& blockInfo = GetBlockInfo(cmd->destination);
                WGPUTexelCopyBufferInfo source = ToWGPU(cmd->source, blockInfo);
                WGPUTexelCopyTextureInfo destination = ToWGPU(cmd->destination);
                WGPUExtent3D size = ToWGPU(cmd->copySize);
                ToBackend(cmd->destination.texture.Get())->SynchronizeTextureBeforeUse();
                wgpu.commandEncoderCopyBufferToTexture(innerEncoder, &source, &destination, &size);
                cmd->destination.texture.Get()->SetIsSubresourceContentInitialized(
                    true, GetSubresourcesAffectedByCopy(cmd->destination, cmd->copySize));
                break;
            }
            case Command::CopyTextureToBuffer: {
                auto cmd = mCommands.NextCommand<CopyTextureToBufferCmd>();
                const TypedTexelBlockInfo& blockInfo = GetBlockInfo(cmd->source);
                WGPUTexelCopyTextureInfo source = ToWGPU(cmd->source);
                WGPUTexelCopyBufferInfo destination = ToWGPU(cmd->destination, blockInfo);
                WGPUExtent3D size = ToWGPU(cmd->copySize);
                ToBackend(cmd->source.texture.Get())->SynchronizeTextureBeforeUse();
                wgpu.commandEncoderCopyTextureToBuffer(innerEncoder, &source, &destination, &size);
                break;
            }
            case Command::CopyTextureToTexture: {
                auto cmd = mCommands.NextCommand<CopyTextureToTextureCmd>();
                WGPUTexelCopyTextureInfo source = ToWGPU(cmd->source);
                WGPUTexelCopyTextureInfo destination = ToWGPU(cmd->destination);
                WGPUExtent3D size = ToWGPU(cmd->copySize);
                ToBackend(cmd->source.texture.Get())->SynchronizeTextureBeforeUse();
                ToBackend(cmd->destination.texture.Get())->SynchronizeTextureBeforeUse();
                wgpu.commandEncoderCopyTextureToTexture(innerEncoder, &source, &destination, &size);
                cmd->destination.texture.Get()->SetIsSubresourceContentInitialized(
                    true, GetSubresourcesAffectedByCopy(cmd->destination, cmd->copySize));
                break;
            }
            case Command::ClearBuffer: {
                auto cmd = mCommands.NextCommand<ClearBufferCmd>();
                wgpu.commandEncoderClearBuffer(
                    innerEncoder, ToBackend(cmd->buffer)->GetInnerHandle(), cmd->offset, cmd->size);
                break;
            }
            case Command::ResolveQuerySet: {
                auto cmd = mCommands.NextCommand<ResolveQuerySetCmd>();
                wgpu.commandEncoderResolveQuerySet(
                    innerEncoder, ToBackend(cmd->querySet)->GetInnerHandle(), cmd->firstQuery,
                    cmd->queryCount, ToBackend(cmd->destination)->GetInnerHandle(),
                    cmd->destinationOffset);
                break;
            }
            case Command::WriteTimestamp: {
                auto cmd = mCommands.NextCommand<WriteTimestampCmd>();
                wgpu.commandEncoderWriteTimestamp(
                    innerEncoder, ToBackend(cmd->querySet)->GetInnerHandle(), cmd->queryIndex);
                break;
            }
            case Command::InsertDebugMarker: {
                auto cmd = mCommands.NextCommand<InsertDebugMarkerCmd>();
                char* label = mCommands.NextData<char>(cmd->length + 1);
                wgpu.commandEncoderInsertDebugMarker(innerEncoder, {label, cmd->length});
                break;
            }
            case Command::PopDebugGroup: {
                mCommands.NextCommand<PopDebugGroupCmd>();
                wgpu.commandEncoderPopDebugGroup(innerEncoder);
                break;
            }
            case Command::PushDebugGroup: {
                auto cmd = mCommands.NextCommand<PushDebugGroupCmd>();
                char* label = mCommands.NextData<char>(cmd->length + 1);
                wgpu.commandEncoderPushDebugGroup(innerEncoder, {label, cmd->length});
                break;
            }
            case Command::WriteBuffer: {
                auto cmd = mCommands.NextCommand<WriteBufferCmd>();
                auto data = mCommands.NextData<uint8_t>(cmd->size);
                wgpu.commandEncoderWriteBuffer(innerEncoder,
                                               ToBackend(cmd->buffer)->GetInnerHandle(),
                                               cmd->offset, data, cmd->size);
                break;
            }
            default:
                DAWN_UNREACHABLE();
        }
    }

    // TODO(crbug.com/413053623): Store WGPUCommandBufferDescriptor and assign here.
    WGPUCommandBuffer result = wgpu.commandEncoderFinish(innerEncoder, nullptr);
    wgpu.commandEncoderRelease(innerEncoder);
    return result;
}

}  // namespace dawn::native::webgpu
