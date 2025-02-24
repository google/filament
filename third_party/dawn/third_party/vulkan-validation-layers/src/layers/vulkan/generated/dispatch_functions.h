// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See dispatch_object_generator.py for modifications

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

// This file contains contains convience functions for non-chassis code that needs to
// make vulkan calls.

#pragma once

#include "chassis/dispatch_object.h"

static inline void DispatchDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(instance);
    dispatch->DestroyInstance(instance, pAllocator);
}

static inline VkResult DispatchEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                                        VkPhysicalDevice* pPhysicalDevices) {
    auto dispatch = vvl::dispatch::GetData(instance);
    return dispatch->EnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
}

static inline void DispatchGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceFeatures(physicalDevice, pFeatures);
}

static inline void DispatchGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                             VkFormatProperties* pFormatProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
}

static inline VkResult DispatchGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                      VkImageType type, VkImageTiling tiling,
                                                                      VkImageUsageFlags usage, VkImageCreateFlags flags,
                                                                      VkImageFormatProperties* pImageFormatProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags,
                                                            pImageFormatProperties);
}

static inline void DispatchGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceProperties(physicalDevice, pProperties);
}

static inline void DispatchGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                  uint32_t* pQueueFamilyPropertyCount,
                                                                  VkQueueFamilyProperties* pQueueFamilyProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

static inline void DispatchGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                                             VkPhysicalDeviceMemoryProperties* pMemoryProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
}

static inline PFN_vkVoidFunction DispatchGetInstanceProcAddr(VkInstance instance, const char* pName) {
    auto dispatch = vvl::dispatch::GetData(instance);
    return dispatch->GetInstanceProcAddr(instance, pName);
}

static inline PFN_vkVoidFunction DispatchGetDeviceProcAddr(VkDevice device, const char* pName) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetDeviceProcAddr(device, pName);
}

static inline VkResult DispatchCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->CreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
}

static inline void DispatchDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyDevice(device, pAllocator);
}

static inline VkResult DispatchEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName,
                                                                  uint32_t* pPropertyCount, VkExtensionProperties* pProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
}

static inline VkResult DispatchEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                              VkLayerProperties* pProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->EnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties);
}

static inline void DispatchGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
}

static inline VkResult DispatchQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) {
    auto dispatch = vvl::dispatch::GetData(queue);
    return dispatch->QueueSubmit(queue, submitCount, pSubmits, fence);
}

static inline VkResult DispatchQueueWaitIdle(VkQueue queue) {
    auto dispatch = vvl::dispatch::GetData(queue);
    return dispatch->QueueWaitIdle(queue);
}

static inline VkResult DispatchDeviceWaitIdle(VkDevice device) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->DeviceWaitIdle(device);
}

static inline VkResult DispatchAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->AllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
}

static inline void DispatchFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->FreeMemory(device, memory, pAllocator);
}

static inline VkResult DispatchMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                                         VkMemoryMapFlags flags, void** ppData) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->MapMemory(device, memory, offset, size, flags, ppData);
}

static inline void DispatchUnmapMemory(VkDevice device, VkDeviceMemory memory) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->UnmapMemory(device, memory);
}

static inline VkResult DispatchFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                       const VkMappedMemoryRange* pMemoryRanges) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->FlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}

static inline VkResult DispatchInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                            const VkMappedMemoryRange* pMemoryRanges) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->InvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}

static inline void DispatchGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory,
                                                     VkDeviceSize* pCommittedMemoryInBytes) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
}

static inline VkResult DispatchBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory,
                                                VkDeviceSize memoryOffset) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->BindBufferMemory(device, buffer, memory, memoryOffset);
}

static inline VkResult DispatchBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->BindImageMemory(device, image, memory, memoryOffset);
}

static inline void DispatchGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                                       VkMemoryRequirements* pMemoryRequirements) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
}

static inline void DispatchGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetImageMemoryRequirements(device, image, pMemoryRequirements);
}

static inline void DispatchGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount,
                                                            VkSparseImageMemoryRequirements* pSparseMemoryRequirements) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

static inline void DispatchGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                        VkImageType type, VkSampleCountFlagBits samples,
                                                                        VkImageUsageFlags usage, VkImageTiling tiling,
                                                                        uint32_t* pPropertyCount,
                                                                        VkSparseImageFormatProperties* pProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount,
                                                           pProperties);
}

static inline VkResult DispatchQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                               VkFence fence) {
    auto dispatch = vvl::dispatch::GetData(queue);
    return dispatch->QueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
}

static inline VkResult DispatchCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateFence(device, pCreateInfo, pAllocator, pFence);
}

static inline void DispatchDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyFence(device, fence, pAllocator);
}

static inline VkResult DispatchResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->ResetFences(device, fenceCount, pFences);
}

static inline VkResult DispatchGetFenceStatus(VkDevice device, VkFence fence) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetFenceStatus(device, fence);
}

static inline VkResult DispatchWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                             uint64_t timeout) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->WaitForFences(device, fenceCount, pFences, waitAll, timeout);
}

static inline VkResult DispatchCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
}

static inline void DispatchDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroySemaphore(device, semaphore, pAllocator);
}

static inline VkResult DispatchCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkEvent* pEvent) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateEvent(device, pCreateInfo, pAllocator, pEvent);
}

static inline void DispatchDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyEvent(device, event, pAllocator);
}

static inline VkResult DispatchGetEventStatus(VkDevice device, VkEvent event) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetEventStatus(device, event);
}

static inline VkResult DispatchSetEvent(VkDevice device, VkEvent event) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->SetEvent(device, event);
}

static inline VkResult DispatchResetEvent(VkDevice device, VkEvent event) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->ResetEvent(device, event);
}

static inline VkResult DispatchCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
}

static inline void DispatchDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyQueryPool(device, queryPool, pAllocator);
}

static inline VkResult DispatchGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                                   size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
}

static inline VkResult DispatchCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
}

static inline void DispatchDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyBuffer(device, buffer, pAllocator);
}

static inline VkResult DispatchCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkBufferView* pView) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateBufferView(device, pCreateInfo, pAllocator, pView);
}

static inline void DispatchDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyBufferView(device, bufferView, pAllocator);
}

static inline VkResult DispatchCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkImage* pImage) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateImage(device, pCreateInfo, pAllocator, pImage);
}

static inline void DispatchDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyImage(device, image, pAllocator);
}

static inline void DispatchGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource,
                                                     VkSubresourceLayout* pLayout) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetImageSubresourceLayout(device, image, pSubresource, pLayout);
}

static inline VkResult DispatchCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkImageView* pView) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateImageView(device, pCreateInfo, pAllocator, pView);
}

static inline void DispatchDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyImageView(device, imageView, pAllocator);
}

static inline VkResult DispatchCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
}

static inline void DispatchDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                               const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyShaderModule(device, shaderModule, pAllocator);
}

static inline VkResult DispatchCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
}

static inline void DispatchDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                                const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyPipelineCache(device, pipelineCache, pAllocator);
}

static inline VkResult DispatchGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize,
                                                    void* pData) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetPipelineCacheData(device, pipelineCache, pDataSize, pData);
}

static inline VkResult DispatchMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                                   const VkPipelineCache* pSrcCaches) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->MergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
}

static inline VkResult DispatchCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                       const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                       const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

static inline VkResult DispatchCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                      const VkComputePipelineCreateInfo* pCreateInfos,
                                                      const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

static inline void DispatchDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyPipeline(device, pipeline, pAllocator);
}

static inline VkResult DispatchCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
}

static inline void DispatchDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                                 const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyPipelineLayout(device, pipelineLayout, pAllocator);
}

static inline VkResult DispatchCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateSampler(device, pCreateInfo, pAllocator, pSampler);
}

static inline void DispatchDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroySampler(device, sampler, pAllocator);
}

static inline VkResult DispatchCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkDescriptorSetLayout* pSetLayout) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
}

static inline void DispatchDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                      const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
}

static inline VkResult DispatchCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
}

static inline void DispatchDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                 const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyDescriptorPool(device, descriptorPool, pAllocator);
}

static inline VkResult DispatchResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                   VkDescriptorPoolResetFlags flags) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->ResetDescriptorPool(device, descriptorPool, flags);
}

static inline VkResult DispatchAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                                      VkDescriptorSet* pDescriptorSets) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->AllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
}

static inline VkResult DispatchFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                                  const VkDescriptorSet* pDescriptorSets) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->FreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
}

static inline void DispatchUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                                const VkCopyDescriptorSet* pDescriptorCopies) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->UpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
}

static inline VkResult DispatchCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
}

static inline void DispatchDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyFramebuffer(device, framebuffer, pAllocator);
}

static inline VkResult DispatchCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
}

static inline void DispatchDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyRenderPass(device, renderPass, pAllocator);
}

static inline void DispatchGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetRenderAreaGranularity(device, renderPass, pGranularity);
}

static inline VkResult DispatchCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
}

static inline void DispatchDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyCommandPool(device, commandPool, pAllocator);
}

static inline VkResult DispatchResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->ResetCommandPool(device, commandPool, flags);
}

static inline VkResult DispatchAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                                      VkCommandBuffer* pCommandBuffers) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->AllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
}

static inline void DispatchFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                              const VkCommandBuffer* pCommandBuffers) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->FreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
}

static inline VkResult DispatchBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    return dispatch->BeginCommandBuffer(commandBuffer, pBeginInfo);
}

static inline VkResult DispatchEndCommandBuffer(VkCommandBuffer commandBuffer) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    return dispatch->EndCommandBuffer(commandBuffer);
}

static inline VkResult DispatchResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    return dispatch->ResetCommandBuffer(commandBuffer, flags);
}

static inline void DispatchCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                           VkPipeline pipeline) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
}

static inline void DispatchCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                          const VkViewport* pViewports) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
}

static inline void DispatchCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                         const VkRect2D* pScissors) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
}

static inline void DispatchCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetLineWidth(commandBuffer, lineWidth);
}

static inline void DispatchCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                           float depthBiasSlopeFactor) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}

static inline void DispatchCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetBlendConstants(commandBuffer, blendConstants);
}

static inline void DispatchCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
}

static inline void DispatchCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                    uint32_t compareMask) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
}

static inline void DispatchCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
}

static inline void DispatchCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetStencilReference(commandBuffer, faceMask, reference);
}

static inline void DispatchCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                 VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                                 const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                                 const uint32_t* pDynamicOffsets) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets,
                                    dynamicOffsetCount, pDynamicOffsets);
}

static inline void DispatchCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkIndexType indexType) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
}

static inline void DispatchCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
}

static inline void DispatchCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                   uint32_t firstVertex, uint32_t firstInstance) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

static inline void DispatchCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                          uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

static inline void DispatchCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                           uint32_t stride) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

static inline void DispatchCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  uint32_t drawCount, uint32_t stride) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

static inline void DispatchCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                       uint32_t groupCountZ) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

static inline void DispatchCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDispatchIndirect(commandBuffer, buffer, offset);
}

static inline void DispatchCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                         uint32_t regionCount, const VkBufferCopy* pRegions) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}

static inline void DispatchCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageCopy* pRegions) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

static inline void DispatchCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageBlit* pRegions, VkFilter filter) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}

static inline void DispatchCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkBufferImageCopy* pRegions) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}

static inline void DispatchCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}

static inline void DispatchCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                           VkDeviceSize dataSize, const void* pData) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
}

static inline void DispatchCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                         VkDeviceSize size, uint32_t data) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
}

static inline void DispatchCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                              const VkClearColorValue* pColor, uint32_t rangeCount,
                                              const VkImageSubresourceRange* pRanges) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
}

static inline void DispatchCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                     const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                     const VkImageSubresourceRange* pRanges) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}

static inline void DispatchCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                               const VkClearAttachment* pAttachments, uint32_t rectCount,
                                               const VkClearRect* pRects) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}

static inline void DispatchCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                           const VkImageResolve* pRegions) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

static inline void DispatchCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetEvent(commandBuffer, event, stageMask);
}

static inline void DispatchCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdResetEvent(commandBuffer, event, stageMask);
}

static inline void DispatchCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                         VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                         uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                         uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                         uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers,
                            bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

static inline void DispatchCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                              VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                              uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                              uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                              uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers,
                                 bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

static inline void DispatchCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                         VkQueryControlFlags flags) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBeginQuery(commandBuffer, queryPool, query, flags);
}

static inline void DispatchCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdEndQuery(commandBuffer, queryPool, query);
}

static inline void DispatchCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                             uint32_t queryCount) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
}

static inline void DispatchCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                             VkQueryPool queryPool, uint32_t query) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
}

static inline void DispatchCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                   uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                   VkDeviceSize stride, VkQueryResultFlags flags) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
}

static inline void DispatchCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                            uint32_t offset, uint32_t size, const void* pValues) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
}

static inline void DispatchCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                              VkSubpassContents contents) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
}

static inline void DispatchCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdNextSubpass(commandBuffer, contents);
}

static inline void DispatchCmdEndRenderPass(VkCommandBuffer commandBuffer) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdEndRenderPass(commandBuffer);
}

static inline void DispatchCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                              const VkCommandBuffer* pCommandBuffers) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
}

static inline VkResult DispatchBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                                 const VkBindBufferMemoryInfo* pBindInfos) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->BindBufferMemory2(device, bindInfoCount, pBindInfos);
}

static inline VkResult DispatchBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->BindImageMemory2(device, bindInfoCount, pBindInfos);
}

static inline void DispatchGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                            uint32_t remoteDeviceIndex,
                                                            VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}

static inline void DispatchCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetDeviceMask(commandBuffer, deviceMask);
}

