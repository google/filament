/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <thread>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/render_pass_helper.h"
#include "../framework/thread_helper.h"

class PositiveCommand : public VkLayerTest {};

TEST_F(PositiveCommand, DrawIndirectCountWithoutFeature) {
    TEST_DESCRIPTION("Use VK_KHR_draw_indirect_count in 1.1 before drawIndirectCount feature was added");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    if (DeviceValidationVersion() != VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Tests requires Vulkan 1.1 exactly";
    }

    vkt::Buffer indirect_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer indexed_indirect_buffer(*m_device, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer index_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    // Make calls to valid commands
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDrawIndirectCountKHR(m_command_buffer.handle(), indirect_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                sizeof(VkDrawIndirectCommand));
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirectCountKHR(m_command_buffer.handle(), indexed_indirect_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                       sizeof(VkDrawIndexedIndirectCommand));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveCommand, DrawIndirectCountWithoutFeature12) {
    TEST_DESCRIPTION("Use VK_KHR_draw_indirect_count in 1.2 using the extension");

    // We need to explicitly allow promoted extensions to be enabled as this test relies on this behavior
    AllowPromotedExtensions();

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Buffer indirect_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer indexed_indirect_buffer(*m_device, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer index_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    // Make calls to valid commands
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDrawIndirectCount(m_command_buffer.handle(), indirect_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                             sizeof(VkDrawIndirectCommand));
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirectCount(m_command_buffer.handle(), indexed_indirect_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                    sizeof(VkDrawIndexedIndirectCommand));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveCommand, DrawIndirectCountWithFeature) {
    TEST_DESCRIPTION("Use VK_KHR_draw_indirect_count in 1.2 with feature bit enabled");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::drawIndirectCount);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Buffer indirect_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer indexed_indirect_buffer(*m_device, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer index_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    // Make calls to valid commands
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDrawIndirectCount(m_command_buffer.handle(), indirect_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                             sizeof(VkDrawIndirectCommand));
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirectCount(m_command_buffer.handle(), indexed_indirect_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                    sizeof(VkDrawIndexedIndirectCommand));
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveCommand, CommandBufferSimultaneousUseSync) {
    RETURN_IF_SKIP(Init());

    // Record (empty!) command buffer that can be submitted multiple times
    // simultaneously.
    VkCommandBufferBeginInfo cbbi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                     VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, nullptr};
    m_command_buffer.Begin(&cbbi);
    m_command_buffer.End();

    VkFenceCreateInfo fci = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0};
    VkFence fence;
    vk::CreateFence(device(), &fci, nullptr, &fence);

    VkSemaphoreCreateInfo sci = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
    VkSemaphore s1, s2;
    vk::CreateSemaphore(device(), &sci, nullptr, &s1);
    vk::CreateSemaphore(device(), &sci, nullptr, &s2);

    // Submit CB once signaling s1, with fence so we can roll forward to its retirement.
    VkSubmitInfo si = {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 1, &m_command_buffer.handle(), 1, &s1};
    vk::QueueSubmit(m_default_queue->handle(), 1, &si, fence);

    // Submit CB again, signaling s2.
    si.pSignalSemaphores = &s2;
    vk::QueueSubmit(m_default_queue->handle(), 1, &si, VK_NULL_HANDLE);

    // Wait for fence.
    vk::WaitForFences(device(), 1, &fence, VK_TRUE, kWaitTimeout);

    // CB is still in flight from second submission, but semaphore s1 is no
    // longer in flight. delete it.
    vk::DestroySemaphore(device(), s1, nullptr);

    // Force device idle and clean up remaining objects
    m_device->Wait();
    vk::DestroySemaphore(device(), s2, nullptr);
    vk::DestroyFence(device(), fence, nullptr);
}

TEST_F(PositiveCommand, FramebufferBindingDestroyCommandPool) {
    TEST_DESCRIPTION(
        "This test should pass. Create a Framebuffer and command buffer, bind them together, then destroy command pool and "
        "framebuffer and verify there are no errors.");

    RETURN_IF_SKIP(Init());

    // A renderpass with one color attachment.
    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    // A compatible framebuffer.
    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::ImageView view = image.CreateView();
    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &view.handle());

    // Explicitly create a command buffer to bind the FB to so that we can then
    //  destroy the command pool in order to implicitly free command buffer
    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer;
    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(device(), &command_buffer_allocate_info, &command_buffer);

    // Begin our cmd buffer with renderpass using our framebuffer
    VkRenderPassBeginInfo rpbi =
        vku::InitStruct<VkRenderPassBeginInfo>(nullptr, rp.Handle(), fb.handle(), VkRect2D{{0, 0}, {32u, 32u}}, 0u, nullptr);
    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    vk::BeginCommandBuffer(command_buffer, &begin_info);

    vk::CmdBeginRenderPass(command_buffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdEndRenderPass(command_buffer);
    vk::EndCommandBuffer(command_buffer);
    // Destroy command pool to implicitly free command buffer
    vk::DestroyCommandPool(device(), command_pool, NULL);
}

