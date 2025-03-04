/*
 *
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
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
 *
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Charles Giessen <charles@lunarg.com>
 */

#include "gpa_helper.h"

#include <string.h>

#include "debug_utils.h"
#include "unknown_function_handling.h"
#include "wsi.h"

void *trampoline_get_proc_addr(struct loader_instance *inst, const char *funcName) {
    // Don't include or check global functions
    if (!strcmp(funcName, "vkGetInstanceProcAddr")) return vkGetInstanceProcAddr;
    if (!strcmp(funcName, "vkDestroyInstance")) return vkDestroyInstance;
    if (!strcmp(funcName, "vkEnumeratePhysicalDevices")) return vkEnumeratePhysicalDevices;
    if (!strcmp(funcName, "vkGetPhysicalDeviceFeatures")) return vkGetPhysicalDeviceFeatures;
    if (!strcmp(funcName, "vkGetPhysicalDeviceFormatProperties")) return vkGetPhysicalDeviceFormatProperties;
    if (!strcmp(funcName, "vkGetPhysicalDeviceImageFormatProperties")) return vkGetPhysicalDeviceImageFormatProperties;
    if (!strcmp(funcName, "vkGetPhysicalDeviceSparseImageFormatProperties")) return vkGetPhysicalDeviceSparseImageFormatProperties;
    if (!strcmp(funcName, "vkGetPhysicalDeviceProperties")) return vkGetPhysicalDeviceProperties;
    if (!strcmp(funcName, "vkGetPhysicalDeviceQueueFamilyProperties")) return vkGetPhysicalDeviceQueueFamilyProperties;
    if (!strcmp(funcName, "vkGetPhysicalDeviceMemoryProperties")) return vkGetPhysicalDeviceMemoryProperties;
    if (!strcmp(funcName, "vkEnumerateDeviceLayerProperties")) return vkEnumerateDeviceLayerProperties;
    if (!strcmp(funcName, "vkEnumerateDeviceExtensionProperties")) return vkEnumerateDeviceExtensionProperties;
    if (!strcmp(funcName, "vkCreateDevice")) return vkCreateDevice;
    if (!strcmp(funcName, "vkGetDeviceProcAddr")) return vkGetDeviceProcAddr;
    if (!strcmp(funcName, "vkDestroyDevice")) return vkDestroyDevice;
    if (!strcmp(funcName, "vkGetDeviceQueue")) return vkGetDeviceQueue;
    if (!strcmp(funcName, "vkQueueSubmit")) return vkQueueSubmit;
    if (!strcmp(funcName, "vkQueueWaitIdle")) return vkQueueWaitIdle;
    if (!strcmp(funcName, "vkDeviceWaitIdle")) return vkDeviceWaitIdle;
    if (!strcmp(funcName, "vkAllocateMemory")) return vkAllocateMemory;
    if (!strcmp(funcName, "vkFreeMemory")) return vkFreeMemory;
    if (!strcmp(funcName, "vkMapMemory")) return vkMapMemory;
    if (!strcmp(funcName, "vkUnmapMemory")) return vkUnmapMemory;
    if (!strcmp(funcName, "vkFlushMappedMemoryRanges")) return vkFlushMappedMemoryRanges;
    if (!strcmp(funcName, "vkInvalidateMappedMemoryRanges")) return vkInvalidateMappedMemoryRanges;
    if (!strcmp(funcName, "vkGetDeviceMemoryCommitment")) return vkGetDeviceMemoryCommitment;
    if (!strcmp(funcName, "vkGetImageSparseMemoryRequirements")) return vkGetImageSparseMemoryRequirements;
    if (!strcmp(funcName, "vkGetImageMemoryRequirements")) return vkGetImageMemoryRequirements;
    if (!strcmp(funcName, "vkGetBufferMemoryRequirements")) return vkGetBufferMemoryRequirements;
    if (!strcmp(funcName, "vkBindImageMemory")) return vkBindImageMemory;
    if (!strcmp(funcName, "vkBindBufferMemory")) return vkBindBufferMemory;
    if (!strcmp(funcName, "vkQueueBindSparse")) return vkQueueBindSparse;
    if (!strcmp(funcName, "vkCreateFence")) return vkCreateFence;
    if (!strcmp(funcName, "vkDestroyFence")) return vkDestroyFence;
    if (!strcmp(funcName, "vkGetFenceStatus")) return vkGetFenceStatus;
    if (!strcmp(funcName, "vkResetFences")) return vkResetFences;
    if (!strcmp(funcName, "vkWaitForFences")) return vkWaitForFences;
    if (!strcmp(funcName, "vkCreateSemaphore")) return vkCreateSemaphore;
    if (!strcmp(funcName, "vkDestroySemaphore")) return vkDestroySemaphore;
    if (!strcmp(funcName, "vkCreateEvent")) return vkCreateEvent;
    if (!strcmp(funcName, "vkDestroyEvent")) return vkDestroyEvent;
    if (!strcmp(funcName, "vkGetEventStatus")) return vkGetEventStatus;
    if (!strcmp(funcName, "vkSetEvent")) return vkSetEvent;
    if (!strcmp(funcName, "vkResetEvent")) return vkResetEvent;
    if (!strcmp(funcName, "vkCreateQueryPool")) return vkCreateQueryPool;
    if (!strcmp(funcName, "vkDestroyQueryPool")) return vkDestroyQueryPool;
    if (!strcmp(funcName, "vkGetQueryPoolResults")) return vkGetQueryPoolResults;
    if (!strcmp(funcName, "vkCreateBuffer")) return vkCreateBuffer;
    if (!strcmp(funcName, "vkDestroyBuffer")) return vkDestroyBuffer;
    if (!strcmp(funcName, "vkCreateBufferView")) return vkCreateBufferView;
    if (!strcmp(funcName, "vkDestroyBufferView")) return vkDestroyBufferView;
    if (!strcmp(funcName, "vkCreateImage")) return vkCreateImage;
    if (!strcmp(funcName, "vkDestroyImage")) return vkDestroyImage;
    if (!strcmp(funcName, "vkGetImageSubresourceLayout")) return vkGetImageSubresourceLayout;
    if (!strcmp(funcName, "vkCreateImageView")) return vkCreateImageView;
    if (!strcmp(funcName, "vkDestroyImageView")) return vkDestroyImageView;
    if (!strcmp(funcName, "vkCreateShaderModule")) return vkCreateShaderModule;
    if (!strcmp(funcName, "vkDestroyShaderModule")) return vkDestroyShaderModule;
    if (!strcmp(funcName, "vkCreatePipelineCache")) return vkCreatePipelineCache;
    if (!strcmp(funcName, "vkDestroyPipelineCache")) return vkDestroyPipelineCache;
    if (!strcmp(funcName, "vkGetPipelineCacheData")) return vkGetPipelineCacheData;
    if (!strcmp(funcName, "vkMergePipelineCaches")) return vkMergePipelineCaches;
    if (!strcmp(funcName, "vkCreateGraphicsPipelines")) return vkCreateGraphicsPipelines;
    if (!strcmp(funcName, "vkCreateComputePipelines")) return vkCreateComputePipelines;
    if (!strcmp(funcName, "vkDestroyPipeline")) return vkDestroyPipeline;
    if (!strcmp(funcName, "vkCreatePipelineLayout")) return vkCreatePipelineLayout;
    if (!strcmp(funcName, "vkDestroyPipelineLayout")) return vkDestroyPipelineLayout;
    if (!strcmp(funcName, "vkCreateSampler")) return vkCreateSampler;
    if (!strcmp(funcName, "vkDestroySampler")) return vkDestroySampler;
    if (!strcmp(funcName, "vkCreateDescriptorSetLayout")) return vkCreateDescriptorSetLayout;
    if (!strcmp(funcName, "vkDestroyDescriptorSetLayout")) return vkDestroyDescriptorSetLayout;
    if (!strcmp(funcName, "vkCreateDescriptorPool")) return vkCreateDescriptorPool;
    if (!strcmp(funcName, "vkDestroyDescriptorPool")) return vkDestroyDescriptorPool;
    if (!strcmp(funcName, "vkResetDescriptorPool")) return vkResetDescriptorPool;
    if (!strcmp(funcName, "vkAllocateDescriptorSets")) return vkAllocateDescriptorSets;
    if (!strcmp(funcName, "vkFreeDescriptorSets")) return vkFreeDescriptorSets;
    if (!strcmp(funcName, "vkUpdateDescriptorSets")) return vkUpdateDescriptorSets;
    if (!strcmp(funcName, "vkCreateFramebuffer")) return vkCreateFramebuffer;
    if (!strcmp(funcName, "vkDestroyFramebuffer")) return vkDestroyFramebuffer;
    if (!strcmp(funcName, "vkCreateRenderPass")) return vkCreateRenderPass;
    if (!strcmp(funcName, "vkDestroyRenderPass")) return vkDestroyRenderPass;
    if (!strcmp(funcName, "vkGetRenderAreaGranularity")) return vkGetRenderAreaGranularity;
    if (!strcmp(funcName, "vkCreateCommandPool")) return vkCreateCommandPool;
    if (!strcmp(funcName, "vkDestroyCommandPool")) return vkDestroyCommandPool;
    if (!strcmp(funcName, "vkResetCommandPool")) return vkResetCommandPool;
    if (!strcmp(funcName, "vkAllocateCommandBuffers")) return vkAllocateCommandBuffers;
    if (!strcmp(funcName, "vkFreeCommandBuffers")) return vkFreeCommandBuffers;
    if (!strcmp(funcName, "vkBeginCommandBuffer")) return vkBeginCommandBuffer;
    if (!strcmp(funcName, "vkEndCommandBuffer")) return vkEndCommandBuffer;
    if (!strcmp(funcName, "vkResetCommandBuffer")) return vkResetCommandBuffer;
    if (!strcmp(funcName, "vkCmdBindPipeline")) return vkCmdBindPipeline;
    if (!strcmp(funcName, "vkCmdBindDescriptorSets")) return vkCmdBindDescriptorSets;
    if (!strcmp(funcName, "vkCmdBindVertexBuffers")) return vkCmdBindVertexBuffers;
    if (!strcmp(funcName, "vkCmdBindIndexBuffer")) return vkCmdBindIndexBuffer;
    if (!strcmp(funcName, "vkCmdSetViewport")) return vkCmdSetViewport;
    if (!strcmp(funcName, "vkCmdSetScissor")) return vkCmdSetScissor;
    if (!strcmp(funcName, "vkCmdSetLineWidth")) return vkCmdSetLineWidth;
    if (!strcmp(funcName, "vkCmdSetDepthBias")) return vkCmdSetDepthBias;
    if (!strcmp(funcName, "vkCmdSetBlendConstants")) return vkCmdSetBlendConstants;
    if (!strcmp(funcName, "vkCmdSetDepthBounds")) return vkCmdSetDepthBounds;
    if (!strcmp(funcName, "vkCmdSetStencilCompareMask")) return vkCmdSetStencilCompareMask;
    if (!strcmp(funcName, "vkCmdSetStencilWriteMask")) return vkCmdSetStencilWriteMask;
    if (!strcmp(funcName, "vkCmdSetStencilReference")) return vkCmdSetStencilReference;
    if (!strcmp(funcName, "vkCmdDraw")) return vkCmdDraw;
    if (!strcmp(funcName, "vkCmdDrawIndexed")) return vkCmdDrawIndexed;
    if (!strcmp(funcName, "vkCmdDrawIndirect")) return vkCmdDrawIndirect;
    if (!strcmp(funcName, "vkCmdDrawIndexedIndirect")) return vkCmdDrawIndexedIndirect;
    if (!strcmp(funcName, "vkCmdDispatch")) return vkCmdDispatch;
    if (!strcmp(funcName, "vkCmdDispatchIndirect")) return vkCmdDispatchIndirect;
    if (!strcmp(funcName, "vkCmdCopyBuffer")) return vkCmdCopyBuffer;
    if (!strcmp(funcName, "vkCmdCopyImage")) return vkCmdCopyImage;
    if (!strcmp(funcName, "vkCmdBlitImage")) return vkCmdBlitImage;
    if (!strcmp(funcName, "vkCmdCopyBufferToImage")) return vkCmdCopyBufferToImage;
    if (!strcmp(funcName, "vkCmdCopyImageToBuffer")) return vkCmdCopyImageToBuffer;
    if (!strcmp(funcName, "vkCmdUpdateBuffer")) return vkCmdUpdateBuffer;
    if (!strcmp(funcName, "vkCmdFillBuffer")) return vkCmdFillBuffer;
    if (!strcmp(funcName, "vkCmdClearColorImage")) return vkCmdClearColorImage;
    if (!strcmp(funcName, "vkCmdClearDepthStencilImage")) return vkCmdClearDepthStencilImage;
    if (!strcmp(funcName, "vkCmdClearAttachments")) return vkCmdClearAttachments;
    if (!strcmp(funcName, "vkCmdResolveImage")) return vkCmdResolveImage;
    if (!strcmp(funcName, "vkCmdSetEvent")) return vkCmdSetEvent;
    if (!strcmp(funcName, "vkCmdResetEvent")) return vkCmdResetEvent;
    if (!strcmp(funcName, "vkCmdWaitEvents")) return vkCmdWaitEvents;
    if (!strcmp(funcName, "vkCmdPipelineBarrier")) return vkCmdPipelineBarrier;
    if (!strcmp(funcName, "vkCmdBeginQuery")) return vkCmdBeginQuery;
    if (!strcmp(funcName, "vkCmdEndQuery")) return vkCmdEndQuery;
    if (!strcmp(funcName, "vkCmdResetQueryPool")) return vkCmdResetQueryPool;
    if (!strcmp(funcName, "vkCmdWriteTimestamp")) return vkCmdWriteTimestamp;
    if (!strcmp(funcName, "vkCmdCopyQueryPoolResults")) return vkCmdCopyQueryPoolResults;
    if (!strcmp(funcName, "vkCmdPushConstants")) return vkCmdPushConstants;
    if (!strcmp(funcName, "vkCmdBeginRenderPass")) return vkCmdBeginRenderPass;
    if (!strcmp(funcName, "vkCmdNextSubpass")) return vkCmdNextSubpass;
    if (!strcmp(funcName, "vkCmdEndRenderPass")) return vkCmdEndRenderPass;
    if (!strcmp(funcName, "vkCmdExecuteCommands")) return vkCmdExecuteCommands;

    // Core 1.1 functions
    if (!strcmp(funcName, "vkEnumeratePhysicalDeviceGroups")) return vkEnumeratePhysicalDeviceGroups;
    if (!strcmp(funcName, "vkGetPhysicalDeviceFeatures2")) return vkGetPhysicalDeviceFeatures2;
    if (!strcmp(funcName, "vkGetPhysicalDeviceProperties2")) return vkGetPhysicalDeviceProperties2;
    if (!strcmp(funcName, "vkGetPhysicalDeviceFormatProperties2")) return vkGetPhysicalDeviceFormatProperties2;
    if (!strcmp(funcName, "vkGetPhysicalDeviceImageFormatProperties2")) return vkGetPhysicalDeviceImageFormatProperties2;
    if (!strcmp(funcName, "vkGetPhysicalDeviceQueueFamilyProperties2")) return vkGetPhysicalDeviceQueueFamilyProperties2;
    if (!strcmp(funcName, "vkGetPhysicalDeviceMemoryProperties2")) return vkGetPhysicalDeviceMemoryProperties2;
    if (!strcmp(funcName, "vkGetPhysicalDeviceSparseImageFormatProperties2"))
        return vkGetPhysicalDeviceSparseImageFormatProperties2;
    if (!strcmp(funcName, "vkGetPhysicalDeviceExternalBufferProperties")) return vkGetPhysicalDeviceExternalBufferProperties;
    if (!strcmp(funcName, "vkGetPhysicalDeviceExternalSemaphoreProperties")) return vkGetPhysicalDeviceExternalSemaphoreProperties;
    if (!strcmp(funcName, "vkGetPhysicalDeviceExternalFenceProperties")) return vkGetPhysicalDeviceExternalFenceProperties;
    if (!strcmp(funcName, "vkBindBufferMemory2")) return vkBindBufferMemory2;
    if (!strcmp(funcName, "vkBindImageMemory2")) return vkBindImageMemory2;
    if (!strcmp(funcName, "vkGetDeviceGroupPeerMemoryFeatures")) return vkGetDeviceGroupPeerMemoryFeatures;
    if (!strcmp(funcName, "vkCmdSetDeviceMask")) return vkCmdSetDeviceMask;
    if (!strcmp(funcName, "vkCmdDispatchBase")) return vkCmdDispatchBase;
    if (!strcmp(funcName, "vkGetImageMemoryRequirements2")) return vkGetImageMemoryRequirements2;
    if (!strcmp(funcName, "vkTrimCommandPool")) return vkTrimCommandPool;
    if (!strcmp(funcName, "vkGetDeviceQueue2")) return vkGetDeviceQueue2;
    if (!strcmp(funcName, "vkCreateSamplerYcbcrConversion")) return vkCreateSamplerYcbcrConversion;
    if (!strcmp(funcName, "vkDestroySamplerYcbcrConversion")) return vkDestroySamplerYcbcrConversion;
    if (!strcmp(funcName, "vkGetDescriptorSetLayoutSupport")) return vkGetDescriptorSetLayoutSupport;
    if (!strcmp(funcName, "vkCreateDescriptorUpdateTemplate")) return vkCreateDescriptorUpdateTemplate;
    if (!strcmp(funcName, "vkDestroyDescriptorUpdateTemplate")) return vkDestroyDescriptorUpdateTemplate;
    if (!strcmp(funcName, "vkUpdateDescriptorSetWithTemplate")) return vkUpdateDescriptorSetWithTemplate;
    if (!strcmp(funcName, "vkGetImageSparseMemoryRequirements2")) return vkGetImageSparseMemoryRequirements2;
    if (!strcmp(funcName, "vkGetBufferMemoryRequirements2")) return vkGetBufferMemoryRequirements2;

    // Core 1.2 functions
    if (!strcmp(funcName, "vkCreateRenderPass2")) return vkCreateRenderPass2;
    if (!strcmp(funcName, "vkCmdBeginRenderPass2")) return vkCmdBeginRenderPass2;
    if (!strcmp(funcName, "vkCmdNextSubpass2")) return vkCmdNextSubpass2;
    if (!strcmp(funcName, "vkCmdEndRenderPass2")) return vkCmdEndRenderPass2;
    if (!strcmp(funcName, "vkCmdDrawIndirectCount")) return vkCmdDrawIndirectCount;
    if (!strcmp(funcName, "vkCmdDrawIndexedIndirectCount")) return vkCmdDrawIndexedIndirectCount;
    if (!strcmp(funcName, "vkGetSemaphoreCounterValue")) return vkGetSemaphoreCounterValue;
    if (!strcmp(funcName, "vkWaitSemaphores")) return vkWaitSemaphores;
    if (!strcmp(funcName, "vkSignalSemaphore")) return vkSignalSemaphore;
    if (!strcmp(funcName, "vkGetBufferDeviceAddress")) return vkGetBufferDeviceAddress;
    if (!strcmp(funcName, "vkGetBufferOpaqueCaptureAddress")) return vkGetBufferOpaqueCaptureAddress;
    if (!strcmp(funcName, "vkGetDeviceMemoryOpaqueCaptureAddress")) return vkGetDeviceMemoryOpaqueCaptureAddress;
    if (!strcmp(funcName, "vkResetQueryPool")) return vkResetQueryPool;

    // Core 1.3 functions
    if (!strcmp(funcName, "vkGetPhysicalDeviceToolProperties")) return vkGetPhysicalDeviceToolProperties;
    if (!strcmp(funcName, "vkCreatePrivateDataSlot")) return vkCreatePrivateDataSlot;
    if (!strcmp(funcName, "vkDestroyPrivateDataSlot")) return vkDestroyPrivateDataSlot;
    if (!strcmp(funcName, "vkSetPrivateData")) return vkSetPrivateData;
    if (!strcmp(funcName, "vkGetPrivateData")) return vkGetPrivateData;
    if (!strcmp(funcName, "vkCmdSetEvent2")) return vkCmdSetEvent2;
    if (!strcmp(funcName, "vkCmdResetEvent2")) return vkCmdResetEvent2;
    if (!strcmp(funcName, "vkCmdWaitEvents2")) return vkCmdWaitEvents2;
    if (!strcmp(funcName, "vkCmdPipelineBarrier2")) return vkCmdPipelineBarrier2;
    if (!strcmp(funcName, "vkCmdWriteTimestamp2")) return vkCmdWriteTimestamp2;
    if (!strcmp(funcName, "vkQueueSubmit2")) return vkQueueSubmit2;
    if (!strcmp(funcName, "vkCmdCopyBuffer2")) return vkCmdCopyBuffer2;
    if (!strcmp(funcName, "vkCmdCopyImage2")) return vkCmdCopyImage2;
    if (!strcmp(funcName, "vkCmdCopyBufferToImage2")) return vkCmdCopyBufferToImage2;
    if (!strcmp(funcName, "vkCmdCopyImageToBuffer2")) return vkCmdCopyImageToBuffer2;
    if (!strcmp(funcName, "vkCmdBlitImage2")) return vkCmdBlitImage2;
    if (!strcmp(funcName, "vkCmdResolveImage2")) return vkCmdResolveImage2;
    if (!strcmp(funcName, "vkCmdBeginRendering")) return vkCmdBeginRendering;
    if (!strcmp(funcName, "vkCmdEndRendering")) return vkCmdEndRendering;
    if (!strcmp(funcName, "vkCmdSetCullMode")) return vkCmdSetCullMode;
    if (!strcmp(funcName, "vkCmdSetFrontFace")) return vkCmdSetFrontFace;
    if (!strcmp(funcName, "vkCmdSetPrimitiveTopology")) return vkCmdSetPrimitiveTopology;
    if (!strcmp(funcName, "vkCmdSetViewportWithCount")) return vkCmdSetViewportWithCount;
    if (!strcmp(funcName, "vkCmdSetScissorWithCount")) return vkCmdSetScissorWithCount;
    if (!strcmp(funcName, "vkCmdBindVertexBuffers2")) return vkCmdBindVertexBuffers2;
    if (!strcmp(funcName, "vkCmdSetDepthTestEnable")) return vkCmdSetDepthTestEnable;
    if (!strcmp(funcName, "vkCmdSetDepthWriteEnable")) return vkCmdSetDepthWriteEnable;
    if (!strcmp(funcName, "vkCmdSetDepthCompareOp")) return vkCmdSetDepthCompareOp;
    if (!strcmp(funcName, "vkCmdSetDepthBoundsTestEnable")) return vkCmdSetDepthBoundsTestEnable;
    if (!strcmp(funcName, "vkCmdSetStencilTestEnable")) return vkCmdSetStencilTestEnable;
    if (!strcmp(funcName, "vkCmdSetStencilOp")) return vkCmdSetStencilOp;
    if (!strcmp(funcName, "vkCmdSetRasterizerDiscardEnable")) return vkCmdSetRasterizerDiscardEnable;
    if (!strcmp(funcName, "vkCmdSetDepthBiasEnable")) return vkCmdSetDepthBiasEnable;
    if (!strcmp(funcName, "vkCmdSetPrimitiveRestartEnable")) return vkCmdSetPrimitiveRestartEnable;
    if (!strcmp(funcName, "vkGetDeviceBufferMemoryRequirements")) return vkGetDeviceBufferMemoryRequirements;
    if (!strcmp(funcName, "vkGetDeviceImageMemoryRequirements")) return vkGetDeviceImageMemoryRequirements;
    if (!strcmp(funcName, "vkGetDeviceImageSparseMemoryRequirements")) return vkGetDeviceImageSparseMemoryRequirements;

    // Core 1.4 functions
    if (!strcmp(funcName, "vkCmdSetLineStipple")) return vkCmdSetLineStipple;
    if (!strcmp(funcName, "vkMapMemory2")) return vkMapMemory2;
    if (!strcmp(funcName, "vkUnmapMemory2")) return vkUnmapMemory2;
    if (!strcmp(funcName, "vkCmdBindIndexBuffer2")) return vkCmdBindIndexBuffer2;
    if (!strcmp(funcName, "vkGetRenderingAreaGranularity")) return vkGetRenderingAreaGranularity;
    if (!strcmp(funcName, "vkGetDeviceImageSubresourceLayout")) return vkGetDeviceImageSubresourceLayout;
    if (!strcmp(funcName, "vkGetImageSubresourceLayout2")) return vkGetImageSubresourceLayout2;
    if (!strcmp(funcName, "vkCmdPushDescriptorSet")) return vkCmdPushDescriptorSet;
    if (!strcmp(funcName, "vkCmdPushDescriptorSetWithTemplate")) return vkCmdPushDescriptorSetWithTemplate;
    if (!strcmp(funcName, "vkCmdSetRenderingAttachmentLocations")) return vkCmdSetRenderingAttachmentLocations;
    if (!strcmp(funcName, "vkCmdSetRenderingInputAttachmentIndices")) return vkCmdSetRenderingInputAttachmentIndices;
    if (!strcmp(funcName, "vkCmdBindDescriptorSets2")) return vkCmdBindDescriptorSets2;
    if (!strcmp(funcName, "vkCmdPushConstants2")) return vkCmdPushConstants2;
    if (!strcmp(funcName, "vkCmdPushDescriptorSet2")) return vkCmdPushDescriptorSet2;
    if (!strcmp(funcName, "vkCmdPushDescriptorSetWithTemplate2")) return vkCmdPushDescriptorSetWithTemplate2;
    if (!strcmp(funcName, "vkCopyMemoryToImage")) return vkCopyMemoryToImage;
    if (!strcmp(funcName, "vkCopyImageToMemory")) return vkCopyImageToMemory;
    if (!strcmp(funcName, "vkCopyImageToImage")) return vkCopyImageToImage;
    if (!strcmp(funcName, "vkTransitionImageLayout")) return vkTransitionImageLayout;

    // Instance extensions
    void *addr;
    if (debug_extensions_InstanceGpa(inst, funcName, &addr)) return addr;

    if (wsi_swapchain_instance_gpa(inst, funcName, &addr)) return addr;

    if (extension_instance_gpa(inst, funcName, &addr)) return addr;

    // Unknown physical device extensions
    addr = loader_phys_dev_ext_gpa_tramp(inst, funcName);
    if (NULL != addr) return addr;

    // Unknown device extensions
    addr = loader_dev_ext_gpa_tramp(inst, funcName);
    return addr;
}

void *globalGetProcAddr(const char *name) {
    if (!name || name[0] != 'v' || name[1] != 'k') return NULL;

    name += 2;
    if (!strcmp(name, "CreateInstance")) return vkCreateInstance;
    if (!strcmp(name, "EnumerateInstanceExtensionProperties")) return vkEnumerateInstanceExtensionProperties;
    if (!strcmp(name, "EnumerateInstanceLayerProperties")) return vkEnumerateInstanceLayerProperties;
    if (!strcmp(name, "EnumerateInstanceVersion")) return vkEnumerateInstanceVersion;

    return NULL;
}
