/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "utils/cast_utils.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

class NegativeQuery : public QueryTest {};

TEST_F(NegativeQuery, PerformanceCreation) {
    TEST_DESCRIPTION("Create performance query without support");
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::performanceCounterQueryPools);
    RETURN_IF_SKIP(Init());

    auto queueFamilyProperties = m_device->Physical().queue_properties_;
    uint32_t queueFamilyIndex = queueFamilyProperties.size();
    std::vector<VkPerformanceCounterKHR> counters;

    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        uint32_t nCounters;

        vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(Gpu(), idx, &nCounters, nullptr, nullptr);
        if (nCounters == 0) continue;

        counters.resize(nCounters);
        for (auto &c : counters) {
            c = vku::InitStructHelper();
        }
        vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(Gpu(), idx, &nCounters, &counters[0], nullptr);
        queueFamilyIndex = idx;
        break;
    }

    if (counters.empty()) {
        GTEST_SKIP() << "No queue reported any performance counter.";
    }

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci = vku::InitStructHelper();
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counters.size();
    std::vector<uint32_t> counterIndices;
    for (uint32_t c = 0; c < counters.size(); c++) counterIndices.push_back(c);
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;

    vkt::QueryPool query_pool;

    // Missing pNext
    m_errorMonitor->SetDesiredError("VUID-VkQueryPoolCreateInfo-queryType-03222");
    query_pool.init(*m_device, query_pool_ci);
    m_errorMonitor->VerifyFound();

    query_pool_ci.pNext = &perf_query_pool_ci;

    // Invalid counter indices
    counterIndices.push_back(counters.size());
    perf_query_pool_ci.counterIndexCount++;
    perf_query_pool_ci.pCounterIndices = counterIndices.data();
    m_errorMonitor->SetDesiredError("VUID-VkQueryPoolPerformanceCreateInfoKHR-pCounterIndices-03321");
    query_pool.init(*m_device, query_pool_ci);
    m_errorMonitor->VerifyFound();
    perf_query_pool_ci.counterIndexCount--;
    counterIndices.pop_back();

    // Success
    query_pool.init(*m_device, query_pool_ci);

    m_command_buffer.Begin();

    // Missing acquire lock
    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryPool-03223");
        vk::CmdBeginQuery(m_command_buffer.handle(), query_pool, 0, 0);
        m_errorMonitor->VerifyFound();
    }

    m_command_buffer.End();
}

TEST_F(NegativeQuery, PerformanceCounterCommandbufferScope) {
    TEST_DESCRIPTION("Insert a performance query begin/end with respect to the command buffer counter scope");

    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::performanceCounterQueryPools);
    RETURN_IF_SKIP(Init());

    auto queueFamilyProperties = m_device->Physical().queue_properties_;
    uint32_t queueFamilyIndex = queueFamilyProperties.size();
    std::vector<VkPerformanceCounterKHR> counters;
    std::vector<uint32_t> counterIndices;

    // Find a single counter with VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_BUFFER_KHR scope.
    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        uint32_t nCounters;

        vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(Gpu(), idx, &nCounters, nullptr, nullptr);
        if (nCounters == 0) continue;

        counters.resize(nCounters);
        for (auto &c : counters) {
            c = vku::InitStructHelper();
        }
        vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(Gpu(), idx, &nCounters, &counters[0], nullptr);
        queueFamilyIndex = idx;

        for (uint32_t counterIdx = 0; counterIdx < counters.size(); counterIdx++) {
            if (counters[counterIdx].scope == VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_BUFFER_KHR) {
                counterIndices.push_back(counterIdx);
                break;
            }
        }

        if (counterIndices.empty()) {
            counters.clear();
            continue;
        }
        break;
    }

    if (counterIndices.empty()) {
        GTEST_SKIP() << "No queue reported any performance counter with command buffer scope.";
    }

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci = vku::InitStructHelper();
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper(&perf_query_pool_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(device(), queueFamilyIndex, 0, &queue);

    {
        VkAcquireProfilingLockInfoKHR lock_info = vku::InitStructHelper();
        VkResult result = vk::AcquireProfilingLockKHR(device(), &lock_info);
        ASSERT_TRUE(result == VK_SUCCESS);
    }

    // Not the first command.
    {
        vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        m_command_buffer.Begin();
        vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 1);
        vk::CmdFillBuffer(m_command_buffer.handle(), buffer, 0, 4096, 0);

        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryPool-03224");
        vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
        m_errorMonitor->VerifyFound();

        m_command_buffer.End();

        VkSubmitInfo submit_info = vku::InitStructHelper();
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = NULL;
        submit_info.pWaitDstStageMask = NULL;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_command_buffer.handle();
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = NULL;
        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(queue);
    }

    // Not last command.
    {
        vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

        m_command_buffer.Begin();
        vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
        vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);
        vk::CmdFillBuffer(m_command_buffer.handle(), buffer, 0, 4096, 0);
        m_command_buffer.End();

        VkSubmitInfo submit_info = vku::InitStructHelper();
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = NULL;
        submit_info.pWaitDstStageMask = NULL;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_command_buffer.handle();
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = NULL;
        m_errorMonitor->SetDesiredError("VUID-vkCmdEndQuery-queryPool-03227");
        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }

    vk::ReleaseProfilingLockKHR(device());
}

TEST_F(NegativeQuery, PerformanceCounterRenderPassScope) {
    TEST_DESCRIPTION("Insert a performance query begin/end with respect to the render pass counter scope");

    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::performanceCounterQueryPools);
    RETURN_IF_SKIP(Init());

    auto queueFamilyProperties = m_device->Physical().queue_properties_;
    uint32_t queueFamilyIndex = queueFamilyProperties.size();
    std::vector<VkPerformanceCounterKHR> counters;
    std::vector<uint32_t> counterIndices;

    // Find a single counter with VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR scope.
    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        uint32_t nCounters;

        vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(Gpu(), idx, &nCounters, nullptr, nullptr);
        if (nCounters == 0) continue;

        counters.resize(nCounters);
        for (auto &c : counters) {
            c = vku::InitStructHelper();
        }
        vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(Gpu(), idx, &nCounters, &counters[0], nullptr);
        queueFamilyIndex = idx;

        for (uint32_t counterIdx = 0; counterIdx < counters.size(); counterIdx++) {
            if (counters[counterIdx].scope == VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR) {
                counterIndices.push_back(counterIdx);
                break;
            }
        }

        if (counterIndices.empty()) {
            counters.clear();
            continue;
        }
        break;
    }

    if (counterIndices.empty()) {
        GTEST_SKIP() << "No queue reported any performance counter with render pass scope.";
    }

    InitRenderTarget();

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci = vku::InitStructHelper();
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper(&perf_query_pool_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(device(), queueFamilyIndex, 0, &queue);

    {
        VkAcquireProfilingLockInfoKHR lock_info = vku::InitStructHelper();
        VkResult result = vk::AcquireProfilingLockKHR(device(), &lock_info);
        ASSERT_TRUE(result == VK_SUCCESS);
    }

    // Inside a render pass.
    {
        m_command_buffer.Begin();
        m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryPool-03225");
        vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
        m_errorMonitor->VerifyFound();

        m_command_buffer.EndRenderPass();
        m_command_buffer.End();

        VkSubmitInfo submit_info = vku::InitStructHelper();
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = NULL;
        submit_info.pWaitDstStageMask = NULL;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_command_buffer.handle();
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = NULL;
        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(queue);
    }

    vk::ReleaseProfilingLockKHR(device());
}

TEST_F(NegativeQuery, PerformanceReleaseProfileLockBeforeSubmit) {
    TEST_DESCRIPTION("Verify that we get an error if we release the profiling lock during the recording of performance queries");

    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::performanceCounterQueryPools);
    RETURN_IF_SKIP(Init());

    auto queueFamilyProperties = m_device->Physical().queue_properties_;
    uint32_t queueFamilyIndex = queueFamilyProperties.size();
    std::vector<VkPerformanceCounterKHR> counters;
    std::vector<uint32_t> counterIndices;

    // Find a single counter with VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_KHR scope.
    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        uint32_t nCounters;

        vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(Gpu(), idx, &nCounters, nullptr, nullptr);
        if (nCounters == 0) continue;

        counters.resize(nCounters);
        for (auto &c : counters) {
            c = vku::InitStructHelper();
        }
        vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(Gpu(), idx, &nCounters, &counters[0], nullptr);
        queueFamilyIndex = idx;

        for (uint32_t counterIdx = 0; counterIdx < counters.size(); counterIdx++) {
            if (counters[counterIdx].scope == VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_KHR) {
                counterIndices.push_back(counterIdx);
                break;
            }
        }

        if (counterIndices.empty()) {
            counters.clear();
            continue;
        }
        break;
    }

    if (counterIndices.empty()) {
        GTEST_SKIP() << "No queue reported any performance counter with render pass scope.";
    }

    InitRenderTarget();

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci = vku::InitStructHelper();
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper(&perf_query_pool_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(device(), queueFamilyIndex, 0, &queue);

    {
        VkAcquireProfilingLockInfoKHR lock_info = vku::InitStructHelper();
        VkResult result = vk::AcquireProfilingLockKHR(device(), &lock_info);
        ASSERT_TRUE(result == VK_SUCCESS);
    }

    {
        m_command_buffer.Reset();
        m_command_buffer.Begin();
        vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 1);
        m_command_buffer.End();

        VkSubmitInfo submit_info = vku::InitStructHelper();
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = NULL;
        submit_info.pWaitDstStageMask = NULL;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_command_buffer.handle();
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = NULL;

        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(queue);
    }

    {
        vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

        m_command_buffer.Reset();
        m_command_buffer.Begin();

        vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);

        // Release while recording.
        vk::ReleaseProfilingLockKHR(device());
        {
            VkAcquireProfilingLockInfoKHR lock_info = vku::InitStructHelper();
            VkResult result = vk::AcquireProfilingLockKHR(device(), &lock_info);
            ASSERT_TRUE(result == VK_SUCCESS);
        }

        vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);

        m_command_buffer.End();

        VkSubmitInfo submit_info = vku::InitStructHelper();
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = NULL;
        submit_info.pWaitDstStageMask = NULL;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_command_buffer.handle();
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = NULL;

        m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-03220");
        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        vk::QueueWaitIdle(queue);
    }

    vk::ReleaseProfilingLockKHR(device());
}

