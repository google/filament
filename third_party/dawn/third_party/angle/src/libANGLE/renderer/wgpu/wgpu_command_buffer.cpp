//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/wgpu/wgpu_command_buffer.h"

namespace rx
{
namespace webgpu
{
namespace
{
template <typename T>
const T *GetReferencedObject(std::unordered_set<T> &referenceList, const T &item)
{
    auto iter = referenceList.insert(item).first;
    return &(*iter);
}

// Get the packed command ID from the current command data
CommandID CurrentCommandID(const uint8_t *commandData)
{
    return *reinterpret_cast<const CommandID *>(commandData);
}

// Get the command struct from the current command data and increment the command data to the next
// command
template <CommandID Command, typename CommandType = CommandTypeHelper<Command>::CommandType>
const CommandType &GetCommandAndIterate(const uint8_t **commandData)
{
    constexpr size_t commandAndIdSize = sizeof(CommandID) + sizeof(CommandType);
    const CommandType *command =
        reinterpret_cast<const CommandType *>(*commandData + sizeof(CommandID));
    *commandData += commandAndIdSize;
    return *command;
}
}  // namespace

CommandBuffer::CommandBuffer() {}

void CommandBuffer::draw(uint32_t vertexCount,
                         uint32_t instanceCount,
                         uint32_t firstVertex,
                         uint32_t firstInstance)
{
    DrawCommand *drawCommand   = initCommand<CommandID::Draw>();
    drawCommand->vertexCount   = vertexCount;
    drawCommand->instanceCount = instanceCount;
    drawCommand->firstVertex   = firstVertex;
    drawCommand->firstInstance = firstInstance;
}

void CommandBuffer::drawIndexed(uint32_t indexCount,
                                uint32_t instanceCount,
                                uint32_t firstIndex,
                                int32_t baseVertex,
                                uint32_t firstInstance)
{
    DrawIndexedCommand *drawIndexedCommand = initCommand<CommandID::DrawIndexed>();
    drawIndexedCommand->indexCount         = indexCount;
    drawIndexedCommand->instanceCount      = instanceCount;
    drawIndexedCommand->firstIndex         = firstIndex;
    drawIndexedCommand->baseVertex         = baseVertex;
    drawIndexedCommand->firstInstance      = firstInstance;
}

void CommandBuffer::setBindGroup(uint32_t groupIndex, wgpu::BindGroup bindGroup)
{
    SetBindGroupCommand *setBindGroupCommand = initCommand<CommandID::SetBindGroup>();
    setBindGroupCommand->groupIndex          = groupIndex;
    setBindGroupCommand->bindGroup = GetReferencedObject(mReferencedBindGroups, bindGroup);
}

void CommandBuffer::setPipeline(wgpu::RenderPipeline pipeline)
{
    SetPipelineCommand *setPiplelineCommand = initCommand<CommandID::SetPipeline>();
    setPiplelineCommand->pipeline = GetReferencedObject(mReferencedRenderPipelines, pipeline);
}

void CommandBuffer::setScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    SetScissorRectCommand *setScissorRectCommand = initCommand<CommandID::SetScissorRect>();
    setScissorRectCommand->x                     = x;
    setScissorRectCommand->y                     = y;
    setScissorRectCommand->width                 = width;
    setScissorRectCommand->height                = height;

