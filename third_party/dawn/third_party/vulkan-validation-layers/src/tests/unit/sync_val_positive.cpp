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
#include "../framework/descriptor_helper.h"
#include "../framework/render_pass_helper.h"
#include "../framework/thread_helper.h"
#include "../framework/queue_submit_context.h"
#include "../layers/sync/sync_settings.h"

class PositiveSyncVal : public VkSyncValTest {};

static const std::array syncval_enables = {VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT};

static const std::array syncval_disables = {
    VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT, VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT,
    VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT, VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};


void VkSyncValTest::InitSyncValFramework(const SyncValSettings *p_sync_settings) {
    std::vector<VkLayerSettingEXT> settings;

    static const SyncValSettings test_default_sync_settings = [] {
        // That's a separate set of defaults for testing purposes.
        // The main layer configuration can have some options turned off by default,
        // but we might still want that functionality to be available for testing.
        SyncValSettings settings;
        settings.submit_time_validation = true;
        settings.shader_accesses_heuristic = true;
        return settings;
    }();
    const SyncValSettings &sync_settings = p_sync_settings ? *p_sync_settings : test_default_sync_settings;

    const auto submit_time_validation = static_cast<VkBool32>(sync_settings.submit_time_validation);
    settings.emplace_back(VkLayerSettingEXT{OBJECT_LAYER_NAME, "syncval_submit_time_validation", VK_LAYER_SETTING_TYPE_BOOL32_EXT,
                                            1, &submit_time_validation});

    const auto shader_accesses_heuristic = static_cast<VkBool32>(sync_settings.shader_accesses_heuristic);
    settings.emplace_back(VkLayerSettingEXT{OBJECT_LAYER_NAME, "syncval_shader_accesses_heuristic",
                                            VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &shader_accesses_heuristic});

    VkLayerSettingsCreateInfoEXT settings_create_info = vku::InitStructHelper();
    settings_create_info.settingCount = size32(settings);
    settings_create_info.pSettings = settings.data();

    VkValidationFeaturesEXT validation_features = vku::InitStructHelper();
    validation_features.enabledValidationFeatureCount = size32(syncval_enables);
    validation_features.pEnabledValidationFeatures = syncval_enables.data();
    if (m_syncval_disable_core) {
        validation_features.disabledValidationFeatureCount = size32(syncval_disables);
        validation_features.pDisabledValidationFeatures = syncval_disables.data();
    }
    validation_features.pNext = &settings_create_info;

    AddRequiredExtensions(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
    InitFramework(&validation_features);
}

void VkSyncValTest::InitSyncVal(const SyncValSettings *p_sync_settings) {
    RETURN_IF_SKIP(InitSyncValFramework(p_sync_settings));
    RETURN_IF_SKIP(InitState());
}

void VkSyncValTest::InitTimelineSemaphore() {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(InitSyncVal());
}

void VkSyncValTest::InitRayTracing() {
    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);

    RETURN_IF_SKIP(InitSyncVal());
}

TEST_F(PositiveSyncVal, BufferCopyNonOverlappedRegions) {
    TEST_DESCRIPTION("Copy to non-overlapped regions of the same buffer");
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    VkBufferCopy first_half = {0, 0, 128};
    VkBufferCopy second_half = {128, 128, 128};

    m_command_buffer.Begin();
    vk::CmdCopyBuffer(m_command_buffer, buffer_b, buffer_a, 1, &first_half);
    vk::CmdCopyBuffer(m_command_buffer, buffer_b, buffer_a, 1, &second_half);
    m_command_buffer.End();
}

