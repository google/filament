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

#include "src/dawn/common/Constants.h"
#include "src/dawn/common/Ref.h"
#include "src/dawn/native/AttachmentState.h"
#include "src/dawn/native/BindingInfo.h"
#include "src/dawn/native/BlockInfo.h"
#include "src/dawn/native/Texture.h"
#include "src/dawn/native/dawn_platform.h"

namespace dawn::native {

class CommandAllocator;
class CommandIterator;
struct TexelBlockInfo;
struct TexelCopyTextureInfo;

// Definition of the commands that are present in the CommandIterator given by the
// CommandBufferBuilder. There are not defined in CommandBuffer.h to break some header
// dependencies: Ref<Object> needs Object to be defined.

enum class Command : uint32_t {
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
    SetImmediates,
    SetIndexBuffer,
    SetVertexBuffer,
    SetResourceTable,
    WriteBuffer,
    WriteTimestamp,
};

struct TimestampWrites {
    TimestampWrites();
    ~TimestampWrites();

    Ref<QuerySetBase> querySet;
    QueryIndex beginningOfPassWriteIndex = kQuerySetIndexUndefinedTyped;
    QueryIndex endOfPassWriteIndex = kQuerySetIndexUndefinedTyped;
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
    QueryIndex queryIndex = kQuerySetIndexUndefinedTyped;
};

struct RenderPassColorAttachmentInfo {
    RenderPassColorAttachmentInfo();
    ~RenderPassColorAttachmentInfo();

    Ref<TextureViewBase> view;
    uint32_t depthSlice = 0;
    Ref<TextureViewBase> resolveTarget;
    wgpu::LoadOp loadOp = wgpu::LoadOp::Undefined;
    wgpu::StoreOp storeOp = wgpu::StoreOp::Undefined;
    dawn::native::Color clearColor = {};
};

struct RenderPassStorageAttachmentInfo {
    RenderPassStorageAttachmentInfo();
    ~RenderPassStorageAttachmentInfo();

    Ref<TextureViewBase> storage;
    wgpu::LoadOp loadOp = wgpu::LoadOp::Undefined;
    wgpu::StoreOp storeOp = wgpu::StoreOp::Undefined;
    dawn::native::Color clearColor = {};
};

struct RenderPassDepthStencilAttachmentInfo {
    RenderPassDepthStencilAttachmentInfo();
    ~RenderPassDepthStencilAttachmentInfo();

    Ref<TextureViewBase> view;
    wgpu::LoadOp depthLoadOp = wgpu::LoadOp::Undefined;
    wgpu::StoreOp depthStoreOp = wgpu::StoreOp::Undefined;
    wgpu::LoadOp stencilLoadOp = wgpu::LoadOp::Undefined;
    wgpu::StoreOp stencilStoreOp = wgpu::StoreOp::Undefined;
    float clearDepth = 0.0f;
    uint32_t clearStencil = 0;
    bool depthReadOnly = false;
    bool stencilReadOnly = false;
};

struct ResolveRect {
    // TODO(https://issues.chromium.org/424536624): Use TexelCount instead of uint32_t.
    uint32_t colorOffsetX = 0;
    uint32_t colorOffsetY = 0;
    uint32_t resolveOffsetX = 0;
    uint32_t resolveOffsetY = 0;
    uint32_t updateWidth = 0;
    uint32_t updateHeight = 0;
    // Returns whether this ResolveRect contains valid dimensions for a partial resolve operation.
    // A resolve rectangle is considered valid only when both width and height are non-zero.
    bool HasValue() const;
};

struct RenderAreaRect {
    // TODO(https://issues.chromium.org/424536624): Use TexelCount instead of uint32_t.
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

struct BeginRenderPassCmd {
    BeginRenderPassCmd();
    ~BeginRenderPassCmd();

    Ref<AttachmentState> attachmentState;
    PerColorAttachment<RenderPassColorAttachmentInfo> colorAttachments;
    RenderPassDepthStencilAttachmentInfo depthStencilAttachment;

    std::array<RenderPassStorageAttachmentInfo, kMaxPLSSlots> storageAttachments;

    // Cache the width and height of all attachments for convenience
    // TODO(https://issues.chromium.org/424536624): Use TexelCount instead of uint32_t.
    uint32_t width = 0;
    uint32_t height = 0;

    // Use the full render pass dimensions as render area to clear attachments. `renderArea` is
    // still used to set the initial scissor rect even if this is true.
    bool forceFullRenderArea = false;
    RenderAreaRect renderArea;

    // Used for partial resolve
    ResolveRect resolveRect;
    bool msaaRenderToSingleSampled = false;

    Ref<QuerySetBase> occlusionQuerySet;
    TimestampWrites timestampWrites;
    std::string label;
};

struct BufferCopy {
    BufferCopy();
    ~BufferCopy();

    Ref<BufferBase> buffer;
    uint64_t offset = 0;
    BlockCount blocksPerRow = BlockCount(0u);
    BlockCount rowsPerImage = BlockCount(0u);
};

struct TextureCopy {
    TextureCopy();
    TextureCopy(const TextureCopy&);
    TextureCopy& operator=(const TextureCopy&);
    ~TextureCopy();

    Ref<TextureBase> texture;
    uint32_t mipLevel = 0;
    TexelOrigin3D origin;  // Texels / array layer
    Aspect aspect = Aspect::None;
};

// Returns the TexelBlockInfo for t's texture and aspect
const TexelBlockInfo& GetBlockInfo(const TextureCopy& t);

struct CopyBufferToBufferCmd {
    CopyBufferToBufferCmd();
    ~CopyBufferToBufferCmd();

