/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2022 NVIDIA CORPORATION.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <chrono>
#include <thread>

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/ray_tracing_objects.h"

// Tests for NVIDIA-specific best practices
const char *kEnableNVIDIAValidation = "VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_NVIDIA";

static constexpr float defaultQueuePriority = 0.0f;

class VkNvidiaBestPracticesLayerTest : public VkBestPracticesLayerTest {};

TEST_F(VkNvidiaBestPracticesLayerTest, PageableDeviceLocalMemory) {
    AddRequiredExtensions(VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableNVIDIAValidation));

    VkDeviceQueueCreateInfo queue_ci = vku::InitStructHelper();
    queue_ci.queueCount = 1;
    queue_ci.pQueuePriorities = &defaultQueuePriority;

    VkDeviceCreateInfo device_ci = vku::InitStructHelper();
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = &queue_ci;

    VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT pageable = vku::InitStructHelper();
    pageable.pageableDeviceLocalMemory = VK_TRUE;

    {
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-CreateDevice-PageableDeviceLocalMemory");
        VkDevice test_device = VK_NULL_HANDLE;
        VkResult err = vk::CreateDevice(Gpu(), &device_ci, nullptr, &test_device);
        m_errorMonitor->VerifyFound();
        if (err == VK_SUCCESS) {
            vk::DestroyDevice(test_device, nullptr);
        }
    }

    // Now enable the expected features
    device_ci.enabledExtensionCount = m_device_extension_names.size();
    device_ci.ppEnabledExtensionNames = m_device_extension_names.data();
    device_ci.pNext = &pageable;

    {
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-CreateDevice-PageableDeviceLocalMemory");
        vkt::Device test_device(Gpu(), device_ci);
        m_errorMonitor->Finish();
    }
}

