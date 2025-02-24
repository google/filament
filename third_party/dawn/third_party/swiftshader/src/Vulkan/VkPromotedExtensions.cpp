// Copyright 2018 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// This file contains function definitions for extensions Vulkan 1.1 and 1.2
// promoted into the core API. (See spec Appendix D: Core Revisions)

// The current list of promoted extensions is:
// VK_KHR_16bit_storage (no functions in this extension)
// VK_KHR_bind_memory2
// VK_KHR_dedicated_allocation (no functions in this extension)
// VK_KHR_descriptor_update_template
// VK_KHR_device_group
// VK_KHR_device_group_creation
// VK_KHR_external_fence (no functions in this extension)
// VK_KHR_external_fence_capabilities
// VK_KHR_external_memory (no functions in this extension)
// VK_KHR_external_memory_capabilities
// VK_KHR_external_semaphore (no functions in this extension)
// VK_KHR_external_semaphore_capabilities
// VK_KHR_get_memory_requirements2
// VK_KHR_get_physical_device_properties2
// VK_KHR_maintenance1
// VK_KHR_maintenance2 (no functions in this extension)
// VK_KHR_maintenance3
// VK_KHR_multiview (no functions in this extension)
// VK_KHR_relaxed_block_layout (no functions in this extension)
// VK_KHR_sampler_ycbcr_conversion
// VK_KHR_shader_draw_parameters (no functions in this extension)
// VK_KHR_storage_buffer_storage_class (no functions in this extension)
// VK_KHR_variable_pointers (no functions in this extension)
//
// 1.2 Extensions:
// VK_KHR_create_renderpass2
//
// 1.3 Extensions
// VK_KHR_copy_commands2
// VK_KHR_dynamic_rendering
// VK_KHR_format_feature_flags2 (no functions in this extension)
// VK_KHR_maintenance4
// VK_KHR_shader_integer_dot_product (no functions in this extension)
// VK_KHR_shader_non_semantic_info (no functions in this extension)
// VK_KHR_shader_terminate_invocation (no functions in this extension)
// VK_KHR_synchronization2
// VK_KHR_zero_initialize_workgroup_memory (no functions in this extension)
// VK_EXT_4444_formats (no functions in this extension)
// VK_EXT_extended_dynamic_state
// VK_EXT_extended_dynamic_state2 (partial promotion, VKCmdSetLogicOpEXT and VKCmdSetPatchControlPointsEXT are not promoted)
// VK_EXT_image_robustness (no functions in this extension)
// VK_EXT_inline_uniform_block (no functions in this extension)
// VK_EXT_pipeline_creation_cache_control (no functions in this extension)
// VK_EXT_pipeline_creation_feedback (no functions in this extension)
// VK_EXT_private_data
// VK_EXT_shader_demote_to_helper_invocation (no functions in this extension)
// VK_EXT_subgroup_size_control (no functions in this extension)
// VK_EXT_texel_buffer_alignment (no functions in this extension)
// VK_EXT_texture_compression_astc_hdr (no functions in this extension)
// VK_EXT_tooling_info
// VK_EXT_ycbcr_2plane_444_formats (no functions in this extension)

#include "Vulkan/VulkanPlatform.hpp"

extern "C" {

// VK_KHR_bind_memory2
VKAPI_ATTR VkResult VKAPI_CALL vkBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo *pBindInfos)
{
	return vkBindBufferMemory2(device, bindInfoCount, pBindInfos);
}

VKAPI_ATTR VkResult VKAPI_CALL vkBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos)
{
	return vkBindImageMemory2(device, bindInfoCount, pBindInfos);
}

// VK_KHR_descriptor_update_template
VKAPI_ATTR VkResult VKAPI_CALL vkCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate)
{
	return vkCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks *pAllocator)
{
	vkDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void *pData)
{
	vkUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
}

// VK_KHR_device_group
VKAPI_ATTR void VKAPI_CALL vkGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags *pPeerMemoryFeatures)
{
	vkGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask)
{
	vkCmdSetDeviceMask(commandBuffer, deviceMask);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	vkCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}

// VK_KHR_device_group_creation
VKAPI_ATTR VkResult VKAPI_CALL vkEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t *pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties)
{
	return vkEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
}

// VK_KHR_external_fence_capabilities
VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo *pExternalFenceInfo, VkExternalFenceProperties *pExternalFenceProperties)
{
	vkGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
}

