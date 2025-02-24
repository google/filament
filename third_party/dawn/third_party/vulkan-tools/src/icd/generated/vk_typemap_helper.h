// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See vulkan_tools_helper_file_generator.py for modifications


/***************************************************************************
 *
 * Copyright (c) 2015-2017 The Khronos Group Inc.
 * Copyright (c) 2015-2017 Valve Corporation
 * Copyright (c) 2015-2017 LunarG, Inc.
 * Copyright (c) 2015-2017 Google Inc.
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
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisforbes@google.com>
 * Author: John Zulauf<jzulauf@lunarg.com>
 *
 ****************************************************************************/

#pragma once
#include <vulkan/vulkan.h>

// These empty generic templates are specialized for each type with sType
// members and for each sType -- providing a two way map between structure
// types and sTypes

template <VkStructureType id> struct LvlSTypeMap {};
template <typename T> struct LvlTypeMap {};

// Map type VkBufferMemoryBarrier to id VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER
template <> struct LvlTypeMap<VkBufferMemoryBarrier> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER> {
    typedef VkBufferMemoryBarrier Type;
};

// Map type VkImageMemoryBarrier to id VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER
template <> struct LvlTypeMap<VkImageMemoryBarrier> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER> {
    typedef VkImageMemoryBarrier Type;
};

// Map type VkMemoryBarrier to id VK_STRUCTURE_TYPE_MEMORY_BARRIER
template <> struct LvlTypeMap<VkMemoryBarrier> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_BARRIER> {
    typedef VkMemoryBarrier Type;
};

// Map type VkApplicationInfo to id VK_STRUCTURE_TYPE_APPLICATION_INFO
template <> struct LvlTypeMap<VkApplicationInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_APPLICATION_INFO> {
    typedef VkApplicationInfo Type;
};

// Map type VkInstanceCreateInfo to id VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO
template <> struct LvlTypeMap<VkInstanceCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO> {
    typedef VkInstanceCreateInfo Type;
};

// Map type VkDeviceQueueCreateInfo to id VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO
template <> struct LvlTypeMap<VkDeviceQueueCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO> {
    typedef VkDeviceQueueCreateInfo Type;
};

// Map type VkDeviceCreateInfo to id VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO
template <> struct LvlTypeMap<VkDeviceCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO> {
    typedef VkDeviceCreateInfo Type;
};

// Map type VkSubmitInfo to id VK_STRUCTURE_TYPE_SUBMIT_INFO
template <> struct LvlTypeMap<VkSubmitInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SUBMIT_INFO> {
    typedef VkSubmitInfo Type;
};

// Map type VkMappedMemoryRange to id VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE
template <> struct LvlTypeMap<VkMappedMemoryRange> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE> {
    typedef VkMappedMemoryRange Type;
};

// Map type VkMemoryAllocateInfo to id VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO
template <> struct LvlTypeMap<VkMemoryAllocateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO> {
    typedef VkMemoryAllocateInfo Type;
};

// Map type VkBindSparseInfo to id VK_STRUCTURE_TYPE_BIND_SPARSE_INFO
template <> struct LvlTypeMap<VkBindSparseInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BIND_SPARSE_INFO> {
    typedef VkBindSparseInfo Type;
};

// Map type VkFenceCreateInfo to id VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
template <> struct LvlTypeMap<VkFenceCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_FENCE_CREATE_INFO> {
    typedef VkFenceCreateInfo Type;
};

// Map type VkSemaphoreCreateInfo to id VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
template <> struct LvlTypeMap<VkSemaphoreCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO> {
    typedef VkSemaphoreCreateInfo Type;
};

// Map type VkEventCreateInfo to id VK_STRUCTURE_TYPE_EVENT_CREATE_INFO
template <> struct LvlTypeMap<VkEventCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EVENT_CREATE_INFO> {
    typedef VkEventCreateInfo Type;
};

// Map type VkQueryPoolCreateInfo to id VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO
template <> struct LvlTypeMap<VkQueryPoolCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO> {
    typedef VkQueryPoolCreateInfo Type;
};

// Map type VkBufferCreateInfo to id VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO
template <> struct LvlTypeMap<VkBufferCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO> {
    typedef VkBufferCreateInfo Type;
};

// Map type VkBufferViewCreateInfo to id VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO
template <> struct LvlTypeMap<VkBufferViewCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO> {
    typedef VkBufferViewCreateInfo Type;
};

// Map type VkImageCreateInfo to id VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO
template <> struct LvlTypeMap<VkImageCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO> {
    typedef VkImageCreateInfo Type;
};

// Map type VkImageViewCreateInfo to id VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO
template <> struct LvlTypeMap<VkImageViewCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO> {
    typedef VkImageViewCreateInfo Type;
};

// Map type VkShaderModuleCreateInfo to id VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO
template <> struct LvlTypeMap<VkShaderModuleCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO> {
    typedef VkShaderModuleCreateInfo Type;
};

// Map type VkPipelineCacheCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineCacheCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO> {
    typedef VkPipelineCacheCreateInfo Type;
};

// Map type VkPipelineShaderStageCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineShaderStageCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO> {
    typedef VkPipelineShaderStageCreateInfo Type;
};

// Map type VkComputePipelineCreateInfo to id VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO
template <> struct LvlTypeMap<VkComputePipelineCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO> {
    typedef VkComputePipelineCreateInfo Type;
};

// Map type VkPipelineVertexInputStateCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineVertexInputStateCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO> {
    typedef VkPipelineVertexInputStateCreateInfo Type;
};

// Map type VkPipelineInputAssemblyStateCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineInputAssemblyStateCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO> {
    typedef VkPipelineInputAssemblyStateCreateInfo Type;
};

// Map type VkPipelineTessellationStateCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineTessellationStateCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO> {
    typedef VkPipelineTessellationStateCreateInfo Type;
};

// Map type VkPipelineViewportStateCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineViewportStateCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO> {
    typedef VkPipelineViewportStateCreateInfo Type;
};

// Map type VkPipelineRasterizationStateCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineRasterizationStateCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO> {
    typedef VkPipelineRasterizationStateCreateInfo Type;
};

// Map type VkPipelineMultisampleStateCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineMultisampleStateCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO> {
    typedef VkPipelineMultisampleStateCreateInfo Type;
};

// Map type VkPipelineDepthStencilStateCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineDepthStencilStateCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO> {
    typedef VkPipelineDepthStencilStateCreateInfo Type;
};

// Map type VkPipelineColorBlendStateCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineColorBlendStateCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO> {
    typedef VkPipelineColorBlendStateCreateInfo Type;
};

// Map type VkPipelineDynamicStateCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineDynamicStateCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO> {
    typedef VkPipelineDynamicStateCreateInfo Type;
};

// Map type VkGraphicsPipelineCreateInfo to id VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO
template <> struct LvlTypeMap<VkGraphicsPipelineCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO> {
    typedef VkGraphicsPipelineCreateInfo Type;
};

// Map type VkPipelineLayoutCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineLayoutCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO> {
    typedef VkPipelineLayoutCreateInfo Type;
};

// Map type VkSamplerCreateInfo to id VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO
template <> struct LvlTypeMap<VkSamplerCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO> {
    typedef VkSamplerCreateInfo Type;
};

// Map type VkCopyDescriptorSet to id VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET
template <> struct LvlTypeMap<VkCopyDescriptorSet> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET> {
    typedef VkCopyDescriptorSet Type;
};

// Map type VkDescriptorPoolCreateInfo to id VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO
template <> struct LvlTypeMap<VkDescriptorPoolCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO> {
    typedef VkDescriptorPoolCreateInfo Type;
};

// Map type VkDescriptorSetAllocateInfo to id VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO
template <> struct LvlTypeMap<VkDescriptorSetAllocateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO> {
    typedef VkDescriptorSetAllocateInfo Type;
};

// Map type VkDescriptorSetLayoutCreateInfo to id VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO
template <> struct LvlTypeMap<VkDescriptorSetLayoutCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO> {
    typedef VkDescriptorSetLayoutCreateInfo Type;
};

// Map type VkWriteDescriptorSet to id VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
template <> struct LvlTypeMap<VkWriteDescriptorSet> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET> {
    typedef VkWriteDescriptorSet Type;
};

// Map type VkFramebufferCreateInfo to id VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO
template <> struct LvlTypeMap<VkFramebufferCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO> {
    typedef VkFramebufferCreateInfo Type;
};

// Map type VkRenderPassCreateInfo to id VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO
template <> struct LvlTypeMap<VkRenderPassCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO> {
    typedef VkRenderPassCreateInfo Type;
};

// Map type VkCommandPoolCreateInfo to id VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO
template <> struct LvlTypeMap<VkCommandPoolCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO> {
    typedef VkCommandPoolCreateInfo Type;
};

// Map type VkCommandBufferAllocateInfo to id VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO
template <> struct LvlTypeMap<VkCommandBufferAllocateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO> {
    typedef VkCommandBufferAllocateInfo Type;
};

// Map type VkCommandBufferInheritanceInfo to id VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO
template <> struct LvlTypeMap<VkCommandBufferInheritanceInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO> {
    typedef VkCommandBufferInheritanceInfo Type;
};

// Map type VkCommandBufferBeginInfo to id VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
template <> struct LvlTypeMap<VkCommandBufferBeginInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO> {
    typedef VkCommandBufferBeginInfo Type;
};

// Map type VkRenderPassBeginInfo to id VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO
template <> struct LvlTypeMap<VkRenderPassBeginInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO> {
    typedef VkRenderPassBeginInfo Type;
};

// Map type VkPhysicalDeviceSubgroupProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceSubgroupProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES> {
    typedef VkPhysicalDeviceSubgroupProperties Type;
};

// Map type VkBindBufferMemoryInfo to id VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO
template <> struct LvlTypeMap<VkBindBufferMemoryInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO> {
    typedef VkBindBufferMemoryInfo Type;
};

// Map type VkBindImageMemoryInfo to id VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO
template <> struct LvlTypeMap<VkBindImageMemoryInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO> {
    typedef VkBindImageMemoryInfo Type;
};

// Map type VkPhysicalDevice16BitStorageFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES
template <> struct LvlTypeMap<VkPhysicalDevice16BitStorageFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES> {
    typedef VkPhysicalDevice16BitStorageFeatures Type;
};

// Map type VkMemoryDedicatedRequirements to id VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS
template <> struct LvlTypeMap<VkMemoryDedicatedRequirements> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS> {
    typedef VkMemoryDedicatedRequirements Type;
};

// Map type VkMemoryDedicatedAllocateInfo to id VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO
template <> struct LvlTypeMap<VkMemoryDedicatedAllocateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO> {
    typedef VkMemoryDedicatedAllocateInfo Type;
};

// Map type VkMemoryAllocateFlagsInfo to id VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO
template <> struct LvlTypeMap<VkMemoryAllocateFlagsInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO> {
    typedef VkMemoryAllocateFlagsInfo Type;
};

// Map type VkDeviceGroupRenderPassBeginInfo to id VK_STRUCTURE_TYPE_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO
template <> struct LvlTypeMap<VkDeviceGroupRenderPassBeginInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO> {
    typedef VkDeviceGroupRenderPassBeginInfo Type;
};

// Map type VkDeviceGroupCommandBufferBeginInfo to id VK_STRUCTURE_TYPE_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO
template <> struct LvlTypeMap<VkDeviceGroupCommandBufferBeginInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO> {
    typedef VkDeviceGroupCommandBufferBeginInfo Type;
};

// Map type VkDeviceGroupSubmitInfo to id VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO
template <> struct LvlTypeMap<VkDeviceGroupSubmitInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO> {
    typedef VkDeviceGroupSubmitInfo Type;
};

// Map type VkDeviceGroupBindSparseInfo to id VK_STRUCTURE_TYPE_DEVICE_GROUP_BIND_SPARSE_INFO
template <> struct LvlTypeMap<VkDeviceGroupBindSparseInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_GROUP_BIND_SPARSE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_GROUP_BIND_SPARSE_INFO> {
    typedef VkDeviceGroupBindSparseInfo Type;
};

// Map type VkBindBufferMemoryDeviceGroupInfo to id VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO
template <> struct LvlTypeMap<VkBindBufferMemoryDeviceGroupInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO> {
    typedef VkBindBufferMemoryDeviceGroupInfo Type;
};

// Map type VkBindImageMemoryDeviceGroupInfo to id VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_DEVICE_GROUP_INFO
template <> struct LvlTypeMap<VkBindImageMemoryDeviceGroupInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_DEVICE_GROUP_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_DEVICE_GROUP_INFO> {
    typedef VkBindImageMemoryDeviceGroupInfo Type;
};

// Map type VkPhysicalDeviceGroupProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceGroupProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES> {
    typedef VkPhysicalDeviceGroupProperties Type;
};

// Map type VkDeviceGroupDeviceCreateInfo to id VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO
template <> struct LvlTypeMap<VkDeviceGroupDeviceCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO> {
    typedef VkDeviceGroupDeviceCreateInfo Type;
};

// Map type VkBufferMemoryRequirementsInfo2 to id VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2
template <> struct LvlTypeMap<VkBufferMemoryRequirementsInfo2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2> {
    typedef VkBufferMemoryRequirementsInfo2 Type;
};

// Map type VkImageMemoryRequirementsInfo2 to id VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2
template <> struct LvlTypeMap<VkImageMemoryRequirementsInfo2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2> {
    typedef VkImageMemoryRequirementsInfo2 Type;
};

// Map type VkImageSparseMemoryRequirementsInfo2 to id VK_STRUCTURE_TYPE_IMAGE_SPARSE_MEMORY_REQUIREMENTS_INFO_2
template <> struct LvlTypeMap<VkImageSparseMemoryRequirementsInfo2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_SPARSE_MEMORY_REQUIREMENTS_INFO_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_SPARSE_MEMORY_REQUIREMENTS_INFO_2> {
    typedef VkImageSparseMemoryRequirementsInfo2 Type;
};

// Map type VkMemoryRequirements2 to id VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2
template <> struct LvlTypeMap<VkMemoryRequirements2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2> {
    typedef VkMemoryRequirements2 Type;
};

// Map type VkSparseImageMemoryRequirements2 to id VK_STRUCTURE_TYPE_SPARSE_IMAGE_MEMORY_REQUIREMENTS_2
template <> struct LvlTypeMap<VkSparseImageMemoryRequirements2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SPARSE_IMAGE_MEMORY_REQUIREMENTS_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SPARSE_IMAGE_MEMORY_REQUIREMENTS_2> {
    typedef VkSparseImageMemoryRequirements2 Type;
};

// Map type VkPhysicalDeviceFeatures2 to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2
template <> struct LvlTypeMap<VkPhysicalDeviceFeatures2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2> {
    typedef VkPhysicalDeviceFeatures2 Type;
};

// Map type VkPhysicalDeviceProperties2 to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2
template <> struct LvlTypeMap<VkPhysicalDeviceProperties2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2> {
    typedef VkPhysicalDeviceProperties2 Type;
};

// Map type VkFormatProperties2 to id VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2
template <> struct LvlTypeMap<VkFormatProperties2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2> {
    typedef VkFormatProperties2 Type;
};

// Map type VkImageFormatProperties2 to id VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2
template <> struct LvlTypeMap<VkImageFormatProperties2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2> {
    typedef VkImageFormatProperties2 Type;
};

// Map type VkPhysicalDeviceImageFormatInfo2 to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2
template <> struct LvlTypeMap<VkPhysicalDeviceImageFormatInfo2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2> {
    typedef VkPhysicalDeviceImageFormatInfo2 Type;
};

// Map type VkQueueFamilyProperties2 to id VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2
template <> struct LvlTypeMap<VkQueueFamilyProperties2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2> {
    typedef VkQueueFamilyProperties2 Type;
};

// Map type VkPhysicalDeviceMemoryProperties2 to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2
template <> struct LvlTypeMap<VkPhysicalDeviceMemoryProperties2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2> {
    typedef VkPhysicalDeviceMemoryProperties2 Type;
};

// Map type VkSparseImageFormatProperties2 to id VK_STRUCTURE_TYPE_SPARSE_IMAGE_FORMAT_PROPERTIES_2
template <> struct LvlTypeMap<VkSparseImageFormatProperties2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SPARSE_IMAGE_FORMAT_PROPERTIES_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SPARSE_IMAGE_FORMAT_PROPERTIES_2> {
    typedef VkSparseImageFormatProperties2 Type;
};

// Map type VkPhysicalDeviceSparseImageFormatInfo2 to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_INFO_2
template <> struct LvlTypeMap<VkPhysicalDeviceSparseImageFormatInfo2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_INFO_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_INFO_2> {
    typedef VkPhysicalDeviceSparseImageFormatInfo2 Type;
};

// Map type VkPhysicalDevicePointClippingProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDevicePointClippingProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES> {
    typedef VkPhysicalDevicePointClippingProperties Type;
};

// Map type VkRenderPassInputAttachmentAspectCreateInfo to id VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO
template <> struct LvlTypeMap<VkRenderPassInputAttachmentAspectCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO> {
    typedef VkRenderPassInputAttachmentAspectCreateInfo Type;
};

// Map type VkImageViewUsageCreateInfo to id VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO
template <> struct LvlTypeMap<VkImageViewUsageCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO> {
    typedef VkImageViewUsageCreateInfo Type;
};

// Map type VkPipelineTessellationDomainOriginStateCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineTessellationDomainOriginStateCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO> {
    typedef VkPipelineTessellationDomainOriginStateCreateInfo Type;
};

// Map type VkRenderPassMultiviewCreateInfo to id VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO
template <> struct LvlTypeMap<VkRenderPassMultiviewCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO> {
    typedef VkRenderPassMultiviewCreateInfo Type;
};

// Map type VkPhysicalDeviceMultiviewFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceMultiviewFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES> {
    typedef VkPhysicalDeviceMultiviewFeatures Type;
};

// Map type VkPhysicalDeviceMultiviewProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceMultiviewProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES> {
    typedef VkPhysicalDeviceMultiviewProperties Type;
};

// Map type VkPhysicalDeviceVariablePointersFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceVariablePointersFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES> {
    typedef VkPhysicalDeviceVariablePointersFeatures Type;
};

// Map type VkPhysicalDeviceProtectedMemoryFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceProtectedMemoryFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES> {
    typedef VkPhysicalDeviceProtectedMemoryFeatures Type;
};

// Map type VkPhysicalDeviceProtectedMemoryProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceProtectedMemoryProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES> {
    typedef VkPhysicalDeviceProtectedMemoryProperties Type;
};

// Map type VkDeviceQueueInfo2 to id VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2
template <> struct LvlTypeMap<VkDeviceQueueInfo2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2> {
    typedef VkDeviceQueueInfo2 Type;
};

// Map type VkProtectedSubmitInfo to id VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO
template <> struct LvlTypeMap<VkProtectedSubmitInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO> {
    typedef VkProtectedSubmitInfo Type;
};

// Map type VkSamplerYcbcrConversionCreateInfo to id VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO
template <> struct LvlTypeMap<VkSamplerYcbcrConversionCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO> {
    typedef VkSamplerYcbcrConversionCreateInfo Type;
};

// Map type VkSamplerYcbcrConversionInfo to id VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO
template <> struct LvlTypeMap<VkSamplerYcbcrConversionInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO> {
    typedef VkSamplerYcbcrConversionInfo Type;
};

// Map type VkBindImagePlaneMemoryInfo to id VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO
template <> struct LvlTypeMap<VkBindImagePlaneMemoryInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO> {
    typedef VkBindImagePlaneMemoryInfo Type;
};

// Map type VkImagePlaneMemoryRequirementsInfo to id VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO
template <> struct LvlTypeMap<VkImagePlaneMemoryRequirementsInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO> {
    typedef VkImagePlaneMemoryRequirementsInfo Type;
};

// Map type VkPhysicalDeviceSamplerYcbcrConversionFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceSamplerYcbcrConversionFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES> {
    typedef VkPhysicalDeviceSamplerYcbcrConversionFeatures Type;
};

// Map type VkSamplerYcbcrConversionImageFormatProperties to id VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_IMAGE_FORMAT_PROPERTIES
template <> struct LvlTypeMap<VkSamplerYcbcrConversionImageFormatProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_IMAGE_FORMAT_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_IMAGE_FORMAT_PROPERTIES> {
    typedef VkSamplerYcbcrConversionImageFormatProperties Type;
};

// Map type VkDescriptorUpdateTemplateCreateInfo to id VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO
template <> struct LvlTypeMap<VkDescriptorUpdateTemplateCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO> {
    typedef VkDescriptorUpdateTemplateCreateInfo Type;
};

// Map type VkPhysicalDeviceExternalImageFormatInfo to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO
template <> struct LvlTypeMap<VkPhysicalDeviceExternalImageFormatInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO> {
    typedef VkPhysicalDeviceExternalImageFormatInfo Type;
};

// Map type VkExternalImageFormatProperties to id VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES
template <> struct LvlTypeMap<VkExternalImageFormatProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES> {
    typedef VkExternalImageFormatProperties Type;
};

// Map type VkPhysicalDeviceExternalBufferInfo to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO
template <> struct LvlTypeMap<VkPhysicalDeviceExternalBufferInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO> {
    typedef VkPhysicalDeviceExternalBufferInfo Type;
};

// Map type VkExternalBufferProperties to id VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES
template <> struct LvlTypeMap<VkExternalBufferProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES> {
    typedef VkExternalBufferProperties Type;
};

// Map type VkPhysicalDeviceIDProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceIDProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES> {
    typedef VkPhysicalDeviceIDProperties Type;
};

// Map type VkExternalMemoryImageCreateInfo to id VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO
template <> struct LvlTypeMap<VkExternalMemoryImageCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO> {
    typedef VkExternalMemoryImageCreateInfo Type;
};

// Map type VkExternalMemoryBufferCreateInfo to id VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO
template <> struct LvlTypeMap<VkExternalMemoryBufferCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO> {
    typedef VkExternalMemoryBufferCreateInfo Type;
};

// Map type VkExportMemoryAllocateInfo to id VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO
template <> struct LvlTypeMap<VkExportMemoryAllocateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO> {
    typedef VkExportMemoryAllocateInfo Type;
};

// Map type VkPhysicalDeviceExternalFenceInfo to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO
template <> struct LvlTypeMap<VkPhysicalDeviceExternalFenceInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO> {
    typedef VkPhysicalDeviceExternalFenceInfo Type;
};

// Map type VkExternalFenceProperties to id VK_STRUCTURE_TYPE_EXTERNAL_FENCE_PROPERTIES
template <> struct LvlTypeMap<VkExternalFenceProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXTERNAL_FENCE_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXTERNAL_FENCE_PROPERTIES> {
    typedef VkExternalFenceProperties Type;
};

// Map type VkExportFenceCreateInfo to id VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO
template <> struct LvlTypeMap<VkExportFenceCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO> {
    typedef VkExportFenceCreateInfo Type;
};

// Map type VkExportSemaphoreCreateInfo to id VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO
template <> struct LvlTypeMap<VkExportSemaphoreCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO> {
    typedef VkExportSemaphoreCreateInfo Type;
};

// Map type VkPhysicalDeviceExternalSemaphoreInfo to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO
template <> struct LvlTypeMap<VkPhysicalDeviceExternalSemaphoreInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO> {
    typedef VkPhysicalDeviceExternalSemaphoreInfo Type;
};

// Map type VkExternalSemaphoreProperties to id VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES
template <> struct LvlTypeMap<VkExternalSemaphoreProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES> {
    typedef VkExternalSemaphoreProperties Type;
};

// Map type VkPhysicalDeviceMaintenance3Properties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceMaintenance3Properties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES> {
    typedef VkPhysicalDeviceMaintenance3Properties Type;
};

// Map type VkDescriptorSetLayoutSupport to id VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_SUPPORT
template <> struct LvlTypeMap<VkDescriptorSetLayoutSupport> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_SUPPORT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_SUPPORT> {
    typedef VkDescriptorSetLayoutSupport Type;
};

// Map type VkPhysicalDeviceShaderDrawParametersFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceShaderDrawParametersFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES> {
    typedef VkPhysicalDeviceShaderDrawParametersFeatures Type;
};

// Map type VkPhysicalDeviceVulkan11Features to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceVulkan11Features> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES> {
    typedef VkPhysicalDeviceVulkan11Features Type;
};

// Map type VkPhysicalDeviceVulkan11Properties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceVulkan11Properties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES> {
    typedef VkPhysicalDeviceVulkan11Properties Type;
};

// Map type VkPhysicalDeviceVulkan12Features to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceVulkan12Features> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES> {
    typedef VkPhysicalDeviceVulkan12Features Type;
};

// Map type VkPhysicalDeviceVulkan12Properties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceVulkan12Properties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES> {
    typedef VkPhysicalDeviceVulkan12Properties Type;
};

// Map type VkImageFormatListCreateInfo to id VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO
template <> struct LvlTypeMap<VkImageFormatListCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO> {
    typedef VkImageFormatListCreateInfo Type;
};

// Map type VkAttachmentDescription2 to id VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2
template <> struct LvlTypeMap<VkAttachmentDescription2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2> {
    typedef VkAttachmentDescription2 Type;
};

// Map type VkAttachmentReference2 to id VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2
template <> struct LvlTypeMap<VkAttachmentReference2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2> {
    typedef VkAttachmentReference2 Type;
};

// Map type VkSubpassDescription2 to id VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2
template <> struct LvlTypeMap<VkSubpassDescription2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2> {
    typedef VkSubpassDescription2 Type;
};

// Map type VkSubpassDependency2 to id VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2
template <> struct LvlTypeMap<VkSubpassDependency2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2> {
    typedef VkSubpassDependency2 Type;
};

// Map type VkRenderPassCreateInfo2 to id VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2
template <> struct LvlTypeMap<VkRenderPassCreateInfo2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2> {
    typedef VkRenderPassCreateInfo2 Type;
};

// Map type VkSubpassBeginInfo to id VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO
template <> struct LvlTypeMap<VkSubpassBeginInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO> {
    typedef VkSubpassBeginInfo Type;
};

// Map type VkSubpassEndInfo to id VK_STRUCTURE_TYPE_SUBPASS_END_INFO
template <> struct LvlTypeMap<VkSubpassEndInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SUBPASS_END_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SUBPASS_END_INFO> {
    typedef VkSubpassEndInfo Type;
};

// Map type VkPhysicalDevice8BitStorageFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES
template <> struct LvlTypeMap<VkPhysicalDevice8BitStorageFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES> {
    typedef VkPhysicalDevice8BitStorageFeatures Type;
};

// Map type VkPhysicalDeviceDriverProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceDriverProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES> {
    typedef VkPhysicalDeviceDriverProperties Type;
};

// Map type VkPhysicalDeviceShaderAtomicInt64Features to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceShaderAtomicInt64Features> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES> {
    typedef VkPhysicalDeviceShaderAtomicInt64Features Type;
};

// Map type VkPhysicalDeviceShaderFloat16Int8Features to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceShaderFloat16Int8Features> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES> {
    typedef VkPhysicalDeviceShaderFloat16Int8Features Type;
};

// Map type VkPhysicalDeviceFloatControlsProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceFloatControlsProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES> {
    typedef VkPhysicalDeviceFloatControlsProperties Type;
};

// Map type VkDescriptorSetLayoutBindingFlagsCreateInfo to id VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO
template <> struct LvlTypeMap<VkDescriptorSetLayoutBindingFlagsCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO> {
    typedef VkDescriptorSetLayoutBindingFlagsCreateInfo Type;
};

// Map type VkPhysicalDeviceDescriptorIndexingFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceDescriptorIndexingFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES> {
    typedef VkPhysicalDeviceDescriptorIndexingFeatures Type;
};

// Map type VkPhysicalDeviceDescriptorIndexingProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceDescriptorIndexingProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES> {
    typedef VkPhysicalDeviceDescriptorIndexingProperties Type;
};

// Map type VkDescriptorSetVariableDescriptorCountAllocateInfo to id VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO
template <> struct LvlTypeMap<VkDescriptorSetVariableDescriptorCountAllocateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO> {
    typedef VkDescriptorSetVariableDescriptorCountAllocateInfo Type;
};

// Map type VkDescriptorSetVariableDescriptorCountLayoutSupport to id VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_LAYOUT_SUPPORT
template <> struct LvlTypeMap<VkDescriptorSetVariableDescriptorCountLayoutSupport> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_LAYOUT_SUPPORT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_LAYOUT_SUPPORT> {
    typedef VkDescriptorSetVariableDescriptorCountLayoutSupport Type;
};

// Map type VkSubpassDescriptionDepthStencilResolve to id VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE
template <> struct LvlTypeMap<VkSubpassDescriptionDepthStencilResolve> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE> {
    typedef VkSubpassDescriptionDepthStencilResolve Type;
};

// Map type VkPhysicalDeviceDepthStencilResolveProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceDepthStencilResolveProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES> {
    typedef VkPhysicalDeviceDepthStencilResolveProperties Type;
};

// Map type VkPhysicalDeviceScalarBlockLayoutFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceScalarBlockLayoutFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES> {
    typedef VkPhysicalDeviceScalarBlockLayoutFeatures Type;
};

// Map type VkImageStencilUsageCreateInfo to id VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO
template <> struct LvlTypeMap<VkImageStencilUsageCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO> {
    typedef VkImageStencilUsageCreateInfo Type;
};

// Map type VkSamplerReductionModeCreateInfo to id VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO
template <> struct LvlTypeMap<VkSamplerReductionModeCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO> {
    typedef VkSamplerReductionModeCreateInfo Type;
};

// Map type VkPhysicalDeviceSamplerFilterMinmaxProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceSamplerFilterMinmaxProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES> {
    typedef VkPhysicalDeviceSamplerFilterMinmaxProperties Type;
};

// Map type VkPhysicalDeviceVulkanMemoryModelFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceVulkanMemoryModelFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES> {
    typedef VkPhysicalDeviceVulkanMemoryModelFeatures Type;
};

// Map type VkPhysicalDeviceImagelessFramebufferFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceImagelessFramebufferFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES> {
    typedef VkPhysicalDeviceImagelessFramebufferFeatures Type;
};

// Map type VkFramebufferAttachmentImageInfo to id VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO
template <> struct LvlTypeMap<VkFramebufferAttachmentImageInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO> {
    typedef VkFramebufferAttachmentImageInfo Type;
};

// Map type VkFramebufferAttachmentsCreateInfo to id VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO
template <> struct LvlTypeMap<VkFramebufferAttachmentsCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO> {
    typedef VkFramebufferAttachmentsCreateInfo Type;
};

// Map type VkRenderPassAttachmentBeginInfo to id VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO
template <> struct LvlTypeMap<VkRenderPassAttachmentBeginInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO> {
    typedef VkRenderPassAttachmentBeginInfo Type;
};

// Map type VkPhysicalDeviceUniformBufferStandardLayoutFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceUniformBufferStandardLayoutFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES> {
    typedef VkPhysicalDeviceUniformBufferStandardLayoutFeatures Type;
};

// Map type VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES> {
    typedef VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures Type;
};

// Map type VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES> {
    typedef VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures Type;
};

// Map type VkAttachmentReferenceStencilLayout to id VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_STENCIL_LAYOUT
template <> struct LvlTypeMap<VkAttachmentReferenceStencilLayout> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_STENCIL_LAYOUT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_STENCIL_LAYOUT> {
    typedef VkAttachmentReferenceStencilLayout Type;
};

// Map type VkAttachmentDescriptionStencilLayout to id VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_STENCIL_LAYOUT
template <> struct LvlTypeMap<VkAttachmentDescriptionStencilLayout> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_STENCIL_LAYOUT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_STENCIL_LAYOUT> {
    typedef VkAttachmentDescriptionStencilLayout Type;
};

// Map type VkPhysicalDeviceHostQueryResetFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceHostQueryResetFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES> {
    typedef VkPhysicalDeviceHostQueryResetFeatures Type;
};

// Map type VkPhysicalDeviceTimelineSemaphoreFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceTimelineSemaphoreFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES> {
    typedef VkPhysicalDeviceTimelineSemaphoreFeatures Type;
};

// Map type VkPhysicalDeviceTimelineSemaphoreProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceTimelineSemaphoreProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES> {
    typedef VkPhysicalDeviceTimelineSemaphoreProperties Type;
};

// Map type VkSemaphoreTypeCreateInfo to id VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO
template <> struct LvlTypeMap<VkSemaphoreTypeCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO> {
    typedef VkSemaphoreTypeCreateInfo Type;
};

// Map type VkTimelineSemaphoreSubmitInfo to id VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO
template <> struct LvlTypeMap<VkTimelineSemaphoreSubmitInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO> {
    typedef VkTimelineSemaphoreSubmitInfo Type;
};

// Map type VkSemaphoreWaitInfo to id VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO
template <> struct LvlTypeMap<VkSemaphoreWaitInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO> {
    typedef VkSemaphoreWaitInfo Type;
};

// Map type VkSemaphoreSignalInfo to id VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO
template <> struct LvlTypeMap<VkSemaphoreSignalInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO> {
    typedef VkSemaphoreSignalInfo Type;
};

// Map type VkPhysicalDeviceBufferDeviceAddressFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceBufferDeviceAddressFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES> {
    typedef VkPhysicalDeviceBufferDeviceAddressFeatures Type;
};

// Map type VkBufferDeviceAddressInfo to id VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO
template <> struct LvlTypeMap<VkBufferDeviceAddressInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO> {
    typedef VkBufferDeviceAddressInfo Type;
};

// Map type VkBufferOpaqueCaptureAddressCreateInfo to id VK_STRUCTURE_TYPE_BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO
template <> struct LvlTypeMap<VkBufferOpaqueCaptureAddressCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO> {
    typedef VkBufferOpaqueCaptureAddressCreateInfo Type;
};

// Map type VkMemoryOpaqueCaptureAddressAllocateInfo to id VK_STRUCTURE_TYPE_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO
template <> struct LvlTypeMap<VkMemoryOpaqueCaptureAddressAllocateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO> {
    typedef VkMemoryOpaqueCaptureAddressAllocateInfo Type;
};

// Map type VkDeviceMemoryOpaqueCaptureAddressInfo to id VK_STRUCTURE_TYPE_DEVICE_MEMORY_OPAQUE_CAPTURE_ADDRESS_INFO
template <> struct LvlTypeMap<VkDeviceMemoryOpaqueCaptureAddressInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_MEMORY_OPAQUE_CAPTURE_ADDRESS_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_MEMORY_OPAQUE_CAPTURE_ADDRESS_INFO> {
    typedef VkDeviceMemoryOpaqueCaptureAddressInfo Type;
};

// Map type VkPhysicalDeviceVulkan13Features to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceVulkan13Features> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES> {
    typedef VkPhysicalDeviceVulkan13Features Type;
};

// Map type VkPhysicalDeviceVulkan13Properties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceVulkan13Properties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES> {
    typedef VkPhysicalDeviceVulkan13Properties Type;
};

// Map type VkPipelineCreationFeedbackCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_CREATION_FEEDBACK_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineCreationFeedbackCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_CREATION_FEEDBACK_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_CREATION_FEEDBACK_CREATE_INFO> {
    typedef VkPipelineCreationFeedbackCreateInfo Type;
};

// Map type VkPhysicalDeviceShaderTerminateInvocationFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceShaderTerminateInvocationFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES> {
    typedef VkPhysicalDeviceShaderTerminateInvocationFeatures Type;
};

// Map type VkPhysicalDeviceToolProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TOOL_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceToolProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TOOL_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TOOL_PROPERTIES> {
    typedef VkPhysicalDeviceToolProperties Type;
};

// Map type VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES> {
    typedef VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures Type;
};

// Map type VkPhysicalDevicePrivateDataFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES
template <> struct LvlTypeMap<VkPhysicalDevicePrivateDataFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES> {
    typedef VkPhysicalDevicePrivateDataFeatures Type;
};

// Map type VkDevicePrivateDataCreateInfo to id VK_STRUCTURE_TYPE_DEVICE_PRIVATE_DATA_CREATE_INFO
template <> struct LvlTypeMap<VkDevicePrivateDataCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_PRIVATE_DATA_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_PRIVATE_DATA_CREATE_INFO> {
    typedef VkDevicePrivateDataCreateInfo Type;
};