static inline void DispatchCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                           uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}

static inline VkResult DispatchEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                             VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) {
    auto dispatch = vvl::dispatch::GetData(instance);
    return dispatch->EnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
}

static inline void DispatchGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                       VkMemoryRequirements2* pMemoryRequirements) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
}

static inline void DispatchGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                        VkMemoryRequirements2* pMemoryRequirements) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
}

static inline void DispatchGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                             uint32_t* pSparseMemoryRequirementCount,
                                                             VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

static inline void DispatchGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
}

static inline void DispatchGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceProperties2(physicalDevice, pProperties);
}

static inline void DispatchGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format,
                                                              VkFormatProperties2* pFormatProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
}

static inline VkResult DispatchGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                       const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                       VkImageFormatProperties2* pImageFormatProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
}

static inline void DispatchGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                   uint32_t* pQueueFamilyPropertyCount,
                                                                   VkQueueFamilyProperties2* pQueueFamilyProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

static inline void DispatchGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice,
                                                              VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
}

static inline void DispatchGetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                         const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
                                                                         uint32_t* pPropertyCount,
                                                                         VkSparseImageFormatProperties2* pProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
}

static inline void DispatchTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->TrimCommandPool(device, commandPool, flags);
}

static inline void DispatchGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetDeviceQueue2(device, pQueueInfo, pQueue);
}

static inline VkResult DispatchCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            VkSamplerYcbcrConversion* pYcbcrConversion) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
}

static inline void DispatchDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                         const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
}

static inline VkResult DispatchCreateDescriptorUpdateTemplate(VkDevice device,
                                                              const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
}

static inline void DispatchDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                           const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
}

static inline void DispatchUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                           VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->UpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
}

static inline void DispatchGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice,
                                                                     const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
                                                                     VkExternalBufferProperties* pExternalBufferProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
}

static inline void DispatchGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice,
                                                                    const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                                    VkExternalFenceProperties* pExternalFenceProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
}

static inline void DispatchGetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
}

static inline void DispatchGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                         VkDescriptorSetLayoutSupport* pSupport) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
}

static inline void DispatchCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                uint32_t stride) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

static inline void DispatchCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

static inline VkResult DispatchCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass);
}

static inline void DispatchCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                               const VkSubpassBeginInfo* pSubpassBeginInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}

static inline void DispatchCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                           const VkSubpassEndInfo* pSubpassEndInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}

static inline void DispatchCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
}

static inline void DispatchResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->ResetQueryPool(device, queryPool, firstQuery, queryCount);
}

static inline VkResult DispatchGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetSemaphoreCounterValue(device, semaphore, pValue);
}

static inline VkResult DispatchWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->WaitSemaphores(device, pWaitInfo, timeout);
}

static inline VkResult DispatchSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->SignalSemaphore(device, pSignalInfo);
}

static inline VkDeviceAddress DispatchGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetBufferDeviceAddress(device, pInfo);
}

static inline uint64_t DispatchGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetBufferOpaqueCaptureAddress(device, pInfo);
}

static inline uint64_t DispatchGetDeviceMemoryOpaqueCaptureAddress(VkDevice device,
                                                                   const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
}

static inline VkResult DispatchGetPhysicalDeviceToolProperties(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                               VkPhysicalDeviceToolProperties* pToolProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceToolProperties(physicalDevice, pToolCount, pToolProperties);
}

static inline VkResult DispatchCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreatePrivateDataSlot(device, pCreateInfo, pAllocator, pPrivateDataSlot);
}

static inline void DispatchDestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                  const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyPrivateDataSlot(device, privateDataSlot, pAllocator);
}

static inline VkResult DispatchSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                              VkPrivateDataSlot privateDataSlot, uint64_t data) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->SetPrivateData(device, objectType, objectHandle, privateDataSlot, data);
}

static inline void DispatchGetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                          VkPrivateDataSlot privateDataSlot, uint64_t* pData) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetPrivateData(device, objectType, objectHandle, privateDataSlot, pData);
}

static inline void DispatchCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetEvent2(commandBuffer, event, pDependencyInfo);
}

static inline void DispatchCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdResetEvent2(commandBuffer, event, stageMask);
}

static inline void DispatchCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                          const VkDependencyInfo* pDependencyInfos) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos);
}

static inline void DispatchCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdPipelineBarrier2(commandBuffer, pDependencyInfo);
}

static inline void DispatchCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                              uint32_t query) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdWriteTimestamp2(commandBuffer, stage, queryPool, query);
}

static inline VkResult DispatchQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence) {
    auto dispatch = vvl::dispatch::GetData(queue);
    return dispatch->QueueSubmit2(queue, submitCount, pSubmits, fence);
}

static inline void DispatchCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdCopyBuffer2(commandBuffer, pCopyBufferInfo);
}

static inline void DispatchCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdCopyImage2(commandBuffer, pCopyImageInfo);
}

static inline void DispatchCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                 const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo);
}

static inline void DispatchCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                 const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo);
}

static inline void DispatchCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBlitImage2(commandBuffer, pBlitImageInfo);
}

static inline void DispatchCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdResolveImage2(commandBuffer, pResolveImageInfo);
}

static inline void DispatchCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBeginRendering(commandBuffer, pRenderingInfo);
}

static inline void DispatchCmdEndRendering(VkCommandBuffer commandBuffer) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdEndRendering(commandBuffer);
}

static inline void DispatchCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetCullMode(commandBuffer, cullMode);
}

static inline void DispatchCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetFrontFace(commandBuffer, frontFace);
}

static inline void DispatchCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetPrimitiveTopology(commandBuffer, primitiveTopology);
}

static inline void DispatchCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                   const VkViewport* pViewports) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetViewportWithCount(commandBuffer, viewportCount, pViewports);
}

static inline void DispatchCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetScissorWithCount(commandBuffer, scissorCount, pScissors);
}

static inline void DispatchCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                 const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                 const VkDeviceSize* pStrides) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
}

static inline void DispatchCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetDepthTestEnable(commandBuffer, depthTestEnable);
}

static inline void DispatchCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetDepthWriteEnable(commandBuffer, depthWriteEnable);
}

static inline void DispatchCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetDepthCompareOp(commandBuffer, depthCompareOp);
}

static inline void DispatchCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable);
}

static inline void DispatchCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetStencilTestEnable(commandBuffer, stencilTestEnable);
}

static inline void DispatchCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                           VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}

static inline void DispatchCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable);
}

static inline void DispatchCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetDepthBiasEnable(commandBuffer, depthBiasEnable);
}

static inline void DispatchCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable);
}

static inline void DispatchGetDeviceBufferMemoryRequirements(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                             VkMemoryRequirements2* pMemoryRequirements) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetDeviceBufferMemoryRequirements(device, pInfo, pMemoryRequirements);
}

static inline void DispatchGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                            VkMemoryRequirements2* pMemoryRequirements) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetDeviceImageMemoryRequirements(device, pInfo, pMemoryRequirements);
}

static inline void DispatchGetDeviceImageSparseMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                                  uint32_t* pSparseMemoryRequirementCount,
                                                                  VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetDeviceImageSparseMemoryRequirements(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

static inline void DispatchCmdSetLineStipple(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                             uint16_t lineStipplePattern) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetLineStipple(commandBuffer, lineStippleFactor, lineStipplePattern);
}

static inline VkResult DispatchMapMemory2(VkDevice device, const VkMemoryMapInfo* pMemoryMapInfo, void** ppData) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->MapMemory2(device, pMemoryMapInfo, ppData);
}

static inline VkResult DispatchUnmapMemory2(VkDevice device, const VkMemoryUnmapInfo* pMemoryUnmapInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->UnmapMemory2(device, pMemoryUnmapInfo);
}

static inline void DispatchCmdBindIndexBuffer2(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               VkDeviceSize size, VkIndexType indexType) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBindIndexBuffer2(commandBuffer, buffer, offset, size, indexType);
}

static inline void DispatchGetRenderingAreaGranularity(VkDevice device, const VkRenderingAreaInfo* pRenderingAreaInfo,
                                                       VkExtent2D* pGranularity) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetRenderingAreaGranularity(device, pRenderingAreaInfo, pGranularity);
}

static inline void DispatchGetDeviceImageSubresourceLayout(VkDevice device, const VkDeviceImageSubresourceInfo* pInfo,
                                                           VkSubresourceLayout2* pLayout) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetDeviceImageSubresourceLayout(device, pInfo, pLayout);
}

static inline void DispatchGetImageSubresourceLayout2(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource,
                                                      VkSubresourceLayout2* pLayout) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetImageSubresourceLayout2(device, image, pSubresource, pLayout);
}

static inline void DispatchCmdPushDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                const VkWriteDescriptorSet* pDescriptorWrites) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdPushDescriptorSet(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
}

static inline void DispatchCmdPushDescriptorSetWithTemplate(VkCommandBuffer commandBuffer,
                                                            VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                            VkPipelineLayout layout, uint32_t set, const void* pData) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdPushDescriptorSetWithTemplate(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
}

static inline void DispatchCmdSetRenderingAttachmentLocations(VkCommandBuffer commandBuffer,
                                                              const VkRenderingAttachmentLocationInfo* pLocationInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetRenderingAttachmentLocations(commandBuffer, pLocationInfo);
}

static inline void DispatchCmdSetRenderingInputAttachmentIndices(
    VkCommandBuffer commandBuffer, const VkRenderingInputAttachmentIndexInfo* pInputAttachmentIndexInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetRenderingInputAttachmentIndices(commandBuffer, pInputAttachmentIndexInfo);
}

static inline void DispatchCmdBindDescriptorSets2(VkCommandBuffer commandBuffer,
                                                  const VkBindDescriptorSetsInfo* pBindDescriptorSetsInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBindDescriptorSets2(commandBuffer, pBindDescriptorSetsInfo);
}

static inline void DispatchCmdPushConstants2(VkCommandBuffer commandBuffer, const VkPushConstantsInfo* pPushConstantsInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdPushConstants2(commandBuffer, pPushConstantsInfo);
}

static inline void DispatchCmdPushDescriptorSet2(VkCommandBuffer commandBuffer,
                                                 const VkPushDescriptorSetInfo* pPushDescriptorSetInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdPushDescriptorSet2(commandBuffer, pPushDescriptorSetInfo);
}

static inline void DispatchCmdPushDescriptorSetWithTemplate2(
    VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo* pPushDescriptorSetWithTemplateInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdPushDescriptorSetWithTemplate2(commandBuffer, pPushDescriptorSetWithTemplateInfo);
}

static inline VkResult DispatchCopyMemoryToImage(VkDevice device, const VkCopyMemoryToImageInfo* pCopyMemoryToImageInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CopyMemoryToImage(device, pCopyMemoryToImageInfo);
}

static inline VkResult DispatchCopyImageToMemory(VkDevice device, const VkCopyImageToMemoryInfo* pCopyImageToMemoryInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CopyImageToMemory(device, pCopyImageToMemoryInfo);
}

static inline VkResult DispatchCopyImageToImage(VkDevice device, const VkCopyImageToImageInfo* pCopyImageToImageInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CopyImageToImage(device, pCopyImageToImageInfo);
}

static inline VkResult DispatchTransitionImageLayout(VkDevice device, uint32_t transitionCount,
                                                     const VkHostImageLayoutTransitionInfo* pTransitions) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->TransitionImageLayout(device, transitionCount, pTransitions);
}

static inline void DispatchDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(instance);
    dispatch->DestroySurfaceKHR(instance, surface, pAllocator);
}

static inline VkResult DispatchGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                  VkSurfaceKHR surface, VkBool32* pSupported) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
}

static inline VkResult DispatchGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                       VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
}

static inline VkResult DispatchGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                  uint32_t* pSurfaceFormatCount,
                                                                  VkSurfaceFormatKHR* pSurfaceFormats) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
}

static inline VkResult DispatchGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                       uint32_t* pPresentModeCount,
                                                                       VkPresentModeKHR* pPresentModes) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
}

static inline VkResult DispatchCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
}

static inline void DispatchDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroySwapchainKHR(device, swapchain, pAllocator);
}

static inline VkResult DispatchGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                                     VkImage* pSwapchainImages) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
}

static inline VkResult DispatchAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                   VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->AcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
}

static inline VkResult DispatchQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) {
    auto dispatch = vvl::dispatch::GetData(queue);
    return dispatch->QueuePresentKHR(queue, pPresentInfo);
}

static inline VkResult DispatchGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
}

static inline VkResult DispatchGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                                    VkDeviceGroupPresentModeFlagsKHR* pModes) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
}

static inline VkResult DispatchGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                     uint32_t* pRectCount, VkRect2D* pRects) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
}

static inline VkResult DispatchAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo,
                                                    uint32_t* pImageIndex) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->AcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
}

static inline VkResult DispatchGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                     VkDisplayPropertiesKHR* pProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
}

static inline VkResult DispatchGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                          VkDisplayPlanePropertiesKHR* pProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
}

static inline VkResult DispatchGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                   uint32_t* pDisplayCount, VkDisplayKHR* pDisplays) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
}

static inline VkResult DispatchGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                           uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
}

static inline VkResult DispatchCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                    const VkDisplayModeCreateInfoKHR* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->CreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
}

static inline VkResult DispatchGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode,
                                                              uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
}

static inline VkResult DispatchCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto dispatch = vvl::dispatch::GetData(instance);
    return dispatch->CreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}

static inline VkResult DispatchCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                         const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                         const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
}
#ifdef VK_USE_PLATFORM_XLIB_KHR

static inline VkResult DispatchCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto dispatch = vvl::dispatch::GetData(instance);
    return dispatch->CreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}

