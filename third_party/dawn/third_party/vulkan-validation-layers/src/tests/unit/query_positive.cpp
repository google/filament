/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

bool QueryTest::HasZeroTimestampValidBits() {
    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_count, queue_props.data());
    return (queue_props[m_device->graphics_queue_node_index_].timestampValidBits == 0);
}

class PositiveQuery : public QueryTest {};

TEST_F(PositiveQuery, OutsideRenderPass) {
    AddRequiredFeature(vkt::Feature::pipelineStatisticsQuery);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkQueryPoolCreateInfo qpci = vkt::QueryPool::CreateInfo(VK_QUERY_TYPE_PIPELINE_STATISTICS, 1);
    qpci.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT;
    vkt::QueryPool query_pool(*m_device, qpci);

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 1);
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_command_buffer.EndRenderPass();
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);
    m_command_buffer.End();
}

TEST_F(PositiveQuery, InsideRenderPass) {
    AddRequiredFeature(vkt::Feature::pipelineStatisticsQuery);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkQueryPoolCreateInfo qpci = vkt::QueryPool::CreateInfo(VK_QUERY_TYPE_PIPELINE_STATISTICS, 1);
    qpci.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT;
    vkt::QueryPool query_pool(*m_device, qpci);

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 1);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveQuery, ResetQueryPoolFromDifferentCB) {
#if defined(VVL_ENABLE_TSAN)
    // NOTE: This test in particular has failed sporadically on CI when TSAN is enabled.
    GTEST_SKIP() << "https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5965";
#endif
    TEST_DESCRIPTION("Reset a query on one CB and use it in another.");

    RETURN_IF_SKIP(Init());

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandPool = m_command_pool.handle();
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(device(), &command_buffer_allocate_info, command_buffer);

    {
        VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();

        vk::BeginCommandBuffer(command_buffer[0], &begin_info);
        vk::CmdResetQueryPool(command_buffer[0], query_pool.handle(), 0, 1);
        vk::EndCommandBuffer(command_buffer[0]);

        vk::BeginCommandBuffer(command_buffer[1], &begin_info);
        vk::CmdBeginQuery(command_buffer[1], query_pool.handle(), 0, 0);
        vk::CmdEndQuery(command_buffer[1], query_pool.handle(), 0);
        vk::EndCommandBuffer(command_buffer[1]);
    }
    {
        VkSubmitInfo submit_info[2]{};
        submit_info[0] = vku::InitStructHelper();
        submit_info[0].commandBufferCount = 1;
        submit_info[0].pCommandBuffers = &command_buffer[0];
        submit_info[0].signalSemaphoreCount = 0;
        submit_info[0].pSignalSemaphores = nullptr;

        submit_info[1] = vku::InitStructHelper();
        submit_info[1].commandBufferCount = 1;
        submit_info[1].pCommandBuffers = &command_buffer[1];
        submit_info[1].signalSemaphoreCount = 0;
        submit_info[1].pSignalSemaphores = nullptr;

        vk::QueueSubmit(m_default_queue->handle(), 2, &submit_info[0], VK_NULL_HANDLE);
    }

    m_default_queue->Wait();

    vk::FreeCommandBuffers(device(), m_command_pool.handle(), 2, command_buffer);
}