TEST_F(PositiveCommand, ClearRectWith2DArray) {
    TEST_DESCRIPTION("Test using VkClearRect with an image that is of a 2D array type.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    for (uint32_t i = 0; i < 2; ++i) {
        VkImageCreateInfo image_ci = vku::InitStructHelper();
        image_ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
        image_ci.imageType = VK_IMAGE_TYPE_3D;
        image_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
        image_ci.extent = {32, 32, 4};
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        vkt::Image image(*m_device, image_ci, vkt::set_layout);
        uint32_t layer_count = i == 0 ? image_ci.extent.depth : VK_REMAINING_ARRAY_LAYERS;
        vkt::ImageView image_view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, layer_count);

        RenderPassSingleSubpass rp(*this);
        rp.AddAttachmentDescription(image_ci.format, image_ci.samples, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
        rp.AddColorAttachment(0);
        rp.CreateRenderPass();

        VkFramebufferCreateInfo fbci = vku::InitStructHelper();
        fbci.renderPass = rp.Handle();
        fbci.attachmentCount = 1;
        fbci.pAttachments = &image_view.handle();
        fbci.width = image_ci.extent.width;
        fbci.height = image_ci.extent.height;
        fbci.layers = image_ci.extent.depth;

        vkt::Framebuffer framebuffer(*m_device, fbci);

        VkClearAttachment color_attachment;
        color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        color_attachment.clearValue.color.float32[0] = 0.0;
        color_attachment.clearValue.color.float32[1] = 0.0;
        color_attachment.clearValue.color.float32[2] = 0.0;
        color_attachment.clearValue.color.float32[3] = 0.0;
        color_attachment.colorAttachment = 0;

        VkClearValue clearValue;
        clearValue.color = {{0, 0, 0, 0}};

        VkRenderPassBeginInfo rpbi = vku::InitStructHelper();
        rpbi.renderPass = rp.Handle();
        rpbi.framebuffer = framebuffer.handle();
        rpbi.renderArea = {{0, 0}, {image_ci.extent.width, image_ci.extent.height}};
        rpbi.clearValueCount = 1;
        rpbi.pClearValues = &clearValue;

        m_command_buffer.Begin();
        m_command_buffer.BeginRenderPass(rpbi);

        VkClearRect clear_rect = {{{0, 0}, {image_ci.extent.width, image_ci.extent.height}}, 0, image_ci.extent.depth};
        vk::CmdClearAttachments(m_command_buffer.handle(), 1, &color_attachment, 1, &clear_rect);

        m_command_buffer.EndRenderPass();
        m_command_buffer.End();

        m_command_buffer.Reset();
    }
}

TEST_F(PositiveCommand, ThreadedCommandBuffersWithLabels) {
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    constexpr int worker_count = 8;
    ThreadTimeoutHelper timeout_helper(worker_count);

    auto worker_thread = [&](int worker_index) {
        auto timeout_guard = timeout_helper.ThreadGuard();
        // Command pool per worker thread
        vkt::CommandPool pool(*m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);

        constexpr int command_buffers_per_pool = 4;
        VkCommandBufferAllocateInfo commands_allocate_info = vku::InitStructHelper();
        commands_allocate_info.commandPool = pool.handle();
        commands_allocate_info.commandBufferCount = command_buffers_per_pool;
        commands_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        const VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();

        VkDebugUtilsLabelEXT label = vku::InitStructHelper();
        label.pLabelName = "Test label";

        constexpr int iteration_count = 1000;
        for (int frame = 0; frame < iteration_count; frame++) {
            std::array<VkCommandBuffer, command_buffers_per_pool> command_buffers;
            ASSERT_EQ(VK_SUCCESS, vk::AllocateCommandBuffers(device(), &commands_allocate_info, command_buffers.data()));
            for (int i = 0; i < command_buffers_per_pool; i++) {
                ASSERT_EQ(VK_SUCCESS, vk::BeginCommandBuffer(command_buffers[i], &begin_info));
                // Record debug label. It's a required step to reproduce the original issue
                vk::CmdInsertDebugUtilsLabelEXT(command_buffers[i], &label);
                ASSERT_EQ(VK_SUCCESS, vk::EndCommandBuffer(command_buffers[i]));
            }
            vk::FreeCommandBuffers(device(), pool.handle(), command_buffers_per_pool, command_buffers.data());
        }
    };
    std::vector<std::thread> workers;
    for (int i = 0; i < worker_count; i++) workers.emplace_back(worker_thread, i);
    constexpr int wait_time = 60;
    if (!timeout_helper.WaitForThreads(wait_time))
        ADD_FAILURE() << "The waiting time for the worker threads exceeded the maximum limit: " << wait_time << " seconds.";
    for (auto &worker : workers) worker.join();
}