TEST_F(VkNvidiaBestPracticesLayerTest, TilingLinear) {
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableNVIDIAValidation));
    RETURN_IF_SKIP(InitState());

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_A8B8G8R8_UNORM_PACK32;
    image_ci.extent = { 512, 512, 1 };
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    {
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-CreateImage-TilingLinear");
        vkt::Image image(*m_device, image_ci, vkt::no_mem);
        m_errorMonitor->Finish();
    }

    image_ci.tiling = VK_IMAGE_TILING_LINEAR;

    {
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-CreateImage-TilingLinear");
        vkt::Image image(*m_device, image_ci, vkt::no_mem);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkNvidiaBestPracticesLayerTest, Depth32Format) {
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableNVIDIAValidation));
    RETURN_IF_SKIP(InitState());

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    // This should be VK_FORMAT_D24_UNORM_S8_UINT, but that's not a required format.
    image_ci.format = VK_FORMAT_A8B8G8R8_UNORM_PACK32;
    image_ci.extent = { 512, 512, 1 };
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    {
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-CreateImage-Depth32Format");
        vkt::Image image(*m_device, image_ci, vkt::no_mem);
        m_errorMonitor->Finish();
    }

    VkFormat formats[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT };

    for (VkFormat format : formats) {
        image_ci.format = format;

        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-CreateImage-Depth32Format");
        vkt::Image image(*m_device, image_ci, vkt::no_mem);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkNvidiaBestPracticesLayerTest, QueueBindSparse_NotAsync) {
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableNVIDIAValidation));
    RETURN_IF_SKIP(InitState());

    if (!m_device->Physical().Features().sparseBinding) {
        GTEST_SKIP() << "Test requires sparseBinding";
    }

    VkDeviceQueueCreateInfo general_queue_ci = vku::InitStructHelper();
    general_queue_ci.queueCount = 1;
    general_queue_ci.pQueuePriorities = &defaultQueuePriority;
    {
        const std::optional<uint32_t> familyIndex = m_device->QueueFamily(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_SPARSE_BINDING_BIT);
        if (!familyIndex) {
            GTEST_SKIP() << "Required queue families not present";
        }
        general_queue_ci.queueFamilyIndex = familyIndex.value();
    }

    VkDeviceQueueCreateInfo transfer_queue_ci = vku::InitStructHelper();
    transfer_queue_ci.queueCount = 1;
    transfer_queue_ci.pQueuePriorities = &defaultQueuePriority;
    {
        const std::optional<uint32_t> familyIndex = m_device->QueueFamily(VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT,
                                                                          VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT);
        if (!familyIndex) {
            GTEST_SKIP() << "Required queue families not present";
        }
        transfer_queue_ci.queueFamilyIndex = familyIndex.value();
    }

    VkDeviceQueueCreateInfo queue_cis[2] = {
        general_queue_ci,
        transfer_queue_ci,
    };
    uint32_t queue_family_indices[] = {
        general_queue_ci.queueFamilyIndex,
        transfer_queue_ci.queueFamilyIndex,
    };

    VkPhysicalDeviceFeatures features = {};
    features.sparseBinding = VK_TRUE;

    VkDeviceCreateInfo device_ci = vku::InitStructHelper();
    device_ci.queueCreateInfoCount = 2;
    device_ci.pQueueCreateInfos = queue_cis;
    device_ci.pEnabledFeatures = &features;

    vkt::Device test_device(Gpu(), device_ci);

    VkQueue graphics_queue = VK_NULL_HANDLE;
    VkQueue transfer_queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(test_device.handle(), general_queue_ci.queueFamilyIndex, 0, &graphics_queue);
    vk::GetDeviceQueue(test_device.handle(), transfer_queue_ci.queueFamilyIndex, 0, &transfer_queue);

    VkBufferCreateInfo sparse_buffer_ci = vku::InitStructHelper();
    sparse_buffer_ci.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    sparse_buffer_ci.size = 0x10000;
    sparse_buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    sparse_buffer_ci.sharingMode = VK_SHARING_MODE_CONCURRENT;
    sparse_buffer_ci.queueFamilyIndexCount = 2;
    sparse_buffer_ci.pQueueFamilyIndices = queue_family_indices;

    vkt::Buffer sparse_buffer(test_device, sparse_buffer_ci, vkt::no_mem);

    const VkMemoryRequirements memory_requirements = sparse_buffer.MemoryRequirements();
    ASSERT_NE(memory_requirements.memoryTypeBits, 0);

    // Find first valid bit, whatever it is
    uint32_t memory_type_index = 0;
    while (((memory_requirements.memoryTypeBits >> memory_type_index) & 1) == 0) {
        ++memory_type_index;
    }

    VkMemoryAllocateInfo memory_ai = vku::InitStructHelper();
    memory_ai.allocationSize = memory_requirements.size;
    memory_ai.memoryTypeIndex = memory_type_index;

    vkt::DeviceMemory sparse_memory(test_device, memory_ai);

    VkSparseMemoryBind bind = {};
    bind.resourceOffset = 0;
    bind.size = sparse_buffer_ci.size;
    bind.memory = sparse_memory.handle();
    bind.memoryOffset = 0;
    bind.flags = 0;

    VkSparseBufferMemoryBindInfo sparse_buffer_bind = {};
    sparse_buffer_bind.buffer = sparse_buffer.handle();
    sparse_buffer_bind.bindCount = 1;
    sparse_buffer_bind.pBinds = &bind;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.bufferBindCount = 1;
    bind_info.pBufferBinds = &sparse_buffer_bind;

    {
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-QueueBindSparse-NotAsync");
        vk::QueueBindSparse(transfer_queue, 1, &bind_info, VK_NULL_HANDLE);
        m_errorMonitor->Finish();
    }

    test_device.Wait();

    {
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-QueueBindSparse-NotAsync");
        vk::QueueBindSparse(graphics_queue, 1, &bind_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkNvidiaBestPracticesLayerTest, AccelerationStructure_NotAsync) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableNVIDIAValidation));
    RETURN_IF_SKIP(InitState());

    vkt::Queue *graphics_queue = m_device->QueuesWithGraphicsCapability()[0];

    vkt::Queue *compute_queue = nullptr;
    for (uint32_t i = 0; i < m_device->QueuesWithComputeCapability().size(); ++i) {
        auto cqi = m_device->QueuesWithComputeCapability()[i];
        if (cqi->family_index != graphics_queue->family_index) {
            compute_queue = cqi;
            break;
        }
    }

    if (compute_queue == nullptr) {
        GTEST_SKIP() << "Could not find a compute queue different from the graphics queue, skipping test";
    }

    std::array<vkt::Queue *, 2> queues = {{graphics_queue, compute_queue}};

    auto build_geometry_info = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);

    for (vkt::Queue *queue : queues) {
        vkt::CommandPool compute_pool(*m_device, queue->family_index);
        vkt::CommandBuffer cmd_buffer(*m_device, compute_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        cmd_buffer.Begin();

        // Those 3 are triggered when allocating memory for the destination acceleration structure buffer and the scratch buffer.
        // This is expected.
        m_errorMonitor->SetAllowedFailureMsg("BestPractices-vkAllocateMemory-small-allocation");
        m_errorMonitor->SetAllowedFailureMsg("BestPractices-vkBindBufferMemory-small-dedicated-allocation");
        m_errorMonitor->SetAllowedFailureMsg("BestPractices-NVIDIA-AllocateMemory-SetPriority");

        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-AccelerationStructure-NotAsync");
        build_geometry_info.BuildCmdBuffer(cmd_buffer.handle());

        if (queue == compute_queue) {
            m_errorMonitor->Finish();
        } else {
            m_errorMonitor->VerifyFound();
        }

        cmd_buffer.End();
    }
}

TEST_F(VkNvidiaBestPracticesLayerTest, AllocateMemory_SetPriority) {
    AddRequiredExtensions(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableNVIDIAValidation));
    RETURN_IF_SKIP(InitState());

    VkMemoryAllocateInfo memory_ai = vku::InitStructHelper();
    memory_ai.allocationSize = 0x100000;
    memory_ai.memoryTypeIndex = 0;

    {
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-AllocateMemory-SetPriority");
        vkt::DeviceMemory memory(*m_device, memory_ai);
        m_errorMonitor->VerifyFound();
    }

    VkMemoryPriorityAllocateInfoEXT priority = vku::InitStructHelper();
    priority.priority = 0.5f;
    memory_ai.pNext = &priority;

    {
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-AllocateMemory-SetPriority");
        vkt::DeviceMemory memory(*m_device, memory_ai);
        m_errorMonitor->Finish();
    }
}