TEST_F(NegativeQuery, PerformanceIncompletePasses) {
    TEST_DESCRIPTION("Verify that we get an error if we don't submit a command buffer for each passes before getting the results.");
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME);

    // Vulkan 1.1 is a dependency of VK_KHR_video_queue, but both the version and the extension
    // is optional from the point of view of this test case
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::hostQueryReset);
    AddRequiredFeature(vkt::Feature::performanceCounterQueryPools);
    RETURN_IF_SKIP(Init());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR doesn't match up with profile queues";
    }

    VkPhysicalDevicePerformanceQueryPropertiesKHR perf_query_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(perf_query_props);

    auto queueFamilyProperties = m_device->Physical().queue_properties_;
    uint32_t queueFamilyIndex = queueFamilyProperties.size();
    std::vector<VkPerformanceCounterKHR> counters;
    std::vector<uint32_t> counterIndices;
    uint32_t nPasses = 0;

    // Find all counters with VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_KHR scope.
    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        uint32_t nCounters;

        vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(Gpu(), idx, &nCounters, nullptr, nullptr);
        if (nCounters == 0) continue;

        counters.resize(nCounters);
        for (auto &c : counters) {
            c = vku::InitStructHelper();
        }
        vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(Gpu(), idx, &nCounters, &counters[0], nullptr);
        queueFamilyIndex = idx;

        for (uint32_t counterIdx = 0; counterIdx < counters.size(); counterIdx++) {
            if (counters[counterIdx].scope == VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_KHR) counterIndices.push_back(counterIdx);
        }
        if (counterIndices.empty()) continue;  // might not be a scope command

        VkQueryPoolPerformanceCreateInfoKHR create_info = vku::InitStructHelper();
        create_info.queueFamilyIndex = idx;
        create_info.counterIndexCount = counterIndices.size();
        create_info.pCounterIndices = &counterIndices[0];

        vk::GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(Gpu(), &create_info, &nPasses);

        if (nPasses < 2) {
            counters.clear();
            continue;
        }
        break;
    }

    if (counterIndices.empty()) {
        GTEST_SKIP() << "No queue reported a set of counters that needs more than one pass.";
    }

    InitRenderTarget();

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci = vku::InitStructHelper();
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper(&perf_query_pool_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(device(), queueFamilyIndex, 0, &queue);

    {
        VkAcquireProfilingLockInfoKHR lock_info = vku::InitStructHelper();
        VkResult result = vk::AcquireProfilingLockKHR(device(), &lock_info);
        ASSERT_TRUE(result == VK_SUCCESS);
    }

    {
        const VkDeviceSize buf_size =
            std::max((VkDeviceSize)4096, (VkDeviceSize)(sizeof(VkPerformanceCounterResultKHR) * counterIndices.size()));
        vkt::Buffer buffer(*m_device, buf_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

        VkCommandBufferBeginInfo command_buffer_begin_info = vku::InitStructHelper();
        command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        vk::ResetQueryPoolEXT(device(), query_pool.handle(), 0, 1);

        m_command_buffer.Begin(&command_buffer_begin_info);
        vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
        vk::CmdFillBuffer(m_command_buffer.handle(), buffer, 0, buf_size, 0);
        vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);
        m_command_buffer.End();

        // Invalid pass index
        {
            VkPerformanceQuerySubmitInfoKHR perf_submit_info = vku::InitStructHelper();
            perf_submit_info.counterPassIndex = nPasses;
            VkSubmitInfo submit_info = vku::InitStructHelper(&perf_submit_info);
            submit_info.waitSemaphoreCount = 0;
            submit_info.pWaitSemaphores = NULL;
            submit_info.pWaitDstStageMask = NULL;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &m_command_buffer.handle();
            submit_info.signalSemaphoreCount = 0;
            submit_info.pSignalSemaphores = NULL;

            m_errorMonitor->SetDesiredError("VUID-VkPerformanceQuerySubmitInfoKHR-counterPassIndex-03221");
            vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
            m_errorMonitor->VerifyFound();
        }

        // Leave the last pass out.
        for (uint32_t passIdx = 0; passIdx < (nPasses - 1); passIdx++) {
            VkPerformanceQuerySubmitInfoKHR perf_submit_info = vku::InitStructHelper();
            perf_submit_info.counterPassIndex = passIdx;
            VkSubmitInfo submit_info = vku::InitStructHelper(&perf_submit_info);
            submit_info.waitSemaphoreCount = 0;
            submit_info.pWaitSemaphores = NULL;
            submit_info.pWaitDstStageMask = NULL;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &m_command_buffer.handle();
            submit_info.signalSemaphoreCount = 0;
            submit_info.pSignalSemaphores = NULL;

            vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        }

        vk::QueueWaitIdle(queue);

        std::vector<VkPerformanceCounterResultKHR> results;
        results.resize(counterIndices.size());

        m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-queryType-09441");
        vk::GetQueryPoolResults(device(), query_pool.handle(), 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(),
                                &results[0], sizeof(VkPerformanceCounterResultKHR) * results.size(), VK_QUERY_RESULT_WAIT_BIT);
        m_errorMonitor->VerifyFound();

        // submit the last pass
        {
            VkPerformanceQuerySubmitInfoKHR perf_submit_info = vku::InitStructHelper();
            perf_submit_info.counterPassIndex = nPasses - 1;
            VkSubmitInfo submit_info = vku::InitStructHelper(&perf_submit_info);
            submit_info.waitSemaphoreCount = 0;
            submit_info.pWaitSemaphores = NULL;
            submit_info.pWaitDstStageMask = NULL;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &m_command_buffer.handle();
            submit_info.signalSemaphoreCount = 0;
            submit_info.pSignalSemaphores = NULL;

            vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        }

        vk::QueueWaitIdle(queue);

        // The stride is too small to return the data
        if (counterIndices.size() > 2) {
            m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-queryType-04519");
            vk::GetQueryPoolResults(device(), query_pool.handle(), 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(),
                                    &results[0], sizeof(VkPerformanceCounterResultKHR) * (results.size() - 1), 0);
            m_errorMonitor->VerifyFound();
        }

        // Invalid stride
        {
            std::vector<VkPerformanceCounterResultKHR> results_invalid_stride;
            results_invalid_stride.resize(counterIndices.size() * 2);
            m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-queryType-03229");
            vk::GetQueryPoolResults(
                device(), query_pool.handle(), 0, 1, sizeof(VkPerformanceCounterResultKHR) * results_invalid_stride.size(),
                &results_invalid_stride[0], sizeof(VkPerformanceCounterResultKHR) * results_invalid_stride.size() + 4,
                VK_QUERY_RESULT_WAIT_BIT);
            m_errorMonitor->VerifyFound();
        }

        // Invalid flags for vkCmdCopyQueryPoolResults
        if (perf_query_props.allowCommandBufferQueryCopies) {
            m_command_buffer.Begin(&command_buffer_begin_info);
            m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-queryType-09440");
            vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool.handle(), 0, 1, buffer, 0,
                                        sizeof(VkPerformanceCounterResultKHR) * results.size(),
                                        VK_QUERY_RESULT_WITH_AVAILABILITY_BIT);
            m_errorMonitor->VerifyFound();
            m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-queryType-09440");
            vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool.handle(), 0, 1, buffer, 0,
                                        sizeof(VkPerformanceCounterResultKHR) * results.size(), VK_QUERY_RESULT_PARTIAL_BIT);
            m_errorMonitor->VerifyFound();
            m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-queryType-09440");
            vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool.handle(), 0, 1, buffer, 0,
                                        sizeof(VkPerformanceCounterResultKHR) * results.size(), VK_QUERY_RESULT_64_BIT);
            m_errorMonitor->VerifyFound();
            if (IsExtensionsEnabled(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME)) {
                m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-queryType-09440");
                vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool.handle(), 0, 1, buffer, 0,
                                            sizeof(VkPerformanceCounterResultKHR) * results.size(),
                                            VK_QUERY_RESULT_WITH_STATUS_BIT_KHR);
                m_errorMonitor->VerifyFound();
            }
            m_command_buffer.End();
        }

        // Invalid flags for vkGetQueryPoolResults
        m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-queryType-09440");
        vk::GetQueryPoolResults(device(), query_pool.handle(), 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(),
                                &results[0], sizeof(VkPerformanceCounterResultKHR) * results.size(),
                                VK_QUERY_RESULT_WITH_AVAILABILITY_BIT);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-queryType-09440");
        vk::GetQueryPoolResults(device(), query_pool.handle(), 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(),
                                &results[0], sizeof(VkPerformanceCounterResultKHR) * results.size(), VK_QUERY_RESULT_PARTIAL_BIT);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-queryType-09440");
        vk::GetQueryPoolResults(device(), query_pool.handle(), 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(),
                                &results[0], sizeof(VkPerformanceCounterResultKHR) * results.size(), VK_QUERY_RESULT_64_BIT);
        m_errorMonitor->VerifyFound();
        if (IsExtensionsEnabled(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME)) {
            m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-queryType-09440");
            vk::GetQueryPoolResults(device(), query_pool.handle(), 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(),
                                    &results[0], sizeof(VkPerformanceCounterResultKHR) * results.size(),
                                    VK_QUERY_RESULT_WITH_STATUS_BIT_KHR);
            m_errorMonitor->VerifyFound();
        }

        vk::GetQueryPoolResults(device(), query_pool.handle(), 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(),
                                &results[0], sizeof(VkPerformanceCounterResultKHR) * results.size(), VK_QUERY_RESULT_WAIT_BIT);
    }

    vk::ReleaseProfilingLockKHR(device());
}