// Map type VkPrivateDataSlotCreateInfo to id VK_STRUCTURE_TYPE_PRIVATE_DATA_SLOT_CREATE_INFO
template <> struct LvlTypeMap<VkPrivateDataSlotCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PRIVATE_DATA_SLOT_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PRIVATE_DATA_SLOT_CREATE_INFO> {
    typedef VkPrivateDataSlotCreateInfo Type;
};

// Map type VkPhysicalDevicePipelineCreationCacheControlFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES
template <> struct LvlTypeMap<VkPhysicalDevicePipelineCreationCacheControlFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES> {
    typedef VkPhysicalDevicePipelineCreationCacheControlFeatures Type;
};

// Map type VkMemoryBarrier2 to id VK_STRUCTURE_TYPE_MEMORY_BARRIER_2
template <> struct LvlTypeMap<VkMemoryBarrier2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_BARRIER_2> {
    typedef VkMemoryBarrier2 Type;
};

// Map type VkBufferMemoryBarrier2 to id VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2
template <> struct LvlTypeMap<VkBufferMemoryBarrier2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2> {
    typedef VkBufferMemoryBarrier2 Type;
};

// Map type VkImageMemoryBarrier2 to id VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2
template <> struct LvlTypeMap<VkImageMemoryBarrier2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2> {
    typedef VkImageMemoryBarrier2 Type;
};

// Map type VkDependencyInfo to id VK_STRUCTURE_TYPE_DEPENDENCY_INFO
template <> struct LvlTypeMap<VkDependencyInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEPENDENCY_INFO> {
    typedef VkDependencyInfo Type;
};

// Map type VkSemaphoreSubmitInfo to id VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO
template <> struct LvlTypeMap<VkSemaphoreSubmitInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO> {
    typedef VkSemaphoreSubmitInfo Type;
};

// Map type VkCommandBufferSubmitInfo to id VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO
template <> struct LvlTypeMap<VkCommandBufferSubmitInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO> {
    typedef VkCommandBufferSubmitInfo Type;
};

// Map type VkSubmitInfo2 to id VK_STRUCTURE_TYPE_SUBMIT_INFO_2
template <> struct LvlTypeMap<VkSubmitInfo2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SUBMIT_INFO_2> {
    typedef VkSubmitInfo2 Type;
};

// Map type VkPhysicalDeviceSynchronization2Features to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceSynchronization2Features> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES> {
    typedef VkPhysicalDeviceSynchronization2Features Type;
};

// Map type VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES> {
    typedef VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures Type;
};

// Map type VkPhysicalDeviceImageRobustnessFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceImageRobustnessFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES> {
    typedef VkPhysicalDeviceImageRobustnessFeatures Type;
};

// Map type VkBufferCopy2 to id VK_STRUCTURE_TYPE_BUFFER_COPY_2
template <> struct LvlTypeMap<VkBufferCopy2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_COPY_2> {
    typedef VkBufferCopy2 Type;
};

// Map type VkCopyBufferInfo2 to id VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2
template <> struct LvlTypeMap<VkCopyBufferInfo2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2> {
    typedef VkCopyBufferInfo2 Type;
};

// Map type VkImageCopy2 to id VK_STRUCTURE_TYPE_IMAGE_COPY_2
template <> struct LvlTypeMap<VkImageCopy2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_COPY_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_COPY_2> {
    typedef VkImageCopy2 Type;
};

// Map type VkCopyImageInfo2 to id VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2
template <> struct LvlTypeMap<VkCopyImageInfo2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2> {
    typedef VkCopyImageInfo2 Type;
};

// Map type VkBufferImageCopy2 to id VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2
template <> struct LvlTypeMap<VkBufferImageCopy2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2> {
    typedef VkBufferImageCopy2 Type;
};

// Map type VkCopyBufferToImageInfo2 to id VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2
template <> struct LvlTypeMap<VkCopyBufferToImageInfo2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2> {
    typedef VkCopyBufferToImageInfo2 Type;
};

// Map type VkCopyImageToBufferInfo2 to id VK_STRUCTURE_TYPE_COPY_IMAGE_TO_BUFFER_INFO_2
template <> struct LvlTypeMap<VkCopyImageToBufferInfo2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COPY_IMAGE_TO_BUFFER_INFO_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COPY_IMAGE_TO_BUFFER_INFO_2> {
    typedef VkCopyImageToBufferInfo2 Type;
};

// Map type VkImageBlit2 to id VK_STRUCTURE_TYPE_IMAGE_BLIT_2
template <> struct LvlTypeMap<VkImageBlit2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_BLIT_2> {
    typedef VkImageBlit2 Type;
};

// Map type VkBlitImageInfo2 to id VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2
template <> struct LvlTypeMap<VkBlitImageInfo2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2> {
    typedef VkBlitImageInfo2 Type;
};

// Map type VkImageResolve2 to id VK_STRUCTURE_TYPE_IMAGE_RESOLVE_2
template <> struct LvlTypeMap<VkImageResolve2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_RESOLVE_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_RESOLVE_2> {
    typedef VkImageResolve2 Type;
};

// Map type VkResolveImageInfo2 to id VK_STRUCTURE_TYPE_RESOLVE_IMAGE_INFO_2
template <> struct LvlTypeMap<VkResolveImageInfo2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RESOLVE_IMAGE_INFO_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RESOLVE_IMAGE_INFO_2> {
    typedef VkResolveImageInfo2 Type;
};

// Map type VkPhysicalDeviceSubgroupSizeControlFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceSubgroupSizeControlFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES> {
    typedef VkPhysicalDeviceSubgroupSizeControlFeatures Type;
};

// Map type VkPhysicalDeviceSubgroupSizeControlProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceSubgroupSizeControlProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES> {
    typedef VkPhysicalDeviceSubgroupSizeControlProperties Type;
};

// Map type VkPipelineShaderStageRequiredSubgroupSizeCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineShaderStageRequiredSubgroupSizeCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO> {
    typedef VkPipelineShaderStageRequiredSubgroupSizeCreateInfo Type;
};

// Map type VkPhysicalDeviceInlineUniformBlockFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceInlineUniformBlockFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES> {
    typedef VkPhysicalDeviceInlineUniformBlockFeatures Type;
};

// Map type VkPhysicalDeviceInlineUniformBlockProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceInlineUniformBlockProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_PROPERTIES> {
    typedef VkPhysicalDeviceInlineUniformBlockProperties Type;
};

// Map type VkWriteDescriptorSetInlineUniformBlock to id VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK
template <> struct LvlTypeMap<VkWriteDescriptorSetInlineUniformBlock> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK> {
    typedef VkWriteDescriptorSetInlineUniformBlock Type;
};

// Map type VkDescriptorPoolInlineUniformBlockCreateInfo to id VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_INLINE_UNIFORM_BLOCK_CREATE_INFO
template <> struct LvlTypeMap<VkDescriptorPoolInlineUniformBlockCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_INLINE_UNIFORM_BLOCK_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_INLINE_UNIFORM_BLOCK_CREATE_INFO> {
    typedef VkDescriptorPoolInlineUniformBlockCreateInfo Type;
};

// Map type VkPhysicalDeviceTextureCompressionASTCHDRFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceTextureCompressionASTCHDRFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES> {
    typedef VkPhysicalDeviceTextureCompressionASTCHDRFeatures Type;
};

// Map type VkRenderingAttachmentInfo to id VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO
template <> struct LvlTypeMap<VkRenderingAttachmentInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO> {
    typedef VkRenderingAttachmentInfo Type;
};

// Map type VkRenderingInfo to id VK_STRUCTURE_TYPE_RENDERING_INFO
template <> struct LvlTypeMap<VkRenderingInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDERING_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDERING_INFO> {
    typedef VkRenderingInfo Type;
};

// Map type VkPipelineRenderingCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineRenderingCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO> {
    typedef VkPipelineRenderingCreateInfo Type;
};

// Map type VkPhysicalDeviceDynamicRenderingFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceDynamicRenderingFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES> {
    typedef VkPhysicalDeviceDynamicRenderingFeatures Type;
};

// Map type VkCommandBufferInheritanceRenderingInfo to id VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO
template <> struct LvlTypeMap<VkCommandBufferInheritanceRenderingInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO> {
    typedef VkCommandBufferInheritanceRenderingInfo Type;
};

// Map type VkPhysicalDeviceShaderIntegerDotProductFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceShaderIntegerDotProductFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES> {
    typedef VkPhysicalDeviceShaderIntegerDotProductFeatures Type;
};

// Map type VkPhysicalDeviceShaderIntegerDotProductProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceShaderIntegerDotProductProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_PROPERTIES> {
    typedef VkPhysicalDeviceShaderIntegerDotProductProperties Type;
};

// Map type VkPhysicalDeviceTexelBufferAlignmentProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceTexelBufferAlignmentProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_PROPERTIES> {
    typedef VkPhysicalDeviceTexelBufferAlignmentProperties Type;
};

// Map type VkFormatProperties3 to id VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_3
template <> struct LvlTypeMap<VkFormatProperties3> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_3;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_3> {
    typedef VkFormatProperties3 Type;
};

// Map type VkPhysicalDeviceMaintenance4Features to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceMaintenance4Features> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES> {
    typedef VkPhysicalDeviceMaintenance4Features Type;
};

// Map type VkPhysicalDeviceMaintenance4Properties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceMaintenance4Properties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES> {
    typedef VkPhysicalDeviceMaintenance4Properties Type;
};

// Map type VkDeviceBufferMemoryRequirements to id VK_STRUCTURE_TYPE_DEVICE_BUFFER_MEMORY_REQUIREMENTS
template <> struct LvlTypeMap<VkDeviceBufferMemoryRequirements> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_BUFFER_MEMORY_REQUIREMENTS;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_BUFFER_MEMORY_REQUIREMENTS> {
    typedef VkDeviceBufferMemoryRequirements Type;
};

// Map type VkDeviceImageMemoryRequirements to id VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS
template <> struct LvlTypeMap<VkDeviceImageMemoryRequirements> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS> {
    typedef VkDeviceImageMemoryRequirements Type;
};

// Map type VkPhysicalDeviceVulkan14Features to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceVulkan14Features> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES> {
    typedef VkPhysicalDeviceVulkan14Features Type;
};

// Map type VkPhysicalDeviceVulkan14Properties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceVulkan14Properties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_PROPERTIES> {
    typedef VkPhysicalDeviceVulkan14Properties Type;
};

// Map type VkDeviceQueueGlobalPriorityCreateInfo to id VK_STRUCTURE_TYPE_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO
template <> struct LvlTypeMap<VkDeviceQueueGlobalPriorityCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO> {
    typedef VkDeviceQueueGlobalPriorityCreateInfo Type;
};

// Map type VkPhysicalDeviceGlobalPriorityQueryFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GLOBAL_PRIORITY_QUERY_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceGlobalPriorityQueryFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GLOBAL_PRIORITY_QUERY_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GLOBAL_PRIORITY_QUERY_FEATURES> {
    typedef VkPhysicalDeviceGlobalPriorityQueryFeatures Type;
};

// Map type VkQueueFamilyGlobalPriorityProperties to id VK_STRUCTURE_TYPE_QUEUE_FAMILY_GLOBAL_PRIORITY_PROPERTIES
template <> struct LvlTypeMap<VkQueueFamilyGlobalPriorityProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_GLOBAL_PRIORITY_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_QUEUE_FAMILY_GLOBAL_PRIORITY_PROPERTIES> {
    typedef VkQueueFamilyGlobalPriorityProperties Type;
};

// Map type VkPhysicalDeviceShaderSubgroupRotateFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_ROTATE_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceShaderSubgroupRotateFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_ROTATE_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_ROTATE_FEATURES> {
    typedef VkPhysicalDeviceShaderSubgroupRotateFeatures Type;
};

// Map type VkPhysicalDeviceShaderFloatControls2Features to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT_CONTROLS_2_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceShaderFloatControls2Features> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT_CONTROLS_2_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT_CONTROLS_2_FEATURES> {
    typedef VkPhysicalDeviceShaderFloatControls2Features Type;
};

// Map type VkPhysicalDeviceShaderExpectAssumeFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EXPECT_ASSUME_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceShaderExpectAssumeFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EXPECT_ASSUME_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EXPECT_ASSUME_FEATURES> {
    typedef VkPhysicalDeviceShaderExpectAssumeFeatures Type;
};

// Map type VkPhysicalDeviceLineRasterizationFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceLineRasterizationFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES> {
    typedef VkPhysicalDeviceLineRasterizationFeatures Type;
};

// Map type VkPhysicalDeviceLineRasterizationProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceLineRasterizationProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_PROPERTIES> {
    typedef VkPhysicalDeviceLineRasterizationProperties Type;
};

// Map type VkPipelineRasterizationLineStateCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineRasterizationLineStateCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO> {
    typedef VkPipelineRasterizationLineStateCreateInfo Type;
};

// Map type VkPhysicalDeviceVertexAttributeDivisorProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceVertexAttributeDivisorProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES> {
    typedef VkPhysicalDeviceVertexAttributeDivisorProperties Type;
};

// Map type VkPipelineVertexInputDivisorStateCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineVertexInputDivisorStateCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO> {
    typedef VkPipelineVertexInputDivisorStateCreateInfo Type;
};

// Map type VkPhysicalDeviceVertexAttributeDivisorFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceVertexAttributeDivisorFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES> {
    typedef VkPhysicalDeviceVertexAttributeDivisorFeatures Type;
};

// Map type VkPhysicalDeviceIndexTypeUint8Features to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceIndexTypeUint8Features> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES> {
    typedef VkPhysicalDeviceIndexTypeUint8Features Type;
};

// Map type VkMemoryMapInfo to id VK_STRUCTURE_TYPE_MEMORY_MAP_INFO
template <> struct LvlTypeMap<VkMemoryMapInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_MAP_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_MAP_INFO> {
    typedef VkMemoryMapInfo Type;
};

// Map type VkMemoryUnmapInfo to id VK_STRUCTURE_TYPE_MEMORY_UNMAP_INFO
template <> struct LvlTypeMap<VkMemoryUnmapInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_UNMAP_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_UNMAP_INFO> {
    typedef VkMemoryUnmapInfo Type;
};

// Map type VkPhysicalDeviceMaintenance5Features to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceMaintenance5Features> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES> {
    typedef VkPhysicalDeviceMaintenance5Features Type;
};

// Map type VkPhysicalDeviceMaintenance5Properties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceMaintenance5Properties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_PROPERTIES> {
    typedef VkPhysicalDeviceMaintenance5Properties Type;
};

// Map type VkRenderingAreaInfo to id VK_STRUCTURE_TYPE_RENDERING_AREA_INFO
template <> struct LvlTypeMap<VkRenderingAreaInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDERING_AREA_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDERING_AREA_INFO> {
    typedef VkRenderingAreaInfo Type;
};

// Map type VkImageSubresource2 to id VK_STRUCTURE_TYPE_IMAGE_SUBRESOURCE_2
template <> struct LvlTypeMap<VkImageSubresource2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_SUBRESOURCE_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_SUBRESOURCE_2> {
    typedef VkImageSubresource2 Type;
};

// Map type VkDeviceImageSubresourceInfo to id VK_STRUCTURE_TYPE_DEVICE_IMAGE_SUBRESOURCE_INFO
template <> struct LvlTypeMap<VkDeviceImageSubresourceInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_IMAGE_SUBRESOURCE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_IMAGE_SUBRESOURCE_INFO> {
    typedef VkDeviceImageSubresourceInfo Type;
};

// Map type VkSubresourceLayout2 to id VK_STRUCTURE_TYPE_SUBRESOURCE_LAYOUT_2
template <> struct LvlTypeMap<VkSubresourceLayout2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SUBRESOURCE_LAYOUT_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SUBRESOURCE_LAYOUT_2> {
    typedef VkSubresourceLayout2 Type;
};

// Map type VkPipelineCreateFlags2CreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_CREATE_FLAGS_2_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineCreateFlags2CreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_CREATE_FLAGS_2_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_CREATE_FLAGS_2_CREATE_INFO> {
    typedef VkPipelineCreateFlags2CreateInfo Type;
};

// Map type VkBufferUsageFlags2CreateInfo to id VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO
template <> struct LvlTypeMap<VkBufferUsageFlags2CreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO> {
    typedef VkBufferUsageFlags2CreateInfo Type;
};

// Map type VkPhysicalDevicePushDescriptorProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDevicePushDescriptorProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES> {
    typedef VkPhysicalDevicePushDescriptorProperties Type;
};

// Map type VkPhysicalDeviceDynamicRenderingLocalReadFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceDynamicRenderingLocalReadFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES> {
    typedef VkPhysicalDeviceDynamicRenderingLocalReadFeatures Type;
};

// Map type VkRenderingAttachmentLocationInfo to id VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_LOCATION_INFO
template <> struct LvlTypeMap<VkRenderingAttachmentLocationInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_LOCATION_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_LOCATION_INFO> {
    typedef VkRenderingAttachmentLocationInfo Type;
};

// Map type VkRenderingInputAttachmentIndexInfo to id VK_STRUCTURE_TYPE_RENDERING_INPUT_ATTACHMENT_INDEX_INFO
template <> struct LvlTypeMap<VkRenderingInputAttachmentIndexInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDERING_INPUT_ATTACHMENT_INDEX_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDERING_INPUT_ATTACHMENT_INDEX_INFO> {
    typedef VkRenderingInputAttachmentIndexInfo Type;
};

// Map type VkPhysicalDeviceMaintenance6Features to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceMaintenance6Features> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES> {
    typedef VkPhysicalDeviceMaintenance6Features Type;
};

// Map type VkPhysicalDeviceMaintenance6Properties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceMaintenance6Properties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_PROPERTIES> {
    typedef VkPhysicalDeviceMaintenance6Properties Type;
};

// Map type VkBindMemoryStatus to id VK_STRUCTURE_TYPE_BIND_MEMORY_STATUS
template <> struct LvlTypeMap<VkBindMemoryStatus> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BIND_MEMORY_STATUS;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BIND_MEMORY_STATUS> {
    typedef VkBindMemoryStatus Type;
};

// Map type VkBindDescriptorSetsInfo to id VK_STRUCTURE_TYPE_BIND_DESCRIPTOR_SETS_INFO
template <> struct LvlTypeMap<VkBindDescriptorSetsInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BIND_DESCRIPTOR_SETS_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BIND_DESCRIPTOR_SETS_INFO> {
    typedef VkBindDescriptorSetsInfo Type;
};

// Map type VkPushConstantsInfo to id VK_STRUCTURE_TYPE_PUSH_CONSTANTS_INFO
template <> struct LvlTypeMap<VkPushConstantsInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PUSH_CONSTANTS_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PUSH_CONSTANTS_INFO> {
    typedef VkPushConstantsInfo Type;
};

// Map type VkPushDescriptorSetInfo to id VK_STRUCTURE_TYPE_PUSH_DESCRIPTOR_SET_INFO
template <> struct LvlTypeMap<VkPushDescriptorSetInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PUSH_DESCRIPTOR_SET_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PUSH_DESCRIPTOR_SET_INFO> {
    typedef VkPushDescriptorSetInfo Type;
};

// Map type VkPushDescriptorSetWithTemplateInfo to id VK_STRUCTURE_TYPE_PUSH_DESCRIPTOR_SET_WITH_TEMPLATE_INFO
template <> struct LvlTypeMap<VkPushDescriptorSetWithTemplateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PUSH_DESCRIPTOR_SET_WITH_TEMPLATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PUSH_DESCRIPTOR_SET_WITH_TEMPLATE_INFO> {
    typedef VkPushDescriptorSetWithTemplateInfo Type;
};

// Map type VkPhysicalDevicePipelineProtectedAccessFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROTECTED_ACCESS_FEATURES
template <> struct LvlTypeMap<VkPhysicalDevicePipelineProtectedAccessFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROTECTED_ACCESS_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROTECTED_ACCESS_FEATURES> {
    typedef VkPhysicalDevicePipelineProtectedAccessFeatures Type;
};

// Map type VkPhysicalDevicePipelineRobustnessFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES
template <> struct LvlTypeMap<VkPhysicalDevicePipelineRobustnessFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES> {
    typedef VkPhysicalDevicePipelineRobustnessFeatures Type;
};

// Map type VkPhysicalDevicePipelineRobustnessProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDevicePipelineRobustnessProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_PROPERTIES> {
    typedef VkPhysicalDevicePipelineRobustnessProperties Type;
};

// Map type VkPipelineRobustnessCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_ROBUSTNESS_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineRobustnessCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_ROBUSTNESS_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_ROBUSTNESS_CREATE_INFO> {
    typedef VkPipelineRobustnessCreateInfo Type;
};

// Map type VkPhysicalDeviceHostImageCopyFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceHostImageCopyFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES> {
    typedef VkPhysicalDeviceHostImageCopyFeatures Type;
};

// Map type VkPhysicalDeviceHostImageCopyProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceHostImageCopyProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_PROPERTIES> {
    typedef VkPhysicalDeviceHostImageCopyProperties Type;
};

// Map type VkMemoryToImageCopy to id VK_STRUCTURE_TYPE_MEMORY_TO_IMAGE_COPY
template <> struct LvlTypeMap<VkMemoryToImageCopy> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_TO_IMAGE_COPY;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_TO_IMAGE_COPY> {
    typedef VkMemoryToImageCopy Type;
};

// Map type VkImageToMemoryCopy to id VK_STRUCTURE_TYPE_IMAGE_TO_MEMORY_COPY
template <> struct LvlTypeMap<VkImageToMemoryCopy> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_TO_MEMORY_COPY;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_TO_MEMORY_COPY> {
    typedef VkImageToMemoryCopy Type;
};

// Map type VkCopyMemoryToImageInfo to id VK_STRUCTURE_TYPE_COPY_MEMORY_TO_IMAGE_INFO
template <> struct LvlTypeMap<VkCopyMemoryToImageInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COPY_MEMORY_TO_IMAGE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COPY_MEMORY_TO_IMAGE_INFO> {
    typedef VkCopyMemoryToImageInfo Type;
};

// Map type VkCopyImageToMemoryInfo to id VK_STRUCTURE_TYPE_COPY_IMAGE_TO_MEMORY_INFO
template <> struct LvlTypeMap<VkCopyImageToMemoryInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COPY_IMAGE_TO_MEMORY_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COPY_IMAGE_TO_MEMORY_INFO> {
    typedef VkCopyImageToMemoryInfo Type;
};

// Map type VkCopyImageToImageInfo to id VK_STRUCTURE_TYPE_COPY_IMAGE_TO_IMAGE_INFO
template <> struct LvlTypeMap<VkCopyImageToImageInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COPY_IMAGE_TO_IMAGE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COPY_IMAGE_TO_IMAGE_INFO> {
    typedef VkCopyImageToImageInfo Type;
};

// Map type VkHostImageLayoutTransitionInfo to id VK_STRUCTURE_TYPE_HOST_IMAGE_LAYOUT_TRANSITION_INFO
template <> struct LvlTypeMap<VkHostImageLayoutTransitionInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_HOST_IMAGE_LAYOUT_TRANSITION_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_HOST_IMAGE_LAYOUT_TRANSITION_INFO> {
    typedef VkHostImageLayoutTransitionInfo Type;
};

// Map type VkSubresourceHostMemcpySize to id VK_STRUCTURE_TYPE_SUBRESOURCE_HOST_MEMCPY_SIZE
template <> struct LvlTypeMap<VkSubresourceHostMemcpySize> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SUBRESOURCE_HOST_MEMCPY_SIZE;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SUBRESOURCE_HOST_MEMCPY_SIZE> {
    typedef VkSubresourceHostMemcpySize Type;
};

// Map type VkHostImageCopyDevicePerformanceQuery to id VK_STRUCTURE_TYPE_HOST_IMAGE_COPY_DEVICE_PERFORMANCE_QUERY
template <> struct LvlTypeMap<VkHostImageCopyDevicePerformanceQuery> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_HOST_IMAGE_COPY_DEVICE_PERFORMANCE_QUERY;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_HOST_IMAGE_COPY_DEVICE_PERFORMANCE_QUERY> {
    typedef VkHostImageCopyDevicePerformanceQuery Type;
};

// Map type VkSwapchainCreateInfoKHR to id VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkSwapchainCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR> {
    typedef VkSwapchainCreateInfoKHR Type;
};

// Map type VkPresentInfoKHR to id VK_STRUCTURE_TYPE_PRESENT_INFO_KHR
template <> struct LvlTypeMap<VkPresentInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PRESENT_INFO_KHR> {
    typedef VkPresentInfoKHR Type;
};

// Map type VkImageSwapchainCreateInfoKHR to id VK_STRUCTURE_TYPE_IMAGE_SWAPCHAIN_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkImageSwapchainCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_SWAPCHAIN_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_SWAPCHAIN_CREATE_INFO_KHR> {
    typedef VkImageSwapchainCreateInfoKHR Type;
};

// Map type VkBindImageMemorySwapchainInfoKHR to id VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR
template <> struct LvlTypeMap<VkBindImageMemorySwapchainInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR> {
    typedef VkBindImageMemorySwapchainInfoKHR Type;
};

// Map type VkAcquireNextImageInfoKHR to id VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR
template <> struct LvlTypeMap<VkAcquireNextImageInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR> {
    typedef VkAcquireNextImageInfoKHR Type;
};

// Map type VkDeviceGroupPresentCapabilitiesKHR to id VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_CAPABILITIES_KHR
template <> struct LvlTypeMap<VkDeviceGroupPresentCapabilitiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_CAPABILITIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_CAPABILITIES_KHR> {
    typedef VkDeviceGroupPresentCapabilitiesKHR Type;
};

// Map type VkDeviceGroupPresentInfoKHR to id VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_INFO_KHR
template <> struct LvlTypeMap<VkDeviceGroupPresentInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_INFO_KHR> {
    typedef VkDeviceGroupPresentInfoKHR Type;
};

// Map type VkDeviceGroupSwapchainCreateInfoKHR to id VK_STRUCTURE_TYPE_DEVICE_GROUP_SWAPCHAIN_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkDeviceGroupSwapchainCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_GROUP_SWAPCHAIN_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_GROUP_SWAPCHAIN_CREATE_INFO_KHR> {
    typedef VkDeviceGroupSwapchainCreateInfoKHR Type;
};

// Map type VkDisplayModeCreateInfoKHR to id VK_STRUCTURE_TYPE_DISPLAY_MODE_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkDisplayModeCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DISPLAY_MODE_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DISPLAY_MODE_CREATE_INFO_KHR> {
    typedef VkDisplayModeCreateInfoKHR Type;
};

// Map type VkDisplaySurfaceCreateInfoKHR to id VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkDisplaySurfaceCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR> {
    typedef VkDisplaySurfaceCreateInfoKHR Type;
};

// Map type VkDisplayPresentInfoKHR to id VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR
template <> struct LvlTypeMap<VkDisplayPresentInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR> {
    typedef VkDisplayPresentInfoKHR Type;
};

#ifdef VK_USE_PLATFORM_XLIB_KHR
// Map type VkXlibSurfaceCreateInfoKHR to id VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkXlibSurfaceCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR> {
    typedef VkXlibSurfaceCreateInfoKHR Type;
};

#endif // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
// Map type VkXcbSurfaceCreateInfoKHR to id VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkXcbSurfaceCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR> {
    typedef VkXcbSurfaceCreateInfoKHR Type;
};

#endif // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
// Map type VkWaylandSurfaceCreateInfoKHR to id VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkWaylandSurfaceCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR> {
    typedef VkWaylandSurfaceCreateInfoKHR Type;
};

#endif // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
// Map type VkAndroidSurfaceCreateInfoKHR to id VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkAndroidSurfaceCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR> {
    typedef VkAndroidSurfaceCreateInfoKHR Type;
};

#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
// Map type VkWin32SurfaceCreateInfoKHR to id VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkWin32SurfaceCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR> {
    typedef VkWin32SurfaceCreateInfoKHR Type;
};

#endif // VK_USE_PLATFORM_WIN32_KHR
// Map type VkQueueFamilyQueryResultStatusPropertiesKHR to id VK_STRUCTURE_TYPE_QUEUE_FAMILY_QUERY_RESULT_STATUS_PROPERTIES_KHR
template <> struct LvlTypeMap<VkQueueFamilyQueryResultStatusPropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_QUERY_RESULT_STATUS_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_QUEUE_FAMILY_QUERY_RESULT_STATUS_PROPERTIES_KHR> {
    typedef VkQueueFamilyQueryResultStatusPropertiesKHR Type;
};

// Map type VkQueueFamilyVideoPropertiesKHR to id VK_STRUCTURE_TYPE_QUEUE_FAMILY_VIDEO_PROPERTIES_KHR
template <> struct LvlTypeMap<VkQueueFamilyVideoPropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_VIDEO_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_QUEUE_FAMILY_VIDEO_PROPERTIES_KHR> {
    typedef VkQueueFamilyVideoPropertiesKHR Type;
};

// Map type VkVideoProfileInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR
template <> struct LvlTypeMap<VkVideoProfileInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR> {
    typedef VkVideoProfileInfoKHR Type;
};

// Map type VkVideoProfileListInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_PROFILE_LIST_INFO_KHR
template <> struct LvlTypeMap<VkVideoProfileListInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_PROFILE_LIST_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_PROFILE_LIST_INFO_KHR> {
    typedef VkVideoProfileListInfoKHR Type;
};

// Map type VkVideoCapabilitiesKHR to id VK_STRUCTURE_TYPE_VIDEO_CAPABILITIES_KHR
template <> struct LvlTypeMap<VkVideoCapabilitiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_CAPABILITIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_CAPABILITIES_KHR> {
    typedef VkVideoCapabilitiesKHR Type;
};

// Map type VkPhysicalDeviceVideoFormatInfoKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_FORMAT_INFO_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceVideoFormatInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_FORMAT_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_FORMAT_INFO_KHR> {
    typedef VkPhysicalDeviceVideoFormatInfoKHR Type;
};

// Map type VkVideoFormatPropertiesKHR to id VK_STRUCTURE_TYPE_VIDEO_FORMAT_PROPERTIES_KHR
template <> struct LvlTypeMap<VkVideoFormatPropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_FORMAT_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_FORMAT_PROPERTIES_KHR> {
    typedef VkVideoFormatPropertiesKHR Type;
};

// Map type VkVideoPictureResourceInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_PICTURE_RESOURCE_INFO_KHR
template <> struct LvlTypeMap<VkVideoPictureResourceInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_PICTURE_RESOURCE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_PICTURE_RESOURCE_INFO_KHR> {
    typedef VkVideoPictureResourceInfoKHR Type;
};

// Map type VkVideoReferenceSlotInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_REFERENCE_SLOT_INFO_KHR
template <> struct LvlTypeMap<VkVideoReferenceSlotInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_REFERENCE_SLOT_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_REFERENCE_SLOT_INFO_KHR> {
    typedef VkVideoReferenceSlotInfoKHR Type;
};

// Map type VkVideoSessionMemoryRequirementsKHR to id VK_STRUCTURE_TYPE_VIDEO_SESSION_MEMORY_REQUIREMENTS_KHR
template <> struct LvlTypeMap<VkVideoSessionMemoryRequirementsKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_SESSION_MEMORY_REQUIREMENTS_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_SESSION_MEMORY_REQUIREMENTS_KHR> {
    typedef VkVideoSessionMemoryRequirementsKHR Type;
};

// Map type VkBindVideoSessionMemoryInfoKHR to id VK_STRUCTURE_TYPE_BIND_VIDEO_SESSION_MEMORY_INFO_KHR
template <> struct LvlTypeMap<VkBindVideoSessionMemoryInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BIND_VIDEO_SESSION_MEMORY_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BIND_VIDEO_SESSION_MEMORY_INFO_KHR> {
    typedef VkBindVideoSessionMemoryInfoKHR Type;
};

// Map type VkVideoSessionCreateInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_SESSION_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkVideoSessionCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_SESSION_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_SESSION_CREATE_INFO_KHR> {
    typedef VkVideoSessionCreateInfoKHR Type;
};

// Map type VkVideoSessionParametersCreateInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_SESSION_PARAMETERS_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkVideoSessionParametersCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_SESSION_PARAMETERS_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_SESSION_PARAMETERS_CREATE_INFO_KHR> {
    typedef VkVideoSessionParametersCreateInfoKHR Type;
};

// Map type VkVideoSessionParametersUpdateInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_SESSION_PARAMETERS_UPDATE_INFO_KHR
template <> struct LvlTypeMap<VkVideoSessionParametersUpdateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_SESSION_PARAMETERS_UPDATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_SESSION_PARAMETERS_UPDATE_INFO_KHR> {
    typedef VkVideoSessionParametersUpdateInfoKHR Type;
};

// Map type VkVideoBeginCodingInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_BEGIN_CODING_INFO_KHR
template <> struct LvlTypeMap<VkVideoBeginCodingInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_BEGIN_CODING_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_BEGIN_CODING_INFO_KHR> {
    typedef VkVideoBeginCodingInfoKHR Type;
};

// Map type VkVideoEndCodingInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_END_CODING_INFO_KHR
template <> struct LvlTypeMap<VkVideoEndCodingInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_END_CODING_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_END_CODING_INFO_KHR> {
    typedef VkVideoEndCodingInfoKHR Type;
};

// Map type VkVideoCodingControlInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_CODING_CONTROL_INFO_KHR
template <> struct LvlTypeMap<VkVideoCodingControlInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_CODING_CONTROL_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_CODING_CONTROL_INFO_KHR> {
    typedef VkVideoCodingControlInfoKHR Type;
};

// Map type VkVideoDecodeCapabilitiesKHR to id VK_STRUCTURE_TYPE_VIDEO_DECODE_CAPABILITIES_KHR
template <> struct LvlTypeMap<VkVideoDecodeCapabilitiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_DECODE_CAPABILITIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_DECODE_CAPABILITIES_KHR> {
    typedef VkVideoDecodeCapabilitiesKHR Type;
};

// Map type VkVideoDecodeUsageInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_DECODE_USAGE_INFO_KHR
template <> struct LvlTypeMap<VkVideoDecodeUsageInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_DECODE_USAGE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_DECODE_USAGE_INFO_KHR> {
    typedef VkVideoDecodeUsageInfoKHR Type;
};

// Map type VkVideoDecodeInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_DECODE_INFO_KHR
template <> struct LvlTypeMap<VkVideoDecodeInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_DECODE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_DECODE_INFO_KHR> {
    typedef VkVideoDecodeInfoKHR Type;
};

// Map type VkVideoEncodeH264CapabilitiesKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_CAPABILITIES_KHR
template <> struct LvlTypeMap<VkVideoEncodeH264CapabilitiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_CAPABILITIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_CAPABILITIES_KHR> {
    typedef VkVideoEncodeH264CapabilitiesKHR Type;
};

// Map type VkVideoEncodeH264QualityLevelPropertiesKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_QUALITY_LEVEL_PROPERTIES_KHR
template <> struct LvlTypeMap<VkVideoEncodeH264QualityLevelPropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_QUALITY_LEVEL_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_QUALITY_LEVEL_PROPERTIES_KHR> {
    typedef VkVideoEncodeH264QualityLevelPropertiesKHR Type;
};

// Map type VkVideoEncodeH264SessionCreateInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeH264SessionCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_CREATE_INFO_KHR> {
    typedef VkVideoEncodeH264SessionCreateInfoKHR Type;
};

// Map type VkVideoEncodeH264SessionParametersAddInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_ADD_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeH264SessionParametersAddInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_ADD_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_ADD_INFO_KHR> {
    typedef VkVideoEncodeH264SessionParametersAddInfoKHR Type;
};

// Map type VkVideoEncodeH264SessionParametersCreateInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeH264SessionParametersCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_CREATE_INFO_KHR> {
    typedef VkVideoEncodeH264SessionParametersCreateInfoKHR Type;
};

// Map type VkVideoEncodeH264SessionParametersGetInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_GET_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeH264SessionParametersGetInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_GET_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_GET_INFO_KHR> {
    typedef VkVideoEncodeH264SessionParametersGetInfoKHR Type;
};

// Map type VkVideoEncodeH264SessionParametersFeedbackInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_FEEDBACK_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeH264SessionParametersFeedbackInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_FEEDBACK_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_FEEDBACK_INFO_KHR> {
    typedef VkVideoEncodeH264SessionParametersFeedbackInfoKHR Type;
};

