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

#include "dawn/native/webgpu/CommandBufferHelpers.h"

#include <vector>

#include "dawn/native/CommandAllocator.h"
#include "dawn/native/Commands.h"
#include "dawn/native/webgpu/BindGroupWGPU.h"
#include "dawn/native/webgpu/BufferWGPU.h"
#include "dawn/native/webgpu/CaptureContext.h"
#include "dawn/native/webgpu/ComputePipelineWGPU.h"
#include "dawn/native/webgpu/RenderBundleWGPU.h"
#include "dawn/native/webgpu/RenderPipelineWGPU.h"

namespace dawn::native::webgpu {

void CaptureSharedCommand(CaptureContext& captureContext, CommandIterator& commands, Command type) {
    switch (type) {
        case Command::SetBindGroup: {
            const auto& cmd = *commands.NextCommand<SetBindGroupCmd>();
            const uint32_t* dynamicOffsetsData =
                cmd.dynamicOffsetCount > 0 ? commands.NextData<uint32_t>(cmd.dynamicOffsetCount)
                                           : nullptr;
            schema::CommandBufferCommandSetBindGroupCmd data{{
                .data = {{
                    .index = uint32_t(cmd.index),
                    .bindGroupId = captureContext.GetId(cmd.group),
                    .dynamicOffsets = std::vector<uint32_t>(
                        dynamicOffsetsData, dynamicOffsetsData + cmd.dynamicOffsetCount),
                }},
            }};
            Serialize(captureContext, data);
            break;
        }
        case Command::SetImmediates: {
            const auto& cmd = *commands.NextCommand<SetImmediatesCmd>();
            const uint8_t* values = commands.NextData<uint8_t>(cmd.size);
            schema::CommandBufferCommandSetImmediatesCmd data{{
                .data = {{
                    .offset = cmd.offset,
                    .data = std::vector<uint8_t>(values, values + cmd.size),
                }},
            }};
            Serialize(captureContext, data);
            break;
        }
        default:
            DAWN_UNREACHABLE();
            break;
    }
}

// Returns true if command was handled
void CaptureDebugCommand(CaptureContext& captureContext, CommandIterator& commands, Command type) {
    switch (type) {
        case Command::PushDebugGroup: {
            const auto& cmd = *commands.NextCommand<PushDebugGroupCmd>();
            const char* label = commands.NextData<char>(cmd.length + 1);
            schema::CommandBufferCommandPushDebugGroupCmd data{{
                .data = {{
                    .groupLabel = label,
                }},
            }};
            Serialize(captureContext, data);
            break;
        }
        case Command::PopDebugGroup: {
            commands.NextCommand<PopDebugGroupCmd>();
            Serialize(captureContext, schema::CommandBufferCommand::PopDebugGroup);
            break;
        }
        case Command::InsertDebugMarker: {
            const auto& cmd = *commands.NextCommand<InsertDebugMarkerCmd>();
            const char* label = commands.NextData<char>(cmd.length + 1);
            schema::CommandBufferCommandInsertDebugMarkerCmd data{{
                .data = {{
                    .markerLabel = label,
                }},
            }};
            Serialize(captureContext, data);
            break;
        }
        default:
            DAWN_UNREACHABLE();
            break;
    }
}

// Captures commands common to a render pass and a render bundle
MaybeError CaptureRenderCommand(CaptureContext& captureContext,
                                CommandIterator& commands,
                                Command type) {
    switch (type) {
        case Command::SetRenderPipeline: {
            const auto& cmd = *commands.NextCommand<SetRenderPipelineCmd>();
            schema::CommandBufferCommandSetRenderPipelineCmd data{{
                .data = {{
                    .pipelineId = captureContext.GetId(cmd.pipeline.Get()),
                }},
            }};
            Serialize(captureContext, data);
            break;
        }
        case Command::SetVertexBuffer: {
            const auto& cmd = *commands.NextCommand<SetVertexBufferCmd>();
            schema::CommandBufferCommandSetVertexBufferCmd data{{
                .data = {{
                    .slot = uint32_t(cmd.slot),
                    .bufferId = captureContext.GetId(cmd.buffer),
                    .offset = cmd.offset,
                    .size = cmd.size,
                }},
            }};
            Serialize(captureContext, data);
            break;
        }
        case Command::SetIndexBuffer: {
            const auto& cmd = *commands.NextCommand<SetIndexBufferCmd>();
            schema::CommandBufferCommandSetIndexBufferCmd data{{
                .data = {{
                    .bufferId = captureContext.GetId(cmd.buffer),
                    .format = cmd.format,
                    .offset = cmd.offset,
                    .size = cmd.size,
                }},
            }};
            Serialize(captureContext, data);
            break;
        }
        case Command::Draw: {
            const auto& cmd = *commands.NextCommand<DrawCmd>();
            schema::CommandBufferCommandDrawCmd data{{
                .data = {{
                    .vertexCount = cmd.vertexCount,
                    .instanceCount = cmd.instanceCount,
                    .firstVertex = cmd.firstVertex,
                    .firstInstance = cmd.firstInstance,
                }},
            }};
            Serialize(captureContext, data);
            break;
        }
        case Command::DrawIndexed: {
            const auto& cmd = *commands.NextCommand<DrawIndexedCmd>();
            schema::CommandBufferCommandDrawIndexedCmd data{{
                .data = {{
                    .indexCount = cmd.indexCount,
                    .instanceCount = cmd.instanceCount,
                    .firstIndex = cmd.firstIndex,
                    .baseVertex = cmd.baseVertex,
                    .firstInstance = cmd.firstInstance,
                }},
            }};
            Serialize(captureContext, data);
            break;
        }
        case Command::DrawIndirect: {
            const auto& cmd = *commands.NextCommand<DrawIndirectCmd>();
            schema::CommandBufferCommandDrawIndirectCmd data{{
                .data = {{
                    .indirectBufferId = captureContext.GetId(cmd.indirectBuffer),
                    .indirectOffset = cmd.indirectOffset,
                }},
            }};
            Serialize(captureContext, data);
            break;
        }
        case Command::DrawIndexedIndirect: {
            const auto& cmd = *commands.NextCommand<DrawIndexedIndirectCmd>();
            schema::CommandBufferCommandDrawIndexedIndirectCmd data{{
                .data = {{
                    .indirectBufferId = captureContext.GetId(cmd.indirectBuffer),
                    .indirectOffset = cmd.indirectOffset,
                }},
            }};
            Serialize(captureContext, data);
            break;
        }
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

    return {};
}

// Gathers resources used by commands from both render passes and render bundles.
MaybeError GatherReferencedResourcesFromRenderCommand(CaptureContext& captureContext,
                                                      CommandIterator& commands,
                                                      CommandBufferResourceUsages& usedResources,
                                                      Command type) {
    switch (type) {
        case Command::SetRenderPipeline: {
            auto cmd = commands.NextCommand<SetRenderPipelineCmd>();
            usedResources.renderPipelines.push_back(cmd->pipeline.Get());
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
        case Command::BeginOcclusionQuery:
        case Command::EndOcclusionQuery:
        case Command::SetBlendConstant:
        case Command::SetScissorRect:
        case Command::SetStencilReference:
        case Command::SetViewport:
        case Command::Draw:
        case Command::DrawIndexed:
        case Command::DrawIndirect:
        case Command::DrawIndexedIndirect:
        case Command::WriteTimestamp:
        case Command::SetImmediates:
        case Command::SetIndexBuffer:
        case Command::SetVertexBuffer:
        case Command::PushDebugGroup:
        case Command::InsertDebugMarker:
        case Command::PopDebugGroup:
            SkipCommand(&commands, type);
            break;
        default:
            return DAWN_UNIMPLEMENTED_ERROR("Unimplemented command");
    }
    return {};
}

MaybeError AddUsedResources(CaptureContext& captureContext,
                            const CommandBufferResourceUsages& usedResources) {
    for (auto pipeline : usedResources.computePipelines) {
        DAWN_TRY(captureContext.AddResource(ToBackend(pipeline)));
    }
    for (auto pipeline : usedResources.renderPipelines) {
        DAWN_TRY(captureContext.AddResource(ToBackend(pipeline)));
    }
    for (auto bindGroup : usedResources.bindGroups) {
        DAWN_TRY(captureContext.AddResource(ToBackend(bindGroup)));
    }
    for (auto bundle : usedResources.renderBundles) {
        DAWN_TRY(captureContext.AddResource(ToBackend(bundle)));
    }
    return {};
}

}  // namespace dawn::native::webgpu
