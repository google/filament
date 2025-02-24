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
class NegativeThreading : public VkLayerTest {};

TEST_F(NegativeThreading, CommandBufferCollision) {
    m_errorMonitor->SetDesiredError("THREADING ERROR");
    m_errorMonitor->SetAllowedFailureMsg("THREADING ERROR");  // Ignore any extra threading errors found beyond the first one

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Test takes magnitude of time longer for profiles and slows down testing
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    // Calls AllocateCommandBuffers
    vkt::CommandBuffer commandBuffer(*m_device, m_command_pool);

    commandBuffer.Begin();

    vkt::Event event(*m_device);
    VkResult err;

    err = vk::ResetEvent(device(), event.handle());
    ASSERT_EQ(VK_SUCCESS, err);

    ThreadTestData data;
    data.commandBuffer = commandBuffer.handle();
    data.event = event.handle();
    std::atomic<bool> bailout{false};
    data.bailout = &bailout;
    m_errorMonitor->SetBailout(data.bailout);

    // First do some correct operations using multiple threads.
    // Add many entries to command buffer from another thread.
    std::thread thread1(AddToCommandBuffer, &data);
    // Make non-conflicting calls from this thread at the same time.
    for (int i = 0; i < 80000; i++) {
        uint32_t count;
        vk::EnumeratePhysicalDevices(instance(), &count, NULL);
    }
    thread1.join();

    // Then do some incorrect operations using multiple threads.
    // Add many entries to command buffer from another thread.
    std::thread thread2(AddToCommandBuffer, &data);
    // Add many entries to command buffer from this thread at the same time.
    AddToCommandBuffer(&data);

    thread2.join();
    commandBuffer.End();

    m_errorMonitor->SetBailout(NULL);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeThreading, UpdateDescriptorCollision) {
    TEST_DESCRIPTION("Two threads updating the same descriptor set, expected to generate a threading error");

    m_errorMonitor->SetDesiredError("vkUpdateDescriptorSets(): THREADING ERROR");
    m_errorMonitor->SetAllowedFailureMsg("THREADING ERROR");  // Ignore any extra threading errors found beyond the first one

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    OneOffDescriptorSet normal_descriptor_set(m_device,
                                              {
                                                  {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                              },
                                              0);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    ThreadTestData data;
    data.device = device();
    data.descriptorSet = normal_descriptor_set.set_;
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
    data2.descriptorSet = normal_descriptor_set.set_;
    data2.binding = 1;
    data2.buffer = buffer.handle();
    data2.bailout = &bailout;

    UpdateDescriptor(&data2);

    thread.join();

    m_errorMonitor->SetBailout(NULL);

    m_errorMonitor->VerifyFound();
}
#endif  // GTEST_IS_THREADSAFE