TEST_F(PositiveSyncVal, BufferCopySecondary) {
    TEST_DESCRIPTION("Execution dependency protects protects READ access from subsequent WRITEs");
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_c(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    // buffer_c READ
    vkt::CommandBuffer secondary_cb1(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary_cb1.Begin();
    secondary_cb1.Copy(buffer_c, buffer_a);
    secondary_cb1.End();

    // Execution dependency
    vkt::CommandBuffer secondary_cb2(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary_cb2.Begin();
    vk::CmdPipelineBarrier(secondary_cb2, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr,
                           0, nullptr);
    secondary_cb2.End();

    // buffer_c WRITE
    vkt::CommandBuffer secondary_cb3(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary_cb3.Begin();
    secondary_cb3.Copy(buffer_b, buffer_c);
    secondary_cb3.End();

    m_command_buffer.Begin();
    VkCommandBuffer three_cbs[3] = {secondary_cb1, secondary_cb2, secondary_cb3};
    vk::CmdExecuteCommands(m_command_buffer, 3, three_cbs);
    m_command_buffer.End();
}

TEST_F(PositiveSyncVal, CmdClearAttachmentLayer) {
    TEST_DESCRIPTION(
        "Clear one attachment layer and copy to a different one."
        "This checks for bug regression that produced a false-positive WAW hazard.");

    // VK_EXT_load_store_op_none is needed to disable render pass load/store accesses, so clearing
    // attachment inside a render pass can create hazards with the copy operations outside render pass.
    AddRequiredExtensions(VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME);

    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    const uint32_t width = 256;
    const uint32_t height = 128;
    const uint32_t layers = 2;
    const VkFormat rt_format = VK_FORMAT_B8G8R8A8_UNORM;
    const auto transfer_usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    const auto rt_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | transfer_usage;

    vkt::Image image(*m_device, width, height, 1, rt_format, transfer_usage);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::Image rt(*m_device, vkt::Image::ImageCreateInfo2D(width, height, 1, layers, rt_format, rt_usage));
    rt.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    auto attachment_without_load_store = [](VkFormat format) {
        VkAttachmentDescription attachment = {};
        attachment.format = format;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_NONE;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_NONE;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_NONE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_NONE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
        attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
        return attachment;
    };
    const VkAttachmentDescription attachment = attachment_without_load_store(rt_format);
    const VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_GENERAL};

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_ref;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attachment;
    vkt::RenderPass render_pass(*m_device, rpci);

    vkt::ImageView rt_view = rt.CreateView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, 0, 1, 0, layers);
    VkFramebufferCreateInfo fbci = vku::InitStructHelper();
    fbci.flags = 0;
    fbci.renderPass = render_pass;
    fbci.attachmentCount = 1;
    fbci.pAttachments = &rt_view.handle();
    fbci.width = width;
    fbci.height = height;
    fbci.layers = layers;
    vkt::Framebuffer framebuffer(*m_device, fbci);

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.renderPass = render_pass;
    pipe.CreateGraphicsPipeline();

    VkImageCopy copy_region = {};
    copy_region.srcSubresource = {VkImageAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT), 0, 0, 1};
    copy_region.dstSubresource = {VkImageAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT), 0, 0 /* copy only to layer 0 */, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {width, height, 1};

    VkClearRect clear_rect = {};
    clear_rect.rect.offset = {0, 0};
    clear_rect.rect.extent = {width, height};
    clear_rect.baseArrayLayer = 1;  // skip layer 0, clear only layer 1
    clear_rect.layerCount = 1;
    const VkClearAttachment clear_attachment = {VK_IMAGE_ASPECT_COLOR_BIT};

    m_command_buffer.Begin();
    // Write 1: Copy to render target's layer 0
    vk::CmdCopyImage(m_command_buffer, image, VK_IMAGE_LAYOUT_GENERAL, rt, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_command_buffer.BeginRenderPass(render_pass, framebuffer, width, height);
    // Write 2: Clear render target's layer 1
    vk::CmdClearAttachments(m_command_buffer, 1, &clear_attachment, 1, &clear_rect);
    vk::CmdEndRenderPass(m_command_buffer);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

// Image transition ensures that image data is made visible and available when necessary.
// The user's responsibility is only to properly define the barrier. This test checks that
// writing to the image immediately after the transition  does not produce WAW hazard against
// the writes performed by the transition.
TEST_F(PositiveSyncVal, WriteToImageAfterTransition) {
    TEST_DESCRIPTION("Perform image transition then copy to image from buffer");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    constexpr uint32_t width = 256;
    constexpr uint32_t height = 128;
    constexpr VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;

    vkt::Buffer buffer(*m_device, width * height * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Image image(*m_device, width, height, 1, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    VkImageMemoryBarrier barrier = vku::InitStructHelper();
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.layerCount = 1;

    VkBufferImageCopy region = {};
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageExtent = {width, height, 1};

    m_command_buffer.Begin();
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr,
                           1, &barrier);
    vk::CmdCopyBufferToImage(m_command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    m_command_buffer.End();
}

// Regression test for vkWaitSemaphores timeout while waiting for vkSignalSemaphore.
// This scenario is not an issue for validation objects that use fine grained locking.
// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4968
TEST_F(PositiveSyncVal, SignalAndWaitSemaphoreOnHost) {
    TEST_DESCRIPTION("Signal semaphore on the host and wait on the host");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitSyncValFramework());
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(InitState());
    if (IsPlatformMockICD()) {
        // Mock does not support proper ordering of events, e.g. wait can return before signal
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    constexpr uint64_t max_signal_value = 10'000;

    VkSemaphoreTypeCreateInfo semaphore_type_info = vku::InitStructHelper();
    semaphore_type_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    const VkSemaphoreCreateInfo create_info = vku::InitStructHelper(&semaphore_type_info);
    vkt::Semaphore semaphore(*m_device, create_info);

    std::atomic<bool> bailout{false};
    m_errorMonitor->SetBailout(&bailout);

    // Send signals
    auto signaling_thread = std::thread{[&] {
        uint64_t last_signalled_value = 0;
        while (last_signalled_value != max_signal_value) {
            VkSemaphoreSignalInfo signal_info = vku::InitStructHelper();
            signal_info.semaphore = semaphore;
            signal_info.value = ++last_signalled_value;
            ASSERT_EQ(VK_SUCCESS, vk::SignalSemaphore(*m_device, &signal_info));
            if (bailout.load()) {
                break;
            }
        }
    }};
    // Wait for each signal
    uint64_t wait_value = 1;
    while (wait_value <= max_signal_value) {
        VkSemaphoreWaitInfo wait_info = vku::InitStructHelper();
        wait_info.flags = VK_SEMAPHORE_WAIT_ANY_BIT;
        wait_info.semaphoreCount = 1;
        wait_info.pSemaphores = &semaphore.handle();
        wait_info.pValues = &wait_value;
        ASSERT_EQ(VK_SUCCESS, vk::WaitSemaphores(*m_device, &wait_info, vvl::kU64Max));
        ++wait_value;
        if (bailout.load()) {
            break;
        }
    }
    signaling_thread.join();
}

// Regression test for vkGetSemaphoreCounterValue timeout while waiting for vkSignalSemaphore.
// This scenario is not an issue for validation objects that use fine grained locking.
// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4968
TEST_F(PositiveSyncVal, SignalAndGetSemaphoreCounter) {
    TEST_DESCRIPTION("Singal semaphore on the host and regularly read semaphore payload value");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitSyncValFramework());
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(InitState());
    if (IsPlatformMockICD()) {
        // Mock does not support precise semaphore counter reporting
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    constexpr uint64_t max_signal_value = 1'000;

    VkSemaphoreTypeCreateInfo semaphore_type_info = vku::InitStructHelper();
    semaphore_type_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    const VkSemaphoreCreateInfo create_info = vku::InitStructHelper(&semaphore_type_info);
    vkt::Semaphore semaphore(*m_device, create_info);

    std::atomic<bool> bailout{false};
    m_errorMonitor->SetBailout(&bailout);

    // Send signals
    auto signaling_thread = std::thread{[&] {
        uint64_t last_signalled_value = 0;
        while (last_signalled_value != max_signal_value) {
            VkSemaphoreSignalInfo signal_info = vku::InitStructHelper();
            signal_info.semaphore = semaphore;
            signal_info.value = ++last_signalled_value;
            ASSERT_EQ(VK_SUCCESS, vk::SignalSemaphore(*m_device, &signal_info));
            if (bailout.load()) {
                break;
            }
        }
    }};
    // Spin until semaphore payload value equals maximum signaled value
    uint64_t counter = 0;
    while (counter != max_signal_value) {
        ASSERT_EQ(VK_SUCCESS, vk::GetSemaphoreCounterValue(*m_device, semaphore, &counter));
        if (bailout.load()) {
            break;
        }
    }
    signaling_thread.join();
}

TEST_F(PositiveSyncVal, GetSemaphoreCounterFromMultipleThreads) {
    TEST_DESCRIPTION("Read semaphore counter value from multiple threads");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitSyncValFramework());
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(InitState());
    if (IsPlatformMockICD()) {
        // Mock does not support precise semaphore counter reporting
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    constexpr uint64_t max_signal_value = 5'000;  // 15'000 in the original version for stress testing
    constexpr int waiter_count = 8;               // 24 in the original version for stress testing

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    ThreadTimeoutHelper timeout_helper(waiter_count + 1 /* signaling thread*/);

    std::atomic<bool> bailout{false};
    m_errorMonitor->SetBailout(&bailout);

    // Start a bunch of waiter threads
    auto waiting_thread = [&]() {
        auto timeout_guard = timeout_helper.ThreadGuard();
        uint64_t counter = 0;
        while (counter != max_signal_value) {
            ASSERT_EQ(VK_SUCCESS, vk::GetSemaphoreCounterValue(*m_device, semaphore, &counter));
            if (bailout.load()) {
                break;
            }
        }
    };
    std::vector<std::thread> waiters;
    for (int i = 0; i < waiter_count; i++) {
        waiters.emplace_back(waiting_thread);
    }
    // The signaling thread advances semaphore's payload value
    auto signaling_thread = std::thread([&] {
        auto timeout_guard = timeout_helper.ThreadGuard();
        uint64_t last_signalled_value = 0;
        while (last_signalled_value != max_signal_value) {
            VkSemaphoreSignalInfo signal_info = vku::InitStructHelper();
            signal_info.semaphore = semaphore;
            signal_info.value = ++last_signalled_value;
            ASSERT_EQ(VK_SUCCESS, vk::SignalSemaphore(*m_device, &signal_info));
            if (bailout.load()) {
                break;
            }
        }
    });

    constexpr int wait_time = 100;
    if (!timeout_helper.WaitForThreads(wait_time)) {
        ADD_FAILURE() << "The waiting time for the worker threads exceeded the maximum limit: " << wait_time << " seconds.";
        bailout.store(true);
    }
    for (auto &waiter : waiters) {
        waiter.join();
    }
    signaling_thread.join();
}

TEST_F(PositiveSyncVal, ShaderReferencesNotBoundSet) {
    TEST_DESCRIPTION("Shader references a descriptor set that was not bound. SyncVal should not crash if core checks are disabled");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
    const vkt::DescriptorSetLayout set_layout(*m_device, {binding});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&set_layout, &set_layout});
    OneOffDescriptorSet set(m_device, {binding});

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    // Bind set 0.
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &set.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    // Core checks prevent SyncVal from running when error is found. This test has core checks disabled and also invalid
    // setup where a shader uses not bound set 1.
    // Check that syncval does not cause out of bounds access (PerSet has single element (index 0), shader set index is 1).
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);

    vk::CmdEndRenderPass(m_command_buffer);
    m_command_buffer.End();
}

TEST_F(PositiveSyncVal, SeparateAvailabilityAndVisibilityForBuffer) {
    TEST_DESCRIPTION("Use separate barriers for availability and visibility operations.");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    constexpr VkDeviceSize size = 1024;
    const vkt::Buffer staging_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    const vkt::Buffer buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferCopy region = {};
    region.size = size;

    m_command_buffer.Begin();
    // Perform a copy
    vk::CmdCopyBuffer(m_command_buffer, staging_buffer, buffer, 1, &region);

    // Make writes available
    VkBufferMemoryBarrier barrier_a = vku::InitStructHelper();
    barrier_a.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier_a.dstAccessMask = 0;
    barrier_a.buffer = buffer;
    barrier_a.size = VK_WHOLE_SIZE;
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1,
                           &barrier_a, 0, nullptr);

    // Make writes visible
    VkBufferMemoryBarrier barrier_b = vku::InitStructHelper();
    barrier_b.srcAccessMask = 0;  // already available
    barrier_b.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier_b.buffer = buffer;
    barrier_b.size = VK_WHOLE_SIZE;
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1,
                           &barrier_b, 0, nullptr);

    // Perform one more copy. Should not generate WAW.
    vk::CmdCopyBuffer(m_command_buffer, staging_buffer, buffer, 1, &region);
    m_command_buffer.End();
}