TEST_F(VkNvidiaBestPracticesLayerTest, AllocateMemory_ReuseAllocations) {
    AddRequiredExtensions(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableNVIDIAValidation));
    RETURN_IF_SKIP(InitState());

    VkMemoryAllocateInfo memory_ai = vku::InitStructHelper();
    memory_ai.allocationSize = 0x100000;
    memory_ai.memoryTypeIndex = 0;

    VkMemoryPriorityAllocateInfoEXT priority = vku::InitStructHelper();
    priority.priority = 0.5f;
    memory_ai.pNext = &priority;

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-AllocateMemory-ReuseAllocations");
    { vkt::DeviceMemory memory(*m_device, memory_ai); }

    std::this_thread::sleep_for(std::chrono::seconds{6});

    { vkt::DeviceMemory memory(*m_device, memory_ai); }

    m_errorMonitor->Finish();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-AllocateMemory-ReuseAllocations");

    { vkt::DeviceMemory memory(*m_device, memory_ai); }

    m_errorMonitor->VerifyFound();
}

TEST_F(VkNvidiaBestPracticesLayerTest, BindMemory_NoPriority) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableNVIDIAValidation));

    RETURN_IF_SKIP(InitState());

    VkDeviceQueueCreateInfo queue_ci = vku::InitStructHelper();
    queue_ci.queueFamilyIndex = 0;
    queue_ci.queueCount = 1;
    queue_ci.pQueuePriorities = &defaultQueuePriority;

    VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT pageable_features = vku::InitStructHelper();
    pageable_features.pNext = nullptr;
    pageable_features.pageableDeviceLocalMemory = VK_TRUE;

    VkPhysicalDeviceMaintenance4Features maintenance4_features = vku::InitStructHelper();
    maintenance4_features.pNext = &pageable_features;
    maintenance4_features.maintenance4 = VK_TRUE;

    VkDeviceCreateInfo device_ci = vku::InitStructHelper();
    device_ci.pNext = &maintenance4_features;
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = &queue_ci;
    device_ci.enabledExtensionCount = m_device_extension_names.size();
    device_ci.ppEnabledExtensionNames = m_device_extension_names.data();

    vkt::Device test_device(Gpu(), device_ci);

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 0x100000;
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(test_device, buffer_ci, vkt::no_mem);
    vkt::Buffer buffer_b(test_device, buffer_ci, vkt::no_mem);

    const VkMemoryRequirements memory_requirements = buffer_a.MemoryRequirements();
    ASSERT_NE(memory_requirements.memoryTypeBits, 0);

    // Find first valid bit, whatever it is
    uint32_t memory_type_index = 0;
    while (((memory_requirements.memoryTypeBits >> memory_type_index) & 1) == 0) {
        ++memory_type_index;
    }

    VkMemoryAllocateInfo memory_ai = vku::InitStructHelper();
    memory_ai.allocationSize = memory_requirements.size;
    memory_ai.memoryTypeIndex = memory_type_index;

    vkt::DeviceMemory memory(test_device, memory_ai);

    {
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-BindMemory-NoPriority");
        vk::BindBufferMemory(test_device.handle(), buffer_a.handle(), memory.handle(), 0);
        m_errorMonitor->VerifyFound();
    }

    vk::SetDeviceMemoryPriorityEXT(test_device.handle(), memory.handle(), 0.5f);

    {
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-BindMemory-NoPriority");
        vk::BindBufferMemory(test_device.handle(), buffer_b.handle(), memory.handle(), 0);
        m_errorMonitor->Finish();
    }
}

TEST_F(VkNvidiaBestPracticesLayerTest, BindMemory_StaticPriority) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableNVIDIAValidation));

    RETURN_IF_SKIP(InitState());

    VkDeviceQueueCreateInfo queue_ci = vku::InitStructHelper();
    queue_ci.queueFamilyIndex = 0;
    queue_ci.queueCount = 1;
    queue_ci.pQueuePriorities = &defaultQueuePriority;

    VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT pageable_features = vku::InitStructHelper();
    pageable_features.pNext = nullptr;
    pageable_features.pageableDeviceLocalMemory = VK_TRUE;

    VkPhysicalDeviceMaintenance4Features maintenance4_features = vku::InitStructHelper();
    maintenance4_features.pNext = &pageable_features;
    maintenance4_features.maintenance4 = VK_TRUE;

    VkDeviceCreateInfo device_ci = vku::InitStructHelper();
    device_ci.pNext = &maintenance4_features;
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = &queue_ci;
    device_ci.enabledExtensionCount = m_device_extension_names.size();
    device_ci.ppEnabledExtensionNames = m_device_extension_names.data();

    vkt::Device test_device(Gpu(), device_ci);

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 0x100000;
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(test_device, buffer_ci, vkt::no_mem);
    vkt::Buffer buffer_b(test_device, buffer_ci, vkt::no_mem);

    const VkMemoryRequirements memory_requirements = buffer_a.MemoryRequirements();
    ASSERT_NE(memory_requirements.memoryTypeBits, 0);

    // Find first valid bit, whatever it is
    uint32_t memory_type_index = 0;
    while (((memory_requirements.memoryTypeBits >> memory_type_index) & 1) == 0) {
        ++memory_type_index;
    }

    VkMemoryPriorityAllocateInfoEXT priority = vku::InitStructHelper();
    priority.priority = 0.5f;

    VkMemoryAllocateInfo memory_ai = vku::InitStructHelper();
    memory_ai.pNext = &priority;
    memory_ai.allocationSize = memory_requirements.size;
    memory_ai.memoryTypeIndex = memory_type_index;

    vkt::DeviceMemory memory(test_device, memory_ai);

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-BindMemory-NoPriority");
    vk::BindBufferMemory(test_device.handle(), buffer_b.handle(), memory.handle(), 0);
    m_errorMonitor->Finish();
}

