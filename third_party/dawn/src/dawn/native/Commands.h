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

#ifndef SRC_DAWN_NATIVE_COMMANDS_H_
#define SRC_DAWN_NATIVE_COMMANDS_H_

#include <array>
#include <bitset>
#include <string>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/Ref.h"

#include "dawn/native/AttachmentState.h"
#include "dawn/native/BindingInfo.h"
#include "dawn/native/Texture.h"

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

class CommandAllocator;

// Definition of the commands that are present in the CommandIterator given by the
// CommandBufferBuilder. There are not defined in CommandBuffer.h to break some header
// dependencies: Ref<Object> needs Object to be defined.

enum class Command {
    BeginComputePass,
    BeginOcclusionQuery,
    BeginRenderPass,
    ClearBuffer,
    CopyBufferToBuffer,
    CopyBufferToTexture,
    CopyTextureToBuffer,
    CopyTextureToTexture,
    Dispatch,
    DispatchIndirect,
    Draw,
    DrawIndexed,
    DrawIndirect,
    DrawIndexedIndirect,
    MultiDrawIndirect,
    MultiDrawIndexedIndirect,
    EndComputePass,
    EndOcclusionQuery,
    EndRenderPass,
    ExecuteBundles,
    InsertDebugMarker,
    PixelLocalStorageBarrier,
    PopDebugGroup,
    PushDebugGroup,
    ResolveQuerySet,
    SetComputePipeline,
    SetRenderPipeline,
    SetStencilReference,
    SetViewport,
    SetScissorRect,
    SetBlendConstant,
    SetBindGroup,
    SetIndexBuffer,
    SetVertexBuffer,
    WriteBuffer,
    WriteTimestamp,
};

struct TimestampWrites {
    TimestampWrites();
    ~TimestampWrites();

    Ref<QuerySetBase> querySet;
    uint32_t beginningOfPassWriteIndex = wgpu::kQuerySetIndexUndefined;
    uint32_t endOfPassWriteIndex = wgpu::kQuerySetIndexUndefined;
};

struct BeginComputePassCmd {
    BeginComputePassCmd();
    ~BeginComputePassCmd();

    TimestampWrites timestampWrites;
    std::string label;
};

struct BeginOcclusionQueryCmd {
    BeginOcclusionQueryCmd();
    ~BeginOcclusionQueryCmd();

    Ref<QuerySetBase> querySet;
    uint32_t queryIndex;
};

struct RenderPassColorAttachmentInfo {
    RenderPassColorAttachmentInfo();
    ~RenderPassColorAttachmentInfo();

    Ref<TextureViewBase> view;
    uint32_t depthSlice;
    Ref<TextureViewBase> resolveTarget;
    wgpu::LoadOp loadOp;
    wgpu::StoreOp storeOp;
    dawn::native::Color clearColor;
};

struct RenderPassStorageAttachmentInfo {
    RenderPassStorageAttachmentInfo();
    ~RenderPassStorageAttachmentInfo();

    Ref<TextureViewBase> storage;
    wgpu::LoadOp loadOp;
    wgpu::StoreOp storeOp;
    dawn::native::Color clearColor;
};

struct RenderPassDepthStencilAttachmentInfo {
    RenderPassDepthStencilAttachmentInfo();
    ~RenderPassDepthStencilAttachmentInfo();

    Ref<TextureViewBase> view;
    wgpu::LoadOp depthLoadOp;
    wgpu::StoreOp depthStoreOp;
    wgpu::LoadOp stencilLoadOp;
    wgpu::StoreOp stencilStoreOp;
    float clearDepth;
    uint32_t clearStencil;
    bool depthReadOnly;
    bool stencilReadOnly;
};

struct BeginRenderPassCmd {
    BeginRenderPassCmd();
    ~BeginRenderPassCmd();

    Ref<AttachmentState> attachmentState;
    PerColorAttachment<RenderPassColorAttachmentInfo> colorAttachments;
    RenderPassDepthStencilAttachmentInfo depthStencilAttachment;

    std::array<RenderPassStorageAttachmentInfo, kMaxPLSSlots> storageAttachments;

    // Cache the width and height of all attachments for convenience
    uint32_t width;
    uint32_t height;

    Ref<QuerySetBase> occlusionQuerySet;
    TimestampWrites timestampWrites;
    std::string label;
};

struct BufferCopy {
    BufferCopy();
    ~BufferCopy();

    Ref<BufferBase> buffer;
    uint64_t offset;
    uint32_t bytesPerRow;
    uint32_t rowsPerImage;
};

struct TextureCopy {
    TextureCopy();
    TextureCopy(const TextureCopy&);
    TextureCopy& operator=(const TextureCopy&);
    ~TextureCopy();

    Ref<TextureBase> texture;
    uint32_t mipLevel;
    Origin3D origin;  // Texels / array layer
    Aspect aspect;
};

struct CopyBufferToBufferCmd {
    CopyBufferToBufferCmd();
    ~CopyBufferToBufferCmd();

    Ref<BufferBase> source;
    uint64_t sourceOffset;
    Ref<BufferBase> destination;
    uint64_t destinationOffset;
    uint64_t size;
};