TEST_F(PositiveCommand, ClearAttachmentsDepthStencil) {
    TEST_DESCRIPTION("Call CmdClearAttachments with no depth/stencil attachment.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    m_command_buffer.Begin();
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkClearAttachment attachment;
    attachment.colorAttachment = 0;
    VkClearRect clear_rect = {};
    clear_rect.rect.offset = {0, 0};
    clear_rect.rect.extent = {1, 1};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;

    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &attachment, 1, &clear_rect);

    attachment.clearValue.depthStencil.depth = 0.0f;
    // Aspect Mask can be non-color because pDepthStencilAttachment is null
    attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &attachment, 1, &clear_rect);
    attachment.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &attachment, 1, &clear_rect);
}

TEST_F(PositiveCommand, ClearColorImageWithValidRange) {
    TEST_DESCRIPTION("Record clear color with a valid VkImageSubresourceRange");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    const VkClearColorValue clear_color = {{0.0f, 0.0f, 0.0f, 1.0f}};

    m_command_buffer.Begin();
    const auto cb_handle = m_command_buffer.handle();

    // Try good case
    {
        VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
    }

    image.ImageMemoryBarrier(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                             VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Try good case with VK_REMAINING
    {
        VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
    }
}

TEST_F(PositiveCommand, ClearDepthStencilWithValidRange) {
    TEST_DESCRIPTION("Record clear depth with a valid VkImageSubresourceRange");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    auto depth_format = FindSupportedDepthStencilFormat(Gpu());
    vkt::Image image(*m_device, 32, 32, 1, depth_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    const VkImageAspectFlags ds_aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    image.SetLayout(ds_aspect, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    const VkClearDepthStencilValue clear_value = {};

    m_command_buffer.Begin();
    const auto cb_handle = m_command_buffer.handle();

    // Try good case
    {
        VkImageSubresourceRange range = {ds_aspect, 0, 1, 0, 1};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
    }

    image.ImageMemoryBarrier(m_command_buffer, ds_aspect, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Try good case with VK_REMAINING
    {
        VkImageSubresourceRange range = {ds_aspect, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
    }
}

TEST_F(PositiveCommand, ClearColor64Bit) {
    TEST_DESCRIPTION("Clear with a 64-bit format");
    RETURN_IF_SKIP(Init());

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R64_UINT, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "VK_FORMAT_R64_UINT format not supported";
    }
    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R64_UINT, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    m_command_buffer.Begin();
    const VkClearColorValue clear_color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vk::CmdClearColorImage(m_command_buffer.handle(), image.handle(), image.Layout(), &clear_color, 1, &range);
}

TEST_F(PositiveCommand, FillBufferCmdPoolTransferQueue) {
    TEST_DESCRIPTION(
        "Use a command buffer with vkCmdFillBuffer that was allocated from a command pool that does not support graphics or "
        "compute opeartions");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());

    auto tranfer_family = m_device->TransferOnlyQueueFamily();
    if (!tranfer_family.has_value()) {
        GTEST_SKIP() << "Transfer-only queue family not found";
    }

    vkt::CommandPool pool(*m_device, tranfer_family.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer cb(*m_device, pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    vkt::Buffer buffer(*m_device, 20, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    cb.Begin();
    vk::CmdFillBuffer(cb.handle(), buffer.handle(), 0, 12, 0x11111111);
    cb.End();
}

TEST_F(PositiveCommand, MultiDrawMaintenance5) {
    TEST_DESCRIPTION("Test validation of multi_draw extension with VK_KHR_maintenance5");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    AddRequiredFeature(vkt::Feature::multiDraw);
    RETURN_IF_SKIP(Init());

    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    // Try existing VUID checks
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    // New VUIDs added with multi_draw (also see GPU-AV)
    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkMultiDrawIndexedInfoEXT multi_draw_indices = {0, 511, 0};

    vk::CmdBindIndexBuffer2KHR(m_command_buffer.handle(), buffer.handle(), 0, 1024, VK_INDEX_TYPE_UINT16);
    vk::CmdDrawMultiIndexedEXT(m_command_buffer.handle(), 1, &multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);

    vk::CmdBindIndexBuffer2KHR(m_command_buffer.handle(), buffer.handle(), 2, 1022, VK_INDEX_TYPE_UINT16);
    vk::CmdDrawMultiIndexedEXT(m_command_buffer.handle(), 1, &multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);

    vk::CmdBindIndexBuffer2KHR(m_command_buffer.handle(), buffer.handle(), 0, VK_WHOLE_SIZE, VK_INDEX_TYPE_UINT16);
    vk::CmdDrawMultiIndexedEXT(m_command_buffer.handle(), 1, &multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);

    multi_draw_indices.firstIndex = 16;
    multi_draw_indices.indexCount = 128;
    vk::CmdBindIndexBuffer2KHR(m_command_buffer.handle(), buffer.handle(), 32, 288, VK_INDEX_TYPE_UINT16);
    vk::CmdDrawMultiIndexedEXT(m_command_buffer.handle(), 1, &multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
}

TEST_F(PositiveCommand, MultiDrawMaintenance5Mixed) {
    TEST_DESCRIPTION("Test vkCmdBindIndexBuffer2KHR with vkCmdBindIndexBuffer");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    AddRequiredFeature(vkt::Feature::multiDraw);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    // Try existing VUID checks
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    // New VUIDs added with multi_draw (also see GPU-AV)
    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VkMultiDrawIndexedInfoEXT multi_draw_indices = {0, 511, 0};

    // too small
    vk::CmdBindIndexBuffer2KHR(m_command_buffer.handle(), buffer.handle(), 0, 1000, VK_INDEX_TYPE_UINT16);
    // should be overwritten
    vk::CmdBindIndexBuffer(m_command_buffer.handle(), buffer.handle(), 0, VK_INDEX_TYPE_UINT16);
    vk::CmdDrawMultiIndexedEXT(m_command_buffer.handle(), 1, &multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
}

TEST_F(PositiveCommand, ImageFormatTypeMismatchWithZeroExtend) {
    TEST_DESCRIPTION("Use SignExtend to turn a UINT resource into a SINT.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitRenderTarget());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    const char *csSource = R"(
                     OpCapability Shader
                     OpCapability StorageImageExtendedFormats
                     OpMemoryModel Logical GLSL450
                     OpEntryPoint GLCompute %main "main" %image_ptr
                     OpExecutionMode %main LocalSize 1 1 1
                     OpSource GLSL 450
                     OpDecorate %image_ptr DescriptorSet 0
                     OpDecorate %image_ptr Binding 0
                     OpDecorate %image_ptr NonReadable
%type_void         = OpTypeVoid
%type_u32          = OpTypeInt 32 0
%type_i32          = OpTypeInt 32 1
%type_vec3_u32     = OpTypeVector %type_u32 3
%type_vec4_u32     = OpTypeVector %type_u32 4
%type_vec2_i32     = OpTypeVector %type_i32 2
%type_fn_void      = OpTypeFunction %type_void
%type_ptr_fn       = OpTypePointer Function %type_vec4_u32
%type_image        = OpTypeImage %type_u32 2D 0 0 0 2 Rgba32ui
%type_ptr_image    = OpTypePointer UniformConstant %type_image
%image_ptr         = OpVariable %type_ptr_image UniformConstant
%const_i32_0       = OpConstant %type_i32 0
%const_vec2_i32_00 = OpConstantComposite %type_vec2_i32 %const_i32_0 %const_i32_0
%main              = OpFunction %type_void None %type_fn_void
%label             = OpLabel
%store_location    = OpVariable %type_ptr_fn Function
%image             = OpLoad %type_image %image_ptr
%value             = OpImageRead %type_vec4_u32 %image %const_vec2_i32_00 SignExtend
                     OpStore %store_location %value
                     OpReturn
                     OpFunctionEnd
              )";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_.reset(new VkShaderObj(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM));
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    pipe.CreateComputePipeline();

    VkFormat format = VK_FORMAT_R32G32B32A32_SINT;
    vkt::Image image(*m_device, 32, 32, 1, format, VK_IMAGE_USAGE_STORAGE_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView();

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                   VK_IMAGE_LAYOUT_GENERAL);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);

    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    m_command_buffer.End();
}

TEST_F(PositiveCommand, ImageFormatTypeMismatchRedundantExtend) {
    TEST_DESCRIPTION("Use ZeroExtend as a redundant was to be SINT.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitRenderTarget());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    const char *csSource = R"(
                     OpCapability Shader
                     OpCapability StorageImageExtendedFormats
                     OpMemoryModel Logical GLSL450
                     OpEntryPoint GLCompute %main "main" %image_ptr
                     OpExecutionMode %main LocalSize 1 1 1
                     OpSource GLSL 450
                     OpDecorate %image_ptr DescriptorSet 0
                     OpDecorate %image_ptr Binding 0
                     OpDecorate %image_ptr NonReadable
%type_void         = OpTypeVoid
%type_u32          = OpTypeInt 32 0
%type_i32          = OpTypeInt 32 1
%type_vec3_u32     = OpTypeVector %type_u32 3
%type_vec4_u32     = OpTypeVector %type_u32 4
%type_vec2_i32     = OpTypeVector %type_i32 2
%type_fn_void      = OpTypeFunction %type_void
%type_ptr_fn       = OpTypePointer Function %type_vec4_u32
%type_image        = OpTypeImage %type_u32 2D 0 0 0 2 Rgba32i
%type_ptr_image    = OpTypePointer UniformConstant %type_image
%image_ptr         = OpVariable %type_ptr_image UniformConstant
%const_i32_0       = OpConstant %type_i32 0
%const_vec2_i32_00 = OpConstantComposite %type_vec2_i32 %const_i32_0 %const_i32_0
%main              = OpFunction %type_void None %type_fn_void
%label             = OpLabel
%store_location    = OpVariable %type_ptr_fn Function
%image             = OpLoad %type_image %image_ptr
                   ; both valid, ZeroExtend is acting as redundant
%value             = OpImageRead %type_vec4_u32 %image %const_vec2_i32_00
%value2            = OpImageRead %type_vec4_u32 %image %const_vec2_i32_00 ZeroExtend
                     OpStore %store_location %value
                     OpReturn
                     OpFunctionEnd
              )";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_.reset(new VkShaderObj(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM));
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    pipe.CreateComputePipeline();

    VkFormat format = VK_FORMAT_R32G32B32A32_UINT;
    vkt::Image image(*m_device, 32, 32, 1, format, VK_IMAGE_USAGE_STORAGE_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView();

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                   VK_IMAGE_LAYOUT_GENERAL);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);

    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    m_command_buffer.End();
}

TEST_F(PositiveCommand, DeviceLost) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    if (!IsPlatformMockICD()) {
        GTEST_SKIP() << "Test only supported by MockICD";
    }

    // Destroying should not throw VUID-vkDestroyPipeline-pipeline-00765
    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_command_buffer.End();

    // Destroying should not throw VUID-vkDestroyFence-fence-01120
    vkt::Fence fence(*m_device);

    // Special way to force VK_ERROR_DEVICE_LOST with MockICD
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkSubmitInfo-pNext-pNext");
    VkExportFenceCreateInfo fault_injection = vku::InitStructHelper();

    VkSubmitInfo submit_info = vku::InitStructHelper(&fault_injection);
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffer.handle();
    VkResult result = vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, fence.handle());

    if (result != VK_ERROR_DEVICE_LOST) {
        vk::QueueWaitIdle(m_default_queue->handle());
        GTEST_SKIP() << "No device lost found";
    }
}

TEST_F(PositiveCommand, CommandBufferInheritanceInfoIgnoredPointer) {
    TEST_DESCRIPTION("VkCommandBufferInheritanceInfo is ignored if using a primary command buffer.");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const uint64_t fake_address_64 = 0xCDCDCDCDCDCDCDCD;
    const uint64_t fake_address_32 = 0xCDCDCDCD;
    const void *undereferencable_pointer =
        sizeof(void *) == 8 ? reinterpret_cast<void *>(fake_address_64) : reinterpret_cast<void *>(fake_address_32);

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    begin_info.pInheritanceInfo = reinterpret_cast<const VkCommandBufferInheritanceInfo *>(undereferencable_pointer);
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.End();
}