TEST_F(PositiveSyncVal, LayoutTransitionWithAlreadyAvailableImage) {
    TEST_DESCRIPTION(
        "Image barrier makes image available but not visible. A subsequent layout transition barrier should not generate hazards. "
        "Available memory is automatically made visible to a layout transition.");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    constexpr VkDeviceSize buffer_size = 64 * 64 * 4;
    const vkt::Buffer buffer(*m_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    vkt::Image image(*m_device, 64, 64, 1, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    m_command_buffer.Begin();

    // Copy data from buffer to image
    VkBufferImageCopy region = {};
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageExtent = {64, 64, 1};
    vk::CmdCopyBufferToImage(m_command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // Make writes available
    VkImageMemoryBarrier barrier_a = vku::InitStructHelper();
    barrier_a.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier_a.dstAccessMask = 0;
    barrier_a.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier_a.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier_a.image = image;
    barrier_a.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                           nullptr, 1, &barrier_a);

    // Transition to new layout. Available memory should automatically be made visible to the layout transition.
    VkImageMemoryBarrier barrier_b = vku::InitStructHelper();
    barrier_b.srcAccessMask = 0;  // already available
    barrier_b.dstAccessMask = 0;  // for this test we don't care if the memory is visible after the transition or not
    barrier_b.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier_b.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier_b.image = image;
    barrier_b.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &barrier_b);
    m_command_buffer.End();
}

TEST_F(PositiveSyncVal, ImageArrayDynamicIndexing) {
    TEST_DESCRIPTION("Access different elements of the image array using dynamic indexing. There should be no hazards");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::shaderStorageImageArrayNonUniformIndexing);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    AddRequiredFeature(vkt::Feature::fragmentStoresAndAtomics);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    constexpr VkDescriptorType descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    constexpr VkImageLayout image_layout = VK_IMAGE_LAYOUT_GENERAL;

    vkt::Image images[4];
    vkt::ImageView views[4];
    for (int i = 0; i < 4; i++) {
        images[i].Init(*m_device, 64, 64, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT);
        ASSERT_TRUE(images[i].initialized());
        images[i].SetLayout(image_layout);
        views[i] = images[i].CreateView();
    }

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, descriptor_type, 4, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    descriptor_set.WriteDescriptorImageInfo(0, views[0], VK_NULL_HANDLE, descriptor_type, image_layout, 0);
    descriptor_set.WriteDescriptorImageInfo(0, views[1], VK_NULL_HANDLE, descriptor_type, image_layout, 1);
    descriptor_set.WriteDescriptorImageInfo(0, views[2], VK_NULL_HANDLE, descriptor_type, image_layout, 2);
    descriptor_set.WriteDescriptorImageInfo(0, views[3], VK_NULL_HANDLE, descriptor_type, image_layout, 3);
    descriptor_set.UpdateDescriptorSets();

    // Write to image 0 or 1
    const char fsSource[] = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set=0, binding=0, rgba8) uniform image2D image_array[];
        void main() {
           imageStore(image_array[nonuniformEXT(int(gl_FragCoord.x) % 2)], ivec2(1, 1), vec4(1.0, 0.5, 0.2, 1.0));
        }
    )glsl";
    const VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
    CreatePipelineHelper gfx_pipe(*this);
    gfx_pipe.shader_stages_ = {gfx_pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    gfx_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    gfx_pipe.CreateGraphicsPipeline();

    // Read from image 2 or 3
    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set=0, binding=0, rgba8) uniform image2D image_array[];
        void main() {
            vec4 data = imageLoad(image_array[nonuniformEXT(2 + (gl_LocalInvocationID.x % 2))], ivec2(1, 1));
        }
    )glsl";
    CreateComputePipelineHelper cs_pipe(*this);
    cs_pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    cs_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    cs_pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    // Graphics pipeline writes
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gfx_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gfx_pipe.pipeline_layout_, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    // Compute pipeline reads
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipe.pipeline_layout_, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveSyncVal, ImageArrayConstantIndexing) {
    TEST_DESCRIPTION("Access different elements of the image array using constant indices. There should be no hazards");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::fragmentStoresAndAtomics);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::Image image(*m_device, 64, 64, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    const vkt::ImageView view = image.CreateView();

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    descriptor_set.WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL, 0);
    descriptor_set.WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL, 1);
    descriptor_set.UpdateDescriptorSets();

    // Write to image 0
    const char fsSource[] = R"glsl(
        #version 450
        layout(set=0, binding=0, rgba8) uniform image2D image_array[];
        void main() {
           imageStore(image_array[0], ivec2(1, 1), vec4(1.0, 0.5, 0.2, 1.0));
        }
    )glsl";
    const VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
    CreatePipelineHelper gfx_pipe(*this);
    gfx_pipe.shader_stages_ = {gfx_pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    gfx_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    gfx_pipe.CreateGraphicsPipeline();

    // Read from image 1
    char const *cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0, rgba8) uniform image2D image_array[];
        void main() {
            vec4 data = imageLoad(image_array[1], ivec2(1, 1));
        }
    )glsl";
    CreateComputePipelineHelper cs_pipe(*this);
    cs_pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    cs_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    cs_pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    // Graphics pipeline writes
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gfx_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gfx_pipe.pipeline_layout_, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    // Compute pipeline reads
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipe.pipeline_layout_, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveSyncVal, TexelBufferArrayConstantIndexing) {
    TEST_DESCRIPTION("Access different elements of the texel buffer array using constant indices. There should be no hazards");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::fragmentStoresAndAtomics);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    const vkt::Buffer buffer0(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
    const vkt::Buffer buffer1(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
    const vkt::BufferView buffer_view0(*m_device, vkt::BufferView::CreateInfo(buffer0, VK_FORMAT_R8G8B8A8_UINT));
    const vkt::BufferView buffer_view1(*m_device, vkt::BufferView::CreateInfo(buffer1, VK_FORMAT_R8G8B8A8_UINT));

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    descriptor_set.WriteDescriptorBufferView(0, buffer_view0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 0);
    descriptor_set.WriteDescriptorBufferView(0, buffer_view1, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1);
    descriptor_set.UpdateDescriptorSets();

    // Write to texel buffer 0
    const char fsSource[] = R"glsl(
        #version 450
        layout(set=0, binding=0, rgba8ui) uniform uimageBuffer texel_buffer_array[];
        void main() {
           imageStore(texel_buffer_array[0], 0, uvec4(1, 2, 3, 42));
        }
    )glsl";
    const VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
    CreatePipelineHelper gfx_pipe(*this);
    gfx_pipe.shader_stages_ = {gfx_pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    gfx_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    gfx_pipe.CreateGraphicsPipeline();

    // Read from texel buffer 1
    char const *cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0, rgba8ui) uniform uimageBuffer texel_buffer_array[];
        void main() {
            uvec4 data = imageLoad(texel_buffer_array[1], 0);
        }
    )glsl";
    CreateComputePipelineHelper cs_pipe(*this);
    cs_pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    cs_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    cs_pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    // Graphics pipeline writes
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gfx_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gfx_pipe.pipeline_layout_, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    // Compute pipeline reads
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipe.pipeline_layout_, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveSyncVal, QSBufferCopyHazardsDisabled) {
    SyncValSettings settings;
    settings.submit_time_validation = false;
    RETURN_IF_SKIP(InitSyncValFramework(&settings));
    RETURN_IF_SKIP(InitState());

    QSTestContext test(m_device, m_device->QueuesWithGraphicsCapability()[0]);
    if (!test.Valid()) {
        GTEST_SKIP() << "Test requires a valid queue object.";
    }

    test.RecordCopy(test.cba, test.buffer_a, test.buffer_b);
    test.RecordCopy(test.cbb, test.buffer_c, test.buffer_a);

    VkSubmitInfo submit1 = vku::InitStructHelper();
    submit1.commandBufferCount = 2;
    VkCommandBuffer two_cbs[2] = {test.h_cba, test.h_cbb};
    submit1.pCommandBuffers = two_cbs;

    // This should be a hazard if we didn't disable it at InitSyncValFramework time
    vk::QueueSubmit(test.q0, 1, &submit1, VK_NULL_HANDLE);

    test.DeviceWait();
}

TEST_F(PositiveSyncVal, QSTransitionWithSrcNoneStage) {
    TEST_DESCRIPTION(
        "Two submission batches synchronized with binary semaphore. Layout transition in the second batch should not interfere "
        "with image read in the previous batch.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Image image(*m_device, 64, 64, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView();

    vkt::Semaphore semaphore(*m_device);

    // Submit 0: shader reads image data (read access)
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr}});
    descriptor_set.WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.UpdateDescriptorSets();

    char const *cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0, rgba8) uniform image2D image;
        void main() {
            vec4 data = imageLoad(image, ivec2(1, 1));
        }
    )glsl";
    CreateComputePipelineHelper cs_pipe(*this);
    cs_pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    cs_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    cs_pipe.CreateComputePipeline();

    vkt::CommandBuffer cb(*m_device, m_command_pool);
    cb.Begin();
    vk::CmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipe.Handle());
    vk::CmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipe.pipeline_layout_, 0, 1, &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(cb, 1, 1, 1);
    cb.End();

    m_default_queue->Submit2(cb, vkt::Signal(semaphore));

    // Submit 1: transition image layout (write access)
    VkImageMemoryBarrier2 layout_transition = vku::InitStructHelper();
    // NOTE: this test check that using NONE as source stage for transition works correctly.
    // Wait on ALL_COMMANDS semaphore should protect this submission from previous accesses.
    layout_transition.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    layout_transition.srcAccessMask = 0;
    layout_transition.dstAccessMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    layout_transition.dstAccessMask = 0;
    layout_transition.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    layout_transition.image = image;
    layout_transition.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &layout_transition;

    vkt::CommandBuffer cb2(*m_device, m_command_pool);
    cb2.Begin();
    vk::CmdPipelineBarrier2(cb2, &dep_info);
    cb2.End();

    m_default_queue->Submit2(cb2, vkt::Wait(semaphore));
    m_default_queue->Wait();
}

