//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SecondaryCommandBuffer:
//    CPU-side storage of commands to delay GPU-side allocation until commands are submitted.
//

#include "libANGLE/renderer/vulkan/SecondaryCommandBuffer.h"
#include "common/debug.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"
#include "libANGLE/trace.h"

namespace rx
{
namespace vk
{
namespace priv
{
namespace
{
const char *GetCommandString(CommandID id)
{
    switch (id)
    {
        case CommandID::Invalid:
            return "--Invalid--";
        case CommandID::BeginDebugUtilsLabel:
            return "BeginDebugUtilsLabel";
        case CommandID::BeginQuery:
            return "BeginQuery";
        case CommandID::BeginTransformFeedback:
            return "BeginTransformFeedback";
        case CommandID::BindComputePipeline:
            return "BindComputePipeline";
        case CommandID::BindDescriptorSets:
            return "BindDescriptorSets";
        case CommandID::BindGraphicsPipeline:
            return "BindGraphicsPipeline";
        case CommandID::BindIndexBuffer:
            return "BindIndexBuffer";
        case CommandID::BindTransformFeedbackBuffers:
            return "BindTransformFeedbackBuffers";
        case CommandID::BindVertexBuffers:
            return "BindVertexBuffers";
        case CommandID::BindVertexBuffers2:
            return "BindVertexBuffers2";
        case CommandID::BlitImage:
            return "BlitImage";
        case CommandID::BufferBarrier:
            return "BufferBarrier";
        case CommandID::ClearAttachments:
            return "ClearAttachments";
        case CommandID::ClearColorImage:
            return "ClearColorImage";
        case CommandID::ClearDepthStencilImage:
            return "ClearDepthStencilImage";
        case CommandID::CopyBuffer:
            return "CopyBuffer";
        case CommandID::CopyBufferToImage:
            return "CopyBufferToImage";
        case CommandID::CopyImage:
            return "CopyImage";
        case CommandID::CopyImageToBuffer:
            return "CopyImageToBuffer";
        case CommandID::Dispatch:
            return "Dispatch";
        case CommandID::DispatchIndirect:
            return "DispatchIndirect";
        case CommandID::Draw:
            return "Draw";
        case CommandID::DrawIndexed:
            return "DrawIndexed";
        case CommandID::DrawIndexedBaseVertex:
            return "DrawIndexedBaseVertex";
        case CommandID::DrawIndexedIndirect:
            return "DrawIndexedIndirect";
        case CommandID::DrawIndexedInstanced:
            return "DrawIndexedInstanced";
        case CommandID::DrawIndexedInstancedBaseVertex:
            return "DrawIndexedInstancedBaseVertex";
        case CommandID::DrawIndexedInstancedBaseVertexBaseInstance:
            return "DrawIndexedInstancedBaseVertexBaseInstance";
        case CommandID::DrawIndirect:
            return "DrawIndirect";
        case CommandID::DrawInstanced:
            return "DrawInstanced";
        case CommandID::DrawInstancedBaseInstance:
            return "DrawInstancedBaseInstance";
        case CommandID::EndDebugUtilsLabel:
            return "EndDebugUtilsLabel";
        case CommandID::EndQuery:
            return "EndQuery";
        case CommandID::EndTransformFeedback:
            return "EndTransformFeedback";
        case CommandID::FillBuffer:
            return "FillBuffer";
        case CommandID::ImageBarrier:
            return "ImageBarrier";
        case CommandID::ImageWaitEvent:
            return "ImageWaitEvent";
        case CommandID::InsertDebugUtilsLabel:
            return "InsertDebugUtilsLabel";
        case CommandID::MemoryBarrier:
            return "MemoryBarrier";
        case CommandID::NextSubpass:
            return "NextSubpass";
        case CommandID::PipelineBarrier:
            return "PipelineBarrier";
        case CommandID::PushConstants:
            return "PushConstants";
        case CommandID::ResetEvent:
            return "ResetEvent";
        case CommandID::ResetQueryPool:
            return "ResetQueryPool";
        case CommandID::ResolveImage:
            return "ResolveImage";
        case CommandID::SetBlendConstants:
            return "SetBlendConstants";
        case CommandID::SetCullMode:
            return "SetCullMode";
        case CommandID::SetDepthBias:
            return "SetDepthBias";
        case CommandID::SetDepthBiasEnable:
            return "SetDepthBiasEnable";
        case CommandID::SetDepthCompareOp:
            return "SetDepthCompareOp";
        case CommandID::SetDepthTestEnable:
            return "SetDepthTestEnable";
        case CommandID::SetDepthWriteEnable:
            return "SetDepthWriteEnable";
        case CommandID::SetEvent:
            return "SetEvent";
        case CommandID::SetFragmentShadingRate:
            return "SetFragmentShadingRate";
        case CommandID::SetFrontFace:
            return "SetFrontFace";
        case CommandID::SetLineWidth:
            return "SetLineWidth";
        case CommandID::SetLogicOp:
            return "SetLogicOp";
        case CommandID::SetPrimitiveRestartEnable:
            return "SetPrimitiveRestartEnable";
        case CommandID::SetRasterizerDiscardEnable:
            return "SetRasterizerDiscardEnable";
        case CommandID::SetScissor:
            return "SetScissor";
        case CommandID::SetStencilCompareMask:
            return "SetStencilCompareMask";
        case CommandID::SetStencilOp:
            return "SetStencilOp";
        case CommandID::SetStencilReference:
            return "SetStencilReference";
        case CommandID::SetStencilTestEnable:
            return "SetStencilTestEnable";
        case CommandID::SetStencilWriteMask:
            return "SetStencilWriteMask";
        case CommandID::SetVertexInput:
            return "SetVertexInput";
        case CommandID::SetViewport:
            return "SetViewport";
        case CommandID::WaitEvents:
            return "WaitEvents";
        case CommandID::WriteTimestamp:
            return "WriteTimestamp";
        default:
            // Need this to work around MSVC warning 4715.
            UNREACHABLE();
            return "--unreachable--";
    }
}

// Given the pointer to the parameter struct, returns the pointer to the first array parameter.
template <typename T, typename StructType>
ANGLE_INLINE const T *GetFirstArrayParameter(StructType *param)
{
    // The first array parameter is always right after the struct itself.
    return Offset<T>(param, sizeof(*param));
}

// Given the pointer to one array parameter (and its array count), returns the pointer to the next
// array parameter.
template <typename NextT, typename PrevT>
ANGLE_INLINE const NextT *GetNextArrayParameter(const PrevT *array, size_t arrayLen)
{
    const size_t arrayAllocateBytes = roundUpPow2<size_t>(sizeof(*array) * arrayLen, 8u);
    return Offset<NextT>(array, arrayAllocateBytes);
}
}  // namespace

ANGLE_INLINE const CommandHeader *NextCommand(const CommandHeader *command)
{
    return reinterpret_cast<const CommandHeader *>(reinterpret_cast<const uint8_t *>(command) +
                                                   command->size);
}

// Parse the cmds in this cmd buffer into given primary cmd buffer
void SecondaryCommandBuffer::executeCommands(PrimaryCommandBuffer *primary)
{
    VkCommandBuffer cmdBuffer = primary->getHandle();

    ANGLE_TRACE_EVENT0("gpu.angle", "SecondaryCommandBuffer::executeCommands");

    // Used for ring buffer allocators only.
    mCommandAllocator.terminateLastCommandBlock();

    for (const CommandHeader *command : mCommands)
    {
        for (const CommandHeader *currentCommand                      = command;
             currentCommand->id != CommandID::Invalid; currentCommand = NextCommand(currentCommand))
        {
            switch (currentCommand->id)
            {
                case CommandID::BeginDebugUtilsLabel:
                {
                    const DebugUtilsLabelParams *params =
                        getParamPtr<DebugUtilsLabelParams>(currentCommand);
                    const char *pLabelName           = GetFirstArrayParameter<char>(params);
                    const VkDebugUtilsLabelEXT label = {
                        VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                        nullptr,
                        pLabelName,
                        {params->color[0], params->color[1], params->color[2], params->color[3]}};
                    ASSERT(vkCmdBeginDebugUtilsLabelEXT);
                    vkCmdBeginDebugUtilsLabelEXT(cmdBuffer, &label);
                    break;
                }
                case CommandID::BeginQuery:
                {
                    const BeginQueryParams *params = getParamPtr<BeginQueryParams>(currentCommand);
                    vkCmdBeginQuery(cmdBuffer, params->queryPool, params->query, 0);
                    break;
                }
                case CommandID::BeginTransformFeedback:
                {
                    const BeginTransformFeedbackParams *params =
                        getParamPtr<BeginTransformFeedbackParams>(currentCommand);
                    const VkBuffer *counterBuffers = GetFirstArrayParameter<VkBuffer>(params);
                    const VkDeviceSize *counterBufferOffsets =
                        reinterpret_cast<const VkDeviceSize *>(counterBuffers +
                                                               params->bufferCount);
                    vkCmdBeginTransformFeedbackEXT(cmdBuffer, 0, params->bufferCount,
                                                   counterBuffers, counterBufferOffsets);
                    break;
                }
                case CommandID::BindComputePipeline:
                {
                    const BindPipelineParams *params =
                        getParamPtr<BindPipelineParams>(currentCommand);
                    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, params->pipeline);
                    break;
                }
                case CommandID::BindDescriptorSets:
                {
                    const BindDescriptorSetParams *params =
                        getParamPtr<BindDescriptorSetParams>(currentCommand);
                    const VkDescriptorSet *descriptorSets =
                        GetFirstArrayParameter<VkDescriptorSet>(params);
                    const uint32_t *dynamicOffsets =
                        GetNextArrayParameter<uint32_t>(descriptorSets, params->descriptorSetCount);
                    vkCmdBindDescriptorSets(cmdBuffer, params->pipelineBindPoint, params->layout,
                                            params->firstSet, params->descriptorSetCount,
                                            descriptorSets, params->dynamicOffsetCount,
                                            dynamicOffsets);
                    break;
                }
                case CommandID::BindGraphicsPipeline:
                {
                    const BindPipelineParams *params =
                        getParamPtr<BindPipelineParams>(currentCommand);
                    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, params->pipeline);
                    break;
                }
                case CommandID::BindIndexBuffer:
                {
                    const BindIndexBufferParams *params =
                        getParamPtr<BindIndexBufferParams>(currentCommand);
                    vkCmdBindIndexBuffer(cmdBuffer, params->buffer, params->offset,
                                         params->indexType);
                    break;
                }
                case CommandID::BindTransformFeedbackBuffers:
                {
                    const BindTransformFeedbackBuffersParams *params =
                        getParamPtr<BindTransformFeedbackBuffersParams>(currentCommand);
                    const VkBuffer *buffers = GetFirstArrayParameter<VkBuffer>(params);
                    const VkDeviceSize *offsets =
                        GetNextArrayParameter<VkDeviceSize>(buffers, params->bindingCount);
                    const VkDeviceSize *sizes =
                        GetNextArrayParameter<VkDeviceSize>(offsets, params->bindingCount);
                    vkCmdBindTransformFeedbackBuffersEXT(cmdBuffer, 0, params->bindingCount,
                                                         buffers, offsets, sizes);
                    break;
                }
                case CommandID::BindVertexBuffers:
                {
                    const BindVertexBuffersParams *params =
                        getParamPtr<BindVertexBuffersParams>(currentCommand);
                    const VkBuffer *buffers = GetFirstArrayParameter<VkBuffer>(params);
                    const VkDeviceSize *offsets =
                        GetNextArrayParameter<VkDeviceSize>(buffers, params->bindingCount);
                    vkCmdBindVertexBuffers(cmdBuffer, 0, params->bindingCount, buffers, offsets);
                    break;
                }
                case CommandID::BindVertexBuffers2:
                {
                    const BindVertexBuffers2Params *params =
                        getParamPtr<BindVertexBuffers2Params>(currentCommand);
                    const VkBuffer *buffers = GetFirstArrayParameter<VkBuffer>(params);
                    const VkDeviceSize *offsets =
                        GetNextArrayParameter<VkDeviceSize>(buffers, params->bindingCount);
                    const VkDeviceSize *strides =
                        GetNextArrayParameter<VkDeviceSize>(offsets, params->bindingCount);
                    vkCmdBindVertexBuffers2EXT(cmdBuffer, 0, params->bindingCount, buffers, offsets,
                                               nullptr, strides);
                    break;
                }
                case CommandID::BlitImage:
                {
                    const BlitImageParams *params = getParamPtr<BlitImageParams>(currentCommand);
                    vkCmdBlitImage(cmdBuffer, params->srcImage,
                                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, params->dstImage,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &params->region,
                                   params->filter);
                    break;
                }
                case CommandID::BufferBarrier:
                {
                    const BufferBarrierParams *params =
                        getParamPtr<BufferBarrierParams>(currentCommand);
                    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                         VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 1,
                                         &params->bufferMemoryBarrier, 0, nullptr);
                    break;
                }
                case CommandID::BufferBarrier2:
                {
                    const BufferBarrier2Params *params =
                        getParamPtr<BufferBarrier2Params>(currentCommand);

                    const VkDependencyInfo dependencyInfo = {
                        VK_STRUCTURE_TYPE_DEPENDENCY_INFO, nullptr, 0,      0, nullptr, 1,
                        &params->bufferMemoryBarrier2,     0,       nullptr};

                    vkCmdPipelineBarrier2KHR(cmdBuffer, &dependencyInfo);
                    break;
                }
                case CommandID::ClearAttachments:
                {
                    const ClearAttachmentsParams *params =
                        getParamPtr<ClearAttachmentsParams>(currentCommand);
                    const VkClearAttachment *attachments =
                        GetFirstArrayParameter<VkClearAttachment>(params);
                    vkCmdClearAttachments(cmdBuffer, params->attachmentCount, attachments, 1,
                                          &params->rect);
                    break;
                }
                case CommandID::ClearColorImage:
                {
                    const ClearColorImageParams *params =
                        getParamPtr<ClearColorImageParams>(currentCommand);
                    vkCmdClearColorImage(cmdBuffer, params->image, params->imageLayout,
                                         &params->color, 1, &params->range);
                    break;
                }
                case CommandID::ClearDepthStencilImage:
                {
                    const ClearDepthStencilImageParams *params =
                        getParamPtr<ClearDepthStencilImageParams>(currentCommand);
                    vkCmdClearDepthStencilImage(cmdBuffer, params->image, params->imageLayout,
                                                &params->depthStencil, 1, &params->range);
                    break;
                }
                case CommandID::CopyBuffer:
                {
                    const CopyBufferParams *params = getParamPtr<CopyBufferParams>(currentCommand);
                    const VkBufferCopy *regions    = GetFirstArrayParameter<VkBufferCopy>(params);
                    vkCmdCopyBuffer(cmdBuffer, params->srcBuffer, params->destBuffer,
                                    params->regionCount, regions);
                    break;
                }
                case CommandID::CopyBufferToImage:
                {
                    const CopyBufferToImageParams *params =
                        getParamPtr<CopyBufferToImageParams>(currentCommand);
                    vkCmdCopyBufferToImage(cmdBuffer, params->srcBuffer, params->dstImage,
                                           params->dstImageLayout, 1, &params->region);
                    break;
                }
                case CommandID::CopyImage:
                {
                    const CopyImageParams *params = getParamPtr<CopyImageParams>(currentCommand);
                    vkCmdCopyImage(cmdBuffer, params->srcImage, params->srcImageLayout,
                                   params->dstImage, params->dstImageLayout, 1, &params->region);
                    break;
                }
                case CommandID::CopyImageToBuffer:
                {
                    const CopyImageToBufferParams *params =
                        getParamPtr<CopyImageToBufferParams>(currentCommand);
                    vkCmdCopyImageToBuffer(cmdBuffer, params->srcImage, params->srcImageLayout,
                                           params->dstBuffer, 1, &params->region);
                    break;
                }
                case CommandID::Dispatch:
                {
                    const DispatchParams *params = getParamPtr<DispatchParams>(currentCommand);
                    vkCmdDispatch(cmdBuffer, params->groupCountX, params->groupCountY,
                                  params->groupCountZ);
                    break;
                }
                case CommandID::DispatchIndirect:
                {
                    const DispatchIndirectParams *params =
                        getParamPtr<DispatchIndirectParams>(currentCommand);
                    vkCmdDispatchIndirect(cmdBuffer, params->buffer, params->offset);
                    break;
                }
                case CommandID::Draw:
                {
                    const DrawParams *params = getParamPtr<DrawParams>(currentCommand);
                    vkCmdDraw(cmdBuffer, params->vertexCount, 1, params->firstVertex, 0);
                    break;
                }
                case CommandID::DrawIndexed:
                {
                    const DrawIndexedParams *params =
                        getParamPtr<DrawIndexedParams>(currentCommand);
                    vkCmdDrawIndexed(cmdBuffer, params->indexCount, 1, 0, 0, 0);
                    break;
                }
                case CommandID::DrawIndexedBaseVertex:
                {
                    const DrawIndexedBaseVertexParams *params =
                        getParamPtr<DrawIndexedBaseVertexParams>(currentCommand);
                    vkCmdDrawIndexed(cmdBuffer, params->indexCount, 1, 0, params->vertexOffset, 0);
                    break;
                }
                case CommandID::DrawIndexedIndirect:
                {
                    const DrawIndexedIndirectParams *params =
                        getParamPtr<DrawIndexedIndirectParams>(currentCommand);
                    vkCmdDrawIndexedIndirect(cmdBuffer, params->buffer, params->offset,
                                             params->drawCount, params->stride);
                    break;
                }
                case CommandID::DrawIndexedInstanced:
                {
                    const DrawIndexedInstancedParams *params =
                        getParamPtr<DrawIndexedInstancedParams>(currentCommand);
                    vkCmdDrawIndexed(cmdBuffer, params->indexCount, params->instanceCount, 0, 0, 0);
                    break;
                }
                case CommandID::DrawIndexedInstancedBaseVertex:
                {
                    const DrawIndexedInstancedBaseVertexParams *params =
                        getParamPtr<DrawIndexedInstancedBaseVertexParams>(currentCommand);
                    vkCmdDrawIndexed(cmdBuffer, params->indexCount, params->instanceCount, 0,
                                     params->vertexOffset, 0);
                    break;
                }
                case CommandID::DrawIndexedInstancedBaseVertexBaseInstance:
                {
                    const DrawIndexedInstancedBaseVertexBaseInstanceParams *params =
                        getParamPtr<DrawIndexedInstancedBaseVertexBaseInstanceParams>(
                            currentCommand);
                    vkCmdDrawIndexed(cmdBuffer, params->indexCount, params->instanceCount,
                                     params->firstIndex, params->vertexOffset,
                                     params->firstInstance);
                    break;
                }
                case CommandID::DrawIndirect:
                {
                    const DrawIndirectParams *params =
                        getParamPtr<DrawIndirectParams>(currentCommand);
                    vkCmdDrawIndirect(cmdBuffer, params->buffer, params->offset, params->drawCount,
                                      params->stride);
                    break;
                }
                case CommandID::DrawInstanced:
                {
                    const DrawInstancedParams *params =
                        getParamPtr<DrawInstancedParams>(currentCommand);
                    vkCmdDraw(cmdBuffer, params->vertexCount, params->instanceCount,
                              params->firstVertex, 0);
                    break;
                }
                case CommandID::DrawInstancedBaseInstance:
                {
                    const DrawInstancedBaseInstanceParams *params =
                        getParamPtr<DrawInstancedBaseInstanceParams>(currentCommand);
                    vkCmdDraw(cmdBuffer, params->vertexCount, params->instanceCount,
                              params->firstVertex, params->firstInstance);
                    break;
                }
                case CommandID::EndDebugUtilsLabel:
                {
                    ASSERT(vkCmdEndDebugUtilsLabelEXT);
                    vkCmdEndDebugUtilsLabelEXT(cmdBuffer);
                    break;
                }
                case CommandID::EndQuery:
                {
                    const EndQueryParams *params = getParamPtr<EndQueryParams>(currentCommand);
                    vkCmdEndQuery(cmdBuffer, params->queryPool, params->query);
                    break;
                }
                case CommandID::EndTransformFeedback:
                {
                    const EndTransformFeedbackParams *params =
                        getParamPtr<EndTransformFeedbackParams>(currentCommand);
                    const VkBuffer *counterBuffers = GetFirstArrayParameter<VkBuffer>(params);
                    const VkDeviceSize *counterBufferOffsets =
                        reinterpret_cast<const VkDeviceSize *>(counterBuffers +
                                                               params->bufferCount);
                    vkCmdEndTransformFeedbackEXT(cmdBuffer, 0, params->bufferCount, counterBuffers,
                                                 counterBufferOffsets);
                    break;
                }
                case CommandID::FillBuffer:
                {
                    const FillBufferParams *params = getParamPtr<FillBufferParams>(currentCommand);
                    vkCmdFillBuffer(cmdBuffer, params->dstBuffer, params->dstOffset, params->size,
                                    params->data);
                    break;
                }
                case CommandID::ImageBarrier:
                {
                    const ImageBarrierParams *params =
                        getParamPtr<ImageBarrierParams>(currentCommand);
                    const VkImageMemoryBarrier *imageMemoryBarriers =
                        GetFirstArrayParameter<VkImageMemoryBarrier>(params);
                    vkCmdPipelineBarrier(cmdBuffer, params->srcStageMask, params->dstStageMask, 0,
                                         0, nullptr, 0, nullptr, 1, imageMemoryBarriers);
                    break;
                }
                case CommandID::ImageBarrier2:
                {
                    const ImageBarrier2Params *params =
                        getParamPtr<ImageBarrier2Params>(currentCommand);
                    const VkImageMemoryBarrier2 *imageMemoryBarriers2 =
                        GetFirstArrayParameter<VkImageMemoryBarrier2>(params);
                    VkDependencyInfo pDependencyInfo         = {};
                    pDependencyInfo.sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
                    pDependencyInfo.dependencyFlags          = 0;
                    pDependencyInfo.memoryBarrierCount       = 0;
                    pDependencyInfo.pMemoryBarriers          = nullptr;
                    pDependencyInfo.bufferMemoryBarrierCount = 0;
                    pDependencyInfo.pBufferMemoryBarriers    = nullptr;
                    pDependencyInfo.imageMemoryBarrierCount  = 1;
                    pDependencyInfo.pImageMemoryBarriers     = imageMemoryBarriers2;
                    vkCmdPipelineBarrier2KHR(cmdBuffer, &pDependencyInfo);
                    break;
                }
                case CommandID::ImageWaitEvent:
                {
                    const ImageWaitEventParams *params =
                        getParamPtr<ImageWaitEventParams>(currentCommand);
                    const VkImageMemoryBarrier *imageMemoryBarriers =
                        GetFirstArrayParameter<VkImageMemoryBarrier>(params);
                    vkCmdWaitEvents(cmdBuffer, 1, &(params->event), params->srcStageMask,
                                    params->dstStageMask, 0, nullptr, 0, nullptr, 1,
                                    imageMemoryBarriers);
                    break;
                }
                case CommandID::InsertDebugUtilsLabel:
                {
                    const DebugUtilsLabelParams *params =
                        getParamPtr<DebugUtilsLabelParams>(currentCommand);
                    const char *pLabelName           = GetFirstArrayParameter<char>(params);
                    const VkDebugUtilsLabelEXT label = {
                        VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                        nullptr,
                        pLabelName,
                        {params->color[0], params->color[1], params->color[2], params->color[3]}};
                    ASSERT(vkCmdInsertDebugUtilsLabelEXT);
                    vkCmdInsertDebugUtilsLabelEXT(cmdBuffer, &label);
                    break;
                }
                case CommandID::MemoryBarrier:
                {
                    const MemoryBarrierParams *params =
                        getParamPtr<MemoryBarrierParams>(currentCommand);
                    const VkMemoryBarrier *memoryBarriers =
                        GetFirstArrayParameter<VkMemoryBarrier>(params);
                    vkCmdPipelineBarrier(cmdBuffer, params->srcStageMask, params->dstStageMask, 0,
                                         1, memoryBarriers, 0, nullptr, 0, nullptr);
                    break;
                }
                case CommandID::MemoryBarrier2:
                {
                    const MemoryBarrier2Params *params =
                        getParamPtr<MemoryBarrier2Params>(currentCommand);

                    const VkMemoryBarrier2 *memoryBarriers2 =
                        GetFirstArrayParameter<VkMemoryBarrier2>(params);
                    VkDependencyInfo pDependencyInfo         = {};
                    pDependencyInfo.sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
                    pDependencyInfo.memoryBarrierCount       = 1;
                    pDependencyInfo.pMemoryBarriers          = memoryBarriers2;
                    pDependencyInfo.bufferMemoryBarrierCount = 0;
                    pDependencyInfo.pBufferMemoryBarriers    = nullptr;
                    pDependencyInfo.imageMemoryBarrierCount  = 0;
                    pDependencyInfo.pImageMemoryBarriers     = nullptr;
                    vkCmdPipelineBarrier2KHR(cmdBuffer, &pDependencyInfo);
                    break;
                }
                case CommandID::NextSubpass:
                {
                    vkCmdNextSubpass(cmdBuffer, VK_SUBPASS_CONTENTS_INLINE);
                    break;
                }
                case CommandID::PipelineBarrier:
                {
                    const PipelineBarrierParams *params =
                        getParamPtr<PipelineBarrierParams>(currentCommand);
                    const VkMemoryBarrier *memoryBarriers =
                        GetFirstArrayParameter<VkMemoryBarrier>(params);
                    const VkImageMemoryBarrier *imageMemoryBarriers =
                        GetNextArrayParameter<VkImageMemoryBarrier>(memoryBarriers,
                                                                    params->memoryBarrierCount);
                    vkCmdPipelineBarrier(cmdBuffer, params->srcStageMask, params->dstStageMask,
                                         params->dependencyFlags, params->memoryBarrierCount,
                                         memoryBarriers, 0, nullptr,
                                         params->imageMemoryBarrierCount, imageMemoryBarriers);
                    break;
                }
                case CommandID::PipelineBarrier2:
                {
                    const PipelineBarrierParams2 *params =
                        getParamPtr<PipelineBarrierParams2>(currentCommand);
                    const VkMemoryBarrier2 *memoryBarriers2 =
                        GetFirstArrayParameter<VkMemoryBarrier2>(params);
                    const VkImageMemoryBarrier2 *imageMemoryBarriers2 =
                        GetNextArrayParameter<VkImageMemoryBarrier2>(memoryBarriers2,
                                                                     params->memoryBarrierCount);
                    VkDependencyInfo dependencyInfo         = {};
                    dependencyInfo.sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
                    dependencyInfo.pNext                    = nullptr;
                    dependencyInfo.dependencyFlags          = params->dependencyFlags;
                    dependencyInfo.memoryBarrierCount       = params->memoryBarrierCount;
                    dependencyInfo.pMemoryBarriers          = memoryBarriers2;
                    dependencyInfo.bufferMemoryBarrierCount = 0;
                    dependencyInfo.pBufferMemoryBarriers    = nullptr;
                    dependencyInfo.imageMemoryBarrierCount  = params->imageMemoryBarrierCount;
                    dependencyInfo.pImageMemoryBarriers     = imageMemoryBarriers2;
                    vkCmdPipelineBarrier2KHR(cmdBuffer, &dependencyInfo);
                    break;
                }
                case CommandID::PushConstants:
                {
                    const PushConstantsParams *params =
                        getParamPtr<PushConstantsParams>(currentCommand);
                    const void *data = GetFirstArrayParameter<void>(params);
                    vkCmdPushConstants(cmdBuffer, params->layout, params->flag, params->offset,
                                       params->size, data);
                    break;
                }
                case CommandID::ResetEvent:
                {
                    const ResetEventParams *params = getParamPtr<ResetEventParams>(currentCommand);
                    vkCmdResetEvent(cmdBuffer, params->event, params->stageMask);
                    break;
                }
                case CommandID::ResetQueryPool:
                {
                    const ResetQueryPoolParams *params =
                        getParamPtr<ResetQueryPoolParams>(currentCommand);
                    vkCmdResetQueryPool(cmdBuffer, params->queryPool, params->firstQuery,
                                        params->queryCount);
                    break;
                }
                case CommandID::ResolveImage:
                {
                    const ResolveImageParams *params =
                        getParamPtr<ResolveImageParams>(currentCommand);
                    vkCmdResolveImage(cmdBuffer, params->srcImage,
                                      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, params->dstImage,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &params->region);
                    break;
                }
                case CommandID::SetBlendConstants:
                {
                    const SetBlendConstantsParams *params =
                        getParamPtr<SetBlendConstantsParams>(currentCommand);
                    vkCmdSetBlendConstants(cmdBuffer, params->blendConstants);
                    break;
                }
                case CommandID::SetCullMode:
                {
                    const SetCullModeParams *params =
                        getParamPtr<SetCullModeParams>(currentCommand);
                    vkCmdSetCullModeEXT(cmdBuffer, params->cullMode);
                    break;
                }
                case CommandID::SetDepthBias:
                {
                    const SetDepthBiasParams *params =
                        getParamPtr<SetDepthBiasParams>(currentCommand);
                    vkCmdSetDepthBias(cmdBuffer, params->depthBiasConstantFactor,
                                      params->depthBiasClamp, params->depthBiasSlopeFactor);
                    break;
                }
                case CommandID::SetDepthBiasEnable:
                {
                    const SetDepthBiasEnableParams *params =
                        getParamPtr<SetDepthBiasEnableParams>(currentCommand);
                    vkCmdSetDepthBiasEnableEXT(cmdBuffer, params->depthBiasEnable);
                    break;
                }
                case CommandID::SetDepthCompareOp:
                {
                    const SetDepthCompareOpParams *params =
                        getParamPtr<SetDepthCompareOpParams>(currentCommand);
                    vkCmdSetDepthCompareOpEXT(cmdBuffer, params->depthCompareOp);
                    break;
                }
                case CommandID::SetDepthTestEnable:
                {
                    const SetDepthTestEnableParams *params =
                        getParamPtr<SetDepthTestEnableParams>(currentCommand);
                    vkCmdSetDepthTestEnableEXT(cmdBuffer, params->depthTestEnable);
                    break;
                }
                case CommandID::SetDepthWriteEnable:
                {
                    const SetDepthWriteEnableParams *params =
                        getParamPtr<SetDepthWriteEnableParams>(currentCommand);
                    vkCmdSetDepthWriteEnableEXT(cmdBuffer, params->depthWriteEnable);
                    break;
                }
                case CommandID::SetEvent:
                {
                    const SetEventParams *params = getParamPtr<SetEventParams>(currentCommand);
                    vkCmdSetEvent(cmdBuffer, params->event, params->stageMask);
                    break;
                }
                case CommandID::SetFragmentShadingRate:
                {
                    const SetFragmentShadingRateParams *params =
                        getParamPtr<SetFragmentShadingRateParams>(currentCommand);
                    const VkExtent2D fragmentSize = {params->fragmentWidth, params->fragmentHeight};
                    const VkFragmentShadingRateCombinerOpKHR ops[2] = {
                        VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
                        static_cast<VkFragmentShadingRateCombinerOpKHR>(
                            params->vkFragmentShadingRateCombinerOp1)};
                    vkCmdSetFragmentShadingRateKHR(cmdBuffer, &fragmentSize, ops);
                    break;
                }
                case CommandID::SetFrontFace:
                {
                    const SetFrontFaceParams *params =
                        getParamPtr<SetFrontFaceParams>(currentCommand);
                    vkCmdSetFrontFaceEXT(cmdBuffer, params->frontFace);
                    break;
                }
                case CommandID::SetLineWidth:
                {
                    const SetLineWidthParams *params =
                        getParamPtr<SetLineWidthParams>(currentCommand);
                    vkCmdSetLineWidth(cmdBuffer, params->lineWidth);
                    break;
                }
                case CommandID::SetLogicOp:
                {
                    const SetLogicOpParams *params = getParamPtr<SetLogicOpParams>(currentCommand);
                    vkCmdSetLogicOpEXT(cmdBuffer, params->logicOp);
                    break;
                }
                case CommandID::SetPrimitiveRestartEnable:
                {
                    const SetPrimitiveRestartEnableParams *params =
                        getParamPtr<SetPrimitiveRestartEnableParams>(currentCommand);
                    vkCmdSetPrimitiveRestartEnableEXT(cmdBuffer, params->primitiveRestartEnable);
                    break;
                }
                case CommandID::SetRasterizerDiscardEnable:
                {
                    const SetRasterizerDiscardEnableParams *params =
                        getParamPtr<SetRasterizerDiscardEnableParams>(currentCommand);
                    vkCmdSetRasterizerDiscardEnableEXT(cmdBuffer, params->rasterizerDiscardEnable);
                    break;
                }
                case CommandID::SetScissor:
                {
                    const SetScissorParams *params = getParamPtr<SetScissorParams>(currentCommand);
                    vkCmdSetScissor(cmdBuffer, 0, 1, &params->scissor);
                    break;
                }
                case CommandID::SetStencilCompareMask:
                {
                    const SetStencilCompareMaskParams *params =
                        getParamPtr<SetStencilCompareMaskParams>(currentCommand);
                    vkCmdSetStencilCompareMask(cmdBuffer, VK_STENCIL_FACE_FRONT_BIT,
                                               params->compareFrontMask);
                    vkCmdSetStencilCompareMask(cmdBuffer, VK_STENCIL_FACE_BACK_BIT,
                                               params->compareBackMask);
                    break;
                }
                case CommandID::SetStencilOp:
                {
                    const SetStencilOpParams *params =
                        getParamPtr<SetStencilOpParams>(currentCommand);
                    vkCmdSetStencilOpEXT(cmdBuffer,
                                         static_cast<VkStencilFaceFlags>(params->faceMask),
                                         static_cast<VkStencilOp>(params->failOp),
                                         static_cast<VkStencilOp>(params->passOp),
                                         static_cast<VkStencilOp>(params->depthFailOp),
                                         static_cast<VkCompareOp>(params->compareOp));
                    break;
                }
                case CommandID::SetStencilReference:
                {
                    const SetStencilReferenceParams *params =
                        getParamPtr<SetStencilReferenceParams>(currentCommand);
                    vkCmdSetStencilReference(cmdBuffer, VK_STENCIL_FACE_FRONT_BIT,
                                             params->frontReference);
                    vkCmdSetStencilReference(cmdBuffer, VK_STENCIL_FACE_BACK_BIT,
                                             params->backReference);
                    break;
                }
                case CommandID::SetStencilTestEnable:
                {
                    const SetStencilTestEnableParams *params =
                        getParamPtr<SetStencilTestEnableParams>(currentCommand);
                    vkCmdSetStencilTestEnableEXT(cmdBuffer, params->stencilTestEnable);
                    break;
                }
                case CommandID::SetStencilWriteMask:
                {
                    const SetStencilWriteMaskParams *params =
                        getParamPtr<SetStencilWriteMaskParams>(currentCommand);
                    vkCmdSetStencilWriteMask(cmdBuffer, VK_STENCIL_FACE_FRONT_BIT,
                                             params->writeFrontMask);
                    vkCmdSetStencilWriteMask(cmdBuffer, VK_STENCIL_FACE_BACK_BIT,
                                             params->writeBackMask);
                    break;
                }
                case CommandID::SetVertexInput:
                {
                    const SetVertexInputParams *params =
                        getParamPtr<SetVertexInputParams>(currentCommand);
                    const VkVertexInputBindingDescription2EXT *vertexBindingDescriptions =
                        GetFirstArrayParameter<VkVertexInputBindingDescription2EXT>(params);
                    const VkVertexInputAttributeDescription2EXT *vertexAttributeDescriptions =
                        GetNextArrayParameter<VkVertexInputAttributeDescription2EXT,
                                              VkVertexInputBindingDescription2EXT>(
                            vertexBindingDescriptions, params->vertexBindingDescriptionCount);

                    vkCmdSetVertexInputEXT(
                        cmdBuffer, params->vertexBindingDescriptionCount, vertexBindingDescriptions,
                        params->vertexAttributeDescriptionCount, vertexAttributeDescriptions);
                    break;
                }
                case CommandID::SetViewport:
                {
                    const SetViewportParams *params =
                        getParamPtr<SetViewportParams>(currentCommand);
                    vkCmdSetViewport(cmdBuffer, 0, 1, &params->viewport);
                    break;
                }
                case CommandID::WaitEvents:
                {
                    const WaitEventsParams *params = getParamPtr<WaitEventsParams>(currentCommand);
                    const VkEvent *events          = GetFirstArrayParameter<VkEvent>(params);
                    const VkMemoryBarrier *memoryBarriers =
                        GetNextArrayParameter<VkMemoryBarrier>(events, params->eventCount);
                    const VkImageMemoryBarrier *imageMemoryBarriers =
                        GetNextArrayParameter<VkImageMemoryBarrier>(memoryBarriers,
                                                                    params->memoryBarrierCount);
                    vkCmdWaitEvents(cmdBuffer, params->eventCount, events, params->srcStageMask,
                                    params->dstStageMask, params->memoryBarrierCount,
                                    memoryBarriers, 0, nullptr, params->imageMemoryBarrierCount,
                                    imageMemoryBarriers);
                    break;
                }
                case CommandID::WriteTimestamp:
                {
                    const WriteTimestampParams *params =
                        getParamPtr<WriteTimestampParams>(currentCommand);
                    vkCmdWriteTimestamp(cmdBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                        params->queryPool, params->query);
                    break;
                }
                case CommandID::WriteTimestamp2:
                {
                    const WriteTimestampParams *params =
                        getParamPtr<WriteTimestampParams>(currentCommand);
                    vkCmdWriteTimestamp2KHR(cmdBuffer, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
                                            params->queryPool, params->query);
                    break;
                }
                default:
                {
                    UNREACHABLE();
                    break;
                }
            }
        }
    }
}