// VK_KHR_external_memory_capabilities
VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceExternalBufferPropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo, VkExternalBufferProperties *pExternalBufferProperties)
{
	vkGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
}

// VK_KHR_external_semaphore_capabilities
VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo, VkExternalSemaphoreProperties *pExternalSemaphoreProperties)
{
	vkGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
}

// VK_KHR_get_memory_requirements2
VKAPI_ATTR void VKAPI_CALL vkGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo, VkMemoryRequirements2 *pMemoryRequirements)
{
	vkGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2 *pInfo, VkMemoryRequirements2 *pMemoryRequirements)
{
	vkGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo, uint32_t *pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements)
{
	vkGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

// VK_KHR_get_physical_device_properties2
VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2 *pFeatures)
{
	vkGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2 *pProperties)
{
	vkGetPhysicalDeviceProperties2(physicalDevice, pProperties);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2 *pFormatProperties)
{
	vkGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo, VkImageFormatProperties2 *pImageFormatProperties)
{
	return vkGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties2 *pQueueFamilyProperties)
{
	vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2 *pMemoryProperties)
{
	vkGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceSparseImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2 *pFormatInfo, uint32_t *pPropertyCount, VkSparseImageFormatProperties2 *pProperties)
{
	vkGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
}

// VK_KHR_maintenance1
VKAPI_ATTR void VKAPI_CALL vkTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags)
{
	vkTrimCommandPool(device, commandPool, flags);
}

// VK_KHR_maintenance3
VKAPI_ATTR void VKAPI_CALL vkGetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo, VkDescriptorSetLayoutSupport *pSupport)
{
	vkGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
}

// VK_KHR_sampler_ycbcr_conversion
VKAPI_ATTR VkResult VKAPI_CALL vkCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSamplerYcbcrConversion *pYcbcrConversion)
{
	return vkCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
}

VKAPI_ATTR void VKAPI_CALL vkDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks *pAllocator)
{
	vkDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
}

// VK_KHR_create_renderpass2
VKAPI_ATTR VkResult VKAPI_CALL vkCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass)
{
	return vkCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin, const VkSubpassBeginInfo *pSubpassBegin)
{
	vkCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBegin);
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEnd)
{
	vkCmdEndRenderPass2(commandBuffer, pSubpassEnd);
}

VKAPI_ATTR void VKAPI_CALL vkCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBegin, const VkSubpassEndInfo *pSubpassEnd)
{
	vkCmdNextSubpass2(commandBuffer, pSubpassBegin, pSubpassEnd);
}

// VK_EXT_host_query_reset
VKAPI_ATTR void VKAPI_CALL vkResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount)
{
	vkResetQueryPool(device, queryPool, firstQuery, queryCount);
}

// VK_KHR_timeline_semaphore
VKAPI_ATTR VkResult VKAPI_CALL vkGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t *pValue)
{
	return vkGetSemaphoreCounterValue(device, semaphore, pValue);
}

VKAPI_ATTR VkResult VKAPI_CALL vkSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo)
{
	return vkSignalSemaphore(device, pSignalInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL vkWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout)
{
	return vkWaitSemaphores(device, pWaitInfo, timeout);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride)
{
	vkCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride)
{
	vkCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

VKAPI_ATTR uint64_t VKAPI_CALL vkGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo *pInfo)
{
	return vkGetBufferDeviceAddress(device, pInfo);
}

VKAPI_ATTR uint64_t VKAPI_CALL vkGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo *pInfo)
{
	return vkGetBufferOpaqueCaptureAddress(device, pInfo);
}

VKAPI_ATTR uint64_t VKAPI_CALL vkGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo *pInfo)
{
	return vkGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
}

// VK_EXT_tooling_info
VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t *pToolCount, VkPhysicalDeviceToolPropertiesEXT *pToolProperties)
{
	return vkGetPhysicalDeviceToolProperties(physicalDevice, pToolCount, pToolProperties);
}