TEST_F(PositiveSyncVal, QSTransitionWithSrcNoneStage2) {
    TEST_DESCRIPTION(
        "Two submission batches synchronized with binary semaphore. Layout transition in the second batch should not interfere "
        "with image write in the first batch.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Image image(*m_device, 64, 64, 1, VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageMemoryBarrier2 layout_transition = vku::InitStructHelper();
    layout_transition.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    layout_transition.srcAccessMask = VK_ACCESS_2_NONE;
    layout_transition.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    layout_transition.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    layout_transition.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    layout_transition.image = image;
    layout_transition.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &layout_transition;

    vkt::Semaphore semaphore(*m_device);

    // Submit 1: Clear image (WRITE access)
    vkt::CommandBuffer cb(*m_device, m_command_pool);
    cb.Begin();
    vk::CmdClearColorImage(cb, image, VK_IMAGE_LAYOUT_GENERAL, &m_clear_color, 1, &layout_transition.subresourceRange);
    cb.End();
    m_default_queue->Submit2(cb, vkt::Signal(semaphore));

    // Submit 2: Transition layout (WRITE access)
    vkt::CommandBuffer cb2(*m_device, m_command_pool);
    cb2.Begin();
    vk::CmdPipelineBarrier2(cb2, &dep_info);
    cb2.End();
    m_default_queue->Submit2(cb2, vkt::Wait(semaphore));
    m_default_queue->Wait();
}

TEST_F(PositiveSyncVal, QSTransitionAndRead) {
    TEST_DESCRIPTION("Transition and read image in different submits synchronized via ALL_COMMANDS semaphore");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Image image(*m_device, 64, 64, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT);
    vkt::ImageView view = image.CreateView();

    vkt::Semaphore semaphore(*m_device);

    // Submit0: transition image and signal semaphore with ALL_COMMANDS scope
    VkImageMemoryBarrier2 layout_transition = vku::InitStructHelper();
    layout_transition.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    layout_transition.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    layout_transition.dstAccessMask = VK_PIPELINE_STAGE_2_NONE;
    layout_transition.dstAccessMask = 0;
    layout_transition.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    layout_transition.image = image;
    layout_transition.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &layout_transition;

    vkt::CommandBuffer cb(*m_device, m_command_pool);
    cb.Begin();
    vk::CmdPipelineBarrier2(cb, &dep_info);
    cb.End();
    m_default_queue->Submit2(cb, vkt::Signal(semaphore));

    // Submit1: wait for the semaphore and read image in the shader
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr}});
    descriptor_set.WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.UpdateDescriptorSets();

    char const *cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0, rgba8) uniform image2D image;
        void main() {
            vec4 data = imageLoad(image, ivec2(1, 1));
        }
    )glsl";
    CreateComputePipelineHelper cs_pipe(*this);
    cs_pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    cs_pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    cs_pipe.CreateComputePipeline();

    vkt::CommandBuffer cb2(*m_device, m_command_pool);
    cb2.Begin();
    vk::CmdBindPipeline(cb2, VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipe.Handle());
    vk::CmdBindDescriptorSets(cb2, VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipe.pipeline_layout_, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(cb2, 1, 1, 1);
    cb2.End();
    m_default_queue->Submit2(cb2, vkt::Wait(semaphore));
    m_default_queue->Wait();
}