// Map type VkVideoEncodeH264NaluSliceInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_NALU_SLICE_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeH264NaluSliceInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_NALU_SLICE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_NALU_SLICE_INFO_KHR> {
    typedef VkVideoEncodeH264NaluSliceInfoKHR Type;
};

// Map type VkVideoEncodeH264PictureInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PICTURE_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeH264PictureInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PICTURE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PICTURE_INFO_KHR> {
    typedef VkVideoEncodeH264PictureInfoKHR Type;
};

// Map type VkVideoEncodeH264DpbSlotInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_DPB_SLOT_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeH264DpbSlotInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_DPB_SLOT_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_DPB_SLOT_INFO_KHR> {
    typedef VkVideoEncodeH264DpbSlotInfoKHR Type;
};

// Map type VkVideoEncodeH264ProfileInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PROFILE_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeH264ProfileInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PROFILE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PROFILE_INFO_KHR> {
    typedef VkVideoEncodeH264ProfileInfoKHR Type;
};

// Map type VkVideoEncodeH264RateControlInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_RATE_CONTROL_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeH264RateControlInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_RATE_CONTROL_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_RATE_CONTROL_INFO_KHR> {
    typedef VkVideoEncodeH264RateControlInfoKHR Type;
};

// Map type VkVideoEncodeH264RateControlLayerInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_RATE_CONTROL_LAYER_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeH264RateControlLayerInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_RATE_CONTROL_LAYER_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_RATE_CONTROL_LAYER_INFO_KHR> {
    typedef VkVideoEncodeH264RateControlLayerInfoKHR Type;
};

// Map type VkVideoEncodeH264GopRemainingFrameInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_GOP_REMAINING_FRAME_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeH264GopRemainingFrameInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_GOP_REMAINING_FRAME_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_GOP_REMAINING_FRAME_INFO_KHR> {
    typedef VkVideoEncodeH264GopRemainingFrameInfoKHR Type;
};

// Map type VkVideoEncodeH265CapabilitiesKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_CAPABILITIES_KHR
template <> struct LvlTypeMap<VkVideoEncodeH265CapabilitiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_CAPABILITIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_CAPABILITIES_KHR> {
    typedef VkVideoEncodeH265CapabilitiesKHR Type;
};

// Map type VkVideoEncodeH265SessionCreateInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeH265SessionCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_CREATE_INFO_KHR> {
    typedef VkVideoEncodeH265SessionCreateInfoKHR Type;
};

// Map type VkVideoEncodeH265QualityLevelPropertiesKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_QUALITY_LEVEL_PROPERTIES_KHR
template <> struct LvlTypeMap<VkVideoEncodeH265QualityLevelPropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_QUALITY_LEVEL_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_QUALITY_LEVEL_PROPERTIES_KHR> {
    typedef VkVideoEncodeH265QualityLevelPropertiesKHR Type;
};

// Map type VkVideoEncodeH265SessionParametersAddInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_ADD_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeH265SessionParametersAddInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_ADD_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_ADD_INFO_KHR> {
    typedef VkVideoEncodeH265SessionParametersAddInfoKHR Type;
};

// Map type VkVideoEncodeH265SessionParametersCreateInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeH265SessionParametersCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_CREATE_INFO_KHR> {
    typedef VkVideoEncodeH265SessionParametersCreateInfoKHR Type;
};

// Map type VkVideoEncodeH265SessionParametersGetInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_GET_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeH265SessionParametersGetInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_GET_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_GET_INFO_KHR> {
    typedef VkVideoEncodeH265SessionParametersGetInfoKHR Type;
};

// Map type VkVideoEncodeH265SessionParametersFeedbackInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_FEEDBACK_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeH265SessionParametersFeedbackInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_FEEDBACK_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_FEEDBACK_INFO_KHR> {
    typedef VkVideoEncodeH265SessionParametersFeedbackInfoKHR Type;
};

// Map type VkVideoEncodeH265NaluSliceSegmentInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_NALU_SLICE_SEGMENT_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeH265NaluSliceSegmentInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_NALU_SLICE_SEGMENT_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_NALU_SLICE_SEGMENT_INFO_KHR> {
    typedef VkVideoEncodeH265NaluSliceSegmentInfoKHR Type;
};

// Map type VkVideoEncodeH265PictureInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PICTURE_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeH265PictureInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PICTURE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PICTURE_INFO_KHR> {
    typedef VkVideoEncodeH265PictureInfoKHR Type;
};

// Map type VkVideoEncodeH265DpbSlotInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_DPB_SLOT_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeH265DpbSlotInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_DPB_SLOT_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_DPB_SLOT_INFO_KHR> {
    typedef VkVideoEncodeH265DpbSlotInfoKHR Type;
};

// Map type VkVideoEncodeH265ProfileInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PROFILE_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeH265ProfileInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PROFILE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PROFILE_INFO_KHR> {
    typedef VkVideoEncodeH265ProfileInfoKHR Type;
};

// Map type VkVideoEncodeH265RateControlInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_RATE_CONTROL_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeH265RateControlInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_RATE_CONTROL_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_RATE_CONTROL_INFO_KHR> {
    typedef VkVideoEncodeH265RateControlInfoKHR Type;
};

// Map type VkVideoEncodeH265RateControlLayerInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_RATE_CONTROL_LAYER_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeH265RateControlLayerInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_RATE_CONTROL_LAYER_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_RATE_CONTROL_LAYER_INFO_KHR> {
    typedef VkVideoEncodeH265RateControlLayerInfoKHR Type;
};

// Map type VkVideoEncodeH265GopRemainingFrameInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_GOP_REMAINING_FRAME_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeH265GopRemainingFrameInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_GOP_REMAINING_FRAME_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_GOP_REMAINING_FRAME_INFO_KHR> {
    typedef VkVideoEncodeH265GopRemainingFrameInfoKHR Type;
};

// Map type VkVideoDecodeH264ProfileInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PROFILE_INFO_KHR
template <> struct LvlTypeMap<VkVideoDecodeH264ProfileInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PROFILE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PROFILE_INFO_KHR> {
    typedef VkVideoDecodeH264ProfileInfoKHR Type;
};

// Map type VkVideoDecodeH264CapabilitiesKHR to id VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_CAPABILITIES_KHR
template <> struct LvlTypeMap<VkVideoDecodeH264CapabilitiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_CAPABILITIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_CAPABILITIES_KHR> {
    typedef VkVideoDecodeH264CapabilitiesKHR Type;
};

// Map type VkVideoDecodeH264SessionParametersAddInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_ADD_INFO_KHR
template <> struct LvlTypeMap<VkVideoDecodeH264SessionParametersAddInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_ADD_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_ADD_INFO_KHR> {
    typedef VkVideoDecodeH264SessionParametersAddInfoKHR Type;
};

// Map type VkVideoDecodeH264SessionParametersCreateInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkVideoDecodeH264SessionParametersCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_CREATE_INFO_KHR> {
    typedef VkVideoDecodeH264SessionParametersCreateInfoKHR Type;
};

// Map type VkVideoDecodeH264PictureInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PICTURE_INFO_KHR
template <> struct LvlTypeMap<VkVideoDecodeH264PictureInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PICTURE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PICTURE_INFO_KHR> {
    typedef VkVideoDecodeH264PictureInfoKHR Type;
};

// Map type VkVideoDecodeH264DpbSlotInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_DPB_SLOT_INFO_KHR
template <> struct LvlTypeMap<VkVideoDecodeH264DpbSlotInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_DPB_SLOT_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_DPB_SLOT_INFO_KHR> {
    typedef VkVideoDecodeH264DpbSlotInfoKHR Type;
};

#ifdef VK_USE_PLATFORM_WIN32_KHR
// Map type VkImportMemoryWin32HandleInfoKHR to id VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR
template <> struct LvlTypeMap<VkImportMemoryWin32HandleInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR> {
    typedef VkImportMemoryWin32HandleInfoKHR Type;
};

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
// Map type VkExportMemoryWin32HandleInfoKHR to id VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR
template <> struct LvlTypeMap<VkExportMemoryWin32HandleInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR> {
    typedef VkExportMemoryWin32HandleInfoKHR Type;
};

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
// Map type VkMemoryWin32HandlePropertiesKHR to id VK_STRUCTURE_TYPE_MEMORY_WIN32_HANDLE_PROPERTIES_KHR
template <> struct LvlTypeMap<VkMemoryWin32HandlePropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_WIN32_HANDLE_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_WIN32_HANDLE_PROPERTIES_KHR> {
    typedef VkMemoryWin32HandlePropertiesKHR Type;
};

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
// Map type VkMemoryGetWin32HandleInfoKHR to id VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR
template <> struct LvlTypeMap<VkMemoryGetWin32HandleInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR> {
    typedef VkMemoryGetWin32HandleInfoKHR Type;
};

#endif // VK_USE_PLATFORM_WIN32_KHR
// Map type VkImportMemoryFdInfoKHR to id VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR
template <> struct LvlTypeMap<VkImportMemoryFdInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR> {
    typedef VkImportMemoryFdInfoKHR Type;
};

// Map type VkMemoryFdPropertiesKHR to id VK_STRUCTURE_TYPE_MEMORY_FD_PROPERTIES_KHR
template <> struct LvlTypeMap<VkMemoryFdPropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_FD_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_FD_PROPERTIES_KHR> {
    typedef VkMemoryFdPropertiesKHR Type;
};

// Map type VkMemoryGetFdInfoKHR to id VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR
template <> struct LvlTypeMap<VkMemoryGetFdInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR> {
    typedef VkMemoryGetFdInfoKHR Type;
};

#ifdef VK_USE_PLATFORM_WIN32_KHR
// Map type VkWin32KeyedMutexAcquireReleaseInfoKHR to id VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_KHR
template <> struct LvlTypeMap<VkWin32KeyedMutexAcquireReleaseInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_KHR> {
    typedef VkWin32KeyedMutexAcquireReleaseInfoKHR Type;
};

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
// Map type VkImportSemaphoreWin32HandleInfoKHR to id VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR
template <> struct LvlTypeMap<VkImportSemaphoreWin32HandleInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR> {
    typedef VkImportSemaphoreWin32HandleInfoKHR Type;
};

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
// Map type VkExportSemaphoreWin32HandleInfoKHR to id VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR
template <> struct LvlTypeMap<VkExportSemaphoreWin32HandleInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR> {
    typedef VkExportSemaphoreWin32HandleInfoKHR Type;
};

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
// Map type VkD3D12FenceSubmitInfoKHR to id VK_STRUCTURE_TYPE_D3D12_FENCE_SUBMIT_INFO_KHR
template <> struct LvlTypeMap<VkD3D12FenceSubmitInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_D3D12_FENCE_SUBMIT_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_D3D12_FENCE_SUBMIT_INFO_KHR> {
    typedef VkD3D12FenceSubmitInfoKHR Type;
};

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
// Map type VkSemaphoreGetWin32HandleInfoKHR to id VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR
template <> struct LvlTypeMap<VkSemaphoreGetWin32HandleInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR> {
    typedef VkSemaphoreGetWin32HandleInfoKHR Type;
};

#endif // VK_USE_PLATFORM_WIN32_KHR
// Map type VkImportSemaphoreFdInfoKHR to id VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR
template <> struct LvlTypeMap<VkImportSemaphoreFdInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR> {
    typedef VkImportSemaphoreFdInfoKHR Type;
};

// Map type VkSemaphoreGetFdInfoKHR to id VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR
template <> struct LvlTypeMap<VkSemaphoreGetFdInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR> {
    typedef VkSemaphoreGetFdInfoKHR Type;
};

// Map type VkPresentRegionsKHR to id VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR
template <> struct LvlTypeMap<VkPresentRegionsKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR> {
    typedef VkPresentRegionsKHR Type;
};

// Map type VkSharedPresentSurfaceCapabilitiesKHR to id VK_STRUCTURE_TYPE_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR
template <> struct LvlTypeMap<VkSharedPresentSurfaceCapabilitiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR> {
    typedef VkSharedPresentSurfaceCapabilitiesKHR Type;
};

#ifdef VK_USE_PLATFORM_WIN32_KHR
// Map type VkImportFenceWin32HandleInfoKHR to id VK_STRUCTURE_TYPE_IMPORT_FENCE_WIN32_HANDLE_INFO_KHR
template <> struct LvlTypeMap<VkImportFenceWin32HandleInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_FENCE_WIN32_HANDLE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_FENCE_WIN32_HANDLE_INFO_KHR> {
    typedef VkImportFenceWin32HandleInfoKHR Type;
};

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
// Map type VkExportFenceWin32HandleInfoKHR to id VK_STRUCTURE_TYPE_EXPORT_FENCE_WIN32_HANDLE_INFO_KHR
template <> struct LvlTypeMap<VkExportFenceWin32HandleInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXPORT_FENCE_WIN32_HANDLE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXPORT_FENCE_WIN32_HANDLE_INFO_KHR> {
    typedef VkExportFenceWin32HandleInfoKHR Type;
};

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
// Map type VkFenceGetWin32HandleInfoKHR to id VK_STRUCTURE_TYPE_FENCE_GET_WIN32_HANDLE_INFO_KHR
template <> struct LvlTypeMap<VkFenceGetWin32HandleInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_FENCE_GET_WIN32_HANDLE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_FENCE_GET_WIN32_HANDLE_INFO_KHR> {
    typedef VkFenceGetWin32HandleInfoKHR Type;
};

#endif // VK_USE_PLATFORM_WIN32_KHR
// Map type VkImportFenceFdInfoKHR to id VK_STRUCTURE_TYPE_IMPORT_FENCE_FD_INFO_KHR
template <> struct LvlTypeMap<VkImportFenceFdInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_FENCE_FD_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_FENCE_FD_INFO_KHR> {
    typedef VkImportFenceFdInfoKHR Type;
};

// Map type VkFenceGetFdInfoKHR to id VK_STRUCTURE_TYPE_FENCE_GET_FD_INFO_KHR
template <> struct LvlTypeMap<VkFenceGetFdInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_FENCE_GET_FD_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_FENCE_GET_FD_INFO_KHR> {
    typedef VkFenceGetFdInfoKHR Type;
};

// Map type VkPhysicalDevicePerformanceQueryFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDevicePerformanceQueryFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR> {
    typedef VkPhysicalDevicePerformanceQueryFeaturesKHR Type;
};

// Map type VkPhysicalDevicePerformanceQueryPropertiesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_PROPERTIES_KHR
template <> struct LvlTypeMap<VkPhysicalDevicePerformanceQueryPropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_PROPERTIES_KHR> {
    typedef VkPhysicalDevicePerformanceQueryPropertiesKHR Type;
};

// Map type VkPerformanceCounterKHR to id VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_KHR
template <> struct LvlTypeMap<VkPerformanceCounterKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_KHR> {
    typedef VkPerformanceCounterKHR Type;
};

// Map type VkPerformanceCounterDescriptionKHR to id VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_DESCRIPTION_KHR
template <> struct LvlTypeMap<VkPerformanceCounterDescriptionKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_DESCRIPTION_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_DESCRIPTION_KHR> {
    typedef VkPerformanceCounterDescriptionKHR Type;
};

// Map type VkQueryPoolPerformanceCreateInfoKHR to id VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkQueryPoolPerformanceCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR> {
    typedef VkQueryPoolPerformanceCreateInfoKHR Type;
};

// Map type VkAcquireProfilingLockInfoKHR to id VK_STRUCTURE_TYPE_ACQUIRE_PROFILING_LOCK_INFO_KHR
template <> struct LvlTypeMap<VkAcquireProfilingLockInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ACQUIRE_PROFILING_LOCK_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ACQUIRE_PROFILING_LOCK_INFO_KHR> {
    typedef VkAcquireProfilingLockInfoKHR Type;
};

// Map type VkPerformanceQuerySubmitInfoKHR to id VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_SUBMIT_INFO_KHR
template <> struct LvlTypeMap<VkPerformanceQuerySubmitInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_SUBMIT_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_SUBMIT_INFO_KHR> {
    typedef VkPerformanceQuerySubmitInfoKHR Type;
};

// Map type VkPhysicalDeviceSurfaceInfo2KHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceSurfaceInfo2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR> {
    typedef VkPhysicalDeviceSurfaceInfo2KHR Type;
};

// Map type VkSurfaceCapabilities2KHR to id VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR
template <> struct LvlTypeMap<VkSurfaceCapabilities2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR> {
    typedef VkSurfaceCapabilities2KHR Type;
};

// Map type VkSurfaceFormat2KHR to id VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR
template <> struct LvlTypeMap<VkSurfaceFormat2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR> {
    typedef VkSurfaceFormat2KHR Type;
};

// Map type VkDisplayProperties2KHR to id VK_STRUCTURE_TYPE_DISPLAY_PROPERTIES_2_KHR
template <> struct LvlTypeMap<VkDisplayProperties2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DISPLAY_PROPERTIES_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DISPLAY_PROPERTIES_2_KHR> {
    typedef VkDisplayProperties2KHR Type;
};

// Map type VkDisplayPlaneProperties2KHR to id VK_STRUCTURE_TYPE_DISPLAY_PLANE_PROPERTIES_2_KHR
template <> struct LvlTypeMap<VkDisplayPlaneProperties2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DISPLAY_PLANE_PROPERTIES_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DISPLAY_PLANE_PROPERTIES_2_KHR> {
    typedef VkDisplayPlaneProperties2KHR Type;
};

// Map type VkDisplayModeProperties2KHR to id VK_STRUCTURE_TYPE_DISPLAY_MODE_PROPERTIES_2_KHR
template <> struct LvlTypeMap<VkDisplayModeProperties2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DISPLAY_MODE_PROPERTIES_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DISPLAY_MODE_PROPERTIES_2_KHR> {
    typedef VkDisplayModeProperties2KHR Type;
};

// Map type VkDisplayPlaneInfo2KHR to id VK_STRUCTURE_TYPE_DISPLAY_PLANE_INFO_2_KHR
template <> struct LvlTypeMap<VkDisplayPlaneInfo2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DISPLAY_PLANE_INFO_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DISPLAY_PLANE_INFO_2_KHR> {
    typedef VkDisplayPlaneInfo2KHR Type;
};

// Map type VkDisplayPlaneCapabilities2KHR to id VK_STRUCTURE_TYPE_DISPLAY_PLANE_CAPABILITIES_2_KHR
template <> struct LvlTypeMap<VkDisplayPlaneCapabilities2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DISPLAY_PLANE_CAPABILITIES_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DISPLAY_PLANE_CAPABILITIES_2_KHR> {
    typedef VkDisplayPlaneCapabilities2KHR Type;
};

#ifdef VK_ENABLE_BETA_EXTENSIONS
// Map type VkPhysicalDevicePortabilitySubsetFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDevicePortabilitySubsetFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR> {
    typedef VkPhysicalDevicePortabilitySubsetFeaturesKHR Type;
};

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
// Map type VkPhysicalDevicePortabilitySubsetPropertiesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_PROPERTIES_KHR
template <> struct LvlTypeMap<VkPhysicalDevicePortabilitySubsetPropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_PROPERTIES_KHR> {
    typedef VkPhysicalDevicePortabilitySubsetPropertiesKHR Type;
};

#endif // VK_ENABLE_BETA_EXTENSIONS
// Map type VkPhysicalDeviceShaderClockFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceShaderClockFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR> {
    typedef VkPhysicalDeviceShaderClockFeaturesKHR Type;
};

// Map type VkVideoDecodeH265ProfileInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PROFILE_INFO_KHR
template <> struct LvlTypeMap<VkVideoDecodeH265ProfileInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PROFILE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PROFILE_INFO_KHR> {
    typedef VkVideoDecodeH265ProfileInfoKHR Type;
};

// Map type VkVideoDecodeH265CapabilitiesKHR to id VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_CAPABILITIES_KHR
template <> struct LvlTypeMap<VkVideoDecodeH265CapabilitiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_CAPABILITIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_CAPABILITIES_KHR> {
    typedef VkVideoDecodeH265CapabilitiesKHR Type;
};

// Map type VkVideoDecodeH265SessionParametersAddInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_PARAMETERS_ADD_INFO_KHR
template <> struct LvlTypeMap<VkVideoDecodeH265SessionParametersAddInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_PARAMETERS_ADD_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_PARAMETERS_ADD_INFO_KHR> {
    typedef VkVideoDecodeH265SessionParametersAddInfoKHR Type;
};

// Map type VkVideoDecodeH265SessionParametersCreateInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_PARAMETERS_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkVideoDecodeH265SessionParametersCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_PARAMETERS_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_PARAMETERS_CREATE_INFO_KHR> {
    typedef VkVideoDecodeH265SessionParametersCreateInfoKHR Type;
};

// Map type VkVideoDecodeH265PictureInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PICTURE_INFO_KHR
template <> struct LvlTypeMap<VkVideoDecodeH265PictureInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PICTURE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PICTURE_INFO_KHR> {
    typedef VkVideoDecodeH265PictureInfoKHR Type;
};

// Map type VkVideoDecodeH265DpbSlotInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_DPB_SLOT_INFO_KHR
template <> struct LvlTypeMap<VkVideoDecodeH265DpbSlotInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_DPB_SLOT_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_DPB_SLOT_INFO_KHR> {
    typedef VkVideoDecodeH265DpbSlotInfoKHR Type;
};

// Map type VkFragmentShadingRateAttachmentInfoKHR to id VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR
template <> struct LvlTypeMap<VkFragmentShadingRateAttachmentInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR> {
    typedef VkFragmentShadingRateAttachmentInfoKHR Type;
};

// Map type VkPipelineFragmentShadingRateStateCreateInfoKHR to id VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_STATE_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkPipelineFragmentShadingRateStateCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_STATE_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_STATE_CREATE_INFO_KHR> {
    typedef VkPipelineFragmentShadingRateStateCreateInfoKHR Type;
};

// Map type VkPhysicalDeviceFragmentShadingRateFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceFragmentShadingRateFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR> {
    typedef VkPhysicalDeviceFragmentShadingRateFeaturesKHR Type;
};

// Map type VkPhysicalDeviceFragmentShadingRatePropertiesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceFragmentShadingRatePropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR> {
    typedef VkPhysicalDeviceFragmentShadingRatePropertiesKHR Type;
};

// Map type VkPhysicalDeviceFragmentShadingRateKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceFragmentShadingRateKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_KHR> {
    typedef VkPhysicalDeviceFragmentShadingRateKHR Type;
};

// Map type VkRenderingFragmentShadingRateAttachmentInfoKHR to id VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR
template <> struct LvlTypeMap<VkRenderingFragmentShadingRateAttachmentInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR> {
    typedef VkRenderingFragmentShadingRateAttachmentInfoKHR Type;
};

// Map type VkPhysicalDeviceShaderQuadControlFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_QUAD_CONTROL_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceShaderQuadControlFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_QUAD_CONTROL_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_QUAD_CONTROL_FEATURES_KHR> {
    typedef VkPhysicalDeviceShaderQuadControlFeaturesKHR Type;
};

// Map type VkSurfaceProtectedCapabilitiesKHR to id VK_STRUCTURE_TYPE_SURFACE_PROTECTED_CAPABILITIES_KHR
template <> struct LvlTypeMap<VkSurfaceProtectedCapabilitiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SURFACE_PROTECTED_CAPABILITIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SURFACE_PROTECTED_CAPABILITIES_KHR> {
    typedef VkSurfaceProtectedCapabilitiesKHR Type;
};

// Map type VkPhysicalDevicePresentWaitFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDevicePresentWaitFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR> {
    typedef VkPhysicalDevicePresentWaitFeaturesKHR Type;
};

// Map type VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR> {
    typedef VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR Type;
};

// Map type VkPipelineInfoKHR to id VK_STRUCTURE_TYPE_PIPELINE_INFO_KHR
template <> struct LvlTypeMap<VkPipelineInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_INFO_KHR> {
    typedef VkPipelineInfoKHR Type;
};

// Map type VkPipelineExecutablePropertiesKHR to id VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_PROPERTIES_KHR
template <> struct LvlTypeMap<VkPipelineExecutablePropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_PROPERTIES_KHR> {
    typedef VkPipelineExecutablePropertiesKHR Type;
};

// Map type VkPipelineExecutableInfoKHR to id VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INFO_KHR
template <> struct LvlTypeMap<VkPipelineExecutableInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INFO_KHR> {
    typedef VkPipelineExecutableInfoKHR Type;
};

// Map type VkPipelineExecutableStatisticKHR to id VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_STATISTIC_KHR
template <> struct LvlTypeMap<VkPipelineExecutableStatisticKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_STATISTIC_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_STATISTIC_KHR> {
    typedef VkPipelineExecutableStatisticKHR Type;
};

// Map type VkPipelineExecutableInternalRepresentationKHR to id VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INTERNAL_REPRESENTATION_KHR
template <> struct LvlTypeMap<VkPipelineExecutableInternalRepresentationKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INTERNAL_REPRESENTATION_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INTERNAL_REPRESENTATION_KHR> {
    typedef VkPipelineExecutableInternalRepresentationKHR Type;
};

// Map type VkPipelineLibraryCreateInfoKHR to id VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkPipelineLibraryCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR> {
    typedef VkPipelineLibraryCreateInfoKHR Type;
};

// Map type VkPresentIdKHR to id VK_STRUCTURE_TYPE_PRESENT_ID_KHR
template <> struct LvlTypeMap<VkPresentIdKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PRESENT_ID_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PRESENT_ID_KHR> {
    typedef VkPresentIdKHR Type;
};

// Map type VkPhysicalDevicePresentIdFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDevicePresentIdFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR> {
    typedef VkPhysicalDevicePresentIdFeaturesKHR Type;
};

// Map type VkVideoEncodeInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_INFO_KHR> {
    typedef VkVideoEncodeInfoKHR Type;
};

// Map type VkVideoEncodeCapabilitiesKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_CAPABILITIES_KHR
template <> struct LvlTypeMap<VkVideoEncodeCapabilitiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_CAPABILITIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_CAPABILITIES_KHR> {
    typedef VkVideoEncodeCapabilitiesKHR Type;
};

// Map type VkQueryPoolVideoEncodeFeedbackCreateInfoKHR to id VK_STRUCTURE_TYPE_QUERY_POOL_VIDEO_ENCODE_FEEDBACK_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkQueryPoolVideoEncodeFeedbackCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_QUERY_POOL_VIDEO_ENCODE_FEEDBACK_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_QUERY_POOL_VIDEO_ENCODE_FEEDBACK_CREATE_INFO_KHR> {
    typedef VkQueryPoolVideoEncodeFeedbackCreateInfoKHR Type;
};

// Map type VkVideoEncodeUsageInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_USAGE_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeUsageInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_USAGE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_USAGE_INFO_KHR> {
    typedef VkVideoEncodeUsageInfoKHR Type;
};

// Map type VkVideoEncodeRateControlLayerInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_RATE_CONTROL_LAYER_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeRateControlLayerInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_RATE_CONTROL_LAYER_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_RATE_CONTROL_LAYER_INFO_KHR> {
    typedef VkVideoEncodeRateControlLayerInfoKHR Type;
};

// Map type VkVideoEncodeRateControlInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_RATE_CONTROL_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeRateControlInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_RATE_CONTROL_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_RATE_CONTROL_INFO_KHR> {
    typedef VkVideoEncodeRateControlInfoKHR Type;
};

// Map type VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_QUALITY_LEVEL_INFO_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_QUALITY_LEVEL_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_QUALITY_LEVEL_INFO_KHR> {
    typedef VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR Type;
};

// Map type VkVideoEncodeQualityLevelPropertiesKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUALITY_LEVEL_PROPERTIES_KHR
template <> struct LvlTypeMap<VkVideoEncodeQualityLevelPropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUALITY_LEVEL_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUALITY_LEVEL_PROPERTIES_KHR> {
    typedef VkVideoEncodeQualityLevelPropertiesKHR Type;
};

// Map type VkVideoEncodeQualityLevelInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUALITY_LEVEL_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeQualityLevelInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUALITY_LEVEL_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUALITY_LEVEL_INFO_KHR> {
    typedef VkVideoEncodeQualityLevelInfoKHR Type;
};

// Map type VkVideoEncodeSessionParametersGetInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_SESSION_PARAMETERS_GET_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeSessionParametersGetInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_SESSION_PARAMETERS_GET_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_SESSION_PARAMETERS_GET_INFO_KHR> {
    typedef VkVideoEncodeSessionParametersGetInfoKHR Type;
};

// Map type VkVideoEncodeSessionParametersFeedbackInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_SESSION_PARAMETERS_FEEDBACK_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeSessionParametersFeedbackInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_SESSION_PARAMETERS_FEEDBACK_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_SESSION_PARAMETERS_FEEDBACK_INFO_KHR> {
    typedef VkVideoEncodeSessionParametersFeedbackInfoKHR Type;
};

// Map type VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR> {
    typedef VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR Type;
};

// Map type VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_PROPERTIES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_PROPERTIES_KHR> {
    typedef VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR Type;
};

// Map type VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_FEATURES_KHR> {
    typedef VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR Type;
};

// Map type VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_FEATURES_KHR> {
    typedef VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR Type;
};

// Map type VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR> {
    typedef VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR Type;
};

// Map type VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MAXIMAL_RECONVERGENCE_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MAXIMAL_RECONVERGENCE_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MAXIMAL_RECONVERGENCE_FEATURES_KHR> {
    typedef VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR Type;
};

// Map type VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR> {
    typedef VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR Type;
};

// Map type VkPhysicalDevicePipelineBinaryFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_BINARY_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDevicePipelineBinaryFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_BINARY_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_BINARY_FEATURES_KHR> {
    typedef VkPhysicalDevicePipelineBinaryFeaturesKHR Type;
};

// Map type VkPhysicalDevicePipelineBinaryPropertiesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_BINARY_PROPERTIES_KHR
template <> struct LvlTypeMap<VkPhysicalDevicePipelineBinaryPropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_BINARY_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_BINARY_PROPERTIES_KHR> {
    typedef VkPhysicalDevicePipelineBinaryPropertiesKHR Type;
};

// Map type VkDevicePipelineBinaryInternalCacheControlKHR to id VK_STRUCTURE_TYPE_DEVICE_PIPELINE_BINARY_INTERNAL_CACHE_CONTROL_KHR
template <> struct LvlTypeMap<VkDevicePipelineBinaryInternalCacheControlKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_PIPELINE_BINARY_INTERNAL_CACHE_CONTROL_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_PIPELINE_BINARY_INTERNAL_CACHE_CONTROL_KHR> {
    typedef VkDevicePipelineBinaryInternalCacheControlKHR Type;
};

// Map type VkPipelineBinaryKeyKHR to id VK_STRUCTURE_TYPE_PIPELINE_BINARY_KEY_KHR
template <> struct LvlTypeMap<VkPipelineBinaryKeyKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_BINARY_KEY_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_BINARY_KEY_KHR> {
    typedef VkPipelineBinaryKeyKHR Type;
};

// Map type VkPipelineCreateInfoKHR to id VK_STRUCTURE_TYPE_PIPELINE_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkPipelineCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_CREATE_INFO_KHR> {
    typedef VkPipelineCreateInfoKHR Type;
};

// Map type VkPipelineBinaryCreateInfoKHR to id VK_STRUCTURE_TYPE_PIPELINE_BINARY_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkPipelineBinaryCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_BINARY_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_BINARY_CREATE_INFO_KHR> {
    typedef VkPipelineBinaryCreateInfoKHR Type;
};

// Map type VkPipelineBinaryInfoKHR to id VK_STRUCTURE_TYPE_PIPELINE_BINARY_INFO_KHR
template <> struct LvlTypeMap<VkPipelineBinaryInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_BINARY_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_BINARY_INFO_KHR> {
    typedef VkPipelineBinaryInfoKHR Type;
};

// Map type VkReleaseCapturedPipelineDataInfoKHR to id VK_STRUCTURE_TYPE_RELEASE_CAPTURED_PIPELINE_DATA_INFO_KHR
template <> struct LvlTypeMap<VkReleaseCapturedPipelineDataInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RELEASE_CAPTURED_PIPELINE_DATA_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RELEASE_CAPTURED_PIPELINE_DATA_INFO_KHR> {
    typedef VkReleaseCapturedPipelineDataInfoKHR Type;
};

// Map type VkPipelineBinaryDataInfoKHR to id VK_STRUCTURE_TYPE_PIPELINE_BINARY_DATA_INFO_KHR
template <> struct LvlTypeMap<VkPipelineBinaryDataInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_BINARY_DATA_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_BINARY_DATA_INFO_KHR> {
    typedef VkPipelineBinaryDataInfoKHR Type;
};

// Map type VkPipelineBinaryHandlesInfoKHR to id VK_STRUCTURE_TYPE_PIPELINE_BINARY_HANDLES_INFO_KHR
template <> struct LvlTypeMap<VkPipelineBinaryHandlesInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_BINARY_HANDLES_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_BINARY_HANDLES_INFO_KHR> {
    typedef VkPipelineBinaryHandlesInfoKHR Type;
};

// Map type VkCooperativeMatrixPropertiesKHR to id VK_STRUCTURE_TYPE_COOPERATIVE_MATRIX_PROPERTIES_KHR
template <> struct LvlTypeMap<VkCooperativeMatrixPropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COOPERATIVE_MATRIX_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COOPERATIVE_MATRIX_PROPERTIES_KHR> {
    typedef VkCooperativeMatrixPropertiesKHR Type;
};

// Map type VkPhysicalDeviceCooperativeMatrixFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceCooperativeMatrixFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR> {
    typedef VkPhysicalDeviceCooperativeMatrixFeaturesKHR Type;
};

// Map type VkPhysicalDeviceCooperativeMatrixPropertiesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceCooperativeMatrixPropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_KHR> {
    typedef VkPhysicalDeviceCooperativeMatrixPropertiesKHR Type;
};

// Map type VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_KHR> {
    typedef VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR Type;
};

// Map type VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_PROPERTIES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_PROPERTIES_KHR> {
    typedef VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR Type;
};

// Map type VkVideoDecodeAV1ProfileInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_PROFILE_INFO_KHR
template <> struct LvlTypeMap<VkVideoDecodeAV1ProfileInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_PROFILE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_PROFILE_INFO_KHR> {
    typedef VkVideoDecodeAV1ProfileInfoKHR Type;
};

// Map type VkVideoDecodeAV1CapabilitiesKHR to id VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_CAPABILITIES_KHR
template <> struct LvlTypeMap<VkVideoDecodeAV1CapabilitiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_CAPABILITIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_CAPABILITIES_KHR> {
    typedef VkVideoDecodeAV1CapabilitiesKHR Type;
};

// Map type VkVideoDecodeAV1SessionParametersCreateInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_SESSION_PARAMETERS_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkVideoDecodeAV1SessionParametersCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_SESSION_PARAMETERS_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_SESSION_PARAMETERS_CREATE_INFO_KHR> {
    typedef VkVideoDecodeAV1SessionParametersCreateInfoKHR Type;
};

// Map type VkVideoDecodeAV1PictureInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_PICTURE_INFO_KHR
template <> struct LvlTypeMap<VkVideoDecodeAV1PictureInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_PICTURE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_PICTURE_INFO_KHR> {
    typedef VkVideoDecodeAV1PictureInfoKHR Type;
};

// Map type VkVideoDecodeAV1DpbSlotInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_DPB_SLOT_INFO_KHR
template <> struct LvlTypeMap<VkVideoDecodeAV1DpbSlotInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_DPB_SLOT_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_DPB_SLOT_INFO_KHR> {
    typedef VkVideoDecodeAV1DpbSlotInfoKHR Type;
};

// Map type VkPhysicalDeviceVideoEncodeAV1FeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_AV1_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceVideoEncodeAV1FeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_AV1_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_AV1_FEATURES_KHR> {
    typedef VkPhysicalDeviceVideoEncodeAV1FeaturesKHR Type;
};

// Map type VkVideoEncodeAV1CapabilitiesKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_CAPABILITIES_KHR
template <> struct LvlTypeMap<VkVideoEncodeAV1CapabilitiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_CAPABILITIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_CAPABILITIES_KHR> {
    typedef VkVideoEncodeAV1CapabilitiesKHR Type;
};

// Map type VkVideoEncodeAV1QualityLevelPropertiesKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_QUALITY_LEVEL_PROPERTIES_KHR
template <> struct LvlTypeMap<VkVideoEncodeAV1QualityLevelPropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_QUALITY_LEVEL_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_QUALITY_LEVEL_PROPERTIES_KHR> {
    typedef VkVideoEncodeAV1QualityLevelPropertiesKHR Type;
};