static inline VkBool32 DispatchGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                           uint32_t queueFamilyIndex, Display* dpy,
                                                                           VisualID visualID) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
}
#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR

static inline VkResult DispatchCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto dispatch = vvl::dispatch::GetData(instance);
    return dispatch->CreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}

static inline VkBool32 DispatchGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                          uint32_t queueFamilyIndex, xcb_connection_t* connection,
                                                                          xcb_visualid_t visual_id) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
}
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR

static inline VkResult DispatchCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto dispatch = vvl::dispatch::GetData(instance);
    return dispatch->CreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}

static inline VkBool32 DispatchGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                              uint32_t queueFamilyIndex,
                                                                              struct wl_display* display) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR

static inline VkResult DispatchCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto dispatch = vvl::dispatch::GetData(instance);
    return dispatch->CreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR

static inline VkResult DispatchCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto dispatch = vvl::dispatch::GetData(instance);
    return dispatch->CreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}

static inline VkBool32 DispatchGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                            uint32_t queueFamilyIndex) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

static inline VkResult DispatchGetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                                     const VkVideoProfileInfoKHR* pVideoProfile,
                                                                     VkVideoCapabilitiesKHR* pCapabilities) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceVideoCapabilitiesKHR(physicalDevice, pVideoProfile, pCapabilities);
}

static inline VkResult DispatchGetPhysicalDeviceVideoFormatPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                         const VkPhysicalDeviceVideoFormatInfoKHR* pVideoFormatInfo,
                                                                         uint32_t* pVideoFormatPropertyCount,
                                                                         VkVideoFormatPropertiesKHR* pVideoFormatProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceVideoFormatPropertiesKHR(physicalDevice, pVideoFormatInfo, pVideoFormatPropertyCount,
                                                               pVideoFormatProperties);
}

static inline VkResult DispatchCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateVideoSessionKHR(device, pCreateInfo, pAllocator, pVideoSession);
}

static inline void DispatchDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                  const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyVideoSessionKHR(device, videoSession, pAllocator);
}

static inline VkResult DispatchGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                                    uint32_t* pMemoryRequirementsCount,
                                                                    VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetVideoSessionMemoryRequirementsKHR(device, videoSession, pMemoryRequirementsCount, pMemoryRequirements);
}

static inline VkResult DispatchBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                         uint32_t bindSessionMemoryInfoCount,
                                                         const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->BindVideoSessionMemoryKHR(device, videoSession, bindSessionMemoryInfoCount, pBindSessionMemoryInfos);
}

static inline VkResult DispatchCreateVideoSessionParametersKHR(VkDevice device,
                                                               const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               VkVideoSessionParametersKHR* pVideoSessionParameters) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateVideoSessionParametersKHR(device, pCreateInfo, pAllocator, pVideoSessionParameters);
}

static inline VkResult DispatchUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                               const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->UpdateVideoSessionParametersKHR(device, videoSessionParameters, pUpdateInfo);
}

static inline void DispatchDestroyVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                            const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyVideoSessionParametersKHR(device, videoSessionParameters, pAllocator);
}

static inline void DispatchCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR* pBeginInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBeginVideoCodingKHR(commandBuffer, pBeginInfo);
}

static inline void DispatchCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR* pEndCodingInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdEndVideoCodingKHR(commandBuffer, pEndCodingInfo);
}

static inline void DispatchCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                    const VkVideoCodingControlInfoKHR* pCodingControlInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdControlVideoCodingKHR(commandBuffer, pCodingControlInfo);
}

static inline void DispatchCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDecodeVideoKHR(commandBuffer, pDecodeInfo);
}

static inline void DispatchCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBeginRenderingKHR(commandBuffer, pRenderingInfo);
}

static inline void DispatchCmdEndRenderingKHR(VkCommandBuffer commandBuffer) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdEndRenderingKHR(commandBuffer);
}

static inline void DispatchGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
}

static inline void DispatchGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice,
                                                           VkPhysicalDeviceProperties2* pProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
}

static inline void DispatchGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                 VkFormatProperties2* pFormatProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
}

static inline VkResult DispatchGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                          const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                          VkImageFormatProperties2* pImageFormatProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties);
}

static inline void DispatchGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                      uint32_t* pQueueFamilyPropertyCount,
                                                                      VkQueueFamilyProperties2* pQueueFamilyProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

static inline void DispatchGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                 VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
}

static inline void DispatchGetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount,
    VkSparseImageFormatProperties2* pProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
}

static inline void DispatchGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex,
                                                               uint32_t remoteDeviceIndex,
                                                               VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}

static inline void DispatchCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetDeviceMaskKHR(commandBuffer, deviceMask);
}

static inline void DispatchCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                              uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                              uint32_t groupCountZ) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}

static inline void DispatchTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->TrimCommandPoolKHR(device, commandPool, flags);
}

static inline VkResult DispatchEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                                VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) {
    auto dispatch = vvl::dispatch::GetData(instance);
    return dispatch->EnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
}

static inline void DispatchGetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
    VkExternalBufferProperties* pExternalBufferProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

static inline VkResult DispatchGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                       HANDLE* pHandle) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
}

static inline VkResult DispatchGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                 HANDLE handle,
                                                                 VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

static inline VkResult DispatchGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetMemoryFdKHR(device, pGetFdInfo, pFd);
}

static inline VkResult DispatchGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                                        VkMemoryFdPropertiesKHR* pMemoryFdProperties) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);
}

static inline void DispatchGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

static inline VkResult DispatchImportSemaphoreWin32HandleKHR(
    VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->ImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);
}

static inline VkResult DispatchGetSemaphoreWin32HandleKHR(VkDevice device,
                                                          const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                          HANDLE* pHandle) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

static inline VkResult DispatchImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->ImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
}

static inline VkResult DispatchGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetSemaphoreFdKHR(device, pGetFdInfo, pFd);
}

static inline void DispatchCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                   VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                   const VkWriteDescriptorSet* pDescriptorWrites) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
}

static inline void DispatchCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                               VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                               VkPipelineLayout layout, uint32_t set, const void* pData) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
}

static inline VkResult DispatchCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                                 const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                                 const VkAllocationCallbacks* pAllocator,
                                                                 VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
}

static inline void DispatchDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                              const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
}

static inline void DispatchUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                              VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                              const void* pData) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->UpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData);
}

static inline VkResult DispatchCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass);
}

static inline void DispatchCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                                  const VkSubpassBeginInfo* pSubpassBeginInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}

static inline void DispatchCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                              const VkSubpassEndInfo* pSubpassEndInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}

static inline void DispatchCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
}

static inline VkResult DispatchGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetSwapchainStatusKHR(device, swapchain);
}

static inline void DispatchGetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                       const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                                       VkExternalFenceProperties* pExternalFenceProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

static inline VkResult DispatchImportFenceWin32HandleKHR(VkDevice device,
                                                         const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->ImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo);
}

static inline VkResult DispatchGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                      HANDLE* pHandle) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

static inline VkResult DispatchImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->ImportFenceFdKHR(device, pImportFenceFdInfo);
}

static inline VkResult DispatchGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetFenceFdKHR(device, pGetFdInfo, pFd);
}

static inline VkResult DispatchEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters,
    VkPerformanceCounterDescriptionKHR* pCounterDescriptions) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount,
                                                                                   pCounters, pCounterDescriptions);
}

static inline void DispatchGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
    VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR* pPerformanceQueryCreateInfo, uint32_t* pNumPasses) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
}

static inline VkResult DispatchAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->AcquireProfilingLockKHR(device, pInfo);
}

static inline void DispatchReleaseProfilingLockKHR(VkDevice device) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->ReleaseProfilingLockKHR(device);
}

static inline VkResult DispatchGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                        const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                        VkSurfaceCapabilities2KHR* pSurfaceCapabilities) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
}

static inline VkResult DispatchGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                                   const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                   uint32_t* pSurfaceFormatCount,
                                                                   VkSurfaceFormat2KHR* pSurfaceFormats) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
}

static inline VkResult DispatchGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                      VkDisplayProperties2KHR* pProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
}

static inline VkResult DispatchGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                           uint32_t* pPropertyCount,
                                                                           VkDisplayPlaneProperties2KHR* pProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties);
}

static inline VkResult DispatchGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                            uint32_t* pPropertyCount, VkDisplayModeProperties2KHR* pProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
}

static inline VkResult DispatchGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                               const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                               VkDisplayPlaneCapabilities2KHR* pCapabilities) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
}

static inline void DispatchGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                          VkMemoryRequirements2* pMemoryRequirements) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
}

static inline void DispatchGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
                                                           VkMemoryRequirements2* pMemoryRequirements) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
}

static inline void DispatchGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                                uint32_t* pSparseMemoryRequirementCount,
                                                                VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

static inline VkResult DispatchCreateSamplerYcbcrConversionKHR(VkDevice device,
                                                               const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               VkSamplerYcbcrConversion* pYcbcrConversion) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);
}

static inline void DispatchDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                            const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
}

static inline VkResult DispatchBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                    const VkBindBufferMemoryInfo* pBindInfos) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->BindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
}

static inline VkResult DispatchBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                   const VkBindImageMemoryInfo* pBindInfos) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->BindImageMemory2KHR(device, bindInfoCount, pBindInfos);
}

static inline void DispatchGetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                            VkDescriptorSetLayoutSupport* pSupport) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
}

static inline void DispatchCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                   uint32_t stride) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

static inline void DispatchCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                          uint32_t maxDrawCount, uint32_t stride) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

static inline VkResult DispatchGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetSemaphoreCounterValueKHR(device, semaphore, pValue);
}

static inline VkResult DispatchWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->WaitSemaphoresKHR(device, pWaitInfo, timeout);
}

static inline VkResult DispatchSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->SignalSemaphoreKHR(device, pSignalInfo);
}

static inline VkResult DispatchGetPhysicalDeviceFragmentShadingRatesKHR(
    VkPhysicalDevice physicalDevice, uint32_t* pFragmentShadingRateCount,
    VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceFragmentShadingRatesKHR(physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates);
}

static inline void DispatchCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize,
                                                        const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetFragmentShadingRateKHR(commandBuffer, pFragmentSize, combinerOps);
}

static inline void DispatchCmdSetRenderingAttachmentLocationsKHR(VkCommandBuffer commandBuffer,
                                                                 const VkRenderingAttachmentLocationInfo* pLocationInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetRenderingAttachmentLocationsKHR(commandBuffer, pLocationInfo);
}

static inline void DispatchCmdSetRenderingInputAttachmentIndicesKHR(
    VkCommandBuffer commandBuffer, const VkRenderingInputAttachmentIndexInfo* pInputAttachmentIndexInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetRenderingInputAttachmentIndicesKHR(commandBuffer, pInputAttachmentIndexInfo);
}

static inline VkResult DispatchWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->WaitForPresentKHR(device, swapchain, presentId, timeout);
}

static inline VkDeviceAddress DispatchGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetBufferDeviceAddressKHR(device, pInfo);
}

static inline uint64_t DispatchGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetBufferOpaqueCaptureAddressKHR(device, pInfo);
}

static inline uint64_t DispatchGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device,
                                                                      const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetDeviceMemoryOpaqueCaptureAddressKHR(device, pInfo);
}

static inline VkResult DispatchCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                                          VkDeferredOperationKHR* pDeferredOperation) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateDeferredOperationKHR(device, pAllocator, pDeferredOperation);
}

static inline void DispatchDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation,
                                                       const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyDeferredOperationKHR(device, operation, pAllocator);
}

static inline uint32_t DispatchGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetDeferredOperationMaxConcurrencyKHR(device, operation);
}

static inline VkResult DispatchGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetDeferredOperationResultKHR(device, operation);
}

static inline VkResult DispatchDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->DeferredOperationJoinKHR(device, operation);
}

static inline VkResult DispatchGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo,
                                                                  uint32_t* pExecutableCount,
                                                                  VkPipelineExecutablePropertiesKHR* pProperties) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties);
}

static inline VkResult DispatchGetPipelineExecutableStatisticsKHR(VkDevice device,
                                                                  const VkPipelineExecutableInfoKHR* pExecutableInfo,
                                                                  uint32_t* pStatisticCount,
                                                                  VkPipelineExecutableStatisticKHR* pStatistics) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics);
}

static inline VkResult DispatchGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount,
                                                                     pInternalRepresentations);
}

static inline VkResult DispatchMapMemory2KHR(VkDevice device, const VkMemoryMapInfo* pMemoryMapInfo, void** ppData) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->MapMemory2KHR(device, pMemoryMapInfo, ppData);
}

static inline VkResult DispatchUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfo* pMemoryUnmapInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->UnmapMemory2KHR(device, pMemoryUnmapInfo);
}

static inline VkResult DispatchGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* pQualityLevelInfo,
    VkVideoEncodeQualityLevelPropertiesKHR* pQualityLevelProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(physicalDevice, pQualityLevelInfo,
                                                                           pQualityLevelProperties);
}

static inline VkResult DispatchGetEncodedVideoSessionParametersKHR(
    VkDevice device, const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
    VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo, size_t* pDataSize, void* pData) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetEncodedVideoSessionParametersKHR(device, pVideoSessionParametersInfo, pFeedbackInfo, pDataSize, pData);
}

static inline void DispatchCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdEncodeVideoKHR(commandBuffer, pEncodeInfo);
}

static inline void DispatchCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetEvent2KHR(commandBuffer, event, pDependencyInfo);
}

static inline void DispatchCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdResetEvent2KHR(commandBuffer, event, stageMask);
}

static inline void DispatchCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                             const VkDependencyInfo* pDependencyInfos) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos);
}

