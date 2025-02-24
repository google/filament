/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 * Modifications Copyright (C) 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <thread>
#include "../framework/layer_validation_tests.h"
#include "../framework/descriptor_helper.h"
#include "../framework/thread_helper.h"

#if GTEST_IS_THREADSAFE
class PositiveThreading : public VkLayerTest {};

TEST_F(PositiveThreading, DisplayObjects) {
    TEST_DESCRIPTION("Create and use VkDisplayKHR objects with GetPhysicalDeviceDisplayPropertiesKHR in thread-safety.");

    AddRequiredExtensions(VK_KHR_SURFACE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DISPLAY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    uint32_t prop_count = 0;
    vk::GetPhysicalDeviceDisplayPropertiesKHR(Gpu(), &prop_count, nullptr);
    if (prop_count == 0) {
        GTEST_SKIP() << "No VkDisplayKHR properties to query";
    }

    std::vector<VkDisplayPropertiesKHR> display_props{prop_count};
    // Create a VkDisplayKHR object
    vk::GetPhysicalDeviceDisplayPropertiesKHR(Gpu(), &prop_count, display_props.data());
    ASSERT_NE(prop_count, 0U);

    // Now use this new object in an API call that thread safety will track
    prop_count = 0;
    vk::GetDisplayModePropertiesKHR(Gpu(), display_props[0].display, &prop_count, nullptr);
}

TEST_F(PositiveThreading, DisplayPlaneObjects) {
    TEST_DESCRIPTION("Create and use VkDisplayKHR objects with GetPhysicalDeviceDisplayPlanePropertiesKHR in thread-safety.");

    AddRequiredExtensions(VK_KHR_SURFACE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DISPLAY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    uint32_t prop_count = 0;
    vk::GetPhysicalDeviceDisplayPlanePropertiesKHR(Gpu(), &prop_count, nullptr);
    if (prop_count != 0) {
        // only grab first plane property
        prop_count = 1;
        VkDisplayPlanePropertiesKHR display_plane_props = {};
        // Create a VkDisplayKHR object
        vk::GetPhysicalDeviceDisplayPlanePropertiesKHR(Gpu(), &prop_count, &display_plane_props);
        // Now use this new object in an API call
        prop_count = 0;
        vk::GetDisplayModePropertiesKHR(Gpu(), display_plane_props.currentDisplay, &prop_count, nullptr);
    }
}

TEST_F(PositiveThreading, UpdateDescriptorUpdateAfterBindNoCollision) {
    TEST_DESCRIPTION("Two threads updating the same UAB descriptor set, expected not to generate a threading error");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingStorageBufferUpdateAfterBind);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    OneOffDescriptorIndexingSet descriptor_set(m_device, {
                                                             {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
                                                              nullptr, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
                                                             {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
                                                              nullptr, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
                                                         });
    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    ThreadTestData data;
    data.device = device();
    data.descriptorSet = descriptor_set.set_;
    data.binding = 0;
    data.buffer = buffer.handle();
    std::atomic<bool> bailout{false};
    data.bailout = &bailout;
    m_errorMonitor->SetBailout(data.bailout);

    // Update descriptors from another thread.
    std::thread thread(UpdateDescriptor, &data);
    // Update descriptors from this thread at the same time.

    ThreadTestData data2;
    data2.device = device();
    data2.descriptorSet = descriptor_set.set_;
    data2.binding = 1;
    data2.buffer = buffer.handle();
    data2.bailout = &bailout;

    UpdateDescriptor(&data2);

    thread.join();

    m_errorMonitor->SetBailout(NULL);
}

TEST_F(PositiveThreading, UpdateDescriptorUnusedWhilePendingNoCollision) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8975");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingUpdateUnusedWhilePending);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    OneOffDescriptorIndexingSet descriptor_set(m_device, {
                                                             {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
                                                              nullptr, VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT},
                                                             {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
                                                              nullptr, VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT},
                                                         });
    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    ThreadTestData data;
    data.device = device();
    data.descriptorSet = descriptor_set.set_;
    data.binding = 0;
    data.buffer = buffer.handle();
    std::atomic<bool> bailout{false};
    data.bailout = &bailout;
    m_errorMonitor->SetBailout(data.bailout);

    // Update descriptors from another thread.
    std::thread thread(UpdateDescriptor, &data);
    // Update descriptors from this thread at the same time.

    ThreadTestData data2;
    data2.device = device();
    data2.descriptorSet = descriptor_set.set_;
    data2.binding = 1;
    data2.buffer = buffer.handle();
    data2.bailout = &bailout;

    UpdateDescriptor(&data2);

    thread.join();

    m_errorMonitor->SetBailout(NULL);
}