static VkDescriptorSetLayoutBinding CreateSingleDescriptorBinding(VkDescriptorType type, uint32_t binding) {
    VkDescriptorSetLayoutBinding layout_binding = {};
    layout_binding.binding = binding;
    layout_binding.descriptorType = type;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layout_binding.pImmutableSamplers = nullptr;
    return layout_binding;
}

TEST_F(VkNvidiaBestPracticesLayerTest, CreatePipelineLayout_SeparateSampler) {
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableNVIDIAValidation));
    RETURN_IF_SKIP(InitState());

    VkDescriptorSetLayoutBinding separate_bindings[] = {
        CreateSingleDescriptorBinding(VK_DESCRIPTOR_TYPE_SAMPLER, 0),
        CreateSingleDescriptorBinding(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1),
    };
    VkDescriptorSetLayoutBinding combined_bindings[] = {
        CreateSingleDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0),
    };

    VkDescriptorSetLayoutCreateInfo separate_set_layout_ci = vku::InitStructHelper();
    separate_set_layout_ci.flags = 0;
    separate_set_layout_ci.bindingCount = sizeof(separate_bindings) / sizeof(separate_bindings[0]);
    separate_set_layout_ci.pBindings = separate_bindings;

    VkDescriptorSetLayoutCreateInfo combined_set_layout_ci = vku::InitStructHelper();
    combined_set_layout_ci.flags = 0;
    combined_set_layout_ci.bindingCount = sizeof(combined_bindings) / sizeof(combined_bindings[0]);
    combined_set_layout_ci.pBindings = combined_bindings;

    vkt::DescriptorSetLayout separate_set_layout(*m_device, separate_set_layout_ci);
    vkt::DescriptorSetLayout combined_set_layout(*m_device, combined_set_layout_ci);

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.flags = 0;
    pipeline_layout_ci.pushConstantRangeCount = 0;
    pipeline_layout_ci.pPushConstantRanges = nullptr;

    {
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-CreatePipelineLayout-SeparateSampler");
        vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci, {&separate_set_layout});
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-CreatePipelineLayout-SeparateSampler");
        vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci, {&combined_set_layout});
        m_errorMonitor->Finish();
    }
}

TEST_F(VkNvidiaBestPracticesLayerTest, CreatePipelineLayout_LargePipelineLayout) {
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableNVIDIAValidation));
    RETURN_IF_SKIP(InitState());
    if (m_device->Physical().limits_.maxPerStageDescriptorStorageBuffers < 16) {
        GTEST_SKIP() << "maxPerStageDescriptorStorageBuffers of 16 required";
    }

    VkDescriptorSetLayoutBinding large_bindings[] = {
        { 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 16, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
        { 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
    };
    VkDescriptorSetLayoutBinding small_bindings[] = {
        { 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 16, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
    };

    VkDescriptorSetLayoutCreateInfo large_set_layout_ci = vku::InitStructHelper();
    large_set_layout_ci.flags = 0;
    large_set_layout_ci.bindingCount = sizeof(large_bindings) / sizeof(large_bindings[0]);
    large_set_layout_ci.pBindings = large_bindings;

    VkDescriptorSetLayoutCreateInfo small_set_layout_ci = vku::InitStructHelper();
    small_set_layout_ci.flags = 0;
    small_set_layout_ci.bindingCount = sizeof(small_bindings) / sizeof(small_bindings[0]);
    small_set_layout_ci.pBindings = small_bindings;

    vkt::DescriptorSetLayout large_set_layout(*m_device, large_set_layout_ci);
    vkt::DescriptorSetLayout small_set_layout(*m_device, small_set_layout_ci);

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.flags = 0;
    pipeline_layout_ci.pushConstantRangeCount = 0;
    pipeline_layout_ci.pPushConstantRanges = nullptr;

    {
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                             "BestPractices-NVIDIA-CreatePipelineLayout-LargePipelineLayout");
        vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci, {&large_set_layout});
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                             "BestPractices-NVIDIA-CreatePipelineLayout-LargePipelineLayout");
        vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci, {&small_set_layout});
        m_errorMonitor->Finish();
    }
}