static inline void DispatchCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdPipelineBarrier2KHR(commandBuffer, pDependencyInfo);
}

static inline void DispatchCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                                 uint32_t query) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
}

static inline VkResult DispatchQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence) {
    auto dispatch = vvl::dispatch::GetData(queue);
    return dispatch->QueueSubmit2KHR(queue, submitCount, pSubmits, fence);
}

static inline void DispatchCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdCopyBuffer2KHR(commandBuffer, pCopyBufferInfo);
}

static inline void DispatchCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
}

static inline void DispatchCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                    const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
}

static inline void DispatchCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                    const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
}

static inline void DispatchCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
}

static inline void DispatchCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
}

static inline void DispatchCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdTraceRaysIndirect2KHR(commandBuffer, indirectDeviceAddress);
}

static inline void DispatchGetDeviceBufferMemoryRequirementsKHR(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
                                                                VkMemoryRequirements2* pMemoryRequirements) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetDeviceBufferMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
}

static inline void DispatchGetDeviceImageMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                               VkMemoryRequirements2* pMemoryRequirements) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetDeviceImageMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
}

static inline void DispatchGetDeviceImageSparseMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
                                                                     uint32_t* pSparseMemoryRequirementCount,
                                                                     VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetDeviceImageSparseMemoryRequirementsKHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

static inline void DispatchCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkDeviceSize size, VkIndexType indexType) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBindIndexBuffer2KHR(commandBuffer, buffer, offset, size, indexType);
}

static inline void DispatchGetRenderingAreaGranularityKHR(VkDevice device, const VkRenderingAreaInfo* pRenderingAreaInfo,
                                                          VkExtent2D* pGranularity) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetRenderingAreaGranularityKHR(device, pRenderingAreaInfo, pGranularity);
}

static inline void DispatchGetDeviceImageSubresourceLayoutKHR(VkDevice device, const VkDeviceImageSubresourceInfo* pInfo,
                                                              VkSubresourceLayout2* pLayout) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetDeviceImageSubresourceLayoutKHR(device, pInfo, pLayout);
}

static inline void DispatchGetImageSubresourceLayout2KHR(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource,
                                                         VkSubresourceLayout2* pLayout) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetImageSubresourceLayout2KHR(device, image, pSubresource, pLayout);
}

static inline VkResult DispatchCreatePipelineBinariesKHR(VkDevice device, const VkPipelineBinaryCreateInfoKHR* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkPipelineBinaryHandlesInfoKHR* pBinaries) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreatePipelineBinariesKHR(device, pCreateInfo, pAllocator, pBinaries);
}

static inline void DispatchDestroyPipelineBinaryKHR(VkDevice device, VkPipelineBinaryKHR pipelineBinary,
                                                    const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyPipelineBinaryKHR(device, pipelineBinary, pAllocator);
}

static inline VkResult DispatchGetPipelineKeyKHR(VkDevice device, const VkPipelineCreateInfoKHR* pPipelineCreateInfo,
                                                 VkPipelineBinaryKeyKHR* pPipelineKey) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetPipelineKeyKHR(device, pPipelineCreateInfo, pPipelineKey);
}

static inline VkResult DispatchGetPipelineBinaryDataKHR(VkDevice device, const VkPipelineBinaryDataInfoKHR* pInfo,
                                                        VkPipelineBinaryKeyKHR* pPipelineBinaryKey, size_t* pPipelineBinaryDataSize,
                                                        void* pPipelineBinaryData) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetPipelineBinaryDataKHR(device, pInfo, pPipelineBinaryKey, pPipelineBinaryDataSize, pPipelineBinaryData);
}

static inline VkResult DispatchReleaseCapturedPipelineDataKHR(VkDevice device, const VkReleaseCapturedPipelineDataInfoKHR* pInfo,
                                                              const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->ReleaseCapturedPipelineDataKHR(device, pInfo, pAllocator);
}

static inline VkResult DispatchGetPhysicalDeviceCooperativeMatrixPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                               uint32_t* pPropertyCount,
                                                                               VkCooperativeMatrixPropertiesKHR* pProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceCooperativeMatrixPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
}

static inline void DispatchCmdSetLineStippleKHR(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                uint16_t lineStipplePattern) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetLineStippleKHR(commandBuffer, lineStippleFactor, lineStipplePattern);
}

static inline VkResult DispatchGetPhysicalDeviceCalibrateableTimeDomainsKHR(VkPhysicalDevice physicalDevice,
                                                                            uint32_t* pTimeDomainCount,
                                                                            VkTimeDomainKHR* pTimeDomains) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceCalibrateableTimeDomainsKHR(physicalDevice, pTimeDomainCount, pTimeDomains);
}

static inline VkResult DispatchGetCalibratedTimestampsKHR(VkDevice device, uint32_t timestampCount,
                                                          const VkCalibratedTimestampInfoKHR* pTimestampInfos,
                                                          uint64_t* pTimestamps, uint64_t* pMaxDeviation) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetCalibratedTimestampsKHR(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
}

static inline void DispatchCmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer,
                                                     const VkBindDescriptorSetsInfo* pBindDescriptorSetsInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBindDescriptorSets2KHR(commandBuffer, pBindDescriptorSetsInfo);
}

static inline void DispatchCmdPushConstants2KHR(VkCommandBuffer commandBuffer, const VkPushConstantsInfo* pPushConstantsInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdPushConstants2KHR(commandBuffer, pPushConstantsInfo);
}

static inline void DispatchCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer,
                                                    const VkPushDescriptorSetInfo* pPushDescriptorSetInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdPushDescriptorSet2KHR(commandBuffer, pPushDescriptorSetInfo);
}

static inline void DispatchCmdPushDescriptorSetWithTemplate2KHR(
    VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo* pPushDescriptorSetWithTemplateInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdPushDescriptorSetWithTemplate2KHR(commandBuffer, pPushDescriptorSetWithTemplateInfo);
}

static inline void DispatchCmdSetDescriptorBufferOffsets2EXT(
    VkCommandBuffer commandBuffer, const VkSetDescriptorBufferOffsetsInfoEXT* pSetDescriptorBufferOffsetsInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetDescriptorBufferOffsets2EXT(commandBuffer, pSetDescriptorBufferOffsetsInfo);
}

static inline void DispatchCmdBindDescriptorBufferEmbeddedSamplers2EXT(
    VkCommandBuffer commandBuffer, const VkBindDescriptorBufferEmbeddedSamplersInfoEXT* pBindDescriptorBufferEmbeddedSamplersInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBindDescriptorBufferEmbeddedSamplers2EXT(commandBuffer, pBindDescriptorBufferEmbeddedSamplersInfo);
}

static inline VkResult DispatchCreateDebugReportCallbackEXT(VkInstance instance,
                                                            const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            VkDebugReportCallbackEXT* pCallback) {
    auto dispatch = vvl::dispatch::GetData(instance);
    return dispatch->CreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
}

static inline void DispatchDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                         const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(instance);
    dispatch->DestroyDebugReportCallbackEXT(instance, callback, pAllocator);
}

static inline void DispatchDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                                 VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
                                                 int32_t messageCode, const char* pLayerPrefix, const char* pMessage) {
    auto dispatch = vvl::dispatch::GetData(instance);
    dispatch->DebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
}

static inline VkResult DispatchDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT* pTagInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->DebugMarkerSetObjectTagEXT(device, pTagInfo);
}

static inline VkResult DispatchDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT* pNameInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->DebugMarkerSetObjectNameEXT(device, pNameInfo);
}

static inline void DispatchCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
}

static inline void DispatchCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDebugMarkerEndEXT(commandBuffer);
}

static inline void DispatchCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
}

static inline void DispatchCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                              uint32_t bindingCount, const VkBuffer* pBuffers,
                                                              const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
}

static inline void DispatchCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                        uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                        const VkDeviceSize* pCounterBufferOffsets) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers,
                                           pCounterBufferOffsets);
}

static inline void DispatchCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                      uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                      const VkDeviceSize* pCounterBufferOffsets) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers,
                                         pCounterBufferOffsets);
}

static inline void DispatchCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                   VkQueryControlFlags flags, uint32_t index) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
}

static inline void DispatchCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                                 uint32_t index) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
}

static inline void DispatchCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                       uint32_t firstInstance, VkBuffer counterBuffer,
                                                       VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                       uint32_t vertexStride) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset,
                                          counterOffset, vertexStride);
}

static inline VkResult DispatchCreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateCuModuleNVX(device, pCreateInfo, pAllocator, pModule);
}

static inline VkResult DispatchCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateCuFunctionNVX(device, pCreateInfo, pAllocator, pFunction);
}

static inline void DispatchDestroyCuModuleNVX(VkDevice device, VkCuModuleNVX module, const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyCuModuleNVX(device, module, pAllocator);
}

static inline void DispatchDestroyCuFunctionNVX(VkDevice device, VkCuFunctionNVX function,
                                                const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyCuFunctionNVX(device, function, pAllocator);
}

static inline void DispatchCmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer, const VkCuLaunchInfoNVX* pLaunchInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdCuLaunchKernelNVX(commandBuffer, pLaunchInfo);
}

static inline uint32_t DispatchGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetImageViewHandleNVX(device, pInfo);
}

static inline uint64_t DispatchGetImageViewHandle64NVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetImageViewHandle64NVX(device, pInfo);
}

static inline VkResult DispatchGetImageViewAddressNVX(VkDevice device, VkImageView imageView,
                                                      VkImageViewAddressPropertiesNVX* pProperties) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetImageViewAddressNVX(device, imageView, pProperties);
}

static inline void DispatchCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                   uint32_t stride) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

static inline void DispatchCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                          uint32_t maxDrawCount, uint32_t stride) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

static inline VkResult DispatchGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage,
                                                VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
}
#ifdef VK_USE_PLATFORM_GGP

static inline VkResult DispatchCreateStreamDescriptorSurfaceGGP(VkInstance instance,
                                                                const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto dispatch = vvl::dispatch::GetData(instance);
    return dispatch->CreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_GGP

static inline VkResult DispatchGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
    VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags,
                                                                      externalHandleType, pExternalImageFormatProperties);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

static inline VkResult DispatchGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory,
                                                      VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetMemoryWin32HandleNV(device, memory, handleType, pHandle);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_VI_NN

static inline VkResult DispatchCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto dispatch = vvl::dispatch::GetData(instance);
    return dispatch->CreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_VI_NN

static inline void DispatchCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                                           const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
}

static inline void DispatchCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdEndConditionalRenderingEXT(commandBuffer);
}

static inline void DispatchCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                    const VkViewportWScalingNV* pViewportWScalings) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
}

static inline VkResult DispatchReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->ReleaseDisplayEXT(physicalDevice, display);
}
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

static inline VkResult DispatchAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->AcquireXlibDisplayEXT(physicalDevice, dpy, display);
}

static inline VkResult DispatchGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput,
                                                        VkDisplayKHR* pDisplay) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);
}
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT

static inline VkResult DispatchGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                        VkSurfaceCapabilities2EXT* pSurfaceCapabilities) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
}

static inline VkResult DispatchDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display,
                                                      const VkDisplayPowerInfoEXT* pDisplayPowerInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->DisplayPowerControlEXT(device, display, pDisplayPowerInfo);
}

static inline VkResult DispatchRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->RegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
}

static inline VkResult DispatchRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display,
                                                       const VkDisplayEventInfoEXT* pDisplayEventInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->RegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
}

static inline VkResult DispatchGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                      VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
}

static inline VkResult DispatchGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                             VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);
}

static inline VkResult DispatchGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                               uint32_t* pPresentationTimingCount,
                                                               VkPastPresentationTimingGOOGLE* pPresentationTimings) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings);
}

static inline void DispatchCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                     uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
}

static inline void DispatchCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetDiscardRectangleEnableEXT(commandBuffer, discardRectangleEnable);
}

static inline void DispatchCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer,
                                                         VkDiscardRectangleModeEXT discardRectangleMode) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetDiscardRectangleModeEXT(commandBuffer, discardRectangleMode);
}

static inline void DispatchSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains,
                                             const VkHdrMetadataEXT* pMetadata) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->SetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
}
#ifdef VK_USE_PLATFORM_IOS_MVK

static inline VkResult DispatchCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto dispatch = vvl::dispatch::GetData(instance);
    return dispatch->CreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK

static inline VkResult DispatchCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto dispatch = vvl::dispatch::GetData(instance);
    return dispatch->CreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_MACOS_MVK

static inline VkResult DispatchSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->SetDebugUtilsObjectNameEXT(device, pNameInfo);
}

static inline VkResult DispatchSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->SetDebugUtilsObjectTagEXT(device, pTagInfo);
}

static inline void DispatchQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {
    auto dispatch = vvl::dispatch::GetData(queue);
    dispatch->QueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
}

static inline void DispatchQueueEndDebugUtilsLabelEXT(VkQueue queue) {
    auto dispatch = vvl::dispatch::GetData(queue);
    dispatch->QueueEndDebugUtilsLabelEXT(queue);
}

static inline void DispatchQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {
    auto dispatch = vvl::dispatch::GetData(queue);
    dispatch->QueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
}

static inline void DispatchCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}

static inline void DispatchCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdEndDebugUtilsLabelEXT(commandBuffer);
}

static inline void DispatchCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}

static inline VkResult DispatchCreateDebugUtilsMessengerEXT(VkInstance instance,
                                                            const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator,
                                                            VkDebugUtilsMessengerEXT* pMessenger) {
    auto dispatch = vvl::dispatch::GetData(instance);
    return dispatch->CreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

static inline void DispatchDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                         const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(instance);
    dispatch->DestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

static inline void DispatchSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                      VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) {
    auto dispatch = vvl::dispatch::GetData(instance);
    dispatch->SubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
}
#ifdef VK_USE_PLATFORM_ANDROID_KHR

