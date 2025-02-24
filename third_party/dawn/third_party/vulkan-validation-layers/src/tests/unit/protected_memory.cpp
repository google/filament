/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/render_pass_helper.h"

class NegativeProtectedMemory : public VkLayerTest {};

TEST_F(NegativeProtectedMemory, Queue) {
    TEST_DESCRIPTION("Try creating queue without VK_QUEUE_PROTECTED_BIT capability");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDeviceProtectedMemoryFeatures protected_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(protected_features);

    if (protected_features.protectedMemory == VK_FALSE) {
        GTEST_SKIP() << "test requires protectedMemory";
    }

    // Try to find a protected queue family type
    bool unprotected_queue = false;
    uint32_t queue_family_index = 0;
    uint32_t queue_family_count = 0;
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_family_count, queue_families.data());

    // need to find a queue without protected support
    for (size_t i = 0; i < queue_families.size(); i++) {
        if ((queue_families[i].queueFlags & VK_QUEUE_PROTECTED_BIT) == 0) {
            unprotected_queue = true;
            queue_family_index = i;
            break;
        }
    }

    if (unprotected_queue == false) {
        GTEST_SKIP() << "test requires queue without VK_QUEUE_PROTECTED_BIT";
    }

    float queue_priority = 1.0;
    VkDeviceQueueCreateInfo queue_create_info = vku::InitStructHelper();
    queue_create_info.flags = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT;
    queue_create_info.queueFamilyIndex = queue_family_index;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;

    VkDevice test_device = VK_NULL_HANDLE;
    VkDeviceCreateInfo device_create_info = vku::InitStructHelper(&protected_features);
    device_create_info.flags = 0;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pEnabledFeatures = nullptr;
    device_create_info.enabledLayerCount = 0;
    device_create_info.enabledExtensionCount = 0;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceQueueCreateInfo-flags-06449");
    vk::CreateDevice(Gpu(), &device_create_info, nullptr, &test_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeProtectedMemory, Submit) {
    TEST_DESCRIPTION("Setting protectedSubmit with a queue not created with VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    // creates a queue without VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT
    RETURN_IF_SKIP(Init());

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_PROTECTED_BIT;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    m_errorMonitor->SetDesiredError("VUID-VkCommandPoolCreateInfo-flags-02860");
    vk::CreateCommandPool(device(), &pool_create_info, nullptr, &command_pool);
    m_errorMonitor->VerifyFound();

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.flags = VK_BUFFER_CREATE_PROTECTED_BIT;
    buffer_create_info.size = 4096;
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-flags-01887");

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = VK_IMAGE_CREATE_PROTECTED_BIT;
    image_create_info.extent = {64, 64, 1};
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.arrayLayers = 1;
    image_create_info.mipLevels = 1;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-01890");

    // Try to find memory with protected bit in it at all
    VkDeviceMemory memory_protected = VK_NULL_HANDLE;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = 4096;

    VkPhysicalDeviceMemoryProperties phys_mem_props;
    vk::GetPhysicalDeviceMemoryProperties(Gpu(), &phys_mem_props);
    alloc_info.memoryTypeIndex = phys_mem_props.memoryTypeCount + 1;
    for (uint32_t i = 0; i < phys_mem_props.memoryTypeCount; i++) {
        // Check just protected bit is in type at all
        if ((phys_mem_props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) != 0) {
            alloc_info.memoryTypeIndex = i;
            break;
        }
    }
    if (alloc_info.memoryTypeIndex < phys_mem_props.memoryTypeCount) {
        m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-memoryTypeIndex-01872");
        vk::AllocateMemory(device(), &alloc_info, NULL, &memory_protected);
        m_errorMonitor->VerifyFound();
    }

    VkProtectedSubmitInfo protected_submit_info = vku::InitStructHelper();
    protected_submit_info.protectedSubmit = VK_TRUE;

    VkSubmitInfo submit_info = vku::InitStructHelper(&protected_submit_info);
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffer.handle();

    m_command_buffer.Begin();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-queue-06448");
    m_errorMonitor->SetUnexpectedError("VUID-VkSubmitInfo-pNext-04148");
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeProtectedMemory, Memory) {
    TEST_DESCRIPTION("Validate cases where protectedMemory feature is enabled and usages are invalid");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::sparseBinding);
    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(InitFramework());
    RETURN_IF_SKIP(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_PROTECTED_BIT));

    bool sparse_support = (m_device->Physical().Features().sparseBinding == VK_TRUE);

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.flags = VK_BUFFER_CREATE_PROTECTED_BIT | VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    buffer_create_info.size = 1 << 20;  // 1 MB
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    if (sparse_support == true) {
        CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-None-01888");
    }

    // Create actual protected and unprotected buffers
    buffer_create_info.flags = VK_BUFFER_CREATE_PROTECTED_BIT;
    vkt::Buffer buffer_protected(*m_device, buffer_create_info, vkt::no_mem);

    buffer_create_info.flags = 0;
    vkt::Buffer buffer_unprotected(*m_device, buffer_create_info, vkt::no_mem);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = VK_IMAGE_CREATE_PROTECTED_BIT | VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
    image_create_info.extent = {8, 8, 1};
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.arrayLayers = 1;
    image_create_info.mipLevels = 1;

    if (sparse_support == true) {
        CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-None-01891");
    }

    // Create actual protected and unprotected images
    image_create_info.flags = VK_IMAGE_CREATE_PROTECTED_BIT;
    vkt::Image image_protected(*m_device, image_create_info, vkt::no_mem);
    image_create_info.flags = 0;
    vkt::Image image_unprotected(*m_device, image_create_info, vkt::no_mem);

    // Create protected and unproteced memory
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = 0;

    // set allocationSize to buffer as it will be larger than the image, but query image to avoid BP warning
    VkMemoryRequirements mem_reqs_protected;
    vk::GetImageMemoryRequirements(device(), image_protected.handle(), &mem_reqs_protected);
    vk::GetBufferMemoryRequirements(device(), buffer_protected.handle(), &mem_reqs_protected);
    VkMemoryRequirements mem_reqs_unprotected;
    vk::GetImageMemoryRequirements(device(), image_unprotected.handle(), &mem_reqs_unprotected);
    vk::GetBufferMemoryRequirements(device(), buffer_unprotected.handle(), &mem_reqs_unprotected);

    alloc_info.allocationSize = mem_reqs_protected.size;
    bool found =
        m_device->Physical().SetMemoryType(mem_reqs_unprotected.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_PROTECTED_BIT);
    if (!found) {
        GTEST_SKIP() << "Memory type not found";
    }
    vkt::DeviceMemory memory_protected(*m_device, alloc_info);

    alloc_info.allocationSize = mem_reqs_unprotected.size;
    found = m_device->Physical().SetMemoryType(mem_reqs_unprotected.memoryTypeBits, &alloc_info,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_PROTECTED_BIT);
    if (!found) {
        GTEST_SKIP() << "Memory type not found";
    }
    vkt::DeviceMemory memory_unprotected(*m_device, alloc_info);

    // Bind protected buffer with unprotected memory
    m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-None-01898");
    m_errorMonitor->SetUnexpectedError("VUID-vkBindBufferMemory-memory-01035");
    vk::BindBufferMemory(device(), buffer_protected.handle(), memory_unprotected.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Bind unprotected buffer with protected memory
    m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-None-01899");
    m_errorMonitor->SetUnexpectedError("VUID-vkBindBufferMemory-memory-01035");
    vk::BindBufferMemory(device(), buffer_unprotected.handle(), memory_protected.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Bind protected image with unprotected memory
    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-None-01901");
    m_errorMonitor->SetUnexpectedError("VUID-vkBindImageMemory-memory-01047");
    vk::BindImageMemory(device(), image_protected.handle(), memory_unprotected.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Bind unprotected image with protected memory
    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-None-01902");
    m_errorMonitor->SetUnexpectedError("VUID-vkBindImageMemory-memory-01047");
    vk::BindImageMemory(device(), image_unprotected.handle(), memory_protected.handle(), 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeProtectedMemory, UniqueQueueDeviceCreationBothProtected) {
    TEST_DESCRIPTION("Vulkan 1.1 unique queue detection where both are protected and same queue family");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    // Needed for both protected memory and vkGetDeviceQueue2
    VkPhysicalDeviceProtectedMemoryFeatures protected_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(protected_features);

    if (protected_features.protectedMemory == VK_FALSE) {
        GTEST_SKIP() << "test requires protectedMemory";
    }

    // Try to find a protected queue family type
    bool protected_queue = false;
    VkQueueFamilyProperties queue_properties;  // selected queue family used
    uint32_t queue_family_index = 0;
    uint32_t queue_family_count = 0;
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_family_count, queue_families.data());

    for (size_t i = 0; i < queue_families.size(); i++) {
        // need to have at least 2 queues to use
        if (((queue_families[i].queueFlags & VK_QUEUE_PROTECTED_BIT) != 0) && (queue_families[i].queueCount > 1)) {
            protected_queue = true;
            queue_family_index = i;
            queue_properties = queue_families[i];
            break;
        }
    }

    if (protected_queue == false) {
        GTEST_SKIP() << "test requires queue with VK_QUEUE_PROTECTED_BIT with 2 queues, not available";
    }

    float queue_priority = 1.0;

    VkDeviceQueueCreateInfo queue_create_info[2];
    queue_create_info[0] = vku::InitStructHelper();
    queue_create_info[0].flags = 0;
    queue_create_info[0].queueFamilyIndex = queue_family_index;
    queue_create_info[0].queueCount = 1;
    queue_create_info[0].pQueuePriorities = &queue_priority;

    // queueFamilyIndex is the same and both are empty flags
    queue_create_info[1] = queue_create_info[0];

    VkDevice test_device = VK_NULL_HANDLE;
    VkDeviceCreateInfo device_create_info = vku::InitStructHelper(&protected_features);
    device_create_info.flags = 0;
    device_create_info.pQueueCreateInfos = queue_create_info;
    device_create_info.queueCreateInfoCount = 2;
    device_create_info.pEnabledFeatures = nullptr;
    device_create_info.enabledLayerCount = 0;
    device_create_info.enabledExtensionCount = 0;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-queueFamilyIndex-02802");
    vk::CreateDevice(Gpu(), &device_create_info, nullptr, &test_device);
    m_errorMonitor->VerifyFound();

    // both protected
    queue_create_info[0].flags = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT;
    queue_create_info[1].flags = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-queueFamilyIndex-02802");
    vk::CreateDevice(Gpu(), &device_create_info, nullptr, &test_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeProtectedMemory, GetDeviceQueue) {
    TEST_DESCRIPTION("General testing of vkGetDeviceQueue and general Device creation cases");
    all_queue_count_ = true;
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkDeviceQueueInfo2 queue_info_2 = vku::InitStructHelper();
    VkDevice test_device = VK_NULL_HANDLE;
    VkQueue test_queue = VK_NULL_HANDLE;
    VkResult result;

    // Use the first Physical device and queue family
    // Makes test more portable as every driver has at least 1 queue with a queueCount of 1
    uint32_t queue_family_count = 1;
    uint32_t queue_family_index = 0;
    VkQueueFamilyProperties queue_properties;
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_family_count, &queue_properties);

    float queue_priority = 1.0;
    VkDeviceQueueCreateInfo queue_create_info = vku::InitStructHelper();
    queue_create_info.flags = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT;
    queue_create_info.queueFamilyIndex = queue_family_index;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;

    VkPhysicalDeviceProtectedMemoryFeatures protect_features = vku::InitStructHelper();
    protect_features.protectedMemory = VK_FALSE;  // Starting with it off

    VkDeviceCreateInfo device_create_info = vku::InitStructHelper(&protect_features);
    device_create_info.flags = 0;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pEnabledFeatures = nullptr;
    device_create_info.enabledLayerCount = 0;
    device_create_info.enabledExtensionCount = 0;

    // Protect feature not set
    m_errorMonitor->SetUnexpectedError("VUID-VkDeviceQueueCreateInfo-flags-06449");
    m_errorMonitor->SetDesiredError("VUID-VkDeviceQueueCreateInfo-flags-02861");
    vk::CreateDevice(Gpu(), &device_create_info, nullptr, &test_device);
    m_errorMonitor->VerifyFound();

    GetPhysicalDeviceFeatures2(protect_features);

    if (protect_features.protectedMemory == VK_TRUE) {
        // Might not have protected queue support
        m_errorMonitor->SetUnexpectedError("VUID-VkDeviceQueueCreateInfo-flags-06449");
        result = vk::CreateDevice(Gpu(), &device_create_info, nullptr, &test_device);
        if (result != VK_SUCCESS) {
            GTEST_SKIP() << "CreateDevice returned back not VK_SUCCESS";
        }

        // Try using GetDeviceQueue with a queue that has as flag
        m_errorMonitor->SetDesiredError("VUID-vkGetDeviceQueue-flags-01841");
        vk::GetDeviceQueue(test_device, queue_family_index, 0, &test_queue);
        m_errorMonitor->VerifyFound();

        // Test device created with flag and trying to query with no flag
        queue_info_2.flags = 0;
        queue_info_2.queueFamilyIndex = queue_family_index;
        queue_info_2.queueIndex = 0;
        m_errorMonitor->SetDesiredError("VUID-VkDeviceQueueInfo2-flags-06225");
        vk::GetDeviceQueue2(test_device, &queue_info_2, &test_queue);
        m_errorMonitor->VerifyFound();

        vk::DestroyDevice(test_device, nullptr);
        test_device = VK_NULL_HANDLE;
    }

    // Create device without protected queue
    protect_features.protectedMemory = VK_FALSE;
    queue_create_info.flags = 0;
    result = vk::CreateDevice(Gpu(), &device_create_info, nullptr, &test_device);
    if (result != VK_SUCCESS) {
        GTEST_SKIP() << "CreateDevice returned back not VK_SUCCESS";
    }

    if (queue_properties.queueCount > 1) {
        // Set queueIndex 1 over size of queueCount used to create device
        m_errorMonitor->SetDesiredError("VUID-vkGetDeviceQueue-queueIndex-00385");
        vk::GetDeviceQueue(test_device, queue_family_index, 1, &test_queue);
        m_errorMonitor->VerifyFound();
    }

    // Use an unknown queue family index
    m_errorMonitor->SetDesiredError("VUID-vkGetDeviceQueue-queueFamilyIndex-00384");
    vk::GetDeviceQueue(test_device, queue_family_index + 1, 0, &test_queue);
    m_errorMonitor->VerifyFound();

    queue_info_2.flags = 0;  // same as device creation
    queue_info_2.queueFamilyIndex = queue_family_index;
    queue_info_2.queueIndex = 0;

    if (queue_properties.queueCount > 1) {
        // Set queueIndex 1 over size of queueCount used to create device
        queue_info_2.queueIndex = 1;
        m_errorMonitor->SetDesiredError("VUID-VkDeviceQueueInfo2-queueIndex-01843");
        vk::GetDeviceQueue2(test_device, &queue_info_2, &test_queue);
        m_errorMonitor->VerifyFound();
        queue_info_2.queueIndex = 0;  // reset
    }

    // Use an unknown queue family index
    queue_info_2.queueFamilyIndex = queue_family_index + 1;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceQueueInfo2-queueFamilyIndex-01842");
    vk::GetDeviceQueue2(test_device, &queue_info_2, &test_queue);
    m_errorMonitor->VerifyFound();
    queue_info_2.queueFamilyIndex = queue_family_index;  // reset

    // Test device created with no flags and trying to query with flag
    queue_info_2.flags = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceQueueInfo2-flags-06225");
    vk::GetDeviceQueue2(test_device, &queue_info_2, &test_queue);
    m_errorMonitor->VerifyFound();
    queue_info_2.flags = 0;  // reset

    // Sanity check can still get the queue
    vk::GetDeviceQueue(test_device, queue_family_index, 0, &test_queue);

    vk::DestroyDevice(test_device, nullptr);
}

TEST_F(NegativeProtectedMemory, PipelineProtectedAccess) {
    TEST_DESCRIPTION("Test VUIDs from VK_EXT_pipeline_protected_access");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_PIPELINE_PROTECTED_ACCESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::protectedMemory);
    AddRequiredFeature(vkt::Feature::pipelineProtectedAccess);
    RETURN_IF_SKIP(InitFramework());
    RETURN_IF_SKIP(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_PROTECTED_BIT));
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.VertexShaderOnly();
    pipe.gp_ci_.flags = VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT | VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-flags-07369");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    pipe.gp_ci_.flags = VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindPipeline-pipelineProtectedAccess-07408");
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_errorMonitor->VerifyFound();

    CreatePipelineHelper protected_pipe(*this);
    protected_pipe.shader_stages_ = {protected_pipe.vs_->GetStageCreateInfo()};
    protected_pipe.gp_ci_.flags = VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT;
    protected_pipe.CreateGraphicsPipeline();

    vkt::CommandPool command_pool(*m_device, m_device->graphics_queue_node_index_);
    vkt::CommandBuffer unprotected_cmdbuf(*m_device, command_pool);
    unprotected_cmdbuf.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindPipeline-pipelineProtectedAccess-07409");
    vk::CmdBindPipeline(unprotected_cmdbuf.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, protected_pipe.Handle());
    m_errorMonitor->VerifyFound();

    // Create device without protected access features
    vkt::Device test_device(Gpu(), m_device_extension_names);
    VkShaderObj vs2(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    vs2.InitFromGLSLTry(&test_device);

    const vkt::PipelineLayout test_pipeline_layout(test_device);

    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pColorAttachments = &attach;
    subpass.colorAttachmentCount = 1;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    vkt::RenderPass render_pass(test_device, rpci);

    m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-pipelineProtectedAccess-07368");
    CreatePipelineHelper featureless_pipe(*this);
    featureless_pipe.device_ = &test_device;
    featureless_pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    featureless_pipe.shader_stages_ = {vs2.GetStageCreateInfo()};
    featureless_pipe.gp_ci_.flags = VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT;
    featureless_pipe.gp_ci_.layout = test_pipeline_layout.handle();
    featureless_pipe.gp_ci_.renderPass = render_pass.handle();
    featureless_pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeProtectedMemory, PipelineProtectedAccessGPL) {
    TEST_DESCRIPTION("Test VUIDs from VK_EXT_pipeline_protected_access");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_PIPELINE_PROTECTED_ACCESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    AddRequiredFeature(vkt::Feature::protectedMemory);
    AddRequiredFeature(vkt::Feature::pipelineProtectedAccess);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pre_raster_lib(*this);
    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    VkShaderModuleCreateInfo vs_ci = vku::InitStructHelper();
    vs_ci.codeSize = vs_spv.size() * sizeof(decltype(vs_spv)::value_type);
    vs_ci.pCode = vs_spv.data();

    VkPipelineShaderStageCreateInfo stage_ci = vku::InitStructHelper(&vs_ci);
    stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    stage_ci.module = VK_NULL_HANDLE;
    stage_ci.pName = "main";

    pre_raster_lib.InitPreRasterLibInfo(&stage_ci);
    pre_raster_lib.pipeline_layout_ci_.flags |= VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT;
    ASSERT_EQ(VK_SUCCESS, pre_raster_lib.CreateGraphicsPipeline());

    VkPipeline libraries[1] = {
        pre_raster_lib.Handle(),
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size32(libraries);
    link_info.pLibraries = libraries;

    m_errorMonitor->SetDesiredError("VUID-VkPipelineLibraryCreateInfoKHR-pipeline-07404");
    VkGraphicsPipelineCreateInfo lib_ci = vku::InitStructHelper(&link_info);
    lib_ci.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR | VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT;
    lib_ci.renderPass = RenderPass();
    lib_ci.layout = pre_raster_lib.gp_ci_.layout;
    vkt::Pipeline lib(*m_device, lib_ci);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkPipelineLibraryCreateInfoKHR-pipeline-07406");
    lib_ci.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR | VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT;
    vkt::Pipeline lib2(*m_device, lib_ci);
    m_errorMonitor->VerifyFound();

    CreatePipelineHelper protected_pre_raster_lib(*this);
    protected_pre_raster_lib.InitPreRasterLibInfo(&stage_ci);
    protected_pre_raster_lib.pipeline_layout_ci_.flags |= VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT;
    protected_pre_raster_lib.gp_ci_.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR | VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT;
    ASSERT_EQ(VK_SUCCESS, protected_pre_raster_lib.CreateGraphicsPipeline());
    libraries[0] = protected_pre_raster_lib.Handle();
    VkGraphicsPipelineCreateInfo protected_lib_ci = vku::InitStructHelper(&link_info);
    protected_lib_ci.renderPass = RenderPass();
    protected_lib_ci.layout = pre_raster_lib.gp_ci_.layout;
    m_errorMonitor->SetDesiredError("VUID-VkPipelineLibraryCreateInfoKHR-pipeline-07407");
    lib_ci.flags = 0;
    protected_lib_ci.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    vkt::Pipeline lib3(*m_device, protected_lib_ci);
    m_errorMonitor->VerifyFound();

    CreatePipelineHelper unprotected_pre_raster_lib(*this);
    unprotected_pre_raster_lib.InitPreRasterLibInfo(&stage_ci);
    unprotected_pre_raster_lib.pipeline_layout_ci_.flags |= VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT;
    unprotected_pre_raster_lib.gp_ci_.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR | VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT;
    ASSERT_EQ(VK_SUCCESS, unprotected_pre_raster_lib.CreateGraphicsPipeline());
    libraries[0] = unprotected_pre_raster_lib.Handle();
    VkGraphicsPipelineCreateInfo unprotected_lib_ci = vku::InitStructHelper(&link_info);
    unprotected_lib_ci.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    unprotected_lib_ci.renderPass = RenderPass();
    unprotected_lib_ci.layout = pre_raster_lib.gp_ci_.layout;
    m_errorMonitor->SetDesiredError("VUID-VkPipelineLibraryCreateInfoKHR-pipeline-07405");
    vkt::Pipeline lib4(*m_device, unprotected_lib_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeProtectedMemory, UnprotectedCommands) {
    TEST_DESCRIPTION("Test making commands in unprotected command buffers that can't be used");

    // protect memory added in VK 1.1
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(InitFramework());
    // Turns m_command_buffer into a protected command buffer
    RETURN_IF_SKIP(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_PROTECTED_BIT));

    InitRenderTarget();

    vkt::Buffer indirect_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer indexed_indirect_buffer(*m_device, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer index_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirect-commandBuffer-02711");
    vk::CmdDrawIndirect(m_command_buffer.handle(), indirect_buffer.handle(), 0, 1, sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirect-commandBuffer-02711");
    vk::CmdDrawIndexedIndirect(m_command_buffer.handle(), indexed_indirect_buffer.handle(), 0, 1,
                               sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();

    // Query should be outside renderpass
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 1);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-commandBuffer-01885");
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEndQuery-commandBuffer-01886");
    m_errorMonitor->SetUnexpectedError("VUID-vkCmdEndQuery-None-01923");
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeProtectedMemory, MixingProtectedResources) {
    TEST_DESCRIPTION("Test where there is mixing of protectedMemory backed resource in command buffers");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::fragmentStoresAndAtomics);
    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceProtectedMemoryProperties protected_memory_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(protected_memory_properties);

    vkt::CommandPool protectedCommandPool(*m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_PROTECTED_BIT);
    vkt::CommandBuffer protectedCommandBuffer(*m_device, protectedCommandPool);

    // Create actual protected and unprotected buffers
    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = 1 << 20;  // 1 MB
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    buffer_create_info.flags = VK_BUFFER_CREATE_PROTECTED_BIT;
    vkt::Buffer buffer_protected(*m_device, buffer_create_info, vkt::no_mem);

    buffer_create_info.flags = 0;
    vkt::Buffer buffer_unprotected(*m_device, buffer_create_info, vkt::no_mem);

    // Create actual protected and unprotected images
    const VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageView image_views[2];
    VkImageView image_views_descriptor[2];
    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.extent = {64, 64, 1};
    image_create_info.format = image_format;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.arrayLayers = 1;
    image_create_info.mipLevels = 1;
    image_create_info.flags = VK_IMAGE_CREATE_PROTECTED_BIT;

    vkt::Image image_protected(*m_device, image_create_info, vkt::no_mem);
    vkt::Image image_protected_descriptor(*m_device, image_create_info, vkt::no_mem);
    image_create_info.flags = 0;
    vkt::Image image_unprotected(*m_device, image_create_info, vkt::no_mem);
    vkt::Image image_unprotected_descriptor(*m_device, image_create_info, vkt::no_mem);

    // Create protected and unproteced memory
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = 0;

    VkMemoryRequirements mem_reqs_buffer_protected;
    VkMemoryRequirements mem_reqs_buffer_unprotected;
    vk::GetBufferMemoryRequirements(device(), buffer_protected.handle(), &mem_reqs_buffer_protected);
    vk::GetBufferMemoryRequirements(device(), buffer_unprotected.handle(), &mem_reqs_buffer_unprotected);

    VkMemoryRequirements mem_reqs_image_protected;
    VkMemoryRequirements mem_reqs_image_unprotected;
    vk::GetImageMemoryRequirements(device(), image_protected.handle(), &mem_reqs_image_protected);
    vk::GetImageMemoryRequirements(device(), image_unprotected.handle(), &mem_reqs_image_unprotected);

    bool found =
        m_device->Physical().SetMemoryType(mem_reqs_buffer_protected.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_PROTECTED_BIT);
    if (!found) {
        GTEST_SKIP() << "Memory type not found";
    }
    alloc_info.allocationSize = mem_reqs_buffer_protected.size;
    vkt::DeviceMemory memory_buffer_protected(*m_device, alloc_info);

    found = m_device->Physical().SetMemoryType(mem_reqs_buffer_unprotected.memoryTypeBits, &alloc_info,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_PROTECTED_BIT);
    if (!found) {
        GTEST_SKIP() << "Memory type not found";
    }
    alloc_info.allocationSize = mem_reqs_buffer_unprotected.size;
    vkt::DeviceMemory memory_buffer_unprotected(*m_device, alloc_info);

    alloc_info.allocationSize = mem_reqs_image_protected.size;
    found =
        m_device->Physical().SetMemoryType(mem_reqs_image_protected.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_PROTECTED_BIT);
    if (!found) {
        GTEST_SKIP() << "Memory type not found";
    }
    vkt::DeviceMemory memory_image_protected(*m_device, alloc_info);

    found = m_device->Physical().SetMemoryType(mem_reqs_image_unprotected.memoryTypeBits, &alloc_info,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_PROTECTED_BIT);
    if (!found) {
        GTEST_SKIP() << "Memory type not found";
    }
    alloc_info.allocationSize = mem_reqs_image_unprotected.size;
    vkt::DeviceMemory memory_image_unprotected(*m_device, alloc_info);

    vk::BindBufferMemory(device(), buffer_protected.handle(), memory_buffer_protected.handle(), 0);
    vk::BindBufferMemory(device(), buffer_unprotected.handle(), memory_buffer_unprotected.handle(), 0);
    vk::BindImageMemory(device(), image_protected.handle(), memory_image_protected.handle(), 0);
    vk::BindImageMemory(device(), image_unprotected.handle(), memory_image_unprotected.handle(), 0);
    vk::BindImageMemory(device(), image_protected_descriptor.handle(), memory_image_protected.handle(), 0);
    vk::BindImageMemory(device(), image_unprotected_descriptor.handle(), memory_image_unprotected.handle(), 0);

    // Change layout once memory is bound
    image_protected.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_protected_descriptor.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_unprotected.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_unprotected_descriptor.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    // need memory bound at image view creation time
    vkt::ImageView view0 = image_protected.CreateView();
    vkt::ImageView view1 = image_unprotected.CreateView();
    image_views[0] = view0;
    image_views[1] = view1;
    vkt::ImageView view_descriptor0 = image_protected_descriptor.CreateView();
    vkt::ImageView view_descriptor1 = image_unprotected_descriptor.CreateView();
    image_views_descriptor[0] = view_descriptor0;
    image_views_descriptor[1] = view_descriptor1;

    // A renderpass and framebuffer that contains a protected and unprotected image view
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(image_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rp.AddAttachmentDescription(image_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddAttachmentReference({1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddColorAttachment(0);
    rp.AddColorAttachment(1);
    rp.AddSubpassDependency(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                            VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_WRITE_BIT);
    rp.CreateRenderPass();

    vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 2, image_views, 8, 8);

    // Various structs used for commands
    VkImageSubresourceLayers image_subresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageBlit blit_region = {};
    blit_region.srcSubresource = image_subresource;
    blit_region.dstSubresource = image_subresource;
    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.srcOffsets[1] = {8, 8, 1};
    blit_region.dstOffsets[0] = {0, 8, 0};
    blit_region.dstOffsets[1] = {8, 8, 1};
    VkClearColorValue clear_color = {{0, 0, 0, 0}};
    VkImageSubresourceRange subresource_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkBufferCopy buffer_copy = {0, 0, 64};
    VkBufferImageCopy buffer_image_copy = {};
    buffer_image_copy.bufferRowLength = 0;
    buffer_image_copy.bufferImageHeight = 0;
    buffer_image_copy.imageSubresource = image_subresource;
    buffer_image_copy.imageOffset = {0, 0, 0};
    buffer_image_copy.imageExtent = {1, 1, 1};
    buffer_image_copy.bufferOffset = 0;
    VkImageCopy image_copy = {};
    image_copy.srcSubresource = image_subresource;
    image_copy.srcOffset = {0, 0, 0};
    image_copy.dstSubresource = image_subresource;
    image_copy.dstOffset = {0, 0, 0};
    image_copy.extent = {1, 1, 1};
    uint32_t update_data[4] = {0, 0, 0, 0};
    VkRect2D render_area = {{0, 0}, {8, 8}};
    VkClearAttachment clear_attachments[2] = {{VK_IMAGE_ASPECT_COLOR_BIT, 0, {m_clear_color}},
                                              {VK_IMAGE_ASPECT_COLOR_BIT, 1, {m_clear_color}}};
    VkClearRect clear_rect[2] = {{render_area, 0, 1}, {render_area, 0, 1}};

    const char fsSource[] = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform foo { int x; int y; } bar;
        layout(set=0, binding=1, rgba8) uniform image2D si1;
        layout(location=0) out vec4 x;
        void main(){
           x = vec4(bar.y);
           imageStore(si1, ivec2(0), vec4(0));
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineColorBlendAttachmentState cb_attachments[2] = {};
    memset(cb_attachments, 0, sizeof(VkPipelineColorBlendAttachmentState) * 2);
    CreatePipelineHelper g_pipe(*this);
    g_pipe.gp_ci_.renderPass = rp.Handle();
    g_pipe.shader_stages_ = {g_pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.cb_ci_.attachmentCount = 2;
    g_pipe.cb_ci_.pAttachments = cb_attachments;
    g_pipe.CreateGraphicsPipeline();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    // Use protected resources in unprotected command buffer
    g_pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer_protected.handle(), 0, 1024);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(1, image_views_descriptor[0], sampler.handle(),
                                                     VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    // will get undefined values, but not invalid if protectedNoFault is supported
    // Will still create an empty command buffer to test submit VUs if protectedNoFault is supported
    if (!protected_memory_properties.protectedNoFault) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdBlitImage-commandBuffer-01834");
        vk::CmdBlitImage(m_command_buffer.handle(), image_protected.handle(), VK_IMAGE_LAYOUT_GENERAL, image_unprotected.handle(),
                         VK_IMAGE_LAYOUT_GENERAL, 1, &blit_region, VK_FILTER_NEAREST);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdBlitImage-commandBuffer-01835");
        vk::CmdBlitImage(m_command_buffer.handle(), image_unprotected.handle(), VK_IMAGE_LAYOUT_GENERAL, image_protected.handle(),
                         VK_IMAGE_LAYOUT_GENERAL, 1, &blit_region, VK_FILTER_NEAREST);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-commandBuffer-01805");
        vk::CmdClearColorImage(m_command_buffer.handle(), image_protected.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1,
                               &subresource_range);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBuffer-commandBuffer-01822");
        vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_protected.handle(), buffer_unprotected.handle(), 1, &buffer_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBuffer-commandBuffer-01823");
        vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_unprotected.handle(), buffer_protected.handle(), 1, &buffer_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-commandBuffer-01828");
        vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_protected.handle(), image_unprotected.handle(),
                                 VK_IMAGE_LAYOUT_GENERAL, 1, &buffer_image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-commandBuffer-01829");
        vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_unprotected.handle(), image_protected.handle(),
                                 VK_IMAGE_LAYOUT_GENERAL, 1, &buffer_image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-commandBuffer-01825");
        vk::CmdCopyImage(m_command_buffer.handle(), image_protected.handle(), VK_IMAGE_LAYOUT_GENERAL, image_unprotected.handle(),
                         VK_IMAGE_LAYOUT_GENERAL, 1, &image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-commandBuffer-01826");
        vk::CmdCopyImage(m_command_buffer.handle(), image_unprotected.handle(), VK_IMAGE_LAYOUT_GENERAL, image_protected.handle(),
                         VK_IMAGE_LAYOUT_GENERAL, 1, &image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-commandBuffer-01831");
        vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image_protected.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                 buffer_unprotected.handle(), 1, &buffer_image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-commandBuffer-01832");
        vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image_unprotected.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                 buffer_protected.handle(), 1, &buffer_image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdFillBuffer-commandBuffer-01811");
        vk::CmdFillBuffer(m_command_buffer.handle(), buffer_protected.handle(), 0, 4, 0);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdUpdateBuffer-commandBuffer-01813");
        vk::CmdUpdateBuffer(m_command_buffer.handle(), buffer_protected.handle(), 0, 4, (void *)update_data);
        m_errorMonitor->VerifyFound();

        m_command_buffer.BeginRenderPass(rp.Handle(), framebuffer.handle(), render_area.extent.width, render_area.extent.height);

        m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-commandBuffer-02504");
        vk::CmdClearAttachments(m_command_buffer.handle(), 2, clear_attachments, 2, clear_rect);
        m_errorMonitor->VerifyFound();

        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
        vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0,
                                  1, &g_pipe.descriptor_set_->set_, 0, nullptr);
        VkDeviceSize offset = 0;
        vk::CmdBindVertexBuffers(m_command_buffer.handle(), 0, 1, &buffer_protected.handle(), &offset);
        vk::CmdBindIndexBuffer(m_command_buffer.handle(), buffer_protected.handle(), 0, VK_INDEX_TYPE_UINT16);

        m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexed-commandBuffer-02707");  // color attachment
        m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexed-commandBuffer-02707");  // buffer descriptorSet
        m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexed-commandBuffer-02707");  // image descriptorSet
        m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexed-commandBuffer-02707");  // vertex
        m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexed-commandBuffer-02707");  // index

        vk::CmdDrawIndexed(m_command_buffer.handle(), 1, 0, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        m_command_buffer.EndRenderPass();
    }
    m_command_buffer.End();

    // Use unprotected resources in protected command buffer
    g_pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer_unprotected.handle(), 0, 1024);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(1, image_views_descriptor[1], sampler.handle(),
                                                     VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    protectedCommandBuffer.Begin();
    if (!protected_memory_properties.protectedNoFault) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdBlitImage-commandBuffer-01836");
        vk::CmdBlitImage(protectedCommandBuffer.handle(), image_protected.handle(), VK_IMAGE_LAYOUT_GENERAL,
                         image_unprotected.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &blit_region, VK_FILTER_NEAREST);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-commandBuffer-01806");
        vk::CmdClearColorImage(protectedCommandBuffer.handle(), image_unprotected.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_color,
                               1, &subresource_range);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBuffer-commandBuffer-01824");
        vk::CmdCopyBuffer(protectedCommandBuffer.handle(), buffer_protected.handle(), buffer_unprotected.handle(), 1, &buffer_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-commandBuffer-01830");
        vk::CmdCopyBufferToImage(protectedCommandBuffer.handle(), buffer_protected.handle(), image_unprotected.handle(),
                                 VK_IMAGE_LAYOUT_GENERAL, 1, &buffer_image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-commandBuffer-01827");
        vk::CmdCopyImage(protectedCommandBuffer.handle(), image_protected.handle(), VK_IMAGE_LAYOUT_GENERAL,
                         image_unprotected.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-commandBuffer-01833");
        vk::CmdCopyImageToBuffer(protectedCommandBuffer.handle(), image_protected.handle(), VK_IMAGE_LAYOUT_GENERAL,
                                 buffer_unprotected.handle(), 1, &buffer_image_copy);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdFillBuffer-commandBuffer-01812");
        vk::CmdFillBuffer(protectedCommandBuffer.handle(), buffer_unprotected.handle(), 0, 4, 0);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdUpdateBuffer-commandBuffer-01814");
        vk::CmdUpdateBuffer(protectedCommandBuffer.handle(), buffer_unprotected.handle(), 0, 4, (void *)update_data);
        m_errorMonitor->VerifyFound();

        protectedCommandBuffer.BeginRenderPass(rp.Handle(), framebuffer.handle(), render_area.extent.width,
                                               render_area.extent.height);

        m_errorMonitor->SetDesiredError("VUID-vkCmdClearAttachments-commandBuffer-02505");
        vk::CmdClearAttachments(protectedCommandBuffer.handle(), 2, clear_attachments, 2, clear_rect);
        m_errorMonitor->VerifyFound();

        vk::CmdBindPipeline(protectedCommandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
        vk::CmdBindDescriptorSets(protectedCommandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  g_pipe.pipeline_layout_.handle(), 0, 1, &g_pipe.descriptor_set_->set_, 0, nullptr);
        VkDeviceSize offset = 0;
        vk::CmdBindVertexBuffers(protectedCommandBuffer.handle(), 0, 1, &buffer_unprotected.handle(), &offset);
        vk::CmdBindIndexBuffer(protectedCommandBuffer.handle(), buffer_unprotected.handle(), 0, VK_INDEX_TYPE_UINT16);

        m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexed-commandBuffer-02712");  // color attachment
        m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexed-commandBuffer-02712");  // descriptorSet
        vk::CmdDrawIndexed(protectedCommandBuffer.handle(), 1, 0, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        vk::CmdEndRenderPass(protectedCommandBuffer.handle());
    }
    protectedCommandBuffer.End();

    // Try submitting together to test only 1 error occurs for the corresponding command buffer
    VkCommandBuffer comman_buffers[2] = {m_command_buffer.handle(), protectedCommandBuffer.handle()};

    VkProtectedSubmitInfo protected_submit_info = vku::InitStructHelper();
    VkSubmitInfo submit_info = vku::InitStructHelper(&protected_submit_info);
    submit_info.commandBufferCount = 2;
    submit_info.pCommandBuffers = comman_buffers;

    protected_submit_info.protectedSubmit = VK_TRUE;
    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-queue-06448");
    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-pNext-04148");
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    protected_submit_info.protectedSubmit = VK_FALSE;
    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-pNext-04120");
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // If the VkSubmitInfo::pNext chain does not contain an instance of this structure, the batch is unprotected.
    submit_info.pNext = nullptr;
    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-pNext-04120");
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeProtectedMemory, RayTracingPipeline) {
    TEST_DESCRIPTION("Bind ray tracing pipeline in a protected command buffer");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::protectedMemory);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitFramework());
    RETURN_IF_SKIP(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_PROTECTED_BIT));

    const vkt::PipelineLayout empty_pipeline_layout(*m_device, {});
    VkShaderObj rgen_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj chit_shader(this, kRayTracingMinimalGlsl, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);

    const vkt::PipelineLayout pipeline_layout(*m_device, {});

    std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages;
    shader_stages[0] = vku::InitStructHelper();
    shader_stages[0].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    shader_stages[0].module = chit_shader.handle();
    shader_stages[0].pName = "main";

    shader_stages[1] = vku::InitStructHelper();
    shader_stages[1].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    shader_stages[1].module = rgen_shader.handle();
    shader_stages[1].pName = "main";

    std::array<VkRayTracingShaderGroupCreateInfoKHR, 1> shader_groups;
    shader_groups[0] = vku::InitStructHelper();
    shader_groups[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    shader_groups[0].generalShader = 1;
    shader_groups[0].closestHitShader = VK_SHADER_UNUSED_KHR;
    shader_groups[0].anyHitShader = VK_SHADER_UNUSED_KHR;
    shader_groups[0].intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineCreateInfoKHR raytracing_pipeline_ci = vku::InitStructHelper();
    raytracing_pipeline_ci.flags = 0;
    raytracing_pipeline_ci.stageCount = static_cast<uint32_t>(shader_stages.size());
    raytracing_pipeline_ci.pStages = shader_stages.data();
    raytracing_pipeline_ci.pGroups = shader_groups.data();
    raytracing_pipeline_ci.groupCount = shader_groups.size();
    raytracing_pipeline_ci.layout = pipeline_layout.handle();

    VkPipeline raytracing_pipeline = VK_NULL_HANDLE;
    vk::CreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &raytracing_pipeline_ci, nullptr,
                                     &raytracing_pipeline);

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindPipeline-pipelineBindPoint-06721");
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, raytracing_pipeline);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    vk::DestroyPipeline(device(), raytracing_pipeline, nullptr);
}

TEST_F(NegativeProtectedMemory, RayQuery) {
    TEST_DESCRIPTION("Bind ray tracing pipeline in a protected command buffer");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayQuery);
    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(InitFramework());
    // Turns m_command_buffer into a protected command buffer
    RETURN_IF_SKIP(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_PROTECTED_BIT));
    InitRenderTarget();

    // kFragmentMinimalGlsl but with added OpCapability RayQueryKHR
    const char *spv_source = R"(
               OpCapability Shader
               OpCapability RayQueryKHR
               OpExtension "SPV_KHR_ray_query"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %uFragColor
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %uFragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
 %uFragColor = OpVariable %_ptr_Output_v4float Output
    %float_0 = OpConstant %float 0
    %float_1 = OpConstant %float 1
         %12 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpStore %uFragColor %12
               OpReturn
               OpFunctionEnd
        )";
    VkShaderObj fs(this, spv_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    m_errorMonitor->SetUnexpectedError("VUID-vkCmdDraw-commandBuffer-02712");  // color attachment
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-commandBuffer-04617");     // rayQuery
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeProtectedMemory, Usage) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(Init());

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.flags = VK_BUFFER_CREATE_PROTECTED_BIT;
    buffer_create_info.size = 4096;
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-flags-09641");
}

TEST_F(NegativeProtectedMemory, WriteToProtectedStorageBuffer) {
    TEST_DESCRIPTION("Test OpStore on protected resources");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    AddRequiredFeature(vkt::Feature::protectedMemory);
    AddRequiredFeature(vkt::Feature::fragmentStoresAndAtomics);
    RETURN_IF_SKIP(InitFramework());
    RETURN_IF_SKIP(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_PROTECTED_BIT));
    InitRenderTarget();

    VkPhysicalDeviceProtectedMemoryProperties protected_memory_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(protected_memory_properties);
    if (protected_memory_properties.protectedNoFault) {
        GTEST_SKIP() << "protectedNoFault feature is supported";
    }
    vkt::Buffer buffer_unprotected(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    const char fsSource[] = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer block { vec4 x; };
        layout(location=0) out vec4 color;
        void main(){
           x = gl_FragCoord;
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    pipe.CreateGraphicsPipeline();

    // Use unprotected resources in protected command buffer
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer_unprotected, 0, 256, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-commandBuffer-02712");  // color attachment
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-commandBuffer-02712");  // Write to SSBO
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    vk::CmdEndRenderPass(m_command_buffer.handle());
    m_command_buffer.End();
}

TEST_F(NegativeProtectedMemory, ResolveProtectedImage) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceProtectedMemoryProperties prot_mem_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props = vku::InitStructHelper(&prot_mem_props);
    vk::GetPhysicalDeviceProperties2(Gpu(), &props);

    const auto protected_no_fault_supported = (prot_mem_props.protectedNoFault == VK_TRUE);
    if (protected_no_fault_supported) {
        GTEST_SKIP() << "Test requires protectedMemory support without protectedNoFault support";
    }

    VkImageFormatProperties2 image_format_prop = vku::InitStructHelper();
    VkPhysicalDeviceImageFormatInfo2 image_format_info = vku::InitStructHelper();
    image_format_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_format_info.type = VK_IMAGE_TYPE_2D;
    image_format_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_format_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    VkResult result =
        vk::GetPhysicalDeviceImageFormatProperties2(m_device->Physical().handle(), &image_format_info, &image_format_prop);
    if ((result != VK_SUCCESS) || !(image_format_prop.imageFormatProperties.sampleCounts & VK_SAMPLE_COUNT_4_BIT)) {
        GTEST_SKIP() << "Cannot create an image with format VK_FORMAT_R8G8B8A8_UNORM and sample count VK_SAMPLE_COUNT_4_BIT. "
                        "Skipping remainder of the test";
    }

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
    vkt::Image ms_image(*m_device, image_ci);
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.flags = VK_IMAGE_CREATE_PROTECTED_BIT;
    vkt::Image image_protected(*m_device, image_ci, vkt::no_mem);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
    vkt::Image ms_image_protected(*m_device, image_ci, vkt::no_mem);

    VkPhysicalDeviceMemoryProperties phys_mem_props;
    vk::GetPhysicalDeviceMemoryProperties(Gpu(), &phys_mem_props);
    uint32_t protected_index = phys_mem_props.memoryTypeCount;
    for (uint32_t i = 0; i < phys_mem_props.memoryTypeCount; i++) {
        if ((phys_mem_props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) != 0) {
            protected_index = i;
            break;
        }
    }

    VkMemoryRequirements mem_reqs;
    vk::GetImageMemoryRequirements(device(), image_protected, &mem_reqs);
    VkMemoryRequirements ms_mem_reqs;
    vk::GetImageMemoryRequirements(device(), ms_image_protected, &ms_mem_reqs);

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.allocationSize = mem_reqs.size;
    mem_alloc.memoryTypeIndex = protected_index;
    vkt::DeviceMemory memory(*m_device, mem_alloc);
    vk::BindImageMemory(device(), image_protected.handle(), memory.handle(), 0);

    mem_alloc.allocationSize = ms_mem_reqs.size;
    vkt::DeviceMemory ms_memory(*m_device, mem_alloc);
    vk::BindImageMemory(device(), ms_image_protected.handle(), ms_memory.handle(), 0);

    m_command_buffer.Begin();

    VkImageResolve region;
    region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.srcOffset = {0, 0, 0};
    region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.dstOffset = {0, 0, 0};
    region.extent = {1u, 1u, 1u};

    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-commandBuffer-01837");
    vk::CmdResolveImage(m_command_buffer.handle(), ms_image_protected.handle(), VK_IMAGE_LAYOUT_GENERAL, image.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1u, &region);

    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-commandBuffer-01838");
    vk::CmdResolveImage(m_command_buffer.handle(), ms_image.handle(), VK_IMAGE_LAYOUT_GENERAL, image_protected.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1u, &region);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeProtectedMemory, ResolveImageInProtectedCmdBuffer) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(InitFramework());
    RETURN_IF_SKIP(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_PROTECTED_BIT));

    VkPhysicalDeviceProtectedMemoryProperties prot_mem_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props = vku::InitStructHelper(&prot_mem_props);
    vk::GetPhysicalDeviceProperties2(Gpu(), &props);

    const auto protected_no_fault_supported = (prot_mem_props.protectedNoFault == VK_TRUE);
    if (protected_no_fault_supported) {
        GTEST_SKIP() << "Test requires protectedMemory support without protectedNoFault support";
    }

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
    vkt::Image ms_image(*m_device, image_ci);

    m_command_buffer.Begin();

    VkImageResolve region;
    region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.srcOffset = {0, 0, 0};
    region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.dstOffset = {0, 0, 0};
    region.extent = {1u, 1u, 1u};

    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-commandBuffer-01839");
    vk::CmdResolveImage(m_command_buffer.handle(), ms_image.handle(), VK_IMAGE_LAYOUT_GENERAL, image.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1u, &region);

    m_errorMonitor->VerifyFound();
}