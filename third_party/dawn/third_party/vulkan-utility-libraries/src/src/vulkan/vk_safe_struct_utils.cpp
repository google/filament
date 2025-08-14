// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See safe_struct_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

// NOLINTBEGIN

#include <vulkan/vk_layer.h>
#include <vulkan/utility/vk_safe_struct.hpp>

#include <vector>
#include <cstring>

namespace vku {
char *SafeStringCopy(const char *in_string) {
    if (nullptr == in_string) return nullptr;
    size_t len = std::strlen(in_string);
    char *dest = new char[len + 1];
    dest[len] = '\0';
    std::memcpy(dest, in_string, len);
    return dest;
}

// clang-format off
void *SafePnextCopy(const void *pNext, PNextCopyState* copy_state) {
    void *first_pNext{};
    VkBaseOutStructure *prev_pNext{};
    void *safe_pNext{};

    while (pNext) {
        const VkBaseOutStructure *header = reinterpret_cast<const VkBaseOutStructure *>(pNext);

        switch (header->sType) {
            // Add special-case code to copy beloved secret loader structs
            // Special-case Loader Instance Struct passed to/from layer in pNext chain
            case VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO: {
                VkLayerInstanceCreateInfo *struct_copy = new VkLayerInstanceCreateInfo;
                // TODO: Uses original VkLayerInstanceLink* chain, which should be okay for our uses
                memcpy(struct_copy, pNext, sizeof(VkLayerInstanceCreateInfo));
                safe_pNext = struct_copy;
                break;
            }
            // Special-case Loader Device Struct passed to/from layer in pNext chain
            case VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO: {
                VkLayerDeviceCreateInfo *struct_copy = new VkLayerDeviceCreateInfo;
                // TODO: Uses original VkLayerDeviceLink*, which should be okay for our uses
                memcpy(struct_copy, pNext, sizeof(VkLayerDeviceCreateInfo));
                safe_pNext = struct_copy;
                break;
            }
            case VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO:
                safe_pNext = new safe_VkShaderModuleCreateInfo(reinterpret_cast<const VkShaderModuleCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO:
                safe_pNext = new safe_VkComputePipelineCreateInfo(reinterpret_cast<const VkComputePipelineCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO:
                safe_pNext = new safe_VkGraphicsPipelineCreateInfo(reinterpret_cast<const VkGraphicsPipelineCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO:
                safe_pNext = new safe_VkPipelineLayoutCreateInfo(reinterpret_cast<const VkPipelineLayoutCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceSubgroupProperties(reinterpret_cast<const VkPhysicalDeviceSubgroupProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES:
                safe_pNext = new safe_VkPhysicalDevice16BitStorageFeatures(reinterpret_cast<const VkPhysicalDevice16BitStorageFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS:
                safe_pNext = new safe_VkMemoryDedicatedRequirements(reinterpret_cast<const VkMemoryDedicatedRequirements *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO:
                safe_pNext = new safe_VkMemoryDedicatedAllocateInfo(reinterpret_cast<const VkMemoryDedicatedAllocateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO:
                safe_pNext = new safe_VkMemoryAllocateFlagsInfo(reinterpret_cast<const VkMemoryAllocateFlagsInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO:
                safe_pNext = new safe_VkDeviceGroupRenderPassBeginInfo(reinterpret_cast<const VkDeviceGroupRenderPassBeginInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO:
                safe_pNext = new safe_VkDeviceGroupCommandBufferBeginInfo(reinterpret_cast<const VkDeviceGroupCommandBufferBeginInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO:
                safe_pNext = new safe_VkDeviceGroupSubmitInfo(reinterpret_cast<const VkDeviceGroupSubmitInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_GROUP_BIND_SPARSE_INFO:
                safe_pNext = new safe_VkDeviceGroupBindSparseInfo(reinterpret_cast<const VkDeviceGroupBindSparseInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO:
                safe_pNext = new safe_VkBindBufferMemoryDeviceGroupInfo(reinterpret_cast<const VkBindBufferMemoryDeviceGroupInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_DEVICE_GROUP_INFO:
                safe_pNext = new safe_VkBindImageMemoryDeviceGroupInfo(reinterpret_cast<const VkBindImageMemoryDeviceGroupInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO:
                safe_pNext = new safe_VkDeviceGroupDeviceCreateInfo(reinterpret_cast<const VkDeviceGroupDeviceCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2:
                safe_pNext = new safe_VkPhysicalDeviceFeatures2(reinterpret_cast<const VkPhysicalDeviceFeatures2 *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDevicePointClippingProperties(reinterpret_cast<const VkPhysicalDevicePointClippingProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO:
                safe_pNext = new safe_VkRenderPassInputAttachmentAspectCreateInfo(reinterpret_cast<const VkRenderPassInputAttachmentAspectCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO:
                safe_pNext = new safe_VkImageViewUsageCreateInfo(reinterpret_cast<const VkImageViewUsageCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO:
                safe_pNext = new safe_VkPipelineTessellationDomainOriginStateCreateInfo(reinterpret_cast<const VkPipelineTessellationDomainOriginStateCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO:
                safe_pNext = new safe_VkRenderPassMultiviewCreateInfo(reinterpret_cast<const VkRenderPassMultiviewCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceMultiviewFeatures(reinterpret_cast<const VkPhysicalDeviceMultiviewFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceMultiviewProperties(reinterpret_cast<const VkPhysicalDeviceMultiviewProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceVariablePointersFeatures(reinterpret_cast<const VkPhysicalDeviceVariablePointersFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceProtectedMemoryFeatures(reinterpret_cast<const VkPhysicalDeviceProtectedMemoryFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceProtectedMemoryProperties(reinterpret_cast<const VkPhysicalDeviceProtectedMemoryProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO:
                safe_pNext = new safe_VkProtectedSubmitInfo(reinterpret_cast<const VkProtectedSubmitInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO:
                safe_pNext = new safe_VkSamplerYcbcrConversionInfo(reinterpret_cast<const VkSamplerYcbcrConversionInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO:
                safe_pNext = new safe_VkBindImagePlaneMemoryInfo(reinterpret_cast<const VkBindImagePlaneMemoryInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO:
                safe_pNext = new safe_VkImagePlaneMemoryRequirementsInfo(reinterpret_cast<const VkImagePlaneMemoryRequirementsInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures(reinterpret_cast<const VkPhysicalDeviceSamplerYcbcrConversionFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_IMAGE_FORMAT_PROPERTIES:
                safe_pNext = new safe_VkSamplerYcbcrConversionImageFormatProperties(reinterpret_cast<const VkSamplerYcbcrConversionImageFormatProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO:
                safe_pNext = new safe_VkPhysicalDeviceExternalImageFormatInfo(reinterpret_cast<const VkPhysicalDeviceExternalImageFormatInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES:
                safe_pNext = new safe_VkExternalImageFormatProperties(reinterpret_cast<const VkExternalImageFormatProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceIDProperties(reinterpret_cast<const VkPhysicalDeviceIDProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO:
                safe_pNext = new safe_VkExternalMemoryImageCreateInfo(reinterpret_cast<const VkExternalMemoryImageCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO:
                safe_pNext = new safe_VkExternalMemoryBufferCreateInfo(reinterpret_cast<const VkExternalMemoryBufferCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO:
                safe_pNext = new safe_VkExportMemoryAllocateInfo(reinterpret_cast<const VkExportMemoryAllocateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO:
                safe_pNext = new safe_VkExportFenceCreateInfo(reinterpret_cast<const VkExportFenceCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO:
                safe_pNext = new safe_VkExportSemaphoreCreateInfo(reinterpret_cast<const VkExportSemaphoreCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceMaintenance3Properties(reinterpret_cast<const VkPhysicalDeviceMaintenance3Properties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceShaderDrawParametersFeatures(reinterpret_cast<const VkPhysicalDeviceShaderDrawParametersFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceVulkan11Features(reinterpret_cast<const VkPhysicalDeviceVulkan11Features *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceVulkan11Properties(reinterpret_cast<const VkPhysicalDeviceVulkan11Properties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceVulkan12Features(reinterpret_cast<const VkPhysicalDeviceVulkan12Features *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceVulkan12Properties(reinterpret_cast<const VkPhysicalDeviceVulkan12Properties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO:
                safe_pNext = new safe_VkImageFormatListCreateInfo(reinterpret_cast<const VkImageFormatListCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES:
                safe_pNext = new safe_VkPhysicalDevice8BitStorageFeatures(reinterpret_cast<const VkPhysicalDevice8BitStorageFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceDriverProperties(reinterpret_cast<const VkPhysicalDeviceDriverProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceShaderAtomicInt64Features(reinterpret_cast<const VkPhysicalDeviceShaderAtomicInt64Features *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceShaderFloat16Int8Features(reinterpret_cast<const VkPhysicalDeviceShaderFloat16Int8Features *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceFloatControlsProperties(reinterpret_cast<const VkPhysicalDeviceFloatControlsProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO:
                safe_pNext = new safe_VkDescriptorSetLayoutBindingFlagsCreateInfo(reinterpret_cast<const VkDescriptorSetLayoutBindingFlagsCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceDescriptorIndexingFeatures(reinterpret_cast<const VkPhysicalDeviceDescriptorIndexingFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceDescriptorIndexingProperties(reinterpret_cast<const VkPhysicalDeviceDescriptorIndexingProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO:
                safe_pNext = new safe_VkDescriptorSetVariableDescriptorCountAllocateInfo(reinterpret_cast<const VkDescriptorSetVariableDescriptorCountAllocateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_LAYOUT_SUPPORT:
                safe_pNext = new safe_VkDescriptorSetVariableDescriptorCountLayoutSupport(reinterpret_cast<const VkDescriptorSetVariableDescriptorCountLayoutSupport *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE:
                safe_pNext = new safe_VkSubpassDescriptionDepthStencilResolve(reinterpret_cast<const VkSubpassDescriptionDepthStencilResolve *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceDepthStencilResolveProperties(reinterpret_cast<const VkPhysicalDeviceDepthStencilResolveProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceScalarBlockLayoutFeatures(reinterpret_cast<const VkPhysicalDeviceScalarBlockLayoutFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO:
                safe_pNext = new safe_VkImageStencilUsageCreateInfo(reinterpret_cast<const VkImageStencilUsageCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO:
                safe_pNext = new safe_VkSamplerReductionModeCreateInfo(reinterpret_cast<const VkSamplerReductionModeCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceSamplerFilterMinmaxProperties(reinterpret_cast<const VkPhysicalDeviceSamplerFilterMinmaxProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceVulkanMemoryModelFeatures(reinterpret_cast<const VkPhysicalDeviceVulkanMemoryModelFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceImagelessFramebufferFeatures(reinterpret_cast<const VkPhysicalDeviceImagelessFramebufferFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO:
                safe_pNext = new safe_VkFramebufferAttachmentsCreateInfo(reinterpret_cast<const VkFramebufferAttachmentsCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO:
                safe_pNext = new safe_VkRenderPassAttachmentBeginInfo(reinterpret_cast<const VkRenderPassAttachmentBeginInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceUniformBufferStandardLayoutFeatures(reinterpret_cast<const VkPhysicalDeviceUniformBufferStandardLayoutFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures(reinterpret_cast<const VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures(reinterpret_cast<const VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_STENCIL_LAYOUT:
                safe_pNext = new safe_VkAttachmentReferenceStencilLayout(reinterpret_cast<const VkAttachmentReferenceStencilLayout *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_STENCIL_LAYOUT:
                safe_pNext = new safe_VkAttachmentDescriptionStencilLayout(reinterpret_cast<const VkAttachmentDescriptionStencilLayout *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceHostQueryResetFeatures(reinterpret_cast<const VkPhysicalDeviceHostQueryResetFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceTimelineSemaphoreFeatures(reinterpret_cast<const VkPhysicalDeviceTimelineSemaphoreFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceTimelineSemaphoreProperties(reinterpret_cast<const VkPhysicalDeviceTimelineSemaphoreProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO:
                safe_pNext = new safe_VkSemaphoreTypeCreateInfo(reinterpret_cast<const VkSemaphoreTypeCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO:
                safe_pNext = new safe_VkTimelineSemaphoreSubmitInfo(reinterpret_cast<const VkTimelineSemaphoreSubmitInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceBufferDeviceAddressFeatures(reinterpret_cast<const VkPhysicalDeviceBufferDeviceAddressFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO:
                safe_pNext = new safe_VkBufferOpaqueCaptureAddressCreateInfo(reinterpret_cast<const VkBufferOpaqueCaptureAddressCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO:
                safe_pNext = new safe_VkMemoryOpaqueCaptureAddressAllocateInfo(reinterpret_cast<const VkMemoryOpaqueCaptureAddressAllocateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceVulkan13Features(reinterpret_cast<const VkPhysicalDeviceVulkan13Features *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceVulkan13Properties(reinterpret_cast<const VkPhysicalDeviceVulkan13Properties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_CREATION_FEEDBACK_CREATE_INFO:
                safe_pNext = new safe_VkPipelineCreationFeedbackCreateInfo(reinterpret_cast<const VkPipelineCreationFeedbackCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceShaderTerminateInvocationFeatures(reinterpret_cast<const VkPhysicalDeviceShaderTerminateInvocationFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures(reinterpret_cast<const VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES:
                safe_pNext = new safe_VkPhysicalDevicePrivateDataFeatures(reinterpret_cast<const VkPhysicalDevicePrivateDataFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_PRIVATE_DATA_CREATE_INFO:
                safe_pNext = new safe_VkDevicePrivateDataCreateInfo(reinterpret_cast<const VkDevicePrivateDataCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES:
                safe_pNext = new safe_VkPhysicalDevicePipelineCreationCacheControlFeatures(reinterpret_cast<const VkPhysicalDevicePipelineCreationCacheControlFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_MEMORY_BARRIER_2:
                safe_pNext = new safe_VkMemoryBarrier2(reinterpret_cast<const VkMemoryBarrier2 *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceSynchronization2Features(reinterpret_cast<const VkPhysicalDeviceSynchronization2Features *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures(reinterpret_cast<const VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceImageRobustnessFeatures(reinterpret_cast<const VkPhysicalDeviceImageRobustnessFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceSubgroupSizeControlFeatures(reinterpret_cast<const VkPhysicalDeviceSubgroupSizeControlFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceSubgroupSizeControlProperties(reinterpret_cast<const VkPhysicalDeviceSubgroupSizeControlProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO:
                safe_pNext = new safe_VkPipelineShaderStageRequiredSubgroupSizeCreateInfo(reinterpret_cast<const VkPipelineShaderStageRequiredSubgroupSizeCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceInlineUniformBlockFeatures(reinterpret_cast<const VkPhysicalDeviceInlineUniformBlockFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceInlineUniformBlockProperties(reinterpret_cast<const VkPhysicalDeviceInlineUniformBlockProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK:
                safe_pNext = new safe_VkWriteDescriptorSetInlineUniformBlock(reinterpret_cast<const VkWriteDescriptorSetInlineUniformBlock *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_INLINE_UNIFORM_BLOCK_CREATE_INFO:
                safe_pNext = new safe_VkDescriptorPoolInlineUniformBlockCreateInfo(reinterpret_cast<const VkDescriptorPoolInlineUniformBlockCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceTextureCompressionASTCHDRFeatures(reinterpret_cast<const VkPhysicalDeviceTextureCompressionASTCHDRFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO:
                safe_pNext = new safe_VkPipelineRenderingCreateInfo(reinterpret_cast<const VkPipelineRenderingCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceDynamicRenderingFeatures(reinterpret_cast<const VkPhysicalDeviceDynamicRenderingFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO:
                safe_pNext = new safe_VkCommandBufferInheritanceRenderingInfo(reinterpret_cast<const VkCommandBufferInheritanceRenderingInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceShaderIntegerDotProductFeatures(reinterpret_cast<const VkPhysicalDeviceShaderIntegerDotProductFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceShaderIntegerDotProductProperties(reinterpret_cast<const VkPhysicalDeviceShaderIntegerDotProductProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceTexelBufferAlignmentProperties(reinterpret_cast<const VkPhysicalDeviceTexelBufferAlignmentProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_3:
                safe_pNext = new safe_VkFormatProperties3(reinterpret_cast<const VkFormatProperties3 *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceMaintenance4Features(reinterpret_cast<const VkPhysicalDeviceMaintenance4Features *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceMaintenance4Properties(reinterpret_cast<const VkPhysicalDeviceMaintenance4Properties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceVulkan14Features(reinterpret_cast<const VkPhysicalDeviceVulkan14Features *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceVulkan14Properties(reinterpret_cast<const VkPhysicalDeviceVulkan14Properties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO:
                safe_pNext = new safe_VkDeviceQueueGlobalPriorityCreateInfo(reinterpret_cast<const VkDeviceQueueGlobalPriorityCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GLOBAL_PRIORITY_QUERY_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceGlobalPriorityQueryFeatures(reinterpret_cast<const VkPhysicalDeviceGlobalPriorityQueryFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_QUEUE_FAMILY_GLOBAL_PRIORITY_PROPERTIES:
                safe_pNext = new safe_VkQueueFamilyGlobalPriorityProperties(reinterpret_cast<const VkQueueFamilyGlobalPriorityProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_ROTATE_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceShaderSubgroupRotateFeatures(reinterpret_cast<const VkPhysicalDeviceShaderSubgroupRotateFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT_CONTROLS_2_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceShaderFloatControls2Features(reinterpret_cast<const VkPhysicalDeviceShaderFloatControls2Features *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EXPECT_ASSUME_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceShaderExpectAssumeFeatures(reinterpret_cast<const VkPhysicalDeviceShaderExpectAssumeFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceLineRasterizationFeatures(reinterpret_cast<const VkPhysicalDeviceLineRasterizationFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceLineRasterizationProperties(reinterpret_cast<const VkPhysicalDeviceLineRasterizationProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO:
                safe_pNext = new safe_VkPipelineRasterizationLineStateCreateInfo(reinterpret_cast<const VkPipelineRasterizationLineStateCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceVertexAttributeDivisorProperties(reinterpret_cast<const VkPhysicalDeviceVertexAttributeDivisorProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO:
                safe_pNext = new safe_VkPipelineVertexInputDivisorStateCreateInfo(reinterpret_cast<const VkPipelineVertexInputDivisorStateCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceVertexAttributeDivisorFeatures(reinterpret_cast<const VkPhysicalDeviceVertexAttributeDivisorFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceIndexTypeUint8Features(reinterpret_cast<const VkPhysicalDeviceIndexTypeUint8Features *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceMaintenance5Features(reinterpret_cast<const VkPhysicalDeviceMaintenance5Features *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceMaintenance5Properties(reinterpret_cast<const VkPhysicalDeviceMaintenance5Properties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_CREATE_FLAGS_2_CREATE_INFO:
                safe_pNext = new safe_VkPipelineCreateFlags2CreateInfo(reinterpret_cast<const VkPipelineCreateFlags2CreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO:
                safe_pNext = new safe_VkBufferUsageFlags2CreateInfo(reinterpret_cast<const VkBufferUsageFlags2CreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDevicePushDescriptorProperties(reinterpret_cast<const VkPhysicalDevicePushDescriptorProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceDynamicRenderingLocalReadFeatures(reinterpret_cast<const VkPhysicalDeviceDynamicRenderingLocalReadFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_LOCATION_INFO:
                safe_pNext = new safe_VkRenderingAttachmentLocationInfo(reinterpret_cast<const VkRenderingAttachmentLocationInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDERING_INPUT_ATTACHMENT_INDEX_INFO:
                safe_pNext = new safe_VkRenderingInputAttachmentIndexInfo(reinterpret_cast<const VkRenderingInputAttachmentIndexInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceMaintenance6Features(reinterpret_cast<const VkPhysicalDeviceMaintenance6Features *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceMaintenance6Properties(reinterpret_cast<const VkPhysicalDeviceMaintenance6Properties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_BIND_MEMORY_STATUS:
                safe_pNext = new safe_VkBindMemoryStatus(reinterpret_cast<const VkBindMemoryStatus *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROTECTED_ACCESS_FEATURES:
                safe_pNext = new safe_VkPhysicalDevicePipelineProtectedAccessFeatures(reinterpret_cast<const VkPhysicalDevicePipelineProtectedAccessFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES:
                safe_pNext = new safe_VkPhysicalDevicePipelineRobustnessFeatures(reinterpret_cast<const VkPhysicalDevicePipelineRobustnessFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDevicePipelineRobustnessProperties(reinterpret_cast<const VkPhysicalDevicePipelineRobustnessProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_ROBUSTNESS_CREATE_INFO:
                safe_pNext = new safe_VkPipelineRobustnessCreateInfo(reinterpret_cast<const VkPipelineRobustnessCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceHostImageCopyFeatures(reinterpret_cast<const VkPhysicalDeviceHostImageCopyFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceHostImageCopyProperties(reinterpret_cast<const VkPhysicalDeviceHostImageCopyProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SUBRESOURCE_HOST_MEMCPY_SIZE:
                safe_pNext = new safe_VkSubresourceHostMemcpySize(reinterpret_cast<const VkSubresourceHostMemcpySize *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_HOST_IMAGE_COPY_DEVICE_PERFORMANCE_QUERY:
                safe_pNext = new safe_VkHostImageCopyDevicePerformanceQuery(reinterpret_cast<const VkHostImageCopyDevicePerformanceQuery *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_SWAPCHAIN_CREATE_INFO_KHR:
                safe_pNext = new safe_VkImageSwapchainCreateInfoKHR(reinterpret_cast<const VkImageSwapchainCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR:
                safe_pNext = new safe_VkBindImageMemorySwapchainInfoKHR(reinterpret_cast<const VkBindImageMemorySwapchainInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_INFO_KHR:
                safe_pNext = new safe_VkDeviceGroupPresentInfoKHR(reinterpret_cast<const VkDeviceGroupPresentInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_GROUP_SWAPCHAIN_CREATE_INFO_KHR:
                safe_pNext = new safe_VkDeviceGroupSwapchainCreateInfoKHR(reinterpret_cast<const VkDeviceGroupSwapchainCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR:
                safe_pNext = new safe_VkDisplayPresentInfoKHR(reinterpret_cast<const VkDisplayPresentInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_QUEUE_FAMILY_QUERY_RESULT_STATUS_PROPERTIES_KHR:
                safe_pNext = new safe_VkQueueFamilyQueryResultStatusPropertiesKHR(reinterpret_cast<const VkQueueFamilyQueryResultStatusPropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_QUEUE_FAMILY_VIDEO_PROPERTIES_KHR:
                safe_pNext = new safe_VkQueueFamilyVideoPropertiesKHR(reinterpret_cast<const VkQueueFamilyVideoPropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR:
                safe_pNext = new safe_VkVideoProfileInfoKHR(reinterpret_cast<const VkVideoProfileInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_PROFILE_LIST_INFO_KHR:
                safe_pNext = new safe_VkVideoProfileListInfoKHR(reinterpret_cast<const VkVideoProfileListInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_CAPABILITIES_KHR:
                safe_pNext = new safe_VkVideoDecodeCapabilitiesKHR(reinterpret_cast<const VkVideoDecodeCapabilitiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_USAGE_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeUsageInfoKHR(reinterpret_cast<const VkVideoDecodeUsageInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_CAPABILITIES_KHR:
                safe_pNext = new safe_VkVideoEncodeH264CapabilitiesKHR(reinterpret_cast<const VkVideoEncodeH264CapabilitiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_QUALITY_LEVEL_PROPERTIES_KHR:
                safe_pNext = new safe_VkVideoEncodeH264QualityLevelPropertiesKHR(reinterpret_cast<const VkVideoEncodeH264QualityLevelPropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_CREATE_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeH264SessionCreateInfoKHR(reinterpret_cast<const VkVideoEncodeH264SessionCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_ADD_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeH264SessionParametersAddInfoKHR(reinterpret_cast<const VkVideoEncodeH264SessionParametersAddInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_CREATE_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeH264SessionParametersCreateInfoKHR(reinterpret_cast<const VkVideoEncodeH264SessionParametersCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_GET_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeH264SessionParametersGetInfoKHR(reinterpret_cast<const VkVideoEncodeH264SessionParametersGetInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_FEEDBACK_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeH264SessionParametersFeedbackInfoKHR(reinterpret_cast<const VkVideoEncodeH264SessionParametersFeedbackInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PICTURE_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeH264PictureInfoKHR(reinterpret_cast<const VkVideoEncodeH264PictureInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_DPB_SLOT_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeH264DpbSlotInfoKHR(reinterpret_cast<const VkVideoEncodeH264DpbSlotInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PROFILE_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeH264ProfileInfoKHR(reinterpret_cast<const VkVideoEncodeH264ProfileInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_RATE_CONTROL_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeH264RateControlInfoKHR(reinterpret_cast<const VkVideoEncodeH264RateControlInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_RATE_CONTROL_LAYER_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeH264RateControlLayerInfoKHR(reinterpret_cast<const VkVideoEncodeH264RateControlLayerInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_GOP_REMAINING_FRAME_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeH264GopRemainingFrameInfoKHR(reinterpret_cast<const VkVideoEncodeH264GopRemainingFrameInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_CAPABILITIES_KHR:
                safe_pNext = new safe_VkVideoEncodeH265CapabilitiesKHR(reinterpret_cast<const VkVideoEncodeH265CapabilitiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_CREATE_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeH265SessionCreateInfoKHR(reinterpret_cast<const VkVideoEncodeH265SessionCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_QUALITY_LEVEL_PROPERTIES_KHR:
                safe_pNext = new safe_VkVideoEncodeH265QualityLevelPropertiesKHR(reinterpret_cast<const VkVideoEncodeH265QualityLevelPropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_ADD_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeH265SessionParametersAddInfoKHR(reinterpret_cast<const VkVideoEncodeH265SessionParametersAddInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_CREATE_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeH265SessionParametersCreateInfoKHR(reinterpret_cast<const VkVideoEncodeH265SessionParametersCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_GET_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeH265SessionParametersGetInfoKHR(reinterpret_cast<const VkVideoEncodeH265SessionParametersGetInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_FEEDBACK_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeH265SessionParametersFeedbackInfoKHR(reinterpret_cast<const VkVideoEncodeH265SessionParametersFeedbackInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PICTURE_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeH265PictureInfoKHR(reinterpret_cast<const VkVideoEncodeH265PictureInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_DPB_SLOT_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeH265DpbSlotInfoKHR(reinterpret_cast<const VkVideoEncodeH265DpbSlotInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PROFILE_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeH265ProfileInfoKHR(reinterpret_cast<const VkVideoEncodeH265ProfileInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_RATE_CONTROL_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeH265RateControlInfoKHR(reinterpret_cast<const VkVideoEncodeH265RateControlInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_RATE_CONTROL_LAYER_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeH265RateControlLayerInfoKHR(reinterpret_cast<const VkVideoEncodeH265RateControlLayerInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_GOP_REMAINING_FRAME_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeH265GopRemainingFrameInfoKHR(reinterpret_cast<const VkVideoEncodeH265GopRemainingFrameInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PROFILE_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeH264ProfileInfoKHR(reinterpret_cast<const VkVideoDecodeH264ProfileInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_CAPABILITIES_KHR:
                safe_pNext = new safe_VkVideoDecodeH264CapabilitiesKHR(reinterpret_cast<const VkVideoDecodeH264CapabilitiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_ADD_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeH264SessionParametersAddInfoKHR(reinterpret_cast<const VkVideoDecodeH264SessionParametersAddInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_CREATE_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeH264SessionParametersCreateInfoKHR(reinterpret_cast<const VkVideoDecodeH264SessionParametersCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PICTURE_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeH264PictureInfoKHR(reinterpret_cast<const VkVideoDecodeH264PictureInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_DPB_SLOT_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeH264DpbSlotInfoKHR(reinterpret_cast<const VkVideoDecodeH264DpbSlotInfoKHR *>(pNext), copy_state, false);
                break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR:
                safe_pNext = new safe_VkImportMemoryWin32HandleInfoKHR(reinterpret_cast<const VkImportMemoryWin32HandleInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR:
                safe_pNext = new safe_VkExportMemoryWin32HandleInfoKHR(reinterpret_cast<const VkExportMemoryWin32HandleInfoKHR *>(pNext), copy_state, false);
                break;
#endif  // VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR:
                safe_pNext = new safe_VkImportMemoryFdInfoKHR(reinterpret_cast<const VkImportMemoryFdInfoKHR *>(pNext), copy_state, false);
                break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_KHR:
                safe_pNext = new safe_VkWin32KeyedMutexAcquireReleaseInfoKHR(reinterpret_cast<const VkWin32KeyedMutexAcquireReleaseInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR:
                safe_pNext = new safe_VkExportSemaphoreWin32HandleInfoKHR(reinterpret_cast<const VkExportSemaphoreWin32HandleInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_D3D12_FENCE_SUBMIT_INFO_KHR:
                safe_pNext = new safe_VkD3D12FenceSubmitInfoKHR(reinterpret_cast<const VkD3D12FenceSubmitInfoKHR *>(pNext), copy_state, false);
                break;
#endif  // VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR:
                safe_pNext = new safe_VkPresentRegionsKHR(reinterpret_cast<const VkPresentRegionsKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR:
                safe_pNext = new safe_VkSharedPresentSurfaceCapabilitiesKHR(reinterpret_cast<const VkSharedPresentSurfaceCapabilitiesKHR *>(pNext), copy_state, false);
                break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_EXPORT_FENCE_WIN32_HANDLE_INFO_KHR:
                safe_pNext = new safe_VkExportFenceWin32HandleInfoKHR(reinterpret_cast<const VkExportFenceWin32HandleInfoKHR *>(pNext), copy_state, false);
                break;
#endif  // VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDevicePerformanceQueryFeaturesKHR(reinterpret_cast<const VkPhysicalDevicePerformanceQueryFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_PROPERTIES_KHR:
                safe_pNext = new safe_VkPhysicalDevicePerformanceQueryPropertiesKHR(reinterpret_cast<const VkPhysicalDevicePerformanceQueryPropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR:
                safe_pNext = new safe_VkQueryPoolPerformanceCreateInfoKHR(reinterpret_cast<const VkQueryPoolPerformanceCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_SUBMIT_INFO_KHR:
                safe_pNext = new safe_VkPerformanceQuerySubmitInfoKHR(reinterpret_cast<const VkPerformanceQuerySubmitInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_BFLOAT16_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceShaderBfloat16FeaturesKHR(reinterpret_cast<const VkPhysicalDeviceShaderBfloat16FeaturesKHR *>(pNext), copy_state, false);
                break;
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR(reinterpret_cast<const VkPhysicalDevicePortabilitySubsetFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_PROPERTIES_KHR:
                safe_pNext = new safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR(reinterpret_cast<const VkPhysicalDevicePortabilitySubsetPropertiesKHR *>(pNext), copy_state, false);
                break;
#endif  // VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceShaderClockFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceShaderClockFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PROFILE_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeH265ProfileInfoKHR(reinterpret_cast<const VkVideoDecodeH265ProfileInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_CAPABILITIES_KHR:
                safe_pNext = new safe_VkVideoDecodeH265CapabilitiesKHR(reinterpret_cast<const VkVideoDecodeH265CapabilitiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_PARAMETERS_ADD_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeH265SessionParametersAddInfoKHR(reinterpret_cast<const VkVideoDecodeH265SessionParametersAddInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_PARAMETERS_CREATE_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeH265SessionParametersCreateInfoKHR(reinterpret_cast<const VkVideoDecodeH265SessionParametersCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PICTURE_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeH265PictureInfoKHR(reinterpret_cast<const VkVideoDecodeH265PictureInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_DPB_SLOT_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeH265DpbSlotInfoKHR(reinterpret_cast<const VkVideoDecodeH265DpbSlotInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR:
                safe_pNext = new safe_VkFragmentShadingRateAttachmentInfoKHR(reinterpret_cast<const VkFragmentShadingRateAttachmentInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_STATE_CREATE_INFO_KHR:
                safe_pNext = new safe_VkPipelineFragmentShadingRateStateCreateInfoKHR(reinterpret_cast<const VkPipelineFragmentShadingRateStateCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceFragmentShadingRateFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR(reinterpret_cast<const VkPhysicalDeviceFragmentShadingRatePropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR:
                safe_pNext = new safe_VkRenderingFragmentShadingRateAttachmentInfoKHR(reinterpret_cast<const VkRenderingFragmentShadingRateAttachmentInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_QUAD_CONTROL_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceShaderQuadControlFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceShaderQuadControlFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SURFACE_PROTECTED_CAPABILITIES_KHR:
                safe_pNext = new safe_VkSurfaceProtectedCapabilitiesKHR(reinterpret_cast<const VkSurfaceProtectedCapabilitiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDevicePresentWaitFeaturesKHR(reinterpret_cast<const VkPhysicalDevicePresentWaitFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR(reinterpret_cast<const VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR:
                safe_pNext = new safe_VkPipelineLibraryCreateInfoKHR(reinterpret_cast<const VkPipelineLibraryCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PRESENT_ID_KHR:
                safe_pNext = new safe_VkPresentIdKHR(reinterpret_cast<const VkPresentIdKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDevicePresentIdFeaturesKHR(reinterpret_cast<const VkPhysicalDevicePresentIdFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_CAPABILITIES_KHR:
                safe_pNext = new safe_VkVideoEncodeCapabilitiesKHR(reinterpret_cast<const VkVideoEncodeCapabilitiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_QUERY_POOL_VIDEO_ENCODE_FEEDBACK_CREATE_INFO_KHR:
                safe_pNext = new safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR(reinterpret_cast<const VkQueryPoolVideoEncodeFeedbackCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_USAGE_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeUsageInfoKHR(reinterpret_cast<const VkVideoEncodeUsageInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_RATE_CONTROL_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeRateControlInfoKHR(reinterpret_cast<const VkVideoEncodeRateControlInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUALITY_LEVEL_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeQualityLevelInfoKHR(reinterpret_cast<const VkVideoEncodeQualityLevelInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_PROPERTIES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR(reinterpret_cast<const VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR(reinterpret_cast<const VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MAXIMAL_RECONVERGENCE_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_PRESENT_ID_2_KHR:
                safe_pNext = new safe_VkSurfaceCapabilitiesPresentId2KHR(reinterpret_cast<const VkSurfaceCapabilitiesPresentId2KHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PRESENT_ID_2_KHR:
                safe_pNext = new safe_VkPresentId2KHR(reinterpret_cast<const VkPresentId2KHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_2_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDevicePresentId2FeaturesKHR(reinterpret_cast<const VkPhysicalDevicePresentId2FeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_PRESENT_WAIT_2_KHR:
                safe_pNext = new safe_VkSurfaceCapabilitiesPresentWait2KHR(reinterpret_cast<const VkSurfaceCapabilitiesPresentWait2KHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_2_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDevicePresentWait2FeaturesKHR(reinterpret_cast<const VkPhysicalDevicePresentWait2FeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_BINARY_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDevicePipelineBinaryFeaturesKHR(reinterpret_cast<const VkPhysicalDevicePipelineBinaryFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_BINARY_PROPERTIES_KHR:
                safe_pNext = new safe_VkPhysicalDevicePipelineBinaryPropertiesKHR(reinterpret_cast<const VkPhysicalDevicePipelineBinaryPropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_PIPELINE_BINARY_INTERNAL_CACHE_CONTROL_KHR:
                safe_pNext = new safe_VkDevicePipelineBinaryInternalCacheControlKHR(reinterpret_cast<const VkDevicePipelineBinaryInternalCacheControlKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_BINARY_INFO_KHR:
                safe_pNext = new safe_VkPipelineBinaryInfoKHR(reinterpret_cast<const VkPipelineBinaryInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_KHR:
                safe_pNext = new safe_VkSurfacePresentModeKHR(reinterpret_cast<const VkSurfacePresentModeKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SURFACE_PRESENT_SCALING_CAPABILITIES_KHR:
                safe_pNext = new safe_VkSurfacePresentScalingCapabilitiesKHR(reinterpret_cast<const VkSurfacePresentScalingCapabilitiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_COMPATIBILITY_KHR:
                safe_pNext = new safe_VkSurfacePresentModeCompatibilityKHR(reinterpret_cast<const VkSurfacePresentModeCompatibilityKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR(reinterpret_cast<const VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_KHR:
                safe_pNext = new safe_VkSwapchainPresentFenceInfoKHR(reinterpret_cast<const VkSwapchainPresentFenceInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_MODES_CREATE_INFO_KHR:
                safe_pNext = new safe_VkSwapchainPresentModesCreateInfoKHR(reinterpret_cast<const VkSwapchainPresentModesCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_MODE_INFO_KHR:
                safe_pNext = new safe_VkSwapchainPresentModeInfoKHR(reinterpret_cast<const VkSwapchainPresentModeInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_SCALING_CREATE_INFO_KHR:
                safe_pNext = new safe_VkSwapchainPresentScalingCreateInfoKHR(reinterpret_cast<const VkSwapchainPresentScalingCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceCooperativeMatrixFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR(reinterpret_cast<const VkPhysicalDeviceCooperativeMatrixPropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_PROPERTIES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR(reinterpret_cast<const VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_PROFILE_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeAV1ProfileInfoKHR(reinterpret_cast<const VkVideoDecodeAV1ProfileInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_CAPABILITIES_KHR:
                safe_pNext = new safe_VkVideoDecodeAV1CapabilitiesKHR(reinterpret_cast<const VkVideoDecodeAV1CapabilitiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_SESSION_PARAMETERS_CREATE_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeAV1SessionParametersCreateInfoKHR(reinterpret_cast<const VkVideoDecodeAV1SessionParametersCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_PICTURE_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeAV1PictureInfoKHR(reinterpret_cast<const VkVideoDecodeAV1PictureInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_DPB_SLOT_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeAV1DpbSlotInfoKHR(reinterpret_cast<const VkVideoDecodeAV1DpbSlotInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_AV1_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceVideoEncodeAV1FeaturesKHR(reinterpret_cast<const VkPhysicalDeviceVideoEncodeAV1FeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_CAPABILITIES_KHR:
                safe_pNext = new safe_VkVideoEncodeAV1CapabilitiesKHR(reinterpret_cast<const VkVideoEncodeAV1CapabilitiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_QUALITY_LEVEL_PROPERTIES_KHR:
                safe_pNext = new safe_VkVideoEncodeAV1QualityLevelPropertiesKHR(reinterpret_cast<const VkVideoEncodeAV1QualityLevelPropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_SESSION_CREATE_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeAV1SessionCreateInfoKHR(reinterpret_cast<const VkVideoEncodeAV1SessionCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_SESSION_PARAMETERS_CREATE_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeAV1SessionParametersCreateInfoKHR(reinterpret_cast<const VkVideoEncodeAV1SessionParametersCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_PICTURE_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeAV1PictureInfoKHR(reinterpret_cast<const VkVideoEncodeAV1PictureInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_DPB_SLOT_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeAV1DpbSlotInfoKHR(reinterpret_cast<const VkVideoEncodeAV1DpbSlotInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_PROFILE_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeAV1ProfileInfoKHR(reinterpret_cast<const VkVideoEncodeAV1ProfileInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_GOP_REMAINING_FRAME_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeAV1GopRemainingFrameInfoKHR(reinterpret_cast<const VkVideoEncodeAV1GopRemainingFrameInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_RATE_CONTROL_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeAV1RateControlInfoKHR(reinterpret_cast<const VkVideoEncodeAV1RateControlInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_RATE_CONTROL_LAYER_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeAV1RateControlLayerInfoKHR(reinterpret_cast<const VkVideoEncodeAV1RateControlLayerInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_DECODE_VP9_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceVideoDecodeVP9FeaturesKHR(reinterpret_cast<const VkPhysicalDeviceVideoDecodeVP9FeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_VP9_PROFILE_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeVP9ProfileInfoKHR(reinterpret_cast<const VkVideoDecodeVP9ProfileInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_VP9_CAPABILITIES_KHR:
                safe_pNext = new safe_VkVideoDecodeVP9CapabilitiesKHR(reinterpret_cast<const VkVideoDecodeVP9CapabilitiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_VP9_PICTURE_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeVP9PictureInfoKHR(reinterpret_cast<const VkVideoDecodeVP9PictureInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_1_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceVideoMaintenance1FeaturesKHR(reinterpret_cast<const VkPhysicalDeviceVideoMaintenance1FeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_INLINE_QUERY_INFO_KHR:
                safe_pNext = new safe_VkVideoInlineQueryInfoKHR(reinterpret_cast<const VkVideoInlineQueryInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFIED_IMAGE_LAYOUTS_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_ATTACHMENT_FEEDBACK_LOOP_INFO_EXT:
                safe_pNext = new safe_VkAttachmentFeedbackLoopInfoEXT(reinterpret_cast<const VkAttachmentFeedbackLoopInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_INTRA_REFRESH_CAPABILITIES_KHR:
                safe_pNext = new safe_VkVideoEncodeIntraRefreshCapabilitiesKHR(reinterpret_cast<const VkVideoEncodeIntraRefreshCapabilitiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_SESSION_INTRA_REFRESH_CREATE_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeSessionIntraRefreshCreateInfoKHR(reinterpret_cast<const VkVideoEncodeSessionIntraRefreshCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_INTRA_REFRESH_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeIntraRefreshInfoKHR(reinterpret_cast<const VkVideoEncodeIntraRefreshInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_REFERENCE_INTRA_REFRESH_INFO_KHR:
                safe_pNext = new safe_VkVideoReferenceIntraRefreshInfoKHR(reinterpret_cast<const VkVideoReferenceIntraRefreshInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_INTRA_REFRESH_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_CAPABILITIES_KHR:
                safe_pNext = new safe_VkVideoEncodeQuantizationMapCapabilitiesKHR(reinterpret_cast<const VkVideoEncodeQuantizationMapCapabilitiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_FORMAT_QUANTIZATION_MAP_PROPERTIES_KHR:
                safe_pNext = new safe_VkVideoFormatQuantizationMapPropertiesKHR(reinterpret_cast<const VkVideoFormatQuantizationMapPropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeQuantizationMapInfoKHR(reinterpret_cast<const VkVideoEncodeQuantizationMapInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_SESSION_PARAMETERS_CREATE_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR(reinterpret_cast<const VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_QUANTIZATION_MAP_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_QUANTIZATION_MAP_CAPABILITIES_KHR:
                safe_pNext = new safe_VkVideoEncodeH264QuantizationMapCapabilitiesKHR(reinterpret_cast<const VkVideoEncodeH264QuantizationMapCapabilitiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_QUANTIZATION_MAP_CAPABILITIES_KHR:
                safe_pNext = new safe_VkVideoEncodeH265QuantizationMapCapabilitiesKHR(reinterpret_cast<const VkVideoEncodeH265QuantizationMapCapabilitiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_FORMAT_H265_QUANTIZATION_MAP_PROPERTIES_KHR:
                safe_pNext = new safe_VkVideoFormatH265QuantizationMapPropertiesKHR(reinterpret_cast<const VkVideoFormatH265QuantizationMapPropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_QUANTIZATION_MAP_CAPABILITIES_KHR:
                safe_pNext = new safe_VkVideoEncodeAV1QuantizationMapCapabilitiesKHR(reinterpret_cast<const VkVideoEncodeAV1QuantizationMapCapabilitiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_FORMAT_AV1_QUANTIZATION_MAP_PROPERTIES_KHR:
                safe_pNext = new safe_VkVideoFormatAV1QuantizationMapPropertiesKHR(reinterpret_cast<const VkVideoFormatAV1QuantizationMapPropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_RELAXED_EXTENDED_INSTRUCTION_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceMaintenance7FeaturesKHR(reinterpret_cast<const VkPhysicalDeviceMaintenance7FeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_PROPERTIES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceMaintenance7PropertiesKHR(reinterpret_cast<const VkPhysicalDeviceMaintenance7PropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_API_PROPERTIES_LIST_KHR:
                safe_pNext = new safe_VkPhysicalDeviceLayeredApiPropertiesListKHR(reinterpret_cast<const VkPhysicalDeviceLayeredApiPropertiesListKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_API_VULKAN_PROPERTIES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceLayeredApiVulkanPropertiesKHR(reinterpret_cast<const VkPhysicalDeviceLayeredApiVulkanPropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_8_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceMaintenance8FeaturesKHR(reinterpret_cast<const VkPhysicalDeviceMaintenance8FeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_MEMORY_BARRIER_ACCESS_FLAGS_3_KHR:
                safe_pNext = new safe_VkMemoryBarrierAccessFlags3KHR(reinterpret_cast<const VkMemoryBarrierAccessFlags3KHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_9_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceMaintenance9FeaturesKHR(reinterpret_cast<const VkPhysicalDeviceMaintenance9FeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_9_PROPERTIES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceMaintenance9PropertiesKHR(reinterpret_cast<const VkPhysicalDeviceMaintenance9PropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_QUEUE_FAMILY_OWNERSHIP_TRANSFER_PROPERTIES_KHR:
                safe_pNext = new safe_VkQueueFamilyOwnershipTransferPropertiesKHR(reinterpret_cast<const VkQueueFamilyOwnershipTransferPropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_2_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceVideoMaintenance2FeaturesKHR(reinterpret_cast<const VkPhysicalDeviceVideoMaintenance2FeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_INLINE_SESSION_PARAMETERS_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeH264InlineSessionParametersInfoKHR(reinterpret_cast<const VkVideoDecodeH264InlineSessionParametersInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_INLINE_SESSION_PARAMETERS_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeH265InlineSessionParametersInfoKHR(reinterpret_cast<const VkVideoDecodeH265InlineSessionParametersInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_INLINE_SESSION_PARAMETERS_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeAV1InlineSessionParametersInfoKHR(reinterpret_cast<const VkVideoDecodeAV1InlineSessionParametersInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_ZERO_ONE_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceDepthClampZeroOneFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceDepthClampZeroOneFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceRobustness2FeaturesKHR(reinterpret_cast<const VkPhysicalDeviceRobustness2FeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_PROPERTIES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceRobustness2PropertiesKHR(reinterpret_cast<const VkPhysicalDeviceRobustness2PropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_MODE_FIFO_LATEST_READY_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR(reinterpret_cast<const VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT:
                safe_pNext = new safe_VkDebugReportCallbackCreateInfoEXT(reinterpret_cast<const VkDebugReportCallbackCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_RASTERIZATION_ORDER_AMD:
                safe_pNext = new safe_VkPipelineRasterizationStateRasterizationOrderAMD(reinterpret_cast<const VkPipelineRasterizationStateRasterizationOrderAMD *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_IMAGE_CREATE_INFO_NV:
                safe_pNext = new safe_VkDedicatedAllocationImageCreateInfoNV(reinterpret_cast<const VkDedicatedAllocationImageCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_BUFFER_CREATE_INFO_NV:
                safe_pNext = new safe_VkDedicatedAllocationBufferCreateInfoNV(reinterpret_cast<const VkDedicatedAllocationBufferCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_MEMORY_ALLOCATE_INFO_NV:
                safe_pNext = new safe_VkDedicatedAllocationMemoryAllocateInfoNV(reinterpret_cast<const VkDedicatedAllocationMemoryAllocateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceTransformFeedbackFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceTransformFeedbackPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_STREAM_CREATE_INFO_EXT:
                safe_pNext = new safe_VkPipelineRasterizationStateStreamCreateInfoEXT(reinterpret_cast<const VkPipelineRasterizationStateStreamCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_CU_MODULE_TEXTURING_MODE_CREATE_INFO_NVX:
                safe_pNext = new safe_VkCuModuleTexturingModeCreateInfoNVX(reinterpret_cast<const VkCuModuleTexturingModeCreateInfoNVX *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_TEXTURE_LOD_GATHER_FORMAT_PROPERTIES_AMD:
                safe_pNext = new safe_VkTextureLODGatherFormatPropertiesAMD(reinterpret_cast<const VkTextureLODGatherFormatPropertiesAMD *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CORNER_SAMPLED_IMAGE_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceCornerSampledImageFeaturesNV(reinterpret_cast<const VkPhysicalDeviceCornerSampledImageFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_NV:
                safe_pNext = new safe_VkExternalMemoryImageCreateInfoNV(reinterpret_cast<const VkExternalMemoryImageCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_NV:
                safe_pNext = new safe_VkExportMemoryAllocateInfoNV(reinterpret_cast<const VkExportMemoryAllocateInfoNV *>(pNext), copy_state, false);
                break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_NV:
                safe_pNext = new safe_VkImportMemoryWin32HandleInfoNV(reinterpret_cast<const VkImportMemoryWin32HandleInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_NV:
                safe_pNext = new safe_VkExportMemoryWin32HandleInfoNV(reinterpret_cast<const VkExportMemoryWin32HandleInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_NV:
                safe_pNext = new safe_VkWin32KeyedMutexAcquireReleaseInfoNV(reinterpret_cast<const VkWin32KeyedMutexAcquireReleaseInfoNV *>(pNext), copy_state, false);
                break;
#endif  // VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT:
                safe_pNext = new safe_VkValidationFlagsEXT(reinterpret_cast<const VkValidationFlagsEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_VIEW_ASTC_DECODE_MODE_EXT:
                safe_pNext = new safe_VkImageViewASTCDecodeModeEXT(reinterpret_cast<const VkImageViewASTCDecodeModeEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ASTC_DECODE_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceASTCDecodeFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceASTCDecodeFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceConditionalRenderingFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT:
                safe_pNext = new safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT(reinterpret_cast<const VkCommandBufferInheritanceConditionalRenderingInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_W_SCALING_STATE_CREATE_INFO_NV:
                safe_pNext = new safe_VkPipelineViewportWScalingStateCreateInfoNV(reinterpret_cast<const VkPipelineViewportWScalingStateCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SWAPCHAIN_COUNTER_CREATE_INFO_EXT:
                safe_pNext = new safe_VkSwapchainCounterCreateInfoEXT(reinterpret_cast<const VkSwapchainCounterCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PRESENT_TIMES_INFO_GOOGLE:
                safe_pNext = new safe_VkPresentTimesInfoGOOGLE(reinterpret_cast<const VkPresentTimesInfoGOOGLE *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_ATTRIBUTES_PROPERTIES_NVX:
                safe_pNext = new safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX(reinterpret_cast<const VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_MULTIVIEW_PER_VIEW_ATTRIBUTES_INFO_NVX:
                safe_pNext = new safe_VkMultiviewPerViewAttributesInfoNVX(reinterpret_cast<const VkMultiviewPerViewAttributesInfoNVX *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SWIZZLE_STATE_CREATE_INFO_NV:
                safe_pNext = new safe_VkPipelineViewportSwizzleStateCreateInfoNV(reinterpret_cast<const VkPipelineViewportSwizzleStateCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISCARD_RECTANGLE_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT(reinterpret_cast<const VkPhysicalDeviceDiscardRectanglePropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_DISCARD_RECTANGLE_STATE_CREATE_INFO_EXT:
                safe_pNext = new safe_VkPipelineDiscardRectangleStateCreateInfoEXT(reinterpret_cast<const VkPipelineDiscardRectangleStateCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceConservativeRasterizationPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT:
                safe_pNext = new safe_VkPipelineRasterizationConservativeStateCreateInfoEXT(reinterpret_cast<const VkPipelineRasterizationConservativeStateCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceDepthClipEnableFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT:
                safe_pNext = new safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT(reinterpret_cast<const VkPipelineRasterizationDepthClipStateCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RELAXED_LINE_RASTERIZATION_FEATURES_IMG:
                safe_pNext = new safe_VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG(reinterpret_cast<const VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT:
                safe_pNext = new safe_VkDebugUtilsObjectNameInfoEXT(reinterpret_cast<const VkDebugUtilsObjectNameInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT:
                safe_pNext = new safe_VkDebugUtilsMessengerCreateInfoEXT(reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT *>(pNext), copy_state, false);
                break;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
            case VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_USAGE_ANDROID:
                safe_pNext = new safe_VkAndroidHardwareBufferUsageANDROID(reinterpret_cast<const VkAndroidHardwareBufferUsageANDROID *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID:
                safe_pNext = new safe_VkAndroidHardwareBufferFormatPropertiesANDROID(reinterpret_cast<const VkAndroidHardwareBufferFormatPropertiesANDROID *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID:
                safe_pNext = new safe_VkImportAndroidHardwareBufferInfoANDROID(reinterpret_cast<const VkImportAndroidHardwareBufferInfoANDROID *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID:
                safe_pNext = new safe_VkExternalFormatANDROID(reinterpret_cast<const VkExternalFormatANDROID *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_2_ANDROID:
                safe_pNext = new safe_VkAndroidHardwareBufferFormatProperties2ANDROID(reinterpret_cast<const VkAndroidHardwareBufferFormatProperties2ANDROID *>(pNext), copy_state, false);
                break;
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ENQUEUE_FEATURES_AMDX:
                safe_pNext = new safe_VkPhysicalDeviceShaderEnqueueFeaturesAMDX(reinterpret_cast<const VkPhysicalDeviceShaderEnqueueFeaturesAMDX *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ENQUEUE_PROPERTIES_AMDX:
                safe_pNext = new safe_VkPhysicalDeviceShaderEnqueuePropertiesAMDX(reinterpret_cast<const VkPhysicalDeviceShaderEnqueuePropertiesAMDX *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXECUTION_GRAPH_PIPELINE_CREATE_INFO_AMDX:
                safe_pNext = new safe_VkExecutionGraphPipelineCreateInfoAMDX(reinterpret_cast<const VkExecutionGraphPipelineCreateInfoAMDX *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_NODE_CREATE_INFO_AMDX:
                safe_pNext = new safe_VkPipelineShaderStageNodeCreateInfoAMDX(reinterpret_cast<const VkPipelineShaderStageNodeCreateInfoAMDX *>(pNext), copy_state, false);
                break;
#endif  // VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_ATTACHMENT_SAMPLE_COUNT_INFO_AMD:
                safe_pNext = new safe_VkAttachmentSampleCountInfoAMD(reinterpret_cast<const VkAttachmentSampleCountInfoAMD *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT:
                safe_pNext = new safe_VkSampleLocationsInfoEXT(reinterpret_cast<const VkSampleLocationsInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDER_PASS_SAMPLE_LOCATIONS_BEGIN_INFO_EXT:
                safe_pNext = new safe_VkRenderPassSampleLocationsBeginInfoEXT(reinterpret_cast<const VkRenderPassSampleLocationsBeginInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT:
                safe_pNext = new safe_VkPipelineSampleLocationsStateCreateInfoEXT(reinterpret_cast<const VkPipelineSampleLocationsStateCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceSampleLocationsPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceSampleLocationsPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT:
                safe_pNext = new safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT(reinterpret_cast<const VkPipelineColorBlendAdvancedStateCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_TO_COLOR_STATE_CREATE_INFO_NV:
                safe_pNext = new safe_VkPipelineCoverageToColorStateCreateInfoNV(reinterpret_cast<const VkPipelineCoverageToColorStateCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_MODULATION_STATE_CREATE_INFO_NV:
                safe_pNext = new safe_VkPipelineCoverageModulationStateCreateInfoNV(reinterpret_cast<const VkPipelineCoverageModulationStateCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV(reinterpret_cast<const VkPhysicalDeviceShaderSMBuiltinsPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV(reinterpret_cast<const VkPhysicalDeviceShaderSMBuiltinsFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT:
                safe_pNext = new safe_VkDrmFormatModifierPropertiesListEXT(reinterpret_cast<const VkDrmFormatModifierPropertiesListEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT:
                safe_pNext = new safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT(reinterpret_cast<const VkPhysicalDeviceImageDrmFormatModifierInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT:
                safe_pNext = new safe_VkImageDrmFormatModifierListCreateInfoEXT(reinterpret_cast<const VkImageDrmFormatModifierListCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT:
                safe_pNext = new safe_VkImageDrmFormatModifierExplicitCreateInfoEXT(reinterpret_cast<const VkImageDrmFormatModifierExplicitCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_2_EXT:
                safe_pNext = new safe_VkDrmFormatModifierPropertiesList2EXT(reinterpret_cast<const VkDrmFormatModifierPropertiesList2EXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SHADER_MODULE_VALIDATION_CACHE_CREATE_INFO_EXT:
                safe_pNext = new safe_VkShaderModuleValidationCacheCreateInfoEXT(reinterpret_cast<const VkShaderModuleValidationCacheCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV:
                safe_pNext = new safe_VkPipelineViewportShadingRateImageStateCreateInfoNV(reinterpret_cast<const VkPipelineViewportShadingRateImageStateCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceShadingRateImageFeaturesNV(reinterpret_cast<const VkPhysicalDeviceShadingRateImageFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceShadingRateImagePropertiesNV(reinterpret_cast<const VkPhysicalDeviceShadingRateImagePropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_COARSE_SAMPLE_ORDER_STATE_CREATE_INFO_NV:
                safe_pNext = new safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV(reinterpret_cast<const VkPipelineViewportCoarseSampleOrderStateCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV:
                safe_pNext = new safe_VkWriteDescriptorSetAccelerationStructureNV(reinterpret_cast<const VkWriteDescriptorSetAccelerationStructureNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceRayTracingPropertiesNV(reinterpret_cast<const VkPhysicalDeviceRayTracingPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_REPRESENTATIVE_FRAGMENT_TEST_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV(reinterpret_cast<const VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_REPRESENTATIVE_FRAGMENT_TEST_STATE_CREATE_INFO_NV:
                safe_pNext = new safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV(reinterpret_cast<const VkPipelineRepresentativeFragmentTestStateCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_IMAGE_FORMAT_INFO_EXT:
                safe_pNext = new safe_VkPhysicalDeviceImageViewImageFormatInfoEXT(reinterpret_cast<const VkPhysicalDeviceImageViewImageFormatInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_FILTER_CUBIC_IMAGE_VIEW_IMAGE_FORMAT_PROPERTIES_EXT:
                safe_pNext = new safe_VkFilterCubicImageViewImageFormatPropertiesEXT(reinterpret_cast<const VkFilterCubicImageViewImageFormatPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT:
                safe_pNext = new safe_VkImportMemoryHostPointerInfoEXT(reinterpret_cast<const VkImportMemoryHostPointerInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceExternalMemoryHostPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_COMPILER_CONTROL_CREATE_INFO_AMD:
                safe_pNext = new safe_VkPipelineCompilerControlCreateInfoAMD(reinterpret_cast<const VkPipelineCompilerControlCreateInfoAMD *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_AMD:
                safe_pNext = new safe_VkPhysicalDeviceShaderCorePropertiesAMD(reinterpret_cast<const VkPhysicalDeviceShaderCorePropertiesAMD *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_MEMORY_OVERALLOCATION_CREATE_INFO_AMD:
                safe_pNext = new safe_VkDeviceMemoryOverallocationCreateInfoAMD(reinterpret_cast<const VkDeviceMemoryOverallocationCreateInfoAMD *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT *>(pNext), copy_state, false);
                break;
#ifdef VK_USE_PLATFORM_GGP
            case VK_STRUCTURE_TYPE_PRESENT_FRAME_TOKEN_GGP:
                safe_pNext = new safe_VkPresentFrameTokenGGP(reinterpret_cast<const VkPresentFrameTokenGGP *>(pNext), copy_state, false);
                break;
#endif  // VK_USE_PLATFORM_GGP
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceMeshShaderFeaturesNV(reinterpret_cast<const VkPhysicalDeviceMeshShaderFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceMeshShaderPropertiesNV(reinterpret_cast<const VkPhysicalDeviceMeshShaderPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_FOOTPRINT_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV(reinterpret_cast<const VkPhysicalDeviceShaderImageFootprintFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_EXCLUSIVE_SCISSOR_STATE_CREATE_INFO_NV:
                safe_pNext = new safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV(reinterpret_cast<const VkPipelineViewportExclusiveScissorStateCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXCLUSIVE_SCISSOR_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceExclusiveScissorFeaturesNV(reinterpret_cast<const VkPhysicalDeviceExclusiveScissorFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_NV:
                safe_pNext = new safe_VkQueueFamilyCheckpointPropertiesNV(reinterpret_cast<const VkQueueFamilyCheckpointPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_2_NV:
                safe_pNext = new safe_VkQueueFamilyCheckpointProperties2NV(reinterpret_cast<const VkQueueFamilyCheckpointProperties2NV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_FUNCTIONS_2_FEATURES_INTEL:
                safe_pNext = new safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL(reinterpret_cast<const VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_QUERY_CREATE_INFO_INTEL:
                safe_pNext = new safe_VkQueryPoolPerformanceQueryCreateInfoINTEL(reinterpret_cast<const VkQueryPoolPerformanceQueryCreateInfoINTEL *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDevicePCIBusInfoPropertiesEXT(reinterpret_cast<const VkPhysicalDevicePCIBusInfoPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DISPLAY_NATIVE_HDR_SURFACE_CAPABILITIES_AMD:
                safe_pNext = new safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD(reinterpret_cast<const VkDisplayNativeHdrSurfaceCapabilitiesAMD *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SWAPCHAIN_DISPLAY_NATIVE_HDR_CREATE_INFO_AMD:
                safe_pNext = new safe_VkSwapchainDisplayNativeHdrCreateInfoAMD(reinterpret_cast<const VkSwapchainDisplayNativeHdrCreateInfoAMD *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT:
                safe_pNext = new safe_VkRenderPassFragmentDensityMapCreateInfoEXT(reinterpret_cast<const VkRenderPassFragmentDensityMapCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_DENSITY_MAP_ATTACHMENT_INFO_EXT:
                safe_pNext = new safe_VkRenderingFragmentDensityMapAttachmentInfoEXT(reinterpret_cast<const VkRenderingFragmentDensityMapAttachmentInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_2_AMD:
                safe_pNext = new safe_VkPhysicalDeviceShaderCoreProperties2AMD(reinterpret_cast<const VkPhysicalDeviceShaderCoreProperties2AMD *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COHERENT_MEMORY_FEATURES_AMD:
                safe_pNext = new safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD(reinterpret_cast<const VkPhysicalDeviceCoherentMemoryFeaturesAMD *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT(reinterpret_cast<const VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceMemoryBudgetPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceMemoryPriorityFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_MEMORY_PRIORITY_ALLOCATE_INFO_EXT:
                safe_pNext = new safe_VkMemoryPriorityAllocateInfoEXT(reinterpret_cast<const VkMemoryPriorityAllocateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEDICATED_ALLOCATION_IMAGE_ALIASING_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV(reinterpret_cast<const VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceBufferDeviceAddressFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_CREATE_INFO_EXT:
                safe_pNext = new safe_VkBufferDeviceAddressCreateInfoEXT(reinterpret_cast<const VkBufferDeviceAddressCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT:
                safe_pNext = new safe_VkValidationFeaturesEXT(reinterpret_cast<const VkValidationFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV(reinterpret_cast<const VkPhysicalDeviceCooperativeMatrixFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV(reinterpret_cast<const VkPhysicalDeviceCooperativeMatrixPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COVERAGE_REDUCTION_MODE_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV(reinterpret_cast<const VkPhysicalDeviceCoverageReductionModeFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_REDUCTION_STATE_CREATE_INFO_NV:
                safe_pNext = new safe_VkPipelineCoverageReductionStateCreateInfoNV(reinterpret_cast<const VkPipelineCoverageReductionStateCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_IMAGE_ARRAYS_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceYcbcrImageArraysFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceProvokingVertexFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceProvokingVertexFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceProvokingVertexPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceProvokingVertexPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_PROVOKING_VERTEX_STATE_CREATE_INFO_EXT:
                safe_pNext = new safe_VkPipelineRasterizationProvokingVertexStateCreateInfoEXT(reinterpret_cast<const VkPipelineRasterizationProvokingVertexStateCreateInfoEXT *>(pNext), copy_state, false);
                break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT:
                safe_pNext = new safe_VkSurfaceFullScreenExclusiveInfoEXT(reinterpret_cast<const VkSurfaceFullScreenExclusiveInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_FULL_SCREEN_EXCLUSIVE_EXT:
                safe_pNext = new safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT(reinterpret_cast<const VkSurfaceCapabilitiesFullScreenExclusiveEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT:
                safe_pNext = new safe_VkSurfaceFullScreenExclusiveWin32InfoEXT(reinterpret_cast<const VkSurfaceFullScreenExclusiveWin32InfoEXT *>(pNext), copy_state, false);
                break;
#endif  // VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceExtendedDynamicStateFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAP_MEMORY_PLACED_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceMapMemoryPlacedFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceMapMemoryPlacedFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAP_MEMORY_PLACED_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceMapMemoryPlacedPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceMapMemoryPlacedPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_MEMORY_MAP_PLACED_INFO_EXT:
                safe_pNext = new safe_VkMemoryMapPlacedInfoEXT(reinterpret_cast<const VkMemoryMapPlacedInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_2_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT(reinterpret_cast<const VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV(reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV(reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_SHADER_GROUPS_CREATE_INFO_NV:
                safe_pNext = new safe_VkGraphicsPipelineShaderGroupsCreateInfoNV(reinterpret_cast<const VkGraphicsPipelineShaderGroupsCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INHERITED_VIEWPORT_SCISSOR_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceInheritedViewportScissorFeaturesNV(reinterpret_cast<const VkPhysicalDeviceInheritedViewportScissorFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_VIEWPORT_SCISSOR_INFO_NV:
                safe_pNext = new safe_VkCommandBufferInheritanceViewportScissorInfoNV(reinterpret_cast<const VkCommandBufferInheritanceViewportScissorInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDER_PASS_TRANSFORM_BEGIN_INFO_QCOM:
                safe_pNext = new safe_VkRenderPassTransformBeginInfoQCOM(reinterpret_cast<const VkRenderPassTransformBeginInfoQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDER_PASS_TRANSFORM_INFO_QCOM:
                safe_pNext = new safe_VkCommandBufferInheritanceRenderPassTransformInfoQCOM(reinterpret_cast<const VkCommandBufferInheritanceRenderPassTransformInfoQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_BIAS_CONTROL_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceDepthBiasControlFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceDepthBiasControlFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEPTH_BIAS_REPRESENTATION_INFO_EXT:
                safe_pNext = new safe_VkDepthBiasRepresentationInfoEXT(reinterpret_cast<const VkDepthBiasRepresentationInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_MEMORY_REPORT_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceDeviceMemoryReportFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceDeviceMemoryReportFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_DEVICE_MEMORY_REPORT_CREATE_INFO_EXT:
                safe_pNext = new safe_VkDeviceDeviceMemoryReportCreateInfoEXT(reinterpret_cast<const VkDeviceDeviceMemoryReportCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT:
                safe_pNext = new safe_VkSamplerCustomBorderColorCreateInfoEXT(reinterpret_cast<const VkSamplerCustomBorderColorCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceCustomBorderColorPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceCustomBorderColorFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_BARRIER_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDevicePresentBarrierFeaturesNV(reinterpret_cast<const VkPhysicalDevicePresentBarrierFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_PRESENT_BARRIER_NV:
                safe_pNext = new safe_VkSurfaceCapabilitiesPresentBarrierNV(reinterpret_cast<const VkSurfaceCapabilitiesPresentBarrierNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_BARRIER_CREATE_INFO_NV:
                safe_pNext = new safe_VkSwapchainPresentBarrierCreateInfoNV(reinterpret_cast<const VkSwapchainPresentBarrierCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DIAGNOSTICS_CONFIG_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceDiagnosticsConfigFeaturesNV(reinterpret_cast<const VkPhysicalDeviceDiagnosticsConfigFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_DIAGNOSTICS_CONFIG_CREATE_INFO_NV:
                safe_pNext = new safe_VkDeviceDiagnosticsConfigCreateInfoNV(reinterpret_cast<const VkDeviceDiagnosticsConfigCreateInfoNV *>(pNext), copy_state, false);
                break;
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUDA_KERNEL_LAUNCH_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceCudaKernelLaunchFeaturesNV(reinterpret_cast<const VkPhysicalDeviceCudaKernelLaunchFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUDA_KERNEL_LAUNCH_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceCudaKernelLaunchPropertiesNV(reinterpret_cast<const VkPhysicalDeviceCudaKernelLaunchPropertiesNV *>(pNext), copy_state, false);
                break;
#endif  // VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_SHADING_FEATURES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceTileShadingFeaturesQCOM(reinterpret_cast<const VkPhysicalDeviceTileShadingFeaturesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_SHADING_PROPERTIES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceTileShadingPropertiesQCOM(reinterpret_cast<const VkPhysicalDeviceTileShadingPropertiesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDER_PASS_TILE_SHADING_CREATE_INFO_QCOM:
                safe_pNext = new safe_VkRenderPassTileShadingCreateInfoQCOM(reinterpret_cast<const VkRenderPassTileShadingCreateInfoQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_QUERY_LOW_LATENCY_SUPPORT_NV:
                safe_pNext = new safe_VkQueryLowLatencySupportNV(reinterpret_cast<const VkQueryLowLatencySupportNV *>(pNext), copy_state, false);
                break;
#ifdef VK_USE_PLATFORM_METAL_EXT
            case VK_STRUCTURE_TYPE_EXPORT_METAL_OBJECT_CREATE_INFO_EXT:
                safe_pNext = new safe_VkExportMetalObjectCreateInfoEXT(reinterpret_cast<const VkExportMetalObjectCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXPORT_METAL_DEVICE_INFO_EXT:
                safe_pNext = new safe_VkExportMetalDeviceInfoEXT(reinterpret_cast<const VkExportMetalDeviceInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXPORT_METAL_COMMAND_QUEUE_INFO_EXT:
                safe_pNext = new safe_VkExportMetalCommandQueueInfoEXT(reinterpret_cast<const VkExportMetalCommandQueueInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXPORT_METAL_BUFFER_INFO_EXT:
                safe_pNext = new safe_VkExportMetalBufferInfoEXT(reinterpret_cast<const VkExportMetalBufferInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMPORT_METAL_BUFFER_INFO_EXT:
                safe_pNext = new safe_VkImportMetalBufferInfoEXT(reinterpret_cast<const VkImportMetalBufferInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXPORT_METAL_TEXTURE_INFO_EXT:
                safe_pNext = new safe_VkExportMetalTextureInfoEXT(reinterpret_cast<const VkExportMetalTextureInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMPORT_METAL_TEXTURE_INFO_EXT:
                safe_pNext = new safe_VkImportMetalTextureInfoEXT(reinterpret_cast<const VkImportMetalTextureInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXPORT_METAL_IO_SURFACE_INFO_EXT:
                safe_pNext = new safe_VkExportMetalIOSurfaceInfoEXT(reinterpret_cast<const VkExportMetalIOSurfaceInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMPORT_METAL_IO_SURFACE_INFO_EXT:
                safe_pNext = new safe_VkImportMetalIOSurfaceInfoEXT(reinterpret_cast<const VkImportMetalIOSurfaceInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXPORT_METAL_SHARED_EVENT_INFO_EXT:
                safe_pNext = new safe_VkExportMetalSharedEventInfoEXT(reinterpret_cast<const VkExportMetalSharedEventInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMPORT_METAL_SHARED_EVENT_INFO_EXT:
                safe_pNext = new safe_VkImportMetalSharedEventInfoEXT(reinterpret_cast<const VkImportMetalSharedEventInfoEXT *>(pNext), copy_state, false);
                break;
#endif  // VK_USE_PLATFORM_METAL_EXT
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceDescriptorBufferPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceDescriptorBufferPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_DENSITY_MAP_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceDescriptorBufferFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceDescriptorBufferFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_PUSH_DESCRIPTOR_BUFFER_HANDLE_EXT:
                safe_pNext = new safe_VkDescriptorBufferBindingPushDescriptorBufferHandleEXT(reinterpret_cast<const VkDescriptorBufferBindingPushDescriptorBufferHandleEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_OPAQUE_CAPTURE_DESCRIPTOR_DATA_CREATE_INFO_EXT:
                safe_pNext = new safe_VkOpaqueCaptureDescriptorDataCreateInfoEXT(reinterpret_cast<const VkOpaqueCaptureDescriptorDataCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT:
                safe_pNext = new safe_VkGraphicsPipelineLibraryCreateInfoEXT(reinterpret_cast<const VkGraphicsPipelineLibraryCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EARLY_AND_LATE_FRAGMENT_TESTS_FEATURES_AMD:
                safe_pNext = new safe_VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD(reinterpret_cast<const VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV(reinterpret_cast<const VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV(reinterpret_cast<const VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_ENUM_STATE_CREATE_INFO_NV:
                safe_pNext = new safe_VkPipelineFragmentShadingRateEnumStateCreateInfoNV(reinterpret_cast<const VkPipelineFragmentShadingRateEnumStateCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_MOTION_TRIANGLES_DATA_NV:
                safe_pNext = new safe_VkAccelerationStructureGeometryMotionTrianglesDataNV(reinterpret_cast<const VkAccelerationStructureGeometryMotionTrianglesDataNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MOTION_INFO_NV:
                safe_pNext = new safe_VkAccelerationStructureMotionInfoNV(reinterpret_cast<const VkAccelerationStructureMotionInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MOTION_BLUR_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceRayTracingMotionBlurFeaturesNV(reinterpret_cast<const VkPhysicalDeviceRayTracingMotionBlurFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_2_PLANE_444_FORMATS_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceFragmentDensityMap2FeaturesEXT(reinterpret_cast<const VkPhysicalDeviceFragmentDensityMap2FeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceFragmentDensityMap2PropertiesEXT(reinterpret_cast<const VkPhysicalDeviceFragmentDensityMap2PropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_COPY_COMMAND_TRANSFORM_INFO_QCOM:
                safe_pNext = new safe_VkCopyCommandTransformInfoQCOM(reinterpret_cast<const VkCopyCommandTransformInfoQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceImageCompressionControlFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceImageCompressionControlFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_CONTROL_EXT:
                safe_pNext = new safe_VkImageCompressionControlEXT(reinterpret_cast<const VkImageCompressionControlEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_PROPERTIES_EXT:
                safe_pNext = new safe_VkImageCompressionPropertiesEXT(reinterpret_cast<const VkImageCompressionPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_4444_FORMATS_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDevice4444FormatsFeaturesEXT(reinterpret_cast<const VkPhysicalDevice4444FormatsFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FAULT_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceFaultFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceFaultFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RGBA10X6_FORMATS_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT:
                safe_pNext = new safe_VkMutableDescriptorTypeCreateInfoEXT(reinterpret_cast<const VkMutableDescriptorTypeCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRM_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceDrmPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceDrmPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ADDRESS_BINDING_REPORT_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceAddressBindingReportFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceAddressBindingReportFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_ADDRESS_BINDING_CALLBACK_DATA_EXT:
                safe_pNext = new safe_VkDeviceAddressBindingCallbackDataEXT(reinterpret_cast<const VkDeviceAddressBindingCallbackDataEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_CONTROL_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceDepthClipControlFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceDepthClipControlFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLIP_CONTROL_CREATE_INFO_EXT:
                safe_pNext = new safe_VkPipelineViewportDepthClipControlCreateInfoEXT(reinterpret_cast<const VkPipelineViewportDepthClipControlCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT(reinterpret_cast<const VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT *>(pNext), copy_state, false);
                break;
#ifdef VK_USE_PLATFORM_FUCHSIA
            case VK_STRUCTURE_TYPE_IMPORT_MEMORY_ZIRCON_HANDLE_INFO_FUCHSIA:
                safe_pNext = new safe_VkImportMemoryZirconHandleInfoFUCHSIA(reinterpret_cast<const VkImportMemoryZirconHandleInfoFUCHSIA *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMPORT_MEMORY_BUFFER_COLLECTION_FUCHSIA:
                safe_pNext = new safe_VkImportMemoryBufferCollectionFUCHSIA(reinterpret_cast<const VkImportMemoryBufferCollectionFUCHSIA *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_BUFFER_COLLECTION_IMAGE_CREATE_INFO_FUCHSIA:
                safe_pNext = new safe_VkBufferCollectionImageCreateInfoFUCHSIA(reinterpret_cast<const VkBufferCollectionImageCreateInfoFUCHSIA *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_BUFFER_COLLECTION_BUFFER_CREATE_INFO_FUCHSIA:
                safe_pNext = new safe_VkBufferCollectionBufferCreateInfoFUCHSIA(reinterpret_cast<const VkBufferCollectionBufferCreateInfoFUCHSIA *>(pNext), copy_state, false);
                break;
#endif  // VK_USE_PLATFORM_FUCHSIA
            case VK_STRUCTURE_TYPE_SUBPASS_SHADING_PIPELINE_CREATE_INFO_HUAWEI:
                safe_pNext = new safe_VkSubpassShadingPipelineCreateInfoHUAWEI(reinterpret_cast<const VkSubpassShadingPipelineCreateInfoHUAWEI *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_SHADING_FEATURES_HUAWEI:
                safe_pNext = new safe_VkPhysicalDeviceSubpassShadingFeaturesHUAWEI(reinterpret_cast<const VkPhysicalDeviceSubpassShadingFeaturesHUAWEI *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_SHADING_PROPERTIES_HUAWEI:
                safe_pNext = new safe_VkPhysicalDeviceSubpassShadingPropertiesHUAWEI(reinterpret_cast<const VkPhysicalDeviceSubpassShadingPropertiesHUAWEI *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INVOCATION_MASK_FEATURES_HUAWEI:
                safe_pNext = new safe_VkPhysicalDeviceInvocationMaskFeaturesHUAWEI(reinterpret_cast<const VkPhysicalDeviceInvocationMaskFeaturesHUAWEI *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_RDMA_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceExternalMemoryRDMAFeaturesNV(reinterpret_cast<const VkPhysicalDeviceExternalMemoryRDMAFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROPERTIES_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDevicePipelinePropertiesFeaturesEXT(reinterpret_cast<const VkPhysicalDevicePipelinePropertiesFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAME_BOUNDARY_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceFrameBoundaryFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceFrameBoundaryFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_FRAME_BOUNDARY_EXT:
                safe_pNext = new safe_VkFrameBoundaryEXT(reinterpret_cast<const VkFrameBoundaryEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SUBPASS_RESOLVE_PERFORMANCE_QUERY_EXT:
                safe_pNext = new safe_VkSubpassResolvePerformanceQueryEXT(reinterpret_cast<const VkSubpassResolvePerformanceQueryEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_INFO_EXT:
                safe_pNext = new safe_VkMultisampledRenderToSingleSampledInfoEXT(reinterpret_cast<const VkMultisampledRenderToSingleSampledInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT(reinterpret_cast<const VkPhysicalDeviceExtendedDynamicState2FeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceColorWriteEnableFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_COLOR_WRITE_CREATE_INFO_EXT:
                safe_pNext = new safe_VkPipelineColorWriteCreateInfoEXT(reinterpret_cast<const VkPipelineColorWriteCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVES_GENERATED_QUERY_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT(reinterpret_cast<const VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_MIN_LOD_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceImageViewMinLodFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceImageViewMinLodFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_VIEW_MIN_LOD_CREATE_INFO_EXT:
                safe_pNext = new safe_VkImageViewMinLodCreateInfoEXT(reinterpret_cast<const VkImageViewMinLodCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceMultiDrawFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceMultiDrawFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceMultiDrawPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceMultiDrawPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_2D_VIEW_OF_3D_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceImage2DViewOf3DFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceImage2DViewOf3DFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceShaderTileImageFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceShaderTileImageFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceShaderTileImagePropertiesEXT(reinterpret_cast<const VkPhysicalDeviceShaderTileImagePropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceOpacityMicromapFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceOpacityMicromapFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceOpacityMicromapPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceOpacityMicromapPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_OPACITY_MICROMAP_EXT:
                safe_pNext = new safe_VkAccelerationStructureTrianglesOpacityMicromapEXT(reinterpret_cast<const VkAccelerationStructureTrianglesOpacityMicromapEXT *>(pNext), copy_state, false);
                break;
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISPLACEMENT_MICROMAP_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceDisplacementMicromapFeaturesNV(reinterpret_cast<const VkPhysicalDeviceDisplacementMicromapFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISPLACEMENT_MICROMAP_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceDisplacementMicromapPropertiesNV(reinterpret_cast<const VkPhysicalDeviceDisplacementMicromapPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_DISPLACEMENT_MICROMAP_NV:
                safe_pNext = new safe_VkAccelerationStructureTrianglesDisplacementMicromapNV(reinterpret_cast<const VkAccelerationStructureTrianglesDisplacementMicromapNV *>(pNext), copy_state, false);
                break;
#endif  // VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_FEATURES_HUAWEI:
                safe_pNext = new safe_VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI(reinterpret_cast<const VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_PROPERTIES_HUAWEI:
                safe_pNext = new safe_VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI(reinterpret_cast<const VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_VRS_FEATURES_HUAWEI:
                safe_pNext = new safe_VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI(reinterpret_cast<const VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BORDER_COLOR_SWIZZLE_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceBorderColorSwizzleFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceBorderColorSwizzleFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SAMPLER_BORDER_COLOR_COMPONENT_MAPPING_CREATE_INFO_EXT:
                safe_pNext = new safe_VkSamplerBorderColorComponentMappingCreateInfoEXT(reinterpret_cast<const VkSamplerBorderColorComponentMappingCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PAGEABLE_DEVICE_LOCAL_MEMORY_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT(reinterpret_cast<const VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_ARM:
                safe_pNext = new safe_VkPhysicalDeviceShaderCorePropertiesARM(reinterpret_cast<const VkPhysicalDeviceShaderCorePropertiesARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_QUEUE_SHADER_CORE_CONTROL_CREATE_INFO_ARM:
                safe_pNext = new safe_VkDeviceQueueShaderCoreControlCreateInfoARM(reinterpret_cast<const VkDeviceQueueShaderCoreControlCreateInfoARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCHEDULING_CONTROLS_FEATURES_ARM:
                safe_pNext = new safe_VkPhysicalDeviceSchedulingControlsFeaturesARM(reinterpret_cast<const VkPhysicalDeviceSchedulingControlsFeaturesARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCHEDULING_CONTROLS_PROPERTIES_ARM:
                safe_pNext = new safe_VkPhysicalDeviceSchedulingControlsPropertiesARM(reinterpret_cast<const VkPhysicalDeviceSchedulingControlsPropertiesARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_SLICED_VIEW_OF_3D_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_VIEW_SLICED_CREATE_INFO_EXT:
                safe_pNext = new safe_VkImageViewSlicedCreateInfoEXT(reinterpret_cast<const VkImageViewSlicedCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_SET_HOST_MAPPING_FEATURES_VALVE:
                safe_pNext = new safe_VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE(reinterpret_cast<const VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NON_SEAMLESS_CUBE_MAP_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RENDER_PASS_STRIPED_FEATURES_ARM:
                safe_pNext = new safe_VkPhysicalDeviceRenderPassStripedFeaturesARM(reinterpret_cast<const VkPhysicalDeviceRenderPassStripedFeaturesARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RENDER_PASS_STRIPED_PROPERTIES_ARM:
                safe_pNext = new safe_VkPhysicalDeviceRenderPassStripedPropertiesARM(reinterpret_cast<const VkPhysicalDeviceRenderPassStripedPropertiesARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDER_PASS_STRIPE_BEGIN_INFO_ARM:
                safe_pNext = new safe_VkRenderPassStripeBeginInfoARM(reinterpret_cast<const VkRenderPassStripeBeginInfoARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDER_PASS_STRIPE_SUBMIT_INFO_ARM:
                safe_pNext = new safe_VkRenderPassStripeSubmitInfoARM(reinterpret_cast<const VkRenderPassStripeSubmitInfoARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapOffsetPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_OFFSET_END_INFO_EXT:
                safe_pNext = new safe_VkRenderPassFragmentDensityMapOffsetEndInfoEXT(reinterpret_cast<const VkRenderPassFragmentDensityMapOffsetEndInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceCopyMemoryIndirectFeaturesNV(reinterpret_cast<const VkPhysicalDeviceCopyMemoryIndirectFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceCopyMemoryIndirectPropertiesNV(reinterpret_cast<const VkPhysicalDeviceCopyMemoryIndirectPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceMemoryDecompressionFeaturesNV(reinterpret_cast<const VkPhysicalDeviceMemoryDecompressionFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceMemoryDecompressionPropertiesNV(reinterpret_cast<const VkPhysicalDeviceMemoryDecompressionPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_COMPUTE_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV(reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_INDIRECT_BUFFER_INFO_NV:
                safe_pNext = new safe_VkComputePipelineIndirectBufferInfoNV(reinterpret_cast<const VkComputePipelineIndirectBufferInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_LINEAR_SWEPT_SPHERES_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV(reinterpret_cast<const VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_LINEAR_SWEPT_SPHERES_DATA_NV:
                safe_pNext = new safe_VkAccelerationStructureGeometryLinearSweptSpheresDataNV(reinterpret_cast<const VkAccelerationStructureGeometryLinearSweptSpheresDataNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_SPHERES_DATA_NV:
                safe_pNext = new safe_VkAccelerationStructureGeometrySpheresDataNV(reinterpret_cast<const VkAccelerationStructureGeometrySpheresDataNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINEAR_COLOR_ATTACHMENT_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceLinearColorAttachmentFeaturesNV(reinterpret_cast<const VkPhysicalDeviceLinearColorAttachmentFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_VIEW_SAMPLE_WEIGHT_CREATE_INFO_QCOM:
                safe_pNext = new safe_VkImageViewSampleWeightCreateInfoQCOM(reinterpret_cast<const VkImageViewSampleWeightCreateInfoQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_FEATURES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceImageProcessingFeaturesQCOM(reinterpret_cast<const VkPhysicalDeviceImageProcessingFeaturesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_PROPERTIES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceImageProcessingPropertiesQCOM(reinterpret_cast<const VkPhysicalDeviceImageProcessingPropertiesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceNestedCommandBufferFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceNestedCommandBufferFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceNestedCommandBufferPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceNestedCommandBufferPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_ACQUIRE_UNMODIFIED_EXT:
                safe_pNext = new safe_VkExternalMemoryAcquireUnmodifiedEXT(reinterpret_cast<const VkExternalMemoryAcquireUnmodifiedEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceExtendedDynamicState3FeaturesEXT(reinterpret_cast<const VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceExtendedDynamicState3PropertiesEXT(reinterpret_cast<const VkPhysicalDeviceExtendedDynamicState3PropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_MERGE_FEEDBACK_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDER_PASS_CREATION_CONTROL_EXT:
                safe_pNext = new safe_VkRenderPassCreationControlEXT(reinterpret_cast<const VkRenderPassCreationControlEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDER_PASS_CREATION_FEEDBACK_CREATE_INFO_EXT:
                safe_pNext = new safe_VkRenderPassCreationFeedbackCreateInfoEXT(reinterpret_cast<const VkRenderPassCreationFeedbackCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDER_PASS_SUBPASS_FEEDBACK_CREATE_INFO_EXT:
                safe_pNext = new safe_VkRenderPassSubpassFeedbackCreateInfoEXT(reinterpret_cast<const VkRenderPassSubpassFeedbackCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_LIST_LUNARG:
                safe_pNext = new safe_VkDirectDriverLoadingListLUNARG(reinterpret_cast<const VkDirectDriverLoadingListLUNARG *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_TENSOR_DESCRIPTION_ARM:
                safe_pNext = new safe_VkTensorDescriptionARM(reinterpret_cast<const VkTensorDescriptionARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_TENSOR_ARM:
                safe_pNext = new safe_VkWriteDescriptorSetTensorARM(reinterpret_cast<const VkWriteDescriptorSetTensorARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_TENSOR_FORMAT_PROPERTIES_ARM:
                safe_pNext = new safe_VkTensorFormatPropertiesARM(reinterpret_cast<const VkTensorFormatPropertiesARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TENSOR_PROPERTIES_ARM:
                safe_pNext = new safe_VkPhysicalDeviceTensorPropertiesARM(reinterpret_cast<const VkPhysicalDeviceTensorPropertiesARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_TENSOR_MEMORY_BARRIER_ARM:
                safe_pNext = new safe_VkTensorMemoryBarrierARM(reinterpret_cast<const VkTensorMemoryBarrierARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_TENSOR_DEPENDENCY_INFO_ARM:
                safe_pNext = new safe_VkTensorDependencyInfoARM(reinterpret_cast<const VkTensorDependencyInfoARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TENSOR_FEATURES_ARM:
                safe_pNext = new safe_VkPhysicalDeviceTensorFeaturesARM(reinterpret_cast<const VkPhysicalDeviceTensorFeaturesARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_TENSOR_ARM:
                safe_pNext = new safe_VkMemoryDedicatedAllocateInfoTensorARM(reinterpret_cast<const VkMemoryDedicatedAllocateInfoTensorARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_TENSOR_CREATE_INFO_ARM:
                safe_pNext = new safe_VkExternalMemoryTensorCreateInfoARM(reinterpret_cast<const VkExternalMemoryTensorCreateInfoARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_TENSOR_FEATURES_ARM:
                safe_pNext = new safe_VkPhysicalDeviceDescriptorBufferTensorFeaturesARM(reinterpret_cast<const VkPhysicalDeviceDescriptorBufferTensorFeaturesARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_TENSOR_PROPERTIES_ARM:
                safe_pNext = new safe_VkPhysicalDeviceDescriptorBufferTensorPropertiesARM(reinterpret_cast<const VkPhysicalDeviceDescriptorBufferTensorPropertiesARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DESCRIPTOR_GET_TENSOR_INFO_ARM:
                safe_pNext = new safe_VkDescriptorGetTensorInfoARM(reinterpret_cast<const VkDescriptorGetTensorInfoARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_FRAME_BOUNDARY_TENSORS_ARM:
                safe_pNext = new safe_VkFrameBoundaryTensorsARM(reinterpret_cast<const VkFrameBoundaryTensorsARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_MODULE_IDENTIFIER_CREATE_INFO_EXT:
                safe_pNext = new safe_VkPipelineShaderStageModuleIdentifierCreateInfoEXT(reinterpret_cast<const VkPipelineShaderStageModuleIdentifierCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceOpticalFlowFeaturesNV(reinterpret_cast<const VkPhysicalDeviceOpticalFlowFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceOpticalFlowPropertiesNV(reinterpret_cast<const VkPhysicalDeviceOpticalFlowPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_OPTICAL_FLOW_IMAGE_FORMAT_INFO_NV:
                safe_pNext = new safe_VkOpticalFlowImageFormatInfoNV(reinterpret_cast<const VkOpticalFlowImageFormatInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_OPTICAL_FLOW_SESSION_CREATE_PRIVATE_DATA_INFO_NV:
                safe_pNext = new safe_VkOpticalFlowSessionCreatePrivateDataInfoNV(reinterpret_cast<const VkOpticalFlowSessionCreatePrivateDataInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_DITHERING_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceLegacyDitheringFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceLegacyDitheringFeaturesEXT *>(pNext), copy_state, false);
                break;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_FEATURES_ANDROID:
                safe_pNext = new safe_VkPhysicalDeviceExternalFormatResolveFeaturesANDROID(reinterpret_cast<const VkPhysicalDeviceExternalFormatResolveFeaturesANDROID *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_PROPERTIES_ANDROID:
                safe_pNext = new safe_VkPhysicalDeviceExternalFormatResolvePropertiesANDROID(reinterpret_cast<const VkPhysicalDeviceExternalFormatResolvePropertiesANDROID *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_RESOLVE_PROPERTIES_ANDROID:
                safe_pNext = new safe_VkAndroidHardwareBufferFormatResolvePropertiesANDROID(reinterpret_cast<const VkAndroidHardwareBufferFormatResolvePropertiesANDROID *>(pNext), copy_state, false);
                break;
#endif  // VK_USE_PLATFORM_ANDROID_KHR
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ANTI_LAG_FEATURES_AMD:
                safe_pNext = new safe_VkPhysicalDeviceAntiLagFeaturesAMD(reinterpret_cast<const VkPhysicalDeviceAntiLagFeaturesAMD *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceShaderObjectFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceShaderObjectFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceShaderObjectPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceShaderObjectPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_PROPERTIES_FEATURES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceTilePropertiesFeaturesQCOM(reinterpret_cast<const VkPhysicalDeviceTilePropertiesFeaturesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_AMIGO_PROFILING_FEATURES_SEC:
                safe_pNext = new safe_VkPhysicalDeviceAmigoProfilingFeaturesSEC(reinterpret_cast<const VkPhysicalDeviceAmigoProfilingFeaturesSEC *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_AMIGO_PROFILING_SUBMIT_INFO_SEC:
                safe_pNext = new safe_VkAmigoProfilingSubmitInfoSEC(reinterpret_cast<const VkAmigoProfilingSubmitInfoSEC *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_VIEWPORTS_FEATURES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM(reinterpret_cast<const VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV(reinterpret_cast<const VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV(reinterpret_cast<const VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_VECTOR_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceCooperativeVectorPropertiesNV(reinterpret_cast<const VkPhysicalDeviceCooperativeVectorPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_VECTOR_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceCooperativeVectorFeaturesNV(reinterpret_cast<const VkPhysicalDeviceCooperativeVectorFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_SPARSE_ADDRESS_SPACE_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV(reinterpret_cast<const VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_SPARSE_ADDRESS_SPACE_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV(reinterpret_cast<const VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_VERTEX_ATTRIBUTES_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_VERTEX_ATTRIBUTES_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT:
                safe_pNext = new safe_VkLayerSettingsCreateInfoEXT(reinterpret_cast<const VkLayerSettingsCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_BUILTINS_FEATURES_ARM:
                safe_pNext = new safe_VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM(reinterpret_cast<const VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_BUILTINS_PROPERTIES_ARM:
                safe_pNext = new safe_VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM(reinterpret_cast<const VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_LIBRARY_GROUP_HANDLES_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT(reinterpret_cast<const VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_LATENCY_SUBMISSION_PRESENT_ID_NV:
                safe_pNext = new safe_VkLatencySubmissionPresentIdNV(reinterpret_cast<const VkLatencySubmissionPresentIdNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SWAPCHAIN_LATENCY_CREATE_INFO_NV:
                safe_pNext = new safe_VkSwapchainLatencyCreateInfoNV(reinterpret_cast<const VkSwapchainLatencyCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_LATENCY_SURFACE_CAPABILITIES_NV:
                safe_pNext = new safe_VkLatencySurfaceCapabilitiesNV(reinterpret_cast<const VkLatencySurfaceCapabilitiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DATA_GRAPH_FEATURES_ARM:
                safe_pNext = new safe_VkPhysicalDeviceDataGraphFeaturesARM(reinterpret_cast<const VkPhysicalDeviceDataGraphFeaturesARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_COMPILER_CONTROL_CREATE_INFO_ARM:
                safe_pNext = new safe_VkDataGraphPipelineCompilerControlCreateInfoARM(reinterpret_cast<const VkDataGraphPipelineCompilerControlCreateInfoARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_SHADER_MODULE_CREATE_INFO_ARM:
                safe_pNext = new safe_VkDataGraphPipelineShaderModuleCreateInfoARM(reinterpret_cast<const VkDataGraphPipelineShaderModuleCreateInfoARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_IDENTIFIER_CREATE_INFO_ARM:
                safe_pNext = new safe_VkDataGraphPipelineIdentifierCreateInfoARM(reinterpret_cast<const VkDataGraphPipelineIdentifierCreateInfoARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DATA_GRAPH_PROCESSING_ENGINE_CREATE_INFO_ARM:
                safe_pNext = new safe_VkDataGraphProcessingEngineCreateInfoARM(reinterpret_cast<const VkDataGraphProcessingEngineCreateInfoARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_CONSTANT_TENSOR_SEMI_STRUCTURED_SPARSITY_INFO_ARM:
                safe_pNext = new safe_VkDataGraphPipelineConstantTensorSemiStructuredSparsityInfoARM(reinterpret_cast<const VkDataGraphPipelineConstantTensorSemiStructuredSparsityInfoARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_RENDER_AREAS_FEATURES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM(reinterpret_cast<const VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_MULTIVIEW_PER_VIEW_RENDER_AREAS_RENDER_PASS_BEGIN_INFO_QCOM:
                safe_pNext = new safe_VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM(reinterpret_cast<const VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PER_STAGE_DESCRIPTOR_SET_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDevicePerStageDescriptorSetFeaturesNV(reinterpret_cast<const VkPhysicalDevicePerStageDescriptorSetFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_2_FEATURES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceImageProcessing2FeaturesQCOM(reinterpret_cast<const VkPhysicalDeviceImageProcessing2FeaturesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_2_PROPERTIES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceImageProcessing2PropertiesQCOM(reinterpret_cast<const VkPhysicalDeviceImageProcessing2PropertiesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SAMPLER_BLOCK_MATCH_WINDOW_CREATE_INFO_QCOM:
                safe_pNext = new safe_VkSamplerBlockMatchWindowCreateInfoQCOM(reinterpret_cast<const VkSamplerBlockMatchWindowCreateInfoQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_WEIGHTS_FEATURES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceCubicWeightsFeaturesQCOM(reinterpret_cast<const VkPhysicalDeviceCubicWeightsFeaturesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SAMPLER_CUBIC_WEIGHTS_CREATE_INFO_QCOM:
                safe_pNext = new safe_VkSamplerCubicWeightsCreateInfoQCOM(reinterpret_cast<const VkSamplerCubicWeightsCreateInfoQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_BLIT_IMAGE_CUBIC_WEIGHTS_INFO_QCOM:
                safe_pNext = new safe_VkBlitImageCubicWeightsInfoQCOM(reinterpret_cast<const VkBlitImageCubicWeightsInfoQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_DEGAMMA_FEATURES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceYcbcrDegammaFeaturesQCOM(reinterpret_cast<const VkPhysicalDeviceYcbcrDegammaFeaturesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_YCBCR_DEGAMMA_CREATE_INFO_QCOM:
                safe_pNext = new safe_VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM(reinterpret_cast<const VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_CLAMP_FEATURES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceCubicClampFeaturesQCOM(reinterpret_cast<const VkPhysicalDeviceCubicClampFeaturesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT *>(pNext), copy_state, false);
                break;
#ifdef VK_USE_PLATFORM_SCREEN_QNX
            case VK_STRUCTURE_TYPE_SCREEN_BUFFER_FORMAT_PROPERTIES_QNX:
                safe_pNext = new safe_VkScreenBufferFormatPropertiesQNX(reinterpret_cast<const VkScreenBufferFormatPropertiesQNX *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMPORT_SCREEN_BUFFER_INFO_QNX:
                safe_pNext = new safe_VkImportScreenBufferInfoQNX(reinterpret_cast<const VkImportScreenBufferInfoQNX *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_QNX:
                safe_pNext = new safe_VkExternalFormatQNX(reinterpret_cast<const VkExternalFormatQNX *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_SCREEN_BUFFER_FEATURES_QNX:
                safe_pNext = new safe_VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX(reinterpret_cast<const VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX *>(pNext), copy_state, false);
                break;
#endif  // VK_USE_PLATFORM_SCREEN_QNX
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_DRIVER_PROPERTIES_MSFT:
                safe_pNext = new safe_VkPhysicalDeviceLayeredDriverPropertiesMSFT(reinterpret_cast<const VkPhysicalDeviceLayeredDriverPropertiesMSFT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_POOL_OVERALLOCATION_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV(reinterpret_cast<const VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_MEMORY_HEAP_FEATURES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceTileMemoryHeapFeaturesQCOM(reinterpret_cast<const VkPhysicalDeviceTileMemoryHeapFeaturesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_MEMORY_HEAP_PROPERTIES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceTileMemoryHeapPropertiesQCOM(reinterpret_cast<const VkPhysicalDeviceTileMemoryHeapPropertiesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_TILE_MEMORY_REQUIREMENTS_QCOM:
                safe_pNext = new safe_VkTileMemoryRequirementsQCOM(reinterpret_cast<const VkTileMemoryRequirementsQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_TILE_MEMORY_BIND_INFO_QCOM:
                safe_pNext = new safe_VkTileMemoryBindInfoQCOM(reinterpret_cast<const VkTileMemoryBindInfoQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_TILE_MEMORY_SIZE_INFO_QCOM:
                safe_pNext = new safe_VkTileMemorySizeInfoQCOM(reinterpret_cast<const VkTileMemorySizeInfoQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DISPLAY_SURFACE_STEREO_CREATE_INFO_NV:
                safe_pNext = new safe_VkDisplaySurfaceStereoCreateInfoNV(reinterpret_cast<const VkDisplaySurfaceStereoCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DISPLAY_MODE_STEREO_PROPERTIES_NV:
                safe_pNext = new safe_VkDisplayModeStereoPropertiesNV(reinterpret_cast<const VkDisplayModeStereoPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAW_ACCESS_CHAINS_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceRawAccessChainsFeaturesNV(reinterpret_cast<const VkPhysicalDeviceRawAccessChainsFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXTERNAL_COMPUTE_QUEUE_DEVICE_CREATE_INFO_NV:
                safe_pNext = new safe_VkExternalComputeQueueDeviceCreateInfoNV(reinterpret_cast<const VkExternalComputeQueueDeviceCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_COMPUTE_QUEUE_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceExternalComputeQueuePropertiesNV(reinterpret_cast<const VkPhysicalDeviceExternalComputeQueuePropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMMAND_BUFFER_INHERITANCE_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceCommandBufferInheritanceFeaturesNV(reinterpret_cast<const VkPhysicalDeviceCommandBufferInheritanceFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT16_VECTOR_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV(reinterpret_cast<const VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_REPLICATED_COMPOSITES_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT8_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceShaderFloat8FeaturesEXT(reinterpret_cast<const VkPhysicalDeviceShaderFloat8FeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_VALIDATION_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceRayTracingValidationFeaturesNV(reinterpret_cast<const VkPhysicalDeviceRayTracingValidationFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_ACCELERATION_STRUCTURE_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceClusterAccelerationStructureFeaturesNV(reinterpret_cast<const VkPhysicalDeviceClusterAccelerationStructureFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_ACCELERATION_STRUCTURE_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceClusterAccelerationStructurePropertiesNV(reinterpret_cast<const VkPhysicalDeviceClusterAccelerationStructurePropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CLUSTER_ACCELERATION_STRUCTURE_CREATE_INFO_NV:
                safe_pNext = new safe_VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV(reinterpret_cast<const VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PARTITIONED_ACCELERATION_STRUCTURE_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV(reinterpret_cast<const VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PARTITIONED_ACCELERATION_STRUCTURE_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV(reinterpret_cast<const VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_FLAGS_NV:
                safe_pNext = new safe_VkPartitionedAccelerationStructureFlagsNV(reinterpret_cast<const VkPartitionedAccelerationStructureFlagsNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_PARTITIONED_ACCELERATION_STRUCTURE_NV:
                safe_pNext = new safe_VkWriteDescriptorSetPartitionedAccelerationStructureNV(reinterpret_cast<const VkWriteDescriptorSetPartitionedAccelerationStructureNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_GENERATED_COMMANDS_PIPELINE_INFO_EXT:
                safe_pNext = new safe_VkGeneratedCommandsPipelineInfoEXT(reinterpret_cast<const VkGeneratedCommandsPipelineInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_GENERATED_COMMANDS_SHADER_INFO_EXT:
                safe_pNext = new safe_VkGeneratedCommandsShaderInfoEXT(reinterpret_cast<const VkGeneratedCommandsShaderInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ALIGNMENT_CONTROL_FEATURES_MESA:
                safe_pNext = new safe_VkPhysicalDeviceImageAlignmentControlFeaturesMESA(reinterpret_cast<const VkPhysicalDeviceImageAlignmentControlFeaturesMESA *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ALIGNMENT_CONTROL_PROPERTIES_MESA:
                safe_pNext = new safe_VkPhysicalDeviceImageAlignmentControlPropertiesMESA(reinterpret_cast<const VkPhysicalDeviceImageAlignmentControlPropertiesMESA *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_ALIGNMENT_CONTROL_CREATE_INFO_MESA:
                safe_pNext = new safe_VkImageAlignmentControlCreateInfoMESA(reinterpret_cast<const VkImageAlignmentControlCreateInfoMESA *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_CONTROL_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceDepthClampControlFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceDepthClampControlFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLAMP_CONTROL_CREATE_INFO_EXT:
                safe_pNext = new safe_VkPipelineViewportDepthClampControlCreateInfoEXT(reinterpret_cast<const VkPipelineViewportDepthClampControlCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HDR_VIVID_FEATURES_HUAWEI:
                safe_pNext = new safe_VkPhysicalDeviceHdrVividFeaturesHUAWEI(reinterpret_cast<const VkPhysicalDeviceHdrVividFeaturesHUAWEI *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_HDR_VIVID_DYNAMIC_METADATA_HUAWEI:
                safe_pNext = new safe_VkHdrVividDynamicMetadataHUAWEI(reinterpret_cast<const VkHdrVividDynamicMetadataHUAWEI *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_2_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceCooperativeMatrix2FeaturesNV(reinterpret_cast<const VkPhysicalDeviceCooperativeMatrix2FeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_2_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceCooperativeMatrix2PropertiesNV(reinterpret_cast<const VkPhysicalDeviceCooperativeMatrix2PropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_OPACITY_MICROMAP_FEATURES_ARM:
                safe_pNext = new safe_VkPhysicalDevicePipelineOpacityMicromapFeaturesARM(reinterpret_cast<const VkPhysicalDevicePipelineOpacityMicromapFeaturesARM *>(pNext), copy_state, false);
                break;
#ifdef VK_USE_PLATFORM_METAL_EXT
            case VK_STRUCTURE_TYPE_IMPORT_MEMORY_METAL_HANDLE_INFO_EXT:
                safe_pNext = new safe_VkImportMemoryMetalHandleInfoEXT(reinterpret_cast<const VkImportMemoryMetalHandleInfoEXT *>(pNext), copy_state, false);
                break;
#endif  // VK_USE_PLATFORM_METAL_EXT
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_ROBUSTNESS_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FORMAT_PACK_FEATURES_ARM:
                safe_pNext = new safe_VkPhysicalDeviceFormatPackFeaturesARM(reinterpret_cast<const VkPhysicalDeviceFormatPackFeaturesARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_LAYERED_FEATURES_VALVE:
                safe_pNext = new safe_VkPhysicalDeviceFragmentDensityMapLayeredFeaturesVALVE(reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapLayeredFeaturesVALVE *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_LAYERED_PROPERTIES_VALVE:
                safe_pNext = new safe_VkPhysicalDeviceFragmentDensityMapLayeredPropertiesVALVE(reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapLayeredPropertiesVALVE *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_DENSITY_MAP_LAYERED_CREATE_INFO_VALVE:
                safe_pNext = new safe_VkPipelineFragmentDensityMapLayeredCreateInfoVALVE(reinterpret_cast<const VkPipelineFragmentDensityMapLayeredCreateInfoVALVE *>(pNext), copy_state, false);
                break;
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_SET_PRESENT_CONFIG_NV:
                safe_pNext = new safe_VkSetPresentConfigNV(reinterpret_cast<const VkSetPresentConfigNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_METERING_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDevicePresentMeteringFeaturesNV(reinterpret_cast<const VkPhysicalDevicePresentMeteringFeaturesNV *>(pNext), copy_state, false);
                break;
#endif  // VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_DEVICE_MEMORY_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CACHE_INCREMENTAL_MODE_FEATURES_SEC:
                safe_pNext = new safe_VkPhysicalDevicePipelineCacheIncrementalModeFeaturesSEC(reinterpret_cast<const VkPhysicalDevicePipelineCacheIncrementalModeFeaturesSEC *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR:
                safe_pNext = new safe_VkWriteDescriptorSetAccelerationStructureKHR(reinterpret_cast<const VkWriteDescriptorSetAccelerationStructureKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceAccelerationStructureFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR(reinterpret_cast<const VkPhysicalDeviceAccelerationStructurePropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR:
                safe_pNext = new safe_VkRayTracingPipelineCreateInfoKHR(reinterpret_cast<const VkRayTracingPipelineCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceRayTracingPipelineFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR(reinterpret_cast<const VkPhysicalDeviceRayTracingPipelinePropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceRayQueryFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceRayQueryFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceMeshShaderFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceMeshShaderFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceMeshShaderPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceMeshShaderPropertiesEXT *>(pNext), copy_state, false);
                break;

            default: // Encountered an unknown sType -- skip (do not copy) this entry in the chain
                // If sType is in custom list, construct blind copy
                for (auto item : GetCustomStypeInfo()) {
                    if (item.first == static_cast<uint32_t>(header->sType)) {
                        safe_pNext = malloc(item.second);
                        memcpy(safe_pNext, header, item.second);
                    }
                }
                break;
        }
        if (!first_pNext) {
            first_pNext = safe_pNext;
        }
        pNext = header->pNext;
        if (prev_pNext && safe_pNext) {
            prev_pNext->pNext = (VkBaseOutStructure*)safe_pNext;
        }
        if (safe_pNext) {
            prev_pNext = (VkBaseOutStructure*)safe_pNext;
        }
        safe_pNext = nullptr;
    }

    return first_pNext;
}

void FreePnextChain(const void *pNext) {
    // The pNext parameter is const for convenience, since it is called by code
    // for many structures where the pNext field is const.
    void *current = const_cast<void*>(pNext);
    while (current) {
        auto header = reinterpret_cast<VkBaseOutStructure *>(current);
        void *next = header->pNext;
        // prevent destructors from recursing behind our backs.
        header->pNext = nullptr;

        switch (header->sType) {
            // Special-case Loader Instance Struct passed to/from layer in pNext chain
        case VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO:
            delete reinterpret_cast<VkLayerInstanceCreateInfo *>(current);
            break;
        // Special-case Loader Device Struct passed to/from layer in pNext chain
        case VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO:
            delete reinterpret_cast<VkLayerDeviceCreateInfo *>(current);
            break;
        case VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO:
            delete reinterpret_cast<safe_VkShaderModuleCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO:
            delete reinterpret_cast<safe_VkComputePipelineCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO:
            delete reinterpret_cast<safe_VkGraphicsPipelineCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO:
            delete reinterpret_cast<safe_VkPipelineLayoutCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceSubgroupProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDevice16BitStorageFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS:
            delete reinterpret_cast<safe_VkMemoryDedicatedRequirements *>(header);
            break;
        case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO:
            delete reinterpret_cast<safe_VkMemoryDedicatedAllocateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO:
            delete reinterpret_cast<safe_VkMemoryAllocateFlagsInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO:
            delete reinterpret_cast<safe_VkDeviceGroupRenderPassBeginInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO:
            delete reinterpret_cast<safe_VkDeviceGroupCommandBufferBeginInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO:
            delete reinterpret_cast<safe_VkDeviceGroupSubmitInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_BIND_SPARSE_INFO:
            delete reinterpret_cast<safe_VkDeviceGroupBindSparseInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO:
            delete reinterpret_cast<safe_VkBindBufferMemoryDeviceGroupInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_DEVICE_GROUP_INFO:
            delete reinterpret_cast<safe_VkBindImageMemoryDeviceGroupInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO:
            delete reinterpret_cast<safe_VkDeviceGroupDeviceCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2:
            delete reinterpret_cast<safe_VkPhysicalDeviceFeatures2 *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDevicePointClippingProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO:
            delete reinterpret_cast<safe_VkRenderPassInputAttachmentAspectCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO:
            delete reinterpret_cast<safe_VkImageViewUsageCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO:
            delete reinterpret_cast<safe_VkPipelineTessellationDomainOriginStateCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO:
            delete reinterpret_cast<safe_VkRenderPassMultiviewCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceMultiviewFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceMultiviewProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceVariablePointersFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceProtectedMemoryFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceProtectedMemoryProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO:
            delete reinterpret_cast<safe_VkProtectedSubmitInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO:
            delete reinterpret_cast<safe_VkSamplerYcbcrConversionInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO:
            delete reinterpret_cast<safe_VkBindImagePlaneMemoryInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO:
            delete reinterpret_cast<safe_VkImagePlaneMemoryRequirementsInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_IMAGE_FORMAT_PROPERTIES:
            delete reinterpret_cast<safe_VkSamplerYcbcrConversionImageFormatProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO:
            delete reinterpret_cast<safe_VkPhysicalDeviceExternalImageFormatInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES:
            delete reinterpret_cast<safe_VkExternalImageFormatProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceIDProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO:
            delete reinterpret_cast<safe_VkExternalMemoryImageCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO:
            delete reinterpret_cast<safe_VkExternalMemoryBufferCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO:
            delete reinterpret_cast<safe_VkExportMemoryAllocateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO:
            delete reinterpret_cast<safe_VkExportFenceCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO:
            delete reinterpret_cast<safe_VkExportSemaphoreCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceMaintenance3Properties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderDrawParametersFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceVulkan11Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceVulkan11Properties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceVulkan12Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceVulkan12Properties *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO:
            delete reinterpret_cast<safe_VkImageFormatListCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDevice8BitStorageFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceDriverProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderAtomicInt64Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderFloat16Int8Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceFloatControlsProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO:
            delete reinterpret_cast<safe_VkDescriptorSetLayoutBindingFlagsCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceDescriptorIndexingFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceDescriptorIndexingProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO:
            delete reinterpret_cast<safe_VkDescriptorSetVariableDescriptorCountAllocateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_LAYOUT_SUPPORT:
            delete reinterpret_cast<safe_VkDescriptorSetVariableDescriptorCountLayoutSupport *>(header);
            break;
        case VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE:
            delete reinterpret_cast<safe_VkSubpassDescriptionDepthStencilResolve *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceDepthStencilResolveProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceScalarBlockLayoutFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO:
            delete reinterpret_cast<safe_VkImageStencilUsageCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO:
            delete reinterpret_cast<safe_VkSamplerReductionModeCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceSamplerFilterMinmaxProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceVulkanMemoryModelFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceImagelessFramebufferFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO:
            delete reinterpret_cast<safe_VkFramebufferAttachmentsCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO:
            delete reinterpret_cast<safe_VkRenderPassAttachmentBeginInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceUniformBufferStandardLayoutFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_STENCIL_LAYOUT:
            delete reinterpret_cast<safe_VkAttachmentReferenceStencilLayout *>(header);
            break;
        case VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_STENCIL_LAYOUT:
            delete reinterpret_cast<safe_VkAttachmentDescriptionStencilLayout *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceHostQueryResetFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceTimelineSemaphoreFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceTimelineSemaphoreProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO:
            delete reinterpret_cast<safe_VkSemaphoreTypeCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO:
            delete reinterpret_cast<safe_VkTimelineSemaphoreSubmitInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceBufferDeviceAddressFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO:
            delete reinterpret_cast<safe_VkBufferOpaqueCaptureAddressCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO:
            delete reinterpret_cast<safe_VkMemoryOpaqueCaptureAddressAllocateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceVulkan13Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceVulkan13Properties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_CREATION_FEEDBACK_CREATE_INFO:
            delete reinterpret_cast<safe_VkPipelineCreationFeedbackCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderTerminateInvocationFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDevicePrivateDataFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_PRIVATE_DATA_CREATE_INFO:
            delete reinterpret_cast<safe_VkDevicePrivateDataCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDevicePipelineCreationCacheControlFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_MEMORY_BARRIER_2:
            delete reinterpret_cast<safe_VkMemoryBarrier2 *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceSynchronization2Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceImageRobustnessFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceSubgroupSizeControlFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceSubgroupSizeControlProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO:
            delete reinterpret_cast<safe_VkPipelineShaderStageRequiredSubgroupSizeCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceInlineUniformBlockFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceInlineUniformBlockProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK:
            delete reinterpret_cast<safe_VkWriteDescriptorSetInlineUniformBlock *>(header);
            break;
        case VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_INLINE_UNIFORM_BLOCK_CREATE_INFO:
            delete reinterpret_cast<safe_VkDescriptorPoolInlineUniformBlockCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceTextureCompressionASTCHDRFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO:
            delete reinterpret_cast<safe_VkPipelineRenderingCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceDynamicRenderingFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO:
            delete reinterpret_cast<safe_VkCommandBufferInheritanceRenderingInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderIntegerDotProductFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderIntegerDotProductProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceTexelBufferAlignmentProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_3:
            delete reinterpret_cast<safe_VkFormatProperties3 *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceMaintenance4Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceMaintenance4Properties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceVulkan14Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceVulkan14Properties *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO:
            delete reinterpret_cast<safe_VkDeviceQueueGlobalPriorityCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GLOBAL_PRIORITY_QUERY_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceGlobalPriorityQueryFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUEUE_FAMILY_GLOBAL_PRIORITY_PROPERTIES:
            delete reinterpret_cast<safe_VkQueueFamilyGlobalPriorityProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_ROTATE_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderSubgroupRotateFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT_CONTROLS_2_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderFloatControls2Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EXPECT_ASSUME_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderExpectAssumeFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceLineRasterizationFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceLineRasterizationProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO:
            delete reinterpret_cast<safe_VkPipelineRasterizationLineStateCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceVertexAttributeDivisorProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO:
            delete reinterpret_cast<safe_VkPipelineVertexInputDivisorStateCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceVertexAttributeDivisorFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceIndexTypeUint8Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceMaintenance5Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceMaintenance5Properties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_CREATE_FLAGS_2_CREATE_INFO:
            delete reinterpret_cast<safe_VkPipelineCreateFlags2CreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO:
            delete reinterpret_cast<safe_VkBufferUsageFlags2CreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDevicePushDescriptorProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceDynamicRenderingLocalReadFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_LOCATION_INFO:
            delete reinterpret_cast<safe_VkRenderingAttachmentLocationInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDERING_INPUT_ATTACHMENT_INDEX_INFO:
            delete reinterpret_cast<safe_VkRenderingInputAttachmentIndexInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceMaintenance6Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceMaintenance6Properties *>(header);
            break;
        case VK_STRUCTURE_TYPE_BIND_MEMORY_STATUS:
            delete reinterpret_cast<safe_VkBindMemoryStatus *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROTECTED_ACCESS_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDevicePipelineProtectedAccessFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDevicePipelineRobustnessFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDevicePipelineRobustnessProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_ROBUSTNESS_CREATE_INFO:
            delete reinterpret_cast<safe_VkPipelineRobustnessCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES:
            delete reinterpret_cast<safe_VkPhysicalDeviceHostImageCopyFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_PROPERTIES:
            delete reinterpret_cast<safe_VkPhysicalDeviceHostImageCopyProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_SUBRESOURCE_HOST_MEMCPY_SIZE:
            delete reinterpret_cast<safe_VkSubresourceHostMemcpySize *>(header);
            break;
        case VK_STRUCTURE_TYPE_HOST_IMAGE_COPY_DEVICE_PERFORMANCE_QUERY:
            delete reinterpret_cast<safe_VkHostImageCopyDevicePerformanceQuery *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_SWAPCHAIN_CREATE_INFO_KHR:
            delete reinterpret_cast<safe_VkImageSwapchainCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR:
            delete reinterpret_cast<safe_VkBindImageMemorySwapchainInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_INFO_KHR:
            delete reinterpret_cast<safe_VkDeviceGroupPresentInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_SWAPCHAIN_CREATE_INFO_KHR:
            delete reinterpret_cast<safe_VkDeviceGroupSwapchainCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR:
            delete reinterpret_cast<safe_VkDisplayPresentInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUEUE_FAMILY_QUERY_RESULT_STATUS_PROPERTIES_KHR:
            delete reinterpret_cast<safe_VkQueueFamilyQueryResultStatusPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUEUE_FAMILY_VIDEO_PROPERTIES_KHR:
            delete reinterpret_cast<safe_VkQueueFamilyVideoPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoProfileInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_PROFILE_LIST_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoProfileListInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_CAPABILITIES_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeCapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_USAGE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeUsageInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_CAPABILITIES_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH264CapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_QUALITY_LEVEL_PROPERTIES_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH264QualityLevelPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_CREATE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH264SessionCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_ADD_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH264SessionParametersAddInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_CREATE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH264SessionParametersCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_GET_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH264SessionParametersGetInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_FEEDBACK_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH264SessionParametersFeedbackInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PICTURE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH264PictureInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_DPB_SLOT_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH264DpbSlotInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PROFILE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH264ProfileInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_RATE_CONTROL_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH264RateControlInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_RATE_CONTROL_LAYER_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH264RateControlLayerInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_GOP_REMAINING_FRAME_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH264GopRemainingFrameInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_CAPABILITIES_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH265CapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_CREATE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH265SessionCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_QUALITY_LEVEL_PROPERTIES_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH265QualityLevelPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_ADD_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH265SessionParametersAddInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_CREATE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH265SessionParametersCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_GET_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH265SessionParametersGetInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_FEEDBACK_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH265SessionParametersFeedbackInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PICTURE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH265PictureInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_DPB_SLOT_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH265DpbSlotInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PROFILE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH265ProfileInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_RATE_CONTROL_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH265RateControlInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_RATE_CONTROL_LAYER_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH265RateControlLayerInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_GOP_REMAINING_FRAME_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH265GopRemainingFrameInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PROFILE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeH264ProfileInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_CAPABILITIES_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeH264CapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_ADD_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeH264SessionParametersAddInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_CREATE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeH264SessionParametersCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PICTURE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeH264PictureInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_DPB_SLOT_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeH264DpbSlotInfoKHR *>(header);
            break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR:
            delete reinterpret_cast<safe_VkImportMemoryWin32HandleInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR:
            delete reinterpret_cast<safe_VkExportMemoryWin32HandleInfoKHR *>(header);
            break;
#endif  // VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR:
            delete reinterpret_cast<safe_VkImportMemoryFdInfoKHR *>(header);
            break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_KHR:
            delete reinterpret_cast<safe_VkWin32KeyedMutexAcquireReleaseInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR:
            delete reinterpret_cast<safe_VkExportSemaphoreWin32HandleInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_D3D12_FENCE_SUBMIT_INFO_KHR:
            delete reinterpret_cast<safe_VkD3D12FenceSubmitInfoKHR *>(header);
            break;
#endif  // VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR:
            delete reinterpret_cast<safe_VkPresentRegionsKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR:
            delete reinterpret_cast<safe_VkSharedPresentSurfaceCapabilitiesKHR *>(header);
            break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_EXPORT_FENCE_WIN32_HANDLE_INFO_KHR:
            delete reinterpret_cast<safe_VkExportFenceWin32HandleInfoKHR *>(header);
            break;
#endif  // VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDevicePerformanceQueryFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_PROPERTIES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDevicePerformanceQueryPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR:
            delete reinterpret_cast<safe_VkQueryPoolPerformanceCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_SUBMIT_INFO_KHR:
            delete reinterpret_cast<safe_VkPerformanceQuerySubmitInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_BFLOAT16_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderBfloat16FeaturesKHR *>(header);
            break;
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_PROPERTIES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR *>(header);
            break;
#endif  // VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderClockFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PROFILE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeH265ProfileInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_CAPABILITIES_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeH265CapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_PARAMETERS_ADD_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeH265SessionParametersAddInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_PARAMETERS_CREATE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeH265SessionParametersCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PICTURE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeH265PictureInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_DPB_SLOT_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeH265DpbSlotInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR:
            delete reinterpret_cast<safe_VkFragmentShadingRateAttachmentInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_STATE_CREATE_INFO_KHR:
            delete reinterpret_cast<safe_VkPipelineFragmentShadingRateStateCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR:
            delete reinterpret_cast<safe_VkRenderingFragmentShadingRateAttachmentInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_QUAD_CONTROL_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderQuadControlFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_SURFACE_PROTECTED_CAPABILITIES_KHR:
            delete reinterpret_cast<safe_VkSurfaceProtectedCapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDevicePresentWaitFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR:
            delete reinterpret_cast<safe_VkPipelineLibraryCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PRESENT_ID_KHR:
            delete reinterpret_cast<safe_VkPresentIdKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDevicePresentIdFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_CAPABILITIES_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeCapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUERY_POOL_VIDEO_ENCODE_FEEDBACK_CREATE_INFO_KHR:
            delete reinterpret_cast<safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_USAGE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeUsageInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_RATE_CONTROL_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeRateControlInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUALITY_LEVEL_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeQualityLevelInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_PROPERTIES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MAXIMAL_RECONVERGENCE_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_PRESENT_ID_2_KHR:
            delete reinterpret_cast<safe_VkSurfaceCapabilitiesPresentId2KHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PRESENT_ID_2_KHR:
            delete reinterpret_cast<safe_VkPresentId2KHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_2_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDevicePresentId2FeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_PRESENT_WAIT_2_KHR:
            delete reinterpret_cast<safe_VkSurfaceCapabilitiesPresentWait2KHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_2_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDevicePresentWait2FeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_BINARY_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDevicePipelineBinaryFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_BINARY_PROPERTIES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDevicePipelineBinaryPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_PIPELINE_BINARY_INTERNAL_CACHE_CONTROL_KHR:
            delete reinterpret_cast<safe_VkDevicePipelineBinaryInternalCacheControlKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_BINARY_INFO_KHR:
            delete reinterpret_cast<safe_VkPipelineBinaryInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_KHR:
            delete reinterpret_cast<safe_VkSurfacePresentModeKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_SURFACE_PRESENT_SCALING_CAPABILITIES_KHR:
            delete reinterpret_cast<safe_VkSurfacePresentScalingCapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_COMPATIBILITY_KHR:
            delete reinterpret_cast<safe_VkSurfacePresentModeCompatibilityKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_KHR:
            delete reinterpret_cast<safe_VkSwapchainPresentFenceInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_MODES_CREATE_INFO_KHR:
            delete reinterpret_cast<safe_VkSwapchainPresentModesCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_MODE_INFO_KHR:
            delete reinterpret_cast<safe_VkSwapchainPresentModeInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_SCALING_CREATE_INFO_KHR:
            delete reinterpret_cast<safe_VkSwapchainPresentScalingCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_PROPERTIES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_PROFILE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeAV1ProfileInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_CAPABILITIES_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeAV1CapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_SESSION_PARAMETERS_CREATE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeAV1SessionParametersCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_PICTURE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeAV1PictureInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_DPB_SLOT_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeAV1DpbSlotInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_AV1_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceVideoEncodeAV1FeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_CAPABILITIES_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeAV1CapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_QUALITY_LEVEL_PROPERTIES_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeAV1QualityLevelPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_SESSION_CREATE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeAV1SessionCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_SESSION_PARAMETERS_CREATE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeAV1SessionParametersCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_PICTURE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeAV1PictureInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_DPB_SLOT_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeAV1DpbSlotInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_PROFILE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeAV1ProfileInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_GOP_REMAINING_FRAME_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeAV1GopRemainingFrameInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_RATE_CONTROL_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeAV1RateControlInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_RATE_CONTROL_LAYER_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeAV1RateControlLayerInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_DECODE_VP9_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceVideoDecodeVP9FeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_VP9_PROFILE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeVP9ProfileInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_VP9_CAPABILITIES_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeVP9CapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_VP9_PICTURE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeVP9PictureInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_1_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceVideoMaintenance1FeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_INLINE_QUERY_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoInlineQueryInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFIED_IMAGE_LAYOUTS_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_ATTACHMENT_FEEDBACK_LOOP_INFO_EXT:
            delete reinterpret_cast<safe_VkAttachmentFeedbackLoopInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_INTRA_REFRESH_CAPABILITIES_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeIntraRefreshCapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_SESSION_INTRA_REFRESH_CREATE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeSessionIntraRefreshCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_INTRA_REFRESH_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeIntraRefreshInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_REFERENCE_INTRA_REFRESH_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoReferenceIntraRefreshInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_INTRA_REFRESH_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_CAPABILITIES_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeQuantizationMapCapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_FORMAT_QUANTIZATION_MAP_PROPERTIES_KHR:
            delete reinterpret_cast<safe_VkVideoFormatQuantizationMapPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeQuantizationMapInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_SESSION_PARAMETERS_CREATE_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_QUANTIZATION_MAP_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_QUANTIZATION_MAP_CAPABILITIES_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH264QuantizationMapCapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_QUANTIZATION_MAP_CAPABILITIES_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeH265QuantizationMapCapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_FORMAT_H265_QUANTIZATION_MAP_PROPERTIES_KHR:
            delete reinterpret_cast<safe_VkVideoFormatH265QuantizationMapPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_QUANTIZATION_MAP_CAPABILITIES_KHR:
            delete reinterpret_cast<safe_VkVideoEncodeAV1QuantizationMapCapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_FORMAT_AV1_QUANTIZATION_MAP_PROPERTIES_KHR:
            delete reinterpret_cast<safe_VkVideoFormatAV1QuantizationMapPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_RELAXED_EXTENDED_INSTRUCTION_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceMaintenance7FeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_PROPERTIES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceMaintenance7PropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_API_PROPERTIES_LIST_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceLayeredApiPropertiesListKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_API_VULKAN_PROPERTIES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceLayeredApiVulkanPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_8_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceMaintenance8FeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_MEMORY_BARRIER_ACCESS_FLAGS_3_KHR:
            delete reinterpret_cast<safe_VkMemoryBarrierAccessFlags3KHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_9_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceMaintenance9FeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_9_PROPERTIES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceMaintenance9PropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUEUE_FAMILY_OWNERSHIP_TRANSFER_PROPERTIES_KHR:
            delete reinterpret_cast<safe_VkQueueFamilyOwnershipTransferPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_2_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceVideoMaintenance2FeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_INLINE_SESSION_PARAMETERS_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeH264InlineSessionParametersInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_INLINE_SESSION_PARAMETERS_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeH265InlineSessionParametersInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_INLINE_SESSION_PARAMETERS_INFO_KHR:
            delete reinterpret_cast<safe_VkVideoDecodeAV1InlineSessionParametersInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_ZERO_ONE_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceDepthClampZeroOneFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceRobustness2FeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_PROPERTIES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceRobustness2PropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_MODE_FIFO_LATEST_READY_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkDebugReportCallbackCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_RASTERIZATION_ORDER_AMD:
            delete reinterpret_cast<safe_VkPipelineRasterizationStateRasterizationOrderAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_IMAGE_CREATE_INFO_NV:
            delete reinterpret_cast<safe_VkDedicatedAllocationImageCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_BUFFER_CREATE_INFO_NV:
            delete reinterpret_cast<safe_VkDedicatedAllocationBufferCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_MEMORY_ALLOCATE_INFO_NV:
            delete reinterpret_cast<safe_VkDedicatedAllocationMemoryAllocateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_STREAM_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkPipelineRasterizationStateStreamCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_CU_MODULE_TEXTURING_MODE_CREATE_INFO_NVX:
            delete reinterpret_cast<safe_VkCuModuleTexturingModeCreateInfoNVX *>(header);
            break;
        case VK_STRUCTURE_TYPE_TEXTURE_LOD_GATHER_FORMAT_PROPERTIES_AMD:
            delete reinterpret_cast<safe_VkTextureLODGatherFormatPropertiesAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CORNER_SAMPLED_IMAGE_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceCornerSampledImageFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_NV:
            delete reinterpret_cast<safe_VkExternalMemoryImageCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_NV:
            delete reinterpret_cast<safe_VkExportMemoryAllocateInfoNV *>(header);
            break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_NV:
            delete reinterpret_cast<safe_VkImportMemoryWin32HandleInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_NV:
            delete reinterpret_cast<safe_VkExportMemoryWin32HandleInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_NV:
            delete reinterpret_cast<safe_VkWin32KeyedMutexAcquireReleaseInfoNV *>(header);
            break;
#endif  // VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT:
            delete reinterpret_cast<safe_VkValidationFlagsEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_VIEW_ASTC_DECODE_MODE_EXT:
            delete reinterpret_cast<safe_VkImageViewASTCDecodeModeEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ASTC_DECODE_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceASTCDecodeFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT:
            delete reinterpret_cast<safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_W_SCALING_STATE_CREATE_INFO_NV:
            delete reinterpret_cast<safe_VkPipelineViewportWScalingStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_SWAPCHAIN_COUNTER_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkSwapchainCounterCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PRESENT_TIMES_INFO_GOOGLE:
            delete reinterpret_cast<safe_VkPresentTimesInfoGOOGLE *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_ATTRIBUTES_PROPERTIES_NVX:
            delete reinterpret_cast<safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX *>(header);
            break;
        case VK_STRUCTURE_TYPE_MULTIVIEW_PER_VIEW_ATTRIBUTES_INFO_NVX:
            delete reinterpret_cast<safe_VkMultiviewPerViewAttributesInfoNVX *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SWIZZLE_STATE_CREATE_INFO_NV:
            delete reinterpret_cast<safe_VkPipelineViewportSwizzleStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISCARD_RECTANGLE_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_DISCARD_RECTANGLE_STATE_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkPipelineDiscardRectangleStateCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkPipelineRasterizationConservativeStateCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RELAXED_LINE_RASTERIZATION_FEATURES_IMG:
            delete reinterpret_cast<safe_VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT:
            delete reinterpret_cast<safe_VkDebugUtilsObjectNameInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkDebugUtilsMessengerCreateInfoEXT *>(header);
            break;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        case VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_USAGE_ANDROID:
            delete reinterpret_cast<safe_VkAndroidHardwareBufferUsageANDROID *>(header);
            break;
        case VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID:
            delete reinterpret_cast<safe_VkAndroidHardwareBufferFormatPropertiesANDROID *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID:
            delete reinterpret_cast<safe_VkImportAndroidHardwareBufferInfoANDROID *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID:
            delete reinterpret_cast<safe_VkExternalFormatANDROID *>(header);
            break;
        case VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_2_ANDROID:
            delete reinterpret_cast<safe_VkAndroidHardwareBufferFormatProperties2ANDROID *>(header);
            break;
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ENQUEUE_FEATURES_AMDX:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderEnqueueFeaturesAMDX *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ENQUEUE_PROPERTIES_AMDX:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderEnqueuePropertiesAMDX *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXECUTION_GRAPH_PIPELINE_CREATE_INFO_AMDX:
            delete reinterpret_cast<safe_VkExecutionGraphPipelineCreateInfoAMDX *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_NODE_CREATE_INFO_AMDX:
            delete reinterpret_cast<safe_VkPipelineShaderStageNodeCreateInfoAMDX *>(header);
            break;
#endif  // VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_ATTACHMENT_SAMPLE_COUNT_INFO_AMD:
            delete reinterpret_cast<safe_VkAttachmentSampleCountInfoAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT:
            delete reinterpret_cast<safe_VkSampleLocationsInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_SAMPLE_LOCATIONS_BEGIN_INFO_EXT:
            delete reinterpret_cast<safe_VkRenderPassSampleLocationsBeginInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkPipelineSampleLocationsStateCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceSampleLocationsPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_TO_COLOR_STATE_CREATE_INFO_NV:
            delete reinterpret_cast<safe_VkPipelineCoverageToColorStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_MODULATION_STATE_CREATE_INFO_NV:
            delete reinterpret_cast<safe_VkPipelineCoverageModulationStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_PROPERTIES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT:
            delete reinterpret_cast<safe_VkDrmFormatModifierPropertiesListEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkImageDrmFormatModifierListCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkImageDrmFormatModifierExplicitCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_2_EXT:
            delete reinterpret_cast<safe_VkDrmFormatModifierPropertiesList2EXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SHADER_MODULE_VALIDATION_CACHE_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkShaderModuleValidationCacheCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV:
            delete reinterpret_cast<safe_VkPipelineViewportShadingRateImageStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceShadingRateImageFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_PROPERTIES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceShadingRateImagePropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_COARSE_SAMPLE_ORDER_STATE_CREATE_INFO_NV:
            delete reinterpret_cast<safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV:
            delete reinterpret_cast<safe_VkWriteDescriptorSetAccelerationStructureNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceRayTracingPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_REPRESENTATIVE_FRAGMENT_TEST_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_REPRESENTATIVE_FRAGMENT_TEST_STATE_CREATE_INFO_NV:
            delete reinterpret_cast<safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_IMAGE_FORMAT_INFO_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceImageViewImageFormatInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_FILTER_CUBIC_IMAGE_VIEW_IMAGE_FORMAT_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkFilterCubicImageViewImageFormatPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT:
            delete reinterpret_cast<safe_VkImportMemoryHostPointerInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_COMPILER_CONTROL_CREATE_INFO_AMD:
            delete reinterpret_cast<safe_VkPipelineCompilerControlCreateInfoAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_AMD:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderCorePropertiesAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_MEMORY_OVERALLOCATION_CREATE_INFO_AMD:
            delete reinterpret_cast<safe_VkDeviceMemoryOverallocationCreateInfoAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT *>(header);
            break;
#ifdef VK_USE_PLATFORM_GGP
        case VK_STRUCTURE_TYPE_PRESENT_FRAME_TOKEN_GGP:
            delete reinterpret_cast<safe_VkPresentFrameTokenGGP *>(header);
            break;
#endif  // VK_USE_PLATFORM_GGP
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceMeshShaderFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceMeshShaderPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_FOOTPRINT_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_EXCLUSIVE_SCISSOR_STATE_CREATE_INFO_NV:
            delete reinterpret_cast<safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXCLUSIVE_SCISSOR_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceExclusiveScissorFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_NV:
            delete reinterpret_cast<safe_VkQueueFamilyCheckpointPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_2_NV:
            delete reinterpret_cast<safe_VkQueueFamilyCheckpointProperties2NV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_FUNCTIONS_2_FEATURES_INTEL:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_QUERY_CREATE_INFO_INTEL:
            delete reinterpret_cast<safe_VkQueryPoolPerformanceQueryCreateInfoINTEL *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDevicePCIBusInfoPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DISPLAY_NATIVE_HDR_SURFACE_CAPABILITIES_AMD:
            delete reinterpret_cast<safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_SWAPCHAIN_DISPLAY_NATIVE_HDR_CREATE_INFO_AMD:
            delete reinterpret_cast<safe_VkSwapchainDisplayNativeHdrCreateInfoAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkRenderPassFragmentDensityMapCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_DENSITY_MAP_ATTACHMENT_INFO_EXT:
            delete reinterpret_cast<safe_VkRenderingFragmentDensityMapAttachmentInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_2_AMD:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderCoreProperties2AMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COHERENT_MEMORY_FEATURES_AMD:
            delete reinterpret_cast<safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_MEMORY_PRIORITY_ALLOCATE_INFO_EXT:
            delete reinterpret_cast<safe_VkMemoryPriorityAllocateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEDICATED_ALLOCATION_IMAGE_ALIASING_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkBufferDeviceAddressCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT:
            delete reinterpret_cast<safe_VkValidationFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COVERAGE_REDUCTION_MODE_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_REDUCTION_STATE_CREATE_INFO_NV:
            delete reinterpret_cast<safe_VkPipelineCoverageReductionStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_IMAGE_ARRAYS_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceProvokingVertexFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceProvokingVertexPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_PROVOKING_VERTEX_STATE_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkPipelineRasterizationProvokingVertexStateCreateInfoEXT *>(header);
            break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT:
            delete reinterpret_cast<safe_VkSurfaceFullScreenExclusiveInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_FULL_SCREEN_EXCLUSIVE_EXT:
            delete reinterpret_cast<safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT:
            delete reinterpret_cast<safe_VkSurfaceFullScreenExclusiveWin32InfoEXT *>(header);
            break;
#endif  // VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAP_MEMORY_PLACED_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceMapMemoryPlacedFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAP_MEMORY_PLACED_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceMapMemoryPlacedPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_MEMORY_MAP_PLACED_INFO_EXT:
            delete reinterpret_cast<safe_VkMemoryMapPlacedInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_2_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_PROPERTIES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_SHADER_GROUPS_CREATE_INFO_NV:
            delete reinterpret_cast<safe_VkGraphicsPipelineShaderGroupsCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INHERITED_VIEWPORT_SCISSOR_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceInheritedViewportScissorFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_VIEWPORT_SCISSOR_INFO_NV:
            delete reinterpret_cast<safe_VkCommandBufferInheritanceViewportScissorInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_TRANSFORM_BEGIN_INFO_QCOM:
            delete reinterpret_cast<safe_VkRenderPassTransformBeginInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDER_PASS_TRANSFORM_INFO_QCOM:
            delete reinterpret_cast<safe_VkCommandBufferInheritanceRenderPassTransformInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_BIAS_CONTROL_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceDepthBiasControlFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEPTH_BIAS_REPRESENTATION_INFO_EXT:
            delete reinterpret_cast<safe_VkDepthBiasRepresentationInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_MEMORY_REPORT_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceDeviceMemoryReportFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_DEVICE_MEMORY_REPORT_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkDeviceDeviceMemoryReportCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkSamplerCustomBorderColorCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_BARRIER_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDevicePresentBarrierFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_PRESENT_BARRIER_NV:
            delete reinterpret_cast<safe_VkSurfaceCapabilitiesPresentBarrierNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_BARRIER_CREATE_INFO_NV:
            delete reinterpret_cast<safe_VkSwapchainPresentBarrierCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DIAGNOSTICS_CONFIG_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceDiagnosticsConfigFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_DIAGNOSTICS_CONFIG_CREATE_INFO_NV:
            delete reinterpret_cast<safe_VkDeviceDiagnosticsConfigCreateInfoNV *>(header);
            break;
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUDA_KERNEL_LAUNCH_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceCudaKernelLaunchFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUDA_KERNEL_LAUNCH_PROPERTIES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceCudaKernelLaunchPropertiesNV *>(header);
            break;
#endif  // VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_SHADING_FEATURES_QCOM:
            delete reinterpret_cast<safe_VkPhysicalDeviceTileShadingFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_SHADING_PROPERTIES_QCOM:
            delete reinterpret_cast<safe_VkPhysicalDeviceTileShadingPropertiesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_TILE_SHADING_CREATE_INFO_QCOM:
            delete reinterpret_cast<safe_VkRenderPassTileShadingCreateInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUERY_LOW_LATENCY_SUPPORT_NV:
            delete reinterpret_cast<safe_VkQueryLowLatencySupportNV *>(header);
            break;
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_EXPORT_METAL_OBJECT_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkExportMetalObjectCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_METAL_DEVICE_INFO_EXT:
            delete reinterpret_cast<safe_VkExportMetalDeviceInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_METAL_COMMAND_QUEUE_INFO_EXT:
            delete reinterpret_cast<safe_VkExportMetalCommandQueueInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_METAL_BUFFER_INFO_EXT:
            delete reinterpret_cast<safe_VkExportMetalBufferInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMPORT_METAL_BUFFER_INFO_EXT:
            delete reinterpret_cast<safe_VkImportMetalBufferInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_METAL_TEXTURE_INFO_EXT:
            delete reinterpret_cast<safe_VkExportMetalTextureInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMPORT_METAL_TEXTURE_INFO_EXT:
            delete reinterpret_cast<safe_VkImportMetalTextureInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_METAL_IO_SURFACE_INFO_EXT:
            delete reinterpret_cast<safe_VkExportMetalIOSurfaceInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMPORT_METAL_IO_SURFACE_INFO_EXT:
            delete reinterpret_cast<safe_VkImportMetalIOSurfaceInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_METAL_SHARED_EVENT_INFO_EXT:
            delete reinterpret_cast<safe_VkExportMetalSharedEventInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMPORT_METAL_SHARED_EVENT_INFO_EXT:
            delete reinterpret_cast<safe_VkImportMetalSharedEventInfoEXT *>(header);
            break;
#endif  // VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceDescriptorBufferPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_DENSITY_MAP_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceDescriptorBufferFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_PUSH_DESCRIPTOR_BUFFER_HANDLE_EXT:
            delete reinterpret_cast<safe_VkDescriptorBufferBindingPushDescriptorBufferHandleEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_OPAQUE_CAPTURE_DESCRIPTOR_DATA_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkOpaqueCaptureDescriptorDataCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkGraphicsPipelineLibraryCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EARLY_AND_LATE_FRAGMENT_TESTS_FEATURES_AMD:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_PROPERTIES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_ENUM_STATE_CREATE_INFO_NV:
            delete reinterpret_cast<safe_VkPipelineFragmentShadingRateEnumStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_MOTION_TRIANGLES_DATA_NV:
            delete reinterpret_cast<safe_VkAccelerationStructureGeometryMotionTrianglesDataNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MOTION_INFO_NV:
            delete reinterpret_cast<safe_VkAccelerationStructureMotionInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MOTION_BLUR_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceRayTracingMotionBlurFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_2_PLANE_444_FORMATS_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceFragmentDensityMap2FeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceFragmentDensityMap2PropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_COPY_COMMAND_TRANSFORM_INFO_QCOM:
            delete reinterpret_cast<safe_VkCopyCommandTransformInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceImageCompressionControlFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_CONTROL_EXT:
            delete reinterpret_cast<safe_VkImageCompressionControlEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkImageCompressionPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_4444_FORMATS_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDevice4444FormatsFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FAULT_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceFaultFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RGBA10X6_FORMATS_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkMutableDescriptorTypeCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRM_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceDrmPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ADDRESS_BINDING_REPORT_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceAddressBindingReportFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_ADDRESS_BINDING_CALLBACK_DATA_EXT:
            delete reinterpret_cast<safe_VkDeviceAddressBindingCallbackDataEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_CONTROL_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceDepthClipControlFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLIP_CONTROL_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkPipelineViewportDepthClipControlCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT *>(header);
            break;
#ifdef VK_USE_PLATFORM_FUCHSIA
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_ZIRCON_HANDLE_INFO_FUCHSIA:
            delete reinterpret_cast<safe_VkImportMemoryZirconHandleInfoFUCHSIA *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_BUFFER_COLLECTION_FUCHSIA:
            delete reinterpret_cast<safe_VkImportMemoryBufferCollectionFUCHSIA *>(header);
            break;
        case VK_STRUCTURE_TYPE_BUFFER_COLLECTION_IMAGE_CREATE_INFO_FUCHSIA:
            delete reinterpret_cast<safe_VkBufferCollectionImageCreateInfoFUCHSIA *>(header);
            break;
        case VK_STRUCTURE_TYPE_BUFFER_COLLECTION_BUFFER_CREATE_INFO_FUCHSIA:
            delete reinterpret_cast<safe_VkBufferCollectionBufferCreateInfoFUCHSIA *>(header);
            break;
#endif  // VK_USE_PLATFORM_FUCHSIA
        case VK_STRUCTURE_TYPE_SUBPASS_SHADING_PIPELINE_CREATE_INFO_HUAWEI:
            delete reinterpret_cast<safe_VkSubpassShadingPipelineCreateInfoHUAWEI *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_SHADING_FEATURES_HUAWEI:
            delete reinterpret_cast<safe_VkPhysicalDeviceSubpassShadingFeaturesHUAWEI *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_SHADING_PROPERTIES_HUAWEI:
            delete reinterpret_cast<safe_VkPhysicalDeviceSubpassShadingPropertiesHUAWEI *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INVOCATION_MASK_FEATURES_HUAWEI:
            delete reinterpret_cast<safe_VkPhysicalDeviceInvocationMaskFeaturesHUAWEI *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_RDMA_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceExternalMemoryRDMAFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROPERTIES_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDevicePipelinePropertiesFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAME_BOUNDARY_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceFrameBoundaryFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_FRAME_BOUNDARY_EXT:
            delete reinterpret_cast<safe_VkFrameBoundaryEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SUBPASS_RESOLVE_PERFORMANCE_QUERY_EXT:
            delete reinterpret_cast<safe_VkSubpassResolvePerformanceQueryEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_INFO_EXT:
            delete reinterpret_cast<safe_VkMultisampledRenderToSingleSampledInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_COLOR_WRITE_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkPipelineColorWriteCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVES_GENERATED_QUERY_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_MIN_LOD_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceImageViewMinLodFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_VIEW_MIN_LOD_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkImageViewMinLodCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceMultiDrawFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceMultiDrawPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_2D_VIEW_OF_3D_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceImage2DViewOf3DFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderTileImageFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderTileImagePropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceOpacityMicromapFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceOpacityMicromapPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_OPACITY_MICROMAP_EXT:
            delete reinterpret_cast<safe_VkAccelerationStructureTrianglesOpacityMicromapEXT *>(header);
            break;
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISPLACEMENT_MICROMAP_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceDisplacementMicromapFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISPLACEMENT_MICROMAP_PROPERTIES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceDisplacementMicromapPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_DISPLACEMENT_MICROMAP_NV:
            delete reinterpret_cast<safe_VkAccelerationStructureTrianglesDisplacementMicromapNV *>(header);
            break;
#endif  // VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_FEATURES_HUAWEI:
            delete reinterpret_cast<safe_VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_PROPERTIES_HUAWEI:
            delete reinterpret_cast<safe_VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_VRS_FEATURES_HUAWEI:
            delete reinterpret_cast<safe_VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BORDER_COLOR_SWIZZLE_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceBorderColorSwizzleFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLER_BORDER_COLOR_COMPONENT_MAPPING_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkSamplerBorderColorComponentMappingCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PAGEABLE_DEVICE_LOCAL_MEMORY_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_ARM:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderCorePropertiesARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_QUEUE_SHADER_CORE_CONTROL_CREATE_INFO_ARM:
            delete reinterpret_cast<safe_VkDeviceQueueShaderCoreControlCreateInfoARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCHEDULING_CONTROLS_FEATURES_ARM:
            delete reinterpret_cast<safe_VkPhysicalDeviceSchedulingControlsFeaturesARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCHEDULING_CONTROLS_PROPERTIES_ARM:
            delete reinterpret_cast<safe_VkPhysicalDeviceSchedulingControlsPropertiesARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_SLICED_VIEW_OF_3D_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_VIEW_SLICED_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkImageViewSlicedCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_SET_HOST_MAPPING_FEATURES_VALVE:
            delete reinterpret_cast<safe_VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NON_SEAMLESS_CUBE_MAP_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RENDER_PASS_STRIPED_FEATURES_ARM:
            delete reinterpret_cast<safe_VkPhysicalDeviceRenderPassStripedFeaturesARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RENDER_PASS_STRIPED_PROPERTIES_ARM:
            delete reinterpret_cast<safe_VkPhysicalDeviceRenderPassStripedPropertiesARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_STRIPE_BEGIN_INFO_ARM:
            delete reinterpret_cast<safe_VkRenderPassStripeBeginInfoARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_STRIPE_SUBMIT_INFO_ARM:
            delete reinterpret_cast<safe_VkRenderPassStripeSubmitInfoARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_OFFSET_END_INFO_EXT:
            delete reinterpret_cast<safe_VkRenderPassFragmentDensityMapOffsetEndInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceCopyMemoryIndirectFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_PROPERTIES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceCopyMemoryIndirectPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceMemoryDecompressionFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_PROPERTIES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceMemoryDecompressionPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_COMPUTE_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_INDIRECT_BUFFER_INFO_NV:
            delete reinterpret_cast<safe_VkComputePipelineIndirectBufferInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_LINEAR_SWEPT_SPHERES_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_LINEAR_SWEPT_SPHERES_DATA_NV:
            delete reinterpret_cast<safe_VkAccelerationStructureGeometryLinearSweptSpheresDataNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_SPHERES_DATA_NV:
            delete reinterpret_cast<safe_VkAccelerationStructureGeometrySpheresDataNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINEAR_COLOR_ATTACHMENT_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceLinearColorAttachmentFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_VIEW_SAMPLE_WEIGHT_CREATE_INFO_QCOM:
            delete reinterpret_cast<safe_VkImageViewSampleWeightCreateInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_FEATURES_QCOM:
            delete reinterpret_cast<safe_VkPhysicalDeviceImageProcessingFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_PROPERTIES_QCOM:
            delete reinterpret_cast<safe_VkPhysicalDeviceImageProcessingPropertiesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceNestedCommandBufferFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceNestedCommandBufferPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_ACQUIRE_UNMODIFIED_EXT:
            delete reinterpret_cast<safe_VkExternalMemoryAcquireUnmodifiedEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceExtendedDynamicState3PropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_MERGE_FEEDBACK_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_CREATION_CONTROL_EXT:
            delete reinterpret_cast<safe_VkRenderPassCreationControlEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_CREATION_FEEDBACK_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkRenderPassCreationFeedbackCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_SUBPASS_FEEDBACK_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkRenderPassSubpassFeedbackCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_LIST_LUNARG:
            delete reinterpret_cast<safe_VkDirectDriverLoadingListLUNARG *>(header);
            break;
        case VK_STRUCTURE_TYPE_TENSOR_DESCRIPTION_ARM:
            delete reinterpret_cast<safe_VkTensorDescriptionARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_TENSOR_ARM:
            delete reinterpret_cast<safe_VkWriteDescriptorSetTensorARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_TENSOR_FORMAT_PROPERTIES_ARM:
            delete reinterpret_cast<safe_VkTensorFormatPropertiesARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TENSOR_PROPERTIES_ARM:
            delete reinterpret_cast<safe_VkPhysicalDeviceTensorPropertiesARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_TENSOR_MEMORY_BARRIER_ARM:
            delete reinterpret_cast<safe_VkTensorMemoryBarrierARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_TENSOR_DEPENDENCY_INFO_ARM:
            delete reinterpret_cast<safe_VkTensorDependencyInfoARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TENSOR_FEATURES_ARM:
            delete reinterpret_cast<safe_VkPhysicalDeviceTensorFeaturesARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_TENSOR_ARM:
            delete reinterpret_cast<safe_VkMemoryDedicatedAllocateInfoTensorARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_TENSOR_CREATE_INFO_ARM:
            delete reinterpret_cast<safe_VkExternalMemoryTensorCreateInfoARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_TENSOR_FEATURES_ARM:
            delete reinterpret_cast<safe_VkPhysicalDeviceDescriptorBufferTensorFeaturesARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_TENSOR_PROPERTIES_ARM:
            delete reinterpret_cast<safe_VkPhysicalDeviceDescriptorBufferTensorPropertiesARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_DESCRIPTOR_GET_TENSOR_INFO_ARM:
            delete reinterpret_cast<safe_VkDescriptorGetTensorInfoARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_FRAME_BOUNDARY_TENSORS_ARM:
            delete reinterpret_cast<safe_VkFrameBoundaryTensorsARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_MODULE_IDENTIFIER_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkPipelineShaderStageModuleIdentifierCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceOpticalFlowFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_PROPERTIES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceOpticalFlowPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_OPTICAL_FLOW_IMAGE_FORMAT_INFO_NV:
            delete reinterpret_cast<safe_VkOpticalFlowImageFormatInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_OPTICAL_FLOW_SESSION_CREATE_PRIVATE_DATA_INFO_NV:
            delete reinterpret_cast<safe_VkOpticalFlowSessionCreatePrivateDataInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_DITHERING_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceLegacyDitheringFeaturesEXT *>(header);
            break;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_FEATURES_ANDROID:
            delete reinterpret_cast<safe_VkPhysicalDeviceExternalFormatResolveFeaturesANDROID *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_PROPERTIES_ANDROID:
            delete reinterpret_cast<safe_VkPhysicalDeviceExternalFormatResolvePropertiesANDROID *>(header);
            break;
        case VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_RESOLVE_PROPERTIES_ANDROID:
            delete reinterpret_cast<safe_VkAndroidHardwareBufferFormatResolvePropertiesANDROID *>(header);
            break;
#endif  // VK_USE_PLATFORM_ANDROID_KHR
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ANTI_LAG_FEATURES_AMD:
            delete reinterpret_cast<safe_VkPhysicalDeviceAntiLagFeaturesAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderObjectFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderObjectPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_PROPERTIES_FEATURES_QCOM:
            delete reinterpret_cast<safe_VkPhysicalDeviceTilePropertiesFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_AMIGO_PROFILING_FEATURES_SEC:
            delete reinterpret_cast<safe_VkPhysicalDeviceAmigoProfilingFeaturesSEC *>(header);
            break;
        case VK_STRUCTURE_TYPE_AMIGO_PROFILING_SUBMIT_INFO_SEC:
            delete reinterpret_cast<safe_VkAmigoProfilingSubmitInfoSEC *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_VIEWPORTS_FEATURES_QCOM:
            delete reinterpret_cast<safe_VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_PROPERTIES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_VECTOR_PROPERTIES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceCooperativeVectorPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_VECTOR_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceCooperativeVectorFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_SPARSE_ADDRESS_SPACE_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_SPARSE_ADDRESS_SPACE_PROPERTIES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_VERTEX_ATTRIBUTES_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_VERTEX_ATTRIBUTES_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkLayerSettingsCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_BUILTINS_FEATURES_ARM:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_BUILTINS_PROPERTIES_ARM:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_LIBRARY_GROUP_HANDLES_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_LATENCY_SUBMISSION_PRESENT_ID_NV:
            delete reinterpret_cast<safe_VkLatencySubmissionPresentIdNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_SWAPCHAIN_LATENCY_CREATE_INFO_NV:
            delete reinterpret_cast<safe_VkSwapchainLatencyCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_LATENCY_SURFACE_CAPABILITIES_NV:
            delete reinterpret_cast<safe_VkLatencySurfaceCapabilitiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DATA_GRAPH_FEATURES_ARM:
            delete reinterpret_cast<safe_VkPhysicalDeviceDataGraphFeaturesARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_COMPILER_CONTROL_CREATE_INFO_ARM:
            delete reinterpret_cast<safe_VkDataGraphPipelineCompilerControlCreateInfoARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_SHADER_MODULE_CREATE_INFO_ARM:
            delete reinterpret_cast<safe_VkDataGraphPipelineShaderModuleCreateInfoARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_IDENTIFIER_CREATE_INFO_ARM:
            delete reinterpret_cast<safe_VkDataGraphPipelineIdentifierCreateInfoARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_DATA_GRAPH_PROCESSING_ENGINE_CREATE_INFO_ARM:
            delete reinterpret_cast<safe_VkDataGraphProcessingEngineCreateInfoARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_CONSTANT_TENSOR_SEMI_STRUCTURED_SPARSITY_INFO_ARM:
            delete reinterpret_cast<safe_VkDataGraphPipelineConstantTensorSemiStructuredSparsityInfoARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_RENDER_AREAS_FEATURES_QCOM:
            delete reinterpret_cast<safe_VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_MULTIVIEW_PER_VIEW_RENDER_AREAS_RENDER_PASS_BEGIN_INFO_QCOM:
            delete reinterpret_cast<safe_VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PER_STAGE_DESCRIPTOR_SET_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDevicePerStageDescriptorSetFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_2_FEATURES_QCOM:
            delete reinterpret_cast<safe_VkPhysicalDeviceImageProcessing2FeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_2_PROPERTIES_QCOM:
            delete reinterpret_cast<safe_VkPhysicalDeviceImageProcessing2PropertiesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLER_BLOCK_MATCH_WINDOW_CREATE_INFO_QCOM:
            delete reinterpret_cast<safe_VkSamplerBlockMatchWindowCreateInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_WEIGHTS_FEATURES_QCOM:
            delete reinterpret_cast<safe_VkPhysicalDeviceCubicWeightsFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLER_CUBIC_WEIGHTS_CREATE_INFO_QCOM:
            delete reinterpret_cast<safe_VkSamplerCubicWeightsCreateInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_BLIT_IMAGE_CUBIC_WEIGHTS_INFO_QCOM:
            delete reinterpret_cast<safe_VkBlitImageCubicWeightsInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_DEGAMMA_FEATURES_QCOM:
            delete reinterpret_cast<safe_VkPhysicalDeviceYcbcrDegammaFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_YCBCR_DEGAMMA_CREATE_INFO_QCOM:
            delete reinterpret_cast<safe_VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_CLAMP_FEATURES_QCOM:
            delete reinterpret_cast<safe_VkPhysicalDeviceCubicClampFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT *>(header);
            break;
#ifdef VK_USE_PLATFORM_SCREEN_QNX
        case VK_STRUCTURE_TYPE_SCREEN_BUFFER_FORMAT_PROPERTIES_QNX:
            delete reinterpret_cast<safe_VkScreenBufferFormatPropertiesQNX *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMPORT_SCREEN_BUFFER_INFO_QNX:
            delete reinterpret_cast<safe_VkImportScreenBufferInfoQNX *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_QNX:
            delete reinterpret_cast<safe_VkExternalFormatQNX *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_SCREEN_BUFFER_FEATURES_QNX:
            delete reinterpret_cast<safe_VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX *>(header);
            break;
#endif  // VK_USE_PLATFORM_SCREEN_QNX
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_DRIVER_PROPERTIES_MSFT:
            delete reinterpret_cast<safe_VkPhysicalDeviceLayeredDriverPropertiesMSFT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_POOL_OVERALLOCATION_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_MEMORY_HEAP_FEATURES_QCOM:
            delete reinterpret_cast<safe_VkPhysicalDeviceTileMemoryHeapFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_MEMORY_HEAP_PROPERTIES_QCOM:
            delete reinterpret_cast<safe_VkPhysicalDeviceTileMemoryHeapPropertiesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_TILE_MEMORY_REQUIREMENTS_QCOM:
            delete reinterpret_cast<safe_VkTileMemoryRequirementsQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_TILE_MEMORY_BIND_INFO_QCOM:
            delete reinterpret_cast<safe_VkTileMemoryBindInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_TILE_MEMORY_SIZE_INFO_QCOM:
            delete reinterpret_cast<safe_VkTileMemorySizeInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_DISPLAY_SURFACE_STEREO_CREATE_INFO_NV:
            delete reinterpret_cast<safe_VkDisplaySurfaceStereoCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_DISPLAY_MODE_STEREO_PROPERTIES_NV:
            delete reinterpret_cast<safe_VkDisplayModeStereoPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAW_ACCESS_CHAINS_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceRawAccessChainsFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXTERNAL_COMPUTE_QUEUE_DEVICE_CREATE_INFO_NV:
            delete reinterpret_cast<safe_VkExternalComputeQueueDeviceCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_COMPUTE_QUEUE_PROPERTIES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceExternalComputeQueuePropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMMAND_BUFFER_INHERITANCE_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceCommandBufferInheritanceFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT16_VECTOR_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_REPLICATED_COMPOSITES_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT8_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceShaderFloat8FeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_VALIDATION_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceRayTracingValidationFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_ACCELERATION_STRUCTURE_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceClusterAccelerationStructureFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_ACCELERATION_STRUCTURE_PROPERTIES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceClusterAccelerationStructurePropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CLUSTER_ACCELERATION_STRUCTURE_CREATE_INFO_NV:
            delete reinterpret_cast<safe_VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PARTITIONED_ACCELERATION_STRUCTURE_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PARTITIONED_ACCELERATION_STRUCTURE_PROPERTIES_NV:
            delete reinterpret_cast<safe_VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_FLAGS_NV:
            delete reinterpret_cast<safe_VkPartitionedAccelerationStructureFlagsNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_PARTITIONED_ACCELERATION_STRUCTURE_NV:
            delete reinterpret_cast<safe_VkWriteDescriptorSetPartitionedAccelerationStructureNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_GENERATED_COMMANDS_PIPELINE_INFO_EXT:
            delete reinterpret_cast<safe_VkGeneratedCommandsPipelineInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_GENERATED_COMMANDS_SHADER_INFO_EXT:
            delete reinterpret_cast<safe_VkGeneratedCommandsShaderInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ALIGNMENT_CONTROL_FEATURES_MESA:
            delete reinterpret_cast<safe_VkPhysicalDeviceImageAlignmentControlFeaturesMESA *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ALIGNMENT_CONTROL_PROPERTIES_MESA:
            delete reinterpret_cast<safe_VkPhysicalDeviceImageAlignmentControlPropertiesMESA *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_ALIGNMENT_CONTROL_CREATE_INFO_MESA:
            delete reinterpret_cast<safe_VkImageAlignmentControlCreateInfoMESA *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_CONTROL_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceDepthClampControlFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLAMP_CONTROL_CREATE_INFO_EXT:
            delete reinterpret_cast<safe_VkPipelineViewportDepthClampControlCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HDR_VIVID_FEATURES_HUAWEI:
            delete reinterpret_cast<safe_VkPhysicalDeviceHdrVividFeaturesHUAWEI *>(header);
            break;
        case VK_STRUCTURE_TYPE_HDR_VIVID_DYNAMIC_METADATA_HUAWEI:
            delete reinterpret_cast<safe_VkHdrVividDynamicMetadataHUAWEI *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_2_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceCooperativeMatrix2FeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_2_PROPERTIES_NV:
            delete reinterpret_cast<safe_VkPhysicalDeviceCooperativeMatrix2PropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_OPACITY_MICROMAP_FEATURES_ARM:
            delete reinterpret_cast<safe_VkPhysicalDevicePipelineOpacityMicromapFeaturesARM *>(header);
            break;
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_METAL_HANDLE_INFO_EXT:
            delete reinterpret_cast<safe_VkImportMemoryMetalHandleInfoEXT *>(header);
            break;
#endif  // VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_ROBUSTNESS_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FORMAT_PACK_FEATURES_ARM:
            delete reinterpret_cast<safe_VkPhysicalDeviceFormatPackFeaturesARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_LAYERED_FEATURES_VALVE:
            delete reinterpret_cast<safe_VkPhysicalDeviceFragmentDensityMapLayeredFeaturesVALVE *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_LAYERED_PROPERTIES_VALVE:
            delete reinterpret_cast<safe_VkPhysicalDeviceFragmentDensityMapLayeredPropertiesVALVE *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_DENSITY_MAP_LAYERED_CREATE_INFO_VALVE:
            delete reinterpret_cast<safe_VkPipelineFragmentDensityMapLayeredCreateInfoVALVE *>(header);
            break;
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_SET_PRESENT_CONFIG_NV:
            delete reinterpret_cast<safe_VkSetPresentConfigNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_METERING_FEATURES_NV:
            delete reinterpret_cast<safe_VkPhysicalDevicePresentMeteringFeaturesNV *>(header);
            break;
#endif  // VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_DEVICE_MEMORY_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CACHE_INCREMENTAL_MODE_FEATURES_SEC:
            delete reinterpret_cast<safe_VkPhysicalDevicePipelineCacheIncrementalModeFeaturesSEC *>(header);
            break;
        case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR:
            delete reinterpret_cast<safe_VkWriteDescriptorSetAccelerationStructureKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR:
            delete reinterpret_cast<safe_VkRayTracingPipelineCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR:
            delete reinterpret_cast<safe_VkPhysicalDeviceRayQueryFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceMeshShaderFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT:
            delete reinterpret_cast<safe_VkPhysicalDeviceMeshShaderPropertiesEXT *>(header);
            break;

        default: // Encountered an unknown sType
            // If sType is in custom list, free custom struct memory and clean up
            for (auto item : GetCustomStypeInfo()   ) {
                if (item.first == static_cast<uint32_t>(header->sType)) {
                    free(current);
                    break;
                }
            }
            break;
        }
        current = next;
    }
}  // clang-format on

}  // namespace vku

// NOLINTEND