static inline VkResult DispatchGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                                         VkAndroidHardwareBufferPropertiesANDROID* pProperties) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);
}

static inline VkResult DispatchGetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                                     const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
                                                                     struct AHardwareBuffer** pBuffer) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS

static inline VkResult DispatchCreateExecutionGraphPipelinesAMDX(VkDevice device, VkPipelineCache pipelineCache,
                                                                 uint32_t createInfoCount,
                                                                 const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos,
                                                                 const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateExecutionGraphPipelinesAMDX(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                       pPipelines);
}

static inline VkResult DispatchGetExecutionGraphPipelineScratchSizeAMDX(VkDevice device, VkPipeline executionGraph,
                                                                        VkExecutionGraphPipelineScratchSizeAMDX* pSizeInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetExecutionGraphPipelineScratchSizeAMDX(device, executionGraph, pSizeInfo);
}

static inline VkResult DispatchGetExecutionGraphPipelineNodeIndexAMDX(VkDevice device, VkPipeline executionGraph,
                                                                      const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo,
                                                                      uint32_t* pNodeIndex) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetExecutionGraphPipelineNodeIndexAMDX(device, executionGraph, pNodeInfo, pNodeIndex);
}

static inline void DispatchCmdInitializeGraphScratchMemoryAMDX(VkCommandBuffer commandBuffer, VkPipeline executionGraph,
                                                               VkDeviceAddress scratch, VkDeviceSize scratchSize) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdInitializeGraphScratchMemoryAMDX(commandBuffer, executionGraph, scratch, scratchSize);
}

static inline void DispatchCmdDispatchGraphAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch, VkDeviceSize scratchSize,
                                                const VkDispatchGraphCountInfoAMDX* pCountInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDispatchGraphAMDX(commandBuffer, scratch, scratchSize, pCountInfo);
}

static inline void DispatchCmdDispatchGraphIndirectAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                        VkDeviceSize scratchSize, const VkDispatchGraphCountInfoAMDX* pCountInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDispatchGraphIndirectAMDX(commandBuffer, scratch, scratchSize, pCountInfo);
}

static inline void DispatchCmdDispatchGraphIndirectCountAMDX(VkCommandBuffer commandBuffer, VkDeviceAddress scratch,
                                                             VkDeviceSize scratchSize, VkDeviceAddress countInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDispatchGraphIndirectCountAMDX(commandBuffer, scratch, scratchSize, countInfo);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS

static inline void DispatchCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                    const VkSampleLocationsInfoEXT* pSampleLocationsInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
}

static inline void DispatchGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples,
                                                                     VkMultisamplePropertiesEXT* pMultisampleProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    dispatch->GetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
}

static inline VkResult DispatchGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image,
                                                                      VkImageDrmFormatModifierPropertiesEXT* pProperties) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);
}

static inline VkResult DispatchCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkValidationCacheEXT* pValidationCache) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateValidationCacheEXT(device, pCreateInfo, pAllocator, pValidationCache);
}

static inline void DispatchDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                                     const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyValidationCacheEXT(device, validationCache, pAllocator);
}

static inline VkResult DispatchMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                                        const VkValidationCacheEXT* pSrcCaches) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->MergeValidationCachesEXT(device, dstCache, srcCacheCount, pSrcCaches);
}

static inline VkResult DispatchGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize,
                                                         void* pData) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetValidationCacheDataEXT(device, validationCache, pDataSize, pData);
}

static inline void DispatchCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                     VkImageLayout imageLayout) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
}

static inline void DispatchCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                              uint32_t viewportCount,
                                                              const VkShadingRatePaletteNV* pShadingRatePalettes) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
}

static inline void DispatchCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                     uint32_t customSampleOrderCount,
                                                     const VkCoarseSampleOrderCustomNV* pCustomSampleOrders) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
}

static inline VkResult DispatchCreateAccelerationStructureNV(VkDevice device,
                                                             const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             VkAccelerationStructureNV* pAccelerationStructure) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure);
}

static inline void DispatchDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                          const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);
}

static inline void DispatchGetAccelerationStructureMemoryRequirementsNV(
    VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
}

static inline VkResult DispatchBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                                 const VkBindAccelerationStructureMemoryInfoNV* pBindInfos) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->BindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos);
}

static inline void DispatchCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                           const VkAccelerationStructureInfoNV* pInfo, VkBuffer instanceData,
                                                           VkDeviceSize instanceOffset, VkBool32 update,
                                                           VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                           VkBuffer scratch, VkDeviceSize scratchOffset) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch,
                                              scratchOffset);
}

static inline void DispatchCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                          VkAccelerationStructureNV src, VkCopyAccelerationStructureModeKHR mode) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
}

static inline void DispatchCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                          VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                          VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                          VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                          VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                          VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                          uint32_t width, uint32_t height, uint32_t depth) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer,
                             missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset,
                             hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset,
                             callableShaderBindingStride, width, height, depth);
}

static inline VkResult DispatchCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                           const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                           const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

static inline VkResult DispatchGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                  uint32_t groupCount, size_t dataSize, void* pData) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
}

static inline VkResult DispatchGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup,
                                                                 uint32_t groupCount, size_t dataSize, void* pData) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData);
}

static inline VkResult DispatchGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                                size_t dataSize, void* pData) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData);
}

static inline void DispatchCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer,
                                                                      uint32_t accelerationStructureCount,
                                                                      const VkAccelerationStructureNV* pAccelerationStructures,
                                                                      VkQueryType queryType, VkQueryPool queryPool,
                                                                      uint32_t firstQuery) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, pAccelerationStructures,
                                                         queryType, queryPool, firstQuery);
}

static inline VkResult DispatchCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CompileDeferredNV(device, pipeline, shader);
}

static inline VkResult DispatchGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                 const void* pHostPointer,
                                                                 VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties);
}

static inline void DispatchCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                   VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
}

static inline void DispatchCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkBuffer dstBuffer,
                                                    VkDeviceSize dstOffset, uint32_t marker) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdWriteBufferMarker2AMD(commandBuffer, stage, dstBuffer, dstOffset, marker);
}

static inline VkResult DispatchGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice,
                                                                            uint32_t* pTimeDomainCount,
                                                                            VkTimeDomainKHR* pTimeDomains) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains);
}

static inline VkResult DispatchGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                                          const VkCalibratedTimestampInfoKHR* pTimestampInfos,
                                                          uint64_t* pTimestamps, uint64_t* pMaxDeviation) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
}

static inline void DispatchCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
}

static inline void DispatchCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      uint32_t drawCount, uint32_t stride) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
}

static inline void DispatchCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

static inline void DispatchCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                          uint32_t exclusiveScissorCount,
                                                          const VkBool32* pExclusiveScissorEnables) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetExclusiveScissorEnableNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissorEnables);
}

static inline void DispatchCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                    uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
}

static inline void DispatchCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
}

static inline void DispatchGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                                    VkCheckpointDataNV* pCheckpointData) {
    auto dispatch = vvl::dispatch::GetData(queue);
    dispatch->GetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
}

static inline void DispatchGetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount,
                                                     VkCheckpointData2NV* pCheckpointData) {
    auto dispatch = vvl::dispatch::GetData(queue);
    dispatch->GetQueueCheckpointData2NV(queue, pCheckpointDataCount, pCheckpointData);
}

static inline VkResult DispatchInitializePerformanceApiINTEL(VkDevice device,
                                                             const VkInitializePerformanceApiInfoINTEL* pInitializeInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->InitializePerformanceApiINTEL(device, pInitializeInfo);
}

static inline void DispatchUninitializePerformanceApiINTEL(VkDevice device) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->UninitializePerformanceApiINTEL(device);
}

static inline VkResult DispatchCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer,
                                                            const VkPerformanceMarkerInfoINTEL* pMarkerInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    return dispatch->CmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo);
}

static inline VkResult DispatchCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                                  const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    return dispatch->CmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo);
}

static inline VkResult DispatchCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                              const VkPerformanceOverrideInfoINTEL* pOverrideInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    return dispatch->CmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo);
}

static inline VkResult DispatchAcquirePerformanceConfigurationINTEL(VkDevice device,
                                                                    const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
                                                                    VkPerformanceConfigurationINTEL* pConfiguration) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->AcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration);
}

static inline VkResult DispatchReleasePerformanceConfigurationINTEL(VkDevice device,
                                                                    VkPerformanceConfigurationINTEL configuration) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->ReleasePerformanceConfigurationINTEL(device, configuration);
}

static inline VkResult DispatchQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration) {
    auto dispatch = vvl::dispatch::GetData(queue);
    return dispatch->QueueSetPerformanceConfigurationINTEL(queue, configuration);
}

static inline VkResult DispatchGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter,
                                                            VkPerformanceValueINTEL* pValue) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetPerformanceParameterINTEL(device, parameter, pValue);
}

static inline void DispatchSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->SetLocalDimmingAMD(device, swapChain, localDimmingEnable);
}
#ifdef VK_USE_PLATFORM_FUCHSIA

static inline VkResult DispatchCreateImagePipeSurfaceFUCHSIA(VkInstance instance,
                                                             const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto dispatch = vvl::dispatch::GetData(instance);
    return dispatch->CreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT

static inline VkResult DispatchCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto dispatch = vvl::dispatch::GetData(instance);
    return dispatch->CreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_METAL_EXT

static inline VkDeviceAddress DispatchGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetBufferDeviceAddressEXT(device, pInfo);
}

static inline VkResult DispatchGetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                                  VkPhysicalDeviceToolProperties* pToolProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties);
}

static inline VkResult DispatchGetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice,
                                                                              uint32_t* pPropertyCount,
                                                                              VkCooperativeMatrixPropertiesNV* pProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties);
}

static inline VkResult DispatchGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice physicalDevice, uint32_t* pCombinationCount, VkFramebufferMixedSamplesCombinationNV* pCombinations) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice, pCombinationCount,
                                                                                     pCombinations);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

static inline VkResult DispatchGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
                                                                        const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                        uint32_t* pPresentModeCount,
                                                                        VkPresentModeKHR* pPresentModes) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes);
}

static inline VkResult DispatchAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->AcquireFullScreenExclusiveModeEXT(device, swapchain);
}

static inline VkResult DispatchReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->ReleaseFullScreenExclusiveModeEXT(device, swapchain);
}

static inline VkResult DispatchGetDeviceGroupSurfacePresentModes2EXT(VkDevice device,
                                                                     const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                     VkDeviceGroupPresentModeFlagsKHR* pModes) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

static inline VkResult DispatchCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto dispatch = vvl::dispatch::GetData(instance);
    return dispatch->CreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
}

static inline void DispatchCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                uint16_t lineStipplePattern) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
}

static inline void DispatchResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->ResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
}

static inline void DispatchCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetCullModeEXT(commandBuffer, cullMode);
}

static inline void DispatchCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetFrontFaceEXT(commandBuffer, frontFace);
}

static inline void DispatchCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetPrimitiveTopologyEXT(commandBuffer, primitiveTopology);
}

static inline void DispatchCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                      const VkViewport* pViewports) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetViewportWithCountEXT(commandBuffer, viewportCount, pViewports);
}

static inline void DispatchCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                     const VkRect2D* pScissors) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetScissorWithCountEXT(commandBuffer, scissorCount, pScissors);
}

static inline void DispatchCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                    const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                                    const VkDeviceSize* pSizes, const VkDeviceSize* pStrides) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
}

static inline void DispatchCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);
}

static inline void DispatchCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetDepthWriteEnableEXT(commandBuffer, depthWriteEnable);
}

static inline void DispatchCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);
}

static inline void DispatchCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetDepthBoundsTestEnableEXT(commandBuffer, depthBoundsTestEnable);
}

static inline void DispatchCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetStencilTestEnableEXT(commandBuffer, stencilTestEnable);
}

static inline void DispatchCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                              VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetStencilOpEXT(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}

static inline VkResult DispatchCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfo* pCopyMemoryToImageInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CopyMemoryToImageEXT(device, pCopyMemoryToImageInfo);
}

static inline VkResult DispatchCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfo* pCopyImageToMemoryInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CopyImageToMemoryEXT(device, pCopyImageToMemoryInfo);
}

static inline VkResult DispatchCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfo* pCopyImageToImageInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CopyImageToImageEXT(device, pCopyImageToImageInfo);
}

static inline VkResult DispatchTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                                        const VkHostImageLayoutTransitionInfo* pTransitions) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->TransitionImageLayoutEXT(device, transitionCount, pTransitions);
}

static inline void DispatchGetImageSubresourceLayout2EXT(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource,
                                                         VkSubresourceLayout2* pLayout) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetImageSubresourceLayout2EXT(device, image, pSubresource, pLayout);
}

static inline VkResult DispatchReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->ReleaseSwapchainImagesEXT(device, pReleaseInfo);
}

static inline void DispatchGetGeneratedCommandsMemoryRequirementsNV(VkDevice device,
                                                                    const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
                                                                    VkMemoryRequirements2* pMemoryRequirements) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetGeneratedCommandsMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
}

static inline void DispatchCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer,
                                                            const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo);
}

static inline void DispatchCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                         const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
}

static inline void DispatchCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                        VkPipeline pipeline, uint32_t groupIndex) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
}

static inline VkResult DispatchCreateIndirectCommandsLayoutNV(VkDevice device,
                                                              const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              VkIndirectCommandsLayoutNV* pIndirectCommandsLayout) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
}