// Map type VkVideoEncodeAV1SessionCreateInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_SESSION_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeAV1SessionCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_SESSION_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_SESSION_CREATE_INFO_KHR> {
    typedef VkVideoEncodeAV1SessionCreateInfoKHR Type;
};

// Map type VkVideoEncodeAV1SessionParametersCreateInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_SESSION_PARAMETERS_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeAV1SessionParametersCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_SESSION_PARAMETERS_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_SESSION_PARAMETERS_CREATE_INFO_KHR> {
    typedef VkVideoEncodeAV1SessionParametersCreateInfoKHR Type;
};

// Map type VkVideoEncodeAV1PictureInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_PICTURE_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeAV1PictureInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_PICTURE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_PICTURE_INFO_KHR> {
    typedef VkVideoEncodeAV1PictureInfoKHR Type;
};

// Map type VkVideoEncodeAV1DpbSlotInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_DPB_SLOT_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeAV1DpbSlotInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_DPB_SLOT_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_DPB_SLOT_INFO_KHR> {
    typedef VkVideoEncodeAV1DpbSlotInfoKHR Type;
};

// Map type VkVideoEncodeAV1ProfileInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_PROFILE_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeAV1ProfileInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_PROFILE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_PROFILE_INFO_KHR> {
    typedef VkVideoEncodeAV1ProfileInfoKHR Type;
};

// Map type VkVideoEncodeAV1GopRemainingFrameInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_GOP_REMAINING_FRAME_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeAV1GopRemainingFrameInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_GOP_REMAINING_FRAME_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_GOP_REMAINING_FRAME_INFO_KHR> {
    typedef VkVideoEncodeAV1GopRemainingFrameInfoKHR Type;
};

// Map type VkVideoEncodeAV1RateControlInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_RATE_CONTROL_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeAV1RateControlInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_RATE_CONTROL_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_RATE_CONTROL_INFO_KHR> {
    typedef VkVideoEncodeAV1RateControlInfoKHR Type;
};

// Map type VkVideoEncodeAV1RateControlLayerInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_RATE_CONTROL_LAYER_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeAV1RateControlLayerInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_RATE_CONTROL_LAYER_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_RATE_CONTROL_LAYER_INFO_KHR> {
    typedef VkVideoEncodeAV1RateControlLayerInfoKHR Type;
};

// Map type VkPhysicalDeviceVideoMaintenance1FeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_1_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceVideoMaintenance1FeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_1_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_1_FEATURES_KHR> {
    typedef VkPhysicalDeviceVideoMaintenance1FeaturesKHR Type;
};

// Map type VkVideoInlineQueryInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_INLINE_QUERY_INFO_KHR
template <> struct LvlTypeMap<VkVideoInlineQueryInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_INLINE_QUERY_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_INLINE_QUERY_INFO_KHR> {
    typedef VkVideoInlineQueryInfoKHR Type;
};

// Map type VkCalibratedTimestampInfoKHR to id VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_KHR
template <> struct LvlTypeMap<VkCalibratedTimestampInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_KHR> {
    typedef VkCalibratedTimestampInfoKHR Type;
};

// Map type VkSetDescriptorBufferOffsetsInfoEXT to id VK_STRUCTURE_TYPE_SET_DESCRIPTOR_BUFFER_OFFSETS_INFO_EXT
template <> struct LvlTypeMap<VkSetDescriptorBufferOffsetsInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SET_DESCRIPTOR_BUFFER_OFFSETS_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SET_DESCRIPTOR_BUFFER_OFFSETS_INFO_EXT> {
    typedef VkSetDescriptorBufferOffsetsInfoEXT Type;
};

// Map type VkBindDescriptorBufferEmbeddedSamplersInfoEXT to id VK_STRUCTURE_TYPE_BIND_DESCRIPTOR_BUFFER_EMBEDDED_SAMPLERS_INFO_EXT
template <> struct LvlTypeMap<VkBindDescriptorBufferEmbeddedSamplersInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BIND_DESCRIPTOR_BUFFER_EMBEDDED_SAMPLERS_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BIND_DESCRIPTOR_BUFFER_EMBEDDED_SAMPLERS_INFO_EXT> {
    typedef VkBindDescriptorBufferEmbeddedSamplersInfoEXT Type;
};

// Map type VkVideoEncodeQuantizationMapCapabilitiesKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_CAPABILITIES_KHR
template <> struct LvlTypeMap<VkVideoEncodeQuantizationMapCapabilitiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_CAPABILITIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_CAPABILITIES_KHR> {
    typedef VkVideoEncodeQuantizationMapCapabilitiesKHR Type;
};

// Map type VkVideoFormatQuantizationMapPropertiesKHR to id VK_STRUCTURE_TYPE_VIDEO_FORMAT_QUANTIZATION_MAP_PROPERTIES_KHR
template <> struct LvlTypeMap<VkVideoFormatQuantizationMapPropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_FORMAT_QUANTIZATION_MAP_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_FORMAT_QUANTIZATION_MAP_PROPERTIES_KHR> {
    typedef VkVideoFormatQuantizationMapPropertiesKHR Type;
};

// Map type VkVideoEncodeQuantizationMapInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeQuantizationMapInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_INFO_KHR> {
    typedef VkVideoEncodeQuantizationMapInfoKHR Type;
};

// Map type VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_SESSION_PARAMETERS_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_SESSION_PARAMETERS_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_SESSION_PARAMETERS_CREATE_INFO_KHR> {
    typedef VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR Type;
};

// Map type VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_QUANTIZATION_MAP_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_QUANTIZATION_MAP_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_QUANTIZATION_MAP_FEATURES_KHR> {
    typedef VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR Type;
};

// Map type VkVideoEncodeH264QuantizationMapCapabilitiesKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_QUANTIZATION_MAP_CAPABILITIES_KHR
template <> struct LvlTypeMap<VkVideoEncodeH264QuantizationMapCapabilitiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_QUANTIZATION_MAP_CAPABILITIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_QUANTIZATION_MAP_CAPABILITIES_KHR> {
    typedef VkVideoEncodeH264QuantizationMapCapabilitiesKHR Type;
};

// Map type VkVideoEncodeH265QuantizationMapCapabilitiesKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_QUANTIZATION_MAP_CAPABILITIES_KHR
template <> struct LvlTypeMap<VkVideoEncodeH265QuantizationMapCapabilitiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_QUANTIZATION_MAP_CAPABILITIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_QUANTIZATION_MAP_CAPABILITIES_KHR> {
    typedef VkVideoEncodeH265QuantizationMapCapabilitiesKHR Type;
};

// Map type VkVideoFormatH265QuantizationMapPropertiesKHR to id VK_STRUCTURE_TYPE_VIDEO_FORMAT_H265_QUANTIZATION_MAP_PROPERTIES_KHR
template <> struct LvlTypeMap<VkVideoFormatH265QuantizationMapPropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_FORMAT_H265_QUANTIZATION_MAP_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_FORMAT_H265_QUANTIZATION_MAP_PROPERTIES_KHR> {
    typedef VkVideoFormatH265QuantizationMapPropertiesKHR Type;
};

// Map type VkVideoEncodeAV1QuantizationMapCapabilitiesKHR to id VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_QUANTIZATION_MAP_CAPABILITIES_KHR
template <> struct LvlTypeMap<VkVideoEncodeAV1QuantizationMapCapabilitiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_QUANTIZATION_MAP_CAPABILITIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_QUANTIZATION_MAP_CAPABILITIES_KHR> {
    typedef VkVideoEncodeAV1QuantizationMapCapabilitiesKHR Type;
};

// Map type VkVideoFormatAV1QuantizationMapPropertiesKHR to id VK_STRUCTURE_TYPE_VIDEO_FORMAT_AV1_QUANTIZATION_MAP_PROPERTIES_KHR
template <> struct LvlTypeMap<VkVideoFormatAV1QuantizationMapPropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_FORMAT_AV1_QUANTIZATION_MAP_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_FORMAT_AV1_QUANTIZATION_MAP_PROPERTIES_KHR> {
    typedef VkVideoFormatAV1QuantizationMapPropertiesKHR Type;
};

// Map type VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_RELAXED_EXTENDED_INSTRUCTION_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_RELAXED_EXTENDED_INSTRUCTION_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_RELAXED_EXTENDED_INSTRUCTION_FEATURES_KHR> {
    typedef VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR Type;
};

// Map type VkPhysicalDeviceMaintenance7FeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceMaintenance7FeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_FEATURES_KHR> {
    typedef VkPhysicalDeviceMaintenance7FeaturesKHR Type;
};

// Map type VkPhysicalDeviceMaintenance7PropertiesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_PROPERTIES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceMaintenance7PropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_PROPERTIES_KHR> {
    typedef VkPhysicalDeviceMaintenance7PropertiesKHR Type;
};

// Map type VkPhysicalDeviceLayeredApiPropertiesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_API_PROPERTIES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceLayeredApiPropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_API_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_API_PROPERTIES_KHR> {
    typedef VkPhysicalDeviceLayeredApiPropertiesKHR Type;
};

// Map type VkPhysicalDeviceLayeredApiPropertiesListKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_API_PROPERTIES_LIST_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceLayeredApiPropertiesListKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_API_PROPERTIES_LIST_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_API_PROPERTIES_LIST_KHR> {
    typedef VkPhysicalDeviceLayeredApiPropertiesListKHR Type;
};

// Map type VkPhysicalDeviceLayeredApiVulkanPropertiesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_API_VULKAN_PROPERTIES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceLayeredApiVulkanPropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_API_VULKAN_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_API_VULKAN_PROPERTIES_KHR> {
    typedef VkPhysicalDeviceLayeredApiVulkanPropertiesKHR Type;
};

// Map type VkPhysicalDeviceMaintenance8FeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_8_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceMaintenance8FeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_8_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_8_FEATURES_KHR> {
    typedef VkPhysicalDeviceMaintenance8FeaturesKHR Type;
};

// Map type VkMemoryBarrierAccessFlags3KHR to id VK_STRUCTURE_TYPE_MEMORY_BARRIER_ACCESS_FLAGS_3_KHR
template <> struct LvlTypeMap<VkMemoryBarrierAccessFlags3KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_ACCESS_FLAGS_3_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_BARRIER_ACCESS_FLAGS_3_KHR> {
    typedef VkMemoryBarrierAccessFlags3KHR Type;
};

// Map type VkPhysicalDeviceVideoMaintenance2FeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_2_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceVideoMaintenance2FeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_2_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_2_FEATURES_KHR> {
    typedef VkPhysicalDeviceVideoMaintenance2FeaturesKHR Type;
};

// Map type VkVideoDecodeH264InlineSessionParametersInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_INLINE_SESSION_PARAMETERS_INFO_KHR
template <> struct LvlTypeMap<VkVideoDecodeH264InlineSessionParametersInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_INLINE_SESSION_PARAMETERS_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_INLINE_SESSION_PARAMETERS_INFO_KHR> {
    typedef VkVideoDecodeH264InlineSessionParametersInfoKHR Type;
};

// Map type VkVideoDecodeH265InlineSessionParametersInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_INLINE_SESSION_PARAMETERS_INFO_KHR
template <> struct LvlTypeMap<VkVideoDecodeH265InlineSessionParametersInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_INLINE_SESSION_PARAMETERS_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_INLINE_SESSION_PARAMETERS_INFO_KHR> {
    typedef VkVideoDecodeH265InlineSessionParametersInfoKHR Type;
};

// Map type VkVideoDecodeAV1InlineSessionParametersInfoKHR to id VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_INLINE_SESSION_PARAMETERS_INFO_KHR
template <> struct LvlTypeMap<VkVideoDecodeAV1InlineSessionParametersInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_INLINE_SESSION_PARAMETERS_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_INLINE_SESSION_PARAMETERS_INFO_KHR> {
    typedef VkVideoDecodeAV1InlineSessionParametersInfoKHR Type;
};

// Map type VkPhysicalDeviceDepthClampZeroOneFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_ZERO_ONE_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceDepthClampZeroOneFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_ZERO_ONE_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_ZERO_ONE_FEATURES_KHR> {
    typedef VkPhysicalDeviceDepthClampZeroOneFeaturesKHR Type;
};

// Map type VkDebugReportCallbackCreateInfoEXT to id VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkDebugReportCallbackCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT> {
    typedef VkDebugReportCallbackCreateInfoEXT Type;
};

// Map type VkPipelineRasterizationStateRasterizationOrderAMD to id VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_RASTERIZATION_ORDER_AMD
template <> struct LvlTypeMap<VkPipelineRasterizationStateRasterizationOrderAMD> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_RASTERIZATION_ORDER_AMD;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_RASTERIZATION_ORDER_AMD> {
    typedef VkPipelineRasterizationStateRasterizationOrderAMD Type;
};

// Map type VkDebugMarkerObjectNameInfoEXT to id VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT
template <> struct LvlTypeMap<VkDebugMarkerObjectNameInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT> {
    typedef VkDebugMarkerObjectNameInfoEXT Type;
};

// Map type VkDebugMarkerObjectTagInfoEXT to id VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT
template <> struct LvlTypeMap<VkDebugMarkerObjectTagInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT> {
    typedef VkDebugMarkerObjectTagInfoEXT Type;
};

// Map type VkDebugMarkerMarkerInfoEXT to id VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT
template <> struct LvlTypeMap<VkDebugMarkerMarkerInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT> {
    typedef VkDebugMarkerMarkerInfoEXT Type;
};

// Map type VkDedicatedAllocationImageCreateInfoNV to id VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_IMAGE_CREATE_INFO_NV
template <> struct LvlTypeMap<VkDedicatedAllocationImageCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_IMAGE_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_IMAGE_CREATE_INFO_NV> {
    typedef VkDedicatedAllocationImageCreateInfoNV Type;
};

// Map type VkDedicatedAllocationBufferCreateInfoNV to id VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_BUFFER_CREATE_INFO_NV
template <> struct LvlTypeMap<VkDedicatedAllocationBufferCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_BUFFER_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_BUFFER_CREATE_INFO_NV> {
    typedef VkDedicatedAllocationBufferCreateInfoNV Type;
};

// Map type VkDedicatedAllocationMemoryAllocateInfoNV to id VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_MEMORY_ALLOCATE_INFO_NV
template <> struct LvlTypeMap<VkDedicatedAllocationMemoryAllocateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_MEMORY_ALLOCATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_MEMORY_ALLOCATE_INFO_NV> {
    typedef VkDedicatedAllocationMemoryAllocateInfoNV Type;
};

// Map type VkPhysicalDeviceTransformFeedbackFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceTransformFeedbackFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT> {
    typedef VkPhysicalDeviceTransformFeedbackFeaturesEXT Type;
};

// Map type VkPhysicalDeviceTransformFeedbackPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceTransformFeedbackPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceTransformFeedbackPropertiesEXT Type;
};

// Map type VkPipelineRasterizationStateStreamCreateInfoEXT to id VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_STREAM_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkPipelineRasterizationStateStreamCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_STREAM_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_STREAM_CREATE_INFO_EXT> {
    typedef VkPipelineRasterizationStateStreamCreateInfoEXT Type;
};

// Map type VkCuModuleCreateInfoNVX to id VK_STRUCTURE_TYPE_CU_MODULE_CREATE_INFO_NVX
template <> struct LvlTypeMap<VkCuModuleCreateInfoNVX> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_CU_MODULE_CREATE_INFO_NVX;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_CU_MODULE_CREATE_INFO_NVX> {
    typedef VkCuModuleCreateInfoNVX Type;
};

// Map type VkCuModuleTexturingModeCreateInfoNVX to id VK_STRUCTURE_TYPE_CU_MODULE_TEXTURING_MODE_CREATE_INFO_NVX
template <> struct LvlTypeMap<VkCuModuleTexturingModeCreateInfoNVX> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_CU_MODULE_TEXTURING_MODE_CREATE_INFO_NVX;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_CU_MODULE_TEXTURING_MODE_CREATE_INFO_NVX> {
    typedef VkCuModuleTexturingModeCreateInfoNVX Type;
};

// Map type VkCuFunctionCreateInfoNVX to id VK_STRUCTURE_TYPE_CU_FUNCTION_CREATE_INFO_NVX
template <> struct LvlTypeMap<VkCuFunctionCreateInfoNVX> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_CU_FUNCTION_CREATE_INFO_NVX;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_CU_FUNCTION_CREATE_INFO_NVX> {
    typedef VkCuFunctionCreateInfoNVX Type;
};

// Map type VkCuLaunchInfoNVX to id VK_STRUCTURE_TYPE_CU_LAUNCH_INFO_NVX
template <> struct LvlTypeMap<VkCuLaunchInfoNVX> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_CU_LAUNCH_INFO_NVX;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_CU_LAUNCH_INFO_NVX> {
    typedef VkCuLaunchInfoNVX Type;
};

// Map type VkImageViewHandleInfoNVX to id VK_STRUCTURE_TYPE_IMAGE_VIEW_HANDLE_INFO_NVX
template <> struct LvlTypeMap<VkImageViewHandleInfoNVX> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_VIEW_HANDLE_INFO_NVX;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_VIEW_HANDLE_INFO_NVX> {
    typedef VkImageViewHandleInfoNVX Type;
};

// Map type VkImageViewAddressPropertiesNVX to id VK_STRUCTURE_TYPE_IMAGE_VIEW_ADDRESS_PROPERTIES_NVX
template <> struct LvlTypeMap<VkImageViewAddressPropertiesNVX> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_VIEW_ADDRESS_PROPERTIES_NVX;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_VIEW_ADDRESS_PROPERTIES_NVX> {
    typedef VkImageViewAddressPropertiesNVX Type;
};

// Map type VkTextureLODGatherFormatPropertiesAMD to id VK_STRUCTURE_TYPE_TEXTURE_LOD_GATHER_FORMAT_PROPERTIES_AMD
template <> struct LvlTypeMap<VkTextureLODGatherFormatPropertiesAMD> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_TEXTURE_LOD_GATHER_FORMAT_PROPERTIES_AMD;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_TEXTURE_LOD_GATHER_FORMAT_PROPERTIES_AMD> {
    typedef VkTextureLODGatherFormatPropertiesAMD Type;
};

#ifdef VK_USE_PLATFORM_GGP
// Map type VkStreamDescriptorSurfaceCreateInfoGGP to id VK_STRUCTURE_TYPE_STREAM_DESCRIPTOR_SURFACE_CREATE_INFO_GGP
template <> struct LvlTypeMap<VkStreamDescriptorSurfaceCreateInfoGGP> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_STREAM_DESCRIPTOR_SURFACE_CREATE_INFO_GGP;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_STREAM_DESCRIPTOR_SURFACE_CREATE_INFO_GGP> {
    typedef VkStreamDescriptorSurfaceCreateInfoGGP Type;
};

#endif // VK_USE_PLATFORM_GGP
// Map type VkPhysicalDeviceCornerSampledImageFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CORNER_SAMPLED_IMAGE_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceCornerSampledImageFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CORNER_SAMPLED_IMAGE_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CORNER_SAMPLED_IMAGE_FEATURES_NV> {
    typedef VkPhysicalDeviceCornerSampledImageFeaturesNV Type;
};

// Map type VkExternalMemoryImageCreateInfoNV to id VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_NV
template <> struct LvlTypeMap<VkExternalMemoryImageCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_NV> {
    typedef VkExternalMemoryImageCreateInfoNV Type;
};

// Map type VkExportMemoryAllocateInfoNV to id VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_NV
template <> struct LvlTypeMap<VkExportMemoryAllocateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_NV> {
    typedef VkExportMemoryAllocateInfoNV Type;
};

#ifdef VK_USE_PLATFORM_WIN32_KHR
// Map type VkImportMemoryWin32HandleInfoNV to id VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_NV
template <> struct LvlTypeMap<VkImportMemoryWin32HandleInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_NV> {
    typedef VkImportMemoryWin32HandleInfoNV Type;
};

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
// Map type VkExportMemoryWin32HandleInfoNV to id VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_NV
template <> struct LvlTypeMap<VkExportMemoryWin32HandleInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_NV> {
    typedef VkExportMemoryWin32HandleInfoNV Type;
};

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
// Map type VkWin32KeyedMutexAcquireReleaseInfoNV to id VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_NV
template <> struct LvlTypeMap<VkWin32KeyedMutexAcquireReleaseInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_NV> {
    typedef VkWin32KeyedMutexAcquireReleaseInfoNV Type;
};

#endif // VK_USE_PLATFORM_WIN32_KHR
// Map type VkValidationFlagsEXT to id VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT
template <> struct LvlTypeMap<VkValidationFlagsEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT> {
    typedef VkValidationFlagsEXT Type;
};

#ifdef VK_USE_PLATFORM_VI_NN
// Map type VkViSurfaceCreateInfoNN to id VK_STRUCTURE_TYPE_VI_SURFACE_CREATE_INFO_NN
template <> struct LvlTypeMap<VkViSurfaceCreateInfoNN> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VI_SURFACE_CREATE_INFO_NN;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VI_SURFACE_CREATE_INFO_NN> {
    typedef VkViSurfaceCreateInfoNN Type;
};

#endif // VK_USE_PLATFORM_VI_NN
// Map type VkImageViewASTCDecodeModeEXT to id VK_STRUCTURE_TYPE_IMAGE_VIEW_ASTC_DECODE_MODE_EXT
template <> struct LvlTypeMap<VkImageViewASTCDecodeModeEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_VIEW_ASTC_DECODE_MODE_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_VIEW_ASTC_DECODE_MODE_EXT> {
    typedef VkImageViewASTCDecodeModeEXT Type;
};

// Map type VkPhysicalDeviceASTCDecodeFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ASTC_DECODE_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceASTCDecodeFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ASTC_DECODE_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ASTC_DECODE_FEATURES_EXT> {
    typedef VkPhysicalDeviceASTCDecodeFeaturesEXT Type;
};

// Map type VkConditionalRenderingBeginInfoEXT to id VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT
template <> struct LvlTypeMap<VkConditionalRenderingBeginInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT> {
    typedef VkConditionalRenderingBeginInfoEXT Type;
};

// Map type VkPhysicalDeviceConditionalRenderingFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceConditionalRenderingFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT> {
    typedef VkPhysicalDeviceConditionalRenderingFeaturesEXT Type;
};

// Map type VkCommandBufferInheritanceConditionalRenderingInfoEXT to id VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT
template <> struct LvlTypeMap<VkCommandBufferInheritanceConditionalRenderingInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT> {
    typedef VkCommandBufferInheritanceConditionalRenderingInfoEXT Type;
};

// Map type VkPipelineViewportWScalingStateCreateInfoNV to id VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_W_SCALING_STATE_CREATE_INFO_NV
template <> struct LvlTypeMap<VkPipelineViewportWScalingStateCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_W_SCALING_STATE_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_W_SCALING_STATE_CREATE_INFO_NV> {
    typedef VkPipelineViewportWScalingStateCreateInfoNV Type;
};

// Map type VkSurfaceCapabilities2EXT to id VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_EXT
template <> struct LvlTypeMap<VkSurfaceCapabilities2EXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_EXT> {
    typedef VkSurfaceCapabilities2EXT Type;
};

// Map type VkDisplayPowerInfoEXT to id VK_STRUCTURE_TYPE_DISPLAY_POWER_INFO_EXT
template <> struct LvlTypeMap<VkDisplayPowerInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DISPLAY_POWER_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DISPLAY_POWER_INFO_EXT> {
    typedef VkDisplayPowerInfoEXT Type;
};

// Map type VkDeviceEventInfoEXT to id VK_STRUCTURE_TYPE_DEVICE_EVENT_INFO_EXT
template <> struct LvlTypeMap<VkDeviceEventInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_EVENT_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_EVENT_INFO_EXT> {
    typedef VkDeviceEventInfoEXT Type;
};

// Map type VkDisplayEventInfoEXT to id VK_STRUCTURE_TYPE_DISPLAY_EVENT_INFO_EXT
template <> struct LvlTypeMap<VkDisplayEventInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DISPLAY_EVENT_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DISPLAY_EVENT_INFO_EXT> {
    typedef VkDisplayEventInfoEXT Type;
};

// Map type VkSwapchainCounterCreateInfoEXT to id VK_STRUCTURE_TYPE_SWAPCHAIN_COUNTER_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkSwapchainCounterCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SWAPCHAIN_COUNTER_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SWAPCHAIN_COUNTER_CREATE_INFO_EXT> {
    typedef VkSwapchainCounterCreateInfoEXT Type;
};

// Map type VkPresentTimesInfoGOOGLE to id VK_STRUCTURE_TYPE_PRESENT_TIMES_INFO_GOOGLE
template <> struct LvlTypeMap<VkPresentTimesInfoGOOGLE> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PRESENT_TIMES_INFO_GOOGLE;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PRESENT_TIMES_INFO_GOOGLE> {
    typedef VkPresentTimesInfoGOOGLE Type;
};

// Map type VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_ATTRIBUTES_PROPERTIES_NVX
template <> struct LvlTypeMap<VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_ATTRIBUTES_PROPERTIES_NVX;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_ATTRIBUTES_PROPERTIES_NVX> {
    typedef VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX Type;
};

// Map type VkMultiviewPerViewAttributesInfoNVX to id VK_STRUCTURE_TYPE_MULTIVIEW_PER_VIEW_ATTRIBUTES_INFO_NVX
template <> struct LvlTypeMap<VkMultiviewPerViewAttributesInfoNVX> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MULTIVIEW_PER_VIEW_ATTRIBUTES_INFO_NVX;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MULTIVIEW_PER_VIEW_ATTRIBUTES_INFO_NVX> {
    typedef VkMultiviewPerViewAttributesInfoNVX Type;
};

// Map type VkPipelineViewportSwizzleStateCreateInfoNV to id VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SWIZZLE_STATE_CREATE_INFO_NV
template <> struct LvlTypeMap<VkPipelineViewportSwizzleStateCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SWIZZLE_STATE_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SWIZZLE_STATE_CREATE_INFO_NV> {
    typedef VkPipelineViewportSwizzleStateCreateInfoNV Type;
};

// Map type VkPhysicalDeviceDiscardRectanglePropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISCARD_RECTANGLE_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceDiscardRectanglePropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISCARD_RECTANGLE_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISCARD_RECTANGLE_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceDiscardRectanglePropertiesEXT Type;
};

// Map type VkPipelineDiscardRectangleStateCreateInfoEXT to id VK_STRUCTURE_TYPE_PIPELINE_DISCARD_RECTANGLE_STATE_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkPipelineDiscardRectangleStateCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_DISCARD_RECTANGLE_STATE_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_DISCARD_RECTANGLE_STATE_CREATE_INFO_EXT> {
    typedef VkPipelineDiscardRectangleStateCreateInfoEXT Type;
};

// Map type VkPhysicalDeviceConservativeRasterizationPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceConservativeRasterizationPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceConservativeRasterizationPropertiesEXT Type;
};

// Map type VkPipelineRasterizationConservativeStateCreateInfoEXT to id VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkPipelineRasterizationConservativeStateCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT> {
    typedef VkPipelineRasterizationConservativeStateCreateInfoEXT Type;
};

// Map type VkPhysicalDeviceDepthClipEnableFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceDepthClipEnableFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT> {
    typedef VkPhysicalDeviceDepthClipEnableFeaturesEXT Type;
};

// Map type VkPipelineRasterizationDepthClipStateCreateInfoEXT to id VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkPipelineRasterizationDepthClipStateCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT> {
    typedef VkPipelineRasterizationDepthClipStateCreateInfoEXT Type;
};

// Map type VkHdrMetadataEXT to id VK_STRUCTURE_TYPE_HDR_METADATA_EXT
template <> struct LvlTypeMap<VkHdrMetadataEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_HDR_METADATA_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_HDR_METADATA_EXT> {
    typedef VkHdrMetadataEXT Type;
};

// Map type VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RELAXED_LINE_RASTERIZATION_FEATURES_IMG
template <> struct LvlTypeMap<VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RELAXED_LINE_RASTERIZATION_FEATURES_IMG;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RELAXED_LINE_RASTERIZATION_FEATURES_IMG> {
    typedef VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG Type;
};

#ifdef VK_USE_PLATFORM_IOS_MVK
// Map type VkIOSSurfaceCreateInfoMVK to id VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK
template <> struct LvlTypeMap<VkIOSSurfaceCreateInfoMVK> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK> {
    typedef VkIOSSurfaceCreateInfoMVK Type;
};

#endif // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
// Map type VkMacOSSurfaceCreateInfoMVK to id VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK
template <> struct LvlTypeMap<VkMacOSSurfaceCreateInfoMVK> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK> {
    typedef VkMacOSSurfaceCreateInfoMVK Type;
};

#endif // VK_USE_PLATFORM_MACOS_MVK
// Map type VkDebugUtilsLabelEXT to id VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT
template <> struct LvlTypeMap<VkDebugUtilsLabelEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT> {
    typedef VkDebugUtilsLabelEXT Type;
};

// Map type VkDebugUtilsObjectNameInfoEXT to id VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT
template <> struct LvlTypeMap<VkDebugUtilsObjectNameInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT> {
    typedef VkDebugUtilsObjectNameInfoEXT Type;
};

// Map type VkDebugUtilsMessengerCallbackDataEXT to id VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT
template <> struct LvlTypeMap<VkDebugUtilsMessengerCallbackDataEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT> {
    typedef VkDebugUtilsMessengerCallbackDataEXT Type;
};

// Map type VkDebugUtilsMessengerCreateInfoEXT to id VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkDebugUtilsMessengerCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT> {
    typedef VkDebugUtilsMessengerCreateInfoEXT Type;
};

// Map type VkDebugUtilsObjectTagInfoEXT to id VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT
template <> struct LvlTypeMap<VkDebugUtilsObjectTagInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT> {
    typedef VkDebugUtilsObjectTagInfoEXT Type;
};

#ifdef VK_USE_PLATFORM_ANDROID_KHR
// Map type VkAndroidHardwareBufferUsageANDROID to id VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_USAGE_ANDROID
template <> struct LvlTypeMap<VkAndroidHardwareBufferUsageANDROID> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_USAGE_ANDROID;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_USAGE_ANDROID> {
    typedef VkAndroidHardwareBufferUsageANDROID Type;
};

#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
// Map type VkAndroidHardwareBufferPropertiesANDROID to id VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID
template <> struct LvlTypeMap<VkAndroidHardwareBufferPropertiesANDROID> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID> {
    typedef VkAndroidHardwareBufferPropertiesANDROID Type;
};

#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
// Map type VkAndroidHardwareBufferFormatPropertiesANDROID to id VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID
template <> struct LvlTypeMap<VkAndroidHardwareBufferFormatPropertiesANDROID> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID> {
    typedef VkAndroidHardwareBufferFormatPropertiesANDROID Type;
};

#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
// Map type VkImportAndroidHardwareBufferInfoANDROID to id VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID
template <> struct LvlTypeMap<VkImportAndroidHardwareBufferInfoANDROID> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID> {
    typedef VkImportAndroidHardwareBufferInfoANDROID Type;
};

#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
// Map type VkMemoryGetAndroidHardwareBufferInfoANDROID to id VK_STRUCTURE_TYPE_MEMORY_GET_ANDROID_HARDWARE_BUFFER_INFO_ANDROID
template <> struct LvlTypeMap<VkMemoryGetAndroidHardwareBufferInfoANDROID> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_GET_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_GET_ANDROID_HARDWARE_BUFFER_INFO_ANDROID> {
    typedef VkMemoryGetAndroidHardwareBufferInfoANDROID Type;
};

#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
// Map type VkExternalFormatANDROID to id VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID
template <> struct LvlTypeMap<VkExternalFormatANDROID> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID> {
    typedef VkExternalFormatANDROID Type;
};

#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
// Map type VkAndroidHardwareBufferFormatProperties2ANDROID to id VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_2_ANDROID
template <> struct LvlTypeMap<VkAndroidHardwareBufferFormatProperties2ANDROID> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_2_ANDROID;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_2_ANDROID> {
    typedef VkAndroidHardwareBufferFormatProperties2ANDROID Type;
};

#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS
// Map type VkPhysicalDeviceShaderEnqueueFeaturesAMDX to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ENQUEUE_FEATURES_AMDX
template <> struct LvlTypeMap<VkPhysicalDeviceShaderEnqueueFeaturesAMDX> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ENQUEUE_FEATURES_AMDX;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ENQUEUE_FEATURES_AMDX> {
    typedef VkPhysicalDeviceShaderEnqueueFeaturesAMDX Type;
};

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
// Map type VkPhysicalDeviceShaderEnqueuePropertiesAMDX to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ENQUEUE_PROPERTIES_AMDX
template <> struct LvlTypeMap<VkPhysicalDeviceShaderEnqueuePropertiesAMDX> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ENQUEUE_PROPERTIES_AMDX;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ENQUEUE_PROPERTIES_AMDX> {
    typedef VkPhysicalDeviceShaderEnqueuePropertiesAMDX Type;
};

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
// Map type VkExecutionGraphPipelineScratchSizeAMDX to id VK_STRUCTURE_TYPE_EXECUTION_GRAPH_PIPELINE_SCRATCH_SIZE_AMDX
template <> struct LvlTypeMap<VkExecutionGraphPipelineScratchSizeAMDX> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXECUTION_GRAPH_PIPELINE_SCRATCH_SIZE_AMDX;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXECUTION_GRAPH_PIPELINE_SCRATCH_SIZE_AMDX> {
    typedef VkExecutionGraphPipelineScratchSizeAMDX Type;
};

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
// Map type VkExecutionGraphPipelineCreateInfoAMDX to id VK_STRUCTURE_TYPE_EXECUTION_GRAPH_PIPELINE_CREATE_INFO_AMDX
template <> struct LvlTypeMap<VkExecutionGraphPipelineCreateInfoAMDX> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXECUTION_GRAPH_PIPELINE_CREATE_INFO_AMDX;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXECUTION_GRAPH_PIPELINE_CREATE_INFO_AMDX> {
    typedef VkExecutionGraphPipelineCreateInfoAMDX Type;
};

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
// Map type VkPipelineShaderStageNodeCreateInfoAMDX to id VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_NODE_CREATE_INFO_AMDX
template <> struct LvlTypeMap<VkPipelineShaderStageNodeCreateInfoAMDX> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_NODE_CREATE_INFO_AMDX;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_NODE_CREATE_INFO_AMDX> {
    typedef VkPipelineShaderStageNodeCreateInfoAMDX Type;
};

#endif // VK_ENABLE_BETA_EXTENSIONS
// Map type VkAttachmentSampleCountInfoAMD to id VK_STRUCTURE_TYPE_ATTACHMENT_SAMPLE_COUNT_INFO_AMD
template <> struct LvlTypeMap<VkAttachmentSampleCountInfoAMD> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ATTACHMENT_SAMPLE_COUNT_INFO_AMD;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ATTACHMENT_SAMPLE_COUNT_INFO_AMD> {
    typedef VkAttachmentSampleCountInfoAMD Type;
};

// Map type VkSampleLocationsInfoEXT to id VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT
template <> struct LvlTypeMap<VkSampleLocationsInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT> {
    typedef VkSampleLocationsInfoEXT Type;
};

// Map type VkRenderPassSampleLocationsBeginInfoEXT to id VK_STRUCTURE_TYPE_RENDER_PASS_SAMPLE_LOCATIONS_BEGIN_INFO_EXT
template <> struct LvlTypeMap<VkRenderPassSampleLocationsBeginInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDER_PASS_SAMPLE_LOCATIONS_BEGIN_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDER_PASS_SAMPLE_LOCATIONS_BEGIN_INFO_EXT> {
    typedef VkRenderPassSampleLocationsBeginInfoEXT Type;
};

// Map type VkPipelineSampleLocationsStateCreateInfoEXT to id VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkPipelineSampleLocationsStateCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT> {
    typedef VkPipelineSampleLocationsStateCreateInfoEXT Type;
};

// Map type VkPhysicalDeviceSampleLocationsPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceSampleLocationsPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceSampleLocationsPropertiesEXT Type;
};

// Map type VkMultisamplePropertiesEXT to id VK_STRUCTURE_TYPE_MULTISAMPLE_PROPERTIES_EXT
template <> struct LvlTypeMap<VkMultisamplePropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MULTISAMPLE_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MULTISAMPLE_PROPERTIES_EXT> {
    typedef VkMultisamplePropertiesEXT Type;
};

// Map type VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT> {
    typedef VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT Type;
};

// Map type VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT Type;
};