TEST_F(PositiveThreading, NullFenceCollision) {
    RETURN_IF_SKIP(Init());

    ThreadTestData data;
    data.device = device();
    std::atomic<bool> bailout{false};
    data.bailout = &bailout;
    m_errorMonitor->SetBailout(data.bailout);

    // Call vk::DestroyFence of VK_NULL_HANDLE repeatedly using multiple threads.
    // There should be no validation error from collision of that non-object.
    std::thread thread(ReleaseNullFence, &data);
    for (int i = 0; i < 40000; i++) {
        vk::DestroyFence(device(), VK_NULL_HANDLE, NULL);
    }
    thread.join();

    m_errorMonitor->SetBailout(NULL);
}

TEST_F(PositiveThreading, DebugObjectNames) {
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework(&kDisableMessageLimit));
    RETURN_IF_SKIP(InitState());

    constexpr uint32_t count = 10000u;

    VkDescriptorPoolSize pool_size;
    pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    pool_size.descriptorCount = count;

    VkDescriptorPoolCreateInfo descriptor_pool_ci = vku::InitStructHelper();
    descriptor_pool_ci.maxSets = count;
    descriptor_pool_ci.poolSizeCount = 1u;
    descriptor_pool_ci.pPoolSizes = &pool_size;
    vkt::DescriptorPool descriptor_pool(*m_device, descriptor_pool_ci);

    VkDescriptorSetLayoutBinding binding;
    binding.binding = 0u;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    binding.descriptorCount = 1u;
    binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo set_layout_ci = vku::InitStructHelper();
    set_layout_ci.bindingCount = 1u;
    set_layout_ci.pBindings = &binding;
    vkt::DescriptorSetLayout set_layout(*m_device, set_layout_ci);

    binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    vkt::DescriptorSetLayout set_layout2(*m_device, set_layout_ci);

    vkt::PipelineLayout pipeline_layout(*m_device, {&set_layout2});

    VkDescriptorSetAllocateInfo allocate_info = vku::InitStructHelper();
    allocate_info.descriptorPool = descriptor_pool.handle();
    allocate_info.descriptorSetCount = 1u;
    allocate_info.pSetLayouts = &set_layout.handle();

    VkDescriptorSet descriptor_sets[count];
    for (uint32_t i = 0; i < count; ++i) {
        vk::AllocateDescriptorSets(*m_device, &allocate_info, &descriptor_sets[i]);
    }

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VkDescriptorBufferInfo buffer_info;
    buffer_info.buffer = buffer.handle();
    buffer_info.offset = 0u;
    buffer_info.range = 256u;

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstBinding = 0u;
    descriptor_write.dstArrayElement = 0u;
    descriptor_write.descriptorCount = 1u;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.pBufferInfo = &buffer_info;
    descriptor_write.pTexelBufferView = nullptr;

    for (uint32_t i = 0; i < count; ++i) {
        descriptor_write.dstSet = descriptor_sets[i];
        vk::UpdateDescriptorSets(*m_device, 1u, &descriptor_write, 0u, nullptr);
    }

    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;

    std::atomic<bool> bailout{false};

    for (uint32_t i = 0; i < count; ++i) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorSets-pDescriptorSets-00358");
    }

    m_errorMonitor->SetBailout(&bailout);
    const auto set_name = [&]() {
        for (uint32_t i = 0; i < count; ++i) {
            std::string name = "handle" + std::to_string(i);
            name_info.objectHandle = (uint64_t)descriptor_sets[i];
            name_info.pObjectName = name.c_str();
            vk::SetDebugUtilsObjectNameEXT(*m_device, &name_info);
            if (i % 3 == 0) {
                name_info.pObjectName = nullptr;
                vk::SetDebugUtilsObjectNameEXT(*m_device, &name_info);
            }
        }
    };
    const auto bind_descriptor = [&]() {
        m_command_buffer.Begin();
        for (uint32_t i = 0; i < count; ++i) {
            vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0u, 1u,
                                      &descriptor_sets[i], 0u, nullptr);
        }
        m_command_buffer.End();
    };

    std::thread thread2(bind_descriptor);
    std::thread thread1(set_name);

    thread1.join();
    thread2.join();

    m_errorMonitor->SetBailout(NULL);

    m_errorMonitor->VerifyFound();
}