TEST_F(VkNvidiaBestPracticesLayerTest, BindPipeline_SwitchTessGeometryMesh) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::geometryShader);
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableNVIDIAValidation));
    RETURN_IF_SKIP(InitState());

    if (m_device->Physical().limits_.maxGeometryOutputVertices <= 3) {
        GTEST_SKIP() << "Device doesn't support requried maxGeometryOutputVertices";
    }

    char const *vsSource = R"glsl(
        #version 450
        void main() {}
    )glsl";

    char const *gsSource = R"glsl(
        #version 450
        layout(triangles) in;
        layout(triangle_strip, max_vertices = 3) out;
        void main() {}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();

    CreatePipelineHelper vsPipe(*this, &pipeline_rendering_info);
    vsPipe.shader_stages_ = {vs.GetStageCreateInfo()};
    vsPipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    vsPipe.CreateGraphicsPipeline();

    CreatePipelineHelper vgsPipe(*this, &pipeline_rendering_info);
    vgsPipe.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo()};
    vgsPipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    vgsPipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();

    {
        m_errorMonitor->SetAllowedFailureMsg("BestPractices-Pipeline-SortAndBind");
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-BindPipeline-SwitchTessGeometryMesh");
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, vsPipe.Handle());
        m_errorMonitor->Finish();
    }
    {
        m_errorMonitor->SetAllowedFailureMsg("BestPractices-Pipeline-SortAndBind");
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-BindPipeline-SwitchTessGeometryMesh");
        for (int i = 0; i < 10; ++i) {
            vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, vgsPipe.Handle());
            vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, vsPipe.Handle());
        }
        m_errorMonitor->VerifyFound();
    }
}

// TODO - This test needs to move the positive checks to new test because currently they will trigger many other errors
TEST_F(VkNvidiaBestPracticesLayerTest, BindPipelineZcullDirection) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableNVIDIAValidation));
    RETURN_IF_SKIP(InitState());

    VkFormat depth_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.depthAttachmentFormat = depth_format;
    pipeline_rendering_info.stencilAttachmentFormat = depth_format;

    // 3 array layers
    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 3, depth_format,
                                                  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image(*m_device, image_ci);

    VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
    image_view_ci.image = image.handle();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.format = depth_format;
    // rendering to layer index 1
    image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 1, 1};

    vkt::ImageView depth_image_view(*m_device, image_view_ci);

    VkRenderingAttachmentInfo depth_attachment = vku::InitStructHelper();
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    depth_attachment.imageView = depth_image_view.handle();
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.renderArea.extent = {32, 32};
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.pDepthAttachment = &depth_attachment;
    begin_rendering_info.pStencilAttachment = &depth_attachment;

    VkClearRect clear_rect{};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;
    clear_rect.rect.extent = {32, 32};

    VkClearAttachment attachment{};
    attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    VkImageMemoryBarrier discard_barrier = vku::InitStructHelper();
    discard_barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    discard_barrier.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;
    discard_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    discard_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    discard_barrier.image = image.handle();
    discard_barrier.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 1,
                                        VK_REMAINING_ARRAY_LAYERS};

    VkImageMemoryBarrier2 discard_barrier2 = vku::InitStructHelper();
    discard_barrier2.srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
    discard_barrier2.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;
    discard_barrier2.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    discard_barrier2.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    discard_barrier2.image = image.handle();
    discard_barrier2.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 1,
                                         VK_REMAINING_ARRAY_LAYERS};

    auto set_desired_failure_msg = [this] {
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-Zcull-LessGreaterRatio");
    };

    VkPipelineDepthStencilStateCreateInfo depth_stencil_state_ci = vku::InitStructHelper();

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.ds_ci_ = depth_stencil_state_ci;
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_COMPARE_OP);
    pipe.CreateGraphicsPipeline();

    auto cmd = m_command_buffer.handle();
    m_command_buffer.Begin();

    vk::CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdSetDepthTestEnable(cmd, VK_TRUE);

    {
        SCOPED_TRACE("Unbalance");

        vk::CmdBeginRendering(cmd, &begin_rendering_info);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_LESS);
        for (int i = 0; i < 90; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_GREATER);
        for (int i = 0; i < 10; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        set_desired_failure_msg();
        vk::CmdEndRendering(cmd);
        m_errorMonitor->Finish();
    }
    {
        SCOPED_TRACE("Balance");

        vk::CmdBeginRendering(cmd, &begin_rendering_info);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_LESS);
        for (int i = 0; i < 60; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_GREATER);
        for (int i = 0; i < 40; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        set_desired_failure_msg();
        vk::CmdEndRendering(cmd);
        m_errorMonitor->VerifyFound();

        vk::CmdEndRendering(cmd);  // need to actually end rendering
    }

    {
        // This should miss because BeginRendering uses LOAD_OP_CLEAR
        SCOPED_TRACE("Balance with end rendering");

        vk::CmdBeginRendering(cmd, &begin_rendering_info);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_LESS);
        for (int i = 0; i < 60; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        vk::CmdEndRendering(cmd);

        vk::CmdBeginRendering(cmd, &begin_rendering_info);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_GREATER);
        for (int i = 0; i < 40; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        set_desired_failure_msg();
        vk::CmdEndRendering(cmd);
        m_errorMonitor->Finish();
    }

    {
        SCOPED_TRACE("Clear before balance");

        vk::CmdBeginRendering(cmd, &begin_rendering_info);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_LESS);
        for (int i = 0; i < 60; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        vk::CmdClearAttachments(cmd, 1, &attachment, 1, &clear_rect);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_GREATER);
        for (int i = 0; i < 40; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        set_desired_failure_msg();
        vk::CmdEndRendering(cmd);
        m_errorMonitor->Finish();
    }

    {
        SCOPED_TRACE("Balance before clear");

        vk::CmdBeginRendering(cmd, &begin_rendering_info);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_LESS);
        for (int i = 0; i < 60; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_GREATER);
        for (int i = 0; i < 40; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        set_desired_failure_msg();
        vk::CmdClearAttachments(cmd, 1, &attachment, 1, &clear_rect);
        m_errorMonitor->VerifyFound();

        vk::CmdEndRendering(cmd);  // need to actually end rendering
    }

    m_command_buffer.End();
}