// Map type VkPipelineColorBlendAdvancedStateCreateInfoEXT to id VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkPipelineColorBlendAdvancedStateCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT> {
    typedef VkPipelineColorBlendAdvancedStateCreateInfoEXT Type;
};

// Map type VkPipelineCoverageToColorStateCreateInfoNV to id VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_TO_COLOR_STATE_CREATE_INFO_NV
template <> struct LvlTypeMap<VkPipelineCoverageToColorStateCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_TO_COLOR_STATE_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_TO_COLOR_STATE_CREATE_INFO_NV> {
    typedef VkPipelineCoverageToColorStateCreateInfoNV Type;
};

// Map type VkPipelineCoverageModulationStateCreateInfoNV to id VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_MODULATION_STATE_CREATE_INFO_NV
template <> struct LvlTypeMap<VkPipelineCoverageModulationStateCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_MODULATION_STATE_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_MODULATION_STATE_CREATE_INFO_NV> {
    typedef VkPipelineCoverageModulationStateCreateInfoNV Type;
};

// Map type VkPhysicalDeviceShaderSMBuiltinsPropertiesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_PROPERTIES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceShaderSMBuiltinsPropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_PROPERTIES_NV> {
    typedef VkPhysicalDeviceShaderSMBuiltinsPropertiesNV Type;
};

// Map type VkPhysicalDeviceShaderSMBuiltinsFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceShaderSMBuiltinsFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_FEATURES_NV> {
    typedef VkPhysicalDeviceShaderSMBuiltinsFeaturesNV Type;
};

// Map type VkDrmFormatModifierPropertiesListEXT to id VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT
template <> struct LvlTypeMap<VkDrmFormatModifierPropertiesListEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT> {
    typedef VkDrmFormatModifierPropertiesListEXT Type;
};

// Map type VkPhysicalDeviceImageDrmFormatModifierInfoEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceImageDrmFormatModifierInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT> {
    typedef VkPhysicalDeviceImageDrmFormatModifierInfoEXT Type;
};

// Map type VkImageDrmFormatModifierListCreateInfoEXT to id VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkImageDrmFormatModifierListCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT> {
    typedef VkImageDrmFormatModifierListCreateInfoEXT Type;
};

// Map type VkImageDrmFormatModifierExplicitCreateInfoEXT to id VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkImageDrmFormatModifierExplicitCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT> {
    typedef VkImageDrmFormatModifierExplicitCreateInfoEXT Type;
};

// Map type VkImageDrmFormatModifierPropertiesEXT to id VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_PROPERTIES_EXT
template <> struct LvlTypeMap<VkImageDrmFormatModifierPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_PROPERTIES_EXT> {
    typedef VkImageDrmFormatModifierPropertiesEXT Type;
};

// Map type VkDrmFormatModifierPropertiesList2EXT to id VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_2_EXT
template <> struct LvlTypeMap<VkDrmFormatModifierPropertiesList2EXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_2_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_2_EXT> {
    typedef VkDrmFormatModifierPropertiesList2EXT Type;
};

// Map type VkValidationCacheCreateInfoEXT to id VK_STRUCTURE_TYPE_VALIDATION_CACHE_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkValidationCacheCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VALIDATION_CACHE_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VALIDATION_CACHE_CREATE_INFO_EXT> {
    typedef VkValidationCacheCreateInfoEXT Type;
};

// Map type VkShaderModuleValidationCacheCreateInfoEXT to id VK_STRUCTURE_TYPE_SHADER_MODULE_VALIDATION_CACHE_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkShaderModuleValidationCacheCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SHADER_MODULE_VALIDATION_CACHE_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SHADER_MODULE_VALIDATION_CACHE_CREATE_INFO_EXT> {
    typedef VkShaderModuleValidationCacheCreateInfoEXT Type;
};

// Map type VkPipelineViewportShadingRateImageStateCreateInfoNV to id VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV
template <> struct LvlTypeMap<VkPipelineViewportShadingRateImageStateCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV> {
    typedef VkPipelineViewportShadingRateImageStateCreateInfoNV Type;
};

// Map type VkPhysicalDeviceShadingRateImageFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceShadingRateImageFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_FEATURES_NV> {
    typedef VkPhysicalDeviceShadingRateImageFeaturesNV Type;
};

// Map type VkPhysicalDeviceShadingRateImagePropertiesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_PROPERTIES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceShadingRateImagePropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_PROPERTIES_NV> {
    typedef VkPhysicalDeviceShadingRateImagePropertiesNV Type;
};

// Map type VkPipelineViewportCoarseSampleOrderStateCreateInfoNV to id VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_COARSE_SAMPLE_ORDER_STATE_CREATE_INFO_NV
template <> struct LvlTypeMap<VkPipelineViewportCoarseSampleOrderStateCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_COARSE_SAMPLE_ORDER_STATE_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_COARSE_SAMPLE_ORDER_STATE_CREATE_INFO_NV> {
    typedef VkPipelineViewportCoarseSampleOrderStateCreateInfoNV Type;
};

// Map type VkRayTracingShaderGroupCreateInfoNV to id VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV
template <> struct LvlTypeMap<VkRayTracingShaderGroupCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV> {
    typedef VkRayTracingShaderGroupCreateInfoNV Type;
};

// Map type VkRayTracingPipelineCreateInfoNV to id VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV
template <> struct LvlTypeMap<VkRayTracingPipelineCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV> {
    typedef VkRayTracingPipelineCreateInfoNV Type;
};

// Map type VkGeometryTrianglesNV to id VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV
template <> struct LvlTypeMap<VkGeometryTrianglesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV> {
    typedef VkGeometryTrianglesNV Type;
};

// Map type VkGeometryAABBNV to id VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV
template <> struct LvlTypeMap<VkGeometryAABBNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV> {
    typedef VkGeometryAABBNV Type;
};

// Map type VkGeometryNV to id VK_STRUCTURE_TYPE_GEOMETRY_NV
template <> struct LvlTypeMap<VkGeometryNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_GEOMETRY_NV> {
    typedef VkGeometryNV Type;
};

// Map type VkAccelerationStructureInfoNV to id VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV
template <> struct LvlTypeMap<VkAccelerationStructureInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV> {
    typedef VkAccelerationStructureInfoNV Type;
};

// Map type VkAccelerationStructureCreateInfoNV to id VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV
template <> struct LvlTypeMap<VkAccelerationStructureCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV> {
    typedef VkAccelerationStructureCreateInfoNV Type;
};

// Map type VkBindAccelerationStructureMemoryInfoNV to id VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV
template <> struct LvlTypeMap<VkBindAccelerationStructureMemoryInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV> {
    typedef VkBindAccelerationStructureMemoryInfoNV Type;
};

// Map type VkWriteDescriptorSetAccelerationStructureNV to id VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV
template <> struct LvlTypeMap<VkWriteDescriptorSetAccelerationStructureNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV> {
    typedef VkWriteDescriptorSetAccelerationStructureNV Type;
};

// Map type VkAccelerationStructureMemoryRequirementsInfoNV to id VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV
template <> struct LvlTypeMap<VkAccelerationStructureMemoryRequirementsInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV> {
    typedef VkAccelerationStructureMemoryRequirementsInfoNV Type;
};

// Map type VkPhysicalDeviceRayTracingPropertiesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceRayTracingPropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV> {
    typedef VkPhysicalDeviceRayTracingPropertiesNV Type;
};

// Map type VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_REPRESENTATIVE_FRAGMENT_TEST_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_REPRESENTATIVE_FRAGMENT_TEST_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_REPRESENTATIVE_FRAGMENT_TEST_FEATURES_NV> {
    typedef VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV Type;
};

// Map type VkPipelineRepresentativeFragmentTestStateCreateInfoNV to id VK_STRUCTURE_TYPE_PIPELINE_REPRESENTATIVE_FRAGMENT_TEST_STATE_CREATE_INFO_NV
template <> struct LvlTypeMap<VkPipelineRepresentativeFragmentTestStateCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_REPRESENTATIVE_FRAGMENT_TEST_STATE_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_REPRESENTATIVE_FRAGMENT_TEST_STATE_CREATE_INFO_NV> {
    typedef VkPipelineRepresentativeFragmentTestStateCreateInfoNV Type;
};

// Map type VkPhysicalDeviceImageViewImageFormatInfoEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_IMAGE_FORMAT_INFO_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceImageViewImageFormatInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_IMAGE_FORMAT_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_IMAGE_FORMAT_INFO_EXT> {
    typedef VkPhysicalDeviceImageViewImageFormatInfoEXT Type;
};

// Map type VkFilterCubicImageViewImageFormatPropertiesEXT to id VK_STRUCTURE_TYPE_FILTER_CUBIC_IMAGE_VIEW_IMAGE_FORMAT_PROPERTIES_EXT
template <> struct LvlTypeMap<VkFilterCubicImageViewImageFormatPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_FILTER_CUBIC_IMAGE_VIEW_IMAGE_FORMAT_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_FILTER_CUBIC_IMAGE_VIEW_IMAGE_FORMAT_PROPERTIES_EXT> {
    typedef VkFilterCubicImageViewImageFormatPropertiesEXT Type;
};

// Map type VkImportMemoryHostPointerInfoEXT to id VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT
template <> struct LvlTypeMap<VkImportMemoryHostPointerInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT> {
    typedef VkImportMemoryHostPointerInfoEXT Type;
};

// Map type VkMemoryHostPointerPropertiesEXT to id VK_STRUCTURE_TYPE_MEMORY_HOST_POINTER_PROPERTIES_EXT
template <> struct LvlTypeMap<VkMemoryHostPointerPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_HOST_POINTER_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_HOST_POINTER_PROPERTIES_EXT> {
    typedef VkMemoryHostPointerPropertiesEXT Type;
};

// Map type VkPhysicalDeviceExternalMemoryHostPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceExternalMemoryHostPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceExternalMemoryHostPropertiesEXT Type;
};

// Map type VkPipelineCompilerControlCreateInfoAMD to id VK_STRUCTURE_TYPE_PIPELINE_COMPILER_CONTROL_CREATE_INFO_AMD
template <> struct LvlTypeMap<VkPipelineCompilerControlCreateInfoAMD> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_COMPILER_CONTROL_CREATE_INFO_AMD;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_COMPILER_CONTROL_CREATE_INFO_AMD> {
    typedef VkPipelineCompilerControlCreateInfoAMD Type;
};

// Map type VkPhysicalDeviceShaderCorePropertiesAMD to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_AMD
template <> struct LvlTypeMap<VkPhysicalDeviceShaderCorePropertiesAMD> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_AMD;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_AMD> {
    typedef VkPhysicalDeviceShaderCorePropertiesAMD Type;
};

// Map type VkDeviceMemoryOverallocationCreateInfoAMD to id VK_STRUCTURE_TYPE_DEVICE_MEMORY_OVERALLOCATION_CREATE_INFO_AMD
template <> struct LvlTypeMap<VkDeviceMemoryOverallocationCreateInfoAMD> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_MEMORY_OVERALLOCATION_CREATE_INFO_AMD;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_MEMORY_OVERALLOCATION_CREATE_INFO_AMD> {
    typedef VkDeviceMemoryOverallocationCreateInfoAMD Type;
};

// Map type VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT Type;
};

#ifdef VK_USE_PLATFORM_GGP
// Map type VkPresentFrameTokenGGP to id VK_STRUCTURE_TYPE_PRESENT_FRAME_TOKEN_GGP
template <> struct LvlTypeMap<VkPresentFrameTokenGGP> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PRESENT_FRAME_TOKEN_GGP;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PRESENT_FRAME_TOKEN_GGP> {
    typedef VkPresentFrameTokenGGP Type;
};

#endif // VK_USE_PLATFORM_GGP
// Map type VkPhysicalDeviceMeshShaderFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceMeshShaderFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV> {
    typedef VkPhysicalDeviceMeshShaderFeaturesNV Type;
};

// Map type VkPhysicalDeviceMeshShaderPropertiesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceMeshShaderPropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV> {
    typedef VkPhysicalDeviceMeshShaderPropertiesNV Type;
};

// Map type VkPhysicalDeviceShaderImageFootprintFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_FOOTPRINT_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceShaderImageFootprintFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_FOOTPRINT_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_FOOTPRINT_FEATURES_NV> {
    typedef VkPhysicalDeviceShaderImageFootprintFeaturesNV Type;
};

// Map type VkPipelineViewportExclusiveScissorStateCreateInfoNV to id VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_EXCLUSIVE_SCISSOR_STATE_CREATE_INFO_NV
template <> struct LvlTypeMap<VkPipelineViewportExclusiveScissorStateCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_EXCLUSIVE_SCISSOR_STATE_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_EXCLUSIVE_SCISSOR_STATE_CREATE_INFO_NV> {
    typedef VkPipelineViewportExclusiveScissorStateCreateInfoNV Type;
};

// Map type VkPhysicalDeviceExclusiveScissorFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXCLUSIVE_SCISSOR_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceExclusiveScissorFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXCLUSIVE_SCISSOR_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXCLUSIVE_SCISSOR_FEATURES_NV> {
    typedef VkPhysicalDeviceExclusiveScissorFeaturesNV Type;
};

// Map type VkQueueFamilyCheckpointPropertiesNV to id VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_NV
template <> struct LvlTypeMap<VkQueueFamilyCheckpointPropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_NV> {
    typedef VkQueueFamilyCheckpointPropertiesNV Type;
};

// Map type VkCheckpointDataNV to id VK_STRUCTURE_TYPE_CHECKPOINT_DATA_NV
template <> struct LvlTypeMap<VkCheckpointDataNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_CHECKPOINT_DATA_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_CHECKPOINT_DATA_NV> {
    typedef VkCheckpointDataNV Type;
};

// Map type VkQueueFamilyCheckpointProperties2NV to id VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_2_NV
template <> struct LvlTypeMap<VkQueueFamilyCheckpointProperties2NV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_2_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_2_NV> {
    typedef VkQueueFamilyCheckpointProperties2NV Type;
};

// Map type VkCheckpointData2NV to id VK_STRUCTURE_TYPE_CHECKPOINT_DATA_2_NV
template <> struct LvlTypeMap<VkCheckpointData2NV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_CHECKPOINT_DATA_2_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_CHECKPOINT_DATA_2_NV> {
    typedef VkCheckpointData2NV Type;
};

// Map type VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_FUNCTIONS_2_FEATURES_INTEL
template <> struct LvlTypeMap<VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_FUNCTIONS_2_FEATURES_INTEL;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_FUNCTIONS_2_FEATURES_INTEL> {
    typedef VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL Type;
};

// Map type VkInitializePerformanceApiInfoINTEL to id VK_STRUCTURE_TYPE_INITIALIZE_PERFORMANCE_API_INFO_INTEL
template <> struct LvlTypeMap<VkInitializePerformanceApiInfoINTEL> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_INITIALIZE_PERFORMANCE_API_INFO_INTEL;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_INITIALIZE_PERFORMANCE_API_INFO_INTEL> {
    typedef VkInitializePerformanceApiInfoINTEL Type;
};

// Map type VkQueryPoolPerformanceQueryCreateInfoINTEL to id VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_QUERY_CREATE_INFO_INTEL
template <> struct LvlTypeMap<VkQueryPoolPerformanceQueryCreateInfoINTEL> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_QUERY_CREATE_INFO_INTEL;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_QUERY_CREATE_INFO_INTEL> {
    typedef VkQueryPoolPerformanceQueryCreateInfoINTEL Type;
};

// Map type VkPerformanceMarkerInfoINTEL to id VK_STRUCTURE_TYPE_PERFORMANCE_MARKER_INFO_INTEL
template <> struct LvlTypeMap<VkPerformanceMarkerInfoINTEL> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PERFORMANCE_MARKER_INFO_INTEL;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PERFORMANCE_MARKER_INFO_INTEL> {
    typedef VkPerformanceMarkerInfoINTEL Type;
};

// Map type VkPerformanceStreamMarkerInfoINTEL to id VK_STRUCTURE_TYPE_PERFORMANCE_STREAM_MARKER_INFO_INTEL
template <> struct LvlTypeMap<VkPerformanceStreamMarkerInfoINTEL> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PERFORMANCE_STREAM_MARKER_INFO_INTEL;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PERFORMANCE_STREAM_MARKER_INFO_INTEL> {
    typedef VkPerformanceStreamMarkerInfoINTEL Type;
};

// Map type VkPerformanceOverrideInfoINTEL to id VK_STRUCTURE_TYPE_PERFORMANCE_OVERRIDE_INFO_INTEL
template <> struct LvlTypeMap<VkPerformanceOverrideInfoINTEL> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PERFORMANCE_OVERRIDE_INFO_INTEL;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PERFORMANCE_OVERRIDE_INFO_INTEL> {
    typedef VkPerformanceOverrideInfoINTEL Type;
};

// Map type VkPerformanceConfigurationAcquireInfoINTEL to id VK_STRUCTURE_TYPE_PERFORMANCE_CONFIGURATION_ACQUIRE_INFO_INTEL
template <> struct LvlTypeMap<VkPerformanceConfigurationAcquireInfoINTEL> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PERFORMANCE_CONFIGURATION_ACQUIRE_INFO_INTEL;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PERFORMANCE_CONFIGURATION_ACQUIRE_INFO_INTEL> {
    typedef VkPerformanceConfigurationAcquireInfoINTEL Type;
};

// Map type VkPhysicalDevicePCIBusInfoPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDevicePCIBusInfoPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT> {
    typedef VkPhysicalDevicePCIBusInfoPropertiesEXT Type;
};

// Map type VkDisplayNativeHdrSurfaceCapabilitiesAMD to id VK_STRUCTURE_TYPE_DISPLAY_NATIVE_HDR_SURFACE_CAPABILITIES_AMD
template <> struct LvlTypeMap<VkDisplayNativeHdrSurfaceCapabilitiesAMD> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DISPLAY_NATIVE_HDR_SURFACE_CAPABILITIES_AMD;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DISPLAY_NATIVE_HDR_SURFACE_CAPABILITIES_AMD> {
    typedef VkDisplayNativeHdrSurfaceCapabilitiesAMD Type;
};

// Map type VkSwapchainDisplayNativeHdrCreateInfoAMD to id VK_STRUCTURE_TYPE_SWAPCHAIN_DISPLAY_NATIVE_HDR_CREATE_INFO_AMD
template <> struct LvlTypeMap<VkSwapchainDisplayNativeHdrCreateInfoAMD> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SWAPCHAIN_DISPLAY_NATIVE_HDR_CREATE_INFO_AMD;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SWAPCHAIN_DISPLAY_NATIVE_HDR_CREATE_INFO_AMD> {
    typedef VkSwapchainDisplayNativeHdrCreateInfoAMD Type;
};

#ifdef VK_USE_PLATFORM_FUCHSIA
// Map type VkImagePipeSurfaceCreateInfoFUCHSIA to id VK_STRUCTURE_TYPE_IMAGEPIPE_SURFACE_CREATE_INFO_FUCHSIA
template <> struct LvlTypeMap<VkImagePipeSurfaceCreateInfoFUCHSIA> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGEPIPE_SURFACE_CREATE_INFO_FUCHSIA;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGEPIPE_SURFACE_CREATE_INFO_FUCHSIA> {
    typedef VkImagePipeSurfaceCreateInfoFUCHSIA Type;
};

#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT
// Map type VkMetalSurfaceCreateInfoEXT to id VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkMetalSurfaceCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT> {
    typedef VkMetalSurfaceCreateInfoEXT Type;
};

#endif // VK_USE_PLATFORM_METAL_EXT
// Map type VkPhysicalDeviceFragmentDensityMapFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceFragmentDensityMapFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT> {
    typedef VkPhysicalDeviceFragmentDensityMapFeaturesEXT Type;
};

// Map type VkPhysicalDeviceFragmentDensityMapPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceFragmentDensityMapPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceFragmentDensityMapPropertiesEXT Type;
};

// Map type VkRenderPassFragmentDensityMapCreateInfoEXT to id VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkRenderPassFragmentDensityMapCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT> {
    typedef VkRenderPassFragmentDensityMapCreateInfoEXT Type;
};

// Map type VkRenderingFragmentDensityMapAttachmentInfoEXT to id VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_DENSITY_MAP_ATTACHMENT_INFO_EXT
template <> struct LvlTypeMap<VkRenderingFragmentDensityMapAttachmentInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_DENSITY_MAP_ATTACHMENT_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_DENSITY_MAP_ATTACHMENT_INFO_EXT> {
    typedef VkRenderingFragmentDensityMapAttachmentInfoEXT Type;
};

// Map type VkPhysicalDeviceShaderCoreProperties2AMD to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_2_AMD
template <> struct LvlTypeMap<VkPhysicalDeviceShaderCoreProperties2AMD> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_2_AMD;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_2_AMD> {
    typedef VkPhysicalDeviceShaderCoreProperties2AMD Type;
};

// Map type VkPhysicalDeviceCoherentMemoryFeaturesAMD to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COHERENT_MEMORY_FEATURES_AMD
template <> struct LvlTypeMap<VkPhysicalDeviceCoherentMemoryFeaturesAMD> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COHERENT_MEMORY_FEATURES_AMD;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COHERENT_MEMORY_FEATURES_AMD> {
    typedef VkPhysicalDeviceCoherentMemoryFeaturesAMD Type;
};

// Map type VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT> {
    typedef VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT Type;
};

// Map type VkPhysicalDeviceMemoryBudgetPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceMemoryBudgetPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceMemoryBudgetPropertiesEXT Type;
};

// Map type VkPhysicalDeviceMemoryPriorityFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceMemoryPriorityFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT> {
    typedef VkPhysicalDeviceMemoryPriorityFeaturesEXT Type;
};

// Map type VkMemoryPriorityAllocateInfoEXT to id VK_STRUCTURE_TYPE_MEMORY_PRIORITY_ALLOCATE_INFO_EXT
template <> struct LvlTypeMap<VkMemoryPriorityAllocateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_PRIORITY_ALLOCATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_PRIORITY_ALLOCATE_INFO_EXT> {
    typedef VkMemoryPriorityAllocateInfoEXT Type;
};

// Map type VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEDICATED_ALLOCATION_IMAGE_ALIASING_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEDICATED_ALLOCATION_IMAGE_ALIASING_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEDICATED_ALLOCATION_IMAGE_ALIASING_FEATURES_NV> {
    typedef VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV Type;
};

// Map type VkPhysicalDeviceBufferDeviceAddressFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceBufferDeviceAddressFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT> {
    typedef VkPhysicalDeviceBufferDeviceAddressFeaturesEXT Type;
};

// Map type VkBufferDeviceAddressCreateInfoEXT to id VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkBufferDeviceAddressCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_CREATE_INFO_EXT> {
    typedef VkBufferDeviceAddressCreateInfoEXT Type;
};

// Map type VkValidationFeaturesEXT to id VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT
template <> struct LvlTypeMap<VkValidationFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT> {
    typedef VkValidationFeaturesEXT Type;
};

// Map type VkCooperativeMatrixPropertiesNV to id VK_STRUCTURE_TYPE_COOPERATIVE_MATRIX_PROPERTIES_NV
template <> struct LvlTypeMap<VkCooperativeMatrixPropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COOPERATIVE_MATRIX_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COOPERATIVE_MATRIX_PROPERTIES_NV> {
    typedef VkCooperativeMatrixPropertiesNV Type;
};

// Map type VkPhysicalDeviceCooperativeMatrixFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceCooperativeMatrixFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_NV> {
    typedef VkPhysicalDeviceCooperativeMatrixFeaturesNV Type;
};

// Map type VkPhysicalDeviceCooperativeMatrixPropertiesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceCooperativeMatrixPropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_NV> {
    typedef VkPhysicalDeviceCooperativeMatrixPropertiesNV Type;
};

// Map type VkPhysicalDeviceCoverageReductionModeFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COVERAGE_REDUCTION_MODE_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceCoverageReductionModeFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COVERAGE_REDUCTION_MODE_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COVERAGE_REDUCTION_MODE_FEATURES_NV> {
    typedef VkPhysicalDeviceCoverageReductionModeFeaturesNV Type;
};

// Map type VkPipelineCoverageReductionStateCreateInfoNV to id VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_REDUCTION_STATE_CREATE_INFO_NV
template <> struct LvlTypeMap<VkPipelineCoverageReductionStateCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_REDUCTION_STATE_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_REDUCTION_STATE_CREATE_INFO_NV> {
    typedef VkPipelineCoverageReductionStateCreateInfoNV Type;
};

// Map type VkFramebufferMixedSamplesCombinationNV to id VK_STRUCTURE_TYPE_FRAMEBUFFER_MIXED_SAMPLES_COMBINATION_NV
template <> struct LvlTypeMap<VkFramebufferMixedSamplesCombinationNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_FRAMEBUFFER_MIXED_SAMPLES_COMBINATION_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_FRAMEBUFFER_MIXED_SAMPLES_COMBINATION_NV> {
    typedef VkFramebufferMixedSamplesCombinationNV Type;
};

// Map type VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT> {
    typedef VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT Type;
};

// Map type VkPhysicalDeviceYcbcrImageArraysFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_IMAGE_ARRAYS_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceYcbcrImageArraysFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_IMAGE_ARRAYS_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_IMAGE_ARRAYS_FEATURES_EXT> {
    typedef VkPhysicalDeviceYcbcrImageArraysFeaturesEXT Type;
};

// Map type VkPhysicalDeviceProvokingVertexFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceProvokingVertexFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_FEATURES_EXT> {
    typedef VkPhysicalDeviceProvokingVertexFeaturesEXT Type;
};

// Map type VkPhysicalDeviceProvokingVertexPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceProvokingVertexPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceProvokingVertexPropertiesEXT Type;
};

// Map type VkPipelineRasterizationProvokingVertexStateCreateInfoEXT to id VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_PROVOKING_VERTEX_STATE_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkPipelineRasterizationProvokingVertexStateCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_PROVOKING_VERTEX_STATE_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_PROVOKING_VERTEX_STATE_CREATE_INFO_EXT> {
    typedef VkPipelineRasterizationProvokingVertexStateCreateInfoEXT Type;
};

#ifdef VK_USE_PLATFORM_WIN32_KHR
// Map type VkSurfaceFullScreenExclusiveInfoEXT to id VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT
template <> struct LvlTypeMap<VkSurfaceFullScreenExclusiveInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT> {
    typedef VkSurfaceFullScreenExclusiveInfoEXT Type;
};

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
// Map type VkSurfaceCapabilitiesFullScreenExclusiveEXT to id VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_FULL_SCREEN_EXCLUSIVE_EXT
template <> struct LvlTypeMap<VkSurfaceCapabilitiesFullScreenExclusiveEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_FULL_SCREEN_EXCLUSIVE_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_FULL_SCREEN_EXCLUSIVE_EXT> {
    typedef VkSurfaceCapabilitiesFullScreenExclusiveEXT Type;
};

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
// Map type VkSurfaceFullScreenExclusiveWin32InfoEXT to id VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT
template <> struct LvlTypeMap<VkSurfaceFullScreenExclusiveWin32InfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT> {
    typedef VkSurfaceFullScreenExclusiveWin32InfoEXT Type;
};

#endif // VK_USE_PLATFORM_WIN32_KHR
// Map type VkHeadlessSurfaceCreateInfoEXT to id VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkHeadlessSurfaceCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT> {
    typedef VkHeadlessSurfaceCreateInfoEXT Type;
};

// Map type VkPhysicalDeviceShaderAtomicFloatFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT> {
    typedef VkPhysicalDeviceShaderAtomicFloatFeaturesEXT Type;
};

// Map type VkPhysicalDeviceExtendedDynamicStateFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT> {
    typedef VkPhysicalDeviceExtendedDynamicStateFeaturesEXT Type;
};

// Map type VkPhysicalDeviceMapMemoryPlacedFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAP_MEMORY_PLACED_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceMapMemoryPlacedFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAP_MEMORY_PLACED_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAP_MEMORY_PLACED_FEATURES_EXT> {
    typedef VkPhysicalDeviceMapMemoryPlacedFeaturesEXT Type;
};

// Map type VkPhysicalDeviceMapMemoryPlacedPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAP_MEMORY_PLACED_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceMapMemoryPlacedPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAP_MEMORY_PLACED_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAP_MEMORY_PLACED_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceMapMemoryPlacedPropertiesEXT Type;
};

// Map type VkMemoryMapPlacedInfoEXT to id VK_STRUCTURE_TYPE_MEMORY_MAP_PLACED_INFO_EXT
template <> struct LvlTypeMap<VkMemoryMapPlacedInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_MAP_PLACED_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_MAP_PLACED_INFO_EXT> {
    typedef VkMemoryMapPlacedInfoEXT Type;
};

// Map type VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_2_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_2_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_2_FEATURES_EXT> {
    typedef VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT Type;
};

// Map type VkSurfacePresentModeEXT to id VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_EXT
template <> struct LvlTypeMap<VkSurfacePresentModeEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_EXT> {
    typedef VkSurfacePresentModeEXT Type;
};

// Map type VkSurfacePresentScalingCapabilitiesEXT to id VK_STRUCTURE_TYPE_SURFACE_PRESENT_SCALING_CAPABILITIES_EXT
template <> struct LvlTypeMap<VkSurfacePresentScalingCapabilitiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SURFACE_PRESENT_SCALING_CAPABILITIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SURFACE_PRESENT_SCALING_CAPABILITIES_EXT> {
    typedef VkSurfacePresentScalingCapabilitiesEXT Type;
};

// Map type VkSurfacePresentModeCompatibilityEXT to id VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_COMPATIBILITY_EXT
template <> struct LvlTypeMap<VkSurfacePresentModeCompatibilityEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_COMPATIBILITY_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_COMPATIBILITY_EXT> {
    typedef VkSurfacePresentModeCompatibilityEXT Type;
};

// Map type VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT> {
    typedef VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT Type;
};

// Map type VkSwapchainPresentFenceInfoEXT to id VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_EXT
template <> struct LvlTypeMap<VkSwapchainPresentFenceInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_EXT> {
    typedef VkSwapchainPresentFenceInfoEXT Type;
};

// Map type VkSwapchainPresentModesCreateInfoEXT to id VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_MODES_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkSwapchainPresentModesCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_MODES_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_MODES_CREATE_INFO_EXT> {
    typedef VkSwapchainPresentModesCreateInfoEXT Type;
};

// Map type VkSwapchainPresentModeInfoEXT to id VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_MODE_INFO_EXT
template <> struct LvlTypeMap<VkSwapchainPresentModeInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_MODE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_MODE_INFO_EXT> {
    typedef VkSwapchainPresentModeInfoEXT Type;
};

// Map type VkSwapchainPresentScalingCreateInfoEXT to id VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_SCALING_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkSwapchainPresentScalingCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_SCALING_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_SCALING_CREATE_INFO_EXT> {
    typedef VkSwapchainPresentScalingCreateInfoEXT Type;
};

// Map type VkReleaseSwapchainImagesInfoEXT to id VK_STRUCTURE_TYPE_RELEASE_SWAPCHAIN_IMAGES_INFO_EXT
template <> struct LvlTypeMap<VkReleaseSwapchainImagesInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RELEASE_SWAPCHAIN_IMAGES_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RELEASE_SWAPCHAIN_IMAGES_INFO_EXT> {
    typedef VkReleaseSwapchainImagesInfoEXT Type;
};

// Map type VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_PROPERTIES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_PROPERTIES_NV> {
    typedef VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV Type;
};

// Map type VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_NV> {
    typedef VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV Type;
};

// Map type VkGraphicsShaderGroupCreateInfoNV to id VK_STRUCTURE_TYPE_GRAPHICS_SHADER_GROUP_CREATE_INFO_NV
template <> struct LvlTypeMap<VkGraphicsShaderGroupCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_GRAPHICS_SHADER_GROUP_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_GRAPHICS_SHADER_GROUP_CREATE_INFO_NV> {
    typedef VkGraphicsShaderGroupCreateInfoNV Type;
};

// Map type VkGraphicsPipelineShaderGroupsCreateInfoNV to id VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_SHADER_GROUPS_CREATE_INFO_NV
template <> struct LvlTypeMap<VkGraphicsPipelineShaderGroupsCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_SHADER_GROUPS_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_SHADER_GROUPS_CREATE_INFO_NV> {
    typedef VkGraphicsPipelineShaderGroupsCreateInfoNV Type;
};

// Map type VkIndirectCommandsLayoutTokenNV to id VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_TOKEN_NV
template <> struct LvlTypeMap<VkIndirectCommandsLayoutTokenNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_TOKEN_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_TOKEN_NV> {
    typedef VkIndirectCommandsLayoutTokenNV Type;
};

// Map type VkIndirectCommandsLayoutCreateInfoNV to id VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_CREATE_INFO_NV
template <> struct LvlTypeMap<VkIndirectCommandsLayoutCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_CREATE_INFO_NV> {
    typedef VkIndirectCommandsLayoutCreateInfoNV Type;
};

// Map type VkGeneratedCommandsInfoNV to id VK_STRUCTURE_TYPE_GENERATED_COMMANDS_INFO_NV
template <> struct LvlTypeMap<VkGeneratedCommandsInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_GENERATED_COMMANDS_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_GENERATED_COMMANDS_INFO_NV> {
    typedef VkGeneratedCommandsInfoNV Type;
};

// Map type VkGeneratedCommandsMemoryRequirementsInfoNV to id VK_STRUCTURE_TYPE_GENERATED_COMMANDS_MEMORY_REQUIREMENTS_INFO_NV
template <> struct LvlTypeMap<VkGeneratedCommandsMemoryRequirementsInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_GENERATED_COMMANDS_MEMORY_REQUIREMENTS_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_GENERATED_COMMANDS_MEMORY_REQUIREMENTS_INFO_NV> {
    typedef VkGeneratedCommandsMemoryRequirementsInfoNV Type;
};

// Map type VkPhysicalDeviceInheritedViewportScissorFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INHERITED_VIEWPORT_SCISSOR_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceInheritedViewportScissorFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INHERITED_VIEWPORT_SCISSOR_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INHERITED_VIEWPORT_SCISSOR_FEATURES_NV> {
    typedef VkPhysicalDeviceInheritedViewportScissorFeaturesNV Type;
};

// Map type VkCommandBufferInheritanceViewportScissorInfoNV to id VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_VIEWPORT_SCISSOR_INFO_NV
template <> struct LvlTypeMap<VkCommandBufferInheritanceViewportScissorInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_VIEWPORT_SCISSOR_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_VIEWPORT_SCISSOR_INFO_NV> {
    typedef VkCommandBufferInheritanceViewportScissorInfoNV Type;
};

// Map type VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT> {
    typedef VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT Type;
};

// Map type VkRenderPassTransformBeginInfoQCOM to id VK_STRUCTURE_TYPE_RENDER_PASS_TRANSFORM_BEGIN_INFO_QCOM
template <> struct LvlTypeMap<VkRenderPassTransformBeginInfoQCOM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDER_PASS_TRANSFORM_BEGIN_INFO_QCOM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDER_PASS_TRANSFORM_BEGIN_INFO_QCOM> {
    typedef VkRenderPassTransformBeginInfoQCOM Type;
};

// Map type VkCommandBufferInheritanceRenderPassTransformInfoQCOM to id VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDER_PASS_TRANSFORM_INFO_QCOM
template <> struct LvlTypeMap<VkCommandBufferInheritanceRenderPassTransformInfoQCOM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDER_PASS_TRANSFORM_INFO_QCOM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDER_PASS_TRANSFORM_INFO_QCOM> {
    typedef VkCommandBufferInheritanceRenderPassTransformInfoQCOM Type;
};

// Map type VkPhysicalDeviceDepthBiasControlFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_BIAS_CONTROL_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceDepthBiasControlFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_BIAS_CONTROL_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_BIAS_CONTROL_FEATURES_EXT> {
    typedef VkPhysicalDeviceDepthBiasControlFeaturesEXT Type;
};

// Map type VkDepthBiasInfoEXT to id VK_STRUCTURE_TYPE_DEPTH_BIAS_INFO_EXT
template <> struct LvlTypeMap<VkDepthBiasInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEPTH_BIAS_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEPTH_BIAS_INFO_EXT> {
    typedef VkDepthBiasInfoEXT Type;
};

// Map type VkDepthBiasRepresentationInfoEXT to id VK_STRUCTURE_TYPE_DEPTH_BIAS_REPRESENTATION_INFO_EXT
template <> struct LvlTypeMap<VkDepthBiasRepresentationInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEPTH_BIAS_REPRESENTATION_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEPTH_BIAS_REPRESENTATION_INFO_EXT> {
    typedef VkDepthBiasRepresentationInfoEXT Type;
};