struct CopyBufferToTextureCmd {
    BufferCopy source;
    TextureCopy destination;
    Extent3D copySize;  // Texels
};

struct CopyTextureToBufferCmd {
    TextureCopy source;
    BufferCopy destination;
    Extent3D copySize;  // Texels
};

struct CopyTextureToTextureCmd {
    TextureCopy source;
    TextureCopy destination;
    Extent3D copySize;  // Texels
};

struct DispatchCmd {
    uint32_t x;
    uint32_t y;
    uint32_t z;
};

struct DispatchIndirectCmd {
    DispatchIndirectCmd();
    ~DispatchIndirectCmd();

    Ref<BufferBase> indirectBuffer;
    uint64_t indirectOffset;
};

struct DrawCmd {
    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex;
    uint32_t firstInstance;
};

struct DrawIndexedCmd {
    uint32_t indexCount;
    uint32_t instanceCount;
    uint32_t firstIndex;
    int32_t baseVertex;
    uint32_t firstInstance;
};

struct DrawIndirectCmd {
    DrawIndirectCmd();
    ~DrawIndirectCmd();

    Ref<BufferBase> indirectBuffer;
    uint64_t indirectOffset;
};

struct DrawIndexedIndirectCmd : DrawIndirectCmd {};

struct MultiDrawIndirectCmd {
    MultiDrawIndirectCmd();
    ~MultiDrawIndirectCmd();

    Ref<BufferBase> indirectBuffer;
    uint64_t indirectOffset;
    uint32_t maxDrawCount;
    Ref<BufferBase> drawCountBuffer;
    uint64_t drawCountOffset;
};

struct MultiDrawIndexedIndirectCmd : MultiDrawIndirectCmd {};

struct EndComputePassCmd {
    EndComputePassCmd();
    ~EndComputePassCmd();
};

struct EndOcclusionQueryCmd {
    EndOcclusionQueryCmd();
    ~EndOcclusionQueryCmd();

    Ref<QuerySetBase> querySet;
    uint32_t queryIndex;
};

struct EndRenderPassCmd {
    EndRenderPassCmd();
    ~EndRenderPassCmd();
};

struct ExecuteBundlesCmd {
    uint32_t count;
};

struct ClearBufferCmd {
    ClearBufferCmd();
    ~ClearBufferCmd();

    Ref<BufferBase> buffer;
    uint64_t offset;
    uint64_t size;
};

struct InsertDebugMarkerCmd {
    uint32_t length;
};

struct PixelLocalStorageBarrierCmd {};

struct PopDebugGroupCmd {};

struct PushDebugGroupCmd {
    uint32_t length;
};

struct ResolveQuerySetCmd {
    ResolveQuerySetCmd();
    ~ResolveQuerySetCmd();

    Ref<QuerySetBase> querySet;
    uint32_t firstQuery;
    uint32_t queryCount;
    Ref<BufferBase> destination;
    uint64_t destinationOffset;
};

struct SetComputePipelineCmd {
    SetComputePipelineCmd();
    ~SetComputePipelineCmd();

    Ref<ComputePipelineBase> pipeline;
};

struct SetRenderPipelineCmd {
    SetRenderPipelineCmd();
    ~SetRenderPipelineCmd();

    Ref<RenderPipelineBase> pipeline;
};

struct SetStencilReferenceCmd {
    uint32_t reference;
};

struct SetViewportCmd {
    float x, y, width, height, minDepth, maxDepth;
};

struct SetScissorRectCmd {
    uint32_t x, y, width, height;
};

struct SetBlendConstantCmd {
    Color color;
};

struct SetBindGroupCmd {
    SetBindGroupCmd();
    ~SetBindGroupCmd();

    BindGroupIndex index;
    Ref<BindGroupBase> group;
    uint32_t dynamicOffsetCount;
};

struct SetIndexBufferCmd {
    SetIndexBufferCmd();
    ~SetIndexBufferCmd();

    Ref<BufferBase> buffer;
    wgpu::IndexFormat format;
    uint64_t offset;
    uint64_t size;
};

struct SetVertexBufferCmd {
    SetVertexBufferCmd();
    ~SetVertexBufferCmd();

    VertexBufferSlot slot;
    Ref<BufferBase> buffer;
    uint64_t offset;
    uint64_t size;
};

struct WriteBufferCmd {
    WriteBufferCmd();
    ~WriteBufferCmd();

    Ref<BufferBase> buffer;
    uint64_t offset;
    uint64_t size;
};

struct WriteTimestampCmd {
    WriteTimestampCmd();
    ~WriteTimestampCmd();

    Ref<QuerySetBase> querySet;
    uint32_t queryIndex;
};

// This needs to be called before the CommandIterator is freed so that the Ref<> present in
// the commands have a chance to run their destructor and remove internal references.
class CommandIterator;
void FreeCommands(CommandIterator* commands);

// Helper function to allow skipping over a command when it is unimplemented, while still
// consuming the correct amount of data from the command iterator.
void SkipCommand(CommandIterator* commands, Command type);

// Helper function to copy a wgpu::StringView into a safely null-terminated C-string in commands.
const char* AddNullTerminatedString(CommandAllocator* allocator, StringView s, uint32_t* length);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_COMMANDS_H_
