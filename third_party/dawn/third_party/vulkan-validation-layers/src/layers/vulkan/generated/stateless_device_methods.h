// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See stateless_validation_helper_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ****************************************************************************/

// NOLINTBEGIN
#pragma once
bool PreCallValidateDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                  const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue,
                                   const ErrorObject& error_obj) const override;
bool PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                const ErrorObject& error_obj) const override;
bool PreCallValidateQueueWaitIdle(VkQueue queue, const ErrorObject& error_obj) const override;
bool PreCallValidateDeviceWaitIdle(VkDevice device, const ErrorObject& error_obj) const override;
bool PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                   const ErrorObject& error_obj) const override;
bool PreCallValidateFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator,
                               const ErrorObject& error_obj) const override;
bool PreCallValidateMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                              VkMemoryMapFlags flags, void** ppData, const ErrorObject& error_obj) const override;
bool PreCallValidateUnmapMemory(VkDevice device, VkDeviceMemory memory, const ErrorObject& error_obj) const override;
bool PreCallValidateFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                 const VkMappedMemoryRange* pMemoryRanges,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount,
                                                     VkSparseImageMemoryRequirements* pSparseMemoryRequirements,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                VkFence* pFence, const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator,
                                 const ErrorObject& error_obj) const override;
bool PreCallValidateResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences,
                                const ErrorObject& error_obj) const override;
bool PreCallValidateGetFenceStatus(VkDevice device, VkFence fence, const ErrorObject& error_obj) const override;
bool PreCallValidateWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout,
                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                VkEvent* pEvent, const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator,
                                 const ErrorObject& error_obj) const override;
bool PreCallValidateGetEventStatus(VkDevice device, VkEvent event, const ErrorObject& error_obj) const override;
bool PreCallValidateSetEvent(VkDevice device, VkEvent event, const ErrorObject& error_obj) const override;
bool PreCallValidateResetEvent(VkDevice device, VkEvent event, const ErrorObject& error_obj) const override;
bool PreCallValidateCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                        size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                 VkBuffer* pBuffer, const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator,
                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkBufferView* pView,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                VkImage* pImage, const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator,
                                 const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                              VkSubresourceLayout* pLayout, const ErrorObject& error_obj) const override;
bool PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkImageView* pView,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                        const VkPipelineCache* pSrcCaches, const ErrorObject& error_obj) const override;
bool PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                            const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                            const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                           const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                           VkPipeline* pPipelines, const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                  VkSampler* pSampler, const ErrorObject& error_obj) const override;
bool PreCallValidateDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator,
                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                               const VkAllocationCallbacks* pAllocator,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                           VkDescriptorSet* pDescriptorSets, const ErrorObject& error_obj) const override;
bool PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                       const VkDescriptorSet* pDescriptorSets, const ErrorObject& error_obj) const override;
bool PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                         const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                         const VkCopyDescriptorSet* pDescriptorCopies, const ErrorObject& error_obj) const override;
bool PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                           VkCommandBuffer* pCommandBuffers, const ErrorObject& error_obj) const override;
bool PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                       const VkCommandBuffer* pCommandBuffers, const ErrorObject& error_obj) const override;
bool PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateEndCommandBuffer(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
bool PreCallValidateResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                   const VkViewport* pViewports, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                  const VkRect2D* pScissors, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                    float depthBiasSlopeFactor, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4],
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                          VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                          const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                          const uint32_t* pDynamicOffsets, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                         const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                            uint32_t firstInstance, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                                   int32_t vertexOffset, uint32_t firstInstance, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                    uint32_t stride, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                           uint32_t stride, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                  const VkBufferCopy* pRegions, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                 VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions,
                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                 VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter,
                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                         VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                         VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                    VkDeviceSize dataSize, const void* pData, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                  uint32_t data, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                       const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                              const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                              const VkImageSubresourceRange* pRanges, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                        const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                    VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                const ErrorObject& error_obj) const override;
bool PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                  VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount,
                                  const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                                  const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                                  const VkImageMemoryBarrier* pImageMemoryBarriers, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                       VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                       uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                       uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                       uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags,
                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                const ErrorObject& error_obj) const override;
bool PreCallValidateCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                      uint32_t queryCount, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool,
                                      uint32_t query, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                            uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride,
                                            VkQueryResultFlags flags, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                     uint32_t offset, uint32_t size, const void* pValues,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                       VkSubpassContents contents, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                       const VkCommandBuffer* pCommandBuffers, const ErrorObject& error_obj) const override;
bool PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                     uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                    uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                VkMemoryRequirements2* pMemoryRequirements,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                 VkMemoryRequirements2* pMemoryRequirements,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                      uint32_t* pSparseMemoryRequirementCount,
                                                      VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator,
                                                 VkSamplerYcbcrConversion* pYcbcrConversion,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                    VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                  VkDescriptorSetLayoutSupport* pSupport,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer,
                                         VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                uint32_t stride, const ErrorObject& error_obj) const override;