// Map type VkPhysicalDeviceDeviceMemoryReportFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_MEMORY_REPORT_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceDeviceMemoryReportFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_MEMORY_REPORT_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_MEMORY_REPORT_FEATURES_EXT> {
    typedef VkPhysicalDeviceDeviceMemoryReportFeaturesEXT Type;
};

// Map type VkDeviceMemoryReportCallbackDataEXT to id VK_STRUCTURE_TYPE_DEVICE_MEMORY_REPORT_CALLBACK_DATA_EXT
template <> struct LvlTypeMap<VkDeviceMemoryReportCallbackDataEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_MEMORY_REPORT_CALLBACK_DATA_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_MEMORY_REPORT_CALLBACK_DATA_EXT> {
    typedef VkDeviceMemoryReportCallbackDataEXT Type;
};

// Map type VkDeviceDeviceMemoryReportCreateInfoEXT to id VK_STRUCTURE_TYPE_DEVICE_DEVICE_MEMORY_REPORT_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkDeviceDeviceMemoryReportCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_DEVICE_MEMORY_REPORT_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_DEVICE_MEMORY_REPORT_CREATE_INFO_EXT> {
    typedef VkDeviceDeviceMemoryReportCreateInfoEXT Type;
};

// Map type VkPhysicalDeviceRobustness2FeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceRobustness2FeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT> {
    typedef VkPhysicalDeviceRobustness2FeaturesEXT Type;
};

// Map type VkPhysicalDeviceRobustness2PropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceRobustness2PropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceRobustness2PropertiesEXT Type;
};

// Map type VkSamplerCustomBorderColorCreateInfoEXT to id VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkSamplerCustomBorderColorCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT> {
    typedef VkSamplerCustomBorderColorCreateInfoEXT Type;
};

// Map type VkPhysicalDeviceCustomBorderColorPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceCustomBorderColorPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceCustomBorderColorPropertiesEXT Type;
};

// Map type VkPhysicalDeviceCustomBorderColorFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceCustomBorderColorFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT> {
    typedef VkPhysicalDeviceCustomBorderColorFeaturesEXT Type;
};

// Map type VkPhysicalDevicePresentBarrierFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_BARRIER_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDevicePresentBarrierFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_BARRIER_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_BARRIER_FEATURES_NV> {
    typedef VkPhysicalDevicePresentBarrierFeaturesNV Type;
};

// Map type VkSurfaceCapabilitiesPresentBarrierNV to id VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_PRESENT_BARRIER_NV
template <> struct LvlTypeMap<VkSurfaceCapabilitiesPresentBarrierNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_PRESENT_BARRIER_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_PRESENT_BARRIER_NV> {
    typedef VkSurfaceCapabilitiesPresentBarrierNV Type;
};

// Map type VkSwapchainPresentBarrierCreateInfoNV to id VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_BARRIER_CREATE_INFO_NV
template <> struct LvlTypeMap<VkSwapchainPresentBarrierCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_BARRIER_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_BARRIER_CREATE_INFO_NV> {
    typedef VkSwapchainPresentBarrierCreateInfoNV Type;
};

// Map type VkPhysicalDeviceDiagnosticsConfigFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DIAGNOSTICS_CONFIG_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceDiagnosticsConfigFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DIAGNOSTICS_CONFIG_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DIAGNOSTICS_CONFIG_FEATURES_NV> {
    typedef VkPhysicalDeviceDiagnosticsConfigFeaturesNV Type;
};

// Map type VkDeviceDiagnosticsConfigCreateInfoNV to id VK_STRUCTURE_TYPE_DEVICE_DIAGNOSTICS_CONFIG_CREATE_INFO_NV
template <> struct LvlTypeMap<VkDeviceDiagnosticsConfigCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_DIAGNOSTICS_CONFIG_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_DIAGNOSTICS_CONFIG_CREATE_INFO_NV> {
    typedef VkDeviceDiagnosticsConfigCreateInfoNV Type;
};

// Map type VkCudaModuleCreateInfoNV to id VK_STRUCTURE_TYPE_CUDA_MODULE_CREATE_INFO_NV
template <> struct LvlTypeMap<VkCudaModuleCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_CUDA_MODULE_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_CUDA_MODULE_CREATE_INFO_NV> {
    typedef VkCudaModuleCreateInfoNV Type;
};

// Map type VkCudaFunctionCreateInfoNV to id VK_STRUCTURE_TYPE_CUDA_FUNCTION_CREATE_INFO_NV
template <> struct LvlTypeMap<VkCudaFunctionCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_CUDA_FUNCTION_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_CUDA_FUNCTION_CREATE_INFO_NV> {
    typedef VkCudaFunctionCreateInfoNV Type;
};

// Map type VkCudaLaunchInfoNV to id VK_STRUCTURE_TYPE_CUDA_LAUNCH_INFO_NV
template <> struct LvlTypeMap<VkCudaLaunchInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_CUDA_LAUNCH_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_CUDA_LAUNCH_INFO_NV> {
    typedef VkCudaLaunchInfoNV Type;
};

// Map type VkPhysicalDeviceCudaKernelLaunchFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUDA_KERNEL_LAUNCH_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceCudaKernelLaunchFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUDA_KERNEL_LAUNCH_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUDA_KERNEL_LAUNCH_FEATURES_NV> {
    typedef VkPhysicalDeviceCudaKernelLaunchFeaturesNV Type;
};

// Map type VkPhysicalDeviceCudaKernelLaunchPropertiesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUDA_KERNEL_LAUNCH_PROPERTIES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceCudaKernelLaunchPropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUDA_KERNEL_LAUNCH_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUDA_KERNEL_LAUNCH_PROPERTIES_NV> {
    typedef VkPhysicalDeviceCudaKernelLaunchPropertiesNV Type;
};

// Map type VkQueryLowLatencySupportNV to id VK_STRUCTURE_TYPE_QUERY_LOW_LATENCY_SUPPORT_NV
template <> struct LvlTypeMap<VkQueryLowLatencySupportNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_QUERY_LOW_LATENCY_SUPPORT_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_QUERY_LOW_LATENCY_SUPPORT_NV> {
    typedef VkQueryLowLatencySupportNV Type;
};

#ifdef VK_USE_PLATFORM_METAL_EXT
// Map type VkExportMetalObjectCreateInfoEXT to id VK_STRUCTURE_TYPE_EXPORT_METAL_OBJECT_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkExportMetalObjectCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXPORT_METAL_OBJECT_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXPORT_METAL_OBJECT_CREATE_INFO_EXT> {
    typedef VkExportMetalObjectCreateInfoEXT Type;
};

#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
// Map type VkExportMetalObjectsInfoEXT to id VK_STRUCTURE_TYPE_EXPORT_METAL_OBJECTS_INFO_EXT
template <> struct LvlTypeMap<VkExportMetalObjectsInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXPORT_METAL_OBJECTS_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXPORT_METAL_OBJECTS_INFO_EXT> {
    typedef VkExportMetalObjectsInfoEXT Type;
};

#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
// Map type VkExportMetalDeviceInfoEXT to id VK_STRUCTURE_TYPE_EXPORT_METAL_DEVICE_INFO_EXT
template <> struct LvlTypeMap<VkExportMetalDeviceInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXPORT_METAL_DEVICE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXPORT_METAL_DEVICE_INFO_EXT> {
    typedef VkExportMetalDeviceInfoEXT Type;
};

#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
// Map type VkExportMetalCommandQueueInfoEXT to id VK_STRUCTURE_TYPE_EXPORT_METAL_COMMAND_QUEUE_INFO_EXT
template <> struct LvlTypeMap<VkExportMetalCommandQueueInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXPORT_METAL_COMMAND_QUEUE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXPORT_METAL_COMMAND_QUEUE_INFO_EXT> {
    typedef VkExportMetalCommandQueueInfoEXT Type;
};

#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
// Map type VkExportMetalBufferInfoEXT to id VK_STRUCTURE_TYPE_EXPORT_METAL_BUFFER_INFO_EXT
template <> struct LvlTypeMap<VkExportMetalBufferInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXPORT_METAL_BUFFER_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXPORT_METAL_BUFFER_INFO_EXT> {
    typedef VkExportMetalBufferInfoEXT Type;
};

#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
// Map type VkImportMetalBufferInfoEXT to id VK_STRUCTURE_TYPE_IMPORT_METAL_BUFFER_INFO_EXT
template <> struct LvlTypeMap<VkImportMetalBufferInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_METAL_BUFFER_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_METAL_BUFFER_INFO_EXT> {
    typedef VkImportMetalBufferInfoEXT Type;
};

#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
// Map type VkExportMetalTextureInfoEXT to id VK_STRUCTURE_TYPE_EXPORT_METAL_TEXTURE_INFO_EXT
template <> struct LvlTypeMap<VkExportMetalTextureInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXPORT_METAL_TEXTURE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXPORT_METAL_TEXTURE_INFO_EXT> {
    typedef VkExportMetalTextureInfoEXT Type;
};

#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
// Map type VkImportMetalTextureInfoEXT to id VK_STRUCTURE_TYPE_IMPORT_METAL_TEXTURE_INFO_EXT
template <> struct LvlTypeMap<VkImportMetalTextureInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_METAL_TEXTURE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_METAL_TEXTURE_INFO_EXT> {
    typedef VkImportMetalTextureInfoEXT Type;
};

#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
// Map type VkExportMetalIOSurfaceInfoEXT to id VK_STRUCTURE_TYPE_EXPORT_METAL_IO_SURFACE_INFO_EXT
template <> struct LvlTypeMap<VkExportMetalIOSurfaceInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXPORT_METAL_IO_SURFACE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXPORT_METAL_IO_SURFACE_INFO_EXT> {
    typedef VkExportMetalIOSurfaceInfoEXT Type;
};

#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
// Map type VkImportMetalIOSurfaceInfoEXT to id VK_STRUCTURE_TYPE_IMPORT_METAL_IO_SURFACE_INFO_EXT
template <> struct LvlTypeMap<VkImportMetalIOSurfaceInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_METAL_IO_SURFACE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_METAL_IO_SURFACE_INFO_EXT> {
    typedef VkImportMetalIOSurfaceInfoEXT Type;
};

#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
// Map type VkExportMetalSharedEventInfoEXT to id VK_STRUCTURE_TYPE_EXPORT_METAL_SHARED_EVENT_INFO_EXT
template <> struct LvlTypeMap<VkExportMetalSharedEventInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXPORT_METAL_SHARED_EVENT_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXPORT_METAL_SHARED_EVENT_INFO_EXT> {
    typedef VkExportMetalSharedEventInfoEXT Type;
};

#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
// Map type VkImportMetalSharedEventInfoEXT to id VK_STRUCTURE_TYPE_IMPORT_METAL_SHARED_EVENT_INFO_EXT
template <> struct LvlTypeMap<VkImportMetalSharedEventInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_METAL_SHARED_EVENT_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_METAL_SHARED_EVENT_INFO_EXT> {
    typedef VkImportMetalSharedEventInfoEXT Type;
};

#endif // VK_USE_PLATFORM_METAL_EXT
// Map type VkPhysicalDeviceDescriptorBufferPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceDescriptorBufferPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceDescriptorBufferPropertiesEXT Type;
};

// Map type VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_DENSITY_MAP_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_DENSITY_MAP_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_DENSITY_MAP_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT Type;
};

// Map type VkPhysicalDeviceDescriptorBufferFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceDescriptorBufferFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT> {
    typedef VkPhysicalDeviceDescriptorBufferFeaturesEXT Type;
};

// Map type VkDescriptorAddressInfoEXT to id VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT
template <> struct LvlTypeMap<VkDescriptorAddressInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT> {
    typedef VkDescriptorAddressInfoEXT Type;
};

// Map type VkDescriptorBufferBindingInfoEXT to id VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT
template <> struct LvlTypeMap<VkDescriptorBufferBindingInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT> {
    typedef VkDescriptorBufferBindingInfoEXT Type;
};

// Map type VkDescriptorBufferBindingPushDescriptorBufferHandleEXT to id VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_PUSH_DESCRIPTOR_BUFFER_HANDLE_EXT
template <> struct LvlTypeMap<VkDescriptorBufferBindingPushDescriptorBufferHandleEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_PUSH_DESCRIPTOR_BUFFER_HANDLE_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_PUSH_DESCRIPTOR_BUFFER_HANDLE_EXT> {
    typedef VkDescriptorBufferBindingPushDescriptorBufferHandleEXT Type;
};

// Map type VkDescriptorGetInfoEXT to id VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT
template <> struct LvlTypeMap<VkDescriptorGetInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT> {
    typedef VkDescriptorGetInfoEXT Type;
};

// Map type VkBufferCaptureDescriptorDataInfoEXT to id VK_STRUCTURE_TYPE_BUFFER_CAPTURE_DESCRIPTOR_DATA_INFO_EXT
template <> struct LvlTypeMap<VkBufferCaptureDescriptorDataInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_CAPTURE_DESCRIPTOR_DATA_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_CAPTURE_DESCRIPTOR_DATA_INFO_EXT> {
    typedef VkBufferCaptureDescriptorDataInfoEXT Type;
};

// Map type VkImageCaptureDescriptorDataInfoEXT to id VK_STRUCTURE_TYPE_IMAGE_CAPTURE_DESCRIPTOR_DATA_INFO_EXT
template <> struct LvlTypeMap<VkImageCaptureDescriptorDataInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_CAPTURE_DESCRIPTOR_DATA_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_CAPTURE_DESCRIPTOR_DATA_INFO_EXT> {
    typedef VkImageCaptureDescriptorDataInfoEXT Type;
};

// Map type VkImageViewCaptureDescriptorDataInfoEXT to id VK_STRUCTURE_TYPE_IMAGE_VIEW_CAPTURE_DESCRIPTOR_DATA_INFO_EXT
template <> struct LvlTypeMap<VkImageViewCaptureDescriptorDataInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CAPTURE_DESCRIPTOR_DATA_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_VIEW_CAPTURE_DESCRIPTOR_DATA_INFO_EXT> {
    typedef VkImageViewCaptureDescriptorDataInfoEXT Type;
};

// Map type VkSamplerCaptureDescriptorDataInfoEXT to id VK_STRUCTURE_TYPE_SAMPLER_CAPTURE_DESCRIPTOR_DATA_INFO_EXT
template <> struct LvlTypeMap<VkSamplerCaptureDescriptorDataInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SAMPLER_CAPTURE_DESCRIPTOR_DATA_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SAMPLER_CAPTURE_DESCRIPTOR_DATA_INFO_EXT> {
    typedef VkSamplerCaptureDescriptorDataInfoEXT Type;
};

// Map type VkOpaqueCaptureDescriptorDataCreateInfoEXT to id VK_STRUCTURE_TYPE_OPAQUE_CAPTURE_DESCRIPTOR_DATA_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkOpaqueCaptureDescriptorDataCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_OPAQUE_CAPTURE_DESCRIPTOR_DATA_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_OPAQUE_CAPTURE_DESCRIPTOR_DATA_CREATE_INFO_EXT> {
    typedef VkOpaqueCaptureDescriptorDataCreateInfoEXT Type;
};

// Map type VkAccelerationStructureCaptureDescriptorDataInfoEXT to id VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CAPTURE_DESCRIPTOR_DATA_INFO_EXT
template <> struct LvlTypeMap<VkAccelerationStructureCaptureDescriptorDataInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CAPTURE_DESCRIPTOR_DATA_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CAPTURE_DESCRIPTOR_DATA_INFO_EXT> {
    typedef VkAccelerationStructureCaptureDescriptorDataInfoEXT Type;
};

// Map type VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT> {
    typedef VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT Type;
};

// Map type VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT Type;
};

// Map type VkGraphicsPipelineLibraryCreateInfoEXT to id VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkGraphicsPipelineLibraryCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT> {
    typedef VkGraphicsPipelineLibraryCreateInfoEXT Type;
};

// Map type VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EARLY_AND_LATE_FRAGMENT_TESTS_FEATURES_AMD
template <> struct LvlTypeMap<VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EARLY_AND_LATE_FRAGMENT_TESTS_FEATURES_AMD;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EARLY_AND_LATE_FRAGMENT_TESTS_FEATURES_AMD> {
    typedef VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD Type;
};

// Map type VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_FEATURES_NV> {
    typedef VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV Type;
};

// Map type VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_PROPERTIES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_PROPERTIES_NV> {
    typedef VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV Type;
};

// Map type VkPipelineFragmentShadingRateEnumStateCreateInfoNV to id VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_ENUM_STATE_CREATE_INFO_NV
template <> struct LvlTypeMap<VkPipelineFragmentShadingRateEnumStateCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_ENUM_STATE_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_ENUM_STATE_CREATE_INFO_NV> {
    typedef VkPipelineFragmentShadingRateEnumStateCreateInfoNV Type;
};

// Map type VkAccelerationStructureGeometryMotionTrianglesDataNV to id VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_MOTION_TRIANGLES_DATA_NV
template <> struct LvlTypeMap<VkAccelerationStructureGeometryMotionTrianglesDataNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_MOTION_TRIANGLES_DATA_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_MOTION_TRIANGLES_DATA_NV> {
    typedef VkAccelerationStructureGeometryMotionTrianglesDataNV Type;
};

// Map type VkAccelerationStructureMotionInfoNV to id VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MOTION_INFO_NV
template <> struct LvlTypeMap<VkAccelerationStructureMotionInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MOTION_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MOTION_INFO_NV> {
    typedef VkAccelerationStructureMotionInfoNV Type;
};

// Map type VkPhysicalDeviceRayTracingMotionBlurFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MOTION_BLUR_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceRayTracingMotionBlurFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MOTION_BLUR_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MOTION_BLUR_FEATURES_NV> {
    typedef VkPhysicalDeviceRayTracingMotionBlurFeaturesNV Type;
};

// Map type VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_2_PLANE_444_FORMATS_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_2_PLANE_444_FORMATS_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_2_PLANE_444_FORMATS_FEATURES_EXT> {
    typedef VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT Type;
};

// Map type VkPhysicalDeviceFragmentDensityMap2FeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceFragmentDensityMap2FeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_FEATURES_EXT> {
    typedef VkPhysicalDeviceFragmentDensityMap2FeaturesEXT Type;
};

// Map type VkPhysicalDeviceFragmentDensityMap2PropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceFragmentDensityMap2PropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceFragmentDensityMap2PropertiesEXT Type;
};

// Map type VkCopyCommandTransformInfoQCOM to id VK_STRUCTURE_TYPE_COPY_COMMAND_TRANSFORM_INFO_QCOM
template <> struct LvlTypeMap<VkCopyCommandTransformInfoQCOM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COPY_COMMAND_TRANSFORM_INFO_QCOM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COPY_COMMAND_TRANSFORM_INFO_QCOM> {
    typedef VkCopyCommandTransformInfoQCOM Type;
};

// Map type VkPhysicalDeviceImageCompressionControlFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceImageCompressionControlFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_FEATURES_EXT> {
    typedef VkPhysicalDeviceImageCompressionControlFeaturesEXT Type;
};

// Map type VkImageCompressionControlEXT to id VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_CONTROL_EXT
template <> struct LvlTypeMap<VkImageCompressionControlEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_CONTROL_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_CONTROL_EXT> {
    typedef VkImageCompressionControlEXT Type;
};

// Map type VkImageCompressionPropertiesEXT to id VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_PROPERTIES_EXT
template <> struct LvlTypeMap<VkImageCompressionPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_PROPERTIES_EXT> {
    typedef VkImageCompressionPropertiesEXT Type;
};

// Map type VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_FEATURES_EXT> {
    typedef VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT Type;
};

// Map type VkPhysicalDevice4444FormatsFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_4444_FORMATS_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDevice4444FormatsFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_4444_FORMATS_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_4444_FORMATS_FEATURES_EXT> {
    typedef VkPhysicalDevice4444FormatsFeaturesEXT Type;
};

// Map type VkPhysicalDeviceFaultFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FAULT_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceFaultFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FAULT_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FAULT_FEATURES_EXT> {
    typedef VkPhysicalDeviceFaultFeaturesEXT Type;
};

// Map type VkDeviceFaultCountsEXT to id VK_STRUCTURE_TYPE_DEVICE_FAULT_COUNTS_EXT
template <> struct LvlTypeMap<VkDeviceFaultCountsEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_FAULT_COUNTS_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_FAULT_COUNTS_EXT> {
    typedef VkDeviceFaultCountsEXT Type;
};

// Map type VkDeviceFaultInfoEXT to id VK_STRUCTURE_TYPE_DEVICE_FAULT_INFO_EXT
template <> struct LvlTypeMap<VkDeviceFaultInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_FAULT_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_FAULT_INFO_EXT> {
    typedef VkDeviceFaultInfoEXT Type;
};

// Map type VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT> {
    typedef VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT Type;
};

// Map type VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RGBA10X6_FORMATS_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RGBA10X6_FORMATS_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RGBA10X6_FORMATS_FEATURES_EXT> {
    typedef VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT Type;
};

#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
// Map type VkDirectFBSurfaceCreateInfoEXT to id VK_STRUCTURE_TYPE_DIRECTFB_SURFACE_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkDirectFBSurfaceCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DIRECTFB_SURFACE_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DIRECTFB_SURFACE_CREATE_INFO_EXT> {
    typedef VkDirectFBSurfaceCreateInfoEXT Type;
};

#endif // VK_USE_PLATFORM_DIRECTFB_EXT
// Map type VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT> {
    typedef VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT Type;
};

// Map type VkMutableDescriptorTypeCreateInfoEXT to id VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkMutableDescriptorTypeCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT> {
    typedef VkMutableDescriptorTypeCreateInfoEXT Type;
};

// Map type VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT> {
    typedef VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT Type;
};

// Map type VkVertexInputBindingDescription2EXT to id VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT
template <> struct LvlTypeMap<VkVertexInputBindingDescription2EXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT> {
    typedef VkVertexInputBindingDescription2EXT Type;
};

// Map type VkVertexInputAttributeDescription2EXT to id VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT
template <> struct LvlTypeMap<VkVertexInputAttributeDescription2EXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT> {
    typedef VkVertexInputAttributeDescription2EXT Type;
};

// Map type VkPhysicalDeviceDrmPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRM_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceDrmPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRM_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRM_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceDrmPropertiesEXT Type;
};

// Map type VkPhysicalDeviceAddressBindingReportFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ADDRESS_BINDING_REPORT_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceAddressBindingReportFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ADDRESS_BINDING_REPORT_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ADDRESS_BINDING_REPORT_FEATURES_EXT> {
    typedef VkPhysicalDeviceAddressBindingReportFeaturesEXT Type;
};

// Map type VkDeviceAddressBindingCallbackDataEXT to id VK_STRUCTURE_TYPE_DEVICE_ADDRESS_BINDING_CALLBACK_DATA_EXT
template <> struct LvlTypeMap<VkDeviceAddressBindingCallbackDataEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_ADDRESS_BINDING_CALLBACK_DATA_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_ADDRESS_BINDING_CALLBACK_DATA_EXT> {
    typedef VkDeviceAddressBindingCallbackDataEXT Type;
};

// Map type VkPhysicalDeviceDepthClipControlFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_CONTROL_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceDepthClipControlFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_CONTROL_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_CONTROL_FEATURES_EXT> {
    typedef VkPhysicalDeviceDepthClipControlFeaturesEXT Type;
};

// Map type VkPipelineViewportDepthClipControlCreateInfoEXT to id VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLIP_CONTROL_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkPipelineViewportDepthClipControlCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLIP_CONTROL_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLIP_CONTROL_CREATE_INFO_EXT> {
    typedef VkPipelineViewportDepthClipControlCreateInfoEXT Type;
};

// Map type VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT> {
    typedef VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT Type;
};

// Map type VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_MODE_FIFO_LATEST_READY_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_MODE_FIFO_LATEST_READY_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_MODE_FIFO_LATEST_READY_FEATURES_EXT> {
    typedef VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT Type;
};

#ifdef VK_USE_PLATFORM_FUCHSIA
// Map type VkImportMemoryZirconHandleInfoFUCHSIA to id VK_STRUCTURE_TYPE_IMPORT_MEMORY_ZIRCON_HANDLE_INFO_FUCHSIA
template <> struct LvlTypeMap<VkImportMemoryZirconHandleInfoFUCHSIA> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_ZIRCON_HANDLE_INFO_FUCHSIA;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_MEMORY_ZIRCON_HANDLE_INFO_FUCHSIA> {
    typedef VkImportMemoryZirconHandleInfoFUCHSIA Type;
};

#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
// Map type VkMemoryZirconHandlePropertiesFUCHSIA to id VK_STRUCTURE_TYPE_MEMORY_ZIRCON_HANDLE_PROPERTIES_FUCHSIA
template <> struct LvlTypeMap<VkMemoryZirconHandlePropertiesFUCHSIA> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_ZIRCON_HANDLE_PROPERTIES_FUCHSIA;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_ZIRCON_HANDLE_PROPERTIES_FUCHSIA> {
    typedef VkMemoryZirconHandlePropertiesFUCHSIA Type;
};

#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
// Map type VkMemoryGetZirconHandleInfoFUCHSIA to id VK_STRUCTURE_TYPE_MEMORY_GET_ZIRCON_HANDLE_INFO_FUCHSIA
template <> struct LvlTypeMap<VkMemoryGetZirconHandleInfoFUCHSIA> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_GET_ZIRCON_HANDLE_INFO_FUCHSIA;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_GET_ZIRCON_HANDLE_INFO_FUCHSIA> {
    typedef VkMemoryGetZirconHandleInfoFUCHSIA Type;
};

#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
// Map type VkImportSemaphoreZirconHandleInfoFUCHSIA to id VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_ZIRCON_HANDLE_INFO_FUCHSIA
template <> struct LvlTypeMap<VkImportSemaphoreZirconHandleInfoFUCHSIA> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_ZIRCON_HANDLE_INFO_FUCHSIA;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_ZIRCON_HANDLE_INFO_FUCHSIA> {
    typedef VkImportSemaphoreZirconHandleInfoFUCHSIA Type;
};

#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
// Map type VkSemaphoreGetZirconHandleInfoFUCHSIA to id VK_STRUCTURE_TYPE_SEMAPHORE_GET_ZIRCON_HANDLE_INFO_FUCHSIA
template <> struct LvlTypeMap<VkSemaphoreGetZirconHandleInfoFUCHSIA> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_ZIRCON_HANDLE_INFO_FUCHSIA;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SEMAPHORE_GET_ZIRCON_HANDLE_INFO_FUCHSIA> {
    typedef VkSemaphoreGetZirconHandleInfoFUCHSIA Type;
};

#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
// Map type VkBufferCollectionCreateInfoFUCHSIA to id VK_STRUCTURE_TYPE_BUFFER_COLLECTION_CREATE_INFO_FUCHSIA
template <> struct LvlTypeMap<VkBufferCollectionCreateInfoFUCHSIA> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_COLLECTION_CREATE_INFO_FUCHSIA;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_COLLECTION_CREATE_INFO_FUCHSIA> {
    typedef VkBufferCollectionCreateInfoFUCHSIA Type;
};

#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
// Map type VkImportMemoryBufferCollectionFUCHSIA to id VK_STRUCTURE_TYPE_IMPORT_MEMORY_BUFFER_COLLECTION_FUCHSIA
template <> struct LvlTypeMap<VkImportMemoryBufferCollectionFUCHSIA> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_BUFFER_COLLECTION_FUCHSIA;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_MEMORY_BUFFER_COLLECTION_FUCHSIA> {
    typedef VkImportMemoryBufferCollectionFUCHSIA Type;
};

#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
// Map type VkBufferCollectionImageCreateInfoFUCHSIA to id VK_STRUCTURE_TYPE_BUFFER_COLLECTION_IMAGE_CREATE_INFO_FUCHSIA
template <> struct LvlTypeMap<VkBufferCollectionImageCreateInfoFUCHSIA> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_COLLECTION_IMAGE_CREATE_INFO_FUCHSIA;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_COLLECTION_IMAGE_CREATE_INFO_FUCHSIA> {
    typedef VkBufferCollectionImageCreateInfoFUCHSIA Type;
};

#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
// Map type VkBufferCollectionConstraintsInfoFUCHSIA to id VK_STRUCTURE_TYPE_BUFFER_COLLECTION_CONSTRAINTS_INFO_FUCHSIA
template <> struct LvlTypeMap<VkBufferCollectionConstraintsInfoFUCHSIA> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_COLLECTION_CONSTRAINTS_INFO_FUCHSIA;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_COLLECTION_CONSTRAINTS_INFO_FUCHSIA> {
    typedef VkBufferCollectionConstraintsInfoFUCHSIA Type;
};

#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
// Map type VkBufferConstraintsInfoFUCHSIA to id VK_STRUCTURE_TYPE_BUFFER_CONSTRAINTS_INFO_FUCHSIA
template <> struct LvlTypeMap<VkBufferConstraintsInfoFUCHSIA> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_CONSTRAINTS_INFO_FUCHSIA;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_CONSTRAINTS_INFO_FUCHSIA> {
    typedef VkBufferConstraintsInfoFUCHSIA Type;
};

#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
// Map type VkBufferCollectionBufferCreateInfoFUCHSIA to id VK_STRUCTURE_TYPE_BUFFER_COLLECTION_BUFFER_CREATE_INFO_FUCHSIA
template <> struct LvlTypeMap<VkBufferCollectionBufferCreateInfoFUCHSIA> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_COLLECTION_BUFFER_CREATE_INFO_FUCHSIA;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_COLLECTION_BUFFER_CREATE_INFO_FUCHSIA> {
    typedef VkBufferCollectionBufferCreateInfoFUCHSIA Type;
};

#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
// Map type VkSysmemColorSpaceFUCHSIA to id VK_STRUCTURE_TYPE_SYSMEM_COLOR_SPACE_FUCHSIA
template <> struct LvlTypeMap<VkSysmemColorSpaceFUCHSIA> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SYSMEM_COLOR_SPACE_FUCHSIA;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SYSMEM_COLOR_SPACE_FUCHSIA> {
    typedef VkSysmemColorSpaceFUCHSIA Type;
};

#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
// Map type VkBufferCollectionPropertiesFUCHSIA to id VK_STRUCTURE_TYPE_BUFFER_COLLECTION_PROPERTIES_FUCHSIA
template <> struct LvlTypeMap<VkBufferCollectionPropertiesFUCHSIA> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_COLLECTION_PROPERTIES_FUCHSIA;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_COLLECTION_PROPERTIES_FUCHSIA> {
    typedef VkBufferCollectionPropertiesFUCHSIA Type;
};

#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
// Map type VkImageFormatConstraintsInfoFUCHSIA to id VK_STRUCTURE_TYPE_IMAGE_FORMAT_CONSTRAINTS_INFO_FUCHSIA
template <> struct LvlTypeMap<VkImageFormatConstraintsInfoFUCHSIA> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_CONSTRAINTS_INFO_FUCHSIA;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_FORMAT_CONSTRAINTS_INFO_FUCHSIA> {
    typedef VkImageFormatConstraintsInfoFUCHSIA Type;
};

#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
// Map type VkImageConstraintsInfoFUCHSIA to id VK_STRUCTURE_TYPE_IMAGE_CONSTRAINTS_INFO_FUCHSIA
template <> struct LvlTypeMap<VkImageConstraintsInfoFUCHSIA> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_CONSTRAINTS_INFO_FUCHSIA;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_CONSTRAINTS_INFO_FUCHSIA> {
    typedef VkImageConstraintsInfoFUCHSIA Type;
};

#endif // VK_USE_PLATFORM_FUCHSIA
// Map type VkSubpassShadingPipelineCreateInfoHUAWEI to id VK_STRUCTURE_TYPE_SUBPASS_SHADING_PIPELINE_CREATE_INFO_HUAWEI
template <> struct LvlTypeMap<VkSubpassShadingPipelineCreateInfoHUAWEI> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SUBPASS_SHADING_PIPELINE_CREATE_INFO_HUAWEI;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SUBPASS_SHADING_PIPELINE_CREATE_INFO_HUAWEI> {
    typedef VkSubpassShadingPipelineCreateInfoHUAWEI Type;
};

// Map type VkPhysicalDeviceSubpassShadingFeaturesHUAWEI to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_SHADING_FEATURES_HUAWEI
template <> struct LvlTypeMap<VkPhysicalDeviceSubpassShadingFeaturesHUAWEI> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_SHADING_FEATURES_HUAWEI;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_SHADING_FEATURES_HUAWEI> {
    typedef VkPhysicalDeviceSubpassShadingFeaturesHUAWEI Type;
};

// Map type VkPhysicalDeviceSubpassShadingPropertiesHUAWEI to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_SHADING_PROPERTIES_HUAWEI
template <> struct LvlTypeMap<VkPhysicalDeviceSubpassShadingPropertiesHUAWEI> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_SHADING_PROPERTIES_HUAWEI;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_SHADING_PROPERTIES_HUAWEI> {
    typedef VkPhysicalDeviceSubpassShadingPropertiesHUAWEI Type;
};

// Map type VkPhysicalDeviceInvocationMaskFeaturesHUAWEI to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INVOCATION_MASK_FEATURES_HUAWEI
template <> struct LvlTypeMap<VkPhysicalDeviceInvocationMaskFeaturesHUAWEI> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INVOCATION_MASK_FEATURES_HUAWEI;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INVOCATION_MASK_FEATURES_HUAWEI> {
    typedef VkPhysicalDeviceInvocationMaskFeaturesHUAWEI Type;
};

// Map type VkMemoryGetRemoteAddressInfoNV to id VK_STRUCTURE_TYPE_MEMORY_GET_REMOTE_ADDRESS_INFO_NV
template <> struct LvlTypeMap<VkMemoryGetRemoteAddressInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_GET_REMOTE_ADDRESS_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_GET_REMOTE_ADDRESS_INFO_NV> {
    typedef VkMemoryGetRemoteAddressInfoNV Type;
};

// Map type VkPhysicalDeviceExternalMemoryRDMAFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_RDMA_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceExternalMemoryRDMAFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_RDMA_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_RDMA_FEATURES_NV> {
    typedef VkPhysicalDeviceExternalMemoryRDMAFeaturesNV Type;
};

// Map type VkPipelinePropertiesIdentifierEXT to id VK_STRUCTURE_TYPE_PIPELINE_PROPERTIES_IDENTIFIER_EXT
template <> struct LvlTypeMap<VkPipelinePropertiesIdentifierEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_PROPERTIES_IDENTIFIER_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_PROPERTIES_IDENTIFIER_EXT> {
    typedef VkPipelinePropertiesIdentifierEXT Type;
};

// Map type VkPhysicalDevicePipelinePropertiesFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROPERTIES_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDevicePipelinePropertiesFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROPERTIES_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROPERTIES_FEATURES_EXT> {
    typedef VkPhysicalDevicePipelinePropertiesFeaturesEXT Type;
};

// Map type VkPhysicalDeviceFrameBoundaryFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAME_BOUNDARY_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceFrameBoundaryFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAME_BOUNDARY_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAME_BOUNDARY_FEATURES_EXT> {
    typedef VkPhysicalDeviceFrameBoundaryFeaturesEXT Type;
};

// Map type VkFrameBoundaryEXT to id VK_STRUCTURE_TYPE_FRAME_BOUNDARY_EXT
template <> struct LvlTypeMap<VkFrameBoundaryEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_FRAME_BOUNDARY_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_FRAME_BOUNDARY_EXT> {
    typedef VkFrameBoundaryEXT Type;
};

// Map type VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT> {
    typedef VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT Type;
};

// Map type VkSubpassResolvePerformanceQueryEXT to id VK_STRUCTURE_TYPE_SUBPASS_RESOLVE_PERFORMANCE_QUERY_EXT
template <> struct LvlTypeMap<VkSubpassResolvePerformanceQueryEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SUBPASS_RESOLVE_PERFORMANCE_QUERY_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SUBPASS_RESOLVE_PERFORMANCE_QUERY_EXT> {
    typedef VkSubpassResolvePerformanceQueryEXT Type;
};

// Map type VkMultisampledRenderToSingleSampledInfoEXT to id VK_STRUCTURE_TYPE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_INFO_EXT
template <> struct LvlTypeMap<VkMultisampledRenderToSingleSampledInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_INFO_EXT> {
    typedef VkMultisampledRenderToSingleSampledInfoEXT Type;
};

// Map type VkPhysicalDeviceExtendedDynamicState2FeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT> {
    typedef VkPhysicalDeviceExtendedDynamicState2FeaturesEXT Type;
};

