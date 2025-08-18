// Copyright 2018 The Dawn & Tint Authors
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

#include "dawn/native/Commands.h"

#include "dawn/native/BindGroup.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/CommandAllocator.h"
#include "dawn/native/ComputePipeline.h"
#include "dawn/native/QuerySet.h"
#include "dawn/native/RenderBundle.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/Texture.h"

namespace dawn::native {

void FreeCommands(CommandIterator* commands) {
    commands->Reset();

    Command type;
    while (commands->NextCommandId(&type)) {
        switch (type) {
            case Command::BeginComputePass: {
                BeginComputePassCmd* begin = commands->NextCommand<BeginComputePassCmd>();
                begin->~BeginComputePassCmd();
                break;
            }
            case Command::BeginOcclusionQuery: {
                BeginOcclusionQueryCmd* begin = commands->NextCommand<BeginOcclusionQueryCmd>();
                begin->~BeginOcclusionQueryCmd();
                break;
            }
            case Command::BeginRenderPass: {
                BeginRenderPassCmd* begin = commands->NextCommand<BeginRenderPassCmd>();
                begin->~BeginRenderPassCmd();
                break;
            }
            case Command::CopyBufferToBuffer: {
                CopyBufferToBufferCmd* copy = commands->NextCommand<CopyBufferToBufferCmd>();
                copy->~CopyBufferToBufferCmd();
                break;
            }
            case Command::CopyBufferToTexture: {
                CopyBufferToTextureCmd* copy = commands->NextCommand<CopyBufferToTextureCmd>();
                copy->~CopyBufferToTextureCmd();
                break;
            }
            case Command::CopyTextureToBuffer: {
                CopyTextureToBufferCmd* copy = commands->NextCommand<CopyTextureToBufferCmd>();
                copy->~CopyTextureToBufferCmd();
                break;
            }
            case Command::CopyTextureToTexture: {
                CopyTextureToTextureCmd* copy = commands->NextCommand<CopyTextureToTextureCmd>();
                copy->~CopyTextureToTextureCmd();
                break;
            }
            case Command::Dispatch: {
                DispatchCmd* dispatch = commands->NextCommand<DispatchCmd>();
                dispatch->~DispatchCmd();
                break;
            }
            case Command::DispatchIndirect: {
                DispatchIndirectCmd* dispatch = commands->NextCommand<DispatchIndirectCmd>();
                dispatch->~DispatchIndirectCmd();
                break;
            }
            case Command::Draw: {
                DrawCmd* draw = commands->NextCommand<DrawCmd>();
                draw->~DrawCmd();
                break;
            }
            case Command::DrawIndexed: {
                DrawIndexedCmd* draw = commands->NextCommand<DrawIndexedCmd>();
                draw->~DrawIndexedCmd();
                break;
            }
            case Command::DrawIndirect: {
                DrawIndirectCmd* draw = commands->NextCommand<DrawIndirectCmd>();
                draw->~DrawIndirectCmd();
                break;
            }
            case Command::DrawIndexedIndirect: {
                DrawIndexedIndirectCmd* draw = commands->NextCommand<DrawIndexedIndirectCmd>();
                draw->~DrawIndexedIndirectCmd();
                break;
            }
            case Command::MultiDrawIndirect: {
                MultiDrawIndirectCmd* cmd = commands->NextCommand<MultiDrawIndirectCmd>();
                cmd->~MultiDrawIndirectCmd();
                break;
            }
            case Command::MultiDrawIndexedIndirect: {
                MultiDrawIndexedIndirectCmd* cmd =
                    commands->NextCommand<MultiDrawIndexedIndirectCmd>();
                cmd->~MultiDrawIndexedIndirectCmd();
                break;
            }

            case Command::EndComputePass: {
                EndComputePassCmd* cmd = commands->NextCommand<EndComputePassCmd>();
                cmd->~EndComputePassCmd();
                break;
            }
            case Command::EndOcclusionQuery: {
                EndOcclusionQueryCmd* cmd = commands->NextCommand<EndOcclusionQueryCmd>();
                cmd->~EndOcclusionQueryCmd();
                break;
            }
            case Command::EndRenderPass: {
                EndRenderPassCmd* cmd = commands->NextCommand<EndRenderPassCmd>();
                cmd->~EndRenderPassCmd();
                break;
            }
            case Command::ExecuteBundles: {
                ExecuteBundlesCmd* cmd = commands->NextCommand<ExecuteBundlesCmd>();
                auto bundles = commands->NextData<Ref<RenderBundleBase>>(cmd->count);
                for (size_t i = 0; i < cmd->count; ++i) {
                    (&bundles[i])->~Ref<RenderBundleBase>();
                }
                cmd->~ExecuteBundlesCmd();
                break;
            }
            case Command::ClearBuffer: {
                ClearBufferCmd* cmd = commands->NextCommand<ClearBufferCmd>();
                cmd->~ClearBufferCmd();
                break;
            }
            case Command::PixelLocalStorageBarrier: {
                PixelLocalStorageBarrierCmd* cmd =
                    commands->NextCommand<PixelLocalStorageBarrierCmd>();
                cmd->~PixelLocalStorageBarrierCmd();
                break;
            }
            case Command::InsertDebugMarker: {
                InsertDebugMarkerCmd* cmd = commands->NextCommand<InsertDebugMarkerCmd>();
                commands->NextData<char>(cmd->length + 1);
                cmd->~InsertDebugMarkerCmd();
                break;
            }
            case Command::PopDebugGroup: {
                PopDebugGroupCmd* cmd = commands->NextCommand<PopDebugGroupCmd>();
                cmd->~PopDebugGroupCmd();
                break;
            }
            case Command::PushDebugGroup: {
                PushDebugGroupCmd* cmd = commands->NextCommand<PushDebugGroupCmd>();
                commands->NextData<char>(cmd->length + 1);
                cmd->~PushDebugGroupCmd();
                break;
            }
            case Command::ResolveQuerySet: {
                ResolveQuerySetCmd* cmd = commands->NextCommand<ResolveQuerySetCmd>();
                cmd->~ResolveQuerySetCmd();
                break;
            }
            case Command::SetComputePipeline: {
                SetComputePipelineCmd* cmd = commands->NextCommand<SetComputePipelineCmd>();
                cmd->~SetComputePipelineCmd();
                break;
            }
            case Command::SetRenderPipeline: {
                SetRenderPipelineCmd* cmd = commands->NextCommand<SetRenderPipelineCmd>();
                cmd->~SetRenderPipelineCmd();
                break;
            }
            case Command::SetStencilReference: {
                SetStencilReferenceCmd* cmd = commands->NextCommand<SetStencilReferenceCmd>();
                cmd->~SetStencilReferenceCmd();
                break;
            }
            case Command::SetViewport: {
                SetViewportCmd* cmd = commands->NextCommand<SetViewportCmd>();
                cmd->~SetViewportCmd();
                break;
            }
            case Command::SetScissorRect: {
                SetScissorRectCmd* cmd = commands->NextCommand<SetScissorRectCmd>();
                cmd->~SetScissorRectCmd();
                break;
            }
            case Command::SetBlendConstant: {
                SetBlendConstantCmd* cmd = commands->NextCommand<SetBlendConstantCmd>();
                cmd->~SetBlendConstantCmd();
                break;
            }
            case Command::SetBindGroup: {
                SetBindGroupCmd* cmd = commands->NextCommand<SetBindGroupCmd>();
                if (cmd->dynamicOffsetCount > 0) {
                    commands->NextData<uint32_t>(cmd->dynamicOffsetCount);
                }
                cmd->~SetBindGroupCmd();
                break;
            }
            case Command::SetImmediateData: {
                SetImmediateDataCmd* cmd = commands->NextCommand<SetImmediateDataCmd>();
                if (cmd->size > 0) {
                    commands->NextData<uint8_t>(cmd->size);
                }
                cmd->~SetImmediateDataCmd();
                break;
            }
            case Command::SetIndexBuffer: {
                SetIndexBufferCmd* cmd = commands->NextCommand<SetIndexBufferCmd>();
                cmd->~SetIndexBufferCmd();
                break;
            }
            case Command::SetVertexBuffer: {
                SetVertexBufferCmd* cmd = commands->NextCommand<SetVertexBufferCmd>();
                cmd->~SetVertexBufferCmd();
                break;
            }
            case Command::WriteBuffer: {
                WriteBufferCmd* write = commands->NextCommand<WriteBufferCmd>();
                commands->NextData<uint8_t>(write->size);
                write->~WriteBufferCmd();
                break;
            }
            case Command::WriteTimestamp: {
                WriteTimestampCmd* cmd = commands->NextCommand<WriteTimestampCmd>();
                cmd->~WriteTimestampCmd();
                break;
            }
        }
    }

    commands->MakeEmptyAsDataWasDestroyed();
}

void SkipCommand(CommandIterator* commands, Command type) {
    switch (type) {
        case Command::BeginComputePass:
            commands->NextCommand<BeginComputePassCmd>();
            break;

        case Command::BeginOcclusionQuery:
            commands->NextCommand<BeginOcclusionQueryCmd>();
            break;

        case Command::BeginRenderPass:
            commands->NextCommand<BeginRenderPassCmd>();
            break;

        case Command::CopyBufferToBuffer:
            commands->NextCommand<CopyBufferToBufferCmd>();
            break;

        case Command::CopyBufferToTexture:
            commands->NextCommand<CopyBufferToTextureCmd>();
            break;

        case Command::CopyTextureToBuffer:
            commands->NextCommand<CopyTextureToBufferCmd>();
            break;

        case Command::CopyTextureToTexture:
            commands->NextCommand<CopyTextureToTextureCmd>();
            break;

        case Command::Dispatch:
            commands->NextCommand<DispatchCmd>();
            break;

        case Command::DispatchIndirect:
            commands->NextCommand<DispatchIndirectCmd>();
            break;

        case Command::Draw:
            commands->NextCommand<DrawCmd>();
            break;

        case Command::DrawIndexed:
            commands->NextCommand<DrawIndexedCmd>();
            break;

        case Command::DrawIndirect:
            commands->NextCommand<DrawIndirectCmd>();
            break;

        case Command::DrawIndexedIndirect:
            commands->NextCommand<DrawIndexedIndirectCmd>();
            break;

        case Command::MultiDrawIndirect:
            commands->NextCommand<MultiDrawIndirectCmd>();
            break;

        case Command::MultiDrawIndexedIndirect:
            commands->NextCommand<MultiDrawIndexedIndirectCmd>();
            break;

        case Command::EndComputePass:
            commands->NextCommand<EndComputePassCmd>();
            break;

        case Command::EndOcclusionQuery:
            commands->NextCommand<EndOcclusionQueryCmd>();
            break;

        case Command::EndRenderPass:
            commands->NextCommand<EndRenderPassCmd>();
            break;

        case Command::ExecuteBundles: {
            auto* cmd = commands->NextCommand<ExecuteBundlesCmd>();
            commands->NextData<Ref<RenderBundleBase>>(cmd->count);
            break;
        }

        case Command::ClearBuffer:
            commands->NextCommand<ClearBufferCmd>();
            break;

        case Command::PixelLocalStorageBarrier:
            commands->NextCommand<PixelLocalStorageBarrierCmd>();
            break;

        case Command::InsertDebugMarker: {
            InsertDebugMarkerCmd* cmd = commands->NextCommand<InsertDebugMarkerCmd>();
            commands->NextData<char>(cmd->length + 1);
            break;
        }

        case Command::PopDebugGroup:
            commands->NextCommand<PopDebugGroupCmd>();
            break;

        case Command::PushDebugGroup: {
            PushDebugGroupCmd* cmd = commands->NextCommand<PushDebugGroupCmd>();
            commands->NextData<char>(cmd->length + 1);
            break;
        }

        case Command::ResolveQuerySet: {
            commands->NextCommand<ResolveQuerySetCmd>();
            break;
        }

        case Command::SetComputePipeline:
            commands->NextCommand<SetComputePipelineCmd>();
            break;

        case Command::SetRenderPipeline:
            commands->NextCommand<SetRenderPipelineCmd>();
            break;

        case Command::SetStencilReference:
            commands->NextCommand<SetStencilReferenceCmd>();
            break;

        case Command::SetViewport:
            commands->NextCommand<SetViewportCmd>();
            break;

        case Command::SetScissorRect:
            commands->NextCommand<SetScissorRectCmd>();
            break;

        case Command::SetBlendConstant:
            commands->NextCommand<SetBlendConstantCmd>();
            break;

        case Command::SetBindGroup: {
            SetBindGroupCmd* cmd = commands->NextCommand<SetBindGroupCmd>();
            if (cmd->dynamicOffsetCount > 0) {
                commands->NextData<uint32_t>(cmd->dynamicOffsetCount);
            }
            break;
        }

        case Command::SetImmediateData: {
            SetImmediateDataCmd* cmd = commands->NextCommand<SetImmediateDataCmd>();
            if (cmd->size > 0) {
                commands->NextData<uint8_t>(cmd->size);
            }
            break;
        }

        case Command::SetIndexBuffer:
            commands->NextCommand<SetIndexBufferCmd>();
            break;

        case Command::SetVertexBuffer: {
            commands->NextCommand<SetVertexBufferCmd>();
            break;
        }

        case Command::WriteBuffer:
            commands->NextCommand<WriteBufferCmd>();
            break;

        case Command::WriteTimestamp: {
            commands->NextCommand<WriteTimestampCmd>();
            break;
        }
    }
}

const char* AddNullTerminatedString(CommandAllocator* allocator, StringView s, uint32_t* length) {
    std::string_view view = s;
    *length = view.length();

    // Include extra null-terminator character. The string_view may not be null-terminated. It also
    // may already have a null-terminator inside of it, in which case adding the null-terminator is
    // unnecessary. However, this is unlikely, so always include the extra character.
    char* out = allocator->AllocateData<char>(view.length() + 1);
    memcpy(out, view.data(), view.length());
    out[view.length()] = '\0';

    return out;
}

TimestampWrites::TimestampWrites() = default;
TimestampWrites::~TimestampWrites() = default;

BeginComputePassCmd::BeginComputePassCmd() = default;
BeginComputePassCmd::~BeginComputePassCmd() = default;

BeginOcclusionQueryCmd::BeginOcclusionQueryCmd() = default;
BeginOcclusionQueryCmd::~BeginOcclusionQueryCmd() = default;

RenderPassColorAttachmentInfo::RenderPassColorAttachmentInfo() = default;
RenderPassColorAttachmentInfo::~RenderPassColorAttachmentInfo() = default;

RenderPassStorageAttachmentInfo::RenderPassStorageAttachmentInfo() = default;
RenderPassStorageAttachmentInfo::~RenderPassStorageAttachmentInfo() = default;

RenderPassDepthStencilAttachmentInfo::RenderPassDepthStencilAttachmentInfo() = default;
RenderPassDepthStencilAttachmentInfo::~RenderPassDepthStencilAttachmentInfo() = default;

bool ResolveRect::HasValue() const {
    return updateWidth != 0 && updateHeight != 0;
}

BeginRenderPassCmd::BeginRenderPassCmd() = default;
BeginRenderPassCmd::~BeginRenderPassCmd() = default;

BufferCopy::BufferCopy() = default;
BufferCopy::~BufferCopy() = default;

TextureCopy::TextureCopy() = default;
TextureCopy::TextureCopy(const TextureCopy&) = default;
TextureCopy& TextureCopy::operator=(const TextureCopy&) = default;
TextureCopy::~TextureCopy() = default;

CopyBufferToBufferCmd::CopyBufferToBufferCmd() = default;
CopyBufferToBufferCmd::~CopyBufferToBufferCmd() = default;

DispatchIndirectCmd::DispatchIndirectCmd() = default;
DispatchIndirectCmd::~DispatchIndirectCmd() = default;

DrawIndirectCmd::DrawIndirectCmd() = default;
DrawIndirectCmd::~DrawIndirectCmd() = default;

MultiDrawIndirectCmd::MultiDrawIndirectCmd() = default;
MultiDrawIndirectCmd::~MultiDrawIndirectCmd() = default;

EndComputePassCmd::EndComputePassCmd() = default;
EndComputePassCmd::~EndComputePassCmd() = default;

EndOcclusionQueryCmd::EndOcclusionQueryCmd() = default;
EndOcclusionQueryCmd::~EndOcclusionQueryCmd() = default;

EndRenderPassCmd::EndRenderPassCmd() = default;
EndRenderPassCmd::~EndRenderPassCmd() = default;

ClearBufferCmd::ClearBufferCmd() = default;
ClearBufferCmd::~ClearBufferCmd() = default;

ResolveQuerySetCmd::ResolveQuerySetCmd() = default;
ResolveQuerySetCmd::~ResolveQuerySetCmd() = default;

SetComputePipelineCmd::SetComputePipelineCmd() = default;
SetComputePipelineCmd::~SetComputePipelineCmd() = default;

SetRenderPipelineCmd::SetRenderPipelineCmd() = default;
SetRenderPipelineCmd::~SetRenderPipelineCmd() = default;

SetBindGroupCmd::SetBindGroupCmd() = default;
SetBindGroupCmd::~SetBindGroupCmd() = default;

SetImmediateDataCmd::SetImmediateDataCmd() = default;
SetImmediateDataCmd::~SetImmediateDataCmd() = default;

SetIndexBufferCmd::SetIndexBufferCmd() = default;
SetIndexBufferCmd::~SetIndexBufferCmd() = default;

SetVertexBufferCmd::SetVertexBufferCmd() = default;
SetVertexBufferCmd::~SetVertexBufferCmd() = default;

WriteBufferCmd::WriteBufferCmd() = default;
WriteBufferCmd::~WriteBufferCmd() = default;

WriteTimestampCmd::WriteTimestampCmd() = default;
WriteTimestampCmd::~WriteTimestampCmd() = default;

}  // namespace dawn::native