TEST_F(PositiveSyncVal, DynamicRenderingColorResolve) {
    TEST_DESCRIPTION("Test color resolve with dynamic rendering");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    const uint32_t width = 64;
    const uint32_t height = 64;
    const VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    const VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    auto color_ci = vkt::Image::ImageCreateInfo2D(width, height, 1, 1, color_format, usage);
    color_ci.samples = VK_SAMPLE_COUNT_4_BIT;  // guaranteed by framebufferColorSampleCounts
    vkt::Image color_image(*m_device, color_ci, vkt::set_layout);
    color_image.SetLayout(VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
    vkt::ImageView color_image_view = color_image.CreateView(VK_IMAGE_ASPECT_COLOR_BIT);

    auto color_resolved_ci = vkt::Image::ImageCreateInfo2D(width, height, 1, 1, color_format, usage);
    vkt::Image color_resolved_image(*m_device, color_resolved_ci, vkt::set_layout);
    color_resolved_image.SetLayout(VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
    vkt::ImageView color_resolved_image_view = color_resolved_image.CreateView(VK_IMAGE_ASPECT_COLOR_BIT);

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = color_image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    color_attachment.resolveImageView = color_resolved_image_view;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.clearValue.color = m_clear_color;

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea.extent = {width, height};
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.Begin();
    vk::CmdBeginRendering(m_command_buffer, &rendering_info);
    vk::CmdEndRendering(m_command_buffer);
    m_command_buffer.End();
}

TEST_F(PositiveSyncVal, DynamicRenderingDepthResolve) {
    TEST_DESCRIPTION("Test depth resolve with dynamic rendering");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    VkPhysicalDeviceDepthStencilResolveProperties resolve_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(resolve_properties);
    if ((resolve_properties.supportedDepthResolveModes & VK_RESOLVE_MODE_MIN_BIT) == 0) {
        GTEST_SKIP() << "VK_RESOLVE_MODE_MIN_BIT not supported";
    }

    const uint32_t width = 64;
    const uint32_t height = 64;
    const VkFormat depth_format = FindSupportedDepthOnlyFormat(Gpu());
    const VkImageUsageFlags usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    auto depth_ci = vkt::Image::ImageCreateInfo2D(width, height, 1, 1, depth_format, usage);
    depth_ci.samples = VK_SAMPLE_COUNT_4_BIT;  // guaranteed by framebufferDepthSampleCounts
    vkt::Image depth_image(*m_device, depth_ci, vkt::set_layout);
    depth_image.SetLayout(VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
    vkt::ImageView depth_image_view = depth_image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);

    auto depth_resolved_ci = vkt::Image::ImageCreateInfo2D(width, height, 1, 1, depth_format, usage);
    vkt::Image depth_resolved_image(*m_device, depth_resolved_ci, vkt::set_layout);
    depth_resolved_image.SetLayout(VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
    vkt::ImageView depth_resolved_image_view = depth_resolved_image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);

    VkRenderingAttachmentInfo depth_attachment = vku::InitStructHelper();
    depth_attachment.imageView = depth_image_view;
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    depth_attachment.resolveMode = VK_RESOLVE_MODE_MIN_BIT;
    depth_attachment.resolveImageView = depth_resolved_image_view;
    depth_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.clearValue.depthStencil.depth = 1.0f;

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea.extent = {width, height};
    rendering_info.layerCount = 1;
    rendering_info.pDepthAttachment = &depth_attachment;

    m_command_buffer.Begin();
    vk::CmdBeginRendering(m_command_buffer, &rendering_info);
    vk::CmdEndRendering(m_command_buffer);
    m_command_buffer.End();
}

TEST_F(PositiveSyncVal, FillBuffer) {
    TEST_DESCRIPTION("Synchronize with vkCmdFillBuffer assuming that its writes happen on the CLEAR stage");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    constexpr VkDeviceSize size = 1024;
    vkt::Buffer src_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer dst_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferCopy region{};
    region.size = size;

    VkBufferMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_CLEAR_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
    barrier.buffer = src_buffer;
    barrier.size = size;

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.bufferMemoryBarrierCount = 1;
    dep_info.pBufferMemoryBarriers = &barrier;

    m_command_buffer.Begin();
    vk::CmdFillBuffer(m_command_buffer, src_buffer, 0, size, 42);
    vk::CmdPipelineBarrier2(m_command_buffer, &dep_info);
    vk::CmdCopyBuffer(m_command_buffer, src_buffer, dst_buffer, 1, &region);
    m_command_buffer.End();
}

TEST_F(PositiveSyncVal, UpdateBuffer) {
    TEST_DESCRIPTION("Synchronize with vkCmdUpdateBuffer assuming that its writes happen on the CLEAR stage");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    constexpr VkDeviceSize size = 64;
    vkt::Buffer src_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer dst_buffer(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    std::array<uint8_t, size> data = {};

    VkBufferCopy region{};
    region.size = size;

    VkBufferMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_CLEAR_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
    barrier.buffer = src_buffer;
    barrier.size = size;

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.bufferMemoryBarrierCount = 1;
    dep_info.pBufferMemoryBarriers = &barrier;

    m_command_buffer.Begin();
    vk::CmdUpdateBuffer(m_command_buffer, src_buffer, 0, static_cast<VkDeviceSize>(data.size()), data.data());
    vk::CmdPipelineBarrier2(m_command_buffer, &dep_info);
    vk::CmdCopyBuffer(m_command_buffer, src_buffer, dst_buffer, 1, &region);
    m_command_buffer.End();
}

TEST_F(PositiveSyncVal, QSSynchronizedWritesAndAsyncWait) {
    TEST_DESCRIPTION(
        "Graphics queue: Image transition and subsequent image write are synchronized using a pipeline barrier. Transfer queue: "
        "waits for the image layout transition using a semaphore. This should not affect pipeline barrier on the graphics queue");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Queue *transfer_queue = m_device->TransferOnlyQueue();
    if (!transfer_queue) {
        GTEST_SKIP() << "Transfer-only queue is not present";
    }

    vkt::Image image(*m_device, 64, 64, 1, VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer buffer(*m_device, 64 * 64, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    VkBufferImageCopy region = {};
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageExtent = {64, 64, 1};

    vkt::Semaphore semaphore(*m_device);

    // Submit 0: perform image layout transition on Graphics queue.
    // Image barrier synchronizes with COPY-WRITE access from Submit 2.
    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    cb0.Begin();
    VkImageMemoryBarrier2 image_barrier = vku::InitStructHelper();
    image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    image_barrier.srcAccessMask = VK_ACCESS_2_NONE;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    image_barrier.image = image;
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &image_barrier;
    vk::CmdPipelineBarrier2(cb0, &dep_info);
    cb0.End();
    m_default_queue->Submit2(cb0, vkt::Signal(semaphore));

    // Submit 1: empty submit on Transfer queue that waits for Submit 0.
    transfer_queue->Submit2(vkt::no_cmd, vkt::Wait(semaphore));

    // Submit 2: copy to image on Graphics queue. No synchronization is needed because of COPY+WRITE barrier from Submit 0.
    vkt::CommandBuffer cb2(*m_device, m_command_pool);
    cb2.Begin();
    vk::CmdCopyBufferToImage(cb2, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    cb2.End();
    m_default_queue->Submit2(cb2);

    m_default_queue->Wait();
    transfer_queue->Wait();
}

// TODO:
// It has to be this test found a bug in the existing code, so it's disabled until it's fixed.
// Draw access happen-after loadOp access so it's enough to synchronize with FRAGMENT_SHADER read.
// last_reads contains both loadOp read access and draw (fragment shader) read access. The code
// synchronizes only with draw (FRAGMENT_SHADER) but not with loadOp (COLOR_ATTACHMENT_OUTPUT),
// and the syncval complains that COLOR_ATTACHMENT_OUTPUT access is not synchronized.
TEST_F(PositiveSyncVal, DISABLED_RenderPassStoreOpNone) {
    TEST_DESCRIPTION("Synchronization with draw command when render pass uses storeOp=NONE");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    const VkImageLayout input_attachment_layout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(format, input_attachment_layout, input_attachment_layout, VK_ATTACHMENT_LOAD_OP_LOAD,
                                VK_ATTACHMENT_STORE_OP_NONE);
    rp.AddAttachmentReference({0, input_attachment_layout});
    rp.AddInputAttachment(0);
    rp.CreateRenderPass();

    vkt::Image image(*m_device, 32, 32, 1, format, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    image.SetLayout(input_attachment_layout);
    vkt::ImageView image_view = image.CreateView();
    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &image_view.handle());

    VkImageMemoryBarrier2 layout_transition = vku::InitStructHelper();
    // Form an execution dependency with draw command (FRAGMENT_SHADER). Execution dependency is enough to sync with READ.
    layout_transition.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    layout_transition.srcAccessMask = VK_ACCESS_2_NONE;
    layout_transition.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
    layout_transition.dstAccessMask = VK_ACCESS_2_NONE;
    layout_transition.oldLayout = input_attachment_layout;
    layout_transition.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    layout_transition.image = image;
    layout_transition.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &layout_transition;

    // Fragment shader READs input attachment.
    VkShaderObj fs(this, kFragmentSubpassLoadGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    OneOffDescriptorSet descriptor_set(m_device, {binding});
    descriptor_set.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                                            input_attachment_layout);
    descriptor_set.UpdateDescriptorSets();
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.gp_ci_.renderPass = rp.Handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), fb);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDraw(m_command_buffer, 1, 0, 0, 0);
    m_command_buffer.EndRenderPass();

    // This waits for the FRAGMENT_SHADER read before starting with transition.
    // If storeOp other than NONE was used we had to wait for it instead.
    vk::CmdPipelineBarrier2(m_command_buffer, &dep_info);
    m_command_buffer.End();
}

TEST_F(PositiveSyncVal, ThreadedSubmitAndFenceWait) {
    TEST_DESCRIPTION("Minimal version of https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7250");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    constexpr int N = 100;

    vkt::Buffer src(*m_device, 1024, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer dst(*m_device, 1024, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    VkBufferCopy copy_info{};
    copy_info.size = 1024;

    // Allocate array of fences instead of calling ResetFences to minimize the number of
    // API functions used by this test (leave only those that are part of the regression scenario).
    std::vector<vkt::Fence> fences;
    std::vector<vkt::Fence> thread_fences;
    fences.reserve(N);
    thread_fences.reserve(N);
    for (int i = 0; i < N; i++) {
        fences.emplace_back(*m_device);
        thread_fences.emplace_back(*m_device);
    }

    vkt::CommandBuffer cmd(*m_device, m_command_pool);
    cmd.Begin();
    vk::CmdCopyBuffer(cmd, src, dst, 1, &copy_info);
    cmd.End();

    vkt::CommandBuffer thread_cmd(*m_device, m_command_pool);
    thread_cmd.Begin();
    thread_cmd.End();

    std::mutex queue_mutex;
    std::mutex queue_mutex2;

    // Worker thread runs "submit empty buffer and wait" loop.
    std::thread thread([&] {
        for (int i = 0; i < N; i++) {
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                m_default_queue->Submit(thread_cmd, thread_fences[i]);
            }
            {
                // WaitForFences does not require external synchronization.
                // queue_mutex2 is not needed for correctness, but it was added to decrease
                // the number of degrees of freedom of this test, so it's easier to analyze it.
                std::unique_lock<std::mutex> lock(queue_mutex2);
                vk::WaitForFences(device(), 1, &thread_fences[i].handle(), VK_TRUE, kWaitTimeout);
            }
        }
    });
    // Main thread runs "submit accesses and wait" loop.
    {
        for (int i = 0; i < N; i++) {
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                m_default_queue->Submit(cmd, fences[i]);
            }
            {
                std::unique_lock<std::mutex> lock(queue_mutex2);
                vk::WaitForFences(device(), 1, &fences[i].handle(), VK_TRUE, kWaitTimeout);
            }
        }
    }
    thread.join();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/7713
TEST_F(PositiveSyncVal, CopyBufferToCompressedImage) {
    TEST_DESCRIPTION("Copy from a buffer to compressed image without overlap.");

    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    VkFormatProperties format_properties;
    VkFormat mp_format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    vk::GetPhysicalDeviceFormatProperties(Gpu(), mp_format, &format_properties);
    if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0) {
        GTEST_SKIP()
            << "Device does not support VK_FORMAT_FEATURE_TRANSFER_DST_BIT for VK_FORMAT_BC1_RGBA_UNORM_BLOCK, skipping test.\n";
    }

    const VkDeviceSize buffer_size = 32;  // enough for 8x8 BC1 region
    vkt::Buffer src_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkt::Image dst_image(*m_device, 16, 16, 1, mp_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    VkBufferImageCopy buffer_copy[2] = {};
    buffer_copy[0].imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    buffer_copy[0].imageOffset = {0, 0, 0};
    buffer_copy[0].imageExtent = {8, 8, 1};
    buffer_copy[1].imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    buffer_copy[1].imageOffset = {8, 0, 0};
    buffer_copy[1].imageExtent = {8, 8, 1};

    m_command_buffer.Begin();
    vk::CmdCopyBufferToImage(m_command_buffer, src_buffer, dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_copy[0]);
    vk::CmdCopyBufferToImage(m_command_buffer, src_buffer, dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_copy[1]);
    m_command_buffer.End();
}

TEST_F(PositiveSyncVal, CopyBufferToCompressedImageASTC) {
    TEST_DESCRIPTION("Copy from a buffer to 20x10 ASTC-compressed image without overlap.");

    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    VkFormatProperties format_properties;
    VkFormat format = VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
    vk::GetPhysicalDeviceFormatProperties(Gpu(), format, &format_properties);
    if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0) {
        GTEST_SKIP()
            << "Device does not support VK_FORMAT_FEATURE_TRANSFER_DST_BIT for VK_FORMAT_ASTC_10x10_UNORM_BLOCK, skipping test.\n";
    }

    const VkDeviceSize buffer_size = 32;  // enough for 20x10 ASTC_10x10 region
    vkt::Buffer src_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkt::Image dst_image(*m_device, 20, 10, 1, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    VkBufferImageCopy buffer_copy[2] = {};
    buffer_copy[0].imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    buffer_copy[0].imageOffset = {0, 0, 0};
    buffer_copy[0].imageExtent = {10, 10, 1};
    buffer_copy[1].imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    buffer_copy[1].imageOffset = {10, 0, 0};
    buffer_copy[1].imageExtent = {10, 10, 1};

    m_command_buffer.Begin();
    vk::CmdCopyBufferToImage(m_command_buffer, src_buffer, dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_copy[0]);
    vk::CmdCopyBufferToImage(m_command_buffer, src_buffer, dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_copy[1]);
    m_command_buffer.End();
}

TEST_F(PositiveSyncVal, CopyBufferToCompressedImageASTC2) {
    TEST_DESCRIPTION("Copy from a buffer to 10x20 ASTC-compressed image without overlap.");

    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    VkFormatProperties format_properties;
    VkFormat format = VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
    vk::GetPhysicalDeviceFormatProperties(Gpu(), format, &format_properties);
    if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0) {
        GTEST_SKIP()
            << "Device does not support VK_FORMAT_FEATURE_TRANSFER_DST_BIT for VK_FORMAT_ASTC_10x10_UNORM_BLOCK, skipping test.\n";
    }

    const VkDeviceSize buffer_size = 32;  // enough for 10x20 ASTC_10x10 region
    vkt::Buffer src_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkt::Image dst_image(*m_device, 10, 20, 1, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    VkBufferImageCopy buffer_copy[2] = {};
    buffer_copy[0].imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    buffer_copy[0].imageOffset = {0, 0, 0};
    buffer_copy[0].imageExtent = {10, 10, 1};
    buffer_copy[1].imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    buffer_copy[1].imageOffset = {0, 10, 0};
    buffer_copy[1].imageExtent = {10, 10, 1};

    m_command_buffer.Begin();
    vk::CmdCopyBufferToImage(m_command_buffer, src_buffer, dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_copy[0]);
    vk::CmdCopyBufferToImage(m_command_buffer, src_buffer, dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_copy[1]);
    m_command_buffer.End();
}

TEST_F(PositiveSyncVal, CopyBufferToCompressedImageASTC3) {
    TEST_DESCRIPTION("Copy from a buffer to 20x20 ASTC-compressed with overlap protected by a barrier.");

    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    VkFormatProperties format_properties;
    VkFormat format = VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
    vk::GetPhysicalDeviceFormatProperties(Gpu(), format, &format_properties);
    if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0) {
        GTEST_SKIP()
            << "Device does not support VK_FORMAT_FEATURE_TRANSFER_DST_BIT for VK_FORMAT_ASTC_10x10_UNORM_BLOCK, skipping test.\n";
    }

    const VkDeviceSize buffer_size = 64;  // enough for 20x20 ASTC_10x10 region
    vkt::Buffer src_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkt::Image dst_image(*m_device, 20, 20, 1, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    VkBufferImageCopy buffer_copy[2] = {};
    buffer_copy[0].imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    buffer_copy[0].imageOffset = {10, 10, 0};
    buffer_copy[0].imageExtent = {10, 10, 1};
    buffer_copy[1].imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    buffer_copy[1].imageOffset = {10, 0, 0};
    buffer_copy[1].imageExtent = {10, 20, 1};

    VkImageMemoryBarrier barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.image = dst_image;
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    m_command_buffer.Begin();
    vk::CmdCopyBufferToImage(m_command_buffer, src_buffer, dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_copy[0]);
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                           nullptr, 1, &barrier);
    vk::CmdCopyBufferToImage(m_command_buffer, src_buffer, dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_copy[1]);
    m_command_buffer.End();
}

TEST_F(PositiveSyncVal, SignalAndWaitSemaphoreOneQueueSubmit) {
    TEST_DESCRIPTION("Signal and wait semaphore using one submit command");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Semaphore semaphore(*m_device);

    VkSemaphoreSubmitInfo semaphore_info = vku::InitStructHelper();
    semaphore_info.semaphore = semaphore;
    semaphore_info.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    VkSubmitInfo2 submits[2];
    submits[0] = vku::InitStructHelper();
    submits[0].signalSemaphoreInfoCount = 1;
    submits[0].pSignalSemaphoreInfos = &semaphore_info;

    submits[1] = vku::InitStructHelper();
    submits[1].waitSemaphoreInfoCount = 1;
    submits[1].pWaitSemaphoreInfos = &semaphore_info;

    vk::QueueSubmit2(*m_default_queue, 2, submits, VK_NULL_HANDLE);
    m_default_queue->Wait();
}

TEST_F(PositiveSyncVal, SignalUnsignalSignalMultipleSubmits) {
    TEST_DESCRIPTION("Create a sequence that at some point unsignals and then signals a semaphore through separate submit calls");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Semaphore semaphore(*m_device);

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();

    vkt::CommandBuffer command_buffer2(*m_device, m_command_pool);
    command_buffer2.Begin();
    command_buffer2.Copy(buffer_a, buffer_b);
    command_buffer2.End();

    m_default_queue->Submit2(vkt::no_cmd, vkt::Signal(semaphore));
    m_default_queue->Submit2(m_command_buffer, vkt::Wait(semaphore), vkt::Signal(semaphore));
    m_default_queue->Submit2(command_buffer2, vkt::Wait(semaphore));
    m_default_queue->Wait();
}

TEST_F(PositiveSyncVal, SignalUnsignalSignalSingleSubmit) {
    TEST_DESCRIPTION("Create a sequence that at some point unsignals and then signals a semaphore using a single submit call");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Semaphore semaphore(*m_device);
    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();

    vkt::CommandBuffer command_buffer2(*m_device, m_command_pool);
    command_buffer2.Begin();
    command_buffer2.Copy(buffer_a, buffer_b);
    command_buffer2.End();

    VkSemaphoreSubmitInfo semaphore_info = vku::InitStructHelper();
    semaphore_info.semaphore = semaphore;
    semaphore_info.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    VkCommandBufferSubmitInfo cmd_info = vku::InitStructHelper();
    cmd_info.commandBuffer = m_command_buffer.handle();

    VkCommandBufferSubmitInfo cmd_info2 = vku::InitStructHelper();
    cmd_info2.commandBuffer = command_buffer2.handle();

    VkSubmitInfo2 submits[3];
    submits[0] = vku::InitStructHelper();
    submits[0].signalSemaphoreInfoCount = 1;
    submits[0].pSignalSemaphoreInfos = &semaphore_info;

    submits[1] = vku::InitStructHelper();
    submits[1].waitSemaphoreInfoCount = 1;
    submits[1].pWaitSemaphoreInfos = &semaphore_info;
    submits[1].commandBufferInfoCount = 1;
    submits[1].pCommandBufferInfos = &cmd_info;
    submits[1].signalSemaphoreInfoCount = 1;
    submits[1].pSignalSemaphoreInfos = &semaphore_info;

    submits[2] = vku::InitStructHelper();
    submits[2].waitSemaphoreInfoCount = 1;
    submits[2].pWaitSemaphoreInfos = &semaphore_info;
    submits[2].commandBufferInfoCount = 1;
    submits[2].pCommandBufferInfos = &cmd_info2;

    vk::QueueSubmit2(*m_default_queue, 3, submits, VK_NULL_HANDLE);
    m_default_queue->Wait();
}

TEST_F(PositiveSyncVal, WriteAndReadNonOverlappedUniformBufferRegions) {
    TEST_DESCRIPTION("Specify non-verlapped regions using offset in VkDescriptorBufferInfo");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    // 32 bytes
    const VkDeviceSize uniform_data_size = 8 * sizeof(uint32_t);
    // 128 bytes or more (depending on minUniformBufferOffsetAlignment)
    const VkDeviceSize copy_dst_area_size =
        std::max((VkDeviceSize)128, m_device->Physical().limits_.minUniformBufferOffsetAlignment);
    // 160 bytes or more (depending on minUniformBufferOffsetAlignment)
    const VkDeviceSize size = copy_dst_area_size + uniform_data_size;

    // We have at least 128 bytes of copy destination region, followed by 32 bytes of uniform data.
    // Copying data to the first region (WRITE) should not conflict with uniform READ from the second region.
    vkt::Buffer buffer_a(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    vkt::Buffer buffer_b(*m_device, copy_dst_area_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                       });
    // copy_dst_area_size offset ensures uniform region does not overlap with copy destination.
    descriptor_set.WriteDescriptorBufferInfo(0, buffer_a.handle(), copy_dst_area_size, uniform_data_size,
                                             VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const char *cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform buffer_a { uint x[8]; } constants;
        void main(){
            uint x = constants.x[0];
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1, &descriptor_set.set_,
                              0, nullptr);

    // Writes into region [0..127]
    VkBufferCopy region{};
    region.size = copy_dst_area_size;
    vk::CmdCopyBuffer(m_command_buffer, buffer_b, buffer_a, 1, &region);

    // Reads from region [128..159]
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
}

TEST_F(PositiveSyncVal, WriteAndReadNonOverlappedDynamicUniformBufferRegions) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8084
    TEST_DESCRIPTION("Specify non-verlapped regions using dynamic offset in vkCmdBindDescriptorSets");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    // 32 bytes
    const VkDeviceSize uniform_data_size = 8 * sizeof(uint32_t);
    // 128 bytes or more (depending on minUniformBufferOffsetAlignment)
    const VkDeviceSize copy_dst_area_size =
        std::max((VkDeviceSize)128, m_device->Physical().limits_.minUniformBufferOffsetAlignment);
    // 160 bytes or more (depending on minUniformBufferOffsetAlignment)
    const VkDeviceSize size = copy_dst_area_size + uniform_data_size;

    // We have at least 128 bytes of copy destination region, followed by 32 bytes of uniform data.
    // Copying data to the first region (WRITE) should not conflict with uniform READ from the second region.
    vkt::Buffer buffer_a(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    vkt::Buffer buffer_b(*m_device, copy_dst_area_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                       });

    // Specify 0 base offset, but dynamic offset will ensure that uniform data does not overlap with copy destination.
    descriptor_set.WriteDescriptorBufferInfo(0, buffer_a.handle(), 0, uniform_data_size, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
    descriptor_set.UpdateDescriptorSets();

    const char *cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform buffer_a { uint x[8]; } constants;
        void main(){
            uint x = constants.x[0];
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateComputePipeline();

    // this ensures copy region does not overlap with uniform data region
    uint32_t dynamic_offset = static_cast<uint32_t>(copy_dst_area_size);

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1, &descriptor_set.set_,
                              1, &dynamic_offset);

    // Writes into region [0..127]
    VkBufferCopy region{};
    region.size = copy_dst_area_size;
    vk::CmdCopyBuffer(m_command_buffer, buffer_b, buffer_a, 1, &region);

    // Reads from region [128..159]
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
}

TEST_F(PositiveSyncVal, WriteAndReadNonOverlappedDynamicUniformBufferRegions2) {
    // NOTE: the only difference between this test and the previous one (without suffix 2)
    // is the order of commands. This test does Dispatch and then Copy. This checks for
    // regression when dynamic offset is not applied during Record phase.
    TEST_DESCRIPTION("Specify non-verlapped regions using dynamic offset in vkCmdBindDescriptorSets");
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    // 32 bytes
    const VkDeviceSize uniform_data_size = 8 * sizeof(uint32_t);
    // 128 bytes or more (depending on minUniformBufferOffsetAlignment)
    const VkDeviceSize copy_dst_area_size =
        std::max((VkDeviceSize)128, m_device->Physical().limits_.minUniformBufferOffsetAlignment);
    // 160 bytes or more (depending on minUniformBufferOffsetAlignment)
    const VkDeviceSize size = copy_dst_area_size + uniform_data_size;

    // We have at least 128 bytes of copy destination region, followed by 32 bytes of uniform data.
    // Copying data to the first region (WRITE) should not conflict with uniform READ from the second region.
    vkt::Buffer buffer_a(*m_device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    vkt::Buffer buffer_b(*m_device, copy_dst_area_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                       });

    // Specify 0 base offset, but dynamic offset will ensure that uniform data does not overlap with copy destination.
    descriptor_set.WriteDescriptorBufferInfo(0, buffer_a.handle(), 0, uniform_data_size, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
    descriptor_set.UpdateDescriptorSets();

    const char *cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform buffer_a { uint x[8]; } constants;
        void main(){
            uint x = constants.x[0];
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateComputePipeline();

    // this ensures copy region does not overlap with uniform data region
    uint32_t dynamic_offset = static_cast<uint32_t>(copy_dst_area_size);

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1, &descriptor_set.set_,
                              1, &dynamic_offset);

    // Reads from region [128..159]
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    // Writes into region [0..127]
    VkBufferCopy region{};
    region.size = copy_dst_area_size;
    vk::CmdCopyBuffer(m_command_buffer, buffer_b, buffer_a, 1, &region);

    m_command_buffer.End();
}

TEST_F(PositiveSyncVal, ImageUsedInShaderWithoutAccess) {
    TEST_DESCRIPTION("Test that imageSize() query is not classified as image access");
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Buffer copy_source(*m_device, 32 * 32 * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView();

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT}});
    descriptor_set.WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.UpdateDescriptorSets();

    const char *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0, rgba8) uniform image2D image;
        void main(){
            uvec2 size = imageSize(image);
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateComputePipeline();

    VkBufferImageCopy region{};
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageExtent = {32, 32, 1};

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    // this should not cause WRITE-AFTER-READ because previous dispatch reads only image descriptor
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), copy_source, image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_command_buffer.End();
}

// WARNING: this test passes due to LUCK. Currently syncval does not know about atomic
// accesses and going to treat two atomic writes from different dispatches as WRITE-AFTER-WRITE
// hazard. The reason it does not report WRITE-AFTER-WRITE here is because SPIR-V analysis reports
// READ access for atomicAdd(data[0], 1).
//
// TODO:
// The first step is to try to update SPIR-V static analysis so it reports WRITE and sets a
// flag that variable was used in atomic operation. This change will expose the missing
// syncval ability to detect atomic operation in the form that this test will fail with
// WRITE-AFTER-WRITE report.
//
// The next step is to update syncval heuristic to take into account atomic flag so this test
// passes again.
TEST_F(PositiveSyncVal, AtomicAccessFromTwoDispatches) {
    TEST_DESCRIPTION("Not synchronized dispatches/draws can write to the same memory location by using atomics");
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Buffer buffer(*m_device, 128, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT}});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const char *cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer ssbo { uint data[]; };
        void main(){
            atomicAdd(data[0], 1);
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();
}

// WARNING: this test also passes due to LUCK. Same reason as the previous test.
TEST_F(PositiveSyncVal, AtomicAccessFromTwoSubmits) {
    TEST_DESCRIPTION("Not synchronized dispatches/draws can write to the same memory location by using atomics");
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Buffer buffer(*m_device, 128, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT}});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const char *cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer ssbo { uint data[]; };
        void main(){
            atomicAdd(data[0], 1);
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateComputePipeline();

    m_command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

// TODO: this test does not work due to SupressedBoundDescriptorWAW(). That workaround should be removed.
// Two possible solutions:
// a) Try to detect if there is atomic operation in the buffer access chain. If yes, skip validation.
// b) If a) is hard to do, then this case is in the category that is not handled by the current heuristic
//    and is part of "Shader access heuristic" is disabled by default direction.
TEST_F(PositiveSyncVal, AtomicAccessFromTwoDispatches2) {
    TEST_DESCRIPTION("Use atomic counter so parallel dispatches write to different locations");
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Buffer counter_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    vkt::Buffer data_buffer(*m_device, 128, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT}});
    descriptor_set.WriteDescriptorBufferInfo(0, counter_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferInfo(1, data_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const char *cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer i_am_counter { uint counter[]; };
        layout(set=0, binding=1) buffer i_am_data { uint data[]; };
        void main(){
            uint index = atomicAdd(counter[0], 1);
            data[index] = 42;
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();
}

// Demostrates false-positive from the client's report.
TEST_F(PositiveSyncVal, AtomicAccessFromTwoSubmits2) {
    TEST_DESCRIPTION("Use atomic counter so parallel dispatches write to different locations");
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Buffer counter_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    vkt::Buffer data_buffer(*m_device, 128, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT}});
    descriptor_set.WriteDescriptorBufferInfo(0, counter_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferInfo(1, data_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const char *cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer i_am_counter { uint counter[]; };
        layout(set=0, binding=1) buffer i_am_data { uint data[]; };
        void main(){
            uint index = atomicAdd(counter[0], 1);
            data[index] = 42;
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateComputePipeline();

    m_command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);

    // TODO: this should be a positive test, but currenlty we have a false-positive.
    // Remove error monitor check if we have better solution, or when we disable
    // Shader access heuristic setting for this test (so will simulate configuration
    // when the user disabled the feature).
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(PositiveSyncVal, QueueFamilyOwnershipTransfer) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7024
    TEST_DESCRIPTION("Ownership transfer from transfer to graphics queue");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Queue *transfer_queue = m_device->TransferOnlyQueue();
    if (!transfer_queue) {
        GTEST_SKIP() << "Transfer-only queue is needed";
    }
    vkt::CommandPool transfer_cmd_pool(*m_device, transfer_queue->family_index);
    vkt::CommandBuffer transfer_command_buffer(*m_device, transfer_cmd_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    vkt::Buffer buffer(*m_device, 64 * 64 * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    vkt::Image image(*m_device, 64, 64, 1, VK_FORMAT_R8G8B8A8_UNORM, usage);
    image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    vkt::Semaphore semaphore(*m_device);

    // Tranfer queue: copy to image and issue release operation
    {
        VkBufferImageCopy2 region = vku::InitStructHelper();
        region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        region.imageExtent = {64, 64, 1};

        VkCopyBufferToImageInfo2 copy_info = vku::InitStructHelper();
        copy_info.srcBuffer = buffer;
        copy_info.dstImage = image;
        copy_info.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        copy_info.regionCount = 1;
        copy_info.pRegions = &region;

        VkImageMemoryBarrier2 release_barrier = vku::InitStructHelper();
        release_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
        release_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;

        // Not needed, use semaphore to estabish execution dependency
        release_barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;

        // Visibility operation is not performed for release operation
        release_barrier.dstAccessMask = VK_ACCESS_2_NONE;

        release_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        release_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        release_barrier.srcQueueFamilyIndex = transfer_queue->family_index;
        release_barrier.dstQueueFamilyIndex = m_default_queue->family_index;
        release_barrier.image = image;
        release_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

        VkDependencyInfo release_dependency = vku::InitStructHelper();
        release_dependency.imageMemoryBarrierCount = 1;
        release_dependency.pImageMemoryBarriers = &release_barrier;

        transfer_command_buffer.Begin();
        vk::CmdCopyBufferToImage2(transfer_command_buffer, &copy_info);
        vk::CmdPipelineBarrier2(transfer_command_buffer, &release_dependency);
        transfer_command_buffer.End();

        transfer_queue->Submit2(transfer_command_buffer, vkt::Signal(semaphore));
    }

    // Graphics queue: wait for transfer queue and issue acquire operation
    {
        VkImageMemoryBarrier2 acquire_barrier = vku::InitStructHelper();
        // Not needed, use semaphore to estabish execution dependency
        acquire_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;

        // Availability operation is not performed for release operation
        acquire_barrier.srcAccessMask = VK_ACCESS_2_NONE;

        acquire_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        acquire_barrier.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
        acquire_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        acquire_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        acquire_barrier.srcQueueFamilyIndex = transfer_queue->family_index;
        acquire_barrier.dstQueueFamilyIndex = m_default_queue->family_index;
        acquire_barrier.image = image;
        acquire_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

        VkDependencyInfo acquire_dependency = vku::InitStructHelper();
        acquire_dependency.imageMemoryBarrierCount = 1;
        acquire_dependency.pImageMemoryBarriers = &acquire_barrier;

        m_command_buffer.Begin();
        vk::CmdPipelineBarrier2(m_command_buffer, &acquire_dependency);
        m_command_buffer.End();

        m_default_queue->Submit2(m_command_buffer, vkt::Wait(semaphore));
    }
    m_device->Wait();
}

TEST_F(PositiveSyncVal, IndirectDrawAndSuballocatedVertexBuffer) {
    TEST_DESCRIPTION("Indirect draw reads suballocated vertex buffer. CmdFillBuffer writes to another non-overlapped region");
    RETURN_IF_SKIP(InitSyncVal());
    InitRenderTarget();

    VkDrawIndirectCommand indirect_command = {};
    indirect_command.vertexCount = 3;
    indirect_command.instanceCount = 1;
    vkt::Buffer indirect_buffer(*m_device, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, &indirect_command, sizeof(VkDrawIndirectCommand));

    const size_t vertices_size = 3 * 3 * sizeof(float);  // 3 VK_FORMAT_R32G32B32_SFLOAT vertices
    const size_t fill_region_size = 32;
    const VkDeviceSize buffer_size = vertices_size + fill_region_size;
    vkt::Buffer buffer(*m_device, buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    const VkVertexInputBindingDescription input_binding = {0, 3 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX};
    const VkVertexInputAttributeDescription input_attrib = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};

    CreatePipelineHelper pipe(*this);
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();

    // Indirect draw reads from vertex region
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    const VkDeviceSize vertices_offset = 0;
    vk::CmdBindVertexBuffers(m_command_buffer, 0, 1, &buffer.handle(), &vertices_offset);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDrawIndirect(m_command_buffer, indirect_buffer, 0, 1, 0);
    m_command_buffer.EndRenderPass();

    // Fill region does not intersect vertex region. There should be no hazards.
    vk::CmdFillBuffer(m_command_buffer, buffer, vertices_size, fill_region_size, 0);

    m_command_buffer.End();
}

TEST_F(PositiveSyncVal, IndirectDrawAndSuballocatedIndexBuffer) {
    TEST_DESCRIPTION("Indirect draw reads suballocated index buffer. CmdFillBuffer writes to another non-overlapped region");
    RETURN_IF_SKIP(InitSyncVal());
    InitRenderTarget();

    VkDrawIndexedIndirectCommand indirect_command = {};
    indirect_command.indexCount = 3;
    indirect_command.instanceCount = 1;

    vkt::Buffer indirect_buffer(*m_device, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, &indirect_command,
                                sizeof(VkDrawIndexedIndirectCommand));

    const size_t vertices_size = 3 * 3 * sizeof(float);  // 3 VK_FORMAT_R32G32B32_SFLOAT vertices
    vkt::Buffer vertex_buffer(*m_device, vertices_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    constexpr size_t indices_size = 3 * sizeof(uint32_t);
    constexpr size_t fill_region_size = 12;
    constexpr VkDeviceSize buffer_size = indices_size + fill_region_size;
    uint32_t buffer_data[buffer_size / sizeof(uint32_t)] = {0, 1, 2};  // initialize index region
    vkt::Buffer buffer(*m_device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, buffer_data, buffer_size);

    const VkVertexInputBindingDescription input_binding = {0, 3 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX};
    const VkVertexInputAttributeDescription input_attrib = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};

    CreatePipelineHelper pipe(*this);
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();

    // Indirect draw reads from index region
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    const VkDeviceSize vertices_offset = 0;
    vk::CmdBindIndexBuffer(m_command_buffer, buffer, 0, VK_INDEX_TYPE_UINT32);
    vk::CmdBindVertexBuffers(m_command_buffer, 0, 1, &vertex_buffer.handle(), &vertices_offset);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDrawIndexedIndirect(m_command_buffer, indirect_buffer, 0, 1, 0);
    m_command_buffer.EndRenderPass();

    // Fill region does not intersect index region. There should be no hazards.
    vk::CmdFillBuffer(m_command_buffer, buffer, indices_size, fill_region_size, 1);

    m_command_buffer.End();
}

TEST_F(PositiveSyncVal, DynamicRenderingDSWithOnlyStencilAspect) {
    TEST_DESCRIPTION("");

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Image color_image(*m_device, 32u, 32u, 1u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView color_image_view = color_image.CreateView();

    vkt::Image depth_image(*m_device, 32u, 32u, 1u, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    vkt::ImageView depth_image_view = depth_image.CreateView(VK_IMAGE_ASPECT_STENCIL_BIT);

    {
        RenderPassSingleSubpass rp(*this);
        rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        rp.AddColorAttachment(0);
        rp.AddAttachmentDescription(VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        rp.AddAttachmentReference({1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL});
        rp.AddDepthStencilAttachment(1);
        rp.CreateRenderPass();

        VkImageView image_views[] = {color_image_view, depth_image_view};
        vkt::Framebuffer framebuffer(*m_device, rp.Handle(), 2u, image_views);

        VkClearValue color_clear_value;
        color_clear_value.color.float32[0] = 0.0f;
        color_clear_value.color.float32[1] = 0.0f;
        color_clear_value.color.float32[2] = 0.0f;
        color_clear_value.color.float32[3] = 1.0f;

        VkRenderPassBeginInfo render_pass_begin_info = vku::InitStructHelper();
        render_pass_begin_info.renderPass = rp.Handle();
        render_pass_begin_info.framebuffer = framebuffer.handle();
        render_pass_begin_info.renderArea = {{0, 0}, {32u, 32u}};
        render_pass_begin_info.clearValueCount = 1u;
        render_pass_begin_info.pClearValues = &color_clear_value;

        m_command_buffer.Begin();
        m_command_buffer.BeginRenderPass(render_pass_begin_info);
        m_command_buffer.EndRenderPass();
        m_command_buffer.End();
    }

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = color_image_view.handle();

    VkRenderingAttachmentInfo depth_stencil_attachment = vku::InitStructHelper();
    depth_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_stencil_attachment.imageView = depth_image_view.handle();

    VkRenderingInfo rendering_info = vku::InitStructHelper();
    rendering_info.renderArea = {{0, 0}, {32u, 32u}};
    rendering_info.layerCount = 1u;
    rendering_info.colorAttachmentCount = 1u;
    rendering_info.pColorAttachments = &color_attachment;
    rendering_info.pDepthAttachment = &depth_stencil_attachment;
    rendering_info.pStencilAttachment = &depth_stencil_attachment;

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(rendering_info);
    m_command_buffer.EndRendering();
    m_command_buffer.End();
}

TEST_F(PositiveSyncVal, AmdBufferMarker) {
    TEST_DESCRIPTION("Use barrier to synchronize with AMD buffer marker accesses");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredExtensions(VK_AMD_BUFFER_MARKER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    VkBufferMemoryBarrier2 barrier = vku::InitStructHelper();
    // AMD marker access is WRITE on TRANSFER stage
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    // Buffer copy access
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.buffer = buffer_a;
    barrier.size = 256;

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.bufferMemoryBarrierCount = 1;
    dep_info.pBufferMemoryBarriers = &barrier;

    m_command_buffer.Begin();
    vk::CmdWriteBufferMarkerAMD(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, buffer_a, 0, 1);
    vk::CmdPipelineBarrier2(m_command_buffer, &dep_info);
    m_command_buffer.Copy(buffer_b, buffer_a);
    m_command_buffer.End();
}