bool PreCallValidateCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                        const VkSubpassBeginInfo* pSubpassBeginInfo, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                    const VkSubpassEndInfo* pSubpassEndInfo, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                   const ErrorObject& error_obj) const override;
bool PreCallValidateGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                   const ErrorObject& error_obj) const override;
bool PreCallValidateSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                        const ErrorObject& error_obj) const override;
bool PreCallValidateCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                           const VkAllocationCallbacks* pAllocator, const ErrorObject& error_obj) const override;
bool PreCallValidateSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                   VkPrivateDataSlot privateDataSlot, uint64_t data, const ErrorObject& error_obj) const override;
bool PreCallValidateGetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                   VkPrivateDataSlot privateDataSlot, uint64_t* pData, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                   const VkDependencyInfo* pDependencyInfos, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                       uint32_t query, const ErrorObject& error_obj) const override;
bool PreCallValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdEndRendering(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                          const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                          const VkDeviceSize* pStrides, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                    VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceBufferMemoryRequirements(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                      VkMemoryRequirements2* pMemoryRequirements,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                     VkMemoryRequirements2* pMemoryRequirements,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceImageSparseMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                           uint32_t* pSparseMemoryRequirementCount,
                                                           VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetLineStipple(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateMapMemory2(VkDevice device, const VkMemoryMapInfo* pMemoryMapInfo, void** ppData,
                               const ErrorObject& error_obj) const override;
bool PreCallValidateUnmapMemory2(VkDevice device, const VkMemoryUnmapInfo* pMemoryUnmapInfo,
                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindIndexBuffer2(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size,
                                        VkIndexType indexType, const ErrorObject& error_obj) const override;
bool PreCallValidateGetRenderingAreaGranularity(VkDevice device, const VkRenderingAreaInfo* pRenderingAreaInfo,
                                                VkExtent2D* pGranularity, const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceImageSubresourceLayout(VkDevice device, const VkDeviceImageSubresourceInfo* pInfo,
                                                    VkSubresourceLayout2* pLayout, const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageSubresourceLayout2(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource,
                                               VkSubresourceLayout2* pLayout, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdPushDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                         VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                         const VkWriteDescriptorSet* pDescriptorWrites,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdPushDescriptorSetWithTemplate(VkCommandBuffer commandBuffer,
                                                     VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout,
                                                     uint32_t set, const void* pData, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetRenderingAttachmentLocations(VkCommandBuffer commandBuffer,
                                                       const VkRenderingAttachmentLocationInfo* pLocationInfo,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetRenderingInputAttachmentIndices(VkCommandBuffer commandBuffer,
                                                          const VkRenderingInputAttachmentIndexInfo* pInputAttachmentIndexInfo,
                                                          const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindDescriptorSets2(VkCommandBuffer commandBuffer, const VkBindDescriptorSetsInfo* pBindDescriptorSetsInfo,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdPushConstants2(VkCommandBuffer commandBuffer, const VkPushConstantsInfo* pPushConstantsInfo,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdPushDescriptorSet2(VkCommandBuffer commandBuffer, const VkPushDescriptorSetInfo* pPushDescriptorSetInfo,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateCmdPushDescriptorSetWithTemplate2(VkCommandBuffer commandBuffer,
                                                      const VkPushDescriptorSetWithTemplateInfo* pPushDescriptorSetWithTemplateInfo,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCopyMemoryToImage(VkDevice device, const VkCopyMemoryToImageInfo* pCopyMemoryToImageInfo,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCopyImageToMemory(VkDevice device, const VkCopyImageToMemoryInfo* pCopyImageToMemoryInfo,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCopyImageToImage(VkDevice device, const VkCopyImageToImageInfo* pCopyImageToImageInfo,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateTransitionImageLayout(VkDevice device, uint32_t transitionCount,
                                          const VkHostImageLayoutTransitionInfo* pTransitions,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                          VkImage* pSwapchainImages, const ErrorObject& error_obj) const override;
bool PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                        VkFence fence, uint32_t* pImageIndex, const ErrorObject& error_obj) const override;
bool PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceGroupPresentCapabilitiesKHR(VkDevice device,
                                                         VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities,
                                                         const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                         VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                         const ErrorObject& error_obj) const override;
bool PreCallValidateAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                              const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                              VkSwapchainKHR* pSwapchains, const ErrorObject& error_obj) const override;
bool PreCallValidateCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession, const VkAllocationCallbacks* pAllocator,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                         uint32_t* pMemoryRequirementsCount,
                                                         VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements,
                                                         const ErrorObject& error_obj) const override;
bool PreCallValidateBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession, uint32_t bindSessionMemoryInfoCount,
                                              const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateCreateVideoSessionParametersKHR(VkDevice device, const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    VkVideoSessionParametersKHR* pVideoSessionParameters,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                    const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR* pBeginInfo,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR* pEndCodingInfo,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoCodingControlInfoKHR* pCodingControlInfo,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                        uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures,
                                                        const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                       uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags,
                                       const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                            HANDLE* pHandle, const ErrorObject& error_obj) const override;
bool PreCallValidateGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, HANDLE handle,
                                                      VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties,
                                                      const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd,
                                   const ErrorObject& error_obj) const override;
bool PreCallValidateGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                             VkMemoryFdPropertiesKHR* pMemoryFdProperties,
                                             const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateImportSemaphoreWin32HandleKHR(VkDevice device,
                                                  const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                               HANDLE* pHandle, const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                            VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                            const VkWriteDescriptorSet* pDescriptorWrites,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                        VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                        VkPipelineLayout layout, uint32_t set, const void* pData,
                                                        const ErrorObject& error_obj) const override;
bool PreCallValidateCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                       VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                           const VkSubpassBeginInfo* pSubpassBeginInfo,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                       const VkSubpassEndInfo* pSubpassEndInfo, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain, const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateImportFenceWin32HandleKHR(VkDevice device, const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                           HANDLE* pHandle, const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd,
                                  const ErrorObject& error_obj) const override;
bool PreCallValidateAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateReleaseProfilingLockKHR(VkDevice device, const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                   VkMemoryRequirements2* pMemoryRequirements,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                    VkMemoryRequirements2* pMemoryRequirements,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                         uint32_t* pSparseMemoryRequirementCount,
                                                         VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    VkSamplerYcbcrConversion* pYcbcrConversion,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateGetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                     VkDescriptorSetLayoutSupport* pSupport,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                            uint32_t stride, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                   uint32_t stride, const ErrorObject& error_obj) const override;
bool PreCallValidateGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize,
                                                 const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetRenderingAttachmentLocationsKHR(VkCommandBuffer commandBuffer,
                                                          const VkRenderingAttachmentLocationInfo* pLocationInfo,
                                                          const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetRenderingInputAttachmentIndicesKHR(VkCommandBuffer commandBuffer,
                                                             const VkRenderingInputAttachmentIndexInfo* pInputAttachmentIndexInfo,
                                                             const ErrorObject& error_obj) const override;
bool PreCallValidateWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo,
                                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                               VkDeferredOperationKHR* pDeferredOperation,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                const VkAllocationCallbacks* pAllocator,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                          const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                       uint32_t* pExecutableCount, VkPipelineExecutablePropertiesKHR* pProperties,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetPipelineExecutableStatisticsKHR(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                       uint32_t* pStatisticCount, VkPipelineExecutableStatisticKHR* pStatistics,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations, const ErrorObject& error_obj) const override;
bool PreCallValidateMapMemory2KHR(VkDevice device, const VkMemoryMapInfo* pMemoryMapInfo, void** ppData,
                                  const ErrorObject& error_obj) const override;
bool PreCallValidateUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfo* pMemoryUnmapInfo,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateGetEncodedVideoSessionParametersKHR(VkDevice device,
                                                        const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
                                                        VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo,
                                                        size_t* pDataSize, void* pData,
                                                        const ErrorObject& error_obj) const override;
bool PreCallValidateCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                      const VkDependencyInfo* pDependencyInfos, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                          uint32_t query, const ErrorObject& error_obj) const override;
bool PreCallValidateQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceBufferMemoryRequirementsKHR(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                         VkMemoryRequirements2* pMemoryRequirements,
                                                         const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceImageMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                        VkMemoryRequirements2* pMemoryRequirements,
                                                        const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceImageSparseMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                              uint32_t* pSparseMemoryRequirementCount,
                                                              VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                              const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size,
                                           VkIndexType indexType, const ErrorObject& error_obj) const override;
bool PreCallValidateGetRenderingAreaGranularityKHR(VkDevice device, const VkRenderingAreaInfo* pRenderingAreaInfo,
                                                   VkExtent2D* pGranularity, const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceImageSubresourceLayoutKHR(VkDevice device, const VkDeviceImageSubresourceInfo* pInfo,
                                                       VkSubresourceLayout2* pLayout, const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageSubresourceLayout2KHR(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource,
                                                  VkSubresourceLayout2* pLayout, const ErrorObject& error_obj) const override;
bool PreCallValidateCreatePipelineBinariesKHR(VkDevice device, const VkPipelineBinaryCreateInfoKHR* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkPipelineBinaryHandlesInfoKHR* pBinaries,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyPipelineBinaryKHR(VkDevice device, VkPipelineBinaryKHR pipelineBinary,
                                             const VkAllocationCallbacks* pAllocator, const ErrorObject& error_obj) const override;
bool PreCallValidateGetPipelineKeyKHR(VkDevice device, const VkPipelineCreateInfoKHR* pPipelineCreateInfo,
                                      VkPipelineBinaryKeyKHR* pPipelineKey, const ErrorObject& error_obj) const override;
bool PreCallValidateGetPipelineBinaryDataKHR(VkDevice device, const VkPipelineBinaryDataInfoKHR* pInfo,
                                             VkPipelineBinaryKeyKHR* pPipelineBinaryKey, size_t* pPipelineBinaryDataSize,
                                             void* pPipelineBinaryData, const ErrorObject& error_obj) const override;
bool PreCallValidateReleaseCapturedPipelineDataKHR(VkDevice device, const VkReleaseCapturedPipelineDataInfoKHR* pInfo,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetLineStippleKHR(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateGetCalibratedTimestampsKHR(VkDevice device, uint32_t timestampCount,
                                               const VkCalibratedTimestampInfoKHR* pTimestampInfos, uint64_t* pTimestamps,
                                               uint64_t* pMaxDeviation, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer,
                                              const VkBindDescriptorSetsInfo* pBindDescriptorSetsInfo,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateCmdPushConstants2KHR(VkCommandBuffer commandBuffer, const VkPushConstantsInfo* pPushConstantsInfo,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer, const VkPushDescriptorSetInfo* pPushDescriptorSetInfo,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdPushDescriptorSetWithTemplate2KHR(
    VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo* pPushDescriptorSetWithTemplateInfo,
    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDescriptorBufferOffsets2EXT(VkCommandBuffer commandBuffer,
                                                      const VkSetDescriptorBufferOffsetsInfoEXT* pSetDescriptorBufferOffsetsInfo,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindDescriptorBufferEmbeddedSamplers2EXT(
    VkCommandBuffer commandBuffer, const VkBindDescriptorBufferEmbeddedSamplersInfoEXT* pBindDescriptorBufferEmbeddedSamplersInfo,
    const ErrorObject& error_obj) const override;
bool PreCallValidateDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT* pTagInfo,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT* pNameInfo,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                       const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                                       const VkDeviceSize* pSizes, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                 uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                 const VkDeviceSize* pCounterBufferOffsets,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                               uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                               const VkDeviceSize* pCounterBufferOffsets,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                            VkQueryControlFlags flags, uint32_t index, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                                VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                uint32_t vertexStride, const ErrorObject& error_obj) const override;
bool PreCallValidateCreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyCuModuleNVX(VkDevice device, VkCuModuleNVX module, const VkAllocationCallbacks* pAllocator,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyCuFunctionNVX(VkDevice device, VkCuFunctionNVX function, const VkAllocationCallbacks* pAllocator,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer, const VkCuLaunchInfoNVX* pLaunchInfo,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageViewHandle64NVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageViewAddressNVX(VkDevice device, VkImageView imageView, VkImageViewAddressPropertiesNVX* pProperties,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                            uint32_t stride, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                   uint32_t stride, const ErrorObject& error_obj) const override;
bool PreCallValidateGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                     VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo,
                                     const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory, VkExternalMemoryHandleTypeFlagsNV handleType,
                                           HANDLE* pHandle, const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                                    const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                             const VkViewportWScalingNV* pViewportWScalings,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display, const VkDisplayPowerInfoEXT* pDisplayPowerInfo,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                           const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display, const VkDisplayEventInfoEXT* pDisplayEventInfo,
                                            const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter,
                                           uint64_t* pCounterValue, const ErrorObject& error_obj) const override;
bool PreCallValidateGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                  VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount,
                                                    VkPastPresentationTimingGOOGLE* pPresentationTimings,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                              uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer, VkDiscardRectangleModeEXT discardRectangleMode,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains,
                                      const VkHdrMetadataEXT* pMetadata, const ErrorObject& error_obj) const override;
bool PreCallValidateSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateQueueEndDebugUtilsLabelEXT(VkQueue queue, const ErrorObject& error_obj) const override;
bool PreCallValidateQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                              VkAndroidHardwareBufferPropertiesANDROID* pProperties,
                                                              const ErrorObject& error_obj) const override;
bool PreCallValidateGetMemoryAndroidHardwareBufferANDROID(VkDevice device, const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                          struct AHardwareBuffer** pBuffer,
                                                          const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool PreCallValidateCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                      const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos,
                                                      const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateGetExecutionGraphPipelineScratchSizeAMDX(VkDevice device, VkPipeline executionGraph,
                                                             VkExecutionGraphPipelineScratchSizeAMDX* pSizeInfo,
                                                             const ErrorObject& error_obj) const override;
bool PreCallValidateGetExecutionGraphPipelineNodeIndexAMDX(VkDevice device, VkPipeline executionGraph,
                                                           const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo,
                                                           uint32_t* pNodeIndex, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdInitializeGraphScratchMemoryAMDX(VkCommandBuffer commandBuffer, VkPipeline executionGraph,
                                                        VkDeviceAddress scratch, VkDeviceSize scratchSize,
                                                        const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDispatchGraphAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch, VkDeviceSize scratchSize,
                                         const VkDispatchGraphCountInfoAMDX* pCountInfo,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDispatchGraphIndirectAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch, VkDeviceSize scratchSize,
                                                 const VkDispatchGraphCountInfoAMDX* pCountInfo,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDispatchGraphIndirectCountAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                      VkDeviceSize scratchSize, VkDeviceAddress countInfo,
                                                      const ErrorObject& error_obj) const override;
#endif  // VK_ENABLE_BETA_EXTENSIONS
bool PreCallValidateCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT* pSampleLocationsInfo,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image,
                                                           VkImageDrmFormatModifierPropertiesEXT* pProperties,
                                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkValidationCacheEXT* pValidationCache,
                                             const ErrorObject& error_obj) const;
bool PreCallValidateDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                              const VkAllocationCallbacks* pAllocator, const ErrorObject& error_obj) const;
bool PreCallValidateMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                             const VkValidationCacheEXT* pSrcCaches, const ErrorObject& error_obj) const;
bool PreCallValidateGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize, void* pData,
                                              const ErrorObject& error_obj) const;
bool PreCallValidateCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                       uint32_t viewportCount, const VkShadingRatePaletteNV* pShadingRatePalettes,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                              uint32_t customSampleOrderCount,
                                              const VkCoarseSampleOrderCustomNV* pCustomSampleOrders,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  VkAccelerationStructureNV* pAccelerationStructure,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateGetAccelerationStructureMemoryRequirementsNV(VkDevice device,
                                                                 const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
                                                                 VkMemoryRequirements2KHR* pMemoryRequirements,
                                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                      const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                                    VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                                    VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkBuffer scratch,
                                                    VkDeviceSize scratchOffset, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                   VkAccelerationStructureNV src, VkCopyAccelerationStructureModeKHR mode,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                   VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                   VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                   VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                   VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                   VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                   uint32_t width, uint32_t height, uint32_t depth, const ErrorObject& error_obj) const override;
bool PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                       uint32_t groupCount, size_t dataSize, void* pData,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                      uint32_t groupCount, size_t dataSize, void* pData,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                     size_t dataSize, void* pData, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount,
                                                               const VkAccelerationStructureNV* pAccelerationStructures,
                                                               VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery,
                                                               const ErrorObject& error_obj) const override;
bool PreCallValidateCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                      const void* pHostPointer,
                                                      VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                            VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkBuffer dstBuffer,
                                             VkDeviceSize dstOffset, uint32_t marker, const ErrorObject& error_obj) const override;
bool PreCallValidateGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                               const VkCalibratedTimestampInfoKHR* pTimestampInfos, uint64_t* pTimestamps,
                                               uint64_t* pMaxDeviation, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               uint32_t drawCount, uint32_t stride, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                    uint32_t stride, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                   uint32_t exclusiveScissorCount, const VkBool32* pExclusiveScissorEnables,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                             uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointDataNV* pCheckpointData,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateGetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointData2NV* pCheckpointData,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateInitializePerformanceApiINTEL(VkDevice device, const VkInitializePerformanceApiInfoINTEL* pInitializeInfo,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateUninitializePerformanceApiINTEL(VkDevice device, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceMarkerInfoINTEL* pMarkerInfo,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                       const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                   const VkPerformanceOverrideInfoINTEL* pOverrideInfo,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateAcquirePerformanceConfigurationINTEL(VkDevice device,
                                                         const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
                                                         VkPerformanceConfigurationINTEL* pConfiguration,
                                                         const ErrorObject& error_obj) const override;
bool PreCallValidateReleasePerformanceConfigurationINTEL(VkDevice device, VkPerformanceConfigurationINTEL configuration,
                                                         const ErrorObject& error_obj) const override;
bool PreCallValidateQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration,
                                                          const ErrorObject& error_obj) const override;
bool PreCallValidateGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter,
                                                 VkPerformanceValueINTEL* pValue, const ErrorObject& error_obj) const override;
bool PreCallValidateSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                              const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                      const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                             const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                             const VkDeviceSize* pStrides, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                       VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfo* pCopyMemoryToImageInfo,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfo* pCopyImageToMemoryInfo,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfo* pCopyImageToImageInfo,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                             const VkHostImageLayoutTransitionInfo* pTransitions,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageSubresourceLayout2EXT(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource,
                                                  VkSubresourceLayout2* pLayout, const ErrorObject& error_obj) const override;
bool PreCallValidateReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateGetGeneratedCommandsMemoryRequirementsNV(VkDevice device,
                                                             const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
                                                             VkMemoryRequirements2* pMemoryRequirements,
                                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer,
                                                     const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                  const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                 VkPipeline pipeline, uint32_t groupIndex,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCreateIndirectCommandsLayoutNV(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   VkIndirectCommandsLayoutNV* pIndirectCommandsLayout,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT* pDepthBiasInfo,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                              const VkAllocationCallbacks* pAllocator, const ErrorObject& error_obj) const override;
bool PreCallValidateSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                      VkPrivateDataSlot privateDataSlot, uint64_t data,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                      VkPrivateDataSlot privateDataSlot, uint64_t* pData,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCreateCudaModuleNV(VkDevice device, const VkCudaModuleCreateInfoNV* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkCudaModuleNV* pModule,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetCudaModuleCacheNV(VkDevice device, VkCudaModuleNV module, size_t* pCacheSize, void* pCacheData,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCreateCudaFunctionNV(VkDevice device, const VkCudaFunctionCreateInfoNV* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkCudaFunctionNV* pFunction,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyCudaModuleNV(VkDevice device, VkCudaModuleNV module, const VkAllocationCallbacks* pAllocator,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyCudaFunctionNV(VkDevice device, VkCudaFunctionNV function, const VkAllocationCallbacks* pAllocator,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCudaLaunchKernelNV(VkCommandBuffer commandBuffer, const VkCudaLaunchInfoNV* pLaunchInfo,
                                          const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_METAL_EXT
bool PreCallValidateExportMetalObjectsEXT(VkDevice device, VkExportMetalObjectsInfoEXT* pMetalObjectsInfo,
                                          const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_METAL_EXT
bool PreCallValidateGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout, VkDeviceSize* pLayoutSizeInBytes,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout, uint32_t binding,
                                                           VkDeviceSize* pOffset, const ErrorObject& error_obj) const override;
bool PreCallValidateGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT* pDescriptorInfo, size_t dataSize,
                                     void* pDescriptor, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                const VkDescriptorBufferBindingInfoEXT* pBindingInfos,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                     VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                                     const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                               VkPipelineLayout layout, uint32_t set,
                                                               const ErrorObject& error_obj) const override;
bool PreCallValidateGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
                                                            void* pData, const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageCaptureDescriptorDataInfoEXT* pInfo,
                                                           void* pData, const ErrorObject& error_obj) const override;
bool PreCallValidateGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                               const VkImageViewCaptureDescriptorDataInfoEXT* pInfo, void* pData,
                                                               const ErrorObject& error_obj) const override;
bool PreCallValidateGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
                                                             void* pData, const ErrorObject& error_obj) const override;
bool PreCallValidateGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo, void* pData,
    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate,
                                                    const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT* pFaultCounts, VkDeviceFaultInfoEXT* pFaultInfo,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                         const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                                         uint32_t vertexAttributeDescriptionCount,
                                         const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions,
                                         const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_FUCHSIA
bool PreCallValidateGetMemoryZirconHandleFUCHSIA(VkDevice device, const VkMemoryGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                 zx_handle_t* pZirconHandle, const ErrorObject& error_obj) const override;
bool PreCallValidateGetMemoryZirconHandlePropertiesFUCHSIA(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                           zx_handle_t zirconHandle,
                                                           VkMemoryZirconHandlePropertiesFUCHSIA* pMemoryZirconHandleProperties,
                                                           const ErrorObject& error_obj) const override;
bool PreCallValidateImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo,
    const ErrorObject& error_obj) const override;
bool PreCallValidateGetSemaphoreZirconHandleFUCHSIA(VkDevice device,
                                                    const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                    zx_handle_t* pZirconHandle, const ErrorObject& error_obj) const override;
bool PreCallValidateCreateBufferCollectionFUCHSIA(VkDevice device, const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkBufferCollectionFUCHSIA* pCollection,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateSetBufferCollectionImageConstraintsFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                               const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo,
                                                               const ErrorObject& error_obj) const override;
bool PreCallValidateSetBufferCollectionBufferConstraintsFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo,
                                                                const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyBufferCollectionFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateGetBufferCollectionPropertiesFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                         VkBufferCollectionPropertiesFUCHSIA* pProperties,
                                                         const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_FUCHSIA
bool PreCallValidateGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass,
                                                                  VkExtent2D* pMaxWorkgroupSize,
                                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateGetMemoryRemoteAddressNV(VkDevice device, const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo,
                                             VkRemoteAddressNV* pAddress, const ErrorObject& error_obj) const override;
bool PreCallValidateGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT* pPipelineInfo,
                                             VkBaseOutStructure* pPipelineProperties, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                              const VkBool32* pColorWriteEnables, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                    uint32_t instanceCount, uint32_t firstInstance, uint32_t stride,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                           const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                           uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap, const VkAllocationCallbacks* pAllocator,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkMicromapBuildInfoEXT* pInfos,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                      const VkMicromapBuildInfoEXT* pInfos, const ErrorObject& error_obj) const override;
bool PreCallValidateCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMicromapInfoEXT* pInfo,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                            const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                            const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount, const VkMicromapEXT* pMicromaps,
                                                VkQueryType queryType, size_t dataSize, void* pData, size_t stride,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT* pInfo,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapToMemoryInfoEXT* pInfo,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMemoryToMicromapInfoEXT* pInfo,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount,
                                                   const VkMicromapEXT* pMicromaps, VkQueryType queryType, VkQueryPool queryPool,
                                                   uint32_t firstQuery, const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceMicromapCompatibilityEXT(VkDevice device, const VkMicromapVersionInfoEXT* pVersionInfo,
                                                      VkAccelerationStructureCompatibilityKHR* pCompatibility,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                             const VkMicromapBuildInfoEXT* pBuildInfo, VkMicromapBuildSizesInfoEXT* pSizeInfo,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawClusterHUAWEI(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                         uint32_t groupCountZ, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateGetDescriptorSetLayoutHostMappingInfoVALVE(VkDevice device,
                                                               const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
                                                               VkDescriptorSetLayoutHostMappingInfoVALVE* pHostMapping,
                                                               const ErrorObject& error_obj) const override;
bool PreCallValidateGetDescriptorSetHostMappingVALVE(VkDevice device, VkDescriptorSet descriptorSet, void** ppData,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyMemoryIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress, uint32_t copyCount,
                                            uint32_t stride, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                   uint32_t copyCount, uint32_t stride, VkImage dstImage,
                                                   VkImageLayout dstImageLayout, const VkImageSubresourceLayers* pImageSubresources,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDecompressMemoryNV(VkCommandBuffer commandBuffer, uint32_t decompressRegionCount,
                                          const VkDecompressMemoryRegionNV* pDecompressMemoryRegions,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer, VkDeviceAddress indirectCommandsAddress,
                                                       VkDeviceAddress indirectCommandsCountAddress, uint32_t stride,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetPipelineIndirectMemoryRequirementsNV(VkDevice device, const VkComputePipelineCreateInfo* pCreateInfo,
                                                            VkMemoryRequirements2* pMemoryRequirements,
                                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCmdUpdatePipelineIndirectBufferNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                      VkPipeline pipeline, const ErrorObject& error_obj) const override;
bool PreCallValidateGetPipelineIndirectDeviceAddressNV(VkDevice device, const VkPipelineIndirectDeviceAddressInfoNV* pInfo,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits rasterizationSamples,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                        const VkSampleMask* pSampleMask, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                              const VkBool32* pColorBlendEnables, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                                const VkColorBlendEquationEXT* pColorBlendEquations,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                            const VkColorComponentFlags* pColorWriteMasks,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer, VkTessellationDomainOrigin domainOrigin,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetConservativeRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                           VkConservativeRasterizationModeEXT conservativeRasterizationMode,
                                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer, float extraPrimitiveOverestimationSize,
                                                              const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                                const VkColorBlendAdvancedEXT* pColorBlendAdvanced,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer, VkProvokingVertexModeEXT provokingVertexMode,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer, VkLineRasterizationModeEXT lineRasterizationMode,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                            const VkViewportSwizzleNV* pViewportSwizzles,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer, VkCoverageModulationModeNV coverageModulationMode,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageModulationTableEnable,
                                                          const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer, uint32_t coverageModulationTableCount,
                                                    const float* pCoverageModulationTable,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                             VkBool32 representativeFragmentTestEnable,
                                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer, VkCoverageReductionModeNV coverageReductionMode,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                 VkShaderModuleIdentifierEXT* pIdentifier,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                           VkShaderModuleIdentifierEXT* pIdentifier,
                                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkOpticalFlowSessionNV* pSession,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyOpticalFlowSessionNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                const VkAllocationCallbacks* pAllocator,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateBindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                  VkOpticalFlowSessionBindingPointNV bindingPoint, VkImageView view,
                                                  VkImageLayout layout, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer, VkOpticalFlowSessionNV session,
                                            const VkOpticalFlowExecuteInfoNV* pExecuteInfo,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateAntiLagUpdateAMD(VkDevice device, const VkAntiLagDataAMD* pData, const ErrorObject& error_obj) const override;
bool PreCallValidateCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                     const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator,
                                     const ErrorObject& error_obj) const override;
bool PreCallValidateGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount, const VkShaderStageFlagBits* pStages,
                                      const VkShaderEXT* pShaders, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetDepthClampRangeEXT(VkCommandBuffer commandBuffer, VkDepthClampModeEXT depthClampMode,
                                             const VkDepthClampRangeEXT* pDepthClampRange,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateGetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer, uint32_t* pPropertiesCount,
                                                     VkTilePropertiesQCOM* pProperties,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateGetDynamicRenderingTilePropertiesQCOM(VkDevice device, const VkRenderingInfo* pRenderingInfo,
                                                          VkTilePropertiesQCOM* pProperties,
                                                          const ErrorObject& error_obj) const override;
bool PreCallValidateConvertCooperativeVectorMatrixNV(VkDevice device, const VkConvertCooperativeVectorMatrixInfoNV* pInfo,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCmdConvertCooperativeVectorMatrixNV(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                        const VkConvertCooperativeVectorMatrixInfoNV* pInfos,
                                                        const ErrorObject& error_obj) const override;
bool PreCallValidateSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepModeInfoNV* pSleepModeInfo,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo,
                                   const ErrorObject& error_obj) const override;
bool PreCallValidateSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain,
                                       const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo,
                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain, VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateQueueNotifyOutOfBandNV(VkQueue queue, const VkOutOfBandQueueTypeInfoNV* pQueueTypeInfo,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask,
                                                          const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_SCREEN_QNX
bool PreCallValidateGetScreenBufferPropertiesQNX(VkDevice device, const struct _screen_buffer* buffer,
                                                 VkScreenBufferPropertiesQNX* pProperties,
                                                 const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_SCREEN_QNX
bool PreCallValidateGetClusterAccelerationStructureBuildSizesNV(VkDevice device,
                                                                const VkClusterAccelerationStructureInputInfoNV* pInfo,
                                                                VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo,
                                                                const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBuildClusterAccelerationStructureIndirectNV(
    VkCommandBuffer commandBuffer, const VkClusterAccelerationStructureCommandsInfoNV* pCommandInfos,
    const ErrorObject& error_obj) const override;
bool PreCallValidateGetPartitionedAccelerationStructuresBuildSizesNV(
    VkDevice device, const VkPartitionedAccelerationStructureInstancesInputNV* pInfo,
    VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBuildPartitionedAccelerationStructuresNV(VkCommandBuffer commandBuffer,
                                                                const VkBuildPartitionedAccelerationStructureInfoNV* pBuildInfo,
                                                                const ErrorObject& error_obj) const override;
bool PreCallValidateGetGeneratedCommandsMemoryRequirementsEXT(VkDevice device,
                                                              const VkGeneratedCommandsMemoryRequirementsInfoEXT* pInfo,
                                                              VkMemoryRequirements2* pMemoryRequirements,
                                                              const ErrorObject& error_obj) const override;
bool PreCallValidateCmdPreprocessGeneratedCommandsEXT(VkCommandBuffer commandBuffer,
                                                      const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo,
                                                      VkCommandBuffer stateCommandBuffer,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdExecuteGeneratedCommandsEXT(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                   const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCreateIndirectCommandsLayoutEXT(VkDevice device, const VkIndirectCommandsLayoutCreateInfoEXT* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    VkIndirectCommandsLayoutEXT* pIndirectCommandsLayout,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyIndirectCommandsLayoutEXT(VkDevice device, VkIndirectCommandsLayoutEXT indirectCommandsLayout,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateCreateIndirectExecutionSetEXT(VkDevice device, const VkIndirectExecutionSetCreateInfoEXT* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  VkIndirectExecutionSetEXT* pIndirectExecutionSet,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyIndirectExecutionSetEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateUpdateIndirectExecutionSetPipelineEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                          uint32_t executionSetWriteCount,
                                                          const VkWriteIndirectExecutionSetPipelineEXT* pExecutionSetWrites,
                                                          const ErrorObject& error_obj) const override;
bool PreCallValidateUpdateIndirectExecutionSetShaderEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                        uint32_t executionSetWriteCount,
                                                        const VkWriteIndirectExecutionSetShaderEXT* pExecutionSetWrites,
                                                        const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_METAL_EXT
bool PreCallValidateGetMemoryMetalHandleEXT(VkDevice device, const VkMemoryGetMetalHandleInfoEXT* pGetMetalHandleInfo,
                                            void** pHandle, const ErrorObject& error_obj) const override;
bool PreCallValidateGetMemoryMetalHandlePropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                      const void* pHandle,
                                                      VkMemoryMetalHandlePropertiesEXT* pMemoryMetalHandleProperties,
                                                      const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_METAL_EXT
bool PreCallValidateCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   VkAccelerationStructureKHR* pAccelerationStructure,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBuildAccelerationStructuresKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                      const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                      const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                              const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                              const VkDeviceAddress* pIndirectDeviceAddresses,
                                                              const uint32_t* pIndirectStrides,
                                                              const uint32_t* const* ppMaxPrimitiveCounts,
                                                              const ErrorObject& error_obj) const override;
bool PreCallValidateBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                                   const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                   const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                 const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                         const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                         const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                         const ErrorObject& error_obj) const override;
bool PreCallValidateWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                             const VkAccelerationStructureKHR* pAccelerationStructures,
                                                             VkQueryType queryType, size_t dataSize, void* pData, size_t stride,
                                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                            const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                            const ErrorObject& error_obj) const override;
bool PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                            const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                            const ErrorObject& error_obj) const override;
bool PreCallValidateGetAccelerationStructureDeviceAddressKHR(VkDevice device,
                                                             const VkAccelerationStructureDeviceAddressInfoKHR* pInfo,
                                                             const ErrorObject& error_obj) const override;
bool PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount,
                                                                const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery,
                                                                const ErrorObject& error_obj) const override;
bool PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(VkDevice device,
                                                                   const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
                                                                   VkAccelerationStructureCompatibilityKHR* pCompatibility,
                                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateGetAccelerationStructureBuildSizesKHR(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                          const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
                                                          const uint32_t* pMaxPrimitiveCounts,
                                                          VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo,
                                                          const ErrorObject& error_obj) const override;
bool PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                    const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                    const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                    const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                    uint32_t height, uint32_t depth, const ErrorObject& error_obj) const override;
bool PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                 VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                 const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                 const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                    uint32_t groupCount, size_t dataSize, void* pData,
                                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                            const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                            const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                            const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                            const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                            VkDeviceAddress indirectDeviceAddress, const ErrorObject& error_obj) const override;
bool PreCallValidateGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group,
                                                         VkShaderGroupShaderKHR groupShader,
                                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize,
                                                         const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                        uint32_t groupCountZ, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                uint32_t drawCount, uint32_t stride, const ErrorObject& error_obj) const override;
bool PreCallValidateCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride, const ErrorObject& error_obj) const override;

bool ValidatePipelineViewportStateCreateInfo(const Context& context, const VkPipelineViewportStateCreateInfo& info,
                                             const Location& loc) const;
bool ValidatePipelineTessellationStateCreateInfo(const Context& context, const VkPipelineTessellationStateCreateInfo& info,
                                                 const Location& loc) const;
bool ValidatePipelineVertexInputStateCreateInfo(const Context& context, const VkPipelineVertexInputStateCreateInfo& info,
                                                const Location& loc) const;
bool ValidatePipelineMultisampleStateCreateInfo(const Context& context, const VkPipelineMultisampleStateCreateInfo& info,
                                                const Location& loc) const;
bool ValidatePipelineColorBlendStateCreateInfo(const Context& context, const VkPipelineColorBlendStateCreateInfo& info,
                                               const Location& loc) const;
bool ValidatePipelineDepthStencilStateCreateInfo(const Context& context, const VkPipelineDepthStencilStateCreateInfo& info,
                                                 const Location& loc) const;
bool ValidatePipelineInputAssemblyStateCreateInfo(const Context& context, const VkPipelineInputAssemblyStateCreateInfo& info,
                                                  const Location& loc) const;
bool ValidatePipelineRasterizationStateCreateInfo(const Context& context, const VkPipelineRasterizationStateCreateInfo& info,
                                                  const Location& loc) const;
bool ValidatePipelineShaderStageCreateInfo(const Context& context, const VkPipelineShaderStageCreateInfo& info,
                                           const Location& loc) const;
bool ValidateCommandBufferInheritanceInfo(const Context& context, const VkCommandBufferInheritanceInfo& info,
                                          const Location& loc) const;
bool ValidateDescriptorAddressInfoEXT(const Context& context, const VkDescriptorAddressInfoEXT& info, const Location& loc) const;
bool ValidateAccelerationStructureGeometryTrianglesDataKHR(const Context& context,
                                                           const VkAccelerationStructureGeometryTrianglesDataKHR& info,
                                                           const Location& loc) const;
bool ValidateAccelerationStructureGeometryInstancesDataKHR(const Context& context,
                                                           const VkAccelerationStructureGeometryInstancesDataKHR& info,
                                                           const Location& loc) const;
bool ValidateAccelerationStructureGeometryAabbsDataKHR(const Context& context,
                                                       const VkAccelerationStructureGeometryAabbsDataKHR& info,
                                                       const Location& loc) const;
bool ValidateIndirectExecutionSetPipelineInfoEXT(const Context& context, const VkIndirectExecutionSetPipelineInfoEXT& info,
                                                 const Location& loc) const;
// NOLINTEND