TEST_F(NegativeQuery, PerformanceResetAndBegin) {
    TEST_DESCRIPTION("Verify that we get an error if we reset & begin a performance query within the same primary command buffer.");
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::hostQueryReset);
    AddRequiredFeature(vkt::Feature::performanceCounterQueryPools);
    RETURN_IF_SKIP(Init());

    auto queueFamilyProperties = m_device->Physical().queue_properties_;
    uint32_t queueFamilyIndex = queueFamilyProperties.size();
    std::vector<VkPerformanceCounterKHR> counters;
    std::vector<uint32_t> counterIndices;

    // Find a single counter with VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_KHR scope.
    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        uint32_t nCounters;

        vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(Gpu(), idx, &nCounters, nullptr, nullptr);
        if (nCounters == 0) continue;

        counters.resize(nCounters);
        for (auto &c : counters) {
            c = vku::InitStructHelper();
        }
        vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(Gpu(), idx, &nCounters, &counters[0], nullptr);
        queueFamilyIndex = idx;

        for (uint32_t counterIdx = 0; counterIdx < counters.size(); counterIdx++) {
            if (counters[counterIdx].scope == VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_KHR) {
                counterIndices.push_back(counterIdx);
                break;
            }
        }
        break;
    }

    if (counterIndices.empty()) {
        GTEST_SKIP() << "No queue reported a set of counters that needs more than one pass.";
    }

    InitRenderTarget();

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci = vku::InitStructHelper();
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper(&perf_query_pool_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(device(), queueFamilyIndex, 0, &queue);

    {
        VkAcquireProfilingLockInfoKHR lock_info = vku::InitStructHelper();
        VkResult result = vk::AcquireProfilingLockKHR(device(), &lock_info);
        ASSERT_TRUE(result == VK_SUCCESS);
    }

    {
        vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

        VkCommandBufferBeginInfo command_buffer_begin_info = vku::InitStructHelper();
        command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-None-02863");

        m_command_buffer.Reset();
        m_command_buffer.Begin(&command_buffer_begin_info);
        vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 1);
        vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
        vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);
        m_command_buffer.End();

        {
            VkPerformanceQuerySubmitInfoKHR perf_submit_info = vku::InitStructHelper();
            perf_submit_info.counterPassIndex = 0;
            VkSubmitInfo submit_info = vku::InitStructHelper(&perf_submit_info);
            submit_info.waitSemaphoreCount = 0;
            submit_info.pWaitSemaphores = NULL;
            submit_info.pWaitDstStageMask = NULL;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &m_command_buffer.handle();
            submit_info.signalSemaphoreCount = 0;
            submit_info.pSignalSemaphores = NULL;

            vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        }

        vk::QueueWaitIdle(queue);
        m_errorMonitor->VerifyFound();
    }

    vk::ReleaseProfilingLockKHR(device());
}

TEST_F(NegativeQuery, HostResetNotEnabled) {
    TEST_DESCRIPTION("Use vkResetQueryPoolEXT without enabling the feature");

    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    RETURN_IF_SKIP(Init());

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 1);

    m_errorMonitor->SetDesiredError("VUID-vkResetQueryPool-None-02665");
    vk::ResetQueryPoolEXT(device(), query_pool, 0, 1);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, HostResetFirstQuery) {
    TEST_DESCRIPTION("Bad firstQuery in vkResetQueryPoolEXT");

    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::hostQueryReset);
    RETURN_IF_SKIP(Init());

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 1);

    m_errorMonitor->SetDesiredError("VUID-vkResetQueryPool-firstQuery-09436");
    vk::ResetQueryPoolEXT(device(), query_pool.handle(), 1, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, HostResetBadRange) {
    TEST_DESCRIPTION("Bad range in vkResetQueryPoolEXT");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::hostQueryReset);
    RETURN_IF_SKIP(Init());

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 1);

    m_errorMonitor->SetDesiredError("VUID-vkResetQueryPool-firstQuery-09437");
    vk::ResetQueryPool(device(), query_pool.handle(), 0, 2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, HostResetQueryPool) {
    TEST_DESCRIPTION("Invalid queryPool in vkResetQueryPoolEXT");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::hostQueryReset);
    RETURN_IF_SKIP(Init());

    // Create and destroy a query pool.
    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 1);
    VkQueryPool bad_pool = query_pool.handle();
    query_pool.destroy();

    // Attempt to reuse the query pool handle.
    m_errorMonitor->SetDesiredError("VUID-vkResetQueryPool-queryPool-parameter");
    vk::ResetQueryPool(device(), bad_pool, 0, 1);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, HostResetDevice) {
    TEST_DESCRIPTION("Device not matching queryPool in vkResetQueryPoolEXT");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(host_query_reset_features);
    RETURN_IF_SKIP(InitState(nullptr, &host_query_reset_features));

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 1);

    // Create a second device with the feature enabled.
    vkt::QueueCreateInfoArray queue_info(m_device->Physical().queue_properties_);
    auto features = m_device->Physical().Features();

    VkDeviceCreateInfo device_create_info = vku::InitStructHelper(&host_query_reset_features);
    device_create_info.queueCreateInfoCount = queue_info.Size();
    device_create_info.pQueueCreateInfos = queue_info.Data();
    device_create_info.pEnabledFeatures = &features;
    device_create_info.enabledExtensionCount = m_device_extension_names.size();
    device_create_info.ppEnabledExtensionNames = m_device_extension_names.data();

    VkDevice second_device;
    ASSERT_EQ(VK_SUCCESS, vk::CreateDevice(Gpu(), &device_create_info, nullptr, &second_device));

    m_errorMonitor->SetDesiredError("VUID-vkResetQueryPool-queryPool-parent");
    // Run vk::ResetQueryPoolExt on the wrong device.
    vk::ResetQueryPool(second_device, query_pool.handle(), 0, 1);
    m_errorMonitor->VerifyFound();

    vk::DestroyDevice(second_device, nullptr);
}

TEST_F(NegativeQuery, CmdBufferQueryPoolDestroyed) {
    TEST_DESCRIPTION("Attempt to draw with a command buffer that is invalid due to a query pool dependency being destroyed.");
    RETURN_IF_SKIP(Init());

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 1);

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool, 0, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    // Destroy query pool dependency prior to submit to cause ERROR
    query_pool.destroy();
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, BeginQueryOnTimestampPool) {
    TEST_DESCRIPTION("Call CmdBeginQuery on a TIMESTAMP query pool.");

    RETURN_IF_SKIP(Init());

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 1);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryType-02804");
    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();

    vk::BeginCommandBuffer(m_command_buffer.handle(), &begin_info);
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 1);
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
    vk::EndCommandBuffer(m_command_buffer.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, InsideRenderPass) {
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 1);
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredError("VUID-vkCmdEndQuery-None-07007");
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);
    m_command_buffer.End();
}

TEST_F(NegativeQuery, OutsideRenderPass) {
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 1);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);

    m_errorMonitor->SetDesiredError("VUID-vkCmdEndRenderPass-None-07004");
    m_command_buffer.EndRenderPass();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, InsideRenderPassDynamicRendering) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {64, 64}};
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 1);
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
    m_command_buffer.BeginRendering(begin_rendering_info);

    m_errorMonitor->SetDesiredError("VUID-vkCmdEndQuery-None-07007");
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRendering();
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);
    m_command_buffer.End();
}

TEST_F(NegativeQuery, OutsideRenderPassDynamicRendering) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {64, 64}};
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 1);
    m_command_buffer.BeginRendering(begin_rendering_info);
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);

    m_errorMonitor->SetDesiredError("VUID-vkCmdEndRendering-None-06999");
    m_command_buffer.EndRendering();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, PoolCreate) {
    TEST_DESCRIPTION("Attempt to create a query pool for PIPELINE_STATISTICS without enabling pipeline stats for the device.");

    RETURN_IF_SKIP(Init());

    vkt::QueueCreateInfoArray queue_info(m_device->Physical().queue_properties_);

    VkDevice local_device;
    VkDeviceCreateInfo device_create_info = vku::InitStructHelper();
    auto features = m_device->Physical().Features();
    // Intentionally disable pipeline stats
    features.pipelineStatisticsQuery = VK_FALSE;
    device_create_info.queueCreateInfoCount = queue_info.Size();
    device_create_info.pQueueCreateInfos = queue_info.Data();
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = NULL;
    device_create_info.pEnabledFeatures = &features;
    device_create_info.enabledExtensionCount = m_device_extension_names.size();
    device_create_info.ppEnabledExtensionNames = m_device_extension_names.data();
    VkResult err = vk::CreateDevice(Gpu(), &device_create_info, nullptr, &local_device);
    ASSERT_EQ(VK_SUCCESS, err);

    VkQueryPoolCreateInfo qpci = vkt::QueryPool::CreateInfo(VK_QUERY_TYPE_PIPELINE_STATISTICS, 1);
    qpci.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT;
    VkQueryPool query_pool;

    m_errorMonitor->SetDesiredError("VUID-VkQueryPoolCreateInfo-queryType-00791");
    vk::CreateQueryPool(local_device, &qpci, nullptr, &query_pool);
    m_errorMonitor->VerifyFound();

    qpci.queryType = VK_QUERY_TYPE_OCCLUSION;
    qpci.queryCount = 0;
    m_errorMonitor->SetDesiredError("VUID-VkQueryPoolCreateInfo-queryCount-02763");
    vk::CreateQueryPool(local_device, &qpci, nullptr, &query_pool);
    m_errorMonitor->VerifyFound();

    vk::DestroyDevice(local_device, nullptr);
}

