// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See struct_helper_generator.py for modifications
// Copyright 2023 The Khronos Group Inc.
// Copyright 2023 Valve Corporation
// Copyright 2023 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0
// NOLINTBEGIN
#pragma once
#include <vulkan/vulkan.h>

// clang-format off

namespace vku {

template <typename T>
VkStructureType GetSType() {
    static_assert(sizeof(T) == 0, "GetSType() is being used with an unsupported Type! Is the code-gen up to date?");
    return VK_STRUCTURE_TYPE_APPLICATION_INFO;
}
template <> inline VkStructureType GetSType<VkBufferMemoryBarrier>() { return VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER; }
template <> inline VkStructureType GetSType<VkImageMemoryBarrier>() { return VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER; }
template <> inline VkStructureType GetSType<VkMemoryBarrier>() { return VK_STRUCTURE_TYPE_MEMORY_BARRIER; }
template <> inline VkStructureType GetSType<VkApplicationInfo>() { return VK_STRUCTURE_TYPE_APPLICATION_INFO; }
template <> inline VkStructureType GetSType<VkInstanceCreateInfo>() { return VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkDeviceQueueCreateInfo>() { return VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkDeviceCreateInfo>() { return VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkSubmitInfo>() { return VK_STRUCTURE_TYPE_SUBMIT_INFO; }
template <> inline VkStructureType GetSType<VkMappedMemoryRange>() { return VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE; }
template <> inline VkStructureType GetSType<VkMemoryAllocateInfo>() { return VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO; }
template <> inline VkStructureType GetSType<VkBindSparseInfo>() { return VK_STRUCTURE_TYPE_BIND_SPARSE_INFO; }
template <> inline VkStructureType GetSType<VkFenceCreateInfo>() { return VK_STRUCTURE_TYPE_FENCE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkSemaphoreCreateInfo>() { return VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkEventCreateInfo>() { return VK_STRUCTURE_TYPE_EVENT_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkQueryPoolCreateInfo>() { return VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkBufferCreateInfo>() { return VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkBufferViewCreateInfo>() { return VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkImageCreateInfo>() { return VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkImageViewCreateInfo>() { return VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkShaderModuleCreateInfo>() { return VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPipelineCacheCreateInfo>() { return VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPipelineShaderStageCreateInfo>() { return VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkComputePipelineCreateInfo>() { return VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPipelineVertexInputStateCreateInfo>() { return VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPipelineInputAssemblyStateCreateInfo>() { return VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPipelineTessellationStateCreateInfo>() { return VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPipelineViewportStateCreateInfo>() { return VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPipelineRasterizationStateCreateInfo>() { return VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPipelineMultisampleStateCreateInfo>() { return VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPipelineDepthStencilStateCreateInfo>() { return VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPipelineColorBlendStateCreateInfo>() { return VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPipelineDynamicStateCreateInfo>() { return VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkGraphicsPipelineCreateInfo>() { return VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPipelineLayoutCreateInfo>() { return VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkSamplerCreateInfo>() { return VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkCopyDescriptorSet>() { return VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET; }
template <> inline VkStructureType GetSType<VkDescriptorPoolCreateInfo>() { return VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkDescriptorSetAllocateInfo>() { return VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO; }
template <> inline VkStructureType GetSType<VkDescriptorSetLayoutCreateInfo>() { return VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkWriteDescriptorSet>() { return VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; }
template <> inline VkStructureType GetSType<VkFramebufferCreateInfo>() { return VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkRenderPassCreateInfo>() { return VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkCommandPoolCreateInfo>() { return VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkCommandBufferAllocateInfo>() { return VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; }
template <> inline VkStructureType GetSType<VkCommandBufferInheritanceInfo>() { return VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO; }
template <> inline VkStructureType GetSType<VkCommandBufferBeginInfo>() { return VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO; }
template <> inline VkStructureType GetSType<VkRenderPassBeginInfo>() { return VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceSubgroupProperties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES; }
template <> inline VkStructureType GetSType<VkBindBufferMemoryInfo>() { return VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO; }
template <> inline VkStructureType GetSType<VkBindImageMemoryInfo>() { return VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDevice16BitStorageFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES; }
template <> inline VkStructureType GetSType<VkMemoryDedicatedRequirements>() { return VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS; }
template <> inline VkStructureType GetSType<VkMemoryDedicatedAllocateInfo>() { return VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO; }
template <> inline VkStructureType GetSType<VkMemoryAllocateFlagsInfo>() { return VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO; }
template <> inline VkStructureType GetSType<VkDeviceGroupRenderPassBeginInfo>() { return VK_STRUCTURE_TYPE_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO; }
template <> inline VkStructureType GetSType<VkDeviceGroupCommandBufferBeginInfo>() { return VK_STRUCTURE_TYPE_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO; }
template <> inline VkStructureType GetSType<VkDeviceGroupSubmitInfo>() { return VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO; }
template <> inline VkStructureType GetSType<VkDeviceGroupBindSparseInfo>() { return VK_STRUCTURE_TYPE_DEVICE_GROUP_BIND_SPARSE_INFO; }
template <> inline VkStructureType GetSType<VkBindBufferMemoryDeviceGroupInfo>() { return VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO; }
template <> inline VkStructureType GetSType<VkBindImageMemoryDeviceGroupInfo>() { return VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_DEVICE_GROUP_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceGroupProperties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES; }
template <> inline VkStructureType GetSType<VkDeviceGroupDeviceCreateInfo>() { return VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkBufferMemoryRequirementsInfo2>() { return VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2; }
template <> inline VkStructureType GetSType<VkImageMemoryRequirementsInfo2>() { return VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2; }
template <> inline VkStructureType GetSType<VkImageSparseMemoryRequirementsInfo2>() { return VK_STRUCTURE_TYPE_IMAGE_SPARSE_MEMORY_REQUIREMENTS_INFO_2; }
template <> inline VkStructureType GetSType<VkMemoryRequirements2>() { return VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2; }
template <> inline VkStructureType GetSType<VkSparseImageMemoryRequirements2>() { return VK_STRUCTURE_TYPE_SPARSE_IMAGE_MEMORY_REQUIREMENTS_2; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceFeatures2>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceProperties2>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2; }
template <> inline VkStructureType GetSType<VkFormatProperties2>() { return VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2; }
template <> inline VkStructureType GetSType<VkImageFormatProperties2>() { return VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceImageFormatInfo2>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2; }
template <> inline VkStructureType GetSType<VkQueueFamilyProperties2>() { return VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMemoryProperties2>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2; }
template <> inline VkStructureType GetSType<VkSparseImageFormatProperties2>() { return VK_STRUCTURE_TYPE_SPARSE_IMAGE_FORMAT_PROPERTIES_2; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceSparseImageFormatInfo2>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_INFO_2; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePointClippingProperties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES; }
template <> inline VkStructureType GetSType<VkRenderPassInputAttachmentAspectCreateInfo>() { return VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkImageViewUsageCreateInfo>() { return VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPipelineTessellationDomainOriginStateCreateInfo>() { return VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkRenderPassMultiviewCreateInfo>() { return VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMultiviewFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMultiviewProperties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceVariablePointersFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceProtectedMemoryFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceProtectedMemoryProperties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES; }
template <> inline VkStructureType GetSType<VkDeviceQueueInfo2>() { return VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2; }
template <> inline VkStructureType GetSType<VkProtectedSubmitInfo>() { return VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO; }
template <> inline VkStructureType GetSType<VkSamplerYcbcrConversionCreateInfo>() { return VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkSamplerYcbcrConversionInfo>() { return VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO; }
template <> inline VkStructureType GetSType<VkBindImagePlaneMemoryInfo>() { return VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO; }
template <> inline VkStructureType GetSType<VkImagePlaneMemoryRequirementsInfo>() { return VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceSamplerYcbcrConversionFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES; }
template <> inline VkStructureType GetSType<VkSamplerYcbcrConversionImageFormatProperties>() { return VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_IMAGE_FORMAT_PROPERTIES; }
template <> inline VkStructureType GetSType<VkDescriptorUpdateTemplateCreateInfo>() { return VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceExternalImageFormatInfo>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO; }
template <> inline VkStructureType GetSType<VkExternalImageFormatProperties>() { return VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceExternalBufferInfo>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO; }
template <> inline VkStructureType GetSType<VkExternalBufferProperties>() { return VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceIDProperties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES; }
template <> inline VkStructureType GetSType<VkExternalMemoryImageCreateInfo>() { return VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkExternalMemoryBufferCreateInfo>() { return VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkExportMemoryAllocateInfo>() { return VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceExternalFenceInfo>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO; }
template <> inline VkStructureType GetSType<VkExternalFenceProperties>() { return VK_STRUCTURE_TYPE_EXTERNAL_FENCE_PROPERTIES; }
template <> inline VkStructureType GetSType<VkExportFenceCreateInfo>() { return VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkExportSemaphoreCreateInfo>() { return VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceExternalSemaphoreInfo>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO; }
template <> inline VkStructureType GetSType<VkExternalSemaphoreProperties>() { return VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMaintenance3Properties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES; }
template <> inline VkStructureType GetSType<VkDescriptorSetLayoutSupport>() { return VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_SUPPORT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderDrawParametersFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceVulkan11Features>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceVulkan11Properties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceVulkan12Features>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceVulkan12Properties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES; }
template <> inline VkStructureType GetSType<VkImageFormatListCreateInfo>() { return VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkAttachmentDescription2>() { return VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2; }
template <> inline VkStructureType GetSType<VkAttachmentReference2>() { return VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2; }
template <> inline VkStructureType GetSType<VkSubpassDescription2>() { return VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2; }
template <> inline VkStructureType GetSType<VkSubpassDependency2>() { return VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2; }
template <> inline VkStructureType GetSType<VkRenderPassCreateInfo2>() { return VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2; }
template <> inline VkStructureType GetSType<VkSubpassBeginInfo>() { return VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO; }
template <> inline VkStructureType GetSType<VkSubpassEndInfo>() { return VK_STRUCTURE_TYPE_SUBPASS_END_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDevice8BitStorageFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDriverProperties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderAtomicInt64Features>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderFloat16Int8Features>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceFloatControlsProperties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES; }
template <> inline VkStructureType GetSType<VkDescriptorSetLayoutBindingFlagsCreateInfo>() { return VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDescriptorIndexingFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDescriptorIndexingProperties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES; }
template <> inline VkStructureType GetSType<VkDescriptorSetVariableDescriptorCountAllocateInfo>() { return VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO; }
template <> inline VkStructureType GetSType<VkDescriptorSetVariableDescriptorCountLayoutSupport>() { return VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_LAYOUT_SUPPORT; }
template <> inline VkStructureType GetSType<VkSubpassDescriptionDepthStencilResolve>() { return VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDepthStencilResolveProperties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceScalarBlockLayoutFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES; }
template <> inline VkStructureType GetSType<VkImageStencilUsageCreateInfo>() { return VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkSamplerReductionModeCreateInfo>() { return VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceSamplerFilterMinmaxProperties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceVulkanMemoryModelFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceImagelessFramebufferFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES; }
template <> inline VkStructureType GetSType<VkFramebufferAttachmentImageInfo>() { return VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO; }
template <> inline VkStructureType GetSType<VkFramebufferAttachmentsCreateInfo>() { return VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkRenderPassAttachmentBeginInfo>() { return VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceUniformBufferStandardLayoutFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES; }
template <> inline VkStructureType GetSType<VkAttachmentReferenceStencilLayout>() { return VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_STENCIL_LAYOUT; }
template <> inline VkStructureType GetSType<VkAttachmentDescriptionStencilLayout>() { return VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_STENCIL_LAYOUT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceHostQueryResetFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceTimelineSemaphoreFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceTimelineSemaphoreProperties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES; }
template <> inline VkStructureType GetSType<VkSemaphoreTypeCreateInfo>() { return VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkTimelineSemaphoreSubmitInfo>() { return VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO; }
template <> inline VkStructureType GetSType<VkSemaphoreWaitInfo>() { return VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO; }
template <> inline VkStructureType GetSType<VkSemaphoreSignalInfo>() { return VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceBufferDeviceAddressFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES; }
template <> inline VkStructureType GetSType<VkBufferDeviceAddressInfo>() { return VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO; }
template <> inline VkStructureType GetSType<VkBufferOpaqueCaptureAddressCreateInfo>() { return VK_STRUCTURE_TYPE_BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkMemoryOpaqueCaptureAddressAllocateInfo>() { return VK_STRUCTURE_TYPE_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO; }
template <> inline VkStructureType GetSType<VkDeviceMemoryOpaqueCaptureAddressInfo>() { return VK_STRUCTURE_TYPE_DEVICE_MEMORY_OPAQUE_CAPTURE_ADDRESS_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceVulkan13Features>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceVulkan13Properties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES; }
template <> inline VkStructureType GetSType<VkPipelineCreationFeedbackCreateInfo>() { return VK_STRUCTURE_TYPE_PIPELINE_CREATION_FEEDBACK_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderTerminateInvocationFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceToolProperties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TOOL_PROPERTIES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePrivateDataFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES; }
template <> inline VkStructureType GetSType<VkDevicePrivateDataCreateInfo>() { return VK_STRUCTURE_TYPE_DEVICE_PRIVATE_DATA_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPrivateDataSlotCreateInfo>() { return VK_STRUCTURE_TYPE_PRIVATE_DATA_SLOT_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePipelineCreationCacheControlFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES; }
template <> inline VkStructureType GetSType<VkMemoryBarrier2>() { return VK_STRUCTURE_TYPE_MEMORY_BARRIER_2; }
template <> inline VkStructureType GetSType<VkBufferMemoryBarrier2>() { return VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2; }
template <> inline VkStructureType GetSType<VkImageMemoryBarrier2>() { return VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2; }
template <> inline VkStructureType GetSType<VkDependencyInfo>() { return VK_STRUCTURE_TYPE_DEPENDENCY_INFO; }
template <> inline VkStructureType GetSType<VkSemaphoreSubmitInfo>() { return VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO; }
template <> inline VkStructureType GetSType<VkCommandBufferSubmitInfo>() { return VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO; }
template <> inline VkStructureType GetSType<VkSubmitInfo2>() { return VK_STRUCTURE_TYPE_SUBMIT_INFO_2; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceSynchronization2Features>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceImageRobustnessFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES; }
template <> inline VkStructureType GetSType<VkBufferCopy2>() { return VK_STRUCTURE_TYPE_BUFFER_COPY_2; }
template <> inline VkStructureType GetSType<VkCopyBufferInfo2>() { return VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2; }
template <> inline VkStructureType GetSType<VkImageCopy2>() { return VK_STRUCTURE_TYPE_IMAGE_COPY_2; }
template <> inline VkStructureType GetSType<VkCopyImageInfo2>() { return VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2; }
template <> inline VkStructureType GetSType<VkBufferImageCopy2>() { return VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2; }
template <> inline VkStructureType GetSType<VkCopyBufferToImageInfo2>() { return VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2; }
template <> inline VkStructureType GetSType<VkCopyImageToBufferInfo2>() { return VK_STRUCTURE_TYPE_COPY_IMAGE_TO_BUFFER_INFO_2; }
template <> inline VkStructureType GetSType<VkImageBlit2>() { return VK_STRUCTURE_TYPE_IMAGE_BLIT_2; }
template <> inline VkStructureType GetSType<VkBlitImageInfo2>() { return VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2; }
template <> inline VkStructureType GetSType<VkImageResolve2>() { return VK_STRUCTURE_TYPE_IMAGE_RESOLVE_2; }
template <> inline VkStructureType GetSType<VkResolveImageInfo2>() { return VK_STRUCTURE_TYPE_RESOLVE_IMAGE_INFO_2; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceSubgroupSizeControlFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceSubgroupSizeControlProperties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES; }
template <> inline VkStructureType GetSType<VkPipelineShaderStageRequiredSubgroupSizeCreateInfo>() { return VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceInlineUniformBlockFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceInlineUniformBlockProperties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_PROPERTIES; }
template <> inline VkStructureType GetSType<VkWriteDescriptorSetInlineUniformBlock>() { return VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK; }
template <> inline VkStructureType GetSType<VkDescriptorPoolInlineUniformBlockCreateInfo>() { return VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_INLINE_UNIFORM_BLOCK_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceTextureCompressionASTCHDRFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES; }
template <> inline VkStructureType GetSType<VkRenderingAttachmentInfo>() { return VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO; }
template <> inline VkStructureType GetSType<VkRenderingInfo>() { return VK_STRUCTURE_TYPE_RENDERING_INFO; }
template <> inline VkStructureType GetSType<VkPipelineRenderingCreateInfo>() { return VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDynamicRenderingFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES; }
template <> inline VkStructureType GetSType<VkCommandBufferInheritanceRenderingInfo>() { return VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderIntegerDotProductFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderIntegerDotProductProperties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_PROPERTIES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceTexelBufferAlignmentProperties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_PROPERTIES; }
template <> inline VkStructureType GetSType<VkFormatProperties3>() { return VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_3; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMaintenance4Features>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMaintenance4Properties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES; }
template <> inline VkStructureType GetSType<VkDeviceBufferMemoryRequirements>() { return VK_STRUCTURE_TYPE_DEVICE_BUFFER_MEMORY_REQUIREMENTS; }
template <> inline VkStructureType GetSType<VkDeviceImageMemoryRequirements>() { return VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceVulkan14Features>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceVulkan14Properties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_PROPERTIES; }
template <> inline VkStructureType GetSType<VkDeviceQueueGlobalPriorityCreateInfo>() { return VK_STRUCTURE_TYPE_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceGlobalPriorityQueryFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GLOBAL_PRIORITY_QUERY_FEATURES; }
template <> inline VkStructureType GetSType<VkQueueFamilyGlobalPriorityProperties>() { return VK_STRUCTURE_TYPE_QUEUE_FAMILY_GLOBAL_PRIORITY_PROPERTIES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderSubgroupRotateFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_ROTATE_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderFloatControls2Features>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT_CONTROLS_2_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderExpectAssumeFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EXPECT_ASSUME_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceLineRasterizationFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceLineRasterizationProperties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_PROPERTIES; }
template <> inline VkStructureType GetSType<VkPipelineRasterizationLineStateCreateInfo>() { return VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceVertexAttributeDivisorProperties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES; }
template <> inline VkStructureType GetSType<VkPipelineVertexInputDivisorStateCreateInfo>() { return VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceVertexAttributeDivisorFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceIndexTypeUint8Features>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES; }
template <> inline VkStructureType GetSType<VkMemoryMapInfo>() { return VK_STRUCTURE_TYPE_MEMORY_MAP_INFO; }
template <> inline VkStructureType GetSType<VkMemoryUnmapInfo>() { return VK_STRUCTURE_TYPE_MEMORY_UNMAP_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMaintenance5Features>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMaintenance5Properties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_PROPERTIES; }
template <> inline VkStructureType GetSType<VkRenderingAreaInfo>() { return VK_STRUCTURE_TYPE_RENDERING_AREA_INFO; }
template <> inline VkStructureType GetSType<VkImageSubresource2>() { return VK_STRUCTURE_TYPE_IMAGE_SUBRESOURCE_2; }
template <> inline VkStructureType GetSType<VkDeviceImageSubresourceInfo>() { return VK_STRUCTURE_TYPE_DEVICE_IMAGE_SUBRESOURCE_INFO; }
template <> inline VkStructureType GetSType<VkSubresourceLayout2>() { return VK_STRUCTURE_TYPE_SUBRESOURCE_LAYOUT_2; }
template <> inline VkStructureType GetSType<VkPipelineCreateFlags2CreateInfo>() { return VK_STRUCTURE_TYPE_PIPELINE_CREATE_FLAGS_2_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkBufferUsageFlags2CreateInfo>() { return VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePushDescriptorProperties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDynamicRenderingLocalReadFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES; }
template <> inline VkStructureType GetSType<VkRenderingAttachmentLocationInfo>() { return VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_LOCATION_INFO; }
template <> inline VkStructureType GetSType<VkRenderingInputAttachmentIndexInfo>() { return VK_STRUCTURE_TYPE_RENDERING_INPUT_ATTACHMENT_INDEX_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMaintenance6Features>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMaintenance6Properties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_PROPERTIES; }
template <> inline VkStructureType GetSType<VkBindMemoryStatus>() { return VK_STRUCTURE_TYPE_BIND_MEMORY_STATUS; }
template <> inline VkStructureType GetSType<VkBindDescriptorSetsInfo>() { return VK_STRUCTURE_TYPE_BIND_DESCRIPTOR_SETS_INFO; }
template <> inline VkStructureType GetSType<VkPushConstantsInfo>() { return VK_STRUCTURE_TYPE_PUSH_CONSTANTS_INFO; }
template <> inline VkStructureType GetSType<VkPushDescriptorSetInfo>() { return VK_STRUCTURE_TYPE_PUSH_DESCRIPTOR_SET_INFO; }
template <> inline VkStructureType GetSType<VkPushDescriptorSetWithTemplateInfo>() { return VK_STRUCTURE_TYPE_PUSH_DESCRIPTOR_SET_WITH_TEMPLATE_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePipelineProtectedAccessFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROTECTED_ACCESS_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePipelineRobustnessFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePipelineRobustnessProperties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_PROPERTIES; }
template <> inline VkStructureType GetSType<VkPipelineRobustnessCreateInfo>() { return VK_STRUCTURE_TYPE_PIPELINE_ROBUSTNESS_CREATE_INFO; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceHostImageCopyFeatures>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceHostImageCopyProperties>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_PROPERTIES; }
template <> inline VkStructureType GetSType<VkMemoryToImageCopy>() { return VK_STRUCTURE_TYPE_MEMORY_TO_IMAGE_COPY; }
template <> inline VkStructureType GetSType<VkImageToMemoryCopy>() { return VK_STRUCTURE_TYPE_IMAGE_TO_MEMORY_COPY; }
template <> inline VkStructureType GetSType<VkCopyMemoryToImageInfo>() { return VK_STRUCTURE_TYPE_COPY_MEMORY_TO_IMAGE_INFO; }
template <> inline VkStructureType GetSType<VkCopyImageToMemoryInfo>() { return VK_STRUCTURE_TYPE_COPY_IMAGE_TO_MEMORY_INFO; }
template <> inline VkStructureType GetSType<VkCopyImageToImageInfo>() { return VK_STRUCTURE_TYPE_COPY_IMAGE_TO_IMAGE_INFO; }
template <> inline VkStructureType GetSType<VkHostImageLayoutTransitionInfo>() { return VK_STRUCTURE_TYPE_HOST_IMAGE_LAYOUT_TRANSITION_INFO; }
template <> inline VkStructureType GetSType<VkSubresourceHostMemcpySize>() { return VK_STRUCTURE_TYPE_SUBRESOURCE_HOST_MEMCPY_SIZE; }
template <> inline VkStructureType GetSType<VkHostImageCopyDevicePerformanceQuery>() { return VK_STRUCTURE_TYPE_HOST_IMAGE_COPY_DEVICE_PERFORMANCE_QUERY; }
template <> inline VkStructureType GetSType<VkSwapchainCreateInfoKHR>() { return VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPresentInfoKHR>() { return VK_STRUCTURE_TYPE_PRESENT_INFO_KHR; }
template <> inline VkStructureType GetSType<VkImageSwapchainCreateInfoKHR>() { return VK_STRUCTURE_TYPE_IMAGE_SWAPCHAIN_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkBindImageMemorySwapchainInfoKHR>() { return VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR; }
template <> inline VkStructureType GetSType<VkAcquireNextImageInfoKHR>() { return VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkDeviceGroupPresentCapabilitiesKHR>() { return VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_CAPABILITIES_KHR; }
template <> inline VkStructureType GetSType<VkDeviceGroupPresentInfoKHR>() { return VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_INFO_KHR; }
template <> inline VkStructureType GetSType<VkDeviceGroupSwapchainCreateInfoKHR>() { return VK_STRUCTURE_TYPE_DEVICE_GROUP_SWAPCHAIN_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkDisplayModeCreateInfoKHR>() { return VK_STRUCTURE_TYPE_DISPLAY_MODE_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkDisplaySurfaceCreateInfoKHR>() { return VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkDisplayPresentInfoKHR>() { return VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR; }
#ifdef VK_USE_PLATFORM_XLIB_KHR
template <> inline VkStructureType GetSType<VkXlibSurfaceCreateInfoKHR>() { return VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR; }
#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
template <> inline VkStructureType GetSType<VkXcbSurfaceCreateInfoKHR>() { return VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR; }
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
template <> inline VkStructureType GetSType<VkWaylandSurfaceCreateInfoKHR>() { return VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR; }
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
template <> inline VkStructureType GetSType<VkAndroidSurfaceCreateInfoKHR>() { return VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR; }
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
template <> inline VkStructureType GetSType<VkWin32SurfaceCreateInfoKHR>() { return VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR; }
#endif  // VK_USE_PLATFORM_WIN32_KHR
template <> inline VkStructureType GetSType<VkQueueFamilyQueryResultStatusPropertiesKHR>() { return VK_STRUCTURE_TYPE_QUEUE_FAMILY_QUERY_RESULT_STATUS_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkQueueFamilyVideoPropertiesKHR>() { return VK_STRUCTURE_TYPE_QUEUE_FAMILY_VIDEO_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkVideoProfileInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoProfileListInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_PROFILE_LIST_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoCapabilitiesKHR>() { return VK_STRUCTURE_TYPE_VIDEO_CAPABILITIES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceVideoFormatInfoKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_FORMAT_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoFormatPropertiesKHR>() { return VK_STRUCTURE_TYPE_VIDEO_FORMAT_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkVideoPictureResourceInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_PICTURE_RESOURCE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoReferenceSlotInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_REFERENCE_SLOT_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoSessionMemoryRequirementsKHR>() { return VK_STRUCTURE_TYPE_VIDEO_SESSION_MEMORY_REQUIREMENTS_KHR; }
template <> inline VkStructureType GetSType<VkBindVideoSessionMemoryInfoKHR>() { return VK_STRUCTURE_TYPE_BIND_VIDEO_SESSION_MEMORY_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoSessionCreateInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_SESSION_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoSessionParametersCreateInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_SESSION_PARAMETERS_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoSessionParametersUpdateInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_SESSION_PARAMETERS_UPDATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoBeginCodingInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_BEGIN_CODING_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEndCodingInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_END_CODING_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoCodingControlInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_CODING_CONTROL_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeCapabilitiesKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_CAPABILITIES_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeUsageInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_USAGE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH264CapabilitiesKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_CAPABILITIES_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH264QualityLevelPropertiesKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_QUALITY_LEVEL_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH264SessionCreateInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH264SessionParametersAddInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_ADD_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH264SessionParametersCreateInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH264SessionParametersGetInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_GET_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH264SessionParametersFeedbackInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_FEEDBACK_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH264NaluSliceInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_NALU_SLICE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH264PictureInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PICTURE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH264DpbSlotInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_DPB_SLOT_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH264ProfileInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PROFILE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH264RateControlInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_RATE_CONTROL_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH264RateControlLayerInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_RATE_CONTROL_LAYER_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH264GopRemainingFrameInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_GOP_REMAINING_FRAME_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH265CapabilitiesKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_CAPABILITIES_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH265SessionCreateInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH265QualityLevelPropertiesKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_QUALITY_LEVEL_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH265SessionParametersAddInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_ADD_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH265SessionParametersCreateInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH265SessionParametersGetInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_GET_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH265SessionParametersFeedbackInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_FEEDBACK_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH265NaluSliceSegmentInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_NALU_SLICE_SEGMENT_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH265PictureInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PICTURE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH265DpbSlotInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_DPB_SLOT_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH265ProfileInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PROFILE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH265RateControlInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_RATE_CONTROL_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH265RateControlLayerInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_RATE_CONTROL_LAYER_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH265GopRemainingFrameInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_GOP_REMAINING_FRAME_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeH264ProfileInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PROFILE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeH264CapabilitiesKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_CAPABILITIES_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeH264SessionParametersAddInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_ADD_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeH264SessionParametersCreateInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeH264PictureInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PICTURE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeH264DpbSlotInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_DPB_SLOT_INFO_KHR; }
#ifdef VK_USE_PLATFORM_WIN32_KHR
template <> inline VkStructureType GetSType<VkImportMemoryWin32HandleInfoKHR>() { return VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkExportMemoryWin32HandleInfoKHR>() { return VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkMemoryWin32HandlePropertiesKHR>() { return VK_STRUCTURE_TYPE_MEMORY_WIN32_HANDLE_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkMemoryGetWin32HandleInfoKHR>() { return VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR; }
#endif  // VK_USE_PLATFORM_WIN32_KHR
template <> inline VkStructureType GetSType<VkImportMemoryFdInfoKHR>() { return VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR; }
template <> inline VkStructureType GetSType<VkMemoryFdPropertiesKHR>() { return VK_STRUCTURE_TYPE_MEMORY_FD_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkMemoryGetFdInfoKHR>() { return VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR; }
#ifdef VK_USE_PLATFORM_WIN32_KHR
template <> inline VkStructureType GetSType<VkWin32KeyedMutexAcquireReleaseInfoKHR>() { return VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkImportSemaphoreWin32HandleInfoKHR>() { return VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkExportSemaphoreWin32HandleInfoKHR>() { return VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkD3D12FenceSubmitInfoKHR>() { return VK_STRUCTURE_TYPE_D3D12_FENCE_SUBMIT_INFO_KHR; }
template <> inline VkStructureType GetSType<VkSemaphoreGetWin32HandleInfoKHR>() { return VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR; }
#endif  // VK_USE_PLATFORM_WIN32_KHR
template <> inline VkStructureType GetSType<VkImportSemaphoreFdInfoKHR>() { return VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR; }
template <> inline VkStructureType GetSType<VkSemaphoreGetFdInfoKHR>() { return VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPresentRegionsKHR>() { return VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR; }
template <> inline VkStructureType GetSType<VkSharedPresentSurfaceCapabilitiesKHR>() { return VK_STRUCTURE_TYPE_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR; }
#ifdef VK_USE_PLATFORM_WIN32_KHR
template <> inline VkStructureType GetSType<VkImportFenceWin32HandleInfoKHR>() { return VK_STRUCTURE_TYPE_IMPORT_FENCE_WIN32_HANDLE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkExportFenceWin32HandleInfoKHR>() { return VK_STRUCTURE_TYPE_EXPORT_FENCE_WIN32_HANDLE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkFenceGetWin32HandleInfoKHR>() { return VK_STRUCTURE_TYPE_FENCE_GET_WIN32_HANDLE_INFO_KHR; }
#endif  // VK_USE_PLATFORM_WIN32_KHR
template <> inline VkStructureType GetSType<VkImportFenceFdInfoKHR>() { return VK_STRUCTURE_TYPE_IMPORT_FENCE_FD_INFO_KHR; }
template <> inline VkStructureType GetSType<VkFenceGetFdInfoKHR>() { return VK_STRUCTURE_TYPE_FENCE_GET_FD_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePerformanceQueryFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePerformanceQueryPropertiesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkPerformanceCounterKHR>() { return VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_KHR; }
template <> inline VkStructureType GetSType<VkPerformanceCounterDescriptionKHR>() { return VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_DESCRIPTION_KHR; }
template <> inline VkStructureType GetSType<VkQueryPoolPerformanceCreateInfoKHR>() { return VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkAcquireProfilingLockInfoKHR>() { return VK_STRUCTURE_TYPE_ACQUIRE_PROFILING_LOCK_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPerformanceQuerySubmitInfoKHR>() { return VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_SUBMIT_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceSurfaceInfo2KHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR; }
template <> inline VkStructureType GetSType<VkSurfaceCapabilities2KHR>() { return VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR; }
template <> inline VkStructureType GetSType<VkSurfaceFormat2KHR>() { return VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR; }
template <> inline VkStructureType GetSType<VkDisplayProperties2KHR>() { return VK_STRUCTURE_TYPE_DISPLAY_PROPERTIES_2_KHR; }
template <> inline VkStructureType GetSType<VkDisplayPlaneProperties2KHR>() { return VK_STRUCTURE_TYPE_DISPLAY_PLANE_PROPERTIES_2_KHR; }
template <> inline VkStructureType GetSType<VkDisplayModeProperties2KHR>() { return VK_STRUCTURE_TYPE_DISPLAY_MODE_PROPERTIES_2_KHR; }
template <> inline VkStructureType GetSType<VkDisplayPlaneInfo2KHR>() { return VK_STRUCTURE_TYPE_DISPLAY_PLANE_INFO_2_KHR; }
template <> inline VkStructureType GetSType<VkDisplayPlaneCapabilities2KHR>() { return VK_STRUCTURE_TYPE_DISPLAY_PLANE_CAPABILITIES_2_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderBfloat16FeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_BFLOAT16_FEATURES_KHR; }
#ifdef VK_ENABLE_BETA_EXTENSIONS
template <> inline VkStructureType GetSType<VkPhysicalDevicePortabilitySubsetFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePortabilitySubsetPropertiesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_PROPERTIES_KHR; }
#endif  // VK_ENABLE_BETA_EXTENSIONS
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderClockFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeH265ProfileInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PROFILE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeH265CapabilitiesKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_CAPABILITIES_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeH265SessionParametersAddInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_PARAMETERS_ADD_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeH265SessionParametersCreateInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_PARAMETERS_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeH265PictureInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PICTURE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeH265DpbSlotInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_DPB_SLOT_INFO_KHR; }
template <> inline VkStructureType GetSType<VkFragmentShadingRateAttachmentInfoKHR>() { return VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPipelineFragmentShadingRateStateCreateInfoKHR>() { return VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_STATE_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceFragmentShadingRateKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_KHR; }
template <> inline VkStructureType GetSType<VkRenderingFragmentShadingRateAttachmentInfoKHR>() { return VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderQuadControlFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_QUAD_CONTROL_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkSurfaceProtectedCapabilitiesKHR>() { return VK_STRUCTURE_TYPE_SURFACE_PROTECTED_CAPABILITIES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePresentWaitFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkPipelineInfoKHR>() { return VK_STRUCTURE_TYPE_PIPELINE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPipelineExecutablePropertiesKHR>() { return VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkPipelineExecutableInfoKHR>() { return VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPipelineExecutableStatisticKHR>() { return VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_STATISTIC_KHR; }
template <> inline VkStructureType GetSType<VkPipelineExecutableInternalRepresentationKHR>() { return VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INTERNAL_REPRESENTATION_KHR; }
template <> inline VkStructureType GetSType<VkPipelineLibraryCreateInfoKHR>() { return VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPresentIdKHR>() { return VK_STRUCTURE_TYPE_PRESENT_ID_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePresentIdFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeCapabilitiesKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_CAPABILITIES_KHR; }
template <> inline VkStructureType GetSType<VkQueryPoolVideoEncodeFeedbackCreateInfoKHR>() { return VK_STRUCTURE_TYPE_QUERY_POOL_VIDEO_ENCODE_FEEDBACK_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeUsageInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_USAGE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeRateControlLayerInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_RATE_CONTROL_LAYER_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeRateControlInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_RATE_CONTROL_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_QUALITY_LEVEL_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeQualityLevelPropertiesKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUALITY_LEVEL_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeQualityLevelInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUALITY_LEVEL_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeSessionParametersGetInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_SESSION_PARAMETERS_GET_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeSessionParametersFeedbackInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_SESSION_PARAMETERS_FEEDBACK_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MAXIMAL_RECONVERGENCE_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkSurfaceCapabilitiesPresentId2KHR>() { return VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_PRESENT_ID_2_KHR; }
template <> inline VkStructureType GetSType<VkPresentId2KHR>() { return VK_STRUCTURE_TYPE_PRESENT_ID_2_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePresentId2FeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_2_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkSurfaceCapabilitiesPresentWait2KHR>() { return VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_PRESENT_WAIT_2_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePresentWait2FeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_2_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkPresentWait2InfoKHR>() { return VK_STRUCTURE_TYPE_PRESENT_WAIT_2_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePipelineBinaryFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_BINARY_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePipelineBinaryPropertiesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_BINARY_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkDevicePipelineBinaryInternalCacheControlKHR>() { return VK_STRUCTURE_TYPE_DEVICE_PIPELINE_BINARY_INTERNAL_CACHE_CONTROL_KHR; }
template <> inline VkStructureType GetSType<VkPipelineBinaryKeyKHR>() { return VK_STRUCTURE_TYPE_PIPELINE_BINARY_KEY_KHR; }
template <> inline VkStructureType GetSType<VkPipelineCreateInfoKHR>() { return VK_STRUCTURE_TYPE_PIPELINE_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPipelineBinaryCreateInfoKHR>() { return VK_STRUCTURE_TYPE_PIPELINE_BINARY_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPipelineBinaryInfoKHR>() { return VK_STRUCTURE_TYPE_PIPELINE_BINARY_INFO_KHR; }
template <> inline VkStructureType GetSType<VkReleaseCapturedPipelineDataInfoKHR>() { return VK_STRUCTURE_TYPE_RELEASE_CAPTURED_PIPELINE_DATA_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPipelineBinaryDataInfoKHR>() { return VK_STRUCTURE_TYPE_PIPELINE_BINARY_DATA_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPipelineBinaryHandlesInfoKHR>() { return VK_STRUCTURE_TYPE_PIPELINE_BINARY_HANDLES_INFO_KHR; }
template <> inline VkStructureType GetSType<VkSurfacePresentModeKHR>() { return VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_KHR; }
template <> inline VkStructureType GetSType<VkSurfacePresentScalingCapabilitiesKHR>() { return VK_STRUCTURE_TYPE_SURFACE_PRESENT_SCALING_CAPABILITIES_KHR; }
template <> inline VkStructureType GetSType<VkSurfacePresentModeCompatibilityKHR>() { return VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_COMPATIBILITY_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkSwapchainPresentFenceInfoKHR>() { return VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkSwapchainPresentModesCreateInfoKHR>() { return VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_MODES_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkSwapchainPresentModeInfoKHR>() { return VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_MODE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkSwapchainPresentScalingCreateInfoKHR>() { return VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_SCALING_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkReleaseSwapchainImagesInfoKHR>() { return VK_STRUCTURE_TYPE_RELEASE_SWAPCHAIN_IMAGES_INFO_KHR; }
template <> inline VkStructureType GetSType<VkCooperativeMatrixPropertiesKHR>() { return VK_STRUCTURE_TYPE_COOPERATIVE_MATRIX_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceCooperativeMatrixFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceCooperativeMatrixPropertiesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeAV1ProfileInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_PROFILE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeAV1CapabilitiesKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_CAPABILITIES_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeAV1SessionParametersCreateInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_SESSION_PARAMETERS_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeAV1PictureInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_PICTURE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeAV1DpbSlotInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_DPB_SLOT_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceVideoEncodeAV1FeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_AV1_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeAV1CapabilitiesKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_CAPABILITIES_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeAV1QualityLevelPropertiesKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_QUALITY_LEVEL_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeAV1SessionCreateInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_SESSION_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeAV1SessionParametersCreateInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_SESSION_PARAMETERS_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeAV1PictureInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_PICTURE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeAV1DpbSlotInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_DPB_SLOT_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeAV1ProfileInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_PROFILE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeAV1GopRemainingFrameInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_GOP_REMAINING_FRAME_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeAV1RateControlInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_RATE_CONTROL_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeAV1RateControlLayerInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_RATE_CONTROL_LAYER_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceVideoDecodeVP9FeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_DECODE_VP9_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeVP9ProfileInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_VP9_PROFILE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeVP9CapabilitiesKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_VP9_CAPABILITIES_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeVP9PictureInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_VP9_PICTURE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceVideoMaintenance1FeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_1_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkVideoInlineQueryInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_INLINE_QUERY_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFIED_IMAGE_LAYOUTS_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkAttachmentFeedbackLoopInfoEXT>() { return VK_STRUCTURE_TYPE_ATTACHMENT_FEEDBACK_LOOP_INFO_EXT; }
template <> inline VkStructureType GetSType<VkCalibratedTimestampInfoKHR>() { return VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_KHR; }
template <> inline VkStructureType GetSType<VkSetDescriptorBufferOffsetsInfoEXT>() { return VK_STRUCTURE_TYPE_SET_DESCRIPTOR_BUFFER_OFFSETS_INFO_EXT; }
template <> inline VkStructureType GetSType<VkBindDescriptorBufferEmbeddedSamplersInfoEXT>() { return VK_STRUCTURE_TYPE_BIND_DESCRIPTOR_BUFFER_EMBEDDED_SAMPLERS_INFO_EXT; }
template <> inline VkStructureType GetSType<VkVideoEncodeIntraRefreshCapabilitiesKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_INTRA_REFRESH_CAPABILITIES_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeSessionIntraRefreshCreateInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_SESSION_INTRA_REFRESH_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeIntraRefreshInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_INTRA_REFRESH_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoReferenceIntraRefreshInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_REFERENCE_INTRA_REFRESH_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_INTRA_REFRESH_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeQuantizationMapCapabilitiesKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_CAPABILITIES_KHR; }
template <> inline VkStructureType GetSType<VkVideoFormatQuantizationMapPropertiesKHR>() { return VK_STRUCTURE_TYPE_VIDEO_FORMAT_QUANTIZATION_MAP_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeQuantizationMapInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_SESSION_PARAMETERS_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_QUANTIZATION_MAP_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH264QuantizationMapCapabilitiesKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_QUANTIZATION_MAP_CAPABILITIES_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeH265QuantizationMapCapabilitiesKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_QUANTIZATION_MAP_CAPABILITIES_KHR; }
template <> inline VkStructureType GetSType<VkVideoFormatH265QuantizationMapPropertiesKHR>() { return VK_STRUCTURE_TYPE_VIDEO_FORMAT_H265_QUANTIZATION_MAP_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkVideoEncodeAV1QuantizationMapCapabilitiesKHR>() { return VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_QUANTIZATION_MAP_CAPABILITIES_KHR; }
template <> inline VkStructureType GetSType<VkVideoFormatAV1QuantizationMapPropertiesKHR>() { return VK_STRUCTURE_TYPE_VIDEO_FORMAT_AV1_QUANTIZATION_MAP_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_RELAXED_EXTENDED_INSTRUCTION_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMaintenance7FeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMaintenance7PropertiesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceLayeredApiPropertiesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_API_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceLayeredApiPropertiesListKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_API_PROPERTIES_LIST_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceLayeredApiVulkanPropertiesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_API_VULKAN_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMaintenance8FeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_8_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkMemoryBarrierAccessFlags3KHR>() { return VK_STRUCTURE_TYPE_MEMORY_BARRIER_ACCESS_FLAGS_3_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMaintenance9FeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_9_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMaintenance9PropertiesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_9_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkQueueFamilyOwnershipTransferPropertiesKHR>() { return VK_STRUCTURE_TYPE_QUEUE_FAMILY_OWNERSHIP_TRANSFER_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceVideoMaintenance2FeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_2_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeH264InlineSessionParametersInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_INLINE_SESSION_PARAMETERS_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeH265InlineSessionParametersInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_INLINE_SESSION_PARAMETERS_INFO_KHR; }
template <> inline VkStructureType GetSType<VkVideoDecodeAV1InlineSessionParametersInfoKHR>() { return VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_INLINE_SESSION_PARAMETERS_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDepthClampZeroOneFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_ZERO_ONE_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceRobustness2FeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceRobustness2PropertiesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_MODE_FIFO_LATEST_READY_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkDebugReportCallbackCreateInfoEXT>() { return VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPipelineRasterizationStateRasterizationOrderAMD>() { return VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_RASTERIZATION_ORDER_AMD; }
template <> inline VkStructureType GetSType<VkDebugMarkerObjectNameInfoEXT>() { return VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT; }
template <> inline VkStructureType GetSType<VkDebugMarkerObjectTagInfoEXT>() { return VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT; }
template <> inline VkStructureType GetSType<VkDebugMarkerMarkerInfoEXT>() { return VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT; }
template <> inline VkStructureType GetSType<VkDedicatedAllocationImageCreateInfoNV>() { return VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_IMAGE_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkDedicatedAllocationBufferCreateInfoNV>() { return VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_BUFFER_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkDedicatedAllocationMemoryAllocateInfoNV>() { return VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_MEMORY_ALLOCATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceTransformFeedbackFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceTransformFeedbackPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkPipelineRasterizationStateStreamCreateInfoEXT>() { return VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_STREAM_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkCuModuleCreateInfoNVX>() { return VK_STRUCTURE_TYPE_CU_MODULE_CREATE_INFO_NVX; }
template <> inline VkStructureType GetSType<VkCuModuleTexturingModeCreateInfoNVX>() { return VK_STRUCTURE_TYPE_CU_MODULE_TEXTURING_MODE_CREATE_INFO_NVX; }
template <> inline VkStructureType GetSType<VkCuFunctionCreateInfoNVX>() { return VK_STRUCTURE_TYPE_CU_FUNCTION_CREATE_INFO_NVX; }
template <> inline VkStructureType GetSType<VkCuLaunchInfoNVX>() { return VK_STRUCTURE_TYPE_CU_LAUNCH_INFO_NVX; }
template <> inline VkStructureType GetSType<VkImageViewHandleInfoNVX>() { return VK_STRUCTURE_TYPE_IMAGE_VIEW_HANDLE_INFO_NVX; }
template <> inline VkStructureType GetSType<VkImageViewAddressPropertiesNVX>() { return VK_STRUCTURE_TYPE_IMAGE_VIEW_ADDRESS_PROPERTIES_NVX; }
template <> inline VkStructureType GetSType<VkTextureLODGatherFormatPropertiesAMD>() { return VK_STRUCTURE_TYPE_TEXTURE_LOD_GATHER_FORMAT_PROPERTIES_AMD; }
#ifdef VK_USE_PLATFORM_GGP
template <> inline VkStructureType GetSType<VkStreamDescriptorSurfaceCreateInfoGGP>() { return VK_STRUCTURE_TYPE_STREAM_DESCRIPTOR_SURFACE_CREATE_INFO_GGP; }
#endif  // VK_USE_PLATFORM_GGP
template <> inline VkStructureType GetSType<VkPhysicalDeviceCornerSampledImageFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CORNER_SAMPLED_IMAGE_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkExternalMemoryImageCreateInfoNV>() { return VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkExportMemoryAllocateInfoNV>() { return VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_NV; }
#ifdef VK_USE_PLATFORM_WIN32_KHR
template <> inline VkStructureType GetSType<VkImportMemoryWin32HandleInfoNV>() { return VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_NV; }
template <> inline VkStructureType GetSType<VkExportMemoryWin32HandleInfoNV>() { return VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_NV; }
template <> inline VkStructureType GetSType<VkWin32KeyedMutexAcquireReleaseInfoNV>() { return VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_NV; }
#endif  // VK_USE_PLATFORM_WIN32_KHR
template <> inline VkStructureType GetSType<VkValidationFlagsEXT>() { return VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT; }
#ifdef VK_USE_PLATFORM_VI_NN
template <> inline VkStructureType GetSType<VkViSurfaceCreateInfoNN>() { return VK_STRUCTURE_TYPE_VI_SURFACE_CREATE_INFO_NN; }
#endif  // VK_USE_PLATFORM_VI_NN
template <> inline VkStructureType GetSType<VkImageViewASTCDecodeModeEXT>() { return VK_STRUCTURE_TYPE_IMAGE_VIEW_ASTC_DECODE_MODE_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceASTCDecodeFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ASTC_DECODE_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkConditionalRenderingBeginInfoEXT>() { return VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceConditionalRenderingFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkCommandBufferInheritanceConditionalRenderingInfoEXT>() { return VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPipelineViewportWScalingStateCreateInfoNV>() { return VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_W_SCALING_STATE_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkSurfaceCapabilities2EXT>() { return VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_EXT; }
template <> inline VkStructureType GetSType<VkDisplayPowerInfoEXT>() { return VK_STRUCTURE_TYPE_DISPLAY_POWER_INFO_EXT; }
template <> inline VkStructureType GetSType<VkDeviceEventInfoEXT>() { return VK_STRUCTURE_TYPE_DEVICE_EVENT_INFO_EXT; }
template <> inline VkStructureType GetSType<VkDisplayEventInfoEXT>() { return VK_STRUCTURE_TYPE_DISPLAY_EVENT_INFO_EXT; }
template <> inline VkStructureType GetSType<VkSwapchainCounterCreateInfoEXT>() { return VK_STRUCTURE_TYPE_SWAPCHAIN_COUNTER_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPresentTimesInfoGOOGLE>() { return VK_STRUCTURE_TYPE_PRESENT_TIMES_INFO_GOOGLE; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_ATTRIBUTES_PROPERTIES_NVX; }
template <> inline VkStructureType GetSType<VkMultiviewPerViewAttributesInfoNVX>() { return VK_STRUCTURE_TYPE_MULTIVIEW_PER_VIEW_ATTRIBUTES_INFO_NVX; }
template <> inline VkStructureType GetSType<VkPipelineViewportSwizzleStateCreateInfoNV>() { return VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SWIZZLE_STATE_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDiscardRectanglePropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISCARD_RECTANGLE_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkPipelineDiscardRectangleStateCreateInfoEXT>() { return VK_STRUCTURE_TYPE_PIPELINE_DISCARD_RECTANGLE_STATE_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceConservativeRasterizationPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkPipelineRasterizationConservativeStateCreateInfoEXT>() { return VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDepthClipEnableFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPipelineRasterizationDepthClipStateCreateInfoEXT>() { return VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkHdrMetadataEXT>() { return VK_STRUCTURE_TYPE_HDR_METADATA_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RELAXED_LINE_RASTERIZATION_FEATURES_IMG; }
#ifdef VK_USE_PLATFORM_IOS_MVK
template <> inline VkStructureType GetSType<VkIOSSurfaceCreateInfoMVK>() { return VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK; }
#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
template <> inline VkStructureType GetSType<VkMacOSSurfaceCreateInfoMVK>() { return VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK; }
#endif  // VK_USE_PLATFORM_MACOS_MVK
template <> inline VkStructureType GetSType<VkDebugUtilsLabelEXT>() { return VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT; }
template <> inline VkStructureType GetSType<VkDebugUtilsObjectNameInfoEXT>() { return VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT; }
template <> inline VkStructureType GetSType<VkDebugUtilsMessengerCallbackDataEXT>() { return VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT; }
template <> inline VkStructureType GetSType<VkDebugUtilsMessengerCreateInfoEXT>() { return VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkDebugUtilsObjectTagInfoEXT>() { return VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT; }
#ifdef VK_USE_PLATFORM_ANDROID_KHR
template <> inline VkStructureType GetSType<VkAndroidHardwareBufferUsageANDROID>() { return VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_USAGE_ANDROID; }
template <> inline VkStructureType GetSType<VkAndroidHardwareBufferPropertiesANDROID>() { return VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID; }
template <> inline VkStructureType GetSType<VkAndroidHardwareBufferFormatPropertiesANDROID>() { return VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID; }
template <> inline VkStructureType GetSType<VkImportAndroidHardwareBufferInfoANDROID>() { return VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID; }
template <> inline VkStructureType GetSType<VkMemoryGetAndroidHardwareBufferInfoANDROID>() { return VK_STRUCTURE_TYPE_MEMORY_GET_ANDROID_HARDWARE_BUFFER_INFO_ANDROID; }
template <> inline VkStructureType GetSType<VkExternalFormatANDROID>() { return VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID; }
template <> inline VkStructureType GetSType<VkAndroidHardwareBufferFormatProperties2ANDROID>() { return VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_2_ANDROID; }
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderEnqueueFeaturesAMDX>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ENQUEUE_FEATURES_AMDX; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderEnqueuePropertiesAMDX>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ENQUEUE_PROPERTIES_AMDX; }
template <> inline VkStructureType GetSType<VkExecutionGraphPipelineScratchSizeAMDX>() { return VK_STRUCTURE_TYPE_EXECUTION_GRAPH_PIPELINE_SCRATCH_SIZE_AMDX; }
template <> inline VkStructureType GetSType<VkExecutionGraphPipelineCreateInfoAMDX>() { return VK_STRUCTURE_TYPE_EXECUTION_GRAPH_PIPELINE_CREATE_INFO_AMDX; }
template <> inline VkStructureType GetSType<VkPipelineShaderStageNodeCreateInfoAMDX>() { return VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_NODE_CREATE_INFO_AMDX; }
#endif  // VK_ENABLE_BETA_EXTENSIONS
template <> inline VkStructureType GetSType<VkAttachmentSampleCountInfoAMD>() { return VK_STRUCTURE_TYPE_ATTACHMENT_SAMPLE_COUNT_INFO_AMD; }
template <> inline VkStructureType GetSType<VkSampleLocationsInfoEXT>() { return VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT; }
template <> inline VkStructureType GetSType<VkRenderPassSampleLocationsBeginInfoEXT>() { return VK_STRUCTURE_TYPE_RENDER_PASS_SAMPLE_LOCATIONS_BEGIN_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPipelineSampleLocationsStateCreateInfoEXT>() { return VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceSampleLocationsPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkMultisamplePropertiesEXT>() { return VK_STRUCTURE_TYPE_MULTISAMPLE_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkPipelineColorBlendAdvancedStateCreateInfoEXT>() { return VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPipelineCoverageToColorStateCreateInfoNV>() { return VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_TO_COLOR_STATE_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkPipelineCoverageModulationStateCreateInfoNV>() { return VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_MODULATION_STATE_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderSMBuiltinsPropertiesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_PROPERTIES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderSMBuiltinsFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkDrmFormatModifierPropertiesListEXT>() { return VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT; }
template <> inline VkStructureType GetSType<VkImageDrmFormatModifierListCreateInfoEXT>() { return VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkImageDrmFormatModifierExplicitCreateInfoEXT>() { return VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkImageDrmFormatModifierPropertiesEXT>() { return VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkDrmFormatModifierPropertiesList2EXT>() { return VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_2_EXT; }
template <> inline VkStructureType GetSType<VkValidationCacheCreateInfoEXT>() { return VK_STRUCTURE_TYPE_VALIDATION_CACHE_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkShaderModuleValidationCacheCreateInfoEXT>() { return VK_STRUCTURE_TYPE_SHADER_MODULE_VALIDATION_CACHE_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPipelineViewportShadingRateImageStateCreateInfoNV>() { return VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShadingRateImageFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShadingRateImagePropertiesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_PROPERTIES_NV; }
template <> inline VkStructureType GetSType<VkPipelineViewportCoarseSampleOrderStateCreateInfoNV>() { return VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_COARSE_SAMPLE_ORDER_STATE_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkRayTracingShaderGroupCreateInfoNV>() { return VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkRayTracingPipelineCreateInfoNV>() { return VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkGeometryTrianglesNV>() { return VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV; }
template <> inline VkStructureType GetSType<VkGeometryAABBNV>() { return VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV; }
template <> inline VkStructureType GetSType<VkGeometryNV>() { return VK_STRUCTURE_TYPE_GEOMETRY_NV; }
template <> inline VkStructureType GetSType<VkAccelerationStructureInfoNV>() { return VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV; }
template <> inline VkStructureType GetSType<VkAccelerationStructureCreateInfoNV>() { return VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkBindAccelerationStructureMemoryInfoNV>() { return VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV; }
template <> inline VkStructureType GetSType<VkWriteDescriptorSetAccelerationStructureNV>() { return VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV; }
template <> inline VkStructureType GetSType<VkAccelerationStructureMemoryRequirementsInfoNV>() { return VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceRayTracingPropertiesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_REPRESENTATIVE_FRAGMENT_TEST_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPipelineRepresentativeFragmentTestStateCreateInfoNV>() { return VK_STRUCTURE_TYPE_PIPELINE_REPRESENTATIVE_FRAGMENT_TEST_STATE_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceImageViewImageFormatInfoEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_IMAGE_FORMAT_INFO_EXT; }
template <> inline VkStructureType GetSType<VkFilterCubicImageViewImageFormatPropertiesEXT>() { return VK_STRUCTURE_TYPE_FILTER_CUBIC_IMAGE_VIEW_IMAGE_FORMAT_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkImportMemoryHostPointerInfoEXT>() { return VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT; }
template <> inline VkStructureType GetSType<VkMemoryHostPointerPropertiesEXT>() { return VK_STRUCTURE_TYPE_MEMORY_HOST_POINTER_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceExternalMemoryHostPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkPipelineCompilerControlCreateInfoAMD>() { return VK_STRUCTURE_TYPE_PIPELINE_COMPILER_CONTROL_CREATE_INFO_AMD; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderCorePropertiesAMD>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_AMD; }
template <> inline VkStructureType GetSType<VkDeviceMemoryOverallocationCreateInfoAMD>() { return VK_STRUCTURE_TYPE_DEVICE_MEMORY_OVERALLOCATION_CREATE_INFO_AMD; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT; }
#ifdef VK_USE_PLATFORM_GGP
template <> inline VkStructureType GetSType<VkPresentFrameTokenGGP>() { return VK_STRUCTURE_TYPE_PRESENT_FRAME_TOKEN_GGP; }
#endif  // VK_USE_PLATFORM_GGP
template <> inline VkStructureType GetSType<VkPhysicalDeviceMeshShaderFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMeshShaderPropertiesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderImageFootprintFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_FOOTPRINT_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPipelineViewportExclusiveScissorStateCreateInfoNV>() { return VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_EXCLUSIVE_SCISSOR_STATE_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceExclusiveScissorFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXCLUSIVE_SCISSOR_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkQueueFamilyCheckpointPropertiesNV>() { return VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_NV; }
template <> inline VkStructureType GetSType<VkCheckpointDataNV>() { return VK_STRUCTURE_TYPE_CHECKPOINT_DATA_NV; }
template <> inline VkStructureType GetSType<VkQueueFamilyCheckpointProperties2NV>() { return VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_2_NV; }
template <> inline VkStructureType GetSType<VkCheckpointData2NV>() { return VK_STRUCTURE_TYPE_CHECKPOINT_DATA_2_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_FUNCTIONS_2_FEATURES_INTEL; }
template <> inline VkStructureType GetSType<VkInitializePerformanceApiInfoINTEL>() { return VK_STRUCTURE_TYPE_INITIALIZE_PERFORMANCE_API_INFO_INTEL; }
template <> inline VkStructureType GetSType<VkQueryPoolPerformanceQueryCreateInfoINTEL>() { return VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_QUERY_CREATE_INFO_INTEL; }
template <> inline VkStructureType GetSType<VkPerformanceMarkerInfoINTEL>() { return VK_STRUCTURE_TYPE_PERFORMANCE_MARKER_INFO_INTEL; }
template <> inline VkStructureType GetSType<VkPerformanceStreamMarkerInfoINTEL>() { return VK_STRUCTURE_TYPE_PERFORMANCE_STREAM_MARKER_INFO_INTEL; }
template <> inline VkStructureType GetSType<VkPerformanceOverrideInfoINTEL>() { return VK_STRUCTURE_TYPE_PERFORMANCE_OVERRIDE_INFO_INTEL; }
template <> inline VkStructureType GetSType<VkPerformanceConfigurationAcquireInfoINTEL>() { return VK_STRUCTURE_TYPE_PERFORMANCE_CONFIGURATION_ACQUIRE_INFO_INTEL; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePCIBusInfoPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkDisplayNativeHdrSurfaceCapabilitiesAMD>() { return VK_STRUCTURE_TYPE_DISPLAY_NATIVE_HDR_SURFACE_CAPABILITIES_AMD; }
template <> inline VkStructureType GetSType<VkSwapchainDisplayNativeHdrCreateInfoAMD>() { return VK_STRUCTURE_TYPE_SWAPCHAIN_DISPLAY_NATIVE_HDR_CREATE_INFO_AMD; }
#ifdef VK_USE_PLATFORM_FUCHSIA
template <> inline VkStructureType GetSType<VkImagePipeSurfaceCreateInfoFUCHSIA>() { return VK_STRUCTURE_TYPE_IMAGEPIPE_SURFACE_CREATE_INFO_FUCHSIA; }
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT
template <> inline VkStructureType GetSType<VkMetalSurfaceCreateInfoEXT>() { return VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT; }
#endif  // VK_USE_PLATFORM_METAL_EXT
template <> inline VkStructureType GetSType<VkPhysicalDeviceFragmentDensityMapFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceFragmentDensityMapPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkRenderPassFragmentDensityMapCreateInfoEXT>() { return VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkRenderingFragmentDensityMapAttachmentInfoEXT>() { return VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_DENSITY_MAP_ATTACHMENT_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderCoreProperties2AMD>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_2_AMD; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceCoherentMemoryFeaturesAMD>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COHERENT_MEMORY_FEATURES_AMD; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMemoryBudgetPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMemoryPriorityFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkMemoryPriorityAllocateInfoEXT>() { return VK_STRUCTURE_TYPE_MEMORY_PRIORITY_ALLOCATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEDICATED_ALLOCATION_IMAGE_ALIASING_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceBufferDeviceAddressFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkBufferDeviceAddressCreateInfoEXT>() { return VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkValidationFeaturesEXT>() { return VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkCooperativeMatrixPropertiesNV>() { return VK_STRUCTURE_TYPE_COOPERATIVE_MATRIX_PROPERTIES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceCooperativeMatrixFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceCooperativeMatrixPropertiesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceCoverageReductionModeFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COVERAGE_REDUCTION_MODE_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPipelineCoverageReductionStateCreateInfoNV>() { return VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_REDUCTION_STATE_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkFramebufferMixedSamplesCombinationNV>() { return VK_STRUCTURE_TYPE_FRAMEBUFFER_MIXED_SAMPLES_COMBINATION_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceYcbcrImageArraysFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_IMAGE_ARRAYS_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceProvokingVertexFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceProvokingVertexPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkPipelineRasterizationProvokingVertexStateCreateInfoEXT>() { return VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_PROVOKING_VERTEX_STATE_CREATE_INFO_EXT; }
#ifdef VK_USE_PLATFORM_WIN32_KHR
template <> inline VkStructureType GetSType<VkSurfaceFullScreenExclusiveInfoEXT>() { return VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkSurfaceCapabilitiesFullScreenExclusiveEXT>() { return VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_FULL_SCREEN_EXCLUSIVE_EXT; }
template <> inline VkStructureType GetSType<VkSurfaceFullScreenExclusiveWin32InfoEXT>() { return VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT; }
#endif  // VK_USE_PLATFORM_WIN32_KHR
template <> inline VkStructureType GetSType<VkHeadlessSurfaceCreateInfoEXT>() { return VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMapMemoryPlacedFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAP_MEMORY_PLACED_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMapMemoryPlacedPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAP_MEMORY_PLACED_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkMemoryMapPlacedInfoEXT>() { return VK_STRUCTURE_TYPE_MEMORY_MAP_PLACED_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_2_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_PROPERTIES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkGraphicsShaderGroupCreateInfoNV>() { return VK_STRUCTURE_TYPE_GRAPHICS_SHADER_GROUP_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkGraphicsPipelineShaderGroupsCreateInfoNV>() { return VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_SHADER_GROUPS_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkIndirectCommandsLayoutTokenNV>() { return VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_TOKEN_NV; }
template <> inline VkStructureType GetSType<VkIndirectCommandsLayoutCreateInfoNV>() { return VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkGeneratedCommandsInfoNV>() { return VK_STRUCTURE_TYPE_GENERATED_COMMANDS_INFO_NV; }
template <> inline VkStructureType GetSType<VkGeneratedCommandsMemoryRequirementsInfoNV>() { return VK_STRUCTURE_TYPE_GENERATED_COMMANDS_MEMORY_REQUIREMENTS_INFO_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceInheritedViewportScissorFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INHERITED_VIEWPORT_SCISSOR_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkCommandBufferInheritanceViewportScissorInfoNV>() { return VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_VIEWPORT_SCISSOR_INFO_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkRenderPassTransformBeginInfoQCOM>() { return VK_STRUCTURE_TYPE_RENDER_PASS_TRANSFORM_BEGIN_INFO_QCOM; }
template <> inline VkStructureType GetSType<VkCommandBufferInheritanceRenderPassTransformInfoQCOM>() { return VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDER_PASS_TRANSFORM_INFO_QCOM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDepthBiasControlFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_BIAS_CONTROL_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkDepthBiasInfoEXT>() { return VK_STRUCTURE_TYPE_DEPTH_BIAS_INFO_EXT; }
template <> inline VkStructureType GetSType<VkDepthBiasRepresentationInfoEXT>() { return VK_STRUCTURE_TYPE_DEPTH_BIAS_REPRESENTATION_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDeviceMemoryReportFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_MEMORY_REPORT_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkDeviceMemoryReportCallbackDataEXT>() { return VK_STRUCTURE_TYPE_DEVICE_MEMORY_REPORT_CALLBACK_DATA_EXT; }
template <> inline VkStructureType GetSType<VkDeviceDeviceMemoryReportCreateInfoEXT>() { return VK_STRUCTURE_TYPE_DEVICE_DEVICE_MEMORY_REPORT_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkSamplerCustomBorderColorCreateInfoEXT>() { return VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceCustomBorderColorPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceCustomBorderColorFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePresentBarrierFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_BARRIER_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkSurfaceCapabilitiesPresentBarrierNV>() { return VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_PRESENT_BARRIER_NV; }
template <> inline VkStructureType GetSType<VkSwapchainPresentBarrierCreateInfoNV>() { return VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_BARRIER_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDiagnosticsConfigFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DIAGNOSTICS_CONFIG_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkDeviceDiagnosticsConfigCreateInfoNV>() { return VK_STRUCTURE_TYPE_DEVICE_DIAGNOSTICS_CONFIG_CREATE_INFO_NV; }
#ifdef VK_ENABLE_BETA_EXTENSIONS
template <> inline VkStructureType GetSType<VkCudaModuleCreateInfoNV>() { return VK_STRUCTURE_TYPE_CUDA_MODULE_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkCudaFunctionCreateInfoNV>() { return VK_STRUCTURE_TYPE_CUDA_FUNCTION_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkCudaLaunchInfoNV>() { return VK_STRUCTURE_TYPE_CUDA_LAUNCH_INFO_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceCudaKernelLaunchFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUDA_KERNEL_LAUNCH_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceCudaKernelLaunchPropertiesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUDA_KERNEL_LAUNCH_PROPERTIES_NV; }
#endif  // VK_ENABLE_BETA_EXTENSIONS
template <> inline VkStructureType GetSType<VkPhysicalDeviceTileShadingFeaturesQCOM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_SHADING_FEATURES_QCOM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceTileShadingPropertiesQCOM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_SHADING_PROPERTIES_QCOM; }
template <> inline VkStructureType GetSType<VkRenderPassTileShadingCreateInfoQCOM>() { return VK_STRUCTURE_TYPE_RENDER_PASS_TILE_SHADING_CREATE_INFO_QCOM; }
template <> inline VkStructureType GetSType<VkPerTileBeginInfoQCOM>() { return VK_STRUCTURE_TYPE_PER_TILE_BEGIN_INFO_QCOM; }
template <> inline VkStructureType GetSType<VkPerTileEndInfoQCOM>() { return VK_STRUCTURE_TYPE_PER_TILE_END_INFO_QCOM; }
template <> inline VkStructureType GetSType<VkDispatchTileInfoQCOM>() { return VK_STRUCTURE_TYPE_DISPATCH_TILE_INFO_QCOM; }
template <> inline VkStructureType GetSType<VkQueryLowLatencySupportNV>() { return VK_STRUCTURE_TYPE_QUERY_LOW_LATENCY_SUPPORT_NV; }
#ifdef VK_USE_PLATFORM_METAL_EXT
template <> inline VkStructureType GetSType<VkExportMetalObjectCreateInfoEXT>() { return VK_STRUCTURE_TYPE_EXPORT_METAL_OBJECT_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkExportMetalObjectsInfoEXT>() { return VK_STRUCTURE_TYPE_EXPORT_METAL_OBJECTS_INFO_EXT; }
template <> inline VkStructureType GetSType<VkExportMetalDeviceInfoEXT>() { return VK_STRUCTURE_TYPE_EXPORT_METAL_DEVICE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkExportMetalCommandQueueInfoEXT>() { return VK_STRUCTURE_TYPE_EXPORT_METAL_COMMAND_QUEUE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkExportMetalBufferInfoEXT>() { return VK_STRUCTURE_TYPE_EXPORT_METAL_BUFFER_INFO_EXT; }
template <> inline VkStructureType GetSType<VkImportMetalBufferInfoEXT>() { return VK_STRUCTURE_TYPE_IMPORT_METAL_BUFFER_INFO_EXT; }
template <> inline VkStructureType GetSType<VkExportMetalTextureInfoEXT>() { return VK_STRUCTURE_TYPE_EXPORT_METAL_TEXTURE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkImportMetalTextureInfoEXT>() { return VK_STRUCTURE_TYPE_IMPORT_METAL_TEXTURE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkExportMetalIOSurfaceInfoEXT>() { return VK_STRUCTURE_TYPE_EXPORT_METAL_IO_SURFACE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkImportMetalIOSurfaceInfoEXT>() { return VK_STRUCTURE_TYPE_IMPORT_METAL_IO_SURFACE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkExportMetalSharedEventInfoEXT>() { return VK_STRUCTURE_TYPE_EXPORT_METAL_SHARED_EVENT_INFO_EXT; }
template <> inline VkStructureType GetSType<VkImportMetalSharedEventInfoEXT>() { return VK_STRUCTURE_TYPE_IMPORT_METAL_SHARED_EVENT_INFO_EXT; }
#endif  // VK_USE_PLATFORM_METAL_EXT
template <> inline VkStructureType GetSType<VkPhysicalDeviceDescriptorBufferPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_DENSITY_MAP_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDescriptorBufferFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkDescriptorAddressInfoEXT>() { return VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT; }
template <> inline VkStructureType GetSType<VkDescriptorBufferBindingInfoEXT>() { return VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT; }
template <> inline VkStructureType GetSType<VkDescriptorBufferBindingPushDescriptorBufferHandleEXT>() { return VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_PUSH_DESCRIPTOR_BUFFER_HANDLE_EXT; }
template <> inline VkStructureType GetSType<VkDescriptorGetInfoEXT>() { return VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT; }
template <> inline VkStructureType GetSType<VkBufferCaptureDescriptorDataInfoEXT>() { return VK_STRUCTURE_TYPE_BUFFER_CAPTURE_DESCRIPTOR_DATA_INFO_EXT; }
template <> inline VkStructureType GetSType<VkImageCaptureDescriptorDataInfoEXT>() { return VK_STRUCTURE_TYPE_IMAGE_CAPTURE_DESCRIPTOR_DATA_INFO_EXT; }
template <> inline VkStructureType GetSType<VkImageViewCaptureDescriptorDataInfoEXT>() { return VK_STRUCTURE_TYPE_IMAGE_VIEW_CAPTURE_DESCRIPTOR_DATA_INFO_EXT; }
template <> inline VkStructureType GetSType<VkSamplerCaptureDescriptorDataInfoEXT>() { return VK_STRUCTURE_TYPE_SAMPLER_CAPTURE_DESCRIPTOR_DATA_INFO_EXT; }
template <> inline VkStructureType GetSType<VkOpaqueCaptureDescriptorDataCreateInfoEXT>() { return VK_STRUCTURE_TYPE_OPAQUE_CAPTURE_DESCRIPTOR_DATA_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkAccelerationStructureCaptureDescriptorDataInfoEXT>() { return VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CAPTURE_DESCRIPTOR_DATA_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkGraphicsPipelineLibraryCreateInfoEXT>() { return VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EARLY_AND_LATE_FRAGMENT_TESTS_FEATURES_AMD; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_PROPERTIES_NV; }
template <> inline VkStructureType GetSType<VkPipelineFragmentShadingRateEnumStateCreateInfoNV>() { return VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_ENUM_STATE_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkAccelerationStructureGeometryMotionTrianglesDataNV>() { return VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_MOTION_TRIANGLES_DATA_NV; }
template <> inline VkStructureType GetSType<VkAccelerationStructureMotionInfoNV>() { return VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MOTION_INFO_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceRayTracingMotionBlurFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MOTION_BLUR_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_2_PLANE_444_FORMATS_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceFragmentDensityMap2FeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceFragmentDensityMap2PropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkCopyCommandTransformInfoQCOM>() { return VK_STRUCTURE_TYPE_COPY_COMMAND_TRANSFORM_INFO_QCOM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceImageCompressionControlFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkImageCompressionControlEXT>() { return VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_CONTROL_EXT; }
template <> inline VkStructureType GetSType<VkImageCompressionPropertiesEXT>() { return VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDevice4444FormatsFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_4444_FORMATS_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceFaultFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FAULT_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkDeviceFaultCountsEXT>() { return VK_STRUCTURE_TYPE_DEVICE_FAULT_COUNTS_EXT; }
template <> inline VkStructureType GetSType<VkDeviceFaultInfoEXT>() { return VK_STRUCTURE_TYPE_DEVICE_FAULT_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RGBA10X6_FORMATS_FEATURES_EXT; }
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
template <> inline VkStructureType GetSType<VkDirectFBSurfaceCreateInfoEXT>() { return VK_STRUCTURE_TYPE_DIRECTFB_SURFACE_CREATE_INFO_EXT; }
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
template <> inline VkStructureType GetSType<VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkMutableDescriptorTypeCreateInfoEXT>() { return VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkVertexInputBindingDescription2EXT>() { return VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT; }
template <> inline VkStructureType GetSType<VkVertexInputAttributeDescription2EXT>() { return VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDrmPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRM_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceAddressBindingReportFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ADDRESS_BINDING_REPORT_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkDeviceAddressBindingCallbackDataEXT>() { return VK_STRUCTURE_TYPE_DEVICE_ADDRESS_BINDING_CALLBACK_DATA_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDepthClipControlFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_CONTROL_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPipelineViewportDepthClipControlCreateInfoEXT>() { return VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLIP_CONTROL_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT; }
#ifdef VK_USE_PLATFORM_FUCHSIA
template <> inline VkStructureType GetSType<VkImportMemoryZirconHandleInfoFUCHSIA>() { return VK_STRUCTURE_TYPE_IMPORT_MEMORY_ZIRCON_HANDLE_INFO_FUCHSIA; }
template <> inline VkStructureType GetSType<VkMemoryZirconHandlePropertiesFUCHSIA>() { return VK_STRUCTURE_TYPE_MEMORY_ZIRCON_HANDLE_PROPERTIES_FUCHSIA; }
template <> inline VkStructureType GetSType<VkMemoryGetZirconHandleInfoFUCHSIA>() { return VK_STRUCTURE_TYPE_MEMORY_GET_ZIRCON_HANDLE_INFO_FUCHSIA; }
template <> inline VkStructureType GetSType<VkImportSemaphoreZirconHandleInfoFUCHSIA>() { return VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_ZIRCON_HANDLE_INFO_FUCHSIA; }
template <> inline VkStructureType GetSType<VkSemaphoreGetZirconHandleInfoFUCHSIA>() { return VK_STRUCTURE_TYPE_SEMAPHORE_GET_ZIRCON_HANDLE_INFO_FUCHSIA; }
template <> inline VkStructureType GetSType<VkBufferCollectionCreateInfoFUCHSIA>() { return VK_STRUCTURE_TYPE_BUFFER_COLLECTION_CREATE_INFO_FUCHSIA; }
template <> inline VkStructureType GetSType<VkImportMemoryBufferCollectionFUCHSIA>() { return VK_STRUCTURE_TYPE_IMPORT_MEMORY_BUFFER_COLLECTION_FUCHSIA; }
template <> inline VkStructureType GetSType<VkBufferCollectionImageCreateInfoFUCHSIA>() { return VK_STRUCTURE_TYPE_BUFFER_COLLECTION_IMAGE_CREATE_INFO_FUCHSIA; }
template <> inline VkStructureType GetSType<VkBufferCollectionConstraintsInfoFUCHSIA>() { return VK_STRUCTURE_TYPE_BUFFER_COLLECTION_CONSTRAINTS_INFO_FUCHSIA; }
template <> inline VkStructureType GetSType<VkBufferConstraintsInfoFUCHSIA>() { return VK_STRUCTURE_TYPE_BUFFER_CONSTRAINTS_INFO_FUCHSIA; }
template <> inline VkStructureType GetSType<VkBufferCollectionBufferCreateInfoFUCHSIA>() { return VK_STRUCTURE_TYPE_BUFFER_COLLECTION_BUFFER_CREATE_INFO_FUCHSIA; }
template <> inline VkStructureType GetSType<VkSysmemColorSpaceFUCHSIA>() { return VK_STRUCTURE_TYPE_SYSMEM_COLOR_SPACE_FUCHSIA; }
template <> inline VkStructureType GetSType<VkBufferCollectionPropertiesFUCHSIA>() { return VK_STRUCTURE_TYPE_BUFFER_COLLECTION_PROPERTIES_FUCHSIA; }
template <> inline VkStructureType GetSType<VkImageFormatConstraintsInfoFUCHSIA>() { return VK_STRUCTURE_TYPE_IMAGE_FORMAT_CONSTRAINTS_INFO_FUCHSIA; }
template <> inline VkStructureType GetSType<VkImageConstraintsInfoFUCHSIA>() { return VK_STRUCTURE_TYPE_IMAGE_CONSTRAINTS_INFO_FUCHSIA; }
#endif  // VK_USE_PLATFORM_FUCHSIA
template <> inline VkStructureType GetSType<VkSubpassShadingPipelineCreateInfoHUAWEI>() { return VK_STRUCTURE_TYPE_SUBPASS_SHADING_PIPELINE_CREATE_INFO_HUAWEI; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceSubpassShadingFeaturesHUAWEI>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_SHADING_FEATURES_HUAWEI; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceSubpassShadingPropertiesHUAWEI>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_SHADING_PROPERTIES_HUAWEI; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceInvocationMaskFeaturesHUAWEI>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INVOCATION_MASK_FEATURES_HUAWEI; }
template <> inline VkStructureType GetSType<VkMemoryGetRemoteAddressInfoNV>() { return VK_STRUCTURE_TYPE_MEMORY_GET_REMOTE_ADDRESS_INFO_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceExternalMemoryRDMAFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_RDMA_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPipelinePropertiesIdentifierEXT>() { return VK_STRUCTURE_TYPE_PIPELINE_PROPERTIES_IDENTIFIER_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePipelinePropertiesFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROPERTIES_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceFrameBoundaryFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAME_BOUNDARY_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkFrameBoundaryEXT>() { return VK_STRUCTURE_TYPE_FRAME_BOUNDARY_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkSubpassResolvePerformanceQueryEXT>() { return VK_STRUCTURE_TYPE_SUBPASS_RESOLVE_PERFORMANCE_QUERY_EXT; }
template <> inline VkStructureType GetSType<VkMultisampledRenderToSingleSampledInfoEXT>() { return VK_STRUCTURE_TYPE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT; }
#ifdef VK_USE_PLATFORM_SCREEN_QNX
template <> inline VkStructureType GetSType<VkScreenSurfaceCreateInfoQNX>() { return VK_STRUCTURE_TYPE_SCREEN_SURFACE_CREATE_INFO_QNX; }
#endif  // VK_USE_PLATFORM_SCREEN_QNX
template <> inline VkStructureType GetSType<VkPhysicalDeviceColorWriteEnableFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPipelineColorWriteCreateInfoEXT>() { return VK_STRUCTURE_TYPE_PIPELINE_COLOR_WRITE_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVES_GENERATED_QUERY_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceImageViewMinLodFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_MIN_LOD_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkImageViewMinLodCreateInfoEXT>() { return VK_STRUCTURE_TYPE_IMAGE_VIEW_MIN_LOD_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMultiDrawFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMultiDrawPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceImage2DViewOf3DFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_2D_VIEW_OF_3D_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderTileImageFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderTileImagePropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkMicromapBuildInfoEXT>() { return VK_STRUCTURE_TYPE_MICROMAP_BUILD_INFO_EXT; }
template <> inline VkStructureType GetSType<VkMicromapCreateInfoEXT>() { return VK_STRUCTURE_TYPE_MICROMAP_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceOpacityMicromapFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceOpacityMicromapPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkMicromapVersionInfoEXT>() { return VK_STRUCTURE_TYPE_MICROMAP_VERSION_INFO_EXT; }
template <> inline VkStructureType GetSType<VkCopyMicromapToMemoryInfoEXT>() { return VK_STRUCTURE_TYPE_COPY_MICROMAP_TO_MEMORY_INFO_EXT; }
template <> inline VkStructureType GetSType<VkCopyMemoryToMicromapInfoEXT>() { return VK_STRUCTURE_TYPE_COPY_MEMORY_TO_MICROMAP_INFO_EXT; }
template <> inline VkStructureType GetSType<VkCopyMicromapInfoEXT>() { return VK_STRUCTURE_TYPE_COPY_MICROMAP_INFO_EXT; }
template <> inline VkStructureType GetSType<VkMicromapBuildSizesInfoEXT>() { return VK_STRUCTURE_TYPE_MICROMAP_BUILD_SIZES_INFO_EXT; }
template <> inline VkStructureType GetSType<VkAccelerationStructureTrianglesOpacityMicromapEXT>() { return VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_OPACITY_MICROMAP_EXT; }
#ifdef VK_ENABLE_BETA_EXTENSIONS
template <> inline VkStructureType GetSType<VkPhysicalDeviceDisplacementMicromapFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISPLACEMENT_MICROMAP_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDisplacementMicromapPropertiesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISPLACEMENT_MICROMAP_PROPERTIES_NV; }
template <> inline VkStructureType GetSType<VkAccelerationStructureTrianglesDisplacementMicromapNV>() { return VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_DISPLACEMENT_MICROMAP_NV; }
#endif  // VK_ENABLE_BETA_EXTENSIONS
template <> inline VkStructureType GetSType<VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_FEATURES_HUAWEI; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_PROPERTIES_HUAWEI; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_VRS_FEATURES_HUAWEI; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceBorderColorSwizzleFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BORDER_COLOR_SWIZZLE_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkSamplerBorderColorComponentMappingCreateInfoEXT>() { return VK_STRUCTURE_TYPE_SAMPLER_BORDER_COLOR_COMPONENT_MAPPING_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PAGEABLE_DEVICE_LOCAL_MEMORY_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderCorePropertiesARM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_ARM; }
template <> inline VkStructureType GetSType<VkDeviceQueueShaderCoreControlCreateInfoARM>() { return VK_STRUCTURE_TYPE_DEVICE_QUEUE_SHADER_CORE_CONTROL_CREATE_INFO_ARM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceSchedulingControlsFeaturesARM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCHEDULING_CONTROLS_FEATURES_ARM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceSchedulingControlsPropertiesARM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCHEDULING_CONTROLS_PROPERTIES_ARM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_SLICED_VIEW_OF_3D_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkImageViewSlicedCreateInfoEXT>() { return VK_STRUCTURE_TYPE_IMAGE_VIEW_SLICED_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_SET_HOST_MAPPING_FEATURES_VALVE; }
template <> inline VkStructureType GetSType<VkDescriptorSetBindingReferenceVALVE>() { return VK_STRUCTURE_TYPE_DESCRIPTOR_SET_BINDING_REFERENCE_VALVE; }
template <> inline VkStructureType GetSType<VkDescriptorSetLayoutHostMappingInfoVALVE>() { return VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_HOST_MAPPING_INFO_VALVE; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NON_SEAMLESS_CUBE_MAP_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceRenderPassStripedFeaturesARM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RENDER_PASS_STRIPED_FEATURES_ARM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceRenderPassStripedPropertiesARM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RENDER_PASS_STRIPED_PROPERTIES_ARM; }
template <> inline VkStructureType GetSType<VkRenderPassStripeInfoARM>() { return VK_STRUCTURE_TYPE_RENDER_PASS_STRIPE_INFO_ARM; }
template <> inline VkStructureType GetSType<VkRenderPassStripeBeginInfoARM>() { return VK_STRUCTURE_TYPE_RENDER_PASS_STRIPE_BEGIN_INFO_ARM; }
template <> inline VkStructureType GetSType<VkRenderPassStripeSubmitInfoARM>() { return VK_STRUCTURE_TYPE_RENDER_PASS_STRIPE_SUBMIT_INFO_ARM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceFragmentDensityMapOffsetPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkRenderPassFragmentDensityMapOffsetEndInfoEXT>() { return VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_OFFSET_END_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceCopyMemoryIndirectFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceCopyMemoryIndirectPropertiesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_PROPERTIES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMemoryDecompressionFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMemoryDecompressionPropertiesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_PROPERTIES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_COMPUTE_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkComputePipelineIndirectBufferInfoNV>() { return VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_INDIRECT_BUFFER_INFO_NV; }
template <> inline VkStructureType GetSType<VkPipelineIndirectDeviceAddressInfoNV>() { return VK_STRUCTURE_TYPE_PIPELINE_INDIRECT_DEVICE_ADDRESS_INFO_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_LINEAR_SWEPT_SPHERES_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkAccelerationStructureGeometryLinearSweptSpheresDataNV>() { return VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_LINEAR_SWEPT_SPHERES_DATA_NV; }
template <> inline VkStructureType GetSType<VkAccelerationStructureGeometrySpheresDataNV>() { return VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_SPHERES_DATA_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceLinearColorAttachmentFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINEAR_COLOR_ATTACHMENT_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkImageViewSampleWeightCreateInfoQCOM>() { return VK_STRUCTURE_TYPE_IMAGE_VIEW_SAMPLE_WEIGHT_CREATE_INFO_QCOM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceImageProcessingFeaturesQCOM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_FEATURES_QCOM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceImageProcessingPropertiesQCOM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_PROPERTIES_QCOM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceNestedCommandBufferFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceNestedCommandBufferPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkExternalMemoryAcquireUnmodifiedEXT>() { return VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_ACQUIRE_UNMODIFIED_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceExtendedDynamicState3PropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_MERGE_FEEDBACK_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkRenderPassCreationControlEXT>() { return VK_STRUCTURE_TYPE_RENDER_PASS_CREATION_CONTROL_EXT; }
template <> inline VkStructureType GetSType<VkRenderPassCreationFeedbackCreateInfoEXT>() { return VK_STRUCTURE_TYPE_RENDER_PASS_CREATION_FEEDBACK_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkRenderPassSubpassFeedbackCreateInfoEXT>() { return VK_STRUCTURE_TYPE_RENDER_PASS_SUBPASS_FEEDBACK_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkDirectDriverLoadingInfoLUNARG>() { return VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_INFO_LUNARG; }
template <> inline VkStructureType GetSType<VkDirectDriverLoadingListLUNARG>() { return VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_LIST_LUNARG; }
template <> inline VkStructureType GetSType<VkTensorDescriptionARM>() { return VK_STRUCTURE_TYPE_TENSOR_DESCRIPTION_ARM; }
template <> inline VkStructureType GetSType<VkTensorCreateInfoARM>() { return VK_STRUCTURE_TYPE_TENSOR_CREATE_INFO_ARM; }
template <> inline VkStructureType GetSType<VkTensorViewCreateInfoARM>() { return VK_STRUCTURE_TYPE_TENSOR_VIEW_CREATE_INFO_ARM; }
template <> inline VkStructureType GetSType<VkTensorMemoryRequirementsInfoARM>() { return VK_STRUCTURE_TYPE_TENSOR_MEMORY_REQUIREMENTS_INFO_ARM; }
template <> inline VkStructureType GetSType<VkBindTensorMemoryInfoARM>() { return VK_STRUCTURE_TYPE_BIND_TENSOR_MEMORY_INFO_ARM; }
template <> inline VkStructureType GetSType<VkWriteDescriptorSetTensorARM>() { return VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_TENSOR_ARM; }
template <> inline VkStructureType GetSType<VkTensorFormatPropertiesARM>() { return VK_STRUCTURE_TYPE_TENSOR_FORMAT_PROPERTIES_ARM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceTensorPropertiesARM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TENSOR_PROPERTIES_ARM; }
template <> inline VkStructureType GetSType<VkTensorMemoryBarrierARM>() { return VK_STRUCTURE_TYPE_TENSOR_MEMORY_BARRIER_ARM; }
template <> inline VkStructureType GetSType<VkTensorDependencyInfoARM>() { return VK_STRUCTURE_TYPE_TENSOR_DEPENDENCY_INFO_ARM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceTensorFeaturesARM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TENSOR_FEATURES_ARM; }
template <> inline VkStructureType GetSType<VkDeviceTensorMemoryRequirementsARM>() { return VK_STRUCTURE_TYPE_DEVICE_TENSOR_MEMORY_REQUIREMENTS_ARM; }
template <> inline VkStructureType GetSType<VkTensorCopyARM>() { return VK_STRUCTURE_TYPE_TENSOR_COPY_ARM; }
template <> inline VkStructureType GetSType<VkCopyTensorInfoARM>() { return VK_STRUCTURE_TYPE_COPY_TENSOR_INFO_ARM; }
template <> inline VkStructureType GetSType<VkMemoryDedicatedAllocateInfoTensorARM>() { return VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_TENSOR_ARM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceExternalTensorInfoARM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_TENSOR_INFO_ARM; }
template <> inline VkStructureType GetSType<VkExternalTensorPropertiesARM>() { return VK_STRUCTURE_TYPE_EXTERNAL_TENSOR_PROPERTIES_ARM; }
template <> inline VkStructureType GetSType<VkExternalMemoryTensorCreateInfoARM>() { return VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_TENSOR_CREATE_INFO_ARM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDescriptorBufferTensorFeaturesARM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_TENSOR_FEATURES_ARM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDescriptorBufferTensorPropertiesARM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_TENSOR_PROPERTIES_ARM; }
template <> inline VkStructureType GetSType<VkDescriptorGetTensorInfoARM>() { return VK_STRUCTURE_TYPE_DESCRIPTOR_GET_TENSOR_INFO_ARM; }
template <> inline VkStructureType GetSType<VkTensorCaptureDescriptorDataInfoARM>() { return VK_STRUCTURE_TYPE_TENSOR_CAPTURE_DESCRIPTOR_DATA_INFO_ARM; }
template <> inline VkStructureType GetSType<VkTensorViewCaptureDescriptorDataInfoARM>() { return VK_STRUCTURE_TYPE_TENSOR_VIEW_CAPTURE_DESCRIPTOR_DATA_INFO_ARM; }
template <> inline VkStructureType GetSType<VkFrameBoundaryTensorsARM>() { return VK_STRUCTURE_TYPE_FRAME_BOUNDARY_TENSORS_ARM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkPipelineShaderStageModuleIdentifierCreateInfoEXT>() { return VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_MODULE_IDENTIFIER_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkShaderModuleIdentifierEXT>() { return VK_STRUCTURE_TYPE_SHADER_MODULE_IDENTIFIER_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceOpticalFlowFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceOpticalFlowPropertiesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_PROPERTIES_NV; }
template <> inline VkStructureType GetSType<VkOpticalFlowImageFormatInfoNV>() { return VK_STRUCTURE_TYPE_OPTICAL_FLOW_IMAGE_FORMAT_INFO_NV; }
template <> inline VkStructureType GetSType<VkOpticalFlowImageFormatPropertiesNV>() { return VK_STRUCTURE_TYPE_OPTICAL_FLOW_IMAGE_FORMAT_PROPERTIES_NV; }
template <> inline VkStructureType GetSType<VkOpticalFlowSessionCreateInfoNV>() { return VK_STRUCTURE_TYPE_OPTICAL_FLOW_SESSION_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkOpticalFlowSessionCreatePrivateDataInfoNV>() { return VK_STRUCTURE_TYPE_OPTICAL_FLOW_SESSION_CREATE_PRIVATE_DATA_INFO_NV; }
template <> inline VkStructureType GetSType<VkOpticalFlowExecuteInfoNV>() { return VK_STRUCTURE_TYPE_OPTICAL_FLOW_EXECUTE_INFO_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceLegacyDitheringFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_DITHERING_FEATURES_EXT; }
#ifdef VK_USE_PLATFORM_ANDROID_KHR
template <> inline VkStructureType GetSType<VkPhysicalDeviceExternalFormatResolveFeaturesANDROID>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_FEATURES_ANDROID; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceExternalFormatResolvePropertiesANDROID>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_PROPERTIES_ANDROID; }
template <> inline VkStructureType GetSType<VkAndroidHardwareBufferFormatResolvePropertiesANDROID>() { return VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_RESOLVE_PROPERTIES_ANDROID; }
#endif  // VK_USE_PLATFORM_ANDROID_KHR
template <> inline VkStructureType GetSType<VkPhysicalDeviceAntiLagFeaturesAMD>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ANTI_LAG_FEATURES_AMD; }
template <> inline VkStructureType GetSType<VkAntiLagPresentationInfoAMD>() { return VK_STRUCTURE_TYPE_ANTI_LAG_PRESENTATION_INFO_AMD; }
template <> inline VkStructureType GetSType<VkAntiLagDataAMD>() { return VK_STRUCTURE_TYPE_ANTI_LAG_DATA_AMD; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderObjectFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderObjectPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkShaderCreateInfoEXT>() { return VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceTilePropertiesFeaturesQCOM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_PROPERTIES_FEATURES_QCOM; }
template <> inline VkStructureType GetSType<VkTilePropertiesQCOM>() { return VK_STRUCTURE_TYPE_TILE_PROPERTIES_QCOM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceAmigoProfilingFeaturesSEC>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_AMIGO_PROFILING_FEATURES_SEC; }
template <> inline VkStructureType GetSType<VkAmigoProfilingSubmitInfoSEC>() { return VK_STRUCTURE_TYPE_AMIGO_PROFILING_SUBMIT_INFO_SEC; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_VIEWPORTS_FEATURES_QCOM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_PROPERTIES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceCooperativeVectorPropertiesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_VECTOR_PROPERTIES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceCooperativeVectorFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_VECTOR_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkCooperativeVectorPropertiesNV>() { return VK_STRUCTURE_TYPE_COOPERATIVE_VECTOR_PROPERTIES_NV; }
template <> inline VkStructureType GetSType<VkConvertCooperativeVectorMatrixInfoNV>() { return VK_STRUCTURE_TYPE_CONVERT_COOPERATIVE_VECTOR_MATRIX_INFO_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_SPARSE_ADDRESS_SPACE_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_SPARSE_ADDRESS_SPACE_PROPERTIES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_VERTEX_ATTRIBUTES_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_VERTEX_ATTRIBUTES_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkLayerSettingsCreateInfoEXT>() { return VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_BUILTINS_FEATURES_ARM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_BUILTINS_PROPERTIES_ARM; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_LIBRARY_GROUP_HANDLES_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkLatencySleepModeInfoNV>() { return VK_STRUCTURE_TYPE_LATENCY_SLEEP_MODE_INFO_NV; }
template <> inline VkStructureType GetSType<VkLatencySleepInfoNV>() { return VK_STRUCTURE_TYPE_LATENCY_SLEEP_INFO_NV; }
template <> inline VkStructureType GetSType<VkSetLatencyMarkerInfoNV>() { return VK_STRUCTURE_TYPE_SET_LATENCY_MARKER_INFO_NV; }
template <> inline VkStructureType GetSType<VkLatencyTimingsFrameReportNV>() { return VK_STRUCTURE_TYPE_LATENCY_TIMINGS_FRAME_REPORT_NV; }
template <> inline VkStructureType GetSType<VkGetLatencyMarkerInfoNV>() { return VK_STRUCTURE_TYPE_GET_LATENCY_MARKER_INFO_NV; }
template <> inline VkStructureType GetSType<VkLatencySubmissionPresentIdNV>() { return VK_STRUCTURE_TYPE_LATENCY_SUBMISSION_PRESENT_ID_NV; }
template <> inline VkStructureType GetSType<VkSwapchainLatencyCreateInfoNV>() { return VK_STRUCTURE_TYPE_SWAPCHAIN_LATENCY_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkOutOfBandQueueTypeInfoNV>() { return VK_STRUCTURE_TYPE_OUT_OF_BAND_QUEUE_TYPE_INFO_NV; }
template <> inline VkStructureType GetSType<VkLatencySurfaceCapabilitiesNV>() { return VK_STRUCTURE_TYPE_LATENCY_SURFACE_CAPABILITIES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDataGraphFeaturesARM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DATA_GRAPH_FEATURES_ARM; }
template <> inline VkStructureType GetSType<VkDataGraphPipelineConstantARM>() { return VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_CONSTANT_ARM; }
template <> inline VkStructureType GetSType<VkDataGraphPipelineResourceInfoARM>() { return VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_RESOURCE_INFO_ARM; }
template <> inline VkStructureType GetSType<VkDataGraphPipelineCompilerControlCreateInfoARM>() { return VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_COMPILER_CONTROL_CREATE_INFO_ARM; }
template <> inline VkStructureType GetSType<VkDataGraphPipelineCreateInfoARM>() { return VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_CREATE_INFO_ARM; }
template <> inline VkStructureType GetSType<VkDataGraphPipelineShaderModuleCreateInfoARM>() { return VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_SHADER_MODULE_CREATE_INFO_ARM; }
template <> inline VkStructureType GetSType<VkDataGraphPipelineSessionCreateInfoARM>() { return VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_SESSION_CREATE_INFO_ARM; }
template <> inline VkStructureType GetSType<VkDataGraphPipelineSessionBindPointRequirementsInfoARM>() { return VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_SESSION_BIND_POINT_REQUIREMENTS_INFO_ARM; }
template <> inline VkStructureType GetSType<VkDataGraphPipelineSessionBindPointRequirementARM>() { return VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_SESSION_BIND_POINT_REQUIREMENT_ARM; }
template <> inline VkStructureType GetSType<VkDataGraphPipelineSessionMemoryRequirementsInfoARM>() { return VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_SESSION_MEMORY_REQUIREMENTS_INFO_ARM; }
template <> inline VkStructureType GetSType<VkBindDataGraphPipelineSessionMemoryInfoARM>() { return VK_STRUCTURE_TYPE_BIND_DATA_GRAPH_PIPELINE_SESSION_MEMORY_INFO_ARM; }
template <> inline VkStructureType GetSType<VkDataGraphPipelineInfoARM>() { return VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_INFO_ARM; }
template <> inline VkStructureType GetSType<VkDataGraphPipelinePropertyQueryResultARM>() { return VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_PROPERTY_QUERY_RESULT_ARM; }
template <> inline VkStructureType GetSType<VkDataGraphPipelineIdentifierCreateInfoARM>() { return VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_IDENTIFIER_CREATE_INFO_ARM; }
template <> inline VkStructureType GetSType<VkDataGraphPipelineDispatchInfoARM>() { return VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_DISPATCH_INFO_ARM; }
template <> inline VkStructureType GetSType<VkQueueFamilyDataGraphPropertiesARM>() { return VK_STRUCTURE_TYPE_QUEUE_FAMILY_DATA_GRAPH_PROPERTIES_ARM; }
template <> inline VkStructureType GetSType<VkDataGraphProcessingEngineCreateInfoARM>() { return VK_STRUCTURE_TYPE_DATA_GRAPH_PROCESSING_ENGINE_CREATE_INFO_ARM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceQueueFamilyDataGraphProcessingEngineInfoARM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_QUEUE_FAMILY_DATA_GRAPH_PROCESSING_ENGINE_INFO_ARM; }
template <> inline VkStructureType GetSType<VkQueueFamilyDataGraphProcessingEnginePropertiesARM>() { return VK_STRUCTURE_TYPE_QUEUE_FAMILY_DATA_GRAPH_PROCESSING_ENGINE_PROPERTIES_ARM; }
template <> inline VkStructureType GetSType<VkDataGraphPipelineConstantTensorSemiStructuredSparsityInfoARM>() { return VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_CONSTANT_TENSOR_SEMI_STRUCTURED_SPARSITY_INFO_ARM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_RENDER_AREAS_FEATURES_QCOM; }
template <> inline VkStructureType GetSType<VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM>() { return VK_STRUCTURE_TYPE_MULTIVIEW_PER_VIEW_RENDER_AREAS_RENDER_PASS_BEGIN_INFO_QCOM; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePerStageDescriptorSetFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PER_STAGE_DESCRIPTOR_SET_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceImageProcessing2FeaturesQCOM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_2_FEATURES_QCOM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceImageProcessing2PropertiesQCOM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_2_PROPERTIES_QCOM; }
template <> inline VkStructureType GetSType<VkSamplerBlockMatchWindowCreateInfoQCOM>() { return VK_STRUCTURE_TYPE_SAMPLER_BLOCK_MATCH_WINDOW_CREATE_INFO_QCOM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceCubicWeightsFeaturesQCOM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_WEIGHTS_FEATURES_QCOM; }
template <> inline VkStructureType GetSType<VkSamplerCubicWeightsCreateInfoQCOM>() { return VK_STRUCTURE_TYPE_SAMPLER_CUBIC_WEIGHTS_CREATE_INFO_QCOM; }
template <> inline VkStructureType GetSType<VkBlitImageCubicWeightsInfoQCOM>() { return VK_STRUCTURE_TYPE_BLIT_IMAGE_CUBIC_WEIGHTS_INFO_QCOM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceYcbcrDegammaFeaturesQCOM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_DEGAMMA_FEATURES_QCOM; }
template <> inline VkStructureType GetSType<VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM>() { return VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_YCBCR_DEGAMMA_CREATE_INFO_QCOM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceCubicClampFeaturesQCOM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_CLAMP_FEATURES_QCOM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_FEATURES_EXT; }
#ifdef VK_USE_PLATFORM_SCREEN_QNX
template <> inline VkStructureType GetSType<VkScreenBufferPropertiesQNX>() { return VK_STRUCTURE_TYPE_SCREEN_BUFFER_PROPERTIES_QNX; }
template <> inline VkStructureType GetSType<VkScreenBufferFormatPropertiesQNX>() { return VK_STRUCTURE_TYPE_SCREEN_BUFFER_FORMAT_PROPERTIES_QNX; }
template <> inline VkStructureType GetSType<VkImportScreenBufferInfoQNX>() { return VK_STRUCTURE_TYPE_IMPORT_SCREEN_BUFFER_INFO_QNX; }
template <> inline VkStructureType GetSType<VkExternalFormatQNX>() { return VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_QNX; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_SCREEN_BUFFER_FEATURES_QNX; }
#endif  // VK_USE_PLATFORM_SCREEN_QNX
template <> inline VkStructureType GetSType<VkPhysicalDeviceLayeredDriverPropertiesMSFT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_DRIVER_PROPERTIES_MSFT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_POOL_OVERALLOCATION_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceTileMemoryHeapFeaturesQCOM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_MEMORY_HEAP_FEATURES_QCOM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceTileMemoryHeapPropertiesQCOM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_MEMORY_HEAP_PROPERTIES_QCOM; }
template <> inline VkStructureType GetSType<VkTileMemoryRequirementsQCOM>() { return VK_STRUCTURE_TYPE_TILE_MEMORY_REQUIREMENTS_QCOM; }
template <> inline VkStructureType GetSType<VkTileMemoryBindInfoQCOM>() { return VK_STRUCTURE_TYPE_TILE_MEMORY_BIND_INFO_QCOM; }
template <> inline VkStructureType GetSType<VkTileMemorySizeInfoQCOM>() { return VK_STRUCTURE_TYPE_TILE_MEMORY_SIZE_INFO_QCOM; }
template <> inline VkStructureType GetSType<VkDisplaySurfaceStereoCreateInfoNV>() { return VK_STRUCTURE_TYPE_DISPLAY_SURFACE_STEREO_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkDisplayModeStereoPropertiesNV>() { return VK_STRUCTURE_TYPE_DISPLAY_MODE_STEREO_PROPERTIES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceRawAccessChainsFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAW_ACCESS_CHAINS_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkExternalComputeQueueDeviceCreateInfoNV>() { return VK_STRUCTURE_TYPE_EXTERNAL_COMPUTE_QUEUE_DEVICE_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkExternalComputeQueueCreateInfoNV>() { return VK_STRUCTURE_TYPE_EXTERNAL_COMPUTE_QUEUE_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkExternalComputeQueueDataParamsNV>() { return VK_STRUCTURE_TYPE_EXTERNAL_COMPUTE_QUEUE_DATA_PARAMS_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceExternalComputeQueuePropertiesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_COMPUTE_QUEUE_PROPERTIES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceCommandBufferInheritanceFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMMAND_BUFFER_INHERITANCE_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT16_VECTOR_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_REPLICATED_COMPOSITES_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceShaderFloat8FeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT8_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceRayTracingValidationFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_VALIDATION_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceClusterAccelerationStructureFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_ACCELERATION_STRUCTURE_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceClusterAccelerationStructurePropertiesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_ACCELERATION_STRUCTURE_PROPERTIES_NV; }
template <> inline VkStructureType GetSType<VkClusterAccelerationStructureClustersBottomLevelInputNV>() { return VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_CLUSTERS_BOTTOM_LEVEL_INPUT_NV; }
template <> inline VkStructureType GetSType<VkClusterAccelerationStructureTriangleClusterInputNV>() { return VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_TRIANGLE_CLUSTER_INPUT_NV; }
template <> inline VkStructureType GetSType<VkClusterAccelerationStructureMoveObjectsInputNV>() { return VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_MOVE_OBJECTS_INPUT_NV; }
template <> inline VkStructureType GetSType<VkClusterAccelerationStructureInputInfoNV>() { return VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_INPUT_INFO_NV; }
template <> inline VkStructureType GetSType<VkClusterAccelerationStructureCommandsInfoNV>() { return VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_COMMANDS_INFO_NV; }
template <> inline VkStructureType GetSType<VkAccelerationStructureBuildSizesInfoKHR>() { return VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR; }
template <> inline VkStructureType GetSType<VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV>() { return VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CLUSTER_ACCELERATION_STRUCTURE_CREATE_INFO_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PARTITIONED_ACCELERATION_STRUCTURE_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PARTITIONED_ACCELERATION_STRUCTURE_PROPERTIES_NV; }
template <> inline VkStructureType GetSType<VkPartitionedAccelerationStructureFlagsNV>() { return VK_STRUCTURE_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_FLAGS_NV; }
template <> inline VkStructureType GetSType<VkWriteDescriptorSetPartitionedAccelerationStructureNV>() { return VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_PARTITIONED_ACCELERATION_STRUCTURE_NV; }
template <> inline VkStructureType GetSType<VkPartitionedAccelerationStructureInstancesInputNV>() { return VK_STRUCTURE_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_INSTANCES_INPUT_NV; }
template <> inline VkStructureType GetSType<VkBuildPartitionedAccelerationStructureInfoNV>() { return VK_STRUCTURE_TYPE_BUILD_PARTITIONED_ACCELERATION_STRUCTURE_INFO_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkGeneratedCommandsMemoryRequirementsInfoEXT>() { return VK_STRUCTURE_TYPE_GENERATED_COMMANDS_MEMORY_REQUIREMENTS_INFO_EXT; }
template <> inline VkStructureType GetSType<VkIndirectExecutionSetPipelineInfoEXT>() { return VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_PIPELINE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkIndirectExecutionSetShaderLayoutInfoEXT>() { return VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_SHADER_LAYOUT_INFO_EXT; }
template <> inline VkStructureType GetSType<VkIndirectExecutionSetShaderInfoEXT>() { return VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_SHADER_INFO_EXT; }
template <> inline VkStructureType GetSType<VkIndirectExecutionSetCreateInfoEXT>() { return VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkGeneratedCommandsInfoEXT>() { return VK_STRUCTURE_TYPE_GENERATED_COMMANDS_INFO_EXT; }
template <> inline VkStructureType GetSType<VkWriteIndirectExecutionSetPipelineEXT>() { return VK_STRUCTURE_TYPE_WRITE_INDIRECT_EXECUTION_SET_PIPELINE_EXT; }
template <> inline VkStructureType GetSType<VkIndirectCommandsLayoutTokenEXT>() { return VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_TOKEN_EXT; }
template <> inline VkStructureType GetSType<VkIndirectCommandsLayoutCreateInfoEXT>() { return VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_CREATE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkGeneratedCommandsPipelineInfoEXT>() { return VK_STRUCTURE_TYPE_GENERATED_COMMANDS_PIPELINE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkGeneratedCommandsShaderInfoEXT>() { return VK_STRUCTURE_TYPE_GENERATED_COMMANDS_SHADER_INFO_EXT; }
template <> inline VkStructureType GetSType<VkWriteIndirectExecutionSetShaderEXT>() { return VK_STRUCTURE_TYPE_WRITE_INDIRECT_EXECUTION_SET_SHADER_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceImageAlignmentControlFeaturesMESA>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ALIGNMENT_CONTROL_FEATURES_MESA; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceImageAlignmentControlPropertiesMESA>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ALIGNMENT_CONTROL_PROPERTIES_MESA; }
template <> inline VkStructureType GetSType<VkImageAlignmentControlCreateInfoMESA>() { return VK_STRUCTURE_TYPE_IMAGE_ALIGNMENT_CONTROL_CREATE_INFO_MESA; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceDepthClampControlFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_CONTROL_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPipelineViewportDepthClampControlCreateInfoEXT>() { return VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLAMP_CONTROL_CREATE_INFO_EXT; }
#ifdef VK_USE_PLATFORM_OHOS
template <> inline VkStructureType GetSType<VkOHSurfaceCreateInfoOHOS>() { return VK_STRUCTURE_TYPE_OH_SURFACE_CREATE_INFO_OHOS; }
#endif  // VK_USE_PLATFORM_OHOS
template <> inline VkStructureType GetSType<VkPhysicalDeviceHdrVividFeaturesHUAWEI>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HDR_VIVID_FEATURES_HUAWEI; }
template <> inline VkStructureType GetSType<VkHdrVividDynamicMetadataHUAWEI>() { return VK_STRUCTURE_TYPE_HDR_VIVID_DYNAMIC_METADATA_HUAWEI; }
template <> inline VkStructureType GetSType<VkCooperativeMatrixFlexibleDimensionsPropertiesNV>() { return VK_STRUCTURE_TYPE_COOPERATIVE_MATRIX_FLEXIBLE_DIMENSIONS_PROPERTIES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceCooperativeMatrix2FeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_2_FEATURES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceCooperativeMatrix2PropertiesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_2_PROPERTIES_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePipelineOpacityMicromapFeaturesARM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_OPACITY_MICROMAP_FEATURES_ARM; }
#ifdef VK_USE_PLATFORM_METAL_EXT
template <> inline VkStructureType GetSType<VkImportMemoryMetalHandleInfoEXT>() { return VK_STRUCTURE_TYPE_IMPORT_MEMORY_METAL_HANDLE_INFO_EXT; }
template <> inline VkStructureType GetSType<VkMemoryMetalHandlePropertiesEXT>() { return VK_STRUCTURE_TYPE_MEMORY_METAL_HANDLE_PROPERTIES_EXT; }
template <> inline VkStructureType GetSType<VkMemoryGetMetalHandleInfoEXT>() { return VK_STRUCTURE_TYPE_MEMORY_GET_METAL_HANDLE_INFO_EXT; }
#endif  // VK_USE_PLATFORM_METAL_EXT
template <> inline VkStructureType GetSType<VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_ROBUSTNESS_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceFormatPackFeaturesARM>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FORMAT_PACK_FEATURES_ARM; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceFragmentDensityMapLayeredFeaturesVALVE>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_LAYERED_FEATURES_VALVE; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceFragmentDensityMapLayeredPropertiesVALVE>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_LAYERED_PROPERTIES_VALVE; }
template <> inline VkStructureType GetSType<VkPipelineFragmentDensityMapLayeredCreateInfoVALVE>() { return VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_DENSITY_MAP_LAYERED_CREATE_INFO_VALVE; }
#ifdef VK_ENABLE_BETA_EXTENSIONS
template <> inline VkStructureType GetSType<VkSetPresentConfigNV>() { return VK_STRUCTURE_TYPE_SET_PRESENT_CONFIG_NV; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePresentMeteringFeaturesNV>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_METERING_FEATURES_NV; }
#endif  // VK_ENABLE_BETA_EXTENSIONS
template <> inline VkStructureType GetSType<VkRenderingEndInfoEXT>() { return VK_STRUCTURE_TYPE_RENDERING_END_INFO_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_DEVICE_MEMORY_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDevicePipelineCacheIncrementalModeFeaturesSEC>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CACHE_INCREMENTAL_MODE_FEATURES_SEC; }
template <> inline VkStructureType GetSType<VkAccelerationStructureGeometryTrianglesDataKHR>() { return VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR; }
template <> inline VkStructureType GetSType<VkAccelerationStructureGeometryAabbsDataKHR>() { return VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR; }
template <> inline VkStructureType GetSType<VkAccelerationStructureGeometryInstancesDataKHR>() { return VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR; }
template <> inline VkStructureType GetSType<VkAccelerationStructureGeometryKHR>() { return VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR; }
template <> inline VkStructureType GetSType<VkAccelerationStructureBuildGeometryInfoKHR>() { return VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR; }
template <> inline VkStructureType GetSType<VkAccelerationStructureCreateInfoKHR>() { return VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkWriteDescriptorSetAccelerationStructureKHR>() { return VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceAccelerationStructureFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceAccelerationStructurePropertiesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkAccelerationStructureDeviceAddressInfoKHR>() { return VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR; }
template <> inline VkStructureType GetSType<VkAccelerationStructureVersionInfoKHR>() { return VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_VERSION_INFO_KHR; }
template <> inline VkStructureType GetSType<VkCopyAccelerationStructureToMemoryInfoKHR>() { return VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_TO_MEMORY_INFO_KHR; }
template <> inline VkStructureType GetSType<VkCopyMemoryToAccelerationStructureInfoKHR>() { return VK_STRUCTURE_TYPE_COPY_MEMORY_TO_ACCELERATION_STRUCTURE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkCopyAccelerationStructureInfoKHR>() { return VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkRayTracingShaderGroupCreateInfoKHR>() { return VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkRayTracingPipelineInterfaceCreateInfoKHR>() { return VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_INTERFACE_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkRayTracingPipelineCreateInfoKHR>() { return VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceRayQueryFeaturesKHR>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMeshShaderFeaturesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT; }
template <> inline VkStructureType GetSType<VkPhysicalDeviceMeshShaderPropertiesEXT>() { return VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT; }

// Find an entry of the given type in the const pNext chain
// returns nullptr if the entry is not found
template <typename T> const T *FindStructInPNextChain(const void *next) {
    const VkBaseOutStructure *current = reinterpret_cast<const VkBaseOutStructure *>(next);
    VkStructureType desired_sType = GetSType<T>();
    while (current) {
        if (desired_sType == current->sType) {
            return reinterpret_cast<const T*>(current);
        }
        current = current->pNext;
    }
    return nullptr;
}

// Find an entry of the given type in the non-const pNext chain
// returns nullptr if the entry is not found
template <typename T> T *FindStructInPNextChain(void *next) {
    VkBaseOutStructure *current = reinterpret_cast<VkBaseOutStructure *>(next);
    VkStructureType desired_sType = GetSType<T>();
    while (current) {
        if (desired_sType == current->sType) {
            return reinterpret_cast<T*>(current);
        }
        current = current->pNext;
    }
    return nullptr;
}

// Find last element of pNext chain
inline VkBaseOutStructure *FindLastStructInPNextChain(void *next) {
    auto *current = reinterpret_cast<VkBaseOutStructure *>(next);
    auto *prev = current;
    while (current) {
        prev = current;
        current = reinterpret_cast<VkBaseOutStructure *>(current->pNext);
    }
    return prev;
}

// Init the header of an sType struct
template <typename T>
T InitStruct(void *p_next = nullptr) {
    T out = {};
    out.sType = GetSType<T>();
    out.pNext = p_next;
    return out;
}

// Init the header of an sType struct with pNext and optional fields
template <typename T, typename... StructFields>
T InitStruct(void *p_next, StructFields... fields) {
    T out = {GetSType<T>(), p_next, fields...};
    return out;
}

class InitStructHelper {
    void* p_next = nullptr;
  public:
    InitStructHelper() = default;
    InitStructHelper(void *p_next) : p_next(p_next) {}

    template <typename T>
    operator T() {
        return InitStruct<T>(p_next);
    }
};
#if VK_USE_64_BIT_PTR_DEFINES == 1
template<typename T> VkObjectType GetObjectType() {
    static_assert(sizeof(T) == 0, "GetObjectType() is being used with an unsupported Type! Is the code-gen up to date?");
    return VK_OBJECT_TYPE_UNKNOWN;
}

template<> inline VkObjectType GetObjectType<VkBuffer>() { return VK_OBJECT_TYPE_BUFFER; }
template<> inline VkObjectType GetObjectType<VkImage>() { return VK_OBJECT_TYPE_IMAGE; }
template<> inline VkObjectType GetObjectType<VkInstance>() { return VK_OBJECT_TYPE_INSTANCE; }
template<> inline VkObjectType GetObjectType<VkPhysicalDevice>() { return VK_OBJECT_TYPE_PHYSICAL_DEVICE; }
template<> inline VkObjectType GetObjectType<VkDevice>() { return VK_OBJECT_TYPE_DEVICE; }
template<> inline VkObjectType GetObjectType<VkQueue>() { return VK_OBJECT_TYPE_QUEUE; }
template<> inline VkObjectType GetObjectType<VkSemaphore>() { return VK_OBJECT_TYPE_SEMAPHORE; }
template<> inline VkObjectType GetObjectType<VkCommandBuffer>() { return VK_OBJECT_TYPE_COMMAND_BUFFER; }
template<> inline VkObjectType GetObjectType<VkFence>() { return VK_OBJECT_TYPE_FENCE; }
template<> inline VkObjectType GetObjectType<VkDeviceMemory>() { return VK_OBJECT_TYPE_DEVICE_MEMORY; }
template<> inline VkObjectType GetObjectType<VkEvent>() { return VK_OBJECT_TYPE_EVENT; }
template<> inline VkObjectType GetObjectType<VkQueryPool>() { return VK_OBJECT_TYPE_QUERY_POOL; }
template<> inline VkObjectType GetObjectType<VkBufferView>() { return VK_OBJECT_TYPE_BUFFER_VIEW; }
template<> inline VkObjectType GetObjectType<VkImageView>() { return VK_OBJECT_TYPE_IMAGE_VIEW; }
template<> inline VkObjectType GetObjectType<VkShaderModule>() { return VK_OBJECT_TYPE_SHADER_MODULE; }
template<> inline VkObjectType GetObjectType<VkPipelineCache>() { return VK_OBJECT_TYPE_PIPELINE_CACHE; }
template<> inline VkObjectType GetObjectType<VkPipelineLayout>() { return VK_OBJECT_TYPE_PIPELINE_LAYOUT; }
template<> inline VkObjectType GetObjectType<VkPipeline>() { return VK_OBJECT_TYPE_PIPELINE; }
template<> inline VkObjectType GetObjectType<VkRenderPass>() { return VK_OBJECT_TYPE_RENDER_PASS; }
template<> inline VkObjectType GetObjectType<VkDescriptorSetLayout>() { return VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT; }
template<> inline VkObjectType GetObjectType<VkSampler>() { return VK_OBJECT_TYPE_SAMPLER; }
template<> inline VkObjectType GetObjectType<VkDescriptorSet>() { return VK_OBJECT_TYPE_DESCRIPTOR_SET; }
template<> inline VkObjectType GetObjectType<VkDescriptorPool>() { return VK_OBJECT_TYPE_DESCRIPTOR_POOL; }
template<> inline VkObjectType GetObjectType<VkFramebuffer>() { return VK_OBJECT_TYPE_FRAMEBUFFER; }
template<> inline VkObjectType GetObjectType<VkCommandPool>() { return VK_OBJECT_TYPE_COMMAND_POOL; }
template<> inline VkObjectType GetObjectType<VkSamplerYcbcrConversion>() { return VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION; }
template<> inline VkObjectType GetObjectType<VkDescriptorUpdateTemplate>() { return VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE; }
template<> inline VkObjectType GetObjectType<VkPrivateDataSlot>() { return VK_OBJECT_TYPE_PRIVATE_DATA_SLOT; }
template<> inline VkObjectType GetObjectType<VkSurfaceKHR>() { return VK_OBJECT_TYPE_SURFACE_KHR; }
template<> inline VkObjectType GetObjectType<VkSwapchainKHR>() { return VK_OBJECT_TYPE_SWAPCHAIN_KHR; }
template<> inline VkObjectType GetObjectType<VkDisplayKHR>() { return VK_OBJECT_TYPE_DISPLAY_KHR; }
template<> inline VkObjectType GetObjectType<VkDisplayModeKHR>() { return VK_OBJECT_TYPE_DISPLAY_MODE_KHR; }
template<> inline VkObjectType GetObjectType<VkVideoSessionKHR>() { return VK_OBJECT_TYPE_VIDEO_SESSION_KHR; }
template<> inline VkObjectType GetObjectType<VkVideoSessionParametersKHR>() { return VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR; }
template<> inline VkObjectType GetObjectType<VkDeferredOperationKHR>() { return VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR; }
template<> inline VkObjectType GetObjectType<VkPipelineBinaryKHR>() { return VK_OBJECT_TYPE_PIPELINE_BINARY_KHR; }
template<> inline VkObjectType GetObjectType<VkDebugReportCallbackEXT>() { return VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT; }
template<> inline VkObjectType GetObjectType<VkCuModuleNVX>() { return VK_OBJECT_TYPE_CU_MODULE_NVX; }
template<> inline VkObjectType GetObjectType<VkCuFunctionNVX>() { return VK_OBJECT_TYPE_CU_FUNCTION_NVX; }
template<> inline VkObjectType GetObjectType<VkDebugUtilsMessengerEXT>() { return VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT; }
template<> inline VkObjectType GetObjectType<VkValidationCacheEXT>() { return VK_OBJECT_TYPE_VALIDATION_CACHE_EXT; }
template<> inline VkObjectType GetObjectType<VkAccelerationStructureNV>() { return VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV; }
template<> inline VkObjectType GetObjectType<VkPerformanceConfigurationINTEL>() { return VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL; }
template<> inline VkObjectType GetObjectType<VkIndirectCommandsLayoutNV>() { return VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV; }
#ifdef VK_ENABLE_BETA_EXTENSIONS
template<> inline VkObjectType GetObjectType<VkCudaModuleNV>() { return VK_OBJECT_TYPE_CUDA_MODULE_NV; }
template<> inline VkObjectType GetObjectType<VkCudaFunctionNV>() { return VK_OBJECT_TYPE_CUDA_FUNCTION_NV; }
#endif  // VK_ENABLE_BETA_EXTENSIONS
template<> inline VkObjectType GetObjectType<VkAccelerationStructureKHR>() { return VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR; }
#ifdef VK_USE_PLATFORM_FUCHSIA
template<> inline VkObjectType GetObjectType<VkBufferCollectionFUCHSIA>() { return VK_OBJECT_TYPE_BUFFER_COLLECTION_FUCHSIA; }
#endif  // VK_USE_PLATFORM_FUCHSIA
template<> inline VkObjectType GetObjectType<VkMicromapEXT>() { return VK_OBJECT_TYPE_MICROMAP_EXT; }
template<> inline VkObjectType GetObjectType<VkTensorARM>() { return VK_OBJECT_TYPE_TENSOR_ARM; }
template<> inline VkObjectType GetObjectType<VkTensorViewARM>() { return VK_OBJECT_TYPE_TENSOR_VIEW_ARM; }
template<> inline VkObjectType GetObjectType<VkOpticalFlowSessionNV>() { return VK_OBJECT_TYPE_OPTICAL_FLOW_SESSION_NV; }
template<> inline VkObjectType GetObjectType<VkShaderEXT>() { return VK_OBJECT_TYPE_SHADER_EXT; }
template<> inline VkObjectType GetObjectType<VkDataGraphPipelineSessionARM>() { return VK_OBJECT_TYPE_DATA_GRAPH_PIPELINE_SESSION_ARM; }
template<> inline VkObjectType GetObjectType<VkExternalComputeQueueNV>() { return VK_OBJECT_TYPE_EXTERNAL_COMPUTE_QUEUE_NV; }
template<> inline VkObjectType GetObjectType<VkIndirectExecutionSetEXT>() { return VK_OBJECT_TYPE_INDIRECT_EXECUTION_SET_EXT; }
template<> inline VkObjectType GetObjectType<VkIndirectCommandsLayoutEXT>() { return VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_EXT; }

# else // 32 bit
template<typename T> VkObjectType GetObjectType() {
    static_assert(sizeof(T) == 0, "GetObjectType() does not support 32 bit builds! This is a limitation of the vulkan.h headers");
    return VK_OBJECT_TYPE_UNKNOWN;
}
# endif // VK_USE_64_BIT_PTR_DEFINES == 1
} // namespace vku

// NOLINTEND// clang-format on
