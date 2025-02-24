// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See best_practices_generator.py for modifications

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
void PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                               const RecordObject& record_obj) override;

void PostCallRecordQueueWaitIdle(VkQueue queue, const RecordObject& record_obj) override;

void PostCallRecordDeviceWaitIdle(VkDevice device, const RecordObject& record_obj) override;

void PostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                  const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                  const RecordObject& record_obj) override;

void PostCallRecordMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags,
                             void** ppData, const RecordObject& record_obj) override;

void PostCallRecordFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges,
                                           const RecordObject& record_obj) override;

void PostCallRecordInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                const VkMappedMemoryRange* pMemoryRanges, const RecordObject& record_obj) override;

void PostCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                    const RecordObject& record_obj) override;

void PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                   const RecordObject& record_obj) override;

void PostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence,
                                   const RecordObject& record_obj) override;

void PostCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                               VkFence* pFence, const RecordObject& record_obj) override;

void PostCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences,
                               const RecordObject& record_obj) override;

void PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, const RecordObject& record_obj) override;

void PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout,
                                 const RecordObject& record_obj) override;

void PostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore,
                                   const RecordObject& record_obj) override;

void PostCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                               VkEvent* pEvent, const RecordObject& record_obj) override;

void PostCallRecordGetEventStatus(VkDevice device, VkEvent event, const RecordObject& record_obj) override;

void PostCallRecordSetEvent(VkDevice device, VkEvent event, const RecordObject& record_obj) override;

void PostCallRecordResetEvent(VkDevice device, VkEvent event, const RecordObject& record_obj) override;

void PostCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool,
                                   const RecordObject& record_obj) override;

void PostCallRecordGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                       size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags,
                                       const RecordObject& record_obj) override;

void PostCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                VkBuffer* pBuffer, const RecordObject& record_obj) override;

void PostCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkBufferView* pView,
                                    const RecordObject& record_obj) override;

void PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                               VkImage* pImage, const RecordObject& record_obj) override;

void PostCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkImageView* pView,
                                   const RecordObject& record_obj) override;

void PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                      const RecordObject& record_obj, chassis::CreateShaderModule& chassis_state) override;

void PostCallRecordCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache,
                                       const RecordObject& record_obj) override;

void PostCallRecordGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData,
                                        const RecordObject& record_obj) override;

void PostCallRecordMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                       const VkPipelineCache* pSrcCaches, const RecordObject& record_obj) override;

void PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                           const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                           const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                           const RecordObject& record_obj, PipelineStates& pipeline_states,
                                           chassis::CreateGraphicsPipelines& chassis_state) override;

void PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                          const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                          VkPipeline* pPipelines, const RecordObject& record_obj, PipelineStates& pipeline_states,
                                          chassis::CreateComputePipelines& chassis_state) override;

void PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                        const RecordObject& record_obj) override;

void PostCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                 VkSampler* pSampler, const RecordObject& record_obj) override;

void PostCallRecordCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout,
                                             const RecordObject& record_obj) override;

void PostCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool,
                                        const RecordObject& record_obj) override;

void PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                          VkDescriptorSet* pDescriptorSets, const RecordObject& record_obj,
                                          vvl::AllocateDescriptorSetsData& chassis_state) override;

void PostCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer,
                                     const RecordObject& record_obj) override;

void PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                    const RecordObject& record_obj) override;

void PostCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool,
                                     const RecordObject& record_obj) override;

void PostCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags,
                                    const RecordObject& record_obj) override;

void PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                          VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) override;

void PostCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo,
                                      const RecordObject& record_obj) override;

void PostCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, const RecordObject& record_obj) override;

void PostCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags,
                                      const RecordObject& record_obj) override;

void PostCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                     const RecordObject& record_obj) override;

void PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                    const RecordObject& record_obj) override;

void PostCallRecordCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion,
                                                const RecordObject& record_obj) override;

void PostCallRecordCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                  const RecordObject& record_obj) override;

void PostCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                     const RecordObject& record_obj) override;

void PostCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                            const RecordObject& record_obj) override;

void PostCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                  const RecordObject& record_obj) override;

void PostCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                   const RecordObject& record_obj) override;

void PostCallRecordCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot,
                                         const RecordObject& record_obj) override;

void PostCallRecordSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                  VkPrivateDataSlot privateDataSlot, uint64_t data, const RecordObject& record_obj) override;

void PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                const RecordObject& record_obj) override;

void PostCallRecordMapMemory2(VkDevice device, const VkMemoryMapInfo* pMemoryMapInfo, void** ppData,
                              const RecordObject& record_obj) override;

void PostCallRecordUnmapMemory2(VkDevice device, const VkMemoryUnmapInfo* pMemoryUnmapInfo,
                                const RecordObject& record_obj) override;

void PostCallRecordCopyMemoryToImage(VkDevice device, const VkCopyMemoryToImageInfo* pCopyMemoryToImageInfo,
                                     const RecordObject& record_obj) override;

void PostCallRecordCopyImageToMemory(VkDevice device, const VkCopyImageToMemoryInfo* pCopyImageToMemoryInfo,
                                     const RecordObject& record_obj) override;

void PostCallRecordCopyImageToImage(VkDevice device, const VkCopyImageToImageInfo* pCopyImageToImageInfo,
                                    const RecordObject& record_obj) override;

void PostCallRecordTransitionImageLayout(VkDevice device, uint32_t transitionCount,
                                         const VkHostImageLayoutTransitionInfo* pTransitions,
                                         const RecordObject& record_obj) override;

void PostCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain,
                                      const RecordObject& record_obj) override;

void PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                         VkImage* pSwapchainImages, const RecordObject& record_obj) override;

void PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                       VkFence fence, uint32_t* pImageIndex, const RecordObject& record_obj) override;

void PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, const RecordObject& record_obj) override;

void PostCallRecordGetDeviceGroupPresentCapabilitiesKHR(VkDevice device,
                                                        VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities,
                                                        const RecordObject& record_obj) override;

void PostCallRecordGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                        VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                        const RecordObject& record_obj) override;

void PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex,
                                        const RecordObject& record_obj) override;

void PostCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos,
                                             const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains,
                                             const RecordObject& record_obj) override;

void PostCallRecordCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession,
                                         const RecordObject& record_obj) override;

void PostCallRecordGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                        uint32_t* pMemoryRequirementsCount,
                                                        VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements,
                                                        const RecordObject& record_obj) override;

void PostCallRecordBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession, uint32_t bindSessionMemoryInfoCount,
                                             const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos,
                                             const RecordObject& record_obj) override;

void PostCallRecordCreateVideoSessionParametersKHR(VkDevice device, const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   VkVideoSessionParametersKHR* pVideoSessionParameters,
                                                   const RecordObject& record_obj) override;

void PostCallRecordUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                   const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo,
                                                   const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_WIN32_KHR
void PostCallRecordGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                           HANDLE* pHandle, const RecordObject& record_obj) override;

void PostCallRecordGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, HANDLE handle,
                                                     VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties,
                                                     const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WIN32_KHR
void PostCallRecordGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd,
                                  const RecordObject& record_obj) override;

void PostCallRecordGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                            VkMemoryFdPropertiesKHR* pMemoryFdProperties, const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_WIN32_KHR
void PostCallRecordImportSemaphoreWin32HandleKHR(VkDevice device,
                                                 const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo,
                                                 const RecordObject& record_obj) override;

void PostCallRecordGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                              HANDLE* pHandle, const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WIN32_KHR
void PostCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo,
                                        const RecordObject& record_obj) override;

void PostCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd,
                                     const RecordObject& record_obj) override;

void PostCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                     const RecordObject& record_obj) override;

void PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                        const RecordObject& record_obj) override;

void PostCallRecordGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain, const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_WIN32_KHR
void PostCallRecordImportFenceWin32HandleKHR(VkDevice device, const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo,
                                             const RecordObject& record_obj) override;

void PostCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle,
                                          const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WIN32_KHR
void PostCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo,
                                    const RecordObject& record_obj) override;

void PostCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd,
                                 const RecordObject& record_obj) override;

void PostCallRecordAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo,
                                           const RecordObject& record_obj) override;

void PostCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   VkSamplerYcbcrConversion* pYcbcrConversion,
                                                   const RecordObject& record_obj) override;

void PostCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                        const RecordObject& record_obj) override;

void PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                       const RecordObject& record_obj) override;

void PostCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                               const RecordObject& record_obj) override;

void PostCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                     const RecordObject& record_obj) override;

void PostCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                      const RecordObject& record_obj) override;

void PostCallRecordWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout,
                                     const RecordObject& record_obj) override;

void PostCallRecordCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                              VkDeferredOperationKHR* pDeferredOperation, const RecordObject& record_obj) override;

void PostCallRecordGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                 const RecordObject& record_obj) override;

void PostCallRecordDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation,
                                            const RecordObject& record_obj) override;

void PostCallRecordGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                      uint32_t* pExecutableCount, VkPipelineExecutablePropertiesKHR* pProperties,
                                                      const RecordObject& record_obj) override;

void PostCallRecordGetPipelineExecutableStatisticsKHR(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                      uint32_t* pStatisticCount, VkPipelineExecutableStatisticKHR* pStatistics,
                                                      const RecordObject& record_obj) override;

void PostCallRecordGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations, const RecordObject& record_obj) override;

void PostCallRecordMapMemory2KHR(VkDevice device, const VkMemoryMapInfo* pMemoryMapInfo, void** ppData,
                                 const RecordObject& record_obj) override;

void PostCallRecordUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfo* pMemoryUnmapInfo,
                                   const RecordObject& record_obj) override;

void PostCallRecordGetEncodedVideoSessionParametersKHR(VkDevice device,
                                                       const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
                                                       VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo,
                                                       size_t* pDataSize, void* pData, const RecordObject& record_obj) override;

void PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                   const RecordObject& record_obj) override;

void PostCallRecordCreatePipelineBinariesKHR(VkDevice device, const VkPipelineBinaryCreateInfoKHR* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkPipelineBinaryHandlesInfoKHR* pBinaries,
                                             const RecordObject& record_obj) override;

void PostCallRecordGetPipelineKeyKHR(VkDevice device, const VkPipelineCreateInfoKHR* pPipelineCreateInfo,
                                     VkPipelineBinaryKeyKHR* pPipelineKey, const RecordObject& record_obj) override;

void PostCallRecordGetPipelineBinaryDataKHR(VkDevice device, const VkPipelineBinaryDataInfoKHR* pInfo,
                                            VkPipelineBinaryKeyKHR* pPipelineBinaryKey, size_t* pPipelineBinaryDataSize,
                                            void* pPipelineBinaryData, const RecordObject& record_obj) override;

void PostCallRecordGetCalibratedTimestampsKHR(VkDevice device, uint32_t timestampCount,
                                              const VkCalibratedTimestampInfoKHR* pTimestampInfos, uint64_t* pTimestamps,
                                              uint64_t* pMaxDeviation, const RecordObject& record_obj) override;

void PostCallRecordDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT* pTagInfo,
                                              const RecordObject& record_obj) override;

void PostCallRecordDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT* pNameInfo,
                                               const RecordObject& record_obj) override;

void PostCallRecordCreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule,
                                     const RecordObject& record_obj) override;

void PostCallRecordCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction,
                                       const RecordObject& record_obj) override;

void PostCallRecordGetImageViewAddressNVX(VkDevice device, VkImageView imageView, VkImageViewAddressPropertiesNVX* pProperties,
                                          const RecordObject& record_obj) override;

void PostCallRecordGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                    VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo,
                                    const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_WIN32_KHR
void PostCallRecordGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory, VkExternalMemoryHandleTypeFlagsNV handleType,
                                          HANDLE* pHandle, const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WIN32_KHR
void PostCallRecordDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display, const VkDisplayPowerInfoEXT* pDisplayPowerInfo,
                                          const RecordObject& record_obj) override;

void PostCallRecordRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                          const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                          const RecordObject& record_obj) override;

void PostCallRecordRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display, const VkDisplayEventInfoEXT* pDisplayEventInfo,
                                           const VkAllocationCallbacks* pAllocator, VkFence* pFence,
                                           const RecordObject& record_obj) override;

void PostCallRecordGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter,
                                          uint64_t* pCounterValue, const RecordObject& record_obj) override;

void PostCallRecordGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                 VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties,
                                                 const RecordObject& record_obj) override;

void PostCallRecordGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount,
                                                   VkPastPresentationTimingGOOGLE* pPresentationTimings,
                                                   const RecordObject& record_obj) override;

void PostCallRecordSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo,
                                              const RecordObject& record_obj) override;

void PostCallRecordSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo,
                                             const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_ANDROID_KHR
void PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                             VkAndroidHardwareBufferPropertiesANDROID* pProperties,
                                                             const RecordObject& record_obj) override;

void PostCallRecordGetMemoryAndroidHardwareBufferANDROID(VkDevice device, const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                         struct AHardwareBuffer** pBuffer, const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS
void PostCallRecordCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                     const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos,
                                                     const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                     const RecordObject& record_obj) override;

void PostCallRecordGetExecutionGraphPipelineScratchSizeAMDX(VkDevice device, VkPipeline executionGraph,
                                                            VkExecutionGraphPipelineScratchSizeAMDX* pSizeInfo,
                                                            const RecordObject& record_obj) override;

void PostCallRecordGetExecutionGraphPipelineNodeIndexAMDX(VkDevice device, VkPipeline executionGraph,
                                                          const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo,
                                                          uint32_t* pNodeIndex, const RecordObject& record_obj) override;

#endif  // VK_ENABLE_BETA_EXTENSIONS
void PostCallRecordGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image,
                                                          VkImageDrmFormatModifierPropertiesEXT* pProperties,
                                                          const RecordObject& record_obj) override;

void PostCallRecordCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator,
                                                 VkAccelerationStructureNV* pAccelerationStructure,
                                                 const RecordObject& record_obj) override;

void PostCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                     const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                     const RecordObject& record_obj) override;

void PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                               const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                               const RecordObject& record_obj, PipelineStates& pipeline_states,
                                               chassis::CreateRayTracingPipelinesNV& chassis_state) override;

void PostCallRecordGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                      uint32_t groupCount, size_t dataSize, void* pData,
                                                      const RecordObject& record_obj) override;

void PostCallRecordGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount,
                                                     size_t dataSize, void* pData, const RecordObject& record_obj) override;

void PostCallRecordGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                    size_t dataSize, void* pData, const RecordObject& record_obj) override;

void PostCallRecordCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader,
                                     const RecordObject& record_obj) override;

void PostCallRecordGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                     const void* pHostPointer,
                                                     VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties,
                                                     const RecordObject& record_obj) override;

void PostCallRecordGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                              const VkCalibratedTimestampInfoKHR* pTimestampInfos, uint64_t* pTimestamps,
                                              uint64_t* pMaxDeviation, const RecordObject& record_obj) override;

void PostCallRecordInitializePerformanceApiINTEL(VkDevice device, const VkInitializePerformanceApiInfoINTEL* pInitializeInfo,
                                                 const RecordObject& record_obj) override;

void PostCallRecordCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceMarkerInfoINTEL* pMarkerInfo,
                                                const RecordObject& record_obj) override;

void PostCallRecordCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                      const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo,
                                                      const RecordObject& record_obj) override;

void PostCallRecordCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                  const VkPerformanceOverrideInfoINTEL* pOverrideInfo,
                                                  const RecordObject& record_obj) override;

void PostCallRecordAcquirePerformanceConfigurationINTEL(VkDevice device,
                                                        const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
                                                        VkPerformanceConfigurationINTEL* pConfiguration,
                                                        const RecordObject& record_obj) override;

void PostCallRecordReleasePerformanceConfigurationINTEL(VkDevice device, VkPerformanceConfigurationINTEL configuration,
                                                        const RecordObject& record_obj) override;

void PostCallRecordQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration,
                                                         const RecordObject& record_obj) override;

void PostCallRecordGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter,
                                                VkPerformanceValueINTEL* pValue, const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_WIN32_KHR
void PostCallRecordAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                     const RecordObject& record_obj) override;

void PostCallRecordReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                     const RecordObject& record_obj) override;