static inline void DispatchDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout,
                                                           const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);
}

static inline void DispatchCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT* pDepthBiasInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetDepthBias2EXT(commandBuffer, pDepthBiasInfo);
}

static inline VkResult DispatchAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->AcquireDrmDisplayEXT(physicalDevice, drmFd, display);
}

static inline VkResult DispatchGetDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, uint32_t connectorId,
                                                VkDisplayKHR* display) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetDrmDisplayEXT(physicalDevice, drmFd, connectorId, display);
}

static inline VkResult DispatchCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkPrivateDataSlot* pPrivateDataSlot) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot);
}

static inline void DispatchDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlot privateDataSlot,
                                                     const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);
}

static inline VkResult DispatchSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                                 VkPrivateDataSlot privateDataSlot, uint64_t data) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->SetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data);
}

static inline void DispatchGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                             VkPrivateDataSlot privateDataSlot, uint64_t* pData) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, pData);
}

static inline VkResult DispatchCreateCudaModuleNV(VkDevice device, const VkCudaModuleCreateInfoNV* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkCudaModuleNV* pModule) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateCudaModuleNV(device, pCreateInfo, pAllocator, pModule);
}

static inline VkResult DispatchGetCudaModuleCacheNV(VkDevice device, VkCudaModuleNV module, size_t* pCacheSize, void* pCacheData) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetCudaModuleCacheNV(device, module, pCacheSize, pCacheData);
}

static inline VkResult DispatchCreateCudaFunctionNV(VkDevice device, const VkCudaFunctionCreateInfoNV* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkCudaFunctionNV* pFunction) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateCudaFunctionNV(device, pCreateInfo, pAllocator, pFunction);
}

static inline void DispatchDestroyCudaModuleNV(VkDevice device, VkCudaModuleNV module, const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyCudaModuleNV(device, module, pAllocator);
}

static inline void DispatchDestroyCudaFunctionNV(VkDevice device, VkCudaFunctionNV function,
                                                 const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyCudaFunctionNV(device, function, pAllocator);
}

static inline void DispatchCmdCudaLaunchKernelNV(VkCommandBuffer commandBuffer, const VkCudaLaunchInfoNV* pLaunchInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdCudaLaunchKernelNV(commandBuffer, pLaunchInfo);
}
#ifdef VK_USE_PLATFORM_METAL_EXT

static inline void DispatchExportMetalObjectsEXT(VkDevice device, VkExportMetalObjectsInfoEXT* pMetalObjectsInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->ExportMetalObjectsEXT(device, pMetalObjectsInfo);
}
#endif  // VK_USE_PLATFORM_METAL_EXT

static inline void DispatchGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                         VkDeviceSize* pLayoutSizeInBytes) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetDescriptorSetLayoutSizeEXT(device, layout, pLayoutSizeInBytes);
}

static inline void DispatchGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout, uint32_t binding,
                                                                  VkDeviceSize* pOffset) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetDescriptorSetLayoutBindingOffsetEXT(device, layout, binding, pOffset);
}

static inline void DispatchGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT* pDescriptorInfo, size_t dataSize,
                                            void* pDescriptor) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetDescriptorEXT(device, pDescriptorInfo, dataSize, pDescriptor);
}

static inline void DispatchCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                       const VkDescriptorBufferBindingInfoEXT* pBindingInfos) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBindDescriptorBuffersEXT(commandBuffer, bufferCount, pBindingInfos);
}

static inline void DispatchCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                            VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                                            const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetDescriptorBufferOffsetsEXT(commandBuffer, pipelineBindPoint, layout, firstSet, setCount, pBufferIndices,
                                               pOffsets);
}

static inline void DispatchCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                      VkPipelineBindPoint pipelineBindPoint,
                                                                      VkPipelineLayout layout, uint32_t set) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBindDescriptorBufferEmbeddedSamplersEXT(commandBuffer, pipelineBindPoint, layout, set);
}

static inline VkResult DispatchGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                       const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
                                                                       void* pData) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetBufferOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
}

static inline VkResult DispatchGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                      const VkImageCaptureDescriptorDataInfoEXT* pInfo,
                                                                      void* pData) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetImageOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
}

static inline VkResult DispatchGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                          const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
                                                                          void* pData) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetImageViewOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
}

static inline VkResult DispatchGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                        const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
                                                                        void* pData) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetSamplerOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
}

static inline VkResult DispatchGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo, void* pData) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetAccelerationStructureOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
}

static inline void DispatchCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate,
                                                           const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetFragmentShadingRateEnumNV(commandBuffer, shadingRate, combinerOps);
}

static inline VkResult DispatchGetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT* pFaultCounts,
                                                     VkDeviceFaultInfoEXT* pFaultInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetDeviceFaultInfoEXT(device, pFaultCounts, pFaultInfo);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

static inline VkResult DispatchAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->AcquireWinrtDisplayNV(physicalDevice, display);
}

static inline VkResult DispatchGetWinrtDisplayNV(VkPhysicalDevice physicalDevice, uint32_t deviceRelativeId,
                                                 VkDisplayKHR* pDisplay) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetWinrtDisplayNV(physicalDevice, deviceRelativeId, pDisplay);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT

static inline VkResult DispatchCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto dispatch = vvl::dispatch::GetData(instance);
    return dispatch->CreateDirectFBSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
}

static inline VkBool32 DispatchGetPhysicalDeviceDirectFBPresentationSupportEXT(VkPhysicalDevice physicalDevice,
                                                                               uint32_t queueFamilyIndex, IDirectFB* dfb) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceDirectFBPresentationSupportEXT(physicalDevice, queueFamilyIndex, dfb);
}
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT

static inline void DispatchCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                                const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                                                uint32_t vertexAttributeDescriptionCount,
                                                const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetVertexInputEXT(commandBuffer, vertexBindingDescriptionCount, pVertexBindingDescriptions,
                                   vertexAttributeDescriptionCount, pVertexAttributeDescriptions);
}
#ifdef VK_USE_PLATFORM_FUCHSIA

static inline VkResult DispatchGetMemoryZirconHandleFUCHSIA(VkDevice device,
                                                            const VkMemoryGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                            zx_handle_t* pZirconHandle) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetMemoryZirconHandleFUCHSIA(device, pGetZirconHandleInfo, pZirconHandle);
}

static inline VkResult DispatchGetMemoryZirconHandlePropertiesFUCHSIA(
    VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, zx_handle_t zirconHandle,
    VkMemoryZirconHandlePropertiesFUCHSIA* pMemoryZirconHandleProperties) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetMemoryZirconHandlePropertiesFUCHSIA(device, handleType, zirconHandle, pMemoryZirconHandleProperties);
}

static inline VkResult DispatchImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->ImportSemaphoreZirconHandleFUCHSIA(device, pImportSemaphoreZirconHandleInfo);
}

static inline VkResult DispatchGetSemaphoreZirconHandleFUCHSIA(VkDevice device,
                                                               const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
                                                               zx_handle_t* pZirconHandle) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetSemaphoreZirconHandleFUCHSIA(device, pGetZirconHandleInfo, pZirconHandle);
}

static inline VkResult DispatchCreateBufferCollectionFUCHSIA(VkDevice device,
                                                             const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             VkBufferCollectionFUCHSIA* pCollection) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateBufferCollectionFUCHSIA(device, pCreateInfo, pAllocator, pCollection);
}

static inline VkResult DispatchSetBufferCollectionImageConstraintsFUCHSIA(
    VkDevice device, VkBufferCollectionFUCHSIA collection, const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->SetBufferCollectionImageConstraintsFUCHSIA(device, collection, pImageConstraintsInfo);
}

static inline VkResult DispatchSetBufferCollectionBufferConstraintsFUCHSIA(
    VkDevice device, VkBufferCollectionFUCHSIA collection, const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->SetBufferCollectionBufferConstraintsFUCHSIA(device, collection, pBufferConstraintsInfo);
}

static inline void DispatchDestroyBufferCollectionFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                          const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyBufferCollectionFUCHSIA(device, collection, pAllocator);
}

static inline VkResult DispatchGetBufferCollectionPropertiesFUCHSIA(VkDevice device, VkBufferCollectionFUCHSIA collection,
                                                                    VkBufferCollectionPropertiesFUCHSIA* pProperties) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetBufferCollectionPropertiesFUCHSIA(device, collection, pProperties);
}
#endif  // VK_USE_PLATFORM_FUCHSIA

static inline VkResult DispatchGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass,
                                                                             VkExtent2D* pMaxWorkgroupSize) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(device, renderpass, pMaxWorkgroupSize);
}

static inline void DispatchCmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSubpassShadingHUAWEI(commandBuffer);
}

static inline void DispatchCmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                       VkImageLayout imageLayout) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBindInvocationMaskHUAWEI(commandBuffer, imageView, imageLayout);
}

static inline VkResult DispatchGetMemoryRemoteAddressNV(VkDevice device,
                                                        const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo,
                                                        VkRemoteAddressNV* pAddress) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetMemoryRemoteAddressNV(device, pMemoryGetRemoteAddressInfo, pAddress);
}

static inline VkResult DispatchGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT* pPipelineInfo,
                                                        VkBaseOutStructure* pPipelineProperties) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetPipelinePropertiesEXT(device, pPipelineInfo, pPipelineProperties);
}

static inline void DispatchCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetPatchControlPointsEXT(commandBuffer, patchControlPoints);
}

static inline void DispatchCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetRasterizerDiscardEnableEXT(commandBuffer, rasterizerDiscardEnable);
}

static inline void DispatchCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetDepthBiasEnableEXT(commandBuffer, depthBiasEnable);
}

static inline void DispatchCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetLogicOpEXT(commandBuffer, logicOp);
}

static inline void DispatchCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetPrimitiveRestartEnableEXT(commandBuffer, primitiveRestartEnable);
}
#ifdef VK_USE_PLATFORM_SCREEN_QNX

static inline VkResult DispatchCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto dispatch = vvl::dispatch::GetData(instance);
    return dispatch->CreateScreenSurfaceQNX(instance, pCreateInfo, pAllocator, pSurface);
}

static inline VkBool32 DispatchGetPhysicalDeviceScreenPresentationSupportQNX(VkPhysicalDevice physicalDevice,
                                                                             uint32_t queueFamilyIndex,
                                                                             struct _screen_window* window) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceScreenPresentationSupportQNX(physicalDevice, queueFamilyIndex, window);
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX

static inline void DispatchCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                     const VkBool32* pColorWriteEnables) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetColorWriteEnableEXT(commandBuffer, attachmentCount, pColorWriteEnables);
}

static inline void DispatchCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                           uint32_t instanceCount, uint32_t firstInstance, uint32_t stride) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDrawMultiEXT(commandBuffer, drawCount, pVertexInfo, instanceCount, firstInstance, stride);
}

static inline void DispatchCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                  const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                                  uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDrawMultiIndexedEXT(commandBuffer, drawCount, pIndexInfo, instanceCount, firstInstance, stride, pVertexOffset);
}

static inline VkResult DispatchCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateMicromapEXT(device, pCreateInfo, pAllocator, pMicromap);
}

static inline void DispatchDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap, const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyMicromapEXT(device, micromap, pAllocator);
}

static inline void DispatchCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                const VkMicromapBuildInfoEXT* pInfos) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBuildMicromapsEXT(commandBuffer, infoCount, pInfos);
}

static inline VkResult DispatchBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                                 const VkMicromapBuildInfoEXT* pInfos) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->BuildMicromapsEXT(device, deferredOperation, infoCount, pInfos);
}

static inline VkResult DispatchCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                               const VkCopyMicromapInfoEXT* pInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CopyMicromapEXT(device, deferredOperation, pInfo);
}

static inline VkResult DispatchCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                       const VkCopyMicromapToMemoryInfoEXT* pInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CopyMicromapToMemoryEXT(device, deferredOperation, pInfo);
}

static inline VkResult DispatchCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                       const VkCopyMemoryToMicromapInfoEXT* pInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CopyMemoryToMicromapEXT(device, deferredOperation, pInfo);
}

static inline VkResult DispatchWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount, const VkMicromapEXT* pMicromaps,
                                                           VkQueryType queryType, size_t dataSize, void* pData, size_t stride) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->WriteMicromapsPropertiesEXT(device, micromapCount, pMicromaps, queryType, dataSize, pData, stride);
}

static inline void DispatchCmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT* pInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdCopyMicromapEXT(commandBuffer, pInfo);
}

static inline void DispatchCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapToMemoryInfoEXT* pInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdCopyMicromapToMemoryEXT(commandBuffer, pInfo);
}

static inline void DispatchCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMemoryToMicromapInfoEXT* pInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdCopyMemoryToMicromapEXT(commandBuffer, pInfo);
}

static inline void DispatchCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount,
                                                          const VkMicromapEXT* pMicromaps, VkQueryType queryType,
                                                          VkQueryPool queryPool, uint32_t firstQuery) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdWriteMicromapsPropertiesEXT(commandBuffer, micromapCount, pMicromaps, queryType, queryPool, firstQuery);
}

static inline void DispatchGetDeviceMicromapCompatibilityEXT(VkDevice device, const VkMicromapVersionInfoEXT* pVersionInfo,
                                                             VkAccelerationStructureCompatibilityKHR* pCompatibility) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetDeviceMicromapCompatibilityEXT(device, pVersionInfo, pCompatibility);
}

static inline void DispatchGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                    const VkMicromapBuildInfoEXT* pBuildInfo,
                                                    VkMicromapBuildSizesInfoEXT* pSizeInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetMicromapBuildSizesEXT(device, buildType, pBuildInfo, pSizeInfo);
}

