// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See pnext_chain_extraction_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2023-2025 The Khronos Group Inc.
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
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

#include "pnext_chain_extraction.h"

#include <vulkan/utility/vk_struct_helper.hpp>

namespace vvl {

void *PnextChainAdd(void *chain, void *new_struct) {
    assert(chain);
    assert(new_struct);
    void *chain_end = vku::FindLastStructInPNextChain(chain);
    auto *vk_base_struct = static_cast<VkBaseOutStructure *>(chain_end);
    assert(!vk_base_struct->pNext);
    vk_base_struct->pNext = static_cast<VkBaseOutStructure *>(new_struct);
    return new_struct;
}

void PnextChainRemoveLast(void *chain) {
    if (!chain) {
        return;
    }
    auto *current = static_cast<VkBaseOutStructure *>(chain);
    auto *prev = current;
    while (current->pNext) {
        prev = current;
        current = static_cast<VkBaseOutStructure *>(current->pNext);
    }
    prev->pNext = nullptr;
}

void PnextChainFree(void *chain) {
    if (!chain) return;
    auto header = reinterpret_cast<VkBaseOutStructure *>(chain);
    switch (header->sType) {
        case VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkShaderModuleCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineLayoutCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceSubgroupProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevice16BitStorageFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkMemoryDedicatedRequirements *>(header);
            break;
        case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkMemoryDedicatedAllocateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkMemoryAllocateFlagsInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDeviceGroupRenderPassBeginInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDeviceGroupCommandBufferBeginInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDeviceGroupSubmitInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_BIND_SPARSE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDeviceGroupBindSparseInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkBindBufferMemoryDeviceGroupInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_DEVICE_GROUP_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkBindImageMemoryDeviceGroupInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDeviceGroupDeviceCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceFeatures2 *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePointClippingProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkRenderPassInputAttachmentAspectCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImageViewUsageCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineTessellationDomainOriginStateCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkRenderPassMultiviewCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMultiviewFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMultiviewProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceVariablePointersFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceProtectedMemoryFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceProtectedMemoryProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkProtectedSubmitInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSamplerYcbcrConversionInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkBindImagePlaneMemoryInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImagePlaneMemoryRequirementsInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceSamplerYcbcrConversionFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_IMAGE_FORMAT_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSamplerYcbcrConversionImageFormatProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceExternalImageFormatInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkExternalImageFormatProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceIDProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkExternalMemoryImageCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkExternalMemoryBufferCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkExportMemoryAllocateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkExportFenceCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkExportSemaphoreCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMaintenance3Properties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderDrawParametersFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceVulkan11Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceVulkan11Properties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceVulkan12Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceVulkan12Properties *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImageFormatListCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevice8BitStorageFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDriverProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderAtomicInt64Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderFloat16Int8Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceFloatControlsProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDescriptorSetLayoutBindingFlagsCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDescriptorIndexingFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDescriptorIndexingProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDescriptorSetVariableDescriptorCountAllocateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_LAYOUT_SUPPORT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDescriptorSetVariableDescriptorCountLayoutSupport *>(header);
            break;
        case VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSubpassDescriptionDepthStencilResolve *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDepthStencilResolveProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceScalarBlockLayoutFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImageStencilUsageCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSamplerReductionModeCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceSamplerFilterMinmaxProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceVulkanMemoryModelFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceImagelessFramebufferFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkFramebufferAttachmentsCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkRenderPassAttachmentBeginInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceUniformBufferStandardLayoutFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_STENCIL_LAYOUT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkAttachmentReferenceStencilLayout *>(header);
            break;
        case VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_STENCIL_LAYOUT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkAttachmentDescriptionStencilLayout *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceHostQueryResetFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceTimelineSemaphoreFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceTimelineSemaphoreProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSemaphoreTypeCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkTimelineSemaphoreSubmitInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceBufferDeviceAddressFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkBufferOpaqueCaptureAddressCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkMemoryOpaqueCaptureAddressAllocateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceVulkan13Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceVulkan13Properties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_CREATION_FEEDBACK_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineCreationFeedbackCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderTerminateInvocationFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePrivateDataFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_PRIVATE_DATA_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDevicePrivateDataCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePipelineCreationCacheControlFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_MEMORY_BARRIER_2:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkMemoryBarrier2 *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceSynchronization2Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceImageRobustnessFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceSubgroupSizeControlFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceSubgroupSizeControlProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineShaderStageRequiredSubgroupSizeCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceInlineUniformBlockFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceInlineUniformBlockProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkWriteDescriptorSetInlineUniformBlock *>(header);
            break;
        case VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_INLINE_UNIFORM_BLOCK_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDescriptorPoolInlineUniformBlockCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceTextureCompressionASTCHDRFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineRenderingCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDynamicRenderingFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkCommandBufferInheritanceRenderingInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderIntegerDotProductFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderIntegerDotProductProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceTexelBufferAlignmentProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_3:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkFormatProperties3 *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMaintenance4Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMaintenance4Properties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceVulkan14Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceVulkan14Properties *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDeviceQueueGlobalPriorityCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GLOBAL_PRIORITY_QUERY_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceGlobalPriorityQueryFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUEUE_FAMILY_GLOBAL_PRIORITY_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkQueueFamilyGlobalPriorityProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_ROTATE_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderSubgroupRotateFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT_CONTROLS_2_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderFloatControls2Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EXPECT_ASSUME_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderExpectAssumeFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceLineRasterizationFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceLineRasterizationProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineRasterizationLineStateCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceVertexAttributeDivisorProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineVertexInputDivisorStateCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceVertexAttributeDivisorFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceIndexTypeUint8Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMaintenance5Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMaintenance5Properties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_CREATE_FLAGS_2_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineCreateFlags2CreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkBufferUsageFlags2CreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePushDescriptorProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDynamicRenderingLocalReadFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_LOCATION_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkRenderingAttachmentLocationInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDERING_INPUT_ATTACHMENT_INDEX_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkRenderingInputAttachmentIndexInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMaintenance6Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMaintenance6Properties *>(header);
            break;
        case VK_STRUCTURE_TYPE_BIND_MEMORY_STATUS:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkBindMemoryStatus *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROTECTED_ACCESS_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePipelineProtectedAccessFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePipelineRobustnessFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePipelineRobustnessProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_ROBUSTNESS_CREATE_INFO:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineRobustnessCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceHostImageCopyFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_PROPERTIES:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceHostImageCopyProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_SUBRESOURCE_HOST_MEMCPY_SIZE:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSubresourceHostMemcpySize *>(header);
            break;
        case VK_STRUCTURE_TYPE_HOST_IMAGE_COPY_DEVICE_PERFORMANCE_QUERY:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkHostImageCopyDevicePerformanceQuery *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_SWAPCHAIN_CREATE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImageSwapchainCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkBindImageMemorySwapchainInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDeviceGroupPresentInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_SWAPCHAIN_CREATE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDeviceGroupSwapchainCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDisplayPresentInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUEUE_FAMILY_QUERY_RESULT_STATUS_PROPERTIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkQueueFamilyQueryResultStatusPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUEUE_FAMILY_VIDEO_PROPERTIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkQueueFamilyVideoPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoProfileInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_PROFILE_LIST_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoProfileListInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_CAPABILITIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoDecodeCapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_USAGE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoDecodeUsageInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_CAPABILITIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH264CapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_QUALITY_LEVEL_PROPERTIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH264QualityLevelPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_CREATE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH264SessionCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_ADD_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH264SessionParametersAddInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_CREATE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH264SessionParametersCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_GET_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH264SessionParametersGetInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_FEEDBACK_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH264SessionParametersFeedbackInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PICTURE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH264PictureInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_DPB_SLOT_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH264DpbSlotInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PROFILE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH264ProfileInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_RATE_CONTROL_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH264RateControlInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_RATE_CONTROL_LAYER_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH264RateControlLayerInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_GOP_REMAINING_FRAME_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH264GopRemainingFrameInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_CAPABILITIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH265CapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_CREATE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH265SessionCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_QUALITY_LEVEL_PROPERTIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH265QualityLevelPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_ADD_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH265SessionParametersAddInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_CREATE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH265SessionParametersCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_GET_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH265SessionParametersGetInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_FEEDBACK_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH265SessionParametersFeedbackInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PICTURE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH265PictureInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_DPB_SLOT_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH265DpbSlotInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PROFILE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH265ProfileInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_RATE_CONTROL_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH265RateControlInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_RATE_CONTROL_LAYER_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH265RateControlLayerInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_GOP_REMAINING_FRAME_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH265GopRemainingFrameInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PROFILE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoDecodeH264ProfileInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_CAPABILITIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoDecodeH264CapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_ADD_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoDecodeH264SessionParametersAddInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_CREATE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoDecodeH264SessionParametersCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PICTURE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoDecodeH264PictureInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_DPB_SLOT_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoDecodeH264DpbSlotInfoKHR *>(header);
            break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImportMemoryWin32HandleInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkExportMemoryWin32HandleInfoKHR *>(header);
            break;
#endif  // VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImportMemoryFdInfoKHR *>(header);
            break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkWin32KeyedMutexAcquireReleaseInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkExportSemaphoreWin32HandleInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_D3D12_FENCE_SUBMIT_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkD3D12FenceSubmitInfoKHR *>(header);
            break;
#endif  // VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPresentRegionsKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSharedPresentSurfaceCapabilitiesKHR *>(header);
            break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_EXPORT_FENCE_WIN32_HANDLE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkExportFenceWin32HandleInfoKHR *>(header);
            break;
#endif  // VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePerformanceQueryFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_PROPERTIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePerformanceQueryPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkQueryPoolPerformanceCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_SUBMIT_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPerformanceQuerySubmitInfoKHR *>(header);
            break;
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePortabilitySubsetFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_PROPERTIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePortabilitySubsetPropertiesKHR *>(header);
            break;
#endif  // VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderClockFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PROFILE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoDecodeH265ProfileInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_CAPABILITIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoDecodeH265CapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_PARAMETERS_ADD_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoDecodeH265SessionParametersAddInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_PARAMETERS_CREATE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoDecodeH265SessionParametersCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PICTURE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoDecodeH265PictureInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_DPB_SLOT_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoDecodeH265DpbSlotInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkFragmentShadingRateAttachmentInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_STATE_CREATE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineFragmentShadingRateStateCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceFragmentShadingRateFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceFragmentShadingRatePropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkRenderingFragmentShadingRateAttachmentInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_QUAD_CONTROL_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderQuadControlFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_SURFACE_PROTECTED_CAPABILITIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSurfaceProtectedCapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePresentWaitFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineLibraryCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PRESENT_ID_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPresentIdKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePresentIdFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_CAPABILITIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeCapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUERY_POOL_VIDEO_ENCODE_FEEDBACK_CREATE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkQueryPoolVideoEncodeFeedbackCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_USAGE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeUsageInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_RATE_CONTROL_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeRateControlInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUALITY_LEVEL_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeQualityLevelInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_PROPERTIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MAXIMAL_RECONVERGENCE_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_BINARY_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePipelineBinaryFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_BINARY_PROPERTIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePipelineBinaryPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_PIPELINE_BINARY_INTERNAL_CACHE_CONTROL_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDevicePipelineBinaryInternalCacheControlKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_BINARY_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineBinaryInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceCooperativeMatrixFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceCooperativeMatrixPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_PROPERTIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_PROFILE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoDecodeAV1ProfileInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_CAPABILITIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoDecodeAV1CapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_SESSION_PARAMETERS_CREATE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoDecodeAV1SessionParametersCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_PICTURE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoDecodeAV1PictureInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_DPB_SLOT_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoDecodeAV1DpbSlotInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_AV1_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceVideoEncodeAV1FeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_CAPABILITIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeAV1CapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_QUALITY_LEVEL_PROPERTIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeAV1QualityLevelPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_SESSION_CREATE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeAV1SessionCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_SESSION_PARAMETERS_CREATE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeAV1SessionParametersCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_PICTURE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeAV1PictureInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_DPB_SLOT_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeAV1DpbSlotInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_PROFILE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeAV1ProfileInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_GOP_REMAINING_FRAME_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeAV1GopRemainingFrameInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_RATE_CONTROL_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeAV1RateControlInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_RATE_CONTROL_LAYER_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeAV1RateControlLayerInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_1_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceVideoMaintenance1FeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_INLINE_QUERY_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoInlineQueryInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_CAPABILITIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeQuantizationMapCapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_FORMAT_QUANTIZATION_MAP_PROPERTIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoFormatQuantizationMapPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeQuantizationMapInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_SESSION_PARAMETERS_CREATE_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_QUANTIZATION_MAP_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_QUANTIZATION_MAP_CAPABILITIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH264QuantizationMapCapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_QUANTIZATION_MAP_CAPABILITIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeH265QuantizationMapCapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_FORMAT_H265_QUANTIZATION_MAP_PROPERTIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoFormatH265QuantizationMapPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_QUANTIZATION_MAP_CAPABILITIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoEncodeAV1QuantizationMapCapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_FORMAT_AV1_QUANTIZATION_MAP_PROPERTIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoFormatAV1QuantizationMapPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_RELAXED_EXTENDED_INSTRUCTION_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMaintenance7FeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_PROPERTIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMaintenance7PropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_API_PROPERTIES_LIST_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceLayeredApiPropertiesListKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_API_VULKAN_PROPERTIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceLayeredApiVulkanPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_8_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMaintenance8FeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_MEMORY_BARRIER_ACCESS_FLAGS_3_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkMemoryBarrierAccessFlags3KHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_2_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceVideoMaintenance2FeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_INLINE_SESSION_PARAMETERS_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoDecodeH264InlineSessionParametersInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_INLINE_SESSION_PARAMETERS_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoDecodeH265InlineSessionParametersInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_INLINE_SESSION_PARAMETERS_INFO_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkVideoDecodeAV1InlineSessionParametersInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_ZERO_ONE_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDepthClampZeroOneFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDebugReportCallbackCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_RASTERIZATION_ORDER_AMD:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineRasterizationStateRasterizationOrderAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_IMAGE_CREATE_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDedicatedAllocationImageCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_BUFFER_CREATE_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDedicatedAllocationBufferCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_MEMORY_ALLOCATE_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDedicatedAllocationMemoryAllocateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceTransformFeedbackFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceTransformFeedbackPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_STREAM_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineRasterizationStateStreamCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_CU_MODULE_TEXTURING_MODE_CREATE_INFO_NVX:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkCuModuleTexturingModeCreateInfoNVX *>(header);
            break;
        case VK_STRUCTURE_TYPE_TEXTURE_LOD_GATHER_FORMAT_PROPERTIES_AMD:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkTextureLODGatherFormatPropertiesAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CORNER_SAMPLED_IMAGE_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceCornerSampledImageFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkExternalMemoryImageCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkExportMemoryAllocateInfoNV *>(header);
            break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImportMemoryWin32HandleInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkExportMemoryWin32HandleInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkWin32KeyedMutexAcquireReleaseInfoNV *>(header);
            break;
#endif  // VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkValidationFlagsEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_VIEW_ASTC_DECODE_MODE_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImageViewASTCDecodeModeEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ASTC_DECODE_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceASTCDecodeFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceConditionalRenderingFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkCommandBufferInheritanceConditionalRenderingInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_W_SCALING_STATE_CREATE_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineViewportWScalingStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_SWAPCHAIN_COUNTER_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSwapchainCounterCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PRESENT_TIMES_INFO_GOOGLE:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPresentTimesInfoGOOGLE *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_ATTRIBUTES_PROPERTIES_NVX:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX *>(header);
            break;
        case VK_STRUCTURE_TYPE_MULTIVIEW_PER_VIEW_ATTRIBUTES_INFO_NVX:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkMultiviewPerViewAttributesInfoNVX *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SWIZZLE_STATE_CREATE_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineViewportSwizzleStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISCARD_RECTANGLE_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDiscardRectanglePropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_DISCARD_RECTANGLE_STATE_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineDiscardRectangleStateCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceConservativeRasterizationPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineRasterizationConservativeStateCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDepthClipEnableFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineRasterizationDepthClipStateCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RELAXED_LINE_RASTERIZATION_FEATURES_IMG:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDebugUtilsObjectNameInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT *>(header);
            break;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        case VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_USAGE_ANDROID:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkAndroidHardwareBufferUsageANDROID *>(header);
            break;
        case VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkAndroidHardwareBufferFormatPropertiesANDROID *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImportAndroidHardwareBufferInfoANDROID *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkExternalFormatANDROID *>(header);
            break;
        case VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_2_ANDROID:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkAndroidHardwareBufferFormatProperties2ANDROID *>(header);
            break;
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ENQUEUE_FEATURES_AMDX:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderEnqueueFeaturesAMDX *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ENQUEUE_PROPERTIES_AMDX:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderEnqueuePropertiesAMDX *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_NODE_CREATE_INFO_AMDX:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineShaderStageNodeCreateInfoAMDX *>(header);
            break;
#endif  // VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_ATTACHMENT_SAMPLE_COUNT_INFO_AMD:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkAttachmentSampleCountInfoAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSampleLocationsInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_SAMPLE_LOCATIONS_BEGIN_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkRenderPassSampleLocationsBeginInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineSampleLocationsStateCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceSampleLocationsPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineColorBlendAdvancedStateCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_TO_COLOR_STATE_CREATE_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineCoverageToColorStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_MODULATION_STATE_CREATE_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineCoverageModulationStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_PROPERTIES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderSMBuiltinsPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderSMBuiltinsFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDrmFormatModifierPropertiesListEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceImageDrmFormatModifierInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImageDrmFormatModifierListCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImageDrmFormatModifierExplicitCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_2_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDrmFormatModifierPropertiesList2EXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SHADER_MODULE_VALIDATION_CACHE_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkShaderModuleValidationCacheCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineViewportShadingRateImageStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShadingRateImageFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_PROPERTIES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShadingRateImagePropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_COARSE_SAMPLE_ORDER_STATE_CREATE_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineViewportCoarseSampleOrderStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkWriteDescriptorSetAccelerationStructureNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceRayTracingPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_REPRESENTATIVE_FRAGMENT_TEST_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_REPRESENTATIVE_FRAGMENT_TEST_STATE_CREATE_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineRepresentativeFragmentTestStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_IMAGE_FORMAT_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceImageViewImageFormatInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_FILTER_CUBIC_IMAGE_VIEW_IMAGE_FORMAT_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkFilterCubicImageViewImageFormatPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImportMemoryHostPointerInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceExternalMemoryHostPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_COMPILER_CONTROL_CREATE_INFO_AMD:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineCompilerControlCreateInfoAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_AMD:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderCorePropertiesAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_MEMORY_OVERALLOCATION_CREATE_INFO_AMD:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDeviceMemoryOverallocationCreateInfoAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT *>(header);
            break;
#ifdef VK_USE_PLATFORM_GGP
        case VK_STRUCTURE_TYPE_PRESENT_FRAME_TOKEN_GGP:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPresentFrameTokenGGP *>(header);
            break;
#endif  // VK_USE_PLATFORM_GGP
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMeshShaderFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMeshShaderPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_FOOTPRINT_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderImageFootprintFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_EXCLUSIVE_SCISSOR_STATE_CREATE_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineViewportExclusiveScissorStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXCLUSIVE_SCISSOR_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceExclusiveScissorFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkQueueFamilyCheckpointPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_2_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkQueueFamilyCheckpointProperties2NV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_FUNCTIONS_2_FEATURES_INTEL:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_QUERY_CREATE_INFO_INTEL:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkQueryPoolPerformanceQueryCreateInfoINTEL *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePCIBusInfoPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DISPLAY_NATIVE_HDR_SURFACE_CAPABILITIES_AMD:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDisplayNativeHdrSurfaceCapabilitiesAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_SWAPCHAIN_DISPLAY_NATIVE_HDR_CREATE_INFO_AMD:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSwapchainDisplayNativeHdrCreateInfoAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkRenderPassFragmentDensityMapCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_DENSITY_MAP_ATTACHMENT_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkRenderingFragmentDensityMapAttachmentInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_2_AMD:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderCoreProperties2AMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COHERENT_MEMORY_FEATURES_AMD:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceCoherentMemoryFeaturesAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMemoryBudgetPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMemoryPriorityFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_MEMORY_PRIORITY_ALLOCATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkMemoryPriorityAllocateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEDICATED_ALLOCATION_IMAGE_ALIASING_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceBufferDeviceAddressFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkBufferDeviceAddressCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkValidationFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceCooperativeMatrixFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceCooperativeMatrixPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COVERAGE_REDUCTION_MODE_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceCoverageReductionModeFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_REDUCTION_STATE_CREATE_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineCoverageReductionStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_IMAGE_ARRAYS_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceYcbcrImageArraysFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceProvokingVertexFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceProvokingVertexPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_PROVOKING_VERTEX_STATE_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineRasterizationProvokingVertexStateCreateInfoEXT *>(header);
            break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSurfaceFullScreenExclusiveInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_FULL_SCREEN_EXCLUSIVE_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSurfaceCapabilitiesFullScreenExclusiveEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSurfaceFullScreenExclusiveWin32InfoEXT *>(header);
            break;
#endif  // VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceExtendedDynamicStateFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAP_MEMORY_PLACED_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMapMemoryPlacedFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAP_MEMORY_PLACED_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMapMemoryPlacedPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_MEMORY_MAP_PLACED_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkMemoryMapPlacedInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_2_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSurfacePresentModeEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SURFACE_PRESENT_SCALING_CAPABILITIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSurfacePresentScalingCapabilitiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_COMPATIBILITY_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSurfacePresentModeCompatibilityEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSwapchainPresentFenceInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_MODES_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSwapchainPresentModesCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_MODE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSwapchainPresentModeInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_SCALING_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSwapchainPresentScalingCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_PROPERTIES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_SHADER_GROUPS_CREATE_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkGraphicsPipelineShaderGroupsCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INHERITED_VIEWPORT_SCISSOR_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceInheritedViewportScissorFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_VIEWPORT_SCISSOR_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkCommandBufferInheritanceViewportScissorInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_TRANSFORM_BEGIN_INFO_QCOM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkRenderPassTransformBeginInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDER_PASS_TRANSFORM_INFO_QCOM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkCommandBufferInheritanceRenderPassTransformInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_BIAS_CONTROL_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDepthBiasControlFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEPTH_BIAS_REPRESENTATION_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDepthBiasRepresentationInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_MEMORY_REPORT_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDeviceMemoryReportFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_DEVICE_MEMORY_REPORT_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDeviceDeviceMemoryReportCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceRobustness2FeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceRobustness2PropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSamplerCustomBorderColorCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceCustomBorderColorPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceCustomBorderColorFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_BARRIER_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePresentBarrierFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_PRESENT_BARRIER_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSurfaceCapabilitiesPresentBarrierNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_BARRIER_CREATE_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSwapchainPresentBarrierCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DIAGNOSTICS_CONFIG_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDiagnosticsConfigFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_DIAGNOSTICS_CONFIG_CREATE_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDeviceDiagnosticsConfigCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUDA_KERNEL_LAUNCH_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceCudaKernelLaunchFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUDA_KERNEL_LAUNCH_PROPERTIES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceCudaKernelLaunchPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUERY_LOW_LATENCY_SUPPORT_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkQueryLowLatencySupportNV *>(header);
            break;
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_EXPORT_METAL_OBJECT_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkExportMetalObjectCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_METAL_DEVICE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkExportMetalDeviceInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_METAL_COMMAND_QUEUE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkExportMetalCommandQueueInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_METAL_BUFFER_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkExportMetalBufferInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMPORT_METAL_BUFFER_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImportMetalBufferInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_METAL_TEXTURE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkExportMetalTextureInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMPORT_METAL_TEXTURE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImportMetalTextureInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_METAL_IO_SURFACE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkExportMetalIOSurfaceInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMPORT_METAL_IO_SURFACE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImportMetalIOSurfaceInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_METAL_SHARED_EVENT_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkExportMetalSharedEventInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMPORT_METAL_SHARED_EVENT_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImportMetalSharedEventInfoEXT *>(header);
            break;
#endif  // VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDescriptorBufferPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_DENSITY_MAP_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDescriptorBufferFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_PUSH_DESCRIPTOR_BUFFER_HANDLE_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDescriptorBufferBindingPushDescriptorBufferHandleEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_OPAQUE_CAPTURE_DESCRIPTOR_DATA_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkOpaqueCaptureDescriptorDataCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkGraphicsPipelineLibraryCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EARLY_AND_LATE_FRAGMENT_TESTS_FEATURES_AMD:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_PROPERTIES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_ENUM_STATE_CREATE_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineFragmentShadingRateEnumStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_MOTION_TRIANGLES_DATA_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkAccelerationStructureGeometryMotionTrianglesDataNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MOTION_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkAccelerationStructureMotionInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MOTION_BLUR_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceRayTracingMotionBlurFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_2_PLANE_444_FORMATS_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceFragmentDensityMap2FeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceFragmentDensityMap2PropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_COPY_COMMAND_TRANSFORM_INFO_QCOM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkCopyCommandTransformInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceImageCompressionControlFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_CONTROL_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImageCompressionControlEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImageCompressionPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_4444_FORMATS_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevice4444FormatsFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FAULT_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceFaultFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RGBA10X6_FORMATS_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkMutableDescriptorTypeCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRM_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDrmPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ADDRESS_BINDING_REPORT_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceAddressBindingReportFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_ADDRESS_BINDING_CALLBACK_DATA_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDeviceAddressBindingCallbackDataEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_CONTROL_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDepthClipControlFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLIP_CONTROL_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineViewportDepthClipControlCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_MODE_FIFO_LATEST_READY_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT *>(header);
            break;
#ifdef VK_USE_PLATFORM_FUCHSIA
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_ZIRCON_HANDLE_INFO_FUCHSIA:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImportMemoryZirconHandleInfoFUCHSIA *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_BUFFER_COLLECTION_FUCHSIA:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImportMemoryBufferCollectionFUCHSIA *>(header);
            break;
        case VK_STRUCTURE_TYPE_BUFFER_COLLECTION_IMAGE_CREATE_INFO_FUCHSIA:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkBufferCollectionImageCreateInfoFUCHSIA *>(header);
            break;
        case VK_STRUCTURE_TYPE_BUFFER_COLLECTION_BUFFER_CREATE_INFO_FUCHSIA:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkBufferCollectionBufferCreateInfoFUCHSIA *>(header);
            break;
#endif  // VK_USE_PLATFORM_FUCHSIA
        case VK_STRUCTURE_TYPE_SUBPASS_SHADING_PIPELINE_CREATE_INFO_HUAWEI:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSubpassShadingPipelineCreateInfoHUAWEI *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_SHADING_FEATURES_HUAWEI:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceSubpassShadingFeaturesHUAWEI *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_SHADING_PROPERTIES_HUAWEI:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceSubpassShadingPropertiesHUAWEI *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INVOCATION_MASK_FEATURES_HUAWEI:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceInvocationMaskFeaturesHUAWEI *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_RDMA_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceExternalMemoryRDMAFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROPERTIES_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePipelinePropertiesFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAME_BOUNDARY_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceFrameBoundaryFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_FRAME_BOUNDARY_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkFrameBoundaryEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SUBPASS_RESOLVE_PERFORMANCE_QUERY_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSubpassResolvePerformanceQueryEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkMultisampledRenderToSingleSampledInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceExtendedDynamicState2FeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceColorWriteEnableFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_COLOR_WRITE_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineColorWriteCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVES_GENERATED_QUERY_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_MIN_LOD_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceImageViewMinLodFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_VIEW_MIN_LOD_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImageViewMinLodCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMultiDrawFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMultiDrawPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_2D_VIEW_OF_3D_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceImage2DViewOf3DFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderTileImageFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderTileImagePropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceOpacityMicromapFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceOpacityMicromapPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_OPACITY_MICROMAP_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkAccelerationStructureTrianglesOpacityMicromapEXT *>(header);
            break;
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISPLACEMENT_MICROMAP_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDisplacementMicromapFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISPLACEMENT_MICROMAP_PROPERTIES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDisplacementMicromapPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_DISPLACEMENT_MICROMAP_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkAccelerationStructureTrianglesDisplacementMicromapNV *>(header);
            break;
#endif  // VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_FEATURES_HUAWEI:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_PROPERTIES_HUAWEI:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_VRS_FEATURES_HUAWEI:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BORDER_COLOR_SWIZZLE_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceBorderColorSwizzleFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLER_BORDER_COLOR_COMPONENT_MAPPING_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSamplerBorderColorComponentMappingCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PAGEABLE_DEVICE_LOCAL_MEMORY_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_ARM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderCorePropertiesARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_QUEUE_SHADER_CORE_CONTROL_CREATE_INFO_ARM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDeviceQueueShaderCoreControlCreateInfoARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCHEDULING_CONTROLS_FEATURES_ARM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceSchedulingControlsFeaturesARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCHEDULING_CONTROLS_PROPERTIES_ARM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceSchedulingControlsPropertiesARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_SLICED_VIEW_OF_3D_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_VIEW_SLICED_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImageViewSlicedCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_SET_HOST_MAPPING_FEATURES_VALVE:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NON_SEAMLESS_CUBE_MAP_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RENDER_PASS_STRIPED_FEATURES_ARM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceRenderPassStripedFeaturesARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RENDER_PASS_STRIPED_PROPERTIES_ARM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceRenderPassStripedPropertiesARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_STRIPE_BEGIN_INFO_ARM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkRenderPassStripeBeginInfoARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_STRIPE_SUBMIT_INFO_ARM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkRenderPassStripeSubmitInfoARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_FEATURES_QCOM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_PROPERTIES_QCOM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_SUBPASS_FRAGMENT_DENSITY_MAP_OFFSET_END_INFO_QCOM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSubpassFragmentDensityMapOffsetEndInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceCopyMemoryIndirectFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_PROPERTIES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceCopyMemoryIndirectPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMemoryDecompressionFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_PROPERTIES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMemoryDecompressionPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_COMPUTE_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_INDIRECT_BUFFER_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkComputePipelineIndirectBufferInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_LINEAR_SWEPT_SPHERES_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINEAR_COLOR_ATTACHMENT_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceLinearColorAttachmentFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_VIEW_SAMPLE_WEIGHT_CREATE_INFO_QCOM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImageViewSampleWeightCreateInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_FEATURES_QCOM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceImageProcessingFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_PROPERTIES_QCOM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceImageProcessingPropertiesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceNestedCommandBufferFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceNestedCommandBufferPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_ACQUIRE_UNMODIFIED_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkExternalMemoryAcquireUnmodifiedEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceExtendedDynamicState3PropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_MERGE_FEEDBACK_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_CREATION_CONTROL_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkRenderPassCreationControlEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_CREATION_FEEDBACK_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkRenderPassCreationFeedbackCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_SUBPASS_FEEDBACK_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkRenderPassSubpassFeedbackCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_LIST_LUNARG:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDirectDriverLoadingListLUNARG *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_MODULE_IDENTIFIER_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineShaderStageModuleIdentifierCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceOpticalFlowFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_PROPERTIES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceOpticalFlowPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_OPTICAL_FLOW_IMAGE_FORMAT_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkOpticalFlowImageFormatInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_OPTICAL_FLOW_SESSION_CREATE_PRIVATE_DATA_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkOpticalFlowSessionCreatePrivateDataInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_DITHERING_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceLegacyDitheringFeaturesEXT *>(header);
            break;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_FEATURES_ANDROID:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceExternalFormatResolveFeaturesANDROID *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_PROPERTIES_ANDROID:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceExternalFormatResolvePropertiesANDROID *>(header);
            break;
        case VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_RESOLVE_PROPERTIES_ANDROID:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkAndroidHardwareBufferFormatResolvePropertiesANDROID *>(header);
            break;
#endif  // VK_USE_PLATFORM_ANDROID_KHR
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ANTI_LAG_FEATURES_AMD:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceAntiLagFeaturesAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderObjectFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderObjectPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_PROPERTIES_FEATURES_QCOM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceTilePropertiesFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_AMIGO_PROFILING_FEATURES_SEC:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceAmigoProfilingFeaturesSEC *>(header);
            break;
        case VK_STRUCTURE_TYPE_AMIGO_PROFILING_SUBMIT_INFO_SEC:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkAmigoProfilingSubmitInfoSEC *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_VIEWPORTS_FEATURES_QCOM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_PROPERTIES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_VECTOR_PROPERTIES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceCooperativeVectorPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_VECTOR_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceCooperativeVectorFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_SPARSE_ADDRESS_SPACE_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_SPARSE_ADDRESS_SPACE_PROPERTIES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_VERTEX_ATTRIBUTES_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_VERTEX_ATTRIBUTES_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkLayerSettingsCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_BUILTINS_FEATURES_ARM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_BUILTINS_PROPERTIES_ARM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_LIBRARY_GROUP_HANDLES_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_LATENCY_SUBMISSION_PRESENT_ID_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkLatencySubmissionPresentIdNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_SWAPCHAIN_LATENCY_CREATE_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSwapchainLatencyCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_LATENCY_SURFACE_CAPABILITIES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkLatencySurfaceCapabilitiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_RENDER_AREAS_FEATURES_QCOM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_MULTIVIEW_PER_VIEW_RENDER_AREAS_RENDER_PASS_BEGIN_INFO_QCOM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PER_STAGE_DESCRIPTOR_SET_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePerStageDescriptorSetFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_2_FEATURES_QCOM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceImageProcessing2FeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_2_PROPERTIES_QCOM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceImageProcessing2PropertiesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLER_BLOCK_MATCH_WINDOW_CREATE_INFO_QCOM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSamplerBlockMatchWindowCreateInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_WEIGHTS_FEATURES_QCOM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceCubicWeightsFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLER_CUBIC_WEIGHTS_CREATE_INFO_QCOM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSamplerCubicWeightsCreateInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_BLIT_IMAGE_CUBIC_WEIGHTS_INFO_QCOM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkBlitImageCubicWeightsInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_DEGAMMA_FEATURES_QCOM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceYcbcrDegammaFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_YCBCR_DEGAMMA_CREATE_INFO_QCOM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_CLAMP_FEATURES_QCOM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceCubicClampFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT *>(header);
            break;
#ifdef VK_USE_PLATFORM_SCREEN_QNX
        case VK_STRUCTURE_TYPE_SCREEN_BUFFER_FORMAT_PROPERTIES_QNX:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkScreenBufferFormatPropertiesQNX *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMPORT_SCREEN_BUFFER_INFO_QNX:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImportScreenBufferInfoQNX *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_QNX:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkExternalFormatQNX *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_SCREEN_BUFFER_FEATURES_QNX:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX *>(header);
            break;
#endif  // VK_USE_PLATFORM_SCREEN_QNX
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_DRIVER_PROPERTIES_MSFT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceLayeredDriverPropertiesMSFT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_POOL_OVERALLOCATION_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_DISPLAY_SURFACE_STEREO_CREATE_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDisplaySurfaceStereoCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_DISPLAY_MODE_STEREO_PROPERTIES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkDisplayModeStereoPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAW_ACCESS_CHAINS_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceRawAccessChainsFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMMAND_BUFFER_INHERITANCE_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceCommandBufferInheritanceFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT16_VECTOR_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_REPLICATED_COMPOSITES_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_VALIDATION_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceRayTracingValidationFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_ACCELERATION_STRUCTURE_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceClusterAccelerationStructureFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_ACCELERATION_STRUCTURE_PROPERTIES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceClusterAccelerationStructurePropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CLUSTER_ACCELERATION_STRUCTURE_CREATE_INFO_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PARTITIONED_ACCELERATION_STRUCTURE_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PARTITIONED_ACCELERATION_STRUCTURE_PROPERTIES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_FLAGS_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPartitionedAccelerationStructureFlagsNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_PARTITIONED_ACCELERATION_STRUCTURE_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkWriteDescriptorSetPartitionedAccelerationStructureNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_GENERATED_COMMANDS_PIPELINE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkGeneratedCommandsPipelineInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_GENERATED_COMMANDS_SHADER_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkGeneratedCommandsShaderInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ALIGNMENT_CONTROL_FEATURES_MESA:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceImageAlignmentControlFeaturesMESA *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ALIGNMENT_CONTROL_PROPERTIES_MESA:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceImageAlignmentControlPropertiesMESA *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_ALIGNMENT_CONTROL_CREATE_INFO_MESA:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImageAlignmentControlCreateInfoMESA *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_CONTROL_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceDepthClampControlFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLAMP_CONTROL_CREATE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPipelineViewportDepthClampControlCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HDR_VIVID_FEATURES_HUAWEI:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceHdrVividFeaturesHUAWEI *>(header);
            break;
        case VK_STRUCTURE_TYPE_HDR_VIVID_DYNAMIC_METADATA_HUAWEI:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkHdrVividDynamicMetadataHUAWEI *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_2_FEATURES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceCooperativeMatrix2FeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_2_PROPERTIES_NV:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceCooperativeMatrix2PropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_OPACITY_MICROMAP_FEATURES_ARM:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDevicePipelineOpacityMicromapFeaturesARM *>(header);
            break;
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_METAL_HANDLE_INFO_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkImportMemoryMetalHandleInfoEXT *>(header);
            break;
#endif  // VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_ROBUSTNESS_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkWriteDescriptorSetAccelerationStructureKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceAccelerationStructureFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceAccelerationStructurePropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceRayTracingPipelineFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceRayTracingPipelinePropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceRayQueryFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMeshShaderFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT:
            PnextChainFree(header->pNext);
            header->pNext = nullptr;
            delete reinterpret_cast<const VkPhysicalDeviceMeshShaderPropertiesEXT *>(header);
            break;
        default:
            assert(false);
            break;
    }
}

template <>
void *PnextChainExtract(const void *in_pnext_chain, PnextChainVkPhysicalDeviceImageFormatInfo2 &out) {
    void *chain_begin = nullptr;
    void *chain_end = nullptr;

    if (auto *chain_struct = vku::FindStructInPNextChain<VkImageCompressionControlEXT>(in_pnext_chain)) {
        auto &out_chain_struct = std::get<VkImageCompressionControlEXT>(out);
        out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
        if (!chain_begin) {
            chain_begin = &out_chain_struct;
            chain_end = chain_begin;
        } else {
            chain_end = PnextChainAdd(chain_end, &out_chain_struct);
        }
    }

    if (auto *chain_struct = vku::FindStructInPNextChain<VkImageFormatListCreateInfo>(in_pnext_chain)) {
        auto &out_chain_struct = std::get<VkImageFormatListCreateInfo>(out);
        out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
        if (!chain_begin) {
            chain_begin = &out_chain_struct;
            chain_end = chain_begin;
        } else {
            chain_end = PnextChainAdd(chain_end, &out_chain_struct);
        }
    }

    if (auto *chain_struct = vku::FindStructInPNextChain<VkImageStencilUsageCreateInfo>(in_pnext_chain)) {
        auto &out_chain_struct = std::get<VkImageStencilUsageCreateInfo>(out);
        out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
        if (!chain_begin) {
            chain_begin = &out_chain_struct;
            chain_end = chain_begin;
        } else {
            chain_end = PnextChainAdd(chain_end, &out_chain_struct);
        }
    }

    if (auto *chain_struct = vku::FindStructInPNextChain<VkOpticalFlowImageFormatInfoNV>(in_pnext_chain)) {
        auto &out_chain_struct = std::get<VkOpticalFlowImageFormatInfoNV>(out);
        out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
        if (!chain_begin) {
            chain_begin = &out_chain_struct;
            chain_end = chain_begin;
        } else {
            chain_end = PnextChainAdd(chain_end, &out_chain_struct);
        }
    }

    if (auto *chain_struct = vku::FindStructInPNextChain<VkPhysicalDeviceExternalImageFormatInfo>(in_pnext_chain)) {
        auto &out_chain_struct = std::get<VkPhysicalDeviceExternalImageFormatInfo>(out);
        out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
        if (!chain_begin) {
            chain_begin = &out_chain_struct;
            chain_end = chain_begin;
        } else {
            chain_end = PnextChainAdd(chain_end, &out_chain_struct);
        }
    }

    if (auto *chain_struct = vku::FindStructInPNextChain<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>(in_pnext_chain)) {
        auto &out_chain_struct = std::get<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>(out);
        out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
        if (!chain_begin) {
            chain_begin = &out_chain_struct;
            chain_end = chain_begin;
        } else {
            chain_end = PnextChainAdd(chain_end, &out_chain_struct);
        }
    }

    if (auto *chain_struct = vku::FindStructInPNextChain<VkPhysicalDeviceImageViewImageFormatInfoEXT>(in_pnext_chain)) {
        auto &out_chain_struct = std::get<VkPhysicalDeviceImageViewImageFormatInfoEXT>(out);
        out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
        if (!chain_begin) {
            chain_begin = &out_chain_struct;
            chain_end = chain_begin;
        } else {
            chain_end = PnextChainAdd(chain_end, &out_chain_struct);
        }
    }

    if (auto *chain_struct = vku::FindStructInPNextChain<VkVideoProfileListInfoKHR>(in_pnext_chain)) {
        auto &out_chain_struct = std::get<VkVideoProfileListInfoKHR>(out);
        out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
        if (!chain_begin) {
            chain_begin = &out_chain_struct;
            chain_end = chain_begin;
        } else {
            chain_end = PnextChainAdd(chain_end, &out_chain_struct);
        }
    }

    return chain_begin;
}

}  // namespace vvl

// NOLINTEND