TEST_F(PositiveQuery, BasicQuery) {
    TEST_DESCRIPTION("Use a couple occlusion queries");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 4 * sizeof(uint64_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 2);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 2);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 1, 0);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 1);
    m_command_buffer.EndRenderPass();
    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool.handle(), 0, 2, buffer.handle(), 0, sizeof(uint64_t),
                                VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    uint64_t samples_passed[4];
    vk::GetQueryPoolResults(m_device->handle(), query_pool.handle(), 0, 2, sizeof(samples_passed), samples_passed, sizeof(uint64_t),
                            VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);

    // Now reset query pool in a different command buffer than the BeginQuery
    vk::ResetCommandBuffer(m_command_buffer.handle(), 0);
    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 1);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    vk::ResetCommandBuffer(m_command_buffer.handle(), 0);
    m_command_buffer.Begin();
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveQuery, DestroyQueryPoolBasedOnQueryPoolResults) {
    TEST_DESCRIPTION("Destroy a QueryPool based on vkGetQueryPoolResults");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    std::array<uint64_t, 4> samples_passed = {};
    constexpr uint64_t sizeof_samples_passed = samples_passed.size() * sizeof(uint64_t);
    constexpr VkDeviceSize sample_stride = sizeof(uint64_t);

    vkt::Buffer buffer(*m_device, sizeof_samples_passed, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    constexpr uint32_t query_count = 2;

    VkQueryPoolCreateInfo query_pool_info = vkt::QueryPool::CreateInfo(VK_QUERY_TYPE_OCCLUSION, query_count);
    VkQueryPool query_pool;
    vk::CreateQueryPool(m_device->handle(), &query_pool_info, nullptr, &query_pool);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    // If VK_QUERY_RESULT_WAIT_BIT is not set, vkGetQueryPoolResults may return VK_NOT_READY
    constexpr VkQueryResultFlags query_flags = VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT;

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool, 0, query_count);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool, 0, 0);
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool, 0);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool, 1, 0);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool, 1);
    m_command_buffer.EndRenderPass();
    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool, 0, query_count, buffer.handle(), 0, sample_stride,
                                query_flags);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);

    VkResult res = vk::GetQueryPoolResults(m_device->handle(), query_pool, 0, query_count, sizeof_samples_passed,
                                           samples_passed.data(), sample_stride, query_flags);

    if (res == VK_SUCCESS) {
        // "Applications can verify that queryPool can be destroyed by checking that vkGetQueryPoolResults() without the
        // VK_QUERY_RESULT_PARTIAL_BIT flag returns VK_SUCCESS for all queries that are used in command buffers submitted for
        // execution."
        //
        // i.e. You don't have to wait for an idle queue to destroy the query pool.
        vk::DestroyQueryPool(m_device->handle(), query_pool, nullptr);
        m_default_queue->Wait();
    } else {
        // some devices (pixel 7) will return VK_NOT_READY
        m_default_queue->Wait();
        vk::DestroyQueryPool(m_device->handle(), query_pool, nullptr);
    }
}

TEST_F(PositiveQuery, QueryAndCopySecondaryCommandBuffers) {
    TEST_DESCRIPTION("Issue a query on a secondary command buffer and copy it on a primary.");

    all_queue_count_ = true;
    RETURN_IF_SKIP(Init());
    if ((m_device->Physical().queue_properties_.empty()) || (m_device->Physical().queue_properties_[0].queueCount < 2)) {
        GTEST_SKIP() << "Queue family needs to have multiple queues to run this test";
    }
    if (HasZeroTimestampValidBits()) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 1);

    vkt::CommandPool command_pool(*m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer primary_buffer(*m_device, command_pool);
    vkt::CommandBuffer secondary_buffer(*m_device, command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(device(), m_device->graphics_queue_node_index_, 1, &queue);

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkCommandBufferInheritanceInfo hinfo = vku::InitStructHelper();
    hinfo.renderPass = VK_NULL_HANDLE;
    hinfo.subpass = 0;
    hinfo.framebuffer = VK_NULL_HANDLE;
    hinfo.occlusionQueryEnable = VK_FALSE;
    hinfo.queryFlags = 0;
    hinfo.pipelineStatistics = 0;

    {
        VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
        begin_info.pInheritanceInfo = &hinfo;
        secondary_buffer.Begin(&begin_info);
        vk::CmdResetQueryPool(secondary_buffer.handle(), query_pool.handle(), 0, 1);
        vk::CmdWriteTimestamp(secondary_buffer.handle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, query_pool.handle(), 0);
        secondary_buffer.End();

        primary_buffer.Begin();
        vk::CmdExecuteCommands(primary_buffer.handle(), 1, &secondary_buffer.handle());
        vk::CmdCopyQueryPoolResults(primary_buffer.handle(), query_pool.handle(), 0, 1, buffer.handle(), 0, 0,
                                    VK_QUERY_RESULT_WAIT_BIT);
        primary_buffer.End();
    }

    m_default_queue->Submit(primary_buffer);
    m_default_queue->Wait();
    vk::QueueWaitIdle(queue);
}

TEST_F(PositiveQuery, QueryAndCopyMultipleCommandBuffers) {
    TEST_DESCRIPTION("Issue a query and copy from it on a second command buffer.");

    all_queue_count_ = true;
    RETURN_IF_SKIP(Init());
    if ((m_device->Physical().queue_properties_.empty()) || (m_device->Physical().queue_properties_[0].queueCount < 2)) {
        GTEST_SKIP() << "Queue family needs to have multiple queues to run this test";
    }
    if (HasZeroTimestampValidBits()) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 1);

    VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkt::CommandPool command_pool(*m_device, pool_create_info);

    VkCommandBuffer command_buffer[2];
    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandPool = command_pool.handle();
    command_buffer_allocate_info.commandBufferCount = 2;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(device(), &command_buffer_allocate_info, command_buffer);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(device(), m_device->graphics_queue_node_index_, 1, &queue);

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    {
        VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
        vk::BeginCommandBuffer(command_buffer[0], &begin_info);

        vk::CmdResetQueryPool(command_buffer[0], query_pool.handle(), 0, 1);
        vk::CmdWriteTimestamp(command_buffer[0], VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, query_pool.handle(), 0);

        vk::EndCommandBuffer(command_buffer[0]);

        vk::BeginCommandBuffer(command_buffer[1], &begin_info);

        vk::CmdCopyQueryPoolResults(command_buffer[1], query_pool.handle(), 0, 1, buffer.handle(), 0, 0, VK_QUERY_RESULT_WAIT_BIT);

        vk::EndCommandBuffer(command_buffer[1]);
    }
    {
        VkSubmitInfo submit_info = vku::InitStructHelper();
        submit_info.commandBufferCount = 2;
        submit_info.pCommandBuffers = command_buffer;
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = nullptr;
        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    }

    vk::QueueWaitIdle(queue);
}