TEST_F(NegativeQuery, Sizes) {
    TEST_DESCRIPTION("Invalid size of using queries commands.");

    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, 128, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VkMemoryRequirements mem_reqs = {};
    vk::GetBufferMemoryRequirements(device(), buffer.handle(), &mem_reqs);
    const VkDeviceSize buffer_size = mem_reqs.size;

    const uint32_t query_pool_size = 4;
    vkt::QueryPool occlusion_query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, query_pool_size);

    m_command_buffer.Begin();

    // FirstQuery is too large
    m_errorMonitor->SetDesiredError("VUID-vkCmdResetQueryPool-firstQuery-09436");
    m_errorMonitor->SetDesiredError("VUID-vkCmdResetQueryPool-firstQuery-09437");
    vk::CmdResetQueryPool(m_command_buffer.handle(), occlusion_query_pool.handle(), query_pool_size, 1);
    m_errorMonitor->VerifyFound();

    // Sum of firstQuery and queryCount is too large
    m_errorMonitor->SetDesiredError("VUID-vkCmdResetQueryPool-firstQuery-09437");
    vk::CmdResetQueryPool(m_command_buffer.handle(), occlusion_query_pool.handle(), 1, query_pool_size);
    m_errorMonitor->VerifyFound();

    // Actually reset all queries so they can be used
    vk::CmdResetQueryPool(m_command_buffer.handle(), occlusion_query_pool.handle(), 0, query_pool_size);

    vk::CmdBeginQuery(m_command_buffer.handle(), occlusion_query_pool.handle(), 0, 0);

    // Query index to large
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndQuery-query-00810");
    vk::CmdEndQuery(m_command_buffer.handle(), occlusion_query_pool.handle(), query_pool_size);
    m_errorMonitor->VerifyFound();

    vk::CmdEndQuery(m_command_buffer.handle(), occlusion_query_pool.handle(), 0);

    // FirstQuery is too large
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-firstQuery-09436");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-firstQuery-09437");
    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), occlusion_query_pool.handle(), query_pool_size, 1, buffer.handle(), 0, 0,
                                0);
    m_errorMonitor->VerifyFound();

    // sum of firstQuery and queryCount is too large
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-firstQuery-09437");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-queryCount-09438");
    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), occlusion_query_pool.handle(), 1, query_pool_size, buffer.handle(), 0, 0,
                                0);
    m_errorMonitor->VerifyFound();

    // Offset larger than buffer size
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-dstOffset-00819");
    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), occlusion_query_pool.handle(), 0, 1, buffer.handle(), buffer_size + 4, 0,
                                0);
    m_errorMonitor->VerifyFound();

    // Buffer does not have enough storage from offset to contain result of each query
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-dstBuffer-00824");
    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), occlusion_query_pool.handle(), 0, 2, buffer.handle(), buffer_size - 4, 4,
                                0);
    m_errorMonitor->VerifyFound();

    // Query is not a timestamp type
    if (HasZeroTimestampValidBits()) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdWriteTimestamp-timestampValidBits-00829");
    }
    m_errorMonitor->SetDesiredError("VUID-vkCmdWriteTimestamp-queryPool-01416");
    vk::CmdWriteTimestamp(m_command_buffer.handle(), VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, occlusion_query_pool.handle(), 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();

    const size_t out_data_size = 16;
    uint8_t data[out_data_size];

    // FirstQuery is too large
    m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-firstQuery-09436");
    m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-firstQuery-09437");
    vk::GetQueryPoolResults(device(), occlusion_query_pool.handle(), query_pool_size, 1, out_data_size, &data, 4, 0);
    m_errorMonitor->VerifyFound();

    // Sum of firstQuery and queryCount is too large
    m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-firstQuery-09437");
    vk::GetQueryPoolResults(device(), occlusion_query_pool.handle(), 1, query_pool_size, out_data_size, &data, 4, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, PreciseBit) {
    TEST_DESCRIPTION("Check for correct Query Precise Bit circumstances.");
    AddRequiredFeature(vkt::Feature::pipelineStatisticsQuery);
    AddRequiredFeature(vkt::Feature::occlusionQueryPrecise);
    RETURN_IF_SKIP(Init());

    std::vector<const char *> device_extension_names;
    auto features = m_device->Physical().Features();

    // Test for precise bit when query type is not OCCLUSION
    if (features.occlusionQueryPrecise) {
        vkt::Event event(*m_device);

        VkQueryPoolCreateInfo query_pool_create_info = vkt::QueryPool::CreateInfo(VK_QUERY_TYPE_PIPELINE_STATISTICS, 3);
        query_pool_create_info.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
                                                    VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
                                                    VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT;
        vkt::QueryPool query_pool(*m_device, query_pool_create_info);

        m_command_buffer.Begin();
        vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, query_pool_create_info.queryCount);
        m_command_buffer.End();

        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();

        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryType-00800");

        m_command_buffer.Begin();
        vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, VK_QUERY_CONTROL_PRECISE_BIT);
        m_errorMonitor->VerifyFound();
        // vk::CmdBeginQuery(m_command_buffer.handle(), query_pool, 0, VK_QUERY_CONTROL_PRECISE_BIT);
        m_command_buffer.End();

        const size_t out_data_size = 64;
        uint8_t data[out_data_size];
        // The dataSize is too small to return the data
        m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-dataSize-00817");
        vk::GetQueryPoolResults(device(), query_pool.handle(), 0, 3, 8, &data, 12, 0);
        m_errorMonitor->VerifyFound();
    }

    // Test for precise bit when precise feature is not available
    features.occlusionQueryPrecise = false;
    vkt::Device test_device(Gpu(), device_extension_names, &features);

    VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
    pool_create_info.queueFamilyIndex = test_device.graphics_queue_node_index_;

    VkCommandPool command_pool;
    vk::CreateCommandPool(test_device.handle(), &pool_create_info, nullptr, &command_pool);

    VkCommandBufferAllocateInfo cmd = vku::InitStructHelper();
    cmd.commandPool = command_pool;
    cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd.commandBufferCount = 1;

    VkCommandBuffer cmd_buffer;
    VkResult err = vk::AllocateCommandBuffers(test_device.handle(), &cmd, &cmd_buffer);
    ASSERT_EQ(VK_SUCCESS, err);

    VkCommandBuffer cmd_buffer2;
    err = vk::AllocateCommandBuffers(test_device.handle(), &cmd, &cmd_buffer2);
    ASSERT_EQ(VK_SUCCESS, err);

    VkEvent event;
    VkEventCreateInfo event_create_info = vku::InitStructHelper();
    vk::CreateEvent(test_device.handle(), &event_create_info, nullptr, &event);

    VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                           VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};

    vkt::QueryPool query_pool(test_device, VK_QUERY_TYPE_OCCLUSION, 2);

    vk::BeginCommandBuffer(cmd_buffer2, &begin_info);
    vk::CmdResetQueryPool(cmd_buffer2, query_pool.handle(), 0, 2);
    vk::EndCommandBuffer(cmd_buffer2);

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd_buffer2;
    vk::QueueSubmit(test_device.QueuesWithGraphicsCapability().front()->handle(), 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(test_device.QueuesWithGraphicsCapability().front()->handle());

    vk::BeginCommandBuffer(cmd_buffer, &begin_info);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryType-00800");
    vk::CmdBeginQuery(cmd_buffer, query_pool.handle(), 0, VK_QUERY_CONTROL_PRECISE_BIT);
    m_errorMonitor->VerifyFound();
    vk::EndCommandBuffer(cmd_buffer);

    const size_t out_data_size = 16;
    uint8_t data[out_data_size];
    // The dataSize is too small to return the data
    m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-dataSize-00817");
    vk::GetQueryPoolResults(test_device.handle(), query_pool.handle(), 0, 2, 8, &data, out_data_size / 2, 0);
    m_errorMonitor->VerifyFound();

    vk::DestroyEvent(test_device.handle(), event, nullptr);
    vk::DestroyCommandPool(test_device.handle(), command_pool, nullptr);
}

TEST_F(NegativeQuery, PoolPartialTimestamp) {
    TEST_DESCRIPTION("Request partial result on timestamp query.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    if (HasZeroTimestampValidBits()) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    vkt::Buffer buffer(*m_device, 128, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 1);

    // Use setup as a positive test...
    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 1);
    vk::CmdWriteTimestamp(m_command_buffer.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool.handle(), 0);

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-queryType-09439");
    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool.handle(), 0, 1, buffer.handle(), 0, 8,
                                VK_QUERY_RESULT_PARTIAL_BIT);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();

    // Submit cmd buffer and wait for it.
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffer.handle();
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_default_queue->Wait();

    // Attempt to obtain partial results.
    m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-queryType-09439");
    uint32_t data_space[16];
    m_errorMonitor->SetUnexpectedError("Cannot get query results on queryPool");
    vk::GetQueryPoolResults(m_device->handle(), query_pool, 0, 1, sizeof(data_space), &data_space, sizeof(uint32_t),
                            VK_QUERY_RESULT_PARTIAL_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, PerformanceQueryIntel) {
    TEST_DESCRIPTION("Call CmdCopyQueryPoolResults for an Intel performance query.");

    AddRequiredExtensions(VK_INTEL_PERFORMANCE_QUERY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkInitializePerformanceApiInfoINTEL performance_api_info_intel = vku::InitStructHelper();
    vk::InitializePerformanceApiINTEL(device(), &performance_api_info_intel);

    vkt::Buffer buffer(*m_device, 128, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL, 1);

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-queryType-02734");
    m_command_buffer.Begin();
    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool.handle(), 0, 1, buffer.handle(), 0, 8, 0);
    m_command_buffer.End();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, PoolInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use query pool.");

    RETURN_IF_SKIP(Init());
    if (HasZeroTimestampValidBits()) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.";
    }
    InitRenderTarget();

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_ci.queryCount = 1;
    vk::CreateQueryPool(device(), &query_pool_ci, nullptr, &query_pool);

    m_command_buffer.Begin();
    // Use query pool to create binding with cmd buffer
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool, 0, 1);
    vk::CmdWriteTimestamp(m_command_buffer.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool, 0);
    m_command_buffer.End();

    // Submit cmd buffer and then destroy query pool while in-flight
    m_default_queue->Submit(m_command_buffer);

    m_errorMonitor->SetDesiredError("VUID-vkDestroyQueryPool-queryPool-00793");
    vk::DestroyQueryPool(m_device->handle(), query_pool, NULL);
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
    // Now that cmd buffer done we can safely destroy query_pool
    m_errorMonitor->SetUnexpectedError("If queryPool is not VK_NULL_HANDLE, queryPool must be a valid VkQueryPool handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove QueryPool obj");
    vk::DestroyQueryPool(m_device->handle(), query_pool, NULL);
}