#ifdef VK_USE_PLATFORM_SCREEN_QNX
// Map type VkScreenSurfaceCreateInfoQNX to id VK_STRUCTURE_TYPE_SCREEN_SURFACE_CREATE_INFO_QNX
template <> struct LvlTypeMap<VkScreenSurfaceCreateInfoQNX> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SCREEN_SURFACE_CREATE_INFO_QNX;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SCREEN_SURFACE_CREATE_INFO_QNX> {
    typedef VkScreenSurfaceCreateInfoQNX Type;
};

#endif // VK_USE_PLATFORM_SCREEN_QNX
// Map type VkPhysicalDeviceColorWriteEnableFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceColorWriteEnableFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT> {
    typedef VkPhysicalDeviceColorWriteEnableFeaturesEXT Type;
};

// Map type VkPipelineColorWriteCreateInfoEXT to id VK_STRUCTURE_TYPE_PIPELINE_COLOR_WRITE_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkPipelineColorWriteCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_WRITE_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_COLOR_WRITE_CREATE_INFO_EXT> {
    typedef VkPipelineColorWriteCreateInfoEXT Type;
};

// Map type VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVES_GENERATED_QUERY_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVES_GENERATED_QUERY_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVES_GENERATED_QUERY_FEATURES_EXT> {
    typedef VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT Type;
};

// Map type VkPhysicalDeviceImageViewMinLodFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_MIN_LOD_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceImageViewMinLodFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_MIN_LOD_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_MIN_LOD_FEATURES_EXT> {
    typedef VkPhysicalDeviceImageViewMinLodFeaturesEXT Type;
};

// Map type VkImageViewMinLodCreateInfoEXT to id VK_STRUCTURE_TYPE_IMAGE_VIEW_MIN_LOD_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkImageViewMinLodCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_VIEW_MIN_LOD_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_VIEW_MIN_LOD_CREATE_INFO_EXT> {
    typedef VkImageViewMinLodCreateInfoEXT Type;
};

// Map type VkPhysicalDeviceMultiDrawFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceMultiDrawFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_FEATURES_EXT> {
    typedef VkPhysicalDeviceMultiDrawFeaturesEXT Type;
};

// Map type VkPhysicalDeviceMultiDrawPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceMultiDrawPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceMultiDrawPropertiesEXT Type;
};

// Map type VkPhysicalDeviceImage2DViewOf3DFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_2D_VIEW_OF_3D_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceImage2DViewOf3DFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_2D_VIEW_OF_3D_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_2D_VIEW_OF_3D_FEATURES_EXT> {
    typedef VkPhysicalDeviceImage2DViewOf3DFeaturesEXT Type;
};

// Map type VkPhysicalDeviceShaderTileImageFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceShaderTileImageFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_FEATURES_EXT> {
    typedef VkPhysicalDeviceShaderTileImageFeaturesEXT Type;
};

// Map type VkPhysicalDeviceShaderTileImagePropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceShaderTileImagePropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceShaderTileImagePropertiesEXT Type;
};

// Map type VkMicromapBuildInfoEXT to id VK_STRUCTURE_TYPE_MICROMAP_BUILD_INFO_EXT
template <> struct LvlTypeMap<VkMicromapBuildInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MICROMAP_BUILD_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MICROMAP_BUILD_INFO_EXT> {
    typedef VkMicromapBuildInfoEXT Type;
};

// Map type VkMicromapCreateInfoEXT to id VK_STRUCTURE_TYPE_MICROMAP_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkMicromapCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MICROMAP_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MICROMAP_CREATE_INFO_EXT> {
    typedef VkMicromapCreateInfoEXT Type;
};

// Map type VkPhysicalDeviceOpacityMicromapFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceOpacityMicromapFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_FEATURES_EXT> {
    typedef VkPhysicalDeviceOpacityMicromapFeaturesEXT Type;
};

// Map type VkPhysicalDeviceOpacityMicromapPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceOpacityMicromapPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceOpacityMicromapPropertiesEXT Type;
};

// Map type VkMicromapVersionInfoEXT to id VK_STRUCTURE_TYPE_MICROMAP_VERSION_INFO_EXT
template <> struct LvlTypeMap<VkMicromapVersionInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MICROMAP_VERSION_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MICROMAP_VERSION_INFO_EXT> {
    typedef VkMicromapVersionInfoEXT Type;
};

// Map type VkCopyMicromapToMemoryInfoEXT to id VK_STRUCTURE_TYPE_COPY_MICROMAP_TO_MEMORY_INFO_EXT
template <> struct LvlTypeMap<VkCopyMicromapToMemoryInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COPY_MICROMAP_TO_MEMORY_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COPY_MICROMAP_TO_MEMORY_INFO_EXT> {
    typedef VkCopyMicromapToMemoryInfoEXT Type;
};

// Map type VkCopyMemoryToMicromapInfoEXT to id VK_STRUCTURE_TYPE_COPY_MEMORY_TO_MICROMAP_INFO_EXT
template <> struct LvlTypeMap<VkCopyMemoryToMicromapInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COPY_MEMORY_TO_MICROMAP_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COPY_MEMORY_TO_MICROMAP_INFO_EXT> {
    typedef VkCopyMemoryToMicromapInfoEXT Type;
};

// Map type VkCopyMicromapInfoEXT to id VK_STRUCTURE_TYPE_COPY_MICROMAP_INFO_EXT
template <> struct LvlTypeMap<VkCopyMicromapInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COPY_MICROMAP_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COPY_MICROMAP_INFO_EXT> {
    typedef VkCopyMicromapInfoEXT Type;
};

// Map type VkMicromapBuildSizesInfoEXT to id VK_STRUCTURE_TYPE_MICROMAP_BUILD_SIZES_INFO_EXT
template <> struct LvlTypeMap<VkMicromapBuildSizesInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MICROMAP_BUILD_SIZES_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MICROMAP_BUILD_SIZES_INFO_EXT> {
    typedef VkMicromapBuildSizesInfoEXT Type;
};

// Map type VkAccelerationStructureTrianglesOpacityMicromapEXT to id VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_OPACITY_MICROMAP_EXT
template <> struct LvlTypeMap<VkAccelerationStructureTrianglesOpacityMicromapEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_OPACITY_MICROMAP_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_OPACITY_MICROMAP_EXT> {
    typedef VkAccelerationStructureTrianglesOpacityMicromapEXT Type;
};

#ifdef VK_ENABLE_BETA_EXTENSIONS
// Map type VkPhysicalDeviceDisplacementMicromapFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISPLACEMENT_MICROMAP_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceDisplacementMicromapFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISPLACEMENT_MICROMAP_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISPLACEMENT_MICROMAP_FEATURES_NV> {
    typedef VkPhysicalDeviceDisplacementMicromapFeaturesNV Type;
};

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
// Map type VkPhysicalDeviceDisplacementMicromapPropertiesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISPLACEMENT_MICROMAP_PROPERTIES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceDisplacementMicromapPropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISPLACEMENT_MICROMAP_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISPLACEMENT_MICROMAP_PROPERTIES_NV> {
    typedef VkPhysicalDeviceDisplacementMicromapPropertiesNV Type;
};

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
// Map type VkAccelerationStructureTrianglesDisplacementMicromapNV to id VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_DISPLACEMENT_MICROMAP_NV
template <> struct LvlTypeMap<VkAccelerationStructureTrianglesDisplacementMicromapNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_DISPLACEMENT_MICROMAP_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_DISPLACEMENT_MICROMAP_NV> {
    typedef VkAccelerationStructureTrianglesDisplacementMicromapNV Type;
};

#endif // VK_ENABLE_BETA_EXTENSIONS
// Map type VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_FEATURES_HUAWEI
template <> struct LvlTypeMap<VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_FEATURES_HUAWEI;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_FEATURES_HUAWEI> {
    typedef VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI Type;
};

// Map type VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_PROPERTIES_HUAWEI
template <> struct LvlTypeMap<VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_PROPERTIES_HUAWEI;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_PROPERTIES_HUAWEI> {
    typedef VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI Type;
};

// Map type VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_VRS_FEATURES_HUAWEI
template <> struct LvlTypeMap<VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_VRS_FEATURES_HUAWEI;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_VRS_FEATURES_HUAWEI> {
    typedef VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI Type;
};

// Map type VkPhysicalDeviceBorderColorSwizzleFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BORDER_COLOR_SWIZZLE_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceBorderColorSwizzleFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BORDER_COLOR_SWIZZLE_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BORDER_COLOR_SWIZZLE_FEATURES_EXT> {
    typedef VkPhysicalDeviceBorderColorSwizzleFeaturesEXT Type;
};

// Map type VkSamplerBorderColorComponentMappingCreateInfoEXT to id VK_STRUCTURE_TYPE_SAMPLER_BORDER_COLOR_COMPONENT_MAPPING_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkSamplerBorderColorComponentMappingCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SAMPLER_BORDER_COLOR_COMPONENT_MAPPING_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SAMPLER_BORDER_COLOR_COMPONENT_MAPPING_CREATE_INFO_EXT> {
    typedef VkSamplerBorderColorComponentMappingCreateInfoEXT Type;
};

// Map type VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PAGEABLE_DEVICE_LOCAL_MEMORY_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PAGEABLE_DEVICE_LOCAL_MEMORY_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PAGEABLE_DEVICE_LOCAL_MEMORY_FEATURES_EXT> {
    typedef VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT Type;
};

// Map type VkPhysicalDeviceShaderCorePropertiesARM to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_ARM
template <> struct LvlTypeMap<VkPhysicalDeviceShaderCorePropertiesARM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_ARM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_ARM> {
    typedef VkPhysicalDeviceShaderCorePropertiesARM Type;
};

// Map type VkDeviceQueueShaderCoreControlCreateInfoARM to id VK_STRUCTURE_TYPE_DEVICE_QUEUE_SHADER_CORE_CONTROL_CREATE_INFO_ARM
template <> struct LvlTypeMap<VkDeviceQueueShaderCoreControlCreateInfoARM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_SHADER_CORE_CONTROL_CREATE_INFO_ARM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_QUEUE_SHADER_CORE_CONTROL_CREATE_INFO_ARM> {
    typedef VkDeviceQueueShaderCoreControlCreateInfoARM Type;
};

// Map type VkPhysicalDeviceSchedulingControlsFeaturesARM to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCHEDULING_CONTROLS_FEATURES_ARM
template <> struct LvlTypeMap<VkPhysicalDeviceSchedulingControlsFeaturesARM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCHEDULING_CONTROLS_FEATURES_ARM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCHEDULING_CONTROLS_FEATURES_ARM> {
    typedef VkPhysicalDeviceSchedulingControlsFeaturesARM Type;
};

// Map type VkPhysicalDeviceSchedulingControlsPropertiesARM to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCHEDULING_CONTROLS_PROPERTIES_ARM
template <> struct LvlTypeMap<VkPhysicalDeviceSchedulingControlsPropertiesARM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCHEDULING_CONTROLS_PROPERTIES_ARM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCHEDULING_CONTROLS_PROPERTIES_ARM> {
    typedef VkPhysicalDeviceSchedulingControlsPropertiesARM Type;
};

// Map type VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_SLICED_VIEW_OF_3D_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_SLICED_VIEW_OF_3D_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_SLICED_VIEW_OF_3D_FEATURES_EXT> {
    typedef VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT Type;
};

// Map type VkImageViewSlicedCreateInfoEXT to id VK_STRUCTURE_TYPE_IMAGE_VIEW_SLICED_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkImageViewSlicedCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_VIEW_SLICED_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_VIEW_SLICED_CREATE_INFO_EXT> {
    typedef VkImageViewSlicedCreateInfoEXT Type;
};

// Map type VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_SET_HOST_MAPPING_FEATURES_VALVE
template <> struct LvlTypeMap<VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_SET_HOST_MAPPING_FEATURES_VALVE;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_SET_HOST_MAPPING_FEATURES_VALVE> {
    typedef VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE Type;
};

// Map type VkDescriptorSetBindingReferenceVALVE to id VK_STRUCTURE_TYPE_DESCRIPTOR_SET_BINDING_REFERENCE_VALVE
template <> struct LvlTypeMap<VkDescriptorSetBindingReferenceVALVE> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_BINDING_REFERENCE_VALVE;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DESCRIPTOR_SET_BINDING_REFERENCE_VALVE> {
    typedef VkDescriptorSetBindingReferenceVALVE Type;
};

// Map type VkDescriptorSetLayoutHostMappingInfoVALVE to id VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_HOST_MAPPING_INFO_VALVE
template <> struct LvlTypeMap<VkDescriptorSetLayoutHostMappingInfoVALVE> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_HOST_MAPPING_INFO_VALVE;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_HOST_MAPPING_INFO_VALVE> {
    typedef VkDescriptorSetLayoutHostMappingInfoVALVE Type;
};

// Map type VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NON_SEAMLESS_CUBE_MAP_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NON_SEAMLESS_CUBE_MAP_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NON_SEAMLESS_CUBE_MAP_FEATURES_EXT> {
    typedef VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT Type;
};

// Map type VkPhysicalDeviceRenderPassStripedFeaturesARM to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RENDER_PASS_STRIPED_FEATURES_ARM
template <> struct LvlTypeMap<VkPhysicalDeviceRenderPassStripedFeaturesARM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RENDER_PASS_STRIPED_FEATURES_ARM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RENDER_PASS_STRIPED_FEATURES_ARM> {
    typedef VkPhysicalDeviceRenderPassStripedFeaturesARM Type;
};

// Map type VkPhysicalDeviceRenderPassStripedPropertiesARM to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RENDER_PASS_STRIPED_PROPERTIES_ARM
template <> struct LvlTypeMap<VkPhysicalDeviceRenderPassStripedPropertiesARM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RENDER_PASS_STRIPED_PROPERTIES_ARM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RENDER_PASS_STRIPED_PROPERTIES_ARM> {
    typedef VkPhysicalDeviceRenderPassStripedPropertiesARM Type;
};

// Map type VkRenderPassStripeInfoARM to id VK_STRUCTURE_TYPE_RENDER_PASS_STRIPE_INFO_ARM
template <> struct LvlTypeMap<VkRenderPassStripeInfoARM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDER_PASS_STRIPE_INFO_ARM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDER_PASS_STRIPE_INFO_ARM> {
    typedef VkRenderPassStripeInfoARM Type;
};

// Map type VkRenderPassStripeBeginInfoARM to id VK_STRUCTURE_TYPE_RENDER_PASS_STRIPE_BEGIN_INFO_ARM
template <> struct LvlTypeMap<VkRenderPassStripeBeginInfoARM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDER_PASS_STRIPE_BEGIN_INFO_ARM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDER_PASS_STRIPE_BEGIN_INFO_ARM> {
    typedef VkRenderPassStripeBeginInfoARM Type;
};

// Map type VkRenderPassStripeSubmitInfoARM to id VK_STRUCTURE_TYPE_RENDER_PASS_STRIPE_SUBMIT_INFO_ARM
template <> struct LvlTypeMap<VkRenderPassStripeSubmitInfoARM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDER_PASS_STRIPE_SUBMIT_INFO_ARM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDER_PASS_STRIPE_SUBMIT_INFO_ARM> {
    typedef VkRenderPassStripeSubmitInfoARM Type;
};

// Map type VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_FEATURES_QCOM
template <> struct LvlTypeMap<VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_FEATURES_QCOM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_FEATURES_QCOM> {
    typedef VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM Type;
};

// Map type VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_PROPERTIES_QCOM
template <> struct LvlTypeMap<VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_PROPERTIES_QCOM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_PROPERTIES_QCOM> {
    typedef VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM Type;
};

// Map type VkSubpassFragmentDensityMapOffsetEndInfoQCOM to id VK_STRUCTURE_TYPE_SUBPASS_FRAGMENT_DENSITY_MAP_OFFSET_END_INFO_QCOM
template <> struct LvlTypeMap<VkSubpassFragmentDensityMapOffsetEndInfoQCOM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SUBPASS_FRAGMENT_DENSITY_MAP_OFFSET_END_INFO_QCOM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SUBPASS_FRAGMENT_DENSITY_MAP_OFFSET_END_INFO_QCOM> {
    typedef VkSubpassFragmentDensityMapOffsetEndInfoQCOM Type;
};

// Map type VkPhysicalDeviceCopyMemoryIndirectFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceCopyMemoryIndirectFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_FEATURES_NV> {
    typedef VkPhysicalDeviceCopyMemoryIndirectFeaturesNV Type;
};

// Map type VkPhysicalDeviceCopyMemoryIndirectPropertiesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_PROPERTIES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceCopyMemoryIndirectPropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_PROPERTIES_NV> {
    typedef VkPhysicalDeviceCopyMemoryIndirectPropertiesNV Type;
};

// Map type VkPhysicalDeviceMemoryDecompressionFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceMemoryDecompressionFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_FEATURES_NV> {
    typedef VkPhysicalDeviceMemoryDecompressionFeaturesNV Type;
};

// Map type VkPhysicalDeviceMemoryDecompressionPropertiesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_PROPERTIES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceMemoryDecompressionPropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_PROPERTIES_NV> {
    typedef VkPhysicalDeviceMemoryDecompressionPropertiesNV Type;
};

// Map type VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_COMPUTE_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_COMPUTE_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_COMPUTE_FEATURES_NV> {
    typedef VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV Type;
};

// Map type VkComputePipelineIndirectBufferInfoNV to id VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_INDIRECT_BUFFER_INFO_NV
template <> struct LvlTypeMap<VkComputePipelineIndirectBufferInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_INDIRECT_BUFFER_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_INDIRECT_BUFFER_INFO_NV> {
    typedef VkComputePipelineIndirectBufferInfoNV Type;
};

// Map type VkPipelineIndirectDeviceAddressInfoNV to id VK_STRUCTURE_TYPE_PIPELINE_INDIRECT_DEVICE_ADDRESS_INFO_NV
template <> struct LvlTypeMap<VkPipelineIndirectDeviceAddressInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_INDIRECT_DEVICE_ADDRESS_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_INDIRECT_DEVICE_ADDRESS_INFO_NV> {
    typedef VkPipelineIndirectDeviceAddressInfoNV Type;
};

// Map type VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_LINEAR_SWEPT_SPHERES_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_LINEAR_SWEPT_SPHERES_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_LINEAR_SWEPT_SPHERES_FEATURES_NV> {
    typedef VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV Type;
};

// Map type VkAccelerationStructureGeometryLinearSweptSpheresDataNV to id VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_LINEAR_SWEPT_SPHERES_DATA_NV
template <> struct LvlTypeMap<VkAccelerationStructureGeometryLinearSweptSpheresDataNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_LINEAR_SWEPT_SPHERES_DATA_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_LINEAR_SWEPT_SPHERES_DATA_NV> {
    typedef VkAccelerationStructureGeometryLinearSweptSpheresDataNV Type;
};

// Map type VkAccelerationStructureGeometrySpheresDataNV to id VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_SPHERES_DATA_NV
template <> struct LvlTypeMap<VkAccelerationStructureGeometrySpheresDataNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_SPHERES_DATA_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_SPHERES_DATA_NV> {
    typedef VkAccelerationStructureGeometrySpheresDataNV Type;
};

// Map type VkPhysicalDeviceLinearColorAttachmentFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINEAR_COLOR_ATTACHMENT_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceLinearColorAttachmentFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINEAR_COLOR_ATTACHMENT_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINEAR_COLOR_ATTACHMENT_FEATURES_NV> {
    typedef VkPhysicalDeviceLinearColorAttachmentFeaturesNV Type;
};

// Map type VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_FEATURES_EXT> {
    typedef VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT Type;
};

// Map type VkImageViewSampleWeightCreateInfoQCOM to id VK_STRUCTURE_TYPE_IMAGE_VIEW_SAMPLE_WEIGHT_CREATE_INFO_QCOM
template <> struct LvlTypeMap<VkImageViewSampleWeightCreateInfoQCOM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_VIEW_SAMPLE_WEIGHT_CREATE_INFO_QCOM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_VIEW_SAMPLE_WEIGHT_CREATE_INFO_QCOM> {
    typedef VkImageViewSampleWeightCreateInfoQCOM Type;
};

// Map type VkPhysicalDeviceImageProcessingFeaturesQCOM to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_FEATURES_QCOM
template <> struct LvlTypeMap<VkPhysicalDeviceImageProcessingFeaturesQCOM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_FEATURES_QCOM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_FEATURES_QCOM> {
    typedef VkPhysicalDeviceImageProcessingFeaturesQCOM Type;
};

// Map type VkPhysicalDeviceImageProcessingPropertiesQCOM to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_PROPERTIES_QCOM
template <> struct LvlTypeMap<VkPhysicalDeviceImageProcessingPropertiesQCOM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_PROPERTIES_QCOM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_PROPERTIES_QCOM> {
    typedef VkPhysicalDeviceImageProcessingPropertiesQCOM Type;
};

// Map type VkPhysicalDeviceNestedCommandBufferFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceNestedCommandBufferFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT> {
    typedef VkPhysicalDeviceNestedCommandBufferFeaturesEXT Type;
};

// Map type VkPhysicalDeviceNestedCommandBufferPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceNestedCommandBufferPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceNestedCommandBufferPropertiesEXT Type;
};

// Map type VkExternalMemoryAcquireUnmodifiedEXT to id VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_ACQUIRE_UNMODIFIED_EXT
template <> struct LvlTypeMap<VkExternalMemoryAcquireUnmodifiedEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_ACQUIRE_UNMODIFIED_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_ACQUIRE_UNMODIFIED_EXT> {
    typedef VkExternalMemoryAcquireUnmodifiedEXT Type;
};

// Map type VkPhysicalDeviceExtendedDynamicState3FeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT> {
    typedef VkPhysicalDeviceExtendedDynamicState3FeaturesEXT Type;
};

// Map type VkPhysicalDeviceExtendedDynamicState3PropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceExtendedDynamicState3PropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceExtendedDynamicState3PropertiesEXT Type;
};

// Map type VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_MERGE_FEEDBACK_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_MERGE_FEEDBACK_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_MERGE_FEEDBACK_FEATURES_EXT> {
    typedef VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT Type;
};

// Map type VkRenderPassCreationControlEXT to id VK_STRUCTURE_TYPE_RENDER_PASS_CREATION_CONTROL_EXT
template <> struct LvlTypeMap<VkRenderPassCreationControlEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATION_CONTROL_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDER_PASS_CREATION_CONTROL_EXT> {
    typedef VkRenderPassCreationControlEXT Type;
};

// Map type VkRenderPassCreationFeedbackCreateInfoEXT to id VK_STRUCTURE_TYPE_RENDER_PASS_CREATION_FEEDBACK_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkRenderPassCreationFeedbackCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATION_FEEDBACK_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDER_PASS_CREATION_FEEDBACK_CREATE_INFO_EXT> {
    typedef VkRenderPassCreationFeedbackCreateInfoEXT Type;
};

// Map type VkRenderPassSubpassFeedbackCreateInfoEXT to id VK_STRUCTURE_TYPE_RENDER_PASS_SUBPASS_FEEDBACK_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkRenderPassSubpassFeedbackCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDER_PASS_SUBPASS_FEEDBACK_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDER_PASS_SUBPASS_FEEDBACK_CREATE_INFO_EXT> {
    typedef VkRenderPassSubpassFeedbackCreateInfoEXT Type;
};

// Map type VkDirectDriverLoadingInfoLUNARG to id VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_INFO_LUNARG
template <> struct LvlTypeMap<VkDirectDriverLoadingInfoLUNARG> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_INFO_LUNARG;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_INFO_LUNARG> {
    typedef VkDirectDriverLoadingInfoLUNARG Type;
};

// Map type VkDirectDriverLoadingListLUNARG to id VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_LIST_LUNARG
template <> struct LvlTypeMap<VkDirectDriverLoadingListLUNARG> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_LIST_LUNARG;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_LIST_LUNARG> {
    typedef VkDirectDriverLoadingListLUNARG Type;
};

// Map type VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_FEATURES_EXT> {
    typedef VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT Type;
};

// Map type VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT Type;
};

// Map type VkPipelineShaderStageModuleIdentifierCreateInfoEXT to id VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_MODULE_IDENTIFIER_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkPipelineShaderStageModuleIdentifierCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_MODULE_IDENTIFIER_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_MODULE_IDENTIFIER_CREATE_INFO_EXT> {
    typedef VkPipelineShaderStageModuleIdentifierCreateInfoEXT Type;
};

// Map type VkShaderModuleIdentifierEXT to id VK_STRUCTURE_TYPE_SHADER_MODULE_IDENTIFIER_EXT
template <> struct LvlTypeMap<VkShaderModuleIdentifierEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SHADER_MODULE_IDENTIFIER_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SHADER_MODULE_IDENTIFIER_EXT> {
    typedef VkShaderModuleIdentifierEXT Type;
};

// Map type VkPhysicalDeviceOpticalFlowFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceOpticalFlowFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_FEATURES_NV> {
    typedef VkPhysicalDeviceOpticalFlowFeaturesNV Type;
};

// Map type VkPhysicalDeviceOpticalFlowPropertiesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_PROPERTIES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceOpticalFlowPropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_PROPERTIES_NV> {
    typedef VkPhysicalDeviceOpticalFlowPropertiesNV Type;
};

// Map type VkOpticalFlowImageFormatInfoNV to id VK_STRUCTURE_TYPE_OPTICAL_FLOW_IMAGE_FORMAT_INFO_NV
template <> struct LvlTypeMap<VkOpticalFlowImageFormatInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_OPTICAL_FLOW_IMAGE_FORMAT_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_OPTICAL_FLOW_IMAGE_FORMAT_INFO_NV> {
    typedef VkOpticalFlowImageFormatInfoNV Type;
};

// Map type VkOpticalFlowImageFormatPropertiesNV to id VK_STRUCTURE_TYPE_OPTICAL_FLOW_IMAGE_FORMAT_PROPERTIES_NV
template <> struct LvlTypeMap<VkOpticalFlowImageFormatPropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_OPTICAL_FLOW_IMAGE_FORMAT_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_OPTICAL_FLOW_IMAGE_FORMAT_PROPERTIES_NV> {
    typedef VkOpticalFlowImageFormatPropertiesNV Type;
};

// Map type VkOpticalFlowSessionCreateInfoNV to id VK_STRUCTURE_TYPE_OPTICAL_FLOW_SESSION_CREATE_INFO_NV
template <> struct LvlTypeMap<VkOpticalFlowSessionCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_OPTICAL_FLOW_SESSION_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_OPTICAL_FLOW_SESSION_CREATE_INFO_NV> {
    typedef VkOpticalFlowSessionCreateInfoNV Type;
};

// Map type VkOpticalFlowSessionCreatePrivateDataInfoNV to id VK_STRUCTURE_TYPE_OPTICAL_FLOW_SESSION_CREATE_PRIVATE_DATA_INFO_NV
template <> struct LvlTypeMap<VkOpticalFlowSessionCreatePrivateDataInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_OPTICAL_FLOW_SESSION_CREATE_PRIVATE_DATA_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_OPTICAL_FLOW_SESSION_CREATE_PRIVATE_DATA_INFO_NV> {
    typedef VkOpticalFlowSessionCreatePrivateDataInfoNV Type;
};

// Map type VkOpticalFlowExecuteInfoNV to id VK_STRUCTURE_TYPE_OPTICAL_FLOW_EXECUTE_INFO_NV
template <> struct LvlTypeMap<VkOpticalFlowExecuteInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_OPTICAL_FLOW_EXECUTE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_OPTICAL_FLOW_EXECUTE_INFO_NV> {
    typedef VkOpticalFlowExecuteInfoNV Type;
};

// Map type VkPhysicalDeviceLegacyDitheringFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_DITHERING_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceLegacyDitheringFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_DITHERING_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_DITHERING_FEATURES_EXT> {
    typedef VkPhysicalDeviceLegacyDitheringFeaturesEXT Type;
};

#ifdef VK_USE_PLATFORM_ANDROID_KHR
// Map type VkPhysicalDeviceExternalFormatResolveFeaturesANDROID to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_FEATURES_ANDROID
template <> struct LvlTypeMap<VkPhysicalDeviceExternalFormatResolveFeaturesANDROID> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_FEATURES_ANDROID;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_FEATURES_ANDROID> {
    typedef VkPhysicalDeviceExternalFormatResolveFeaturesANDROID Type;
};

#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
// Map type VkPhysicalDeviceExternalFormatResolvePropertiesANDROID to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_PROPERTIES_ANDROID
template <> struct LvlTypeMap<VkPhysicalDeviceExternalFormatResolvePropertiesANDROID> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_PROPERTIES_ANDROID;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_PROPERTIES_ANDROID> {
    typedef VkPhysicalDeviceExternalFormatResolvePropertiesANDROID Type;
};

#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
// Map type VkAndroidHardwareBufferFormatResolvePropertiesANDROID to id VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_RESOLVE_PROPERTIES_ANDROID
template <> struct LvlTypeMap<VkAndroidHardwareBufferFormatResolvePropertiesANDROID> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_RESOLVE_PROPERTIES_ANDROID;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_RESOLVE_PROPERTIES_ANDROID> {
    typedef VkAndroidHardwareBufferFormatResolvePropertiesANDROID Type;
};

#endif // VK_USE_PLATFORM_ANDROID_KHR
// Map type VkPhysicalDeviceAntiLagFeaturesAMD to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ANTI_LAG_FEATURES_AMD
template <> struct LvlTypeMap<VkPhysicalDeviceAntiLagFeaturesAMD> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ANTI_LAG_FEATURES_AMD;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ANTI_LAG_FEATURES_AMD> {
    typedef VkPhysicalDeviceAntiLagFeaturesAMD Type;
};

// Map type VkAntiLagPresentationInfoAMD to id VK_STRUCTURE_TYPE_ANTI_LAG_PRESENTATION_INFO_AMD
template <> struct LvlTypeMap<VkAntiLagPresentationInfoAMD> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ANTI_LAG_PRESENTATION_INFO_AMD;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ANTI_LAG_PRESENTATION_INFO_AMD> {
    typedef VkAntiLagPresentationInfoAMD Type;
};

// Map type VkAntiLagDataAMD to id VK_STRUCTURE_TYPE_ANTI_LAG_DATA_AMD
template <> struct LvlTypeMap<VkAntiLagDataAMD> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ANTI_LAG_DATA_AMD;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ANTI_LAG_DATA_AMD> {
    typedef VkAntiLagDataAMD Type;
};

// Map type VkPhysicalDeviceShaderObjectFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceShaderObjectFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT> {
    typedef VkPhysicalDeviceShaderObjectFeaturesEXT Type;
};

// Map type VkPhysicalDeviceShaderObjectPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceShaderObjectPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceShaderObjectPropertiesEXT Type;
};

// Map type VkShaderCreateInfoEXT to id VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkShaderCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT> {
    typedef VkShaderCreateInfoEXT Type;
};

// Map type VkPhysicalDeviceTilePropertiesFeaturesQCOM to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_PROPERTIES_FEATURES_QCOM
template <> struct LvlTypeMap<VkPhysicalDeviceTilePropertiesFeaturesQCOM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_PROPERTIES_FEATURES_QCOM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_PROPERTIES_FEATURES_QCOM> {
    typedef VkPhysicalDeviceTilePropertiesFeaturesQCOM Type;
};

// Map type VkTilePropertiesQCOM to id VK_STRUCTURE_TYPE_TILE_PROPERTIES_QCOM
template <> struct LvlTypeMap<VkTilePropertiesQCOM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_TILE_PROPERTIES_QCOM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_TILE_PROPERTIES_QCOM> {
    typedef VkTilePropertiesQCOM Type;
};

// Map type VkPhysicalDeviceAmigoProfilingFeaturesSEC to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_AMIGO_PROFILING_FEATURES_SEC
template <> struct LvlTypeMap<VkPhysicalDeviceAmigoProfilingFeaturesSEC> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_AMIGO_PROFILING_FEATURES_SEC;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_AMIGO_PROFILING_FEATURES_SEC> {
    typedef VkPhysicalDeviceAmigoProfilingFeaturesSEC Type;
};

// Map type VkAmigoProfilingSubmitInfoSEC to id VK_STRUCTURE_TYPE_AMIGO_PROFILING_SUBMIT_INFO_SEC
template <> struct LvlTypeMap<VkAmigoProfilingSubmitInfoSEC> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_AMIGO_PROFILING_SUBMIT_INFO_SEC;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_AMIGO_PROFILING_SUBMIT_INFO_SEC> {
    typedef VkAmigoProfilingSubmitInfoSEC Type;
};

// Map type VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_VIEWPORTS_FEATURES_QCOM
template <> struct LvlTypeMap<VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_VIEWPORTS_FEATURES_QCOM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_VIEWPORTS_FEATURES_QCOM> {
    typedef VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM Type;
};

// Map type VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_PROPERTIES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_PROPERTIES_NV> {
    typedef VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV Type;
};

// Map type VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_FEATURES_NV> {
    typedef VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV Type;
};

// Map type VkPhysicalDeviceCooperativeVectorPropertiesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_VECTOR_PROPERTIES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceCooperativeVectorPropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_VECTOR_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_VECTOR_PROPERTIES_NV> {
    typedef VkPhysicalDeviceCooperativeVectorPropertiesNV Type;
};

// Map type VkPhysicalDeviceCooperativeVectorFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_VECTOR_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceCooperativeVectorFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_VECTOR_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_VECTOR_FEATURES_NV> {
    typedef VkPhysicalDeviceCooperativeVectorFeaturesNV Type;
};

// Map type VkCooperativeVectorPropertiesNV to id VK_STRUCTURE_TYPE_COOPERATIVE_VECTOR_PROPERTIES_NV
template <> struct LvlTypeMap<VkCooperativeVectorPropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COOPERATIVE_VECTOR_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COOPERATIVE_VECTOR_PROPERTIES_NV> {
    typedef VkCooperativeVectorPropertiesNV Type;
};

// Map type VkConvertCooperativeVectorMatrixInfoNV to id VK_STRUCTURE_TYPE_CONVERT_COOPERATIVE_VECTOR_MATRIX_INFO_NV
template <> struct LvlTypeMap<VkConvertCooperativeVectorMatrixInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_CONVERT_COOPERATIVE_VECTOR_MATRIX_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_CONVERT_COOPERATIVE_VECTOR_MATRIX_INFO_NV> {
    typedef VkConvertCooperativeVectorMatrixInfoNV Type;
};

// Map type VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_SPARSE_ADDRESS_SPACE_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_SPARSE_ADDRESS_SPACE_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_SPARSE_ADDRESS_SPACE_FEATURES_NV> {
    typedef VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV Type;
};

// Map type VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_SPARSE_ADDRESS_SPACE_PROPERTIES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_SPARSE_ADDRESS_SPACE_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_SPARSE_ADDRESS_SPACE_PROPERTIES_NV> {
    typedef VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV Type;
};

// Map type VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_VERTEX_ATTRIBUTES_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_VERTEX_ATTRIBUTES_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_VERTEX_ATTRIBUTES_FEATURES_EXT> {
    typedef VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT Type;
};

// Map type VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_VERTEX_ATTRIBUTES_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_VERTEX_ATTRIBUTES_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_VERTEX_ATTRIBUTES_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT Type;
};

// Map type VkLayerSettingsCreateInfoEXT to id VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkLayerSettingsCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT> {
    typedef VkLayerSettingsCreateInfoEXT Type;
};

// Map type VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_BUILTINS_FEATURES_ARM
template <> struct LvlTypeMap<VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_BUILTINS_FEATURES_ARM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_BUILTINS_FEATURES_ARM> {
    typedef VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM Type;
};

// Map type VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_BUILTINS_PROPERTIES_ARM
template <> struct LvlTypeMap<VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_BUILTINS_PROPERTIES_ARM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_BUILTINS_PROPERTIES_ARM> {
    typedef VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM Type;
};

// Map type VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_LIBRARY_GROUP_HANDLES_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_LIBRARY_GROUP_HANDLES_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_LIBRARY_GROUP_HANDLES_FEATURES_EXT> {
    typedef VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT Type;
};

// Map type VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT> {
    typedef VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT Type;
};

// Map type VkLatencySleepModeInfoNV to id VK_STRUCTURE_TYPE_LATENCY_SLEEP_MODE_INFO_NV
template <> struct LvlTypeMap<VkLatencySleepModeInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_LATENCY_SLEEP_MODE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_LATENCY_SLEEP_MODE_INFO_NV> {
    typedef VkLatencySleepModeInfoNV Type;
};