static inline void DispatchCmdDrawClusterHUAWEI(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                                uint32_t groupCountZ) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDrawClusterHUAWEI(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

static inline void DispatchCmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDrawClusterIndirectHUAWEI(commandBuffer, buffer, offset);
}

static inline void DispatchSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->SetDeviceMemoryPriorityEXT(device, memory, priority);
}

static inline void DispatchGetDescriptorSetLayoutHostMappingInfoVALVE(VkDevice device,
                                                                      const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
                                                                      VkDescriptorSetLayoutHostMappingInfoVALVE* pHostMapping) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetDescriptorSetLayoutHostMappingInfoVALVE(device, pBindingReference, pHostMapping);
}

static inline void DispatchGetDescriptorSetHostMappingVALVE(VkDevice device, VkDescriptorSet descriptorSet, void** ppData) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetDescriptorSetHostMappingVALVE(device, descriptorSet, ppData);
}

static inline void DispatchCmdCopyMemoryIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                   uint32_t copyCount, uint32_t stride) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdCopyMemoryIndirectNV(commandBuffer, copyBufferAddress, copyCount, stride);
}

static inline void DispatchCmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress,
                                                          uint32_t copyCount, uint32_t stride, VkImage dstImage,
                                                          VkImageLayout dstImageLayout,
                                                          const VkImageSubresourceLayers* pImageSubresources) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdCopyMemoryToImageIndirectNV(commandBuffer, copyBufferAddress, copyCount, stride, dstImage, dstImageLayout,
                                             pImageSubresources);
}

static inline void DispatchCmdDecompressMemoryNV(VkCommandBuffer commandBuffer, uint32_t decompressRegionCount,
                                                 const VkDecompressMemoryRegionNV* pDecompressMemoryRegions) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDecompressMemoryNV(commandBuffer, decompressRegionCount, pDecompressMemoryRegions);
}

static inline void DispatchCmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer,
                                                              VkDeviceAddress indirectCommandsAddress,
                                                              VkDeviceAddress indirectCommandsCountAddress, uint32_t stride) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDecompressMemoryIndirectCountNV(commandBuffer, indirectCommandsAddress, indirectCommandsCountAddress, stride);
}

static inline void DispatchGetPipelineIndirectMemoryRequirementsNV(VkDevice device, const VkComputePipelineCreateInfo* pCreateInfo,
                                                                   VkMemoryRequirements2* pMemoryRequirements) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetPipelineIndirectMemoryRequirementsNV(device, pCreateInfo, pMemoryRequirements);
}

static inline void DispatchCmdUpdatePipelineIndirectBufferNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                             VkPipeline pipeline) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdUpdatePipelineIndirectBufferNV(commandBuffer, pipelineBindPoint, pipeline);
}

static inline VkDeviceAddress DispatchGetPipelineIndirectDeviceAddressNV(VkDevice device,
                                                                         const VkPipelineIndirectDeviceAddressInfoNV* pInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetPipelineIndirectDeviceAddressNV(device, pInfo);
}

static inline void DispatchCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetDepthClampEnableEXT(commandBuffer, depthClampEnable);
}

static inline void DispatchCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetPolygonModeEXT(commandBuffer, polygonMode);
}

static inline void DispatchCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer,
                                                         VkSampleCountFlagBits rasterizationSamples) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetRasterizationSamplesEXT(commandBuffer, rasterizationSamples);
}

static inline void DispatchCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                               const VkSampleMask* pSampleMask) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetSampleMaskEXT(commandBuffer, samples, pSampleMask);
}

static inline void DispatchCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetAlphaToCoverageEnableEXT(commandBuffer, alphaToCoverageEnable);
}

static inline void DispatchCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetAlphaToOneEnableEXT(commandBuffer, alphaToOneEnable);
}

static inline void DispatchCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetLogicOpEnableEXT(commandBuffer, logicOpEnable);
}

static inline void DispatchCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                     uint32_t attachmentCount, const VkBool32* pColorBlendEnables) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetColorBlendEnableEXT(commandBuffer, firstAttachment, attachmentCount, pColorBlendEnables);
}

static inline void DispatchCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                       uint32_t attachmentCount,
                                                       const VkColorBlendEquationEXT* pColorBlendEquations) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetColorBlendEquationEXT(commandBuffer, firstAttachment, attachmentCount, pColorBlendEquations);
}

static inline void DispatchCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                   uint32_t attachmentCount, const VkColorComponentFlags* pColorWriteMasks) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetColorWriteMaskEXT(commandBuffer, firstAttachment, attachmentCount, pColorWriteMasks);
}

static inline void DispatchCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer,
                                                             VkTessellationDomainOrigin domainOrigin) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetTessellationDomainOriginEXT(commandBuffer, domainOrigin);
}

static inline void DispatchCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetRasterizationStreamEXT(commandBuffer, rasterizationStream);
}

static inline void DispatchCmdSetConservativeRasterizationModeEXT(
    VkCommandBuffer commandBuffer, VkConservativeRasterizationModeEXT conservativeRasterizationMode) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetConservativeRasterizationModeEXT(commandBuffer, conservativeRasterizationMode);
}

static inline void DispatchCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                                     float extraPrimitiveOverestimationSize) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetExtraPrimitiveOverestimationSizeEXT(commandBuffer, extraPrimitiveOverestimationSize);
}

static inline void DispatchCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetDepthClipEnableEXT(commandBuffer, depthClipEnable);
}

static inline void DispatchCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetSampleLocationsEnableEXT(commandBuffer, sampleLocationsEnable);
}

static inline void DispatchCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                       uint32_t attachmentCount,
                                                       const VkColorBlendAdvancedEXT* pColorBlendAdvanced) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetColorBlendAdvancedEXT(commandBuffer, firstAttachment, attachmentCount, pColorBlendAdvanced);
}

static inline void DispatchCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer,
                                                        VkProvokingVertexModeEXT provokingVertexMode) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetProvokingVertexModeEXT(commandBuffer, provokingVertexMode);
}

static inline void DispatchCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                          VkLineRasterizationModeEXT lineRasterizationMode) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetLineRasterizationModeEXT(commandBuffer, lineRasterizationMode);
}

static inline void DispatchCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetLineStippleEnableEXT(commandBuffer, stippledLineEnable);
}

static inline void DispatchCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetDepthClipNegativeOneToOneEXT(commandBuffer, negativeOneToOne);
}

static inline void DispatchCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetViewportWScalingEnableNV(commandBuffer, viewportWScalingEnable);
}

static inline void DispatchCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                   const VkViewportSwizzleNV* pViewportSwizzles) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetViewportSwizzleNV(commandBuffer, firstViewport, viewportCount, pViewportSwizzles);
}

static inline void DispatchCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetCoverageToColorEnableNV(commandBuffer, coverageToColorEnable);
}

static inline void DispatchCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetCoverageToColorLocationNV(commandBuffer, coverageToColorLocation);
}

static inline void DispatchCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                          VkCoverageModulationModeNV coverageModulationMode) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetCoverageModulationModeNV(commandBuffer, coverageModulationMode);
}

static inline void DispatchCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer,
                                                                 VkBool32 coverageModulationTableEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetCoverageModulationTableEnableNV(commandBuffer, coverageModulationTableEnable);
}

static inline void DispatchCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer, uint32_t coverageModulationTableCount,
                                                           const float* pCoverageModulationTable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetCoverageModulationTableNV(commandBuffer, coverageModulationTableCount, pCoverageModulationTable);
}

static inline void DispatchCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetShadingRateImageEnableNV(commandBuffer, shadingRateImageEnable);
}

static inline void DispatchCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                                    VkBool32 representativeFragmentTestEnable) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetRepresentativeFragmentTestEnableNV(commandBuffer, representativeFragmentTestEnable);
}

static inline void DispatchCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer,
                                                         VkCoverageReductionModeNV coverageReductionMode) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetCoverageReductionModeNV(commandBuffer, coverageReductionMode);
}

static inline void DispatchGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                        VkShaderModuleIdentifierEXT* pIdentifier) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetShaderModuleIdentifierEXT(device, shaderModule, pIdentifier);
}

static inline void DispatchGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                                  VkShaderModuleIdentifierEXT* pIdentifier) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetShaderModuleCreateInfoIdentifierEXT(device, pCreateInfo, pIdentifier);
}

static inline VkResult DispatchGetPhysicalDeviceOpticalFlowImageFormatsNV(
    VkPhysicalDevice physicalDevice, const VkOpticalFlowImageFormatInfoNV* pOpticalFlowImageFormatInfo, uint32_t* pFormatCount,
    VkOpticalFlowImageFormatPropertiesNV* pImageFormatProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceOpticalFlowImageFormatsNV(physicalDevice, pOpticalFlowImageFormatInfo, pFormatCount,
                                                                pImageFormatProperties);
}

static inline VkResult DispatchCreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          VkOpticalFlowSessionNV* pSession) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateOpticalFlowSessionNV(device, pCreateInfo, pAllocator, pSession);
}

static inline void DispatchDestroyOpticalFlowSessionNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                       const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyOpticalFlowSessionNV(device, session, pAllocator);
}

static inline VkResult DispatchBindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session,
                                                             VkOpticalFlowSessionBindingPointNV bindingPoint, VkImageView view,
                                                             VkImageLayout layout) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->BindOpticalFlowSessionImageNV(device, session, bindingPoint, view, layout);
}

static inline void DispatchCmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer, VkOpticalFlowSessionNV session,
                                                   const VkOpticalFlowExecuteInfoNV* pExecuteInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdOpticalFlowExecuteNV(commandBuffer, session, pExecuteInfo);
}

static inline void DispatchAntiLagUpdateAMD(VkDevice device, const VkAntiLagDataAMD* pData) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->AntiLagUpdateAMD(device, pData);
}

static inline VkResult DispatchCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                const VkShaderCreateInfoEXT* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                                VkShaderEXT* pShaders) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders);
}

static inline void DispatchDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyShaderEXT(device, shader, pAllocator);
}

static inline VkResult DispatchGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetShaderBinaryDataEXT(device, shader, pDataSize, pData);
}

static inline void DispatchCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount,
                                             const VkShaderStageFlagBits* pStages, const VkShaderEXT* pShaders) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBindShadersEXT(commandBuffer, stageCount, pStages, pShaders);
}

static inline void DispatchCmdSetDepthClampRangeEXT(VkCommandBuffer commandBuffer, VkDepthClampModeEXT depthClampMode,
                                                    const VkDepthClampRangeEXT* pDepthClampRange) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetDepthClampRangeEXT(commandBuffer, depthClampMode, pDepthClampRange);
}

static inline VkResult DispatchGetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer,
                                                                uint32_t* pPropertiesCount, VkTilePropertiesQCOM* pProperties) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetFramebufferTilePropertiesQCOM(device, framebuffer, pPropertiesCount, pProperties);
}

static inline VkResult DispatchGetDynamicRenderingTilePropertiesQCOM(VkDevice device, const VkRenderingInfo* pRenderingInfo,
                                                                     VkTilePropertiesQCOM* pProperties) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetDynamicRenderingTilePropertiesQCOM(device, pRenderingInfo, pProperties);
}

static inline VkResult DispatchGetPhysicalDeviceCooperativeVectorPropertiesNV(VkPhysicalDevice physicalDevice,
                                                                              uint32_t* pPropertyCount,
                                                                              VkCooperativeVectorPropertiesNV* pProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceCooperativeVectorPropertiesNV(physicalDevice, pPropertyCount, pProperties);
}

static inline VkResult DispatchConvertCooperativeVectorMatrixNV(VkDevice device,
                                                                const VkConvertCooperativeVectorMatrixInfoNV* pInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->ConvertCooperativeVectorMatrixNV(device, pInfo);
}

static inline void DispatchCmdConvertCooperativeVectorMatrixNV(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                               const VkConvertCooperativeVectorMatrixInfoNV* pInfos) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdConvertCooperativeVectorMatrixNV(commandBuffer, infoCount, pInfos);
}

static inline VkResult DispatchSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain,
                                                     const VkLatencySleepModeInfoNV* pSleepModeInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->SetLatencySleepModeNV(device, swapchain, pSleepModeInfo);
}

static inline VkResult DispatchLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->LatencySleepNV(device, swapchain, pSleepInfo);
}

static inline void DispatchSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain,
                                              const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->SetLatencyMarkerNV(device, swapchain, pLatencyMarkerInfo);
}

static inline void DispatchGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain,
                                               VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetLatencyTimingsNV(device, swapchain, pLatencyMarkerInfo);
}

static inline void DispatchQueueNotifyOutOfBandNV(VkQueue queue, const VkOutOfBandQueueTypeInfoNV* pQueueTypeInfo) {
    auto dispatch = vvl::dispatch::GetData(queue);
    dispatch->QueueNotifyOutOfBandNV(queue, pQueueTypeInfo);
}

static inline void DispatchCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetAttachmentFeedbackLoopEnableEXT(commandBuffer, aspectMask);
}
#ifdef VK_USE_PLATFORM_SCREEN_QNX

static inline VkResult DispatchGetScreenBufferPropertiesQNX(VkDevice device, const struct _screen_buffer* buffer,
                                                            VkScreenBufferPropertiesQNX* pProperties) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetScreenBufferPropertiesQNX(device, buffer, pProperties);
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX

static inline void DispatchGetClusterAccelerationStructureBuildSizesNV(VkDevice device,
                                                                       const VkClusterAccelerationStructureInputInfoNV* pInfo,
                                                                       VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetClusterAccelerationStructureBuildSizesNV(device, pInfo, pSizeInfo);
}