TEST_F(NegativeQuery, WriteTimeStamp) {
    TEST_DESCRIPTION("Test for invalid query slot in query pool.");

    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    if (HasZeroTimestampValidBits()) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 1);

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdWriteTimestamp-query-04904");
    vk::CmdWriteTimestamp(m_command_buffer.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool.handle(), 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeQuery, CmdEndQueryIndexedEXTIndex) {
    TEST_DESCRIPTION("Test InvalidCmdEndQueryIndexedEXT with invalid index");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceTransformFeedbackPropertiesEXT transform_feedback_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(transform_feedback_properties);
    if (transform_feedback_properties.maxTransformFeedbackStreams < 1) {
        GTEST_SKIP() << "maxTransformFeedbackStreams < 1";
    }

    vkt::QueryPool tf_query_pool(*m_device, VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT, 1);
    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    m_command_buffer.Begin();
    vk::CmdBeginQueryIndexedEXT(m_command_buffer.handle(), tf_query_pool.handle(), 0, 0, 0);
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndQueryIndexedEXT-queryType-06694");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndQueryIndexedEXT-queryType-06696");
    vk::CmdEndQueryIndexedEXT(m_command_buffer.handle(), tf_query_pool.handle(), 0,
                              transform_feedback_properties.maxTransformFeedbackStreams);

    vk::CmdBeginQueryIndexedEXT(m_command_buffer.handle(), query_pool.handle(), 0, 0, 0);
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndQueryIndexedEXT-queryType-06695");
    vk::CmdEndQueryIndexedEXT(m_command_buffer.handle(), query_pool.handle(), 0, 1);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEndQueryIndexedEXT-None-02342");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndQueryIndexedEXT-query-02343");
    vk::CmdEndQueryIndexedEXT(m_command_buffer.handle(), query_pool.handle(), 1, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, CmdEndQueryIndexedEXTPrimitiveGenerated) {
    TEST_DESCRIPTION("Test InvalidCmdEndQueryIndexedEXT with invalid index");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::primitivesGeneratedQueryWithNonZeroStreams);
    AddRequiredFeature(vkt::Feature::primitivesGeneratedQuery);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceTransformFeedbackPropertiesEXT transform_feedback_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(transform_feedback_properties);

    vkt::QueryPool tf_query_pool(*m_device, VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT, 1);
    vkt::QueryPool pg_query_pool(*m_device, VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT, 1);
    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQueryIndexedEXT-queryType-02339");
    vk::CmdBeginQueryIndexedEXT(m_command_buffer.handle(), tf_query_pool.handle(), 0, 0,
                                transform_feedback_properties.maxTransformFeedbackStreams);
    m_errorMonitor->VerifyFound();

    vk::CmdBeginQueryIndexedEXT(m_command_buffer.handle(), tf_query_pool.handle(), 0, 0, 0);

    m_errorMonitor->SetDesiredError("VUID-vkCmdEndQueryIndexedEXT-queryType-06696");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndQueryIndexedEXT-queryType-06694");
    vk::CmdEndQueryIndexedEXT(m_command_buffer.handle(), tf_query_pool.handle(), 0,
                              transform_feedback_properties.maxTransformFeedbackStreams);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQueryIndexedEXT-queryType-06690");
    vk::CmdBeginQueryIndexedEXT(m_command_buffer.handle(), pg_query_pool.handle(), 0, 0,
                                transform_feedback_properties.maxTransformFeedbackStreams);
    m_errorMonitor->VerifyFound();

    vk::CmdBeginQueryIndexedEXT(m_command_buffer.handle(), query_pool.handle(), 0, 0, 0);
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndQueryIndexedEXT-queryType-06695");
    vk::CmdEndQueryIndexedEXT(m_command_buffer.handle(), query_pool.handle(), 0, 1);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, TransformFeedbackStream) {
    TEST_DESCRIPTION(
        "Call CmdBeginQuery with query type transform feedback stream when transformFeedbackQueries is not supported.");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceTransformFeedbackPropertiesEXT transform_feedback_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(transform_feedback_props);
    if (transform_feedback_props.transformFeedbackQueries) {
        GTEST_SKIP() << "Transform feedback queries are supported";
    }

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT, 1);

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 1);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryType-02328");
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeQuery, GetResultsFlags) {
    TEST_DESCRIPTION("Test GetQueryPoolResults with invalid pData and stride");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    const size_t out_data_size = 16;
    uint8_t data[out_data_size];

    VkQueryResultFlags flags = VK_QUERY_RESULT_WITH_STATUS_BIT_KHR | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT;

    m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-flags-09443");
    vk::GetQueryPoolResults(device(), query_pool.handle(), 0, 1, out_data_size, data + 1, 4, flags);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, GetResultsStride) {
    TEST_DESCRIPTION("Test GetQueryPoolResults with invalid queryCount and stride");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 2);
    uint8_t data[8];

    m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-queryCount-09438");
    vk::GetQueryPoolResults(device(), query_pool.handle(), 0, 2, 8, data, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, ResultStatusOnly) {
    TEST_DESCRIPTION("Request result status only query result.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR, 1);
    if (!query_pool.initialized()) {
        GTEST_SKIP() << "Required query not supported";
    }

    const size_t out_data_size = 16;
    uint8_t data[out_data_size];
    m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-queryType-09442");
    vk::GetQueryPoolResults(device(), query_pool.handle(), 0, 1, out_data_size, &data, sizeof(uint32_t), 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, DestroyActiveQueryPool) {
    TEST_DESCRIPTION("Destroy query pool after GetQueryPoolResults() without VK_QUERY_RESULT_PARTIAL_BIT returns VK_SUCCESS");

    RETURN_IF_SKIP(Init());
    if (HasZeroTimestampValidBits()) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.";
    }

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;

    VkQueryPool query_pool;
    vk::CreateQueryPool(device(), &query_pool_create_info, nullptr, &query_pool);

    VkCommandBufferBeginInfo cmd_begin = vku::InitStructHelper();
    cmd_begin.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    m_command_buffer.Begin(&cmd_begin);
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool, 0, 1);
    vk::CmdWriteTimestamp(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, query_pool, 0);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);

    const size_t out_data_size = 16;
    uint8_t data[out_data_size];
    VkResult res;
    do {
        res = vk::GetQueryPoolResults(device(), query_pool, 0, 1, out_data_size, &data, 4, 0);
    } while (res != VK_SUCCESS);

    // Submit the command buffer again, making query pool in use and invalid to destroy
    m_default_queue->Submit(m_command_buffer);

    m_errorMonitor->SetDesiredError("VUID-vkDestroyQueryPool-queryPool-00793");
    vk::DestroyQueryPool(m_device->handle(), query_pool, nullptr);
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
    vk::DestroyQueryPool(m_device->handle(), query_pool, nullptr);
}

TEST_F(NegativeQuery, MultiviewBeginQuery) {
    TEST_DESCRIPTION("Test CmdBeginQuery in subpass with multiview");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());

    VkAttachmentDescription attach = {};
    attach.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach.samples = VK_SAMPLE_COUNT_1_BIT;
    attach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference color_att = {};
    color_att.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_att;

    uint32_t viewMasks[] = {0x3u};
    uint32_t correlationMasks[] = {0x1u};
    VkRenderPassMultiviewCreateInfo rpmv_ci = vku::InitStructHelper();
    rpmv_ci.subpassCount = 1;
    rpmv_ci.pViewMasks = viewMasks;
    rpmv_ci.correlationMaskCount = 1;
    rpmv_ci.pCorrelationMasks = correlationMasks;

    VkRenderPassCreateInfo rp_ci = vku::InitStructHelper(&rpmv_ci);
    rp_ci.attachmentCount = 1;
    rp_ci.pAttachments = &attach;
    rp_ci.subpassCount = 1;
    rp_ci.pSubpasses = &subpass;

    vkt::RenderPass render_pass(*m_device, rp_ci);

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_ci.extent.width = 64;
    image_ci.extent.height = 64;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 4;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, 4);
    VkImageView image_view_handle = image_view.handle();

    vkt::Framebuffer framebuffer(*m_device, render_pass.handle(), 1, &image_view_handle, 64, 64);

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 2);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass.handle(), framebuffer.handle(), 64, 64);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-query-00808");
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 1, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeQuery, PipelineStatisticsQuery) {
    TEST_DESCRIPTION("Test unsupported pipeline statistics queries");

    AddRequiredFeature(vkt::Feature::pipelineStatisticsQuery);

    RETURN_IF_SKIP(Init());

    const std::optional<uint32_t> decode_queue_family_index = m_device->QueueFamily(VK_QUEUE_VIDEO_DECODE_BIT_KHR);
    const std::optional<uint32_t> compute_queue_family_index = m_device->ComputeOnlyQueueFamily();
    if (!decode_queue_family_index && !compute_queue_family_index) {
        GTEST_SKIP() << "required queue families not found";
    }

    if (decode_queue_family_index) {
        vkt::CommandPool command_pool(*m_device, decode_queue_family_index.value());

        vkt::CommandBuffer command_buffer(*m_device, command_pool);
        command_buffer.Begin();

        VkQueryPoolCreateInfo qpci = vkt::QueryPool::CreateInfo(VK_QUERY_TYPE_PIPELINE_STATISTICS, 1);
        qpci.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;
        vkt::QueryPool query_pool(*m_device, qpci);

        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryType-00805");
        vk::CmdBeginQuery(command_buffer.handle(), query_pool.handle(), 0, 0);
        m_errorMonitor->VerifyFound();

        command_buffer.End();
    }

    if (compute_queue_family_index) {
        vkt::CommandPool command_pool(*m_device, compute_queue_family_index.value());

        vkt::CommandBuffer command_buffer(*m_device, command_pool);
        command_buffer.Begin();

        VkQueryPoolCreateInfo qpci = vkt::QueryPool::CreateInfo(VK_QUERY_TYPE_PIPELINE_STATISTICS, 1);
        qpci.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT;
        vkt::QueryPool query_pool(*m_device, qpci);

        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryType-00804");
        vk::CmdBeginQuery(command_buffer.handle(), query_pool.handle(), 0, 0);
        m_errorMonitor->VerifyFound();

        command_buffer.End();
    }
}