    mHasSetScissorCommand = true;
}

void CommandBuffer::setViewport(float x,
                                float y,
                                float width,
                                float height,
                                float minDepth,
                                float maxDepth)
{
    SetViewportCommand *setViewportCommand = initCommand<CommandID::SetViewport>();
    setViewportCommand->x                  = x;
    setViewportCommand->y                  = y;
    setViewportCommand->width              = width;
    setViewportCommand->height             = height;
    setViewportCommand->minDepth           = minDepth;
    setViewportCommand->maxDepth           = maxDepth;

    mHasSetViewportCommand = true;
}

void CommandBuffer::setIndexBuffer(wgpu::Buffer buffer,
                                   wgpu::IndexFormat format,
                                   uint64_t offset,
                                   uint64_t size)
{
    SetIndexBufferCommand *setIndexBufferCommand = initCommand<CommandID::SetIndexBuffer>();
    setIndexBufferCommand->buffer                = GetReferencedObject(mReferencedBuffers, buffer);
    setIndexBufferCommand->format                = format;
    setIndexBufferCommand->offset                = offset;
    setIndexBufferCommand->size                  = size;
}

void CommandBuffer::setVertexBuffer(uint32_t slot, wgpu::Buffer buffer)
{
    SetVertexBufferCommand *setVertexBufferCommand = initCommand<CommandID::SetVertexBuffer>();
    setVertexBufferCommand->slot                   = slot;
    setVertexBufferCommand->buffer = GetReferencedObject(mReferencedBuffers, buffer);
}

void CommandBuffer::clear()
{
    mCommandCount = 0;

    mHasSetScissorCommand  = false;
    mHasSetViewportCommand = false;

    if (!mCommandBlocks.empty())
    {
        // Only clear the command blocks that have been used
        for (size_t cmdBlockIdx = 0; cmdBlockIdx <= mCurrentCommandBlock; cmdBlockIdx++)
        {
            mCommandBlocks[cmdBlockIdx]->clear();
        }
    }
    mCurrentCommandBlock = 0;

    mReferencedRenderPipelines.clear();
    mReferencedBuffers.clear();
}

void CommandBuffer::recordCommands(wgpu::RenderPassEncoder encoder)
{
    ASSERT(hasCommands());
    ASSERT(!mCommandBlocks.empty());

    // Make sure the last block is finalized
    mCommandBlocks[mCurrentCommandBlock]->finalize();

    for (size_t cmdBlockIdx = 0; cmdBlockIdx <= mCurrentCommandBlock; cmdBlockIdx++)
    {
        const CommandBlock *commandBlock = mCommandBlocks[cmdBlockIdx].get();

        const uint8_t *currentCommand = commandBlock->mData;
        while (CurrentCommandID(currentCommand) != CommandID::Invalid)
        {
            switch (CurrentCommandID(currentCommand))
            {
                case CommandID::Invalid:
                    UNREACHABLE();
                    return;

                case CommandID::Draw:
                {
                    const DrawCommand &drawCommand =
                        GetCommandAndIterate<CommandID::Draw>(&currentCommand);
                    encoder.Draw(drawCommand.vertexCount, drawCommand.instanceCount,
                                 drawCommand.firstVertex, drawCommand.firstInstance);
                    break;
                }

                case CommandID::DrawIndexed:
                {
                    const DrawIndexedCommand &drawIndexedCommand =
                        GetCommandAndIterate<CommandID::DrawIndexed>(&currentCommand);
                    encoder.DrawIndexed(
                        drawIndexedCommand.indexCount, drawIndexedCommand.instanceCount,
                        drawIndexedCommand.firstIndex, drawIndexedCommand.baseVertex,
                        drawIndexedCommand.firstInstance);
                    break;
                }

                case CommandID::SetBindGroup:
                {
                    const SetBindGroupCommand &setBindGroupCommand =
                        GetCommandAndIterate<CommandID::SetBindGroup>(&currentCommand);
                    encoder.SetBindGroup(setBindGroupCommand.groupIndex,
                                         *setBindGroupCommand.bindGroup);
                    break;
                }

                case CommandID::SetIndexBuffer:
                {
                    const SetIndexBufferCommand &setIndexBufferCommand =
                        GetCommandAndIterate<CommandID::SetIndexBuffer>(&currentCommand);
                    encoder.SetIndexBuffer(
                        *setIndexBufferCommand.buffer, setIndexBufferCommand.format,
                        setIndexBufferCommand.offset, setIndexBufferCommand.size);
                    break;
                }

                case CommandID::SetPipeline:
                {
                    const SetPipelineCommand &setPiplelineCommand =
                        GetCommandAndIterate<CommandID::SetPipeline>(&currentCommand);
                    encoder.SetPipeline(*setPiplelineCommand.pipeline);
                    break;
                }

                case CommandID::SetScissorRect:
                {
                    const SetScissorRectCommand &setScissorRectCommand =
                        GetCommandAndIterate<CommandID::SetScissorRect>(&currentCommand);
                    encoder.SetScissorRect(setScissorRectCommand.x, setScissorRectCommand.y,
                                           setScissorRectCommand.width,
                                           setScissorRectCommand.height);
                    break;
                }

                case CommandID::SetViewport:
                {
                    const SetViewportCommand &setViewportCommand =
                        GetCommandAndIterate<CommandID::SetViewport>(&currentCommand);
                    encoder.SetViewport(setViewportCommand.x, setViewportCommand.y,
                                        setViewportCommand.width, setViewportCommand.height,
                                        setViewportCommand.minDepth, setViewportCommand.maxDepth);
                    break;
                }

                case CommandID::SetVertexBuffer:
                {
                    const SetVertexBufferCommand &setVertexBufferCommand =
                        GetCommandAndIterate<CommandID::SetVertexBuffer>(&currentCommand);
                    encoder.SetVertexBuffer(setVertexBufferCommand.slot,
                                            *setVertexBufferCommand.buffer);
                    break;
                }

                default:
                    UNREACHABLE();
                    return;
            }
        }
    }
}

void CommandBuffer::nextCommandBlock()
{
    if (mCurrentCommandBlock + 1 < mCommandBlocks.size())
    {
        // There is already a command block allocated. Make sure it's been cleared and use it.
        mCurrentCommandBlock++;
        ASSERT(mCommandBlocks[mCurrentCommandBlock]->mCurrentPosition == 0);
        ASSERT(mCommandBlocks[mCurrentCommandBlock]->mRemainingSize > 0);
    }
    else
    {
        std::unique_ptr<CommandBlock> newBlock = std::make_unique<CommandBlock>();
        mCommandBlocks.push_back(std::move(newBlock));
        mCurrentCommandBlock = mCommandBlocks.size() - 1;
    }
}

void CommandBuffer::CommandBlock::clear()
{
    mCurrentPosition = 0;
    mRemainingSize   = kCommandBlockInitialRemainingSize;
}

void CommandBuffer::CommandBlock::finalize()
{
    // Don't move the current position to allow finalize to be called multiple times if needed
    CommandID *nextCommandID = getDataAtCurrentPositionAndReserveSpace<CommandID>(0);
    *nextCommandID           = CommandID::Invalid;
    mRemainingSize           = 0;
}

}  // namespace webgpu
}  // namespace rx