TEST_F(PositiveQuery, DestroyQueryPoolAfterGetQueryPoolResults) {
    TEST_DESCRIPTION("Destroy query pool after GetQueryPoolResults() without VK_QUERY_RESULT_PARTIAL_BIT returns VK_SUCCESS");

    RETURN_IF_SKIP(Init());
    if (HasZeroTimestampValidBits()) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 1);

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 1);
    vk::CmdWriteTimestamp(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, query_pool.handle(), 0);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);

    const size_t out_data_size = 16;
    uint8_t data[out_data_size];
    VkResult res;
    do {
        res = vk::GetQueryPoolResults(device(), query_pool.handle(), 0, 1, out_data_size, &data, 4, 0);
    } while (res != VK_SUCCESS);

    m_default_queue->Wait();
}

TEST_F(PositiveQuery, WriteTimestampNoneAndAll) {
    TEST_DESCRIPTION("Test using vkCmdWriteTimestamp2 with NONE and ALL_COMMANDS.");

    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    if (HasZeroTimestampValidBits()) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 2);

    m_command_buffer.Begin();
    vk::CmdWriteTimestamp2KHR(m_command_buffer.handle(), VK_PIPELINE_STAGE_2_NONE, query_pool.handle(), 0);
    vk::CmdWriteTimestamp2KHR(m_command_buffer.handle(), VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, query_pool.handle(), 1);
    m_command_buffer.End();
}

TEST_F(PositiveQuery, CommandBufferInheritanceFlags) {
    TEST_DESCRIPTION("Test executing secondary command buffer with VkCommandBufferInheritanceInfo::queryFlags.");
    AddRequiredFeature(vkt::Feature::inheritedQueries);
    AddRequiredFeature(vkt::Feature::occlusionQueryPrecise);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo cbii = vku::InitStructHelper();
    cbii.renderPass = m_renderPass;
    cbii.framebuffer = Framebuffer();
    cbii.occlusionQueryEnable = VK_TRUE;
    cbii.queryFlags = VK_QUERY_CONTROL_PRECISE_BIT;

    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.pInheritanceInfo = &cbii;
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    VkCommandBuffer secondary_handle = secondary.handle();
    vk::BeginCommandBuffer(secondary_handle, &cbbi);
    vk::EndCommandBuffer(secondary_handle);

    m_command_buffer.Begin();
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary_handle);
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);

    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 1);

    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, VK_QUERY_CONTROL_PRECISE_BIT);
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary_handle);
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 0);
    m_command_buffer.End();
}