TEST_F(NegativeQuery, TestGetQueryPoolResultsDataAndStride) {
    TEST_DESCRIPTION("Test pData and stride multiple in GetQueryPoolResults");

    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 1);

    m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-flags-02828");
    const size_t out_data_size = 16;
    uint8_t data[out_data_size];
    vk::GetQueryPoolResults(device(), query_pool.handle(), 0, 1, out_data_size, &data, 3, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, PrimitivesGenerated) {
    TEST_DESCRIPTION("Test unsupported primitives generated queries");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::primitivesGeneratedQuery);

    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceTransformFeedbackPropertiesEXT transform_feedback_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(transform_feedback_properties);

    const std::optional<uint32_t> compute_queue_family_index = m_device->ComputeOnlyQueueFamily();
    if (!compute_queue_family_index) {
        GTEST_SKIP() << "required queue family not found, skipping test";
    }
    vkt::CommandPool command_pool(*m_device, compute_queue_family_index.value());

    vkt::CommandBuffer command_buffer(*m_device, command_pool);
    command_buffer.Begin();

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT, 1);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryType-06687");
    vk::CmdBeginQuery(command_buffer.handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQueryIndexedEXT-queryType-06689");
    vk::CmdBeginQueryIndexedEXT(command_buffer.handle(), query_pool.handle(), 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQueryIndexedEXT-queryType-06690");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQueryIndexedEXT-queryType-06691");
    vk::CmdBeginQueryIndexedEXT(m_command_buffer.handle(), query_pool.handle(), 0, 0,
                                transform_feedback_properties.maxTransformFeedbackStreams);
    m_errorMonitor->VerifyFound();

    vkt::QueryPool occlusion_query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQueryIndexedEXT-queryType-06692");
    vk::CmdBeginQueryIndexedEXT(m_command_buffer.handle(), occlusion_query_pool.handle(), 0, 0, 1);
    m_errorMonitor->VerifyFound();

    vk::CmdBeginQueryIndexedEXT(m_command_buffer.handle(), query_pool.handle(), 0, 0, 0);
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndQueryIndexedEXT-queryType-06696");
    vk::CmdEndQueryIndexedEXT(m_command_buffer.handle(), query_pool.handle(), 0, 1);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, PrimitivesGeneratedFeature) {
    TEST_DESCRIPTION("Test missing primitives generated query feature");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);

    RETURN_IF_SKIP(Init());

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT, 1);
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryType-06688");
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQueryIndexedEXT-queryType-06693");
    vk::CmdBeginQueryIndexedEXT(m_command_buffer.handle(), query_pool.handle(), 0, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, PrimitivesGeneratedDiscardEnabled) {
    TEST_DESCRIPTION("Test missing primitivesGeneratedQueryWithRasterizerDiscard feature.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::primitivesGeneratedQuery);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkPipelineRasterizationStateCreateInfo rs_ci = vku::InitStructHelper();
    rs_ci.lineWidth = 1.0f;
    rs_ci.rasterizerDiscardEnable = VK_TRUE;

    CreatePipelineHelper pipe(*this);
    pipe.rs_state_ci_ = rs_ci;
    // Rasterization discard enable prohibits fragment shader.
    pipe.VertexShaderOnly();
    pipe.CreateGraphicsPipeline();

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT, 1);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-primitivesGeneratedQueryWithRasterizerDiscard-06708");

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, PrimitivesGeneratedStreams) {
    TEST_DESCRIPTION("Test missing primitivesGeneratedQueryWithNonZeroStreams feature.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    AddRequiredFeature(vkt::Feature::geometryStreams);
    AddRequiredFeature(vkt::Feature::primitivesGeneratedQuery);

    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceTransformFeedbackPropertiesEXT xfb_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(xfb_props);
    if (!xfb_props.transformFeedbackRasterizationStreamSelect) {
        GTEST_SKIP() << "VkPhysicalDeviceTransformFeedbackFeaturesEXT::transformFeedbackRasterizationStreamSelect is VK_FALSE";
    }
    InitRenderTarget();

    VkPipelineRasterizationStateStreamCreateInfoEXT rasterization_streams = vku::InitStructHelper();
    rasterization_streams.rasterizationStream = 1;

    CreatePipelineHelper pipe(*this);
    pipe.rs_state_ci_.pNext = &rasterization_streams;
    pipe.CreateGraphicsPipeline();

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT, 1);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-primitivesGeneratedQueryWithNonZeroStreams-06709");

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, CommandBufferMissingOcclusion) {
    TEST_DESCRIPTION(
        "Test executing secondary command buffer without VkCommandBufferInheritanceInfo::occlusionQueryEnable enabled while "
        "occlusion query is active.");
    AddRequiredFeature(vkt::Feature::inheritedQueries);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo cbii = vku::InitStructHelper();
    cbii.renderPass = m_renderPass;
    cbii.framebuffer = Framebuffer();
    cbii.occlusionQueryEnable = VK_FALSE;  // Invalid

    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.pInheritanceInfo = &cbii;

    VkCommandBuffer secondary_handle = secondary.handle();
    vk::BeginCommandBuffer(secondary_handle, &cbbi);
    vk::EndCommandBuffer(secondary_handle);

    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-commandBuffer-00102");
    m_command_buffer.Begin();
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary_handle);
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);
    m_command_buffer.End();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, CommandBufferInheritanceFlags) {
    TEST_DESCRIPTION("Test executing secondary command buffer with bad VkCommandBufferInheritanceInfo::queryFlags.");
    AddRequiredFeature(vkt::Feature::occlusionQueryPrecise);
    AddRequiredFeature(vkt::Feature::inheritedQueries);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo cbii = vku::InitStructHelper();
    cbii.renderPass = m_renderPass;
    cbii.framebuffer = Framebuffer();
    cbii.occlusionQueryEnable = VK_TRUE;
    cbii.queryFlags = 0;

    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.pInheritanceInfo = &cbii;

    VkCommandBuffer secondary_handle = secondary.handle();
    vk::BeginCommandBuffer(secondary_handle, &cbbi);
    vk::EndCommandBuffer(secondary_handle);

    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-commandBuffer-00103");
    m_command_buffer.Begin();
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, VK_QUERY_CONTROL_PRECISE_BIT);
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary_handle);
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);
    m_command_buffer.End();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, MeshShaderQueries) {
    TEST_DESCRIPTION("Invalid usage without meshShaderQueries enabled");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::pipelineStatisticsQuery);
    RETURN_IF_SKIP(Init());

    VkQueryPool pool = VK_NULL_HANDLE;

    VkQueryPoolCreateInfo query_pool_info = vku::InitStructHelper();
    query_pool_info.queryType = VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT;
    query_pool_info.flags = 0;
    query_pool_info.queryCount = 1;
    query_pool_info.pipelineStatistics = 0;
    m_errorMonitor->SetDesiredError("VUID-VkQueryPoolCreateInfo-meshShaderQueries-07068");
    vk::CreateQueryPool(m_device->handle(), &query_pool_info, nullptr, &pool);
    m_errorMonitor->VerifyFound();

    query_pool_info.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
    query_pool_info.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkQueryPoolCreateInfo-meshShaderQueries-07069");
    vk::CreateQueryPool(m_device->handle(), &query_pool_info, nullptr, &pool);
    m_errorMonitor->VerifyFound();

    query_pool_info.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkQueryPoolCreateInfo-meshShaderQueries-07069");
    vk::CreateQueryPool(m_device->handle(), &query_pool_info, nullptr, &pool);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, WriteTimestampWithoutQueryPool) {
    TEST_DESCRIPTION("call vkCmdWriteTimestamp(2) with queryPool being invalid.");

    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    if (HasZeroTimestampValidBits()) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    VkQueryPool bad_query_pool = CastFromUint64<VkQueryPool>(0xFFFFEEEE);

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdWriteTimestamp-queryPool-parameter");
    vk::CmdWriteTimestamp(m_command_buffer.handle(), VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, bad_query_pool, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdWriteTimestamp2-queryPool-parameter");
    vk::CmdWriteTimestamp2KHR(m_command_buffer.handle(), VK_PIPELINE_STAGE_2_NONE, bad_query_pool, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeQuery, DestroyWithoutQueryPool) {
    TEST_DESCRIPTION("call vkDestryQueryPool with queryPool being invalid.");
    RETURN_IF_SKIP(Init());
    VkQueryPool bad_query_pool = CastFromUint64<VkQueryPool>(0xFFFFEEEE);
    m_errorMonitor->SetDesiredError("VUID-vkDestroyQueryPool-queryPool-parameter");
    vk::DestroyQueryPool(device(), bad_query_pool, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, GetQueryPoolResultsWithoutQueryPool) {
    TEST_DESCRIPTION("call vkGetQueryPoolResults with queryPool being invalid.");
    RETURN_IF_SKIP(Init());
    VkQueryPool bad_query_pool = CastFromUint64<VkQueryPool>(0xFFFFEEEE);
    const size_t out_data_size = 16;
    uint8_t data[out_data_size];
    m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-queryPool-parameter");
    vk::GetQueryPoolResults(device(), bad_query_pool, 0, 1, out_data_size, &data, 4, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, CmdEndQueryWithoutQueryPool) {
    TEST_DESCRIPTION("call vkCmdEndQuery with queryPool being invalid.");
    RETURN_IF_SKIP(Init());

    VkQueryPool bad_query_pool = CastFromUint64<VkQueryPool>(0xFFFFEEEE);
    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    m_command_buffer.Begin();
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);

    m_errorMonitor->SetDesiredError("VUID-vkCmdEndQuery-queryPool-parameter");
    vk::CmdEndQuery(m_command_buffer.handle(), bad_query_pool, 0);
    m_errorMonitor->VerifyFound();

    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);
    m_command_buffer.End();
}

TEST_F(NegativeQuery, CmdCopyQueryPoolResultsWithoutQueryPool) {
    TEST_DESCRIPTION("call vkCmdCopyQueryPoolResults with queryPool being invalid.");
    RETURN_IF_SKIP(Init());

    VkQueryPool bad_query_pool = CastFromUint64<VkQueryPool>(0xFFFFEEEE);
    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    m_command_buffer.Begin();
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-queryPool-parameter");
    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), bad_query_pool, 0, 1, buffer.handle(), 0, 0, VK_QUERY_RESULT_WAIT_BIT);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeQuery, CmdResetQueryPoolWithoutQueryPool) {
    TEST_DESCRIPTION("call vkCmdResetQueryPool with queryPool being invalid.");
    RETURN_IF_SKIP(Init());
    VkQueryPool bad_query_pool = CastFromUint64<VkQueryPool>(0xFFFFEEEE);
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdResetQueryPool-queryPool-parameter");
    vk::CmdResetQueryPool(m_command_buffer.handle(), bad_query_pool, 0, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeQuery, ResetQueryPoolWithoutQueryPool) {
    TEST_DESCRIPTION("call vkResetQueryPool with queryPool being invalid.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());
    VkQueryPool bad_query_pool = CastFromUint64<VkQueryPool>(0xFFFFEEEE);
    m_errorMonitor->SetDesiredError("VUID-vkResetQueryPool-queryPool-parameter");
    vk::ResetQueryPool(device(), bad_query_pool, 0, 1);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, ActiveEndQuery) {
    TEST_DESCRIPTION("Check all queries for vkCmdEndQuery are active");
    RETURN_IF_SKIP(Init());

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 1);
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndQuery-None-01923");
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeQuery, ActiveCmdResetQueryPool) {
    TEST_DESCRIPTION("Check all queries for vkCmdResetQueryPool are not active");
    RETURN_IF_SKIP(Init());

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 1);
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->SetDesiredError("VUID-vkCmdResetQueryPool-None-02841");
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 1);
    m_errorMonitor->VerifyFound();
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);
    m_command_buffer.End();
}

