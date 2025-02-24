/*
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

class NegativeDeviceQueue : public VkLayerTest {};

TEST_F(NegativeDeviceQueue, FamilyIndex) {
    TEST_DESCRIPTION("Create device queue with invalid queue family index.");

    RETURN_IF_SKIP(InitFramework());

    uint32_t queue_family_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_props(queue_family_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_family_count, queue_props.data());

    uint32_t queue_family_index = queue_family_count;

    float priority = 1.0f;
    VkDeviceQueueCreateInfo device_queue_ci = vku::InitStructHelper();
    device_queue_ci.queueFamilyIndex = queue_family_index;
    device_queue_ci.queueCount = 1;
    device_queue_ci.pQueuePriorities = &priority;

    VkDeviceCreateInfo device_ci = vku::InitStructHelper();
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = &device_queue_ci;
    device_ci.enabledLayerCount = 0;
    device_ci.enabledExtensionCount = m_device_extension_names.size();
    device_ci.ppEnabledExtensionNames = m_device_extension_names.data();

    m_errorMonitor->SetDesiredError("VUID-VkDeviceQueueCreateInfo-queueFamilyIndex-00381");
    VkDevice device;
    vk::CreateDevice(Gpu(), &device_ci, nullptr, &device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceQueue, FamilyIndexUsage) {
    TEST_DESCRIPTION("Using device queue with invalid queue family index.");
    bool get_physical_device_properties2 = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (get_physical_device_properties2) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    all_queue_count_ = true;
    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    VkBufferCreateInfo buffCI = vku::InitStructHelper();
    buffCI.size = 1024;
    buffCI.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffCI.queueFamilyIndexCount = 2;
    // Introduce failure by specifying invalid queue_family_index
    uint32_t qfi[2];
    qfi[0] = 777;
    qfi[1] = 0;

    buffCI.pQueueFamilyIndices = qfi;
    buffCI.sharingMode = VK_SHARING_MODE_CONCURRENT;  // qfi only matters in CONCURRENT mode

    // Test for queue family index out of range
    CreateBufferTest(*this, &buffCI, "VUID-VkBufferCreateInfo-sharingMode-01419");

    // Test for non-unique QFI in array
    qfi[0] = 0;
    CreateBufferTest(*this, &buffCI, "VUID-VkBufferCreateInfo-sharingMode-01419");

    if (m_device->Physical().queue_properties_.size() > 2) {
        m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pSubmits-04626");

        // Create buffer shared to queue families 1 and 2, but submitted on queue family 0
        buffCI.queueFamilyIndexCount = 2;
        qfi[0] = 1;
        qfi[1] = 2;
        vkt::Buffer ib;
        ib.init(*m_device, buffCI);

        m_command_buffer.Begin();
        vk::CmdFillBuffer(m_command_buffer.handle(), ib.handle(), 0, 16, 5);
        m_command_buffer.End();
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }

    // If there is more than one queue family, create a device with a single queue family, then create a buffer
    // with SHARING_MODE_CONCURRENT that uses a non-device PDEV queue family.
    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_count, NULL);
    std::vector<VkQueueFamilyProperties> queue_props;
    queue_props.resize(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_count, queue_props.data());

    if (queue_count < 3) {
        GTEST_SKIP() << "Multiple queue families are required to run this test.";
    }
    std::vector<float> priorities(queue_props.at(0).queueCount, 1.0f);
    VkDeviceQueueCreateInfo queue_info = vku::InitStructHelper();
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = queue_props.at(0).queueCount;
    queue_info.pQueuePriorities = priorities.data();
    VkDeviceCreateInfo dev_info = vku::InitStructHelper();
    dev_info.queueCreateInfoCount = 1;
    dev_info.pQueueCreateInfos = &queue_info;
    dev_info.enabledLayerCount = 0;
    dev_info.enabledExtensionCount = m_device_extension_names.size();
    dev_info.ppEnabledExtensionNames = m_device_extension_names.data();

    // Create a device with a single queue family
    VkDevice second_device;
    ASSERT_EQ(VK_SUCCESS, vk::CreateDevice(Gpu(), &dev_info, nullptr, &second_device));

    // Select Queue family for CONCURRENT buffer that is not owned by device
    buffCI.queueFamilyIndexCount = 2;
    qfi[1] = 2;
    VkBuffer buffer = VK_NULL_HANDLE;
    vk::CreateBuffer(second_device, &buffCI, NULL, &buffer);
    vk::DestroyBuffer(second_device, buffer, nullptr);
    vk::DestroyDevice(second_device, nullptr);
}

TEST_F(NegativeDeviceQueue, FamilyIndexUnique) {
    TEST_DESCRIPTION("Vulkan 1.0 unique queue detection");
    SetTargetApiVersion(VK_API_VERSION_1_0);

    RETURN_IF_SKIP(InitFramework());

    // use first queue family with at least 2 queues in it
    bool found_queue = false;
    VkQueueFamilyProperties queue_properties;  // selected queue family used
    uint32_t queue_family_index = 0;
    uint32_t queue_family_count = 0;
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_family_count, queue_families.data());

    for (size_t i = 0; i < queue_families.size(); i++) {
        if (queue_families[i].queueCount > 1) {
            found_queue = true;
            queue_family_index = i;
            queue_properties = queue_families[i];
            break;
        }
    }

    if (found_queue == false) {
        GTEST_SKIP() << "test requires queue family with 2 queues, not available";
    }

    float queue_priority = 1.0;
    VkDeviceQueueCreateInfo queue_create_info[2];
    queue_create_info[0] = vku::InitStructHelper();
    queue_create_info[0].flags = 0;
    queue_create_info[0].queueFamilyIndex = queue_family_index;
    queue_create_info[0].queueCount = 1;
    queue_create_info[0].pQueuePriorities = &queue_priority;

    // queueFamilyIndex is the same
    queue_create_info[1] = queue_create_info[0];

    VkDevice test_device = VK_NULL_HANDLE;
    VkDeviceCreateInfo device_create_info = vku::InitStructHelper();
    device_create_info.flags = 0;
    device_create_info.pQueueCreateInfos = queue_create_info;
    device_create_info.queueCreateInfoCount = 2;
    device_create_info.pEnabledFeatures = nullptr;
    device_create_info.enabledLayerCount = 0;
    device_create_info.enabledExtensionCount = 0;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-queueFamilyIndex-02802");
    vk::CreateDevice(Gpu(), &device_create_info, nullptr, &test_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceQueue, MismatchedGlobalPriority) {
    TEST_DESCRIPTION("Create multiple device queues with same queue family index but different global priorty.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GLOBAL_PRIORITY_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework());

    uint32_t queue_family_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_props(queue_family_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_family_count, queue_props.data());

    uint32_t queue_family_index = queue_family_count;

    for (uint32_t i = 0; i < queue_family_count; ++i) {
        if (queue_props[i].queueCount > 1) {
            queue_family_index = i;
            break;
        }
    }
    if (queue_family_index == queue_family_count) {
        GTEST_SKIP() << "Multiple queues from same queue family are required to run this test";
    }

    VkDeviceQueueGlobalPriorityCreateInfo queue_global_priority_ci[2] = {};
    queue_global_priority_ci[0] = vku::InitStructHelper();
    queue_global_priority_ci[0].globalPriority = VK_QUEUE_GLOBAL_PRIORITY_LOW;
    queue_global_priority_ci[1] = vku::InitStructHelper();
    queue_global_priority_ci[1].globalPriority = VK_QUEUE_GLOBAL_PRIORITY_MEDIUM;

    float priorities[] = {1.0f, 1.0f};
    VkDeviceQueueCreateInfo device_queue_ci[2] = {};
    device_queue_ci[0] = vku::InitStructHelper(&queue_global_priority_ci[0]);
    device_queue_ci[0].queueFamilyIndex = queue_family_index;
    device_queue_ci[0].queueCount = 1;
    device_queue_ci[0].pQueuePriorities = &priorities[0];

    device_queue_ci[1] = vku::InitStructHelper(&queue_global_priority_ci[1]);
    device_queue_ci[1].queueFamilyIndex = queue_family_index;
    device_queue_ci[1].queueCount = 1;
    device_queue_ci[1].pQueuePriorities = &priorities[1];

    VkDeviceCreateInfo device_ci = vku::InitStructHelper();
    device_ci.queueCreateInfoCount = 2;
    device_ci.pQueueCreateInfos = device_queue_ci;
    device_ci.enabledLayerCount = 0;
    device_ci.enabledExtensionCount = m_device_extension_names.size();
    device_ci.ppEnabledExtensionNames = m_device_extension_names.data();

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-queueFamilyIndex-02802");
    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pQueueCreateInfos-06654");
    VkDevice device;
    vk::CreateDevice(Gpu(), &device_ci, nullptr, &device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceQueue, QueueCount) {
    TEST_DESCRIPTION("Create device queue with too high of a queueCount.");

    RETURN_IF_SKIP(InitFramework());

    uint32_t queue_family_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_props(queue_family_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_family_count, queue_props.data());

    const uint32_t invalid_count = queue_props[0].queueCount + 1;
    std::vector<float> priorities(invalid_count);
    std::fill(priorities.begin(), priorities.end(), 0.0f);

    VkDeviceQueueCreateInfo device_queue_ci = vku::InitStructHelper();
    device_queue_ci.queueFamilyIndex = 0;
    device_queue_ci.queueCount = queue_props[0].queueCount + 1;
    device_queue_ci.pQueuePriorities = priorities.data();

    VkDeviceCreateInfo device_ci = vku::InitStructHelper();
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = &device_queue_ci;
    device_ci.enabledLayerCount = 0;
    device_ci.enabledExtensionCount = m_device_extension_names.size();
    device_ci.ppEnabledExtensionNames = m_device_extension_names.data();

    VkDevice device = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceQueueCreateInfo-queueCount-00382");
    vk::CreateDevice(Gpu(), &device_ci, nullptr, &device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceQueue, QueuePriorities) {
    TEST_DESCRIPTION("Create device queue with invalid QueuePriorities");

    RETURN_IF_SKIP(InitFramework());

    uint32_t queue_family_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_props(queue_family_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_family_count, queue_props.data());

    VkDeviceQueueCreateInfo device_queue_ci = vku::InitStructHelper();
    device_queue_ci.queueFamilyIndex = 0;
    device_queue_ci.queueCount = 1;

    VkDeviceCreateInfo device_ci = vku::InitStructHelper();
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = &device_queue_ci;
    device_ci.enabledLayerCount = 0;
    device_ci.enabledExtensionCount = m_device_extension_names.size();
    device_ci.ppEnabledExtensionNames = m_device_extension_names.data();

    VkDevice device = VK_NULL_HANDLE;

    const float priority_high = 2.0f;
    device_queue_ci.pQueuePriorities = &priority_high;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceQueueCreateInfo-pQueuePriorities-00383");
    vk::CreateDevice(Gpu(), &device_ci, nullptr, &device);
    m_errorMonitor->VerifyFound();

    const float priority_low = -1.0f;
    device_queue_ci.pQueuePriorities = &priority_low;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceQueueCreateInfo-pQueuePriorities-00383");
    vk::CreateDevice(Gpu(), &device_ci, nullptr, &device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceQueue, BindPipeline) {
    TEST_DESCRIPTION("vkCmdPipelineBarrier with command pool with wrong queues");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    uint32_t only_transfer_queueFamilyIndex = vvl::kU32Max;
    const auto q_props = m_device->Physical().queue_properties_;
    for (uint32_t i = 0; i < (uint32_t)q_props.size(); i++) {
        if (q_props[i].queueFlags == VK_QUEUE_TRANSFER_BIT) {
            only_transfer_queueFamilyIndex = i;
            break;
        }
    }
    if (only_transfer_queueFamilyIndex == vvl::kU32Max) {
        GTEST_SKIP() << "Only VK_QUEUE_TRANSFER_BIT Queue is not supported";
    }
    vkt::CommandPool commandPool(*m_device, only_transfer_queueFamilyIndex);
    vkt::CommandBuffer commandBuffer(*m_device, commandPool);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.CreateGraphicsPipeline();
    CreateComputePipelineHelper c_pipe(*this);
    c_pipe.CreateComputePipeline();

    // Get implicit VU because using Transfer only instead of a Graphics-only or Compute-only queue
    commandBuffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindPipeline-commandBuffer-cmdpool");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindPipeline-pipelineBindPoint-00777");
    vk::CmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, c_pipe.Handle());
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindPipeline-commandBuffer-cmdpool");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindPipeline-pipelineBindPoint-00778");
    vk::CmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    m_errorMonitor->VerifyFound();
    commandBuffer.End();
}

TEST_F(NegativeDeviceQueue, CreateCommandPool) {
    TEST_DESCRIPTION("vkCreateCommandPool with bad queue");
    RETURN_IF_SKIP(Init());
    const size_t queue_count = m_device->Physical().queue_properties_.size();
    m_errorMonitor->SetDesiredError("VUID-vkCreateCommandPool-queueFamilyIndex-01937");
    vkt::CommandPool commandPool(*m_device, queue_count + 1);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceQueue, QueuesSameQueueFamily) {
    TEST_DESCRIPTION("Request more queues than available from queue family");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework());
    VkPhysicalDeviceProtectedMemoryFeatures protected_memory_features = vku::InitStructHelper();
    if (!protected_memory_features.protectedMemory) {
        GTEST_SKIP() << "protectedMemory not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &protected_memory_features));

    uint32_t qf_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &qf_count, nullptr);
    std::vector<VkQueueFamilyProperties> qf_props(qf_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &qf_count, qf_props.data());

    uint32_t index = 0;
    for (uint32_t i = 0; i < qf_count; ++i) {
        if (qf_props[i].queueFlags & VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT) {
            index = i;
            break;
        }
    }

    std::vector<float> priorities(qf_props[index].queueCount, 1.0f);
    VkDeviceQueueCreateInfo device_queue_ci[2];
    device_queue_ci[0] = vku::InitStructHelper();
    device_queue_ci[0].queueFamilyIndex = index;
    device_queue_ci[0].queueCount = qf_props[index].queueCount;
    device_queue_ci[0].pQueuePriorities = priorities.data();
    device_queue_ci[1] = vku::InitStructHelper();
    device_queue_ci[1].flags = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT;
    device_queue_ci[1].queueFamilyIndex = index;
    device_queue_ci[1].queueCount = 1u;
    device_queue_ci[1].pQueuePriorities = priorities.data();

    VkDeviceCreateInfo device_ci = vku::InitStructHelper();
    device_ci.queueCreateInfoCount = 2u;
    device_ci.pQueueCreateInfos = device_queue_ci;

    VkDevice device;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pQueueCreateInfos-06755");
    vk::CreateDevice(Gpu(), &device_ci, nullptr, &device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceQueue, MismatchedQueueFamiliesOnSubmit) {
    TEST_DESCRIPTION(
        "Submit command buffer created using one queue family and attempt to submit them on a queue created in a different queue "
        "family.");

    RETURN_IF_SKIP(Init());  // assumes it initializes all queue families on vk::CreateDevice

    // This test is meaningless unless we have multiple queue families
    auto queue_family_properties = m_device->Physical().queue_properties_;
    std::vector<uint32_t> queue_families;
    for (uint32_t i = 0; i < queue_family_properties.size(); ++i)
        if (queue_family_properties[i].queueCount > 0) queue_families.push_back(i);

    if (queue_families.size() < 2) {
        GTEST_SKIP() << "Device only has one queue family";
    }

    const uint32_t queue_family = queue_families[0];

    const uint32_t other_queue_family = queue_families[1];
    VkQueue other_queue;
    vk::GetDeviceQueue(device(), other_queue_family, 0, &other_queue);

    vkt::CommandPool cmd_pool(*m_device, queue_family);
    vkt::CommandBuffer cmd_buff(*m_device, cmd_pool);

    cmd_buff.Begin();
    cmd_buff.End();

    // Submit on the wrong queue
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd_buff.handle();

    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00074");
    vk::QueueSubmit(other_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceQueue, DeviceCreateInvalidParameters) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework());

    float priority = 1.0f;
    VkDeviceQueueCreateInfo device_queue_ci = vku::InitStructHelper();
    device_queue_ci.queueFamilyIndex = 0u;
    device_queue_ci.queueCount = 1;
    device_queue_ci.pQueuePriorities = &priority;

    VkPhysicalDeviceVulkan11Features features11 = vku::InitStructHelper();
    VkPhysicalDeviceVulkan11Features features11_duplicated = vku::InitStructHelper(&features11);

    VkDeviceCreateInfo device_ci = vku::InitStructHelper(&features11_duplicated);
    device_ci.queueCreateInfoCount = 1u;
    device_ci.pQueueCreateInfos = &device_queue_ci;
    device_ci.enabledLayerCount = 0u;
    device_ci.enabledExtensionCount = m_device_extension_names.size();
    device_ci.ppEnabledExtensionNames = m_device_extension_names.data();

    VkDevice device;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-sType-unique");
    vk::CreateDevice(Gpu(), &device_ci, nullptr, &device);
    m_errorMonitor->VerifyFound();

    device_ci.pNext = nullptr;
    device_ci.flags = 1u;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-flags-zerobitmask");
    vk::CreateDevice(Gpu(), &device_ci, nullptr, &device);
    m_errorMonitor->VerifyFound();

    device_ci.flags = 0u;
    device_ci.pQueueCreateInfos = nullptr;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pQueueCreateInfos-parameter");
    vk::CreateDevice(Gpu(), &device_ci, nullptr, &device);
    m_errorMonitor->VerifyFound();

    device_ci.pQueueCreateInfos = &device_queue_ci;
    device_ci.queueCreateInfoCount = 0u;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-queueCreateInfoCount-arraylength");
    vk::CreateDevice(Gpu(), &device_ci, nullptr, &device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceQueue, DeviceCreateEnabledLayerNamesPointer) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework());

    float priority = 1.0f;
    VkDeviceQueueCreateInfo device_queue_ci = vku::InitStructHelper();
    device_queue_ci.queueFamilyIndex = 0u;
    device_queue_ci.queueCount = 1;
    device_queue_ci.pQueuePriorities = &priority;

    VkDevice device;
    VkDeviceCreateInfo device_ci = vku::InitStructHelper();
    device_ci.queueCreateInfoCount = 1u;
    device_ci.pQueueCreateInfos = &device_queue_ci;
    device_ci.enabledExtensionCount = m_device_extension_names.size();
    device_ci.ppEnabledExtensionNames = m_device_extension_names.data();
    device_ci.enabledLayerCount = 1u;
    device_ci.ppEnabledLayerNames = nullptr;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-ppEnabledLayerNames-parameter");
    vk::CreateDevice(Gpu(), &device_ci, nullptr, &device);
    m_errorMonitor->VerifyFound();
}

// Test works, but needs Loader fix in 369afe24d1351d6e03cbfc3daf1fc5f6cd103649
// Enable once enough CI machines have loader patch
TEST_F(NegativeDeviceQueue, DISABLED_DeviceCreateEnabledExtensionNamesPointer) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework());

    float priority = 1.0f;
    VkDeviceQueueCreateInfo device_queue_ci = vku::InitStructHelper();
    device_queue_ci.queueFamilyIndex = 0u;
    device_queue_ci.queueCount = 1;
    device_queue_ci.pQueuePriorities = &priority;

    VkDevice device;
    VkDeviceCreateInfo device_ci = vku::InitStructHelper();
    device_ci.queueCreateInfoCount = 1u;
    device_ci.pQueueCreateInfos = &device_queue_ci;
    device_ci.enabledLayerCount = 0u;
    device_ci.ppEnabledLayerNames = nullptr;
    device_ci.enabledExtensionCount = 1u;
    device_ci.ppEnabledExtensionNames = nullptr;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-parameter");
    vk::CreateDevice(Gpu(), &device_ci, nullptr, &device);
    m_errorMonitor->VerifyFound();
}