// VK_EXT_private_data
VKAPI_ATTR VkResult VKAPI_CALL vkCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkPrivateDataSlotEXT *pPrivateDataSlot)
{
	return vkCreatePrivateDataSlot(device, pCreateInfo, pAllocator, pPrivateDataSlot);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlotEXT privateDataSlot, const VkAllocationCallbacks *pAllocator)
{
	vkDestroyPrivateDataSlot(device, privateDataSlot, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlotEXT privateDataSlot, uint64_t data)
{
	return vkSetPrivateData(device, objectType, objectHandle, privateDataSlot, data);
}

VKAPI_ATTR void VKAPI_CALL vkGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlotEXT privateDataSlot, uint64_t *pData)
{
	vkGetPrivateData(device, objectType, objectHandle, privateDataSlot, pData);
}

// VK_KHR_synchronization2
VKAPI_ATTR void VKAPI_CALL vkCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfoKHR *pDependencyInfo)
{
	vkCmdSetEvent2(commandBuffer, event, pDependencyInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2KHR stageMask)
{
	vkCmdResetEvent2(commandBuffer, event, stageMask);
}

VKAPI_ATTR void VKAPI_CALL vkCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents, const VkDependencyInfoKHR *pDependencyInfos)
{
	vkCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos);
}

VKAPI_ATTR void VKAPI_CALL vkCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR *pDependencyInfo)
{
	vkCmdPipelineBarrier2(commandBuffer, pDependencyInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR stage, VkQueryPool queryPool, uint32_t query)
{
	vkCmdWriteTimestamp2(commandBuffer, stage, queryPool, query);
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits, VkFence fence)
{
	return vkQueueSubmit2(queue, submitCount, pSubmits, fence);
}

// VK_KHR_copy_commands2
VKAPI_ATTR void VKAPI_CALL vkCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfo)
{
	vkCmdCopyBuffer2(commandBuffer, pCopyBufferInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo)
{
	vkCmdCopyImage2(commandBuffer, pCopyImageInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo)
{
	vkCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo)
{
	vkCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo)
{
	vkCmdBlitImage2(commandBuffer, pBlitImageInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR *pResolveImageInfo)
{
	vkCmdResolveImage2(commandBuffer, pResolveImageInfo);
}

// VK_KHR_dynamic_rendering
VKAPI_ATTR void VKAPI_CALL vkCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfoKHR *pRenderingInfo)
{
	vkCmdBeginRendering(commandBuffer, pRenderingInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndRenderingKHR(VkCommandBuffer commandBuffer)
{
	vkCmdEndRendering(commandBuffer);
}

// VK_EXT_extended_dynamic_state
VKAPI_ATTR void VKAPI_CALL vkCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode)
{
	vkCmdSetCullMode(commandBuffer, cullMode);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace)
{
	vkCmdSetFrontFace(commandBuffer, frontFace);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology)
{
	vkCmdSetPrimitiveTopology(commandBuffer, primitiveTopology);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport *pViewports)
{
	vkCmdSetViewportWithCount(commandBuffer, viewportCount, pViewports);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D *pScissors)
{
	vkCmdSetScissorWithCount(commandBuffer, scissorCount, pScissors);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer *pBuffers, const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes, const VkDeviceSize *pStrides)
{
	vkCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable)
{
	vkCmdSetDepthTestEnable(commandBuffer, depthTestEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable)
{
	vkCmdSetDepthWriteEnable(commandBuffer, depthWriteEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp)
{
	vkCmdSetDepthCompareOp(commandBuffer, depthCompareOp);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable)
{
	vkCmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable)
{
	vkCmdSetStencilTestEnable(commandBuffer, stencilTestEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp)
{
	vkCmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}

// VK_EXT_extended_dynamic_state2
VKAPI_ATTR void VKAPI_CALL vkCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable)
{
	vkCmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable)
{
	vkCmdSetDepthBiasEnable(commandBuffer, depthBiasEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable)
{
	vkCmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable);
}

// VK_KHR_maintenance4
VKAPI_ATTR void VKAPI_CALL vkGetDeviceBufferMemoryRequirementsKHR(VkDevice device, const VkDeviceBufferMemoryRequirementsKHR *pInfo, VkMemoryRequirements2 *pMemoryRequirements)
{
	vkGetDeviceBufferMemoryRequirements(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceImageMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirementsKHR *pInfo, VkMemoryRequirements2 *pMemoryRequirements)
{
	vkGetDeviceImageMemoryRequirements(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceImageSparseMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirementsKHR *pInfo, uint32_t *pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements)
{
	vkGetDeviceImageSparseMemoryRequirements(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}
}