TEST_F(PositiveQuery, PerformanceQueries) {
#if defined(VVL_ENABLE_TSAN)
    // NOTE: This test in particular has failed sporadically on CI when TSAN is enabled.
    GTEST_SKIP() << "https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5965";
#endif
    TEST_DESCRIPTION("Test performance queries.");

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

    std::vector<uint32_t> enabledCounters(128);
    const uint32_t enabledCounterCount = std::min(counterCount, static_cast<uint32_t>(enabledCounters.size()));
    for (uint32_t i = 0; i < enabledCounterCount; ++i) {
        enabledCounters[i] = i;
    }

    auto query_pool_performance_ci = vku::InitStruct<VkQueryPoolPerformanceCreateInfoKHR>();
    query_pool_performance_ci.queueFamilyIndex = m_device->graphics_queue_node_index_;
    query_pool_performance_ci.counterIndexCount = enabledCounterCount;
    query_pool_performance_ci.pCounterIndices = enabledCounters.data();

    uint32_t num_passes = 0u;
    vk::GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(m_device->Physical(), &query_pool_performance_ci, &num_passes);

    auto query_pool_ci = vku::InitStruct<VkQueryPoolCreateInfo>(&query_pool_performance_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1u;

    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    {
        m_command_buffer.Begin();
        vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0u, 1u);
        m_command_buffer.End();

        m_default_queue->Submit(m_command_buffer);
        m_device->Wait();
    }

    vkt::CommandBuffer cmd_buffer(*m_device, m_command_pool);

    auto acquire_profiling_lock_info = vku::InitStruct<VkAcquireProfilingLockInfoKHR>();
    acquire_profiling_lock_info.timeout = std::numeric_limits<uint64_t>::max();

    vk::AcquireProfilingLockKHR(*m_device, &acquire_profiling_lock_info);

    VkCommandBufferBeginInfo info = vku::InitStructHelper();
    cmd_buffer.Begin(&info);

    vk::CmdBeginQuery(cmd_buffer.handle(), query_pool.handle(), 0u, 0u);

    vk::CmdPipelineBarrier(cmd_buffer.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0u, 0u,
                           nullptr, 0u, nullptr, 0u, nullptr);

    vk::CmdEndQuery(cmd_buffer.handle(), query_pool.handle(), 0u);

    cmd_buffer.End();

    for (uint32_t counterPass = 0u; counterPass < num_passes; ++counterPass) {
        auto performance_query_submit_info = vku::InitStruct<VkPerformanceQuerySubmitInfoKHR>();
        performance_query_submit_info.counterPassIndex = counterPass;

        auto submit_info = vku::InitStruct<VkSubmitInfo>(&performance_query_submit_info);
        submit_info.commandBufferCount = 1u;
        submit_info.pCommandBuffers = &cmd_buffer.handle();
        vk::QueueSubmit(m_default_queue->handle(), 1u, &submit_info, VK_NULL_HANDLE);
        m_device->Wait();
    }

    vk::ReleaseProfilingLockKHR(*m_device);

    std::vector<VkPerformanceCounterResultKHR> recordedCounters(enabledCounterCount);
    vk::GetQueryPoolResults(*m_device, query_pool.handle(), 0u, 1u, sizeof(VkPerformanceCounterResultKHR) * enabledCounterCount,
                            recordedCounters.data(), sizeof(VkPerformanceCounterResultKHR) * enabledCounterCount, 0u);
}

TEST_F(PositiveQuery, HostQueryResetSuccess) {
    TEST_DESCRIPTION("Use vkResetQueryPoolEXT normally");

    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::hostQueryReset);
    RETURN_IF_SKIP(Init());

    VkQueryPoolCreateInfo query_pool_create_info = vku::InitStructHelper();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, query_pool_create_info);
    vk::ResetQueryPoolEXT(device(), query_pool.handle(), 0, 1);
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7874
TEST_F(PositiveQuery, ReuseSecondaryWithQueryCommand) {
    TEST_DESCRIPTION("Regression test for a deadlock when secondary command buffer is reused and records a query command");

    RETURN_IF_SKIP(Init());
    if (HasZeroTimestampValidBits()) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.";
    }

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 1);

    vkt::CommandBuffer secondary_buffer(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary_buffer.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    vk::CmdWriteTimestamp(secondary_buffer.handle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, query_pool, 0);
    secondary_buffer.End();

    m_command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool, 0, 1);
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary_buffer.handle());
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    // Submit the command buffer again.
    m_default_queue->Submit(m_command_buffer);

    m_default_queue->Wait();
}