#endif  // GTEST_IS_THREADSAFE

TEST_F(PositiveThreading, Queue) {
#if defined(VVL_ENABLE_TSAN)
    GTEST_SKIP() << "https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5965";
#endif

    TEST_DESCRIPTION("Test concurrent Queue access from vkGet and vkSubmit");

    using namespace std::chrono;
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());

    const auto queue_family = m_device->QueuesWithGraphicsCapability()[0]->family_index;
    constexpr uint32_t queue_index = 0;
    vkt::CommandPool command_pool(*m_device, queue_family);

    const VkDevice device_h = device();
    VkQueue queue_h;
    vk::GetDeviceQueue(device(), queue_family, queue_index, &queue_h);
    vkt::Queue queue_o(queue_h, queue_family);

    const VkCommandBufferAllocateInfo cbai = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, command_pool.handle(),
                                              VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1};
    vkt::CommandBuffer mock_cmdbuff(*m_device, cbai);
    const VkCommandBufferBeginInfo cbbi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                        VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, nullptr};
    mock_cmdbuff.Begin(&cbbi);
    mock_cmdbuff.End();

    std::mutex queue_mutex;

    constexpr auto test_duration = seconds{2};
    const auto timer_begin = steady_clock::now();

    const auto &testing_thread1 = [&]() {
        for (auto timer_now = steady_clock::now(); timer_now - timer_begin < test_duration; timer_now = steady_clock::now()) {
            VkQueue dummy_q;
            vk::GetDeviceQueue(device_h, queue_family, queue_index, &dummy_q);
        }
    };

    const auto &testing_thread2 = [&]() {
        for (auto timer_now = steady_clock::now(); timer_now - timer_begin < test_duration; timer_now = steady_clock::now()) {
            VkSubmitInfo si = vku::InitStructHelper();
            si.commandBufferCount = 1;
            si.pCommandBuffers = &mock_cmdbuff.handle();
            queue_mutex.lock();
            ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit(queue_h, 1, &si, VK_NULL_HANDLE));
            queue_mutex.unlock();
        }
    };

    const auto &testing_thread3 = [&]() {
        for (auto timer_now = steady_clock::now(); timer_now - timer_begin < test_duration; timer_now = steady_clock::now()) {
            queue_mutex.lock();
            ASSERT_EQ(VK_SUCCESS, vk::QueueWaitIdle(queue_h));
            queue_mutex.unlock();
        }
    };

    std::array<std::thread, 3> threads = {std::thread(testing_thread1), std::thread(testing_thread2), std::thread(testing_thread3)};
    for (auto &t : threads) t.join();

    vk::QueueWaitIdle(queue_h);
}
