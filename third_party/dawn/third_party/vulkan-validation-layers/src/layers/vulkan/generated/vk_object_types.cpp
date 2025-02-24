// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See object_types_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google Inc.
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

#include <vulkan/vulkan.h>
#include "vk_object_types.h"

// Helper array to get Vulkan VK_EXT_debug_report object type enum from the internal layers version
static const VkDebugReportObjectTypeEXT kDebugReportLookup[kVulkanObjectTypeMax] = {
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,                     // kVulkanObjectTypeUnknown
    VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,                      // kVulkanObjectTypeBuffer
    VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,                       // kVulkanObjectTypeImage
    VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT,                    // kVulkanObjectTypeInstance
    VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,             // kVulkanObjectTypePhysicalDevice
    VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,                      // kVulkanObjectTypeDevice
    VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT,                       // kVulkanObjectTypeQueue
    VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT,                   // kVulkanObjectTypeSemaphore
    VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,              // kVulkanObjectTypeCommandBuffer
    VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT,                       // kVulkanObjectTypeFence
    VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,               // kVulkanObjectTypeDeviceMemory
    VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT,                       // kVulkanObjectTypeEvent
    VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT,                  // kVulkanObjectTypeQueryPool
    VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT,                 // kVulkanObjectTypeBufferView
    VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT,                  // kVulkanObjectTypeImageView
    VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT,               // kVulkanObjectTypeShaderModule
    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT,              // kVulkanObjectTypePipelineCache
    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT,             // kVulkanObjectTypePipelineLayout
    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,                    // kVulkanObjectTypePipeline
    VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,                 // kVulkanObjectTypeRenderPass
    VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT,       // kVulkanObjectTypeDescriptorSetLayout
    VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT,                     // kVulkanObjectTypeSampler
    VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,              // kVulkanObjectTypeDescriptorSet
    VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT,             // kVulkanObjectTypeDescriptorPool
    VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT,                 // kVulkanObjectTypeFramebuffer
    VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT,                // kVulkanObjectTypeCommandPool
    VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT,    // kVulkanObjectTypeSamplerYcbcrConversion
    VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT,  // kVulkanObjectTypeDescriptorUpdateTemplate
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,                     // kVulkanObjectTypePrivateDataSlot
    VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT,                 // kVulkanObjectTypeSurfaceKHR
    VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,               // kVulkanObjectTypeSwapchainKHR
    VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT,                 // kVulkanObjectTypeDisplayKHR
    VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT,            // kVulkanObjectTypeDisplayModeKHR
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,                     // kVulkanObjectTypeVideoSessionKHR
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,                     // kVulkanObjectTypeVideoSessionParametersKHR
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,                     // kVulkanObjectTypeDeferredOperationKHR
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,                     // kVulkanObjectTypePipelineBinaryKHR
    VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT_EXT,   // kVulkanObjectTypeDebugReportCallbackEXT
    VK_DEBUG_REPORT_OBJECT_TYPE_CU_MODULE_NVX_EXT,               // kVulkanObjectTypeCuModuleNVX
    VK_DEBUG_REPORT_OBJECT_TYPE_CU_FUNCTION_NVX_EXT,             // kVulkanObjectTypeCuFunctionNVX
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,                     // kVulkanObjectTypeDebugUtilsMessengerEXT
    VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT_EXT,        // kVulkanObjectTypeValidationCacheEXT
    VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT,   // kVulkanObjectTypeAccelerationStructureNV
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,                     // kVulkanObjectTypePerformanceConfigurationINTEL
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,                     // kVulkanObjectTypeIndirectCommandsLayoutNV
    VK_DEBUG_REPORT_OBJECT_TYPE_CUDA_MODULE_NV_EXT,              // kVulkanObjectTypeCudaModuleNV
    VK_DEBUG_REPORT_OBJECT_TYPE_CUDA_FUNCTION_NV_EXT,            // kVulkanObjectTypeCudaFunctionNV
    VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR_EXT,  // kVulkanObjectTypeAccelerationStructureKHR
    VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_COLLECTION_FUCHSIA_EXT,   // kVulkanObjectTypeBufferCollectionFUCHSIA
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,                     // kVulkanObjectTypeMicromapEXT
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,                     // kVulkanObjectTypeOpticalFlowSessionNV
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,                     // kVulkanObjectTypeShaderEXT
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,                     // kVulkanObjectTypeIndirectExecutionSetEXT
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,                     // kVulkanObjectTypeIndirectCommandsLayoutEXT
};

// Array of object name strings for OBJECT_TYPE enum conversion
static const char* const kVulkanObjectTypeStrings[kVulkanObjectTypeMax] = {
    "VkNonDispatchableHandle",
    "VkBuffer",
    "VkImage",
    "VkInstance",
    "VkPhysicalDevice",
    "VkDevice",
    "VkQueue",
    "VkSemaphore",
    "VkCommandBuffer",
    "VkFence",
    "VkDeviceMemory",
    "VkEvent",
    "VkQueryPool",
    "VkBufferView",
    "VkImageView",
    "VkShaderModule",
    "VkPipelineCache",
    "VkPipelineLayout",
    "VkPipeline",
    "VkRenderPass",
    "VkDescriptorSetLayout",
    "VkSampler",
    "VkDescriptorSet",
    "VkDescriptorPool",
    "VkFramebuffer",
    "VkCommandPool",
    "VkSamplerYcbcrConversion",
    "VkDescriptorUpdateTemplate",
    "VkPrivateDataSlot",
    "VkSurfaceKHR",
    "VkSwapchainKHR",
    "VkDisplayKHR",
    "VkDisplayModeKHR",
    "VkVideoSessionKHR",
    "VkVideoSessionParametersKHR",
    "VkDeferredOperationKHR",
    "VkPipelineBinaryKHR",
    "VkDebugReportCallbackEXT",
    "VkCuModuleNVX",
    "VkCuFunctionNVX",
    "VkDebugUtilsMessengerEXT",
    "VkValidationCacheEXT",
    "VkAccelerationStructureNV",
    "VkPerformanceConfigurationINTEL",
    "VkIndirectCommandsLayoutNV",
    "VkCudaModuleNV",
    "VkCudaFunctionNV",
    "VkAccelerationStructureKHR",
    "VkBufferCollectionFUCHSIA",
    "VkMicromapEXT",
    "VkOpticalFlowSessionNV",
    "VkShaderEXT",
    "VkIndirectExecutionSetEXT",
    "VkIndirectCommandsLayoutEXT",
};

VkDebugReportObjectTypeEXT GetDebugReport(VulkanObjectType type) { return kDebugReportLookup[type]; }

const char* string_VulkanObjectType(VulkanObjectType type) { return kVulkanObjectTypeStrings[type]; }

// NOLINTEND