TEST_F(NegativeQuery, ActiveCmdCopyQueryPoolResults) {
    TEST_DESCRIPTION("Check all queries for vkCmdCopyQueryPoolResults are not active");
    RETURN_IF_SKIP(Init());

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    vkt::Buffer buffer(*m_device, 128, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 1);
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-None-07429");
    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool.handle(), 0, 1, buffer.handle(), 0, 0,
                                VK_QUERY_RESULT_WAIT_BIT);
    m_errorMonitor->VerifyFound();
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);
    m_command_buffer.End();
}

TEST_F(NegativeQuery, CmdExecuteCommandsActiveQueries) {
    TEST_DESCRIPTION("Check query types when calling vkCmdExecuteCommands");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::inheritedQueries);
    AddRequiredFeature(vkt::Feature::primitivesGeneratedQuery);
    RETURN_IF_SKIP(Init());

    vkt::CommandPool pool(*m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer secondary(*m_device, pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT, 1);

    VkCommandBufferInheritanceInfo cmd_buf_hinfo = vku::InitStructHelper();
    VkCommandBufferBeginInfo cmd_buf_info = vku::InitStructHelper();
    cmd_buf_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cmd_buf_info.pInheritanceInfo = &cmd_buf_hinfo;

    vk::BeginCommandBuffer(secondary.handle(), &cmd_buf_info);
    vk::EndCommandBuffer(secondary.handle());

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 1);
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-commandBuffer-07594");
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1u, &secondary.handle());
    m_errorMonitor->VerifyFound();
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);
    m_command_buffer.End();
}

TEST_F(NegativeQuery, CmdExecuteBeginActiveQuery) {
    TEST_DESCRIPTION("Begin a query in secondary command buffer that is already active in primary command buffer");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::inheritedQueries);
    RETURN_IF_SKIP(Init());

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 2);

    VkCommandBufferInheritanceInfo cbii = vku::InitStructHelper();
    cbii.renderPass = m_renderPass;
    cbii.occlusionQueryEnable = VK_TRUE;

    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.pInheritanceInfo = &cbii;

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin(&cbbi);
    vk::CmdBeginQuery(secondary.handle(), query_pool.handle(), 1u, 0u);
    vk::CmdEndQuery(secondary.handle(), query_pool.handle(), 1u);
    secondary.End();

    m_command_buffer.Begin();
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0u, 0u);
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-pCommandBuffers-00105");
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1u, &secondary.handle());
    m_errorMonitor->VerifyFound();
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0u);
    m_command_buffer.End();
}

TEST_F(NegativeQuery, PerformanceQueryReset) {
    TEST_DESCRIPTION("Invalid performance query pool reset");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::performanceCounterQueryPools);
    RETURN_IF_SKIP(Init());

    uint32_t counterCount = 0u;
    vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(m_device->Physical(), m_device->graphics_queue_node_index_,
                                                                      &counterCount, nullptr, nullptr);
    std::vector<VkPerformanceCounterKHR> counters(counterCount, vku::InitStruct<VkPerformanceCounterKHR>());
    std::vector<VkPerformanceCounterDescriptionKHR> counterDescriptions(counterCount,
                                                                        vku::InitStruct<VkPerformanceCounterDescriptionKHR>());
    vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(m_device->Physical(), m_device->graphics_queue_node_index_,
                                                                      &counterCount, counters.data(), counterDescriptions.data());

    uint32_t enabledCounter = counterCount;
    for (uint32_t i = 0; i < counterCount; ++i) {
        if (counters[i].scope == VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_KHR) {
            enabledCounter = i;
            break;
        }
    }
    if (enabledCounter == counterCount) {
        GTEST_SKIP() << "No counter with scope VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_KHR found";
    }

    auto query_pool_performance_ci = vku::InitStruct<VkQueryPoolPerformanceCreateInfoKHR>();
    query_pool_performance_ci.queueFamilyIndex = m_device->graphics_queue_node_index_;
    query_pool_performance_ci.counterIndexCount = 1u;
    query_pool_performance_ci.pCounterIndices = &enabledCounter;

    uint32_t num_passes = 0u;
    vk::GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(m_device->Physical(), &query_pool_performance_ci, &num_passes);

    auto query_pool_ci = vku::InitStruct<VkQueryPoolCreateInfo>(&query_pool_performance_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1u;

    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    auto acquire_profiling_lock_info = vku::InitStruct<VkAcquireProfilingLockInfoKHR>();
    acquire_profiling_lock_info.timeout = std::numeric_limits<uint64_t>::max();
    vk::AcquireProfilingLockKHR(*m_device, &acquire_profiling_lock_info);

    {
        m_command_buffer.Begin();
        vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0u, 1u);
        m_command_buffer.End();

        m_default_queue->Submit(m_command_buffer);
        m_device->Wait();
    }

    vkt::CommandBuffer command_buffer(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    command_buffer.Begin();
    vk::CmdBeginQuery(command_buffer.handle(), query_pool, 0u, 0u);
    vk::CmdEndQuery(command_buffer.handle(), query_pool, 0u);
    command_buffer.End();

    for (uint32_t i = 0; i < 2; ++i) {
        m_command_buffer.Begin();
        if (i == 0) {
            vk::CmdBeginQuery(m_command_buffer.handle(), query_pool, 0u, 0u);
            vk::CmdEndQuery(m_command_buffer.handle(), query_pool, 0u);
        } else {
            vk::CmdExecuteCommands(m_command_buffer.handle(), 1u, &command_buffer.handle());
        }
        vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool, 0u, 1u);
        m_command_buffer.End();

        auto performance_query_submit_info = vku::InitStruct<VkPerformanceQuerySubmitInfoKHR>();
        performance_query_submit_info.counterPassIndex = 0u;

        auto submit_info = vku::InitStruct<VkSubmitInfo>(&performance_query_submit_info);
        submit_info.commandBufferCount = 1u;
        submit_info.pCommandBuffers = &m_command_buffer.handle();
        m_errorMonitor->SetDesiredError("VUID-vkCmdResetQueryPool-firstQuery-02862");
        vk::QueueSubmit(m_default_queue->handle(), 1u, &submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    vk::ReleaseProfilingLockKHR(*m_device);
}

TEST_F(NegativeQuery, GetQueryPoolResultsWithoutReset) {
    TEST_DESCRIPTION("Get query pool results without ever resetting the query");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    vkt::Buffer buffer(*m_device, 128, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    uint32_t data = 0u;
    m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-None-09401");
    vk::GetQueryPoolResults(*m_device, query_pool.handle(), 0u, 1u, sizeof(uint32_t), &data, sizeof(uint32_t), 0u);
    m_errorMonitor->VerifyFound();

    m_command_buffer.Begin();
    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool.handle(), 0u, 1u, buffer.handle(), 0u, sizeof(uint32_t), 0u);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-None-09402");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeQuery, InvalidMeshQueryAtDraw) {
    TEST_DESCRIPTION("Draw with vertex shader with mesh query active");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::meshShaderQueries);

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT, 1);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0u, 0u);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-stage-07073");
    vk::CmdDraw(m_command_buffer.handle(), 3u, 1u, 0u, 0u);
    m_errorMonitor->VerifyFound();
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0u);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeQuery, PipelineStatisticsQueryWithSecondaryCmdBuffer) {
    TEST_DESCRIPTION("Use a pipeline statistics query in secondary command buffer");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::inheritedQueries);
    AddRequiredFeature(vkt::Feature::pipelineStatisticsQuery);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkQueryPoolCreateInfo qpci = vkt::QueryPool::CreateInfo(VK_QUERY_TYPE_PIPELINE_STATISTICS, 1);
    qpci.pipelineStatistics =
        VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT | VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT;
    vkt::QueryPool query_pool(*m_device, qpci);

    VkCommandBufferInheritanceInfo cbii = vku::InitStructHelper();
    cbii.renderPass = m_renderPass;
    cbii.framebuffer = Framebuffer();
    cbii.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT;

    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.pInheritanceInfo = &cbii;

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin(&cbbi);
    secondary.End();

    m_command_buffer.Begin();
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0u, 0u);
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-commandBuffer-00104");
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1u, &secondary.handle());
    m_errorMonitor->VerifyFound();
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0u);
    m_command_buffer.End();
}