TEST_F(VkNvidiaBestPracticesLayerTest, BindPipelineZcullDirectionDepth) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableNVIDIAValidation));
    RETURN_IF_SKIP(InitState());

    VkFormat depth_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.depthAttachmentFormat = depth_format;
    pipeline_rendering_info.stencilAttachmentFormat = depth_format;

    // 3 array layers
    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 3, depth_format,
                                                  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image(*m_device, image_ci);

    VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
    image_view_ci.image = image.handle();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.format = depth_format;
    // rendering to layer index 1
    image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 1, 1};

    vkt::ImageView depth_image_view(*m_device, image_view_ci);

    VkRenderingAttachmentInfo depth_attachment = vku::InitStructHelper();
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    depth_attachment.imageView = depth_image_view.handle();
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.renderArea.extent = {32, 32};
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.pDepthAttachment = &depth_attachment;
    begin_rendering_info.pStencilAttachment = &depth_attachment;

    VkImageMemoryBarrier discard_barrier = vku::InitStructHelper();
    discard_barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    discard_barrier.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;
    discard_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    discard_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    discard_barrier.image = image.handle();
    discard_barrier.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 1,
                                        VK_REMAINING_ARRAY_LAYERS};

    VkImageMemoryBarrier2 discard_barrier2 = vku::InitStructHelper();
    discard_barrier2.srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
    discard_barrier2.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;
    discard_barrier2.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    discard_barrier2.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    discard_barrier2.image = image.handle();
    discard_barrier2.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 1,
                                         VK_REMAINING_ARRAY_LAYERS};

    VkDependencyInfo discard_dependency_info = vku::InitStructHelper();
    discard_dependency_info.imageMemoryBarrierCount = 1;
    discard_dependency_info.pImageMemoryBarriers = &discard_barrier2;

    auto set_desired_failure_msg = [this] {
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-Zcull-LessGreaterRatio");
    };

    VkPipelineDepthStencilStateCreateInfo depth_stencil_state_ci = vku::InitStructHelper();

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.ds_ci_ = depth_stencil_state_ci;
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_COMPARE_OP);
    pipe.CreateGraphicsPipeline();

    auto cmd = m_command_buffer.handle();
    m_command_buffer.Begin();

    vk::CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdSetDepthTestEnable(cmd, VK_TRUE);

    {
        SCOPED_TRACE("Load previous scope");

        vk::CmdBeginRendering(cmd, &begin_rendering_info);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_LESS);
        for (int i = 0; i < 60; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        vk::CmdEndRendering(cmd);

        vk::CmdBeginRendering(cmd, &begin_rendering_info);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_GREATER);
        for (int i = 0; i < 40; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        set_desired_failure_msg();
        vk::CmdEndRendering(cmd);
        m_errorMonitor->VerifyFound();

        vk::CmdEndRendering(cmd);  // need to actually end rendering
    }

    {
        SCOPED_TRACE("Transition to discard");

        vk::CmdBeginRendering(cmd, &begin_rendering_info);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_LESS);
        for (int i = 0; i < 60; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        vk::CmdEndRendering(cmd);

        vk::CmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0,
                               nullptr, 1, &discard_barrier);

        vk::CmdBeginRendering(cmd, &begin_rendering_info);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_GREATER);
        for (int i = 0; i < 40; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        set_desired_failure_msg();
        vk::CmdEndRendering(cmd);
        m_errorMonitor->Finish();
    }

    {
        SCOPED_TRACE("Transition to discard 2");

        vk::CmdBeginRendering(cmd, &begin_rendering_info);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_LESS);
        for (int i = 0; i < 60; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        vk::CmdEndRendering(cmd);

        vk::CmdPipelineBarrier2(cmd, &discard_dependency_info);

        vk::CmdBeginRendering(cmd, &begin_rendering_info);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_GREATER);
        for (int i = 0; i < 40; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        set_desired_failure_msg();
        vk::CmdEndRendering(cmd);
        m_errorMonitor->Finish();
    }

    {
        SCOPED_TRACE("Balance before discard");

        vk::CmdBeginRendering(cmd, &begin_rendering_info);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_LESS);
        for (int i = 0; i < 60; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_GREATER);
        for (int i = 0; i < 40; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        vk::CmdEndRendering(cmd);

        set_desired_failure_msg();
        vk::CmdPipelineBarrier2(cmd, &discard_dependency_info);
        m_errorMonitor->VerifyFound();
    }

    {
        SCOPED_TRACE("Transfer clear to discard");

        vk::CmdBeginRendering(cmd, &begin_rendering_info);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_LESS);
        for (int i = 0; i < 60; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        vk::CmdEndRendering(cmd);

        VkClearDepthStencilValue ds_value{};
        vk::CmdClearDepthStencilImage(cmd, image.handle(), VK_IMAGE_LAYOUT_GENERAL, &ds_value, 1, &discard_barrier.subresourceRange);

        vk::CmdBeginRendering(cmd, &begin_rendering_info);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_GREATER);
        for (int i = 0; i < 40; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        set_desired_failure_msg();
        vk::CmdEndRendering(cmd);
        m_errorMonitor->Finish();
    }

    m_command_buffer.End();
}