// Map type VkLatencySleepInfoNV to id VK_STRUCTURE_TYPE_LATENCY_SLEEP_INFO_NV
template <> struct LvlTypeMap<VkLatencySleepInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_LATENCY_SLEEP_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_LATENCY_SLEEP_INFO_NV> {
    typedef VkLatencySleepInfoNV Type;
};

// Map type VkSetLatencyMarkerInfoNV to id VK_STRUCTURE_TYPE_SET_LATENCY_MARKER_INFO_NV
template <> struct LvlTypeMap<VkSetLatencyMarkerInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SET_LATENCY_MARKER_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SET_LATENCY_MARKER_INFO_NV> {
    typedef VkSetLatencyMarkerInfoNV Type;
};

// Map type VkLatencyTimingsFrameReportNV to id VK_STRUCTURE_TYPE_LATENCY_TIMINGS_FRAME_REPORT_NV
template <> struct LvlTypeMap<VkLatencyTimingsFrameReportNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_LATENCY_TIMINGS_FRAME_REPORT_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_LATENCY_TIMINGS_FRAME_REPORT_NV> {
    typedef VkLatencyTimingsFrameReportNV Type;
};

// Map type VkGetLatencyMarkerInfoNV to id VK_STRUCTURE_TYPE_GET_LATENCY_MARKER_INFO_NV
template <> struct LvlTypeMap<VkGetLatencyMarkerInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_GET_LATENCY_MARKER_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_GET_LATENCY_MARKER_INFO_NV> {
    typedef VkGetLatencyMarkerInfoNV Type;
};

// Map type VkLatencySubmissionPresentIdNV to id VK_STRUCTURE_TYPE_LATENCY_SUBMISSION_PRESENT_ID_NV
template <> struct LvlTypeMap<VkLatencySubmissionPresentIdNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_LATENCY_SUBMISSION_PRESENT_ID_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_LATENCY_SUBMISSION_PRESENT_ID_NV> {
    typedef VkLatencySubmissionPresentIdNV Type;
};

// Map type VkSwapchainLatencyCreateInfoNV to id VK_STRUCTURE_TYPE_SWAPCHAIN_LATENCY_CREATE_INFO_NV
template <> struct LvlTypeMap<VkSwapchainLatencyCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SWAPCHAIN_LATENCY_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SWAPCHAIN_LATENCY_CREATE_INFO_NV> {
    typedef VkSwapchainLatencyCreateInfoNV Type;
};

// Map type VkOutOfBandQueueTypeInfoNV to id VK_STRUCTURE_TYPE_OUT_OF_BAND_QUEUE_TYPE_INFO_NV
template <> struct LvlTypeMap<VkOutOfBandQueueTypeInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_OUT_OF_BAND_QUEUE_TYPE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_OUT_OF_BAND_QUEUE_TYPE_INFO_NV> {
    typedef VkOutOfBandQueueTypeInfoNV Type;
};

// Map type VkLatencySurfaceCapabilitiesNV to id VK_STRUCTURE_TYPE_LATENCY_SURFACE_CAPABILITIES_NV
template <> struct LvlTypeMap<VkLatencySurfaceCapabilitiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_LATENCY_SURFACE_CAPABILITIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_LATENCY_SURFACE_CAPABILITIES_NV> {
    typedef VkLatencySurfaceCapabilitiesNV Type;
};

// Map type VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_RENDER_AREAS_FEATURES_QCOM
template <> struct LvlTypeMap<VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_RENDER_AREAS_FEATURES_QCOM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_RENDER_AREAS_FEATURES_QCOM> {
    typedef VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM Type;
};

// Map type VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM to id VK_STRUCTURE_TYPE_MULTIVIEW_PER_VIEW_RENDER_AREAS_RENDER_PASS_BEGIN_INFO_QCOM
template <> struct LvlTypeMap<VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MULTIVIEW_PER_VIEW_RENDER_AREAS_RENDER_PASS_BEGIN_INFO_QCOM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MULTIVIEW_PER_VIEW_RENDER_AREAS_RENDER_PASS_BEGIN_INFO_QCOM> {
    typedef VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM Type;
};

// Map type VkPhysicalDevicePerStageDescriptorSetFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PER_STAGE_DESCRIPTOR_SET_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDevicePerStageDescriptorSetFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PER_STAGE_DESCRIPTOR_SET_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PER_STAGE_DESCRIPTOR_SET_FEATURES_NV> {
    typedef VkPhysicalDevicePerStageDescriptorSetFeaturesNV Type;
};

// Map type VkPhysicalDeviceImageProcessing2FeaturesQCOM to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_2_FEATURES_QCOM
template <> struct LvlTypeMap<VkPhysicalDeviceImageProcessing2FeaturesQCOM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_2_FEATURES_QCOM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_2_FEATURES_QCOM> {
    typedef VkPhysicalDeviceImageProcessing2FeaturesQCOM Type;
};

// Map type VkPhysicalDeviceImageProcessing2PropertiesQCOM to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_2_PROPERTIES_QCOM
template <> struct LvlTypeMap<VkPhysicalDeviceImageProcessing2PropertiesQCOM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_2_PROPERTIES_QCOM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_2_PROPERTIES_QCOM> {
    typedef VkPhysicalDeviceImageProcessing2PropertiesQCOM Type;
};

// Map type VkSamplerBlockMatchWindowCreateInfoQCOM to id VK_STRUCTURE_TYPE_SAMPLER_BLOCK_MATCH_WINDOW_CREATE_INFO_QCOM
template <> struct LvlTypeMap<VkSamplerBlockMatchWindowCreateInfoQCOM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SAMPLER_BLOCK_MATCH_WINDOW_CREATE_INFO_QCOM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SAMPLER_BLOCK_MATCH_WINDOW_CREATE_INFO_QCOM> {
    typedef VkSamplerBlockMatchWindowCreateInfoQCOM Type;
};

// Map type VkPhysicalDeviceCubicWeightsFeaturesQCOM to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_WEIGHTS_FEATURES_QCOM
template <> struct LvlTypeMap<VkPhysicalDeviceCubicWeightsFeaturesQCOM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_WEIGHTS_FEATURES_QCOM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_WEIGHTS_FEATURES_QCOM> {
    typedef VkPhysicalDeviceCubicWeightsFeaturesQCOM Type;
};

// Map type VkSamplerCubicWeightsCreateInfoQCOM to id VK_STRUCTURE_TYPE_SAMPLER_CUBIC_WEIGHTS_CREATE_INFO_QCOM
template <> struct LvlTypeMap<VkSamplerCubicWeightsCreateInfoQCOM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SAMPLER_CUBIC_WEIGHTS_CREATE_INFO_QCOM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SAMPLER_CUBIC_WEIGHTS_CREATE_INFO_QCOM> {
    typedef VkSamplerCubicWeightsCreateInfoQCOM Type;
};

// Map type VkBlitImageCubicWeightsInfoQCOM to id VK_STRUCTURE_TYPE_BLIT_IMAGE_CUBIC_WEIGHTS_INFO_QCOM
template <> struct LvlTypeMap<VkBlitImageCubicWeightsInfoQCOM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BLIT_IMAGE_CUBIC_WEIGHTS_INFO_QCOM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BLIT_IMAGE_CUBIC_WEIGHTS_INFO_QCOM> {
    typedef VkBlitImageCubicWeightsInfoQCOM Type;
};

// Map type VkPhysicalDeviceYcbcrDegammaFeaturesQCOM to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_DEGAMMA_FEATURES_QCOM
template <> struct LvlTypeMap<VkPhysicalDeviceYcbcrDegammaFeaturesQCOM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_DEGAMMA_FEATURES_QCOM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_DEGAMMA_FEATURES_QCOM> {
    typedef VkPhysicalDeviceYcbcrDegammaFeaturesQCOM Type;
};

// Map type VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM to id VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_YCBCR_DEGAMMA_CREATE_INFO_QCOM
template <> struct LvlTypeMap<VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_YCBCR_DEGAMMA_CREATE_INFO_QCOM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_YCBCR_DEGAMMA_CREATE_INFO_QCOM> {
    typedef VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM Type;
};

// Map type VkPhysicalDeviceCubicClampFeaturesQCOM to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_CLAMP_FEATURES_QCOM
template <> struct LvlTypeMap<VkPhysicalDeviceCubicClampFeaturesQCOM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_CLAMP_FEATURES_QCOM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_CLAMP_FEATURES_QCOM> {
    typedef VkPhysicalDeviceCubicClampFeaturesQCOM Type;
};

// Map type VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_FEATURES_EXT> {
    typedef VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT Type;
};

#ifdef VK_USE_PLATFORM_SCREEN_QNX
// Map type VkScreenBufferPropertiesQNX to id VK_STRUCTURE_TYPE_SCREEN_BUFFER_PROPERTIES_QNX
template <> struct LvlTypeMap<VkScreenBufferPropertiesQNX> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SCREEN_BUFFER_PROPERTIES_QNX;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SCREEN_BUFFER_PROPERTIES_QNX> {
    typedef VkScreenBufferPropertiesQNX Type;
};

#endif // VK_USE_PLATFORM_SCREEN_QNX
#ifdef VK_USE_PLATFORM_SCREEN_QNX
// Map type VkScreenBufferFormatPropertiesQNX to id VK_STRUCTURE_TYPE_SCREEN_BUFFER_FORMAT_PROPERTIES_QNX
template <> struct LvlTypeMap<VkScreenBufferFormatPropertiesQNX> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SCREEN_BUFFER_FORMAT_PROPERTIES_QNX;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SCREEN_BUFFER_FORMAT_PROPERTIES_QNX> {
    typedef VkScreenBufferFormatPropertiesQNX Type;
};

#endif // VK_USE_PLATFORM_SCREEN_QNX
#ifdef VK_USE_PLATFORM_SCREEN_QNX
// Map type VkImportScreenBufferInfoQNX to id VK_STRUCTURE_TYPE_IMPORT_SCREEN_BUFFER_INFO_QNX
template <> struct LvlTypeMap<VkImportScreenBufferInfoQNX> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_SCREEN_BUFFER_INFO_QNX;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_SCREEN_BUFFER_INFO_QNX> {
    typedef VkImportScreenBufferInfoQNX Type;
};

#endif // VK_USE_PLATFORM_SCREEN_QNX
#ifdef VK_USE_PLATFORM_SCREEN_QNX
// Map type VkExternalFormatQNX to id VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_QNX
template <> struct LvlTypeMap<VkExternalFormatQNX> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_QNX;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_QNX> {
    typedef VkExternalFormatQNX Type;
};

#endif // VK_USE_PLATFORM_SCREEN_QNX
#ifdef VK_USE_PLATFORM_SCREEN_QNX
// Map type VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_SCREEN_BUFFER_FEATURES_QNX
template <> struct LvlTypeMap<VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_SCREEN_BUFFER_FEATURES_QNX;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_SCREEN_BUFFER_FEATURES_QNX> {
    typedef VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX Type;
};

#endif // VK_USE_PLATFORM_SCREEN_QNX
// Map type VkPhysicalDeviceLayeredDriverPropertiesMSFT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_DRIVER_PROPERTIES_MSFT
template <> struct LvlTypeMap<VkPhysicalDeviceLayeredDriverPropertiesMSFT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_DRIVER_PROPERTIES_MSFT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_DRIVER_PROPERTIES_MSFT> {
    typedef VkPhysicalDeviceLayeredDriverPropertiesMSFT Type;
};

// Map type VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_POOL_OVERALLOCATION_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_POOL_OVERALLOCATION_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_POOL_OVERALLOCATION_FEATURES_NV> {
    typedef VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV Type;
};

// Map type VkDisplaySurfaceStereoCreateInfoNV to id VK_STRUCTURE_TYPE_DISPLAY_SURFACE_STEREO_CREATE_INFO_NV
template <> struct LvlTypeMap<VkDisplaySurfaceStereoCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_STEREO_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DISPLAY_SURFACE_STEREO_CREATE_INFO_NV> {
    typedef VkDisplaySurfaceStereoCreateInfoNV Type;
};

// Map type VkDisplayModeStereoPropertiesNV to id VK_STRUCTURE_TYPE_DISPLAY_MODE_STEREO_PROPERTIES_NV
template <> struct LvlTypeMap<VkDisplayModeStereoPropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DISPLAY_MODE_STEREO_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DISPLAY_MODE_STEREO_PROPERTIES_NV> {
    typedef VkDisplayModeStereoPropertiesNV Type;
};

// Map type VkPhysicalDeviceRawAccessChainsFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAW_ACCESS_CHAINS_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceRawAccessChainsFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAW_ACCESS_CHAINS_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAW_ACCESS_CHAINS_FEATURES_NV> {
    typedef VkPhysicalDeviceRawAccessChainsFeaturesNV Type;
};

// Map type VkPhysicalDeviceCommandBufferInheritanceFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMMAND_BUFFER_INHERITANCE_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceCommandBufferInheritanceFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMMAND_BUFFER_INHERITANCE_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMMAND_BUFFER_INHERITANCE_FEATURES_NV> {
    typedef VkPhysicalDeviceCommandBufferInheritanceFeaturesNV Type;
};

// Map type VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT16_VECTOR_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT16_VECTOR_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT16_VECTOR_FEATURES_NV> {
    typedef VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV Type;
};

// Map type VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_REPLICATED_COMPOSITES_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_REPLICATED_COMPOSITES_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_REPLICATED_COMPOSITES_FEATURES_EXT> {
    typedef VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT Type;
};

// Map type VkPhysicalDeviceRayTracingValidationFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_VALIDATION_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceRayTracingValidationFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_VALIDATION_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_VALIDATION_FEATURES_NV> {
    typedef VkPhysicalDeviceRayTracingValidationFeaturesNV Type;
};

// Map type VkPhysicalDeviceClusterAccelerationStructureFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_ACCELERATION_STRUCTURE_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceClusterAccelerationStructureFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_ACCELERATION_STRUCTURE_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_ACCELERATION_STRUCTURE_FEATURES_NV> {
    typedef VkPhysicalDeviceClusterAccelerationStructureFeaturesNV Type;
};

// Map type VkPhysicalDeviceClusterAccelerationStructurePropertiesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_ACCELERATION_STRUCTURE_PROPERTIES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceClusterAccelerationStructurePropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_ACCELERATION_STRUCTURE_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_ACCELERATION_STRUCTURE_PROPERTIES_NV> {
    typedef VkPhysicalDeviceClusterAccelerationStructurePropertiesNV Type;
};

// Map type VkClusterAccelerationStructureClustersBottomLevelInputNV to id VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_CLUSTERS_BOTTOM_LEVEL_INPUT_NV
template <> struct LvlTypeMap<VkClusterAccelerationStructureClustersBottomLevelInputNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_CLUSTERS_BOTTOM_LEVEL_INPUT_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_CLUSTERS_BOTTOM_LEVEL_INPUT_NV> {
    typedef VkClusterAccelerationStructureClustersBottomLevelInputNV Type;
};

// Map type VkClusterAccelerationStructureTriangleClusterInputNV to id VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_TRIANGLE_CLUSTER_INPUT_NV
template <> struct LvlTypeMap<VkClusterAccelerationStructureTriangleClusterInputNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_TRIANGLE_CLUSTER_INPUT_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_TRIANGLE_CLUSTER_INPUT_NV> {
    typedef VkClusterAccelerationStructureTriangleClusterInputNV Type;
};

// Map type VkClusterAccelerationStructureMoveObjectsInputNV to id VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_MOVE_OBJECTS_INPUT_NV
template <> struct LvlTypeMap<VkClusterAccelerationStructureMoveObjectsInputNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_MOVE_OBJECTS_INPUT_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_MOVE_OBJECTS_INPUT_NV> {
    typedef VkClusterAccelerationStructureMoveObjectsInputNV Type;
};

// Map type VkClusterAccelerationStructureInputInfoNV to id VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_INPUT_INFO_NV
template <> struct LvlTypeMap<VkClusterAccelerationStructureInputInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_INPUT_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_INPUT_INFO_NV> {
    typedef VkClusterAccelerationStructureInputInfoNV Type;
};

// Map type VkClusterAccelerationStructureCommandsInfoNV to id VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_COMMANDS_INFO_NV
template <> struct LvlTypeMap<VkClusterAccelerationStructureCommandsInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_COMMANDS_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_COMMANDS_INFO_NV> {
    typedef VkClusterAccelerationStructureCommandsInfoNV Type;
};

// Map type VkAccelerationStructureBuildSizesInfoKHR to id VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR
template <> struct LvlTypeMap<VkAccelerationStructureBuildSizesInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR> {
    typedef VkAccelerationStructureBuildSizesInfoKHR Type;
};

// Map type VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV to id VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CLUSTER_ACCELERATION_STRUCTURE_CREATE_INFO_NV
template <> struct LvlTypeMap<VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CLUSTER_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CLUSTER_ACCELERATION_STRUCTURE_CREATE_INFO_NV> {
    typedef VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV Type;
};

// Map type VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PARTITIONED_ACCELERATION_STRUCTURE_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PARTITIONED_ACCELERATION_STRUCTURE_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PARTITIONED_ACCELERATION_STRUCTURE_FEATURES_NV> {
    typedef VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV Type;
};

// Map type VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PARTITIONED_ACCELERATION_STRUCTURE_PROPERTIES_NV
template <> struct LvlTypeMap<VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PARTITIONED_ACCELERATION_STRUCTURE_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PARTITIONED_ACCELERATION_STRUCTURE_PROPERTIES_NV> {
    typedef VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV Type;
};

// Map type VkPartitionedAccelerationStructureFlagsNV to id VK_STRUCTURE_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_FLAGS_NV
template <> struct LvlTypeMap<VkPartitionedAccelerationStructureFlagsNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_FLAGS_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_FLAGS_NV> {
    typedef VkPartitionedAccelerationStructureFlagsNV Type;
};

// Map type VkWriteDescriptorSetPartitionedAccelerationStructureNV to id VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_PARTITIONED_ACCELERATION_STRUCTURE_NV
template <> struct LvlTypeMap<VkWriteDescriptorSetPartitionedAccelerationStructureNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_PARTITIONED_ACCELERATION_STRUCTURE_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_PARTITIONED_ACCELERATION_STRUCTURE_NV> {
    typedef VkWriteDescriptorSetPartitionedAccelerationStructureNV Type;
};

// Map type VkPartitionedAccelerationStructureInstancesInputNV to id VK_STRUCTURE_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_INSTANCES_INPUT_NV
template <> struct LvlTypeMap<VkPartitionedAccelerationStructureInstancesInputNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_INSTANCES_INPUT_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_INSTANCES_INPUT_NV> {
    typedef VkPartitionedAccelerationStructureInstancesInputNV Type;
};

// Map type VkBuildPartitionedAccelerationStructureInfoNV to id VK_STRUCTURE_TYPE_BUILD_PARTITIONED_ACCELERATION_STRUCTURE_INFO_NV
template <> struct LvlTypeMap<VkBuildPartitionedAccelerationStructureInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUILD_PARTITIONED_ACCELERATION_STRUCTURE_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUILD_PARTITIONED_ACCELERATION_STRUCTURE_INFO_NV> {
    typedef VkBuildPartitionedAccelerationStructureInfoNV Type;
};

// Map type VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_EXT> {
    typedef VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT Type;
};

// Map type VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT Type;
};

// Map type VkGeneratedCommandsMemoryRequirementsInfoEXT to id VK_STRUCTURE_TYPE_GENERATED_COMMANDS_MEMORY_REQUIREMENTS_INFO_EXT
template <> struct LvlTypeMap<VkGeneratedCommandsMemoryRequirementsInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_GENERATED_COMMANDS_MEMORY_REQUIREMENTS_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_GENERATED_COMMANDS_MEMORY_REQUIREMENTS_INFO_EXT> {
    typedef VkGeneratedCommandsMemoryRequirementsInfoEXT Type;
};

// Map type VkIndirectExecutionSetPipelineInfoEXT to id VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_PIPELINE_INFO_EXT
template <> struct LvlTypeMap<VkIndirectExecutionSetPipelineInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_PIPELINE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_PIPELINE_INFO_EXT> {
    typedef VkIndirectExecutionSetPipelineInfoEXT Type;
};

// Map type VkIndirectExecutionSetShaderLayoutInfoEXT to id VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_SHADER_LAYOUT_INFO_EXT
template <> struct LvlTypeMap<VkIndirectExecutionSetShaderLayoutInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_SHADER_LAYOUT_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_SHADER_LAYOUT_INFO_EXT> {
    typedef VkIndirectExecutionSetShaderLayoutInfoEXT Type;
};

// Map type VkIndirectExecutionSetShaderInfoEXT to id VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_SHADER_INFO_EXT
template <> struct LvlTypeMap<VkIndirectExecutionSetShaderInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_SHADER_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_SHADER_INFO_EXT> {
    typedef VkIndirectExecutionSetShaderInfoEXT Type;
};

// Map type VkIndirectExecutionSetCreateInfoEXT to id VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkIndirectExecutionSetCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_CREATE_INFO_EXT> {
    typedef VkIndirectExecutionSetCreateInfoEXT Type;
};

// Map type VkGeneratedCommandsInfoEXT to id VK_STRUCTURE_TYPE_GENERATED_COMMANDS_INFO_EXT
template <> struct LvlTypeMap<VkGeneratedCommandsInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_GENERATED_COMMANDS_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_GENERATED_COMMANDS_INFO_EXT> {
    typedef VkGeneratedCommandsInfoEXT Type;
};

// Map type VkWriteIndirectExecutionSetPipelineEXT to id VK_STRUCTURE_TYPE_WRITE_INDIRECT_EXECUTION_SET_PIPELINE_EXT
template <> struct LvlTypeMap<VkWriteIndirectExecutionSetPipelineEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_WRITE_INDIRECT_EXECUTION_SET_PIPELINE_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_WRITE_INDIRECT_EXECUTION_SET_PIPELINE_EXT> {
    typedef VkWriteIndirectExecutionSetPipelineEXT Type;
};

// Map type VkIndirectCommandsLayoutTokenEXT to id VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_TOKEN_EXT
template <> struct LvlTypeMap<VkIndirectCommandsLayoutTokenEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_TOKEN_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_TOKEN_EXT> {
    typedef VkIndirectCommandsLayoutTokenEXT Type;
};

// Map type VkIndirectCommandsLayoutCreateInfoEXT to id VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkIndirectCommandsLayoutCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_CREATE_INFO_EXT> {
    typedef VkIndirectCommandsLayoutCreateInfoEXT Type;
};

// Map type VkGeneratedCommandsPipelineInfoEXT to id VK_STRUCTURE_TYPE_GENERATED_COMMANDS_PIPELINE_INFO_EXT
template <> struct LvlTypeMap<VkGeneratedCommandsPipelineInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_GENERATED_COMMANDS_PIPELINE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_GENERATED_COMMANDS_PIPELINE_INFO_EXT> {
    typedef VkGeneratedCommandsPipelineInfoEXT Type;
};

// Map type VkGeneratedCommandsShaderInfoEXT to id VK_STRUCTURE_TYPE_GENERATED_COMMANDS_SHADER_INFO_EXT
template <> struct LvlTypeMap<VkGeneratedCommandsShaderInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_GENERATED_COMMANDS_SHADER_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_GENERATED_COMMANDS_SHADER_INFO_EXT> {
    typedef VkGeneratedCommandsShaderInfoEXT Type;
};

// Map type VkWriteIndirectExecutionSetShaderEXT to id VK_STRUCTURE_TYPE_WRITE_INDIRECT_EXECUTION_SET_SHADER_EXT
template <> struct LvlTypeMap<VkWriteIndirectExecutionSetShaderEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_WRITE_INDIRECT_EXECUTION_SET_SHADER_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_WRITE_INDIRECT_EXECUTION_SET_SHADER_EXT> {
    typedef VkWriteIndirectExecutionSetShaderEXT Type;
};

// Map type VkPhysicalDeviceImageAlignmentControlFeaturesMESA to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ALIGNMENT_CONTROL_FEATURES_MESA
template <> struct LvlTypeMap<VkPhysicalDeviceImageAlignmentControlFeaturesMESA> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ALIGNMENT_CONTROL_FEATURES_MESA;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ALIGNMENT_CONTROL_FEATURES_MESA> {
    typedef VkPhysicalDeviceImageAlignmentControlFeaturesMESA Type;
};

// Map type VkPhysicalDeviceImageAlignmentControlPropertiesMESA to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ALIGNMENT_CONTROL_PROPERTIES_MESA
template <> struct LvlTypeMap<VkPhysicalDeviceImageAlignmentControlPropertiesMESA> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ALIGNMENT_CONTROL_PROPERTIES_MESA;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ALIGNMENT_CONTROL_PROPERTIES_MESA> {
    typedef VkPhysicalDeviceImageAlignmentControlPropertiesMESA Type;
};

// Map type VkImageAlignmentControlCreateInfoMESA to id VK_STRUCTURE_TYPE_IMAGE_ALIGNMENT_CONTROL_CREATE_INFO_MESA
template <> struct LvlTypeMap<VkImageAlignmentControlCreateInfoMESA> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_ALIGNMENT_CONTROL_CREATE_INFO_MESA;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_ALIGNMENT_CONTROL_CREATE_INFO_MESA> {
    typedef VkImageAlignmentControlCreateInfoMESA Type;
};

// Map type VkPhysicalDeviceDepthClampControlFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_CONTROL_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceDepthClampControlFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_CONTROL_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_CONTROL_FEATURES_EXT> {
    typedef VkPhysicalDeviceDepthClampControlFeaturesEXT Type;
};

// Map type VkPipelineViewportDepthClampControlCreateInfoEXT to id VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLAMP_CONTROL_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkPipelineViewportDepthClampControlCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLAMP_CONTROL_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLAMP_CONTROL_CREATE_INFO_EXT> {
    typedef VkPipelineViewportDepthClampControlCreateInfoEXT Type;
};

// Map type VkPhysicalDeviceHdrVividFeaturesHUAWEI to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HDR_VIVID_FEATURES_HUAWEI
template <> struct LvlTypeMap<VkPhysicalDeviceHdrVividFeaturesHUAWEI> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HDR_VIVID_FEATURES_HUAWEI;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HDR_VIVID_FEATURES_HUAWEI> {
    typedef VkPhysicalDeviceHdrVividFeaturesHUAWEI Type;
};

// Map type VkHdrVividDynamicMetadataHUAWEI to id VK_STRUCTURE_TYPE_HDR_VIVID_DYNAMIC_METADATA_HUAWEI
template <> struct LvlTypeMap<VkHdrVividDynamicMetadataHUAWEI> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_HDR_VIVID_DYNAMIC_METADATA_HUAWEI;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_HDR_VIVID_DYNAMIC_METADATA_HUAWEI> {
    typedef VkHdrVividDynamicMetadataHUAWEI Type;
};

// Map type VkCooperativeMatrixFlexibleDimensionsPropertiesNV to id VK_STRUCTURE_TYPE_COOPERATIVE_MATRIX_FLEXIBLE_DIMENSIONS_PROPERTIES_NV
template <> struct LvlTypeMap<VkCooperativeMatrixFlexibleDimensionsPropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COOPERATIVE_MATRIX_FLEXIBLE_DIMENSIONS_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COOPERATIVE_MATRIX_FLEXIBLE_DIMENSIONS_PROPERTIES_NV> {
    typedef VkCooperativeMatrixFlexibleDimensionsPropertiesNV Type;
};

// Map type VkPhysicalDeviceCooperativeMatrix2FeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_2_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceCooperativeMatrix2FeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_2_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_2_FEATURES_NV> {
    typedef VkPhysicalDeviceCooperativeMatrix2FeaturesNV Type;
};

// Map type VkPhysicalDeviceCooperativeMatrix2PropertiesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_2_PROPERTIES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceCooperativeMatrix2PropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_2_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_2_PROPERTIES_NV> {
    typedef VkPhysicalDeviceCooperativeMatrix2PropertiesNV Type;
};

// Map type VkPhysicalDevicePipelineOpacityMicromapFeaturesARM to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_OPACITY_MICROMAP_FEATURES_ARM
template <> struct LvlTypeMap<VkPhysicalDevicePipelineOpacityMicromapFeaturesARM> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_OPACITY_MICROMAP_FEATURES_ARM;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_OPACITY_MICROMAP_FEATURES_ARM> {
    typedef VkPhysicalDevicePipelineOpacityMicromapFeaturesARM Type;
};

#ifdef VK_USE_PLATFORM_METAL_EXT
// Map type VkImportMemoryMetalHandleInfoEXT to id VK_STRUCTURE_TYPE_IMPORT_MEMORY_METAL_HANDLE_INFO_EXT
template <> struct LvlTypeMap<VkImportMemoryMetalHandleInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_METAL_HANDLE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_MEMORY_METAL_HANDLE_INFO_EXT> {
    typedef VkImportMemoryMetalHandleInfoEXT Type;
};

#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
// Map type VkMemoryMetalHandlePropertiesEXT to id VK_STRUCTURE_TYPE_MEMORY_METAL_HANDLE_PROPERTIES_EXT
template <> struct LvlTypeMap<VkMemoryMetalHandlePropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_METAL_HANDLE_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_METAL_HANDLE_PROPERTIES_EXT> {
    typedef VkMemoryMetalHandlePropertiesEXT Type;
};

#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
// Map type VkMemoryGetMetalHandleInfoEXT to id VK_STRUCTURE_TYPE_MEMORY_GET_METAL_HANDLE_INFO_EXT
template <> struct LvlTypeMap<VkMemoryGetMetalHandleInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_GET_METAL_HANDLE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_GET_METAL_HANDLE_INFO_EXT> {
    typedef VkMemoryGetMetalHandleInfoEXT Type;
};

#endif // VK_USE_PLATFORM_METAL_EXT
// Map type VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_ROBUSTNESS_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_ROBUSTNESS_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_ROBUSTNESS_FEATURES_EXT> {
    typedef VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT Type;
};

// Map type VkAccelerationStructureGeometryTrianglesDataKHR to id VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR
template <> struct LvlTypeMap<VkAccelerationStructureGeometryTrianglesDataKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR> {
    typedef VkAccelerationStructureGeometryTrianglesDataKHR Type;
};

// Map type VkAccelerationStructureGeometryAabbsDataKHR to id VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR
template <> struct LvlTypeMap<VkAccelerationStructureGeometryAabbsDataKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR> {
    typedef VkAccelerationStructureGeometryAabbsDataKHR Type;
};

// Map type VkAccelerationStructureGeometryInstancesDataKHR to id VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR
template <> struct LvlTypeMap<VkAccelerationStructureGeometryInstancesDataKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR> {
    typedef VkAccelerationStructureGeometryInstancesDataKHR Type;
};

// Map type VkAccelerationStructureGeometryKHR to id VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR
template <> struct LvlTypeMap<VkAccelerationStructureGeometryKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR> {
    typedef VkAccelerationStructureGeometryKHR Type;
};

// Map type VkAccelerationStructureBuildGeometryInfoKHR to id VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR
template <> struct LvlTypeMap<VkAccelerationStructureBuildGeometryInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR> {
    typedef VkAccelerationStructureBuildGeometryInfoKHR Type;
};

// Map type VkAccelerationStructureCreateInfoKHR to id VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkAccelerationStructureCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR> {
    typedef VkAccelerationStructureCreateInfoKHR Type;
};

// Map type VkWriteDescriptorSetAccelerationStructureKHR to id VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR
template <> struct LvlTypeMap<VkWriteDescriptorSetAccelerationStructureKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR> {
    typedef VkWriteDescriptorSetAccelerationStructureKHR Type;
};

// Map type VkPhysicalDeviceAccelerationStructureFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceAccelerationStructureFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR> {
    typedef VkPhysicalDeviceAccelerationStructureFeaturesKHR Type;
};

// Map type VkPhysicalDeviceAccelerationStructurePropertiesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceAccelerationStructurePropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR> {
    typedef VkPhysicalDeviceAccelerationStructurePropertiesKHR Type;
};

// Map type VkAccelerationStructureDeviceAddressInfoKHR to id VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR
template <> struct LvlTypeMap<VkAccelerationStructureDeviceAddressInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR> {
    typedef VkAccelerationStructureDeviceAddressInfoKHR Type;
};

// Map type VkAccelerationStructureVersionInfoKHR to id VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_VERSION_INFO_KHR
template <> struct LvlTypeMap<VkAccelerationStructureVersionInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_VERSION_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_VERSION_INFO_KHR> {
    typedef VkAccelerationStructureVersionInfoKHR Type;
};

// Map type VkCopyAccelerationStructureToMemoryInfoKHR to id VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_TO_MEMORY_INFO_KHR
template <> struct LvlTypeMap<VkCopyAccelerationStructureToMemoryInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_TO_MEMORY_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_TO_MEMORY_INFO_KHR> {
    typedef VkCopyAccelerationStructureToMemoryInfoKHR Type;
};

// Map type VkCopyMemoryToAccelerationStructureInfoKHR to id VK_STRUCTURE_TYPE_COPY_MEMORY_TO_ACCELERATION_STRUCTURE_INFO_KHR
template <> struct LvlTypeMap<VkCopyMemoryToAccelerationStructureInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COPY_MEMORY_TO_ACCELERATION_STRUCTURE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COPY_MEMORY_TO_ACCELERATION_STRUCTURE_INFO_KHR> {
    typedef VkCopyMemoryToAccelerationStructureInfoKHR Type;
};

// Map type VkCopyAccelerationStructureInfoKHR to id VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR
template <> struct LvlTypeMap<VkCopyAccelerationStructureInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR> {
    typedef VkCopyAccelerationStructureInfoKHR Type;
};

// Map type VkRayTracingShaderGroupCreateInfoKHR to id VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkRayTracingShaderGroupCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR> {
    typedef VkRayTracingShaderGroupCreateInfoKHR Type;
};

// Map type VkRayTracingPipelineInterfaceCreateInfoKHR to id VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_INTERFACE_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkRayTracingPipelineInterfaceCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_INTERFACE_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_INTERFACE_CREATE_INFO_KHR> {
    typedef VkRayTracingPipelineInterfaceCreateInfoKHR Type;
};

// Map type VkRayTracingPipelineCreateInfoKHR to id VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkRayTracingPipelineCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR> {
    typedef VkRayTracingPipelineCreateInfoKHR Type;
};

// Map type VkPhysicalDeviceRayTracingPipelineFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceRayTracingPipelineFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR> {
    typedef VkPhysicalDeviceRayTracingPipelineFeaturesKHR Type;
};

// Map type VkPhysicalDeviceRayTracingPipelinePropertiesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceRayTracingPipelinePropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR> {
    typedef VkPhysicalDeviceRayTracingPipelinePropertiesKHR Type;
};

// Map type VkPhysicalDeviceRayQueryFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceRayQueryFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR> {
    typedef VkPhysicalDeviceRayQueryFeaturesKHR Type;
};

// Map type VkPhysicalDeviceMeshShaderFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceMeshShaderFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT> {
    typedef VkPhysicalDeviceMeshShaderFeaturesEXT Type;
};

// Map type VkPhysicalDeviceMeshShaderPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceMeshShaderPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceMeshShaderPropertiesEXT Type;
};

// Header "base class" for pNext chain traversal
struct LvlGenericHeader {
   VkStructureType sType;
   const LvlGenericHeader *pNext;
};
struct LvlGenericModHeader {
   VkStructureType sType;
   LvlGenericModHeader *pNext;
};

// Find an entry of the given type in the pNext chain
template <typename T> const T *lvl_find_in_chain(const void *next) {
    const LvlGenericHeader *current = reinterpret_cast<const LvlGenericHeader *>(next);
    const T *found = nullptr;
    while (current) {
        if (LvlTypeMap<T>::kSType == current->sType) {
            found = reinterpret_cast<const T*>(current);
            current = nullptr;
        } else {
            current = current->pNext;
        }
    }
    return found;
}
// Find an entry of the given type in the pNext chain
template <typename T> T *lvl_find_mod_in_chain(void *next) {
    LvlGenericModHeader *current = reinterpret_cast<LvlGenericModHeader *>(next);
    T *found = nullptr;
    while (current) {
        if (LvlTypeMap<T>::kSType == current->sType) {
            found = reinterpret_cast<T*>(current);
            current = nullptr;
        } else {
            current = current->pNext;
        }
    }
    return found;
}

// Init the header of an sType struct with pNext
template <typename T> T lvl_init_struct(void *p_next) {
    T out = {};
    out.sType = LvlTypeMap<T>::kSType;
    out.pNext = p_next;
    return out;
}

// Init the header of an sType struct
template <typename T> T lvl_init_struct() {
    T out = {};
    out.sType = LvlTypeMap<T>::kSType;
    return out;
}

