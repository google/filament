//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_RENDERER_WGPU_WGPU_COMMAND_BUFFER_H_
#define LIBANGLE_RENDERER_WGPU_WGPU_COMMAND_BUFFER_H_

#include "common/debug.h"
#include "libANGLE/renderer/wgpu/wgpu_utils.h"

#include <dawn/webgpu_cpp.h>
#include <unordered_set>

namespace rx
{
namespace webgpu
{

#define ANGLE_WGPU_COMMANDS_X(PROC) \
    PROC(BeginOcclusionQuery)       \
    PROC(Draw)                      \
    PROC(DrawIndexed)               \
    PROC(DrawIndexedIndirect)       \
    PROC(DrawIndirect)              \
    PROC(End)                       \
    PROC(EndOcclusionQuery)         \
    PROC(ExecuteBundles)            \
    PROC(InsertDebugMarker)         \
    PROC(PixelLocalStorageBarrier)  \
    PROC(PopDebugGroup)             \
    PROC(PushDebugGroup)            \
    PROC(SetBindGroup)              \
    PROC(SetBlendConstant)          \
    PROC(SetIndexBuffer)            \
    PROC(SetLabel)                  \
    PROC(SetPipeline)               \
    PROC(SetScissorRect)            \
    PROC(SetStencilReference)       \
    PROC(SetVertexBuffer)           \
    PROC(SetViewport)               \
    PROC(WriteTimestamp)

#define WGPU_DECLARE_COMMAND_ID(CMD) CMD,

enum class CommandID : uint8_t
{
    Invalid = 0,
    ANGLE_WGPU_COMMANDS_X(WGPU_DECLARE_COMMAND_ID)
};

ANGLE_ENABLE_STRUCT_PADDING_WARNINGS

struct BeginOcclusionQueryCommand
{
    uint64_t pad;
};

struct DrawCommand
{
    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex;
    uint32_t firstInstance;
};

struct DrawIndexedCommand
{
    uint32_t indexCount;
    uint32_t instanceCount;
    uint32_t firstIndex;
    uint32_t baseVertex;
    uint32_t firstInstance;
    uint32_t pad;
};

struct DrawIndexedIndirectCommand
{
    uint64_t pad;
};

struct DrawIndirectCommand
{
    uint64_t pad;
};

struct EndCommand
{
    uint64_t pad;
};

struct EndOcclusionQueryCommand
{
    uint64_t pad;
};

struct ExecuteBundlesCommand
{
    uint64_t pad;
};

struct InsertDebugMarkerCommand
{
    uint64_t pad;
};

struct PixelLocalStorageBarrierCommand
{
    uint64_t pad;
};

struct PopDebugGroupCommand
{
    uint64_t pad;
};

struct PushDebugGroupCommand
{
    uint64_t pad;
};

struct SetBindGroupCommand
{
    uint32_t groupIndex;
    uint32_t pad0;
    union
    {
        const wgpu::BindGroup *bindGroup;
        uint64_t pad1;  // Pad to 64 bits on 32-bit systems
    };
};

struct SetBlendConstantCommand
{
    uint64_t pad;
};

struct SetIndexBufferCommand
{
    union
    {
        const wgpu::Buffer *buffer;
        uint64_t pad0;  // Pad to 64 bits on 32-bit systems
    };
    wgpu::IndexFormat format;
    uint32_t pad1;
    uint64_t offset;
    uint64_t size;
};

struct SetLabelCommand
{
    uint64_t pad;
};

struct SetPipelineCommand
{
    union
    {
        const wgpu::RenderPipeline *pipeline;
        uint64_t padding;  // Pad to 64 bits on 32-bit systems
    };
};

struct SetScissorRectCommand
{
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
};

struct SetStencilReferenceCommand
{
    uint64_t pad;
};

struct SetVertexBufferCommand
{
    uint32_t slot;
    uint32_t pad0;
    union
    {
        const wgpu::Buffer *buffer;
        uint64_t pad1;  // Pad to 64 bits on 32-bit systems
    };
};

struct SetViewportCommand
{
    float x;
    float y;
    float width;
    float height;
    float minDepth;
    float maxDepth;
};

struct WriteTimestampCommand
{
    uint64_t pad;
};

ANGLE_DISABLE_STRUCT_PADDING_WARNINGS

#define VERIFY_COMMAND_8_BYTE_ALIGNMENT(CMD) \
    static_assert((sizeof(CMD##Command) % 8) == 0, "Check " #CMD "Command alignment");
ANGLE_WGPU_COMMANDS_X(VERIFY_COMMAND_8_BYTE_ALIGNMENT)
#undef VERIFY_COMMAND_8_BYTE_ALIGNMENT

template <CommandID>
struct CommandTypeHelper;

#define ANGLE_WGPU_COMMAND_TYPE_HELPER(CMD)  \
    template <>                              \
    struct CommandTypeHelper<CommandID::CMD> \
    {                                        \
        using CommandType = CMD##Command;    \
    };
ANGLE_WGPU_COMMANDS_X(ANGLE_WGPU_COMMAND_TYPE_HELPER)
#undef ANGLE_WGPU_COMMAND_TYPE_HELPER

static constexpr size_t kCommandBlockSize = 1 << 14;  // 16kB

class CommandBuffer
{
  public:
    CommandBuffer();

    void draw(uint32_t vertexCount,
              uint32_t instanceCount,
              uint32_t firstVertex,
              uint32_t firstInstance);
    void drawIndexed(uint32_t indexCount,
                     uint32_t instanceCount,
                     uint32_t firstIndex,
                     int32_t baseVertex,
                     uint32_t firstInstance);
    void setBindGroup(uint32_t groupIndex, wgpu::BindGroup bindGroup);
    void setPipeline(wgpu::RenderPipeline pipeline);
    void setScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    void setViewport(float x, float y, float width, float height, float minDepth, float maxDepth);
    void setIndexBuffer(wgpu::Buffer buffer,
                        wgpu::IndexFormat format,
                        uint64_t offset,
                        uint64_t size);
    void setVertexBuffer(uint32_t slot, wgpu::Buffer buffer);

    void clear();

    bool hasCommands() const { return mCommandCount > 0; }
    bool hasSetScissorCommand() const { return mHasSetScissorCommand; }
    bool hasSetViewportCommand() const { return mHasSetViewportCommand; }

    void recordCommands(wgpu::RenderPassEncoder encoder);

  private:
    struct CommandBlock
    {
        static constexpr size_t kCommandBlockDataSize = kCommandBlockSize - (sizeof(size_t) * 2);
        uint8_t mData[kCommandBlockDataSize]          = {0};

        size_t mCurrentPosition = 0;

        static constexpr size_t kCommandIDSize = sizeof(CommandID);
        static constexpr size_t kCommandBlockInitialRemainingSize =
            kCommandBlockDataSize - kCommandIDSize;  // Leave room for one command ID at the end to
                                                     // signify the end of the list
        size_t mRemainingSize = kCommandBlockInitialRemainingSize;

        void clear();
        void finalize();

        template <typename T>
        T *getDataAtCurrentPositionAndReserveSpace(size_t space)
        {
            T *data = reinterpret_cast<T *>(&mData[mCurrentPosition]);

            ASSERT(mRemainingSize >= space);
            mCurrentPosition += space;
            mRemainingSize -= space;

            return data;
        }
    };
    static constexpr size_t kCommandBlockStructSize = sizeof(CommandBlock);
    static_assert(kCommandBlockStructSize == kCommandBlockSize, "Size mismatch");

    std::vector<std::unique_ptr<CommandBlock>> mCommandBlocks;
    size_t mCurrentCommandBlock = 0;

    size_t mCommandCount = 0;
    bool mHasSetScissorCommand  = false;
    bool mHasSetViewportCommand = false;

    // std::unordered_set required because it does not move elements and stored command reference
    // addresses in the set
    std::unordered_set<wgpu::RenderPipeline> mReferencedRenderPipelines;
    std::unordered_set<wgpu::Buffer> mReferencedBuffers;
    std::unordered_set<wgpu::BindGroup> mReferencedBindGroups;

    void nextCommandBlock();

    void ensureCommandSpace(size_t space)
    {
        if (mCommandBlocks.empty() || mCommandBlocks[mCurrentCommandBlock]->mRemainingSize < space)
        {
            nextCommandBlock();
        }
    }

    template <CommandID Command, typename CommandType = CommandTypeHelper<Command>::CommandType>
    CommandType *initCommand()
    {
        constexpr size_t allocationSize = sizeof(CommandID) + sizeof(CommandType);
        ensureCommandSpace(allocationSize);
        CommandBlock *commandBlock = mCommandBlocks[mCurrentCommandBlock].get();

        uint8_t *idAndCommandStorage =
            commandBlock->getDataAtCurrentPositionAndReserveSpace<uint8_t>(allocationSize);

        CommandID *id = reinterpret_cast<CommandID *>(idAndCommandStorage);
        *id           = Command;

        CommandType *commandStruct =
            reinterpret_cast<CommandType *>(idAndCommandStorage + sizeof(CommandID));

        mCommandCount++;

        return commandStruct;
    }
};

}  // namespace webgpu
}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_WGPU_COMMAND_BUFFER_H_