TEST_F(VkNvidiaBestPracticesLayerTest, BindPipelineZcullDirectionSubresource) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableNVIDIAValidation));
    RETURN_IF_SKIP(InitState());

    VkFormat depth_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.depthAttachmentFormat = depth_format;
    pipeline_rendering_info.stencilAttachmentFormat = depth_format;

    // 3 array layers
    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 3, depth_format,
                                                  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image(*m_device, image_ci);

    VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
    image_view_ci.image = image.handle();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.format = depth_format;
    // rendering to layer index 1
    image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 1, 1};

    vkt::ImageView depth_image_view(*m_device, image_view_ci);

    VkRenderingAttachmentInfo depth_attachment = vku::InitStructHelper();
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    depth_attachment.imageView = depth_image_view.handle();
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.renderArea.extent = {32, 32};
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.pDepthAttachment = &depth_attachment;
    begin_rendering_info.pStencilAttachment = &depth_attachment;

    VkImageMemoryBarrier discard_barrier = vku::InitStructHelper();
    discard_barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    discard_barrier.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;
    discard_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    discard_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    discard_barrier.image = image.handle();
    discard_barrier.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 1,
                                        VK_REMAINING_ARRAY_LAYERS};

    VkImageMemoryBarrier2 discard_barrier2 = vku::InitStructHelper();
    discard_barrier2.srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
    discard_barrier2.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;
    discard_barrier2.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    discard_barrier2.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    discard_barrier2.image = image.handle();
    discard_barrier2.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 1,
                                         VK_REMAINING_ARRAY_LAYERS};

    auto set_desired_failure_msg = [this] {
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-Zcull-LessGreaterRatio");
    };

    VkPipelineDepthStencilStateCreateInfo depth_stencil_state_ci = vku::InitStructHelper();

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.ds_ci_ = depth_stencil_state_ci;
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_COMPARE_OP);
    pipe.CreateGraphicsPipeline();

    auto cmd = m_command_buffer.handle();
    m_command_buffer.Begin();

    vk::CmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdSetDepthTestEnable(cmd, VK_TRUE);

    {
        SCOPED_TRACE("Transfer clear to validate");

        vk::CmdBeginRendering(cmd, &begin_rendering_info);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_LESS);
        for (int i = 0; i < 60; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_GREATER);
        for (int i = 0; i < 40; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        vk::CmdEndRendering(cmd);

        set_desired_failure_msg();
        VkClearDepthStencilValue ds_value{};
        vk::CmdClearDepthStencilImage(cmd, image.handle(), VK_IMAGE_LAYOUT_GENERAL, &ds_value, 1, &discard_barrier.subresourceRange);
        m_errorMonitor->VerifyFound();
    }

    discard_barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    {
        SCOPED_TRACE("Transfer clear with VK_REMAINING_ARRAY_LAYERS");

        vk::CmdBeginRendering(cmd, &begin_rendering_info);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_LESS);
        for (int i = 0; i < 60; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        vk::CmdEndRendering(cmd);

        VkClearDepthStencilValue ds_value{};
        vk::CmdClearDepthStencilImage(cmd, image.handle(), VK_IMAGE_LAYOUT_GENERAL, &ds_value, 1,
                                      &discard_barrier.subresourceRange);

        vk::CmdBeginRendering(cmd, &begin_rendering_info);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_GREATER);
        for (int i = 0; i < 40; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        set_desired_failure_msg();
        vk::CmdEndRendering(cmd);
        m_errorMonitor->Finish();
    }

    discard_barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;

    {
        SCOPED_TRACE("Transfer clear with VK_REMAINING_MIP_LEVELS");

        vk::CmdBeginRendering(cmd, &begin_rendering_info);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_LESS);
        for (int i = 0; i < 60; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        vk::CmdEndRendering(cmd);

        VkClearDepthStencilValue ds_value{};
        vk::CmdClearDepthStencilImage(cmd, image.handle(), VK_IMAGE_LAYOUT_GENERAL, &ds_value, 1,
                                      &discard_barrier.subresourceRange);

        vk::CmdBeginRendering(cmd, &begin_rendering_info);

        vk::CmdSetDepthCompareOp(cmd, VK_COMPARE_OP_GREATER);
        for (int i = 0; i < 40; ++i) vk::CmdDraw(m_command_buffer.handle(), 0, 0, 0, 0);

        set_desired_failure_msg();
        vk::CmdEndRendering(cmd);
        m_errorMonitor->Finish();
    }

    m_command_buffer.End();
}