void SecondaryCommandBuffer::getMemoryUsageStats(size_t *usedMemoryOut,
                                                 size_t *allocatedMemoryOut) const
{
    mCommandAllocator.getMemoryUsageStats(usedMemoryOut, allocatedMemoryOut);
}

void SecondaryCommandBuffer::getMemoryUsageStatsForPoolAlloc(size_t blockSize,
                                                             size_t *usedMemoryOut,
                                                             size_t *allocatedMemoryOut) const
{
    *allocatedMemoryOut = blockSize * mCommands.size();

    *usedMemoryOut = 0;
    for (const CommandHeader *command : mCommands)
    {
        const CommandHeader *commandEnd = command;
        while (commandEnd->id != CommandID::Invalid)
        {
            commandEnd = NextCommand(commandEnd);
        }

        *usedMemoryOut += reinterpret_cast<const uint8_t *>(commandEnd) -
                          reinterpret_cast<const uint8_t *>(command) + sizeof(CommandHeader::id);
    }

    ASSERT(*usedMemoryOut <= *allocatedMemoryOut);
}

std::string SecondaryCommandBuffer::dumpCommands(const char *separator) const
{
    std::stringstream result;
    for (const CommandHeader *command : mCommands)
    {
        for (const CommandHeader *currentCommand                      = command;
             currentCommand->id != CommandID::Invalid; currentCommand = NextCommand(currentCommand))
        {
            result << GetCommandString(currentCommand->id) << separator;
        }
    }
    return result.str();
}

}  // namespace priv
}  // namespace vk
}  // namespace rx