    Ref<BufferBase> source;
    uint64_t sourceOffset = 0;
    Ref<BufferBase> destination;
    uint64_t destinationOffset = 0;
    uint64_t size = 0;
};

struct CopyBufferToTextureCmd {
    BufferCopy source;
    TextureCopy destination;
    // TODO(https://issues.chromium.org/424536624): Use BlockCount instead of TexelCount.
    TexelExtent3D copySize;
};

struct CopyTextureToBufferCmd {
    TextureCopy source;
    BufferCopy destination;
    // TODO(https://issues.chromium.org/424536624): Use BlockCount instead of TexelCount.
    TexelExtent3D copySize;
};

struct CopyTextureToTextureCmd {
    TextureCopy source;
    TextureCopy destination;
    // TODO(https://issues.chromium.org/424536624): Use BlockCount instead of TexelCount.
    TexelExtent3D copySize;
};

struct DispatchCmd {
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t z = 0;
};

struct DispatchIndirectCmd {
    DispatchIndirectCmd();
    ~DispatchIndirectCmd();

    Ref<BufferBase> indirectBuffer;
    uint64_t indirectOffset = 0;
};

struct DrawCmd {
    uint32_t vertexCount = 0;
    uint32_t instanceCount = 0;
    uint32_t firstVertex = 0;
    uint32_t firstInstance = 0;
};

struct DrawIndexedCmd {
    uint32_t indexCount = 0;
    uint32_t instanceCount = 0;
    uint32_t firstIndex = 0;
    int32_t baseVertex = 0;
    uint32_t firstInstance = 0;
};

struct DrawIndirectCmd {
    DrawIndirectCmd();
    ~DrawIndirectCmd();

    Ref<BufferBase> indirectBuffer;
    uint64_t indirectOffset = 0;
};

struct DrawIndexedIndirectCmd : DrawIndirectCmd {};

struct MultiDrawIndirectCmd {
    MultiDrawIndirectCmd();
    ~MultiDrawIndirectCmd();

    Ref<BufferBase> indirectBuffer;
    uint64_t indirectOffset = 0;
    uint32_t maxDrawCount = 0;
    Ref<BufferBase> drawCountBuffer;
    uint64_t drawCountOffset = 0;
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
    QueryIndex queryIndex = kQuerySetIndexUndefinedTyped;
};

struct EndRenderPassCmd {
    EndRenderPassCmd();
    ~EndRenderPassCmd();
};

struct ExecuteBundlesCmd {
    size_t count = 0;
};

struct ClearBufferCmd {
    ClearBufferCmd();
    ~ClearBufferCmd();

    Ref<BufferBase> buffer;
    uint64_t offset = 0;
    uint64_t size = 0;
};

struct InsertDebugMarkerCmd {
    size_t length = 0;
};

struct PixelLocalStorageBarrierCmd {};

struct PopDebugGroupCmd {};

struct PushDebugGroupCmd {
    size_t length = 0;
};

struct ResolveQuerySetCmd {
    ResolveQuerySetCmd();
    ~ResolveQuerySetCmd();

    Ref<QuerySetBase> querySet;
    QueryIndex firstQuery = kQuerySetIndexUndefinedTyped;
    QueryIndex queryCount = QueryIndex(0u);
    Ref<BufferBase> destination;
    uint64_t destinationOffset = 0;
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
    uint32_t reference = 0;
};

struct SetViewportCmd {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float minDepth = 0.0f;
    float maxDepth = 0.0f;
};

struct SetScissorRectCmd {
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

struct SetBlendConstantCmd {
    Color color = {};
};

struct SetBindGroupCmd {
    SetBindGroupCmd();
    ~SetBindGroupCmd();

    BindGroupIndex index = BindGroupIndex(0u);
    Ref<BindGroupBase> group;
    BindingIndex dynamicOffsetCount = BindingIndex{0u};
};

struct SetImmediatesCmd {
    SetImmediatesCmd();
    ~SetImmediatesCmd();

    uint32_t offset = 0;
    size_t size = 0;
};

struct SetIndexBufferCmd {
    SetIndexBufferCmd();
    ~SetIndexBufferCmd();

    Ref<BufferBase> buffer;
    wgpu::IndexFormat format = wgpu::IndexFormat::Undefined;
    uint64_t offset = 0;
    uint64_t size = 0;
};

struct SetVertexBufferCmd {
    SetVertexBufferCmd();
    ~SetVertexBufferCmd();

    VertexBufferSlot slot;
    Ref<BufferBase> buffer;
    uint64_t offset = 0;
    uint64_t size = 0;
};

struct SetResourceTableCmd {
    SetResourceTableCmd();
    ~SetResourceTableCmd();

    Ref<ResourceTableBase> table;
};

struct WriteBufferCmd {
    WriteBufferCmd();
    ~WriteBufferCmd();

    Ref<BufferBase> buffer;
    uint64_t offset = 0;
    size_t size = 0;
};

struct WriteTimestampCmd {
    WriteTimestampCmd();
    ~WriteTimestampCmd();

    Ref<QuerySetBase> querySet;
    QueryIndex queryIndex = kQuerySetIndexUndefinedTyped;
};

// This needs to be called before the CommandIterator is freed so that the Ref<> present in
// the commands have a chance to run their destructor and remove internal references.
class CommandIterator;
void FreeCommands(CommandIterator* commands);

// Helper function to allow skipping over a command when it is unimplemented, while still
// consuming the correct amount of data from the command iterator.
void SkipCommand(CommandIterator* commands, Command type);

// Helper function to copy a wgpu::StringView into the command stream, writing out its size.
std::string_view AddNullTerminatedString(CommandAllocator* allocator,
                                         std::string_view s,
                                         size_t* length);
// Mirror function that gets the same string back as a null-terminated string_view.
std::string_view NextNullTerminatedString(CommandIterator* iterator, size_t length);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_COMMANDS_H_