TEST_F(VkNvidiaBestPracticesLayerTest, ClearColor_NotCompressed) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableNVIDIAValidation));
    RETURN_IF_SKIP(InitState());

    auto set_desired = [this] {
        m_errorMonitor->Finish();
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-ClearColor-NotCompressed");
        m_errorMonitor->SetAllowedFailureMsg("BestPractices-DrawState-ClearCmdBeforeDraw");
    };

    vkt::Image image(*m_device, m_width, m_height, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView image_view = image.CreateView();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = image_view.handle();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.clearValue = {};

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.renderArea.extent = {m_width, m_height};
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    VkClearAttachment clear{};
    clear.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clear.clearValue.color.float32[0] = 0.0f;
    clear.clearValue.color.float32[1] = 0.0f;
    clear.clearValue.color.float32[2] = 0.0f;
    clear.clearValue.color.float32[3] = 0.0f;
    clear.colorAttachment = 0;

    auto set_clear_color = [&clear](const std::array<float, 4>& color) {
        for (size_t i = 0; i < 4; ++i) {
            clear.clearValue.color.float32[i] = color[i];
        }
    };

    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};

    m_command_buffer.Begin();
    vk::CmdBeginRendering(m_command_buffer.handle(), &begin_rendering_info);

    {
        set_desired();
        set_clear_color({1.0f, 0.5f, 0.25f, 0.0f});

        for (int i = 0; i < 16 + 1; ++i) {
            clear.clearValue.color.float32[3] += 0.05f;
            vk::CmdClearAttachments(m_command_buffer.handle(), 1, &clear, 1, &clear_rect);
        }
        m_errorMonitor->VerifyFound();
    }
    {
        set_desired();
        set_clear_color({1.0f, 1.0f, 1.0f, 1.0f});
        vk::CmdClearAttachments(m_command_buffer.handle(), 1, &clear, 1, &clear_rect);
        m_errorMonitor->Finish();
    }
    {
        set_desired();
        set_clear_color({0.0f, 0.0f, 0.0f, 0.0f});
        vk::CmdClearAttachments(m_command_buffer.handle(), 1, &clear, 1, &clear_rect);
        m_errorMonitor->Finish();
    }
    {
        set_desired();
        set_clear_color({0.9f, 1.0f, 1.0f, 1.0f});
        vk::CmdClearAttachments(m_command_buffer.handle(), 1, &clear, 1, &clear_rect);
        m_errorMonitor->VerifyFound();
    }

    vk::CmdEndRendering(m_command_buffer.handle());

    {
        set_desired();
        vk::CmdBeginRendering(m_command_buffer.handle(), &begin_rendering_info);
        m_errorMonitor->Finish();
        vk::CmdEndRendering(m_command_buffer.handle());
    }
    {
        color_attachment.clearValue.color.float32[0] = 0.55f;

        set_desired();
        vk::CmdBeginRendering(m_command_buffer.handle(), &begin_rendering_info);
        m_errorMonitor->VerifyFound();
    }

    m_command_buffer.End();
}

TEST_F(VkNvidiaBestPracticesLayerTest, BeginCommandBuffer_OneTimeSubmit) {
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableNVIDIAValidation));
    RETURN_IF_SKIP(InitState());

    VkCommandPoolCreateInfo command_pool_ci = vku::InitStructHelper();
    command_pool_ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_ci.queueFamilyIndex = m_device->graphics_queue_node_index_;

    vkt::CommandPool command_pool(*m_device, command_pool_ci);

    VkCommandBufferAllocateInfo allocate_info = vku::InitStructHelper();
    allocate_info.commandPool = command_pool.handle();
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = 1;
    vkt::CommandBuffer command_buffer0(*m_device, allocate_info);
    vkt::CommandBuffer command_buffer1(*m_device, allocate_info);
    vkt::CommandBuffer command_buffer2(*m_device, allocate_info);

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();

    {
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-vkBeginCommandBuffer-one-time-submit");

        command_buffer0.Begin(&begin_info);
        command_buffer0.End();

        m_default_queue->Submit(command_buffer0);
        m_device->Wait();

        vk::BeginCommandBuffer(command_buffer0.handle(), &begin_info);
        m_errorMonitor->VerifyFound();
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-vkBeginCommandBuffer-one-time-submit");

        command_buffer1.Begin(&begin_info);
        command_buffer1.End();

        for (int i = 0; i < 2; ++i) {
            m_default_queue->Submit(command_buffer1);
            m_device->Wait();
        }

        vk::BeginCommandBuffer(command_buffer1.handle(), &begin_info);
        m_errorMonitor->Finish();
    }
    {
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-NVIDIA-vkBeginCommandBuffer-one-time-submit");

        command_buffer2.Begin(&begin_info);
        command_buffer2.End();

        m_default_queue->Submit(command_buffer2);
        m_device->Wait();

        vk::BeginCommandBuffer(command_buffer2.handle(), &begin_info);
        m_errorMonitor->Finish();
    }
}