void PostCallRecordGetDeviceGroupSurfacePresentModes2EXT(VkDevice device, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                         VkDeviceGroupPresentModeFlagsKHR* pModes,
                                                         const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WIN32_KHR
void PostCallRecordCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfo* pCopyMemoryToImageInfo,
                                        const RecordObject& record_obj) override;

void PostCallRecordCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfo* pCopyImageToMemoryInfo,
                                        const RecordObject& record_obj) override;

void PostCallRecordCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfo* pCopyImageToImageInfo,
                                       const RecordObject& record_obj) override;

void PostCallRecordTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                            const VkHostImageLayoutTransitionInfo* pTransitions,
                                            const RecordObject& record_obj) override;

void PostCallRecordReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo,
                                             const RecordObject& record_obj) override;

void PostCallRecordCreateIndirectCommandsLayoutNV(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  VkIndirectCommandsLayoutNV* pIndirectCommandsLayout,
                                                  const RecordObject& record_obj) override;

void PostCallRecordCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot,
                                            const RecordObject& record_obj) override;

void PostCallRecordSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                     VkPrivateDataSlot privateDataSlot, uint64_t data, const RecordObject& record_obj) override;

void PostCallRecordCreateCudaModuleNV(VkDevice device, const VkCudaModuleCreateInfoNV* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkCudaModuleNV* pModule,
                                      const RecordObject& record_obj) override;

void PostCallRecordGetCudaModuleCacheNV(VkDevice device, VkCudaModuleNV module, size_t* pCacheSize, void* pCacheData,
                                        const RecordObject& record_obj) override;

void PostCallRecordCreateCudaFunctionNV(VkDevice device, const VkCudaFunctionCreateInfoNV* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkCudaFunctionNV* pFunction,
                                        const RecordObject& record_obj) override;

void PostCallRecordGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
                                                           void* pData, const RecordObject& record_obj) override;

void PostCallRecordGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageCaptureDescriptorDataInfoEXT* pInfo,
                                                          void* pData, const RecordObject& record_obj) override;

void PostCallRecordGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
                                                              void* pData, const RecordObject& record_obj) override;

void PostCallRecordGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
                                                            void* pData, const RecordObject& record_obj) override;

void PostCallRecordGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo, void* pData,
    const RecordObject& record_obj) override;

void PostCallRecordGetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT* pFaultCounts, VkDeviceFaultInfoEXT* pFaultInfo,
                                         const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_FUCHSIA
void PostCallRecordGetMemoryZirconHandleFUCHSIA(VkDevice device, const VkMemoryGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                zx_handle_t* pZirconHandle, const RecordObject& record_obj) override;

void PostCallRecordGetMemoryZirconHandlePropertiesFUCHSIA(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                          zx_handle_t zirconHandle,
                                                          VkMemoryZirconHandlePropertiesFUCHSIA* pMemoryZirconHandleProperties,
                                                          const RecordObject& record_obj) override;

void PostCallRecordImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo,
    const RecordObject& record_obj) override;

void PostCallRecordGetSemaphoreZirconHandleFUCHSIA(VkDevice device,
                                                   const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                   zx_handle_t* pZirconHandle, const RecordObject& record_obj) override;

void PostCallRecordCreateBufferCollectionFUCHSIA(VkDevice device, const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkBufferCollectionFUCHSIA* pCollection,
                                                 const RecordObject& record_obj) override;

void PostCallRecordSetBufferCollectionImageConstraintsFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                              const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo,
                                                              const RecordObject& record_obj) override;

void PostCallRecordSetBufferCollectionBufferConstraintsFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                               const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo,
                                                               const RecordObject& record_obj) override;

void PostCallRecordGetBufferCollectionPropertiesFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                        VkBufferCollectionPropertiesFUCHSIA* pProperties,
                                                        const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_FUCHSIA
void PostCallRecordGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass,
                                                                 VkExtent2D* pMaxWorkgroupSize,
                                                                 const RecordObject& record_obj) override;

void PostCallRecordGetMemoryRemoteAddressNV(VkDevice device, const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo,
                                            VkRemoteAddressNV* pAddress, const RecordObject& record_obj) override;

void PostCallRecordGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT* pPipelineInfo,
                                            VkBaseOutStructure* pPipelineProperties, const RecordObject& record_obj) override;

void PostCallRecordCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap,
                                     const RecordObject& record_obj) override;

void PostCallRecordBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                     const VkMicromapBuildInfoEXT* pInfos, const RecordObject& record_obj) override;

void PostCallRecordCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMicromapInfoEXT* pInfo,
                                   const RecordObject& record_obj) override;

void PostCallRecordCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                           const VkCopyMicromapToMemoryInfoEXT* pInfo, const RecordObject& record_obj) override;

void PostCallRecordCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                           const VkCopyMemoryToMicromapInfoEXT* pInfo, const RecordObject& record_obj) override;

void PostCallRecordWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount, const VkMicromapEXT* pMicromaps,
                                               VkQueryType queryType, size_t dataSize, void* pData, size_t stride,
                                               const RecordObject& record_obj) override;

void PostCallRecordCreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkOpticalFlowSessionNV* pSession,
                                              const RecordObject& record_obj) override;

void PostCallRecordBindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                 VkOpticalFlowSessionBindingPointNV bindingPoint, VkImageView view,
                                                 VkImageLayout layout, const RecordObject& record_obj) override;

void PostCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                    const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders, const RecordObject& record_obj,
                                    chassis::ShaderObject& chassis_state) override;

void PostCallRecordGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData,
                                          const RecordObject& record_obj) override;

void PostCallRecordGetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer, uint32_t* pPropertiesCount,
                                                    VkTilePropertiesQCOM* pProperties, const RecordObject& record_obj) override;

void PostCallRecordConvertCooperativeVectorMatrixNV(VkDevice device, const VkConvertCooperativeVectorMatrixInfoNV* pInfo,
                                                    const RecordObject& record_obj) override;

void PostCallRecordSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepModeInfoNV* pSleepModeInfo,
                                         const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_SCREEN_QNX
void PostCallRecordGetScreenBufferPropertiesQNX(VkDevice device, const struct _screen_buffer* buffer,
                                                VkScreenBufferPropertiesQNX* pProperties, const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_SCREEN_QNX
void PostCallRecordCreateIndirectCommandsLayoutEXT(VkDevice device, const VkIndirectCommandsLayoutCreateInfoEXT* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator,
                                                   VkIndirectCommandsLayoutEXT* pIndirectCommandsLayout,
                                                   const RecordObject& record_obj) override;

void PostCallRecordCreateIndirectExecutionSetEXT(VkDevice device, const VkIndirectExecutionSetCreateInfoEXT* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator,
                                                 VkIndirectExecutionSetEXT* pIndirectExecutionSet,
                                                 const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_METAL_EXT
void PostCallRecordGetMemoryMetalHandleEXT(VkDevice device, const VkMemoryGetMetalHandleInfoEXT* pGetMetalHandleInfo,
                                           void** pHandle, const RecordObject& record_obj) override;

void PostCallRecordGetMemoryMetalHandlePropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                     const void* pHandle,
                                                     VkMemoryMetalHandlePropertiesEXT* pMemoryMetalHandleProperties,
                                                     const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_METAL_EXT
void PostCallRecordCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  VkAccelerationStructureKHR* pAccelerationStructure,
                                                  const RecordObject& record_obj) override;

void PostCallRecordBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                                  const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                  const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos,
                                                  const RecordObject& record_obj) override;

void PostCallRecordCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                const RecordObject& record_obj) override;

void PostCallRecordCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                        const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                        const RecordObject& record_obj) override;

void PostCallRecordCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                        const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                        const RecordObject& record_obj) override;

void PostCallRecordWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                            const VkAccelerationStructureKHR* pAccelerationStructures,
                                                            VkQueryType queryType, size_t dataSize, void* pData, size_t stride,
                                                            const RecordObject& record_obj) override;

void PostCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                const RecordObject& record_obj, PipelineStates& pipeline_states,
                                                std::shared_ptr<chassis::CreateRayTracingPipelinesKHR> chassis_state) override;

void PostCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                   uint32_t groupCount, size_t dataSize, void* pData,
                                                                   const RecordObject& record_obj) override;

// NOLINTEND