TEST_F(NegativeQuery, PipelineStatisticsZero) {
    TEST_DESCRIPTION("Use a pipeline statistics query in secondary command buffer");
    AddRequiredFeature(vkt::Feature::pipelineStatisticsQuery);
    RETURN_IF_SKIP(Init());

    VkQueryPoolCreateInfo qpci = vkt::QueryPool::CreateInfo(VK_QUERY_TYPE_PIPELINE_STATISTICS, 1);
    qpci.pipelineStatistics = 0;
    m_errorMonitor->SetDesiredError("VUID-VkQueryPoolCreateInfo-queryType-09534");
    vkt::QueryPool query_pool(*m_device, qpci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, WriteTimestampInsideRenderPass) {
    TEST_DESCRIPTION("Call vkCmdWriteTimestamp() inside an active render pass instance");

    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 2;
    vkt::QueryPool query_pool(*m_device, query_pool_create_info);

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    uint32_t viewMasks[] = {0x3u};
    uint32_t correlationMasks[] = {0x1u};
    VkRenderPassMultiviewCreateInfo rpmv_ci = vku::InitStructHelper();
    rpmv_ci.subpassCount = 1;
    rpmv_ci.pViewMasks = viewMasks;
    rpmv_ci.correlationMaskCount = 1;
    rpmv_ci.pCorrelationMasks = correlationMasks;

    VkRenderPassCreateInfo render_pass_ci = vku::InitStructHelper(&rpmv_ci);
    render_pass_ci.subpassCount = 1u;
    render_pass_ci.pSubpasses = &subpass;

    vkt::RenderPass render_pass(*m_device, render_pass_ci);

    VkFramebufferCreateInfo framebuffer_ci = vku::InitStructHelper();
    framebuffer_ci.renderPass = render_pass.handle();
    framebuffer_ci.width = 32u;
    framebuffer_ci.height = 32u;
    framebuffer_ci.layers = 1u;

    vkt::Framebuffer framebuffer(*m_device, framebuffer_ci);

    VkRenderPassBeginInfo render_pass_bi = vku::InitStructHelper();
    render_pass_bi.renderPass = render_pass.handle();
    render_pass_bi.framebuffer = framebuffer.handle();
    render_pass_bi.renderArea = {{0, 0}, {32u, 32u}};
    render_pass_bi.clearValueCount = 1u;
    render_pass_bi.pClearValues = m_renderPassClearValues.data();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(render_pass_bi);

    m_errorMonitor->SetDesiredError("VUID-vkCmdWriteTimestamp-query-00831");
    vk::CmdWriteTimestamp(m_command_buffer.handle(), VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, query_pool.handle(), 1);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeQuery, Stride) {
    TEST_DESCRIPTION("Validate Stride parameter.");
    RETURN_IF_SKIP(Init());

    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_count, queue_props.data());
    if (queue_props[m_device->graphics_queue_node_index_].timestampValidBits == 0) {
        GTEST_SKIP() << " Device graphic queue has timestampValidBits of 0, skipping.";
    }

    InitRenderTarget();

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 1);

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 1);
    vk::CmdWriteTimestamp(m_command_buffer.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool.handle(), 0);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    char data_space;
    m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-flags-02828");
    vk::GetQueryPoolResults(m_device->handle(), query_pool.handle(), 0, 1, sizeof(data_space), &data_space, 1,
                            VK_QUERY_RESULT_WAIT_BIT);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-flags-00815");
    vk::GetQueryPoolResults(m_device->handle(), query_pool.handle(), 0, 1, sizeof(data_space), &data_space, 1,
                            (VK_QUERY_RESULT_WAIT_BIT | VK_QUERY_RESULT_64_BIT));
    m_errorMonitor->VerifyFound();

    char data_space4[4] = "";
    vk::GetQueryPoolResults(m_device->handle(), query_pool.handle(), 0, 1, sizeof(data_space4), &data_space4, 4,
                            VK_QUERY_RESULT_WAIT_BIT);

    char data_space8[8] = "";
    vk::GetQueryPoolResults(m_device->handle(), query_pool.handle(), 0, 1, sizeof(data_space8), &data_space8, 8,
                            (VK_QUERY_RESULT_WAIT_BIT | VK_QUERY_RESULT_64_BIT));

    vkt::Buffer buffer(*m_device, 128, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_command_buffer.Reset();
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-flags-00822");
    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool.handle(), 0, 1, buffer.handle(), 1, 1, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-flags-00823");
    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool.handle(), 0, 1, buffer.handle(), 1, 1,
                                VK_QUERY_RESULT_64_BIT);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool.handle(), 0, 1, buffer.handle(), 4, 4, 0);

    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool.handle(), 0, 1, buffer.handle(), 8, 8,
                                VK_QUERY_RESULT_64_BIT);
}

TEST_F(NegativeQuery, PerfQueryQueueFamilyIndex) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::performanceCounterQueryPools);
    RETURN_IF_SKIP(Init());

    vkt::Queue *queue0 = m_default_queue;
    auto queue1_family = m_device->ComputeOnlyQueueFamily();
    if (!queue1_family.has_value()) {
        GTEST_SKIP() << "Can't find two different queue families";
    }
    assert(queue0->family_index != queue1_family.value());

    uint32_t counterCount = 0u;
    vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(m_device->Physical(), 0, &counterCount, nullptr, nullptr);
    std::vector<VkPerformanceCounterKHR> counters(counterCount, vku::InitStruct<VkPerformanceCounterKHR>());
    std::vector<VkPerformanceCounterDescriptionKHR> counterDescriptions(counterCount,
                                                                        vku::InitStruct<VkPerformanceCounterDescriptionKHR>());
    vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(m_device->Physical(), 0, &counterCount, counters.data(),
                                                                      counterDescriptions.data());

    std::vector<uint32_t> enabledCounters(128);
    const uint32_t enabledCounterCount = std::min(counterCount, static_cast<uint32_t>(enabledCounters.size()));
    for (uint32_t i = 0; i < enabledCounterCount; ++i) {
        enabledCounters[i] = i;
    }

    auto query_pool_performance_ci = vku::InitStruct<VkQueryPoolPerformanceCreateInfoKHR>();
    query_pool_performance_ci.queueFamilyIndex = queue0->family_index;
    query_pool_performance_ci.counterIndexCount = enabledCounterCount;
    query_pool_performance_ci.pCounterIndices = enabledCounters.data();

    uint32_t num_passes = 0;
    vk::GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(m_device->Physical(), &query_pool_performance_ci, &num_passes);

    auto query_pool_ci = vku::InitStruct<VkQueryPoolCreateInfo>(&query_pool_performance_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
    pool_create_info.queueFamilyIndex = queue1_family.value();
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkt::CommandPool command_pool(*m_device, pool_create_info);

    vkt::CommandBuffer cb(*m_device, command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    auto acquire_profiling_lock_info = vku::InitStruct<VkAcquireProfilingLockInfoKHR>();
    acquire_profiling_lock_info.timeout = std::numeric_limits<uint64_t>::max();
    vk::AcquireProfilingLockKHR(*m_device, &acquire_profiling_lock_info);

    cb.Begin();
    vk::CmdResetQueryPool(cb.handle(), query_pool.handle(), 0u, 1u);
    cb.End();

    cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryPool-07289");
    vk::CmdBeginQuery(cb.handle(), query_pool, 0, 0);
    m_errorMonitor->VerifyFound();
    cb.End();
    vk::ReleaseProfilingLockKHR(*m_device);
}

TEST_F(NegativeQuery, NoInitReset) {
    AddRequiredFeature(vkt::Feature::pipelineStatisticsQuery);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkQueryPoolCreateInfo qpci = vkt::QueryPool::CreateInfo(VK_QUERY_TYPE_PIPELINE_STATISTICS, 1);
    qpci.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT;
    vkt::QueryPool query_pool(*m_device, qpci);

    m_command_buffer.Begin();
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-None-00807");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, CopyUnavailableQueries) {
    TEST_DESCRIPTION("Copy query results when they are not available");
    RETURN_IF_SKIP(Init());

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 1u);
    vkt::Buffer buffer(*m_device, 16u, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0u, 1u);
    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool.handle(), 0u, 1u, buffer.handle(), 0u, 0u, 0u);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-None-08752");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeQuery, QueryResultCopyBufferInvalidFlags) {
    TEST_DESCRIPTION("Copy query results to a buffer without transfer dst flag");
    RETURN_IF_SKIP(Init());

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 1u);
    vkt::Buffer buffer(*m_device, 16u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0u, 1u);
    vk::CmdWriteTimestamp(m_command_buffer.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool.handle(), 0u);
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-dstBuffer-00825");
    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool.handle(), 0u, 1u, buffer.handle(), 0u, 0u, 0u);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}