TEST_F(PositiveQuery, PerformanceCountersWithoutEnumeration) {
    TEST_DESCRIPTION("Create performance queries without enumerating queue family performance queries");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::performanceCounterQueryPools);
    RETURN_IF_SKIP(Init());

    uint32_t counterCount = 0u;
    vk::EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(m_device->Physical(), m_device->graphics_queue_node_index_,
                                                                      &counterCount, nullptr, nullptr);

    if (counterCount < 128) {
        GTEST_SKIP() << "Required performance query counter count not supported";
    }

    std::vector<uint32_t> enabledCounters(128);
    const uint32_t enabledCounterCount = static_cast<uint32_t>(enabledCounters.size());
    for (uint32_t i = 0; i < enabledCounterCount; ++i) {
        enabledCounters[i] = i;
    }

    auto query_pool_performance_ci = vku::InitStruct<VkQueryPoolPerformanceCreateInfoKHR>();
    query_pool_performance_ci.queueFamilyIndex = m_device->graphics_queue_node_index_;
    query_pool_performance_ci.counterIndexCount = enabledCounterCount;
    query_pool_performance_ci.pCounterIndices = enabledCounters.data();

    uint32_t num_passes = 0u;
    vk::GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(m_device->Physical(), &query_pool_performance_ci, &num_passes);

    auto query_pool_ci = vku::InitStruct<VkQueryPoolCreateInfo>(&query_pool_performance_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1u;

    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    {
        m_command_buffer.Begin();
        vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0u, 1u);
        m_command_buffer.End();

        m_default_queue->Submit(m_command_buffer);
        m_device->Wait();
    }

    vkt::CommandBuffer cmd_buffer(*m_device, m_command_pool);

    auto acquire_profiling_lock_info = vku::InitStruct<VkAcquireProfilingLockInfoKHR>();
    acquire_profiling_lock_info.timeout = std::numeric_limits<uint64_t>::max();

    vk::AcquireProfilingLockKHR(*m_device, &acquire_profiling_lock_info);

    VkCommandBufferBeginInfo info = vku::InitStructHelper();
    cmd_buffer.Begin(&info);

    vk::CmdBeginQuery(cmd_buffer.handle(), query_pool.handle(), 0u, 0u);

    vk::CmdPipelineBarrier(cmd_buffer.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0u, 0u,
                           nullptr, 0u, nullptr, 0u, nullptr);

    vk::CmdEndQuery(cmd_buffer.handle(), query_pool.handle(), 0u);

    cmd_buffer.End();

    for (uint32_t counterPass = 0u; counterPass < num_passes; ++counterPass) {
        auto performance_query_submit_info = vku::InitStruct<VkPerformanceQuerySubmitInfoKHR>();
        performance_query_submit_info.counterPassIndex = counterPass;

        auto submit_info = vku::InitStruct<VkSubmitInfo>(&performance_query_submit_info);
        submit_info.commandBufferCount = 1u;
        submit_info.pCommandBuffers = &cmd_buffer.handle();
        vk::QueueSubmit(m_default_queue->handle(), 1u, &submit_info, VK_NULL_HANDLE);
        m_device->Wait();
    }

    vk::ReleaseProfilingLockKHR(*m_device);

    std::vector<VkPerformanceCounterResultKHR> recordedCounters(enabledCounterCount);
    vk::GetQueryPoolResults(*m_device, query_pool.handle(), 0u, 1u, sizeof(VkPerformanceCounterResultKHR) * enabledCounterCount,
                            recordedCounters.data(), sizeof(VkPerformanceCounterResultKHR) * enabledCounterCount, 0u);
}