static inline void DispatchCmdBuildClusterAccelerationStructureIndirectNV(
    VkCommandBuffer commandBuffer, const VkClusterAccelerationStructureCommandsInfoNV* pCommandInfos) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBuildClusterAccelerationStructureIndirectNV(commandBuffer, pCommandInfos);
}

static inline void DispatchGetPartitionedAccelerationStructuresBuildSizesNV(
    VkDevice device, const VkPartitionedAccelerationStructureInstancesInputNV* pInfo,
    VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetPartitionedAccelerationStructuresBuildSizesNV(device, pInfo, pSizeInfo);
}

static inline void DispatchCmdBuildPartitionedAccelerationStructuresNV(
    VkCommandBuffer commandBuffer, const VkBuildPartitionedAccelerationStructureInfoNV* pBuildInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBuildPartitionedAccelerationStructuresNV(commandBuffer, pBuildInfo);
}

static inline void DispatchGetGeneratedCommandsMemoryRequirementsEXT(VkDevice device,
                                                                     const VkGeneratedCommandsMemoryRequirementsInfoEXT* pInfo,
                                                                     VkMemoryRequirements2* pMemoryRequirements) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetGeneratedCommandsMemoryRequirementsEXT(device, pInfo, pMemoryRequirements);
}

static inline void DispatchCmdPreprocessGeneratedCommandsEXT(VkCommandBuffer commandBuffer,
                                                             const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo,
                                                             VkCommandBuffer stateCommandBuffer) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdPreprocessGeneratedCommandsEXT(commandBuffer, pGeneratedCommandsInfo, stateCommandBuffer);
}

static inline void DispatchCmdExecuteGeneratedCommandsEXT(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                          const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdExecuteGeneratedCommandsEXT(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
}

static inline VkResult DispatchCreateIndirectCommandsLayoutEXT(VkDevice device,
                                                               const VkIndirectCommandsLayoutCreateInfoEXT* pCreateInfo,
                                                               const VkAllocationCallbacks* pAllocator,
                                                               VkIndirectCommandsLayoutEXT* pIndirectCommandsLayout) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateIndirectCommandsLayoutEXT(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
}

static inline void DispatchDestroyIndirectCommandsLayoutEXT(VkDevice device, VkIndirectCommandsLayoutEXT indirectCommandsLayout,
                                                            const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyIndirectCommandsLayoutEXT(device, indirectCommandsLayout, pAllocator);
}

static inline VkResult DispatchCreateIndirectExecutionSetEXT(VkDevice device,
                                                             const VkIndirectExecutionSetCreateInfoEXT* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             VkIndirectExecutionSetEXT* pIndirectExecutionSet) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateIndirectExecutionSetEXT(device, pCreateInfo, pAllocator, pIndirectExecutionSet);
}

static inline void DispatchDestroyIndirectExecutionSetEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                          const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyIndirectExecutionSetEXT(device, indirectExecutionSet, pAllocator);
}

static inline void DispatchUpdateIndirectExecutionSetPipelineEXT(
    VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet, uint32_t executionSetWriteCount,
    const VkWriteIndirectExecutionSetPipelineEXT* pExecutionSetWrites) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->UpdateIndirectExecutionSetPipelineEXT(device, indirectExecutionSet, executionSetWriteCount, pExecutionSetWrites);
}

static inline void DispatchUpdateIndirectExecutionSetShaderEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                               uint32_t executionSetWriteCount,
                                                               const VkWriteIndirectExecutionSetShaderEXT* pExecutionSetWrites) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->UpdateIndirectExecutionSetShaderEXT(device, indirectExecutionSet, executionSetWriteCount, pExecutionSetWrites);
}

static inline VkResult DispatchGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(
    VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixFlexibleDimensionsPropertiesNV* pProperties) {
    auto dispatch = vvl::dispatch::GetData(physicalDevice);
    return dispatch->GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(physicalDevice, pPropertyCount, pProperties);
}
#ifdef VK_USE_PLATFORM_METAL_EXT

static inline VkResult DispatchGetMemoryMetalHandleEXT(VkDevice device, const VkMemoryGetMetalHandleInfoEXT* pGetMetalHandleInfo,
                                                       void** pHandle) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetMemoryMetalHandleEXT(device, pGetMetalHandleInfo, pHandle);
}

static inline VkResult DispatchGetMemoryMetalHandlePropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                 const void* pHandle,
                                                                 VkMemoryMetalHandlePropertiesEXT* pMemoryMetalHandleProperties) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetMemoryMetalHandlePropertiesEXT(device, handleType, pHandle, pMemoryMetalHandleProperties);
}
#endif  // VK_USE_PLATFORM_METAL_EXT

static inline VkResult DispatchCreateAccelerationStructureKHR(VkDevice device,
                                                              const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator,
                                                              VkAccelerationStructureKHR* pAccelerationStructure) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure);
}

static inline void DispatchDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                           const VkAllocationCallbacks* pAllocator) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->DestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
}

static inline void DispatchCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBuildAccelerationStructuresKHR(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
}

static inline void DispatchCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                     const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                                     const VkDeviceAddress* pIndirectDeviceAddresses,
                                                                     const uint32_t* pIndirectStrides,
                                                                     const uint32_t* const* ppMaxPrimitiveCounts) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdBuildAccelerationStructuresIndirectKHR(commandBuffer, infoCount, pInfos, pIndirectDeviceAddresses,
                                                        pIndirectStrides, ppMaxPrimitiveCounts);
}

static inline VkResult DispatchBuildAccelerationStructuresKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->BuildAccelerationStructuresKHR(device, deferredOperation, infoCount, pInfos, ppBuildRangeInfos);
}

static inline VkResult DispatchCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                            const VkCopyAccelerationStructureInfoKHR* pInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CopyAccelerationStructureKHR(device, deferredOperation, pInfo);
}

static inline VkResult DispatchCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CopyAccelerationStructureToMemoryKHR(device, deferredOperation, pInfo);
}

static inline VkResult DispatchCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CopyMemoryToAccelerationStructureKHR(device, deferredOperation, pInfo);
}

static inline VkResult DispatchWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                                        const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                        VkQueryType queryType, size_t dataSize, void* pData,
                                                                        size_t stride) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->WriteAccelerationStructuresPropertiesKHR(device, accelerationStructureCount, pAccelerationStructures,
                                                              queryType, dataSize, pData, stride);
}

static inline void DispatchCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                           const VkCopyAccelerationStructureInfoKHR* pInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
}

static inline void DispatchCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                                   const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
}

static inline void DispatchCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                   const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
}

static inline VkDeviceAddress DispatchGetAccelerationStructureDeviceAddressKHR(
    VkDevice device, const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetAccelerationStructureDeviceAddressKHR(device, pInfo);
}

static inline void DispatchCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer,
                                                                       uint32_t accelerationStructureCount,
                                                                       const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                       VkQueryType queryType, VkQueryPool queryPool,
                                                                       uint32_t firstQuery) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures,
                                                          queryType, queryPool, firstQuery);
}

static inline void DispatchGetDeviceAccelerationStructureCompatibilityKHR(VkDevice device,
                                                                          const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
                                                                          VkAccelerationStructureCompatibilityKHR* pCompatibility) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetDeviceAccelerationStructureCompatibilityKHR(device, pVersionInfo, pCompatibility);
}

static inline void DispatchGetAccelerationStructureBuildSizesKHR(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                                 const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
                                                                 const uint32_t* pMaxPrimitiveCounts,
                                                                 VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo) {
    auto dispatch = vvl::dispatch::GetData(device);
    dispatch->GetAccelerationStructureBuildSizesKHR(device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
}

static inline void DispatchCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                           const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                           const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                           const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                           const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                           uint32_t height, uint32_t depth) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable,
                              pCallableShaderBindingTable, width, height, depth);
}

static inline VkResult DispatchCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                            VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                            const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                            const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->CreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos,
                                                  pAllocator, pPipelines);
}

static inline VkResult DispatchGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline,
                                                                               uint32_t firstGroup, uint32_t groupCount,
                                                                               size_t dataSize, void* pData) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
}

static inline void DispatchCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                   const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                                   const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                                   const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                                   const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                                   VkDeviceAddress indirectDeviceAddress) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable,
                                      pCallableShaderBindingTable, indirectDeviceAddress);
}

static inline VkDeviceSize DispatchGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group,
                                                                        VkShaderGroupShaderKHR groupShader) {
    auto dispatch = vvl::dispatch::GetData(device);
    return dispatch->GetRayTracingShaderGroupStackSizeKHR(device, pipeline, group, groupShader);
}

static inline void DispatchCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdSetRayTracingPipelineStackSizeKHR(commandBuffer, pipelineStackSize);
}

static inline void DispatchCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                               uint32_t groupCountZ) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDrawMeshTasksEXT(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

static inline void DispatchCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       uint32_t drawCount, uint32_t stride) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDrawMeshTasksIndirectEXT(commandBuffer, buffer, offset, drawCount, stride);
}

static inline void DispatchCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                            uint32_t maxDrawCount, uint32_t stride) {
    auto dispatch = vvl::dispatch::GetData(commandBuffer);
    dispatch->CmdDrawMeshTasksIndirectCountEXT(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
// We make many internal dispatch calls to extended query functions which can depend on the API version

static inline void DispatchGetPhysicalDeviceFeatures2Helper(APIVersion api_version, VkPhysicalDevice physicalDevice,
                                                            VkPhysicalDeviceFeatures2* pFeatures) {
    if (api_version >= VK_API_VERSION_1_1) {
        return DispatchGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
    } else {
        return DispatchGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
    }
}

static inline void DispatchGetPhysicalDeviceProperties2Helper(APIVersion api_version, VkPhysicalDevice physicalDevice,
                                                              VkPhysicalDeviceProperties2* pProperties) {
    if (api_version >= VK_API_VERSION_1_1) {
        return DispatchGetPhysicalDeviceProperties2(physicalDevice, pProperties);
    } else {
        return DispatchGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
    }
}

static inline void DispatchGetPhysicalDeviceFormatProperties2Helper(APIVersion api_version, VkPhysicalDevice physicalDevice,
                                                                    VkFormat format, VkFormatProperties2* pFormatProperties) {
    if (api_version >= VK_API_VERSION_1_1) {
        return DispatchGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
    } else {
        return DispatchGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
    }
}

static inline VkResult DispatchGetPhysicalDeviceImageFormatProperties2Helper(
    APIVersion api_version, VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
    VkImageFormatProperties2* pImageFormatProperties) {
    if (api_version >= VK_API_VERSION_1_1) {
        return DispatchGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    } else {
        return DispatchGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    }
}

static inline void DispatchGetPhysicalDeviceQueueFamilyProperties2Helper(APIVersion api_version, VkPhysicalDevice physicalDevice,
                                                                         uint32_t* pQueueFamilyPropertyCount,
                                                                         VkQueueFamilyProperties2* pQueueFamilyProperties) {
    if (api_version >= VK_API_VERSION_1_1) {
        return DispatchGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    } else {
        return DispatchGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount,
                                                                  pQueueFamilyProperties);
    }
}

static inline void DispatchGetPhysicalDeviceMemoryProperties2Helper(APIVersion api_version, VkPhysicalDevice physicalDevice,
                                                                    VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
    if (api_version >= VK_API_VERSION_1_1) {
        return DispatchGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
    } else {
        return DispatchGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
    }
}

static inline void DispatchGetPhysicalDeviceSparseImageFormatProperties2Helper(
    APIVersion api_version, VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties) {
    if (api_version >= VK_API_VERSION_1_1) {
        return DispatchGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    } else {
        return DispatchGetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    }
}

static inline void DispatchGetPhysicalDeviceExternalSemaphorePropertiesHelper(
    APIVersion api_version, VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {
    if (api_version >= VK_API_VERSION_1_1) {
        return DispatchGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo,
                                                                    pExternalSemaphoreProperties);
    } else {
        return DispatchGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo,
                                                                       pExternalSemaphoreProperties);
    }
}

static inline void DispatchGetPhysicalDeviceExternalFencePropertiesHelper(
    APIVersion api_version, VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
    VkExternalFenceProperties* pExternalFenceProperties) {
    if (api_version >= VK_API_VERSION_1_1) {
        return DispatchGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    } else {
        return DispatchGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    }
}

static inline void DispatchGetPhysicalDeviceExternalBufferPropertiesHelper(
    APIVersion api_version, VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
    VkExternalBufferProperties* pExternalBufferProperties) {
    if (api_version >= VK_API_VERSION_1_1) {
        return DispatchGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    } else {
        return DispatchGetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    }
}

static inline void DispatchGetImageMemoryRequirements2Helper(APIVersion api_version, VkDevice device,
                                                             const VkImageMemoryRequirementsInfo2* pInfo,
                                                             VkMemoryRequirements2* pMemoryRequirements) {
    if (api_version >= VK_API_VERSION_1_1) {
        return DispatchGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
    } else {
        return DispatchGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    }
}

static inline void DispatchGetBufferMemoryRequirements2Helper(APIVersion api_version, VkDevice device,
                                                              const VkBufferMemoryRequirementsInfo2* pInfo,
                                                              VkMemoryRequirements2* pMemoryRequirements) {
    if (api_version >= VK_API_VERSION_1_1) {
        return DispatchGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
    } else {
        return DispatchGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    }
}

static inline void DispatchGetImageSparseMemoryRequirements2Helper(APIVersion api_version, VkDevice device,
                                                                   const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                                   uint32_t* pSparseMemoryRequirementCount,
                                                                   VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    if (api_version >= VK_API_VERSION_1_1) {
        return DispatchGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    } else {
        return DispatchGetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount,
                                                            pSparseMemoryRequirements);
    }
}

// NOLINTEND
