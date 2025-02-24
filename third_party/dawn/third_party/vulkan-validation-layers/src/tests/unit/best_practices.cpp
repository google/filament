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

#include <thread>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/render_pass_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/thread_helper.h"

void VkBestPracticesLayerTest::InitBestPracticesFramework(const char *vendor_checks_to_enable) {
    // Enable the vendor-specific checks spcified by vendor_checks_to_enable
    const char *input_values[] = {vendor_checks_to_enable};
    const VkLayerSettingEXT settings[] = {{OBJECT_LAYER_NAME, "enables", VK_LAYER_SETTING_TYPE_STRING_EXT,
                                           static_cast<uint32_t>(std::size(input_values)), input_values}};

    const VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                                  static_cast<uint32_t>(std::size(settings)), settings};

    features_.pNext = &layer_settings_create_info;

    AddRequiredExtensions(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
    InitFramework(&features_);
}

void VkBestPracticesLayerTest::InitBestPractices(const char *ValidationChecksToEnable) {
    RETURN_IF_SKIP(InitBestPracticesFramework(ValidationChecksToEnable));
    RETURN_IF_SKIP(InitState());
}

TEST_F(VkBestPracticesLayerTest, ReturnCodes) {
    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddSurfaceExtension();
    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());
    RETURN_IF_SKIP(InitSwapchain());

    // Attempt to force an invalid return code for an unsupported format
    VkImageFormatProperties2 image_format_prop = {};
    image_format_prop.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
    VkPhysicalDeviceImageFormatInfo2 image_format_info = {};
    image_format_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
    image_format_info.format = VK_FORMAT_R32G32B32_SFLOAT;
    image_format_info.tiling = VK_IMAGE_TILING_LINEAR;
    image_format_info.type = VK_IMAGE_TYPE_3D;
    image_format_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;

    VkResult result =
        vk::GetPhysicalDeviceImageFormatProperties2(m_device->Physical().handle(), &image_format_info, &image_format_prop);
    // Only run this test if this super-wierd format is not supported
    if (VK_SUCCESS != result) {
        m_errorMonitor->SetDesiredWarning("BestPractices-Error-Result");
        vk::GetPhysicalDeviceImageFormatProperties2(m_device->Physical().handle(), &image_format_info, &image_format_prop);
        m_errorMonitor->VerifyFound();
    }

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD because will always return VK_SUCCESS";
    }

    // Force a non-success success code by only asking for a subset of query results
    uint32_t format_count;
    std::vector<VkSurfaceFormatKHR> formats;
    result = vk::GetPhysicalDeviceSurfaceFormatsKHR(Gpu(), m_surface.Handle(), &format_count, NULL);
    if (result != VK_SUCCESS || format_count <= 1) {
        GTEST_SKIP() << "test requires 2 or more extensions available";
    }
    format_count -= 1;
    formats.resize(format_count);

    m_errorMonitor->SetDesiredFailureMsg(kVerboseBit, "BestPractices-Verbose-Success-Logging");
    result = vk::GetPhysicalDeviceSurfaceFormatsKHR(Gpu(), m_surface.Handle(), &format_count, formats.data());
    ASSERT_TRUE(result > VK_SUCCESS);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, UseDeprecatedInstanceExtensions) {
    TEST_DESCRIPTION("Create an instance with a deprecated extension.");

    // We need to explicitly allow promoted extensions to be enabled as this test relies on this behavior
    AllowPromotedExtensions();

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBestPracticesFramework());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD - currently can't create 2 concurrent instances";
    }

    // Create a 1.1 vulkan instance and request an extension promoted to core in 1.1
    if (IsExtensionsEnabled(VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
        // Extra error if VK_EXT_debug_report is used on Android still
        m_errorMonitor->SetDesiredWarning("BestPractices-deprecated-extension");
    }

    m_errorMonitor->SetDesiredWarning("BestPractices-deprecated-extension");  // VK_KHR_get_physical_device_properties2
    m_errorMonitor->SetDesiredWarning("BestPractices-deprecated-extension");  // VK_EXT_validation_features
    m_errorMonitor->SetDesiredWarning("BestPractices-specialuse-extension");  // VK_EXT_debug_utils
    m_errorMonitor->SetDesiredWarning("BestPractices-specialuse-extension");  // VK_EXT_validation_features

    VkInstance dummy = VK_NULL_HANDLE;
    auto features = features_;
    auto ici = GetInstanceCreateInfo();
    features.pNext = ici.pNext;
    ici.pNext = &features;
    vk::CreateInstance(&ici, nullptr, &dummy);
    m_errorMonitor->VerifyFound();

    VkApplicationInfo new_info{};
    new_info.apiVersion = VK_API_VERSION_1_0;
    new_info.pApplicationName = ici.pApplicationInfo->pApplicationName;
    new_info.applicationVersion = ici.pApplicationInfo->applicationVersion;
    new_info.pEngineName = ici.pApplicationInfo->pEngineName;
    new_info.engineVersion = ici.pApplicationInfo->engineVersion;
    ici.pApplicationInfo = &new_info;

    // Create a 1.0 vulkan instance and request an extension promoted to core in 1.1
    if (IsExtensionsEnabled(VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
        // Extra error if VK_EXT_debug_report is used on Android still
        m_errorMonitor->SetDesiredWarning("BestPractices-deprecated-extension");
    }
    m_errorMonitor->SetUnexpectedError("khronos-Validation-debug-build-warning-message");
    m_errorMonitor->SetUnexpectedError("khronos-Validation-fine-grained-locking-warning-message");
    m_errorMonitor->SetDesiredWarning("BestPractices-deprecated-extension");  // VK_EXT_validation_features
    m_errorMonitor->SetDesiredWarning("BestPractices-specialuse-extension");  // VK_EXT_debug_utils
    m_errorMonitor->SetDesiredWarning("BestPractices-specialuse-extension");  // VK_EXT_validation_features
    vk::CreateInstance(&ici, nullptr, &dummy);
    m_errorMonitor->VerifyFound();
    if (dummy != VK_NULL_HANDLE) {
        vk::DestroyInstance(dummy, nullptr);
    }
}

TEST_F(VkBestPracticesLayerTest, UseDeprecatedDeviceExtensions) {
    TEST_DESCRIPTION("Create a device with a deprecated extension.");

    // We need to explicitly allow promoted extensions to be enabled as this test relies on this behavior
    AllowPromotedExtensions();

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBestPracticesFramework());

    VkDevice local_device;
    VkDeviceCreateInfo dev_info = {};
    VkDeviceQueueCreateInfo queue_info = {};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.pNext = NULL;
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    float qp = 1;
    queue_info.pQueuePriorities = &qp;
    dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev_info.pNext = nullptr;
    dev_info.queueCreateInfoCount = 1;
    dev_info.pQueueCreateInfos = &queue_info;
    dev_info.enabledLayerCount = 0;
    dev_info.ppEnabledLayerNames = NULL;
    dev_info.enabledExtensionCount = m_device_extension_names.size();
    dev_info.ppEnabledExtensionNames = m_device_extension_names.data();

    // One for VK_KHR_buffer_device_address
    // One for the dependency extension VK_KHR_device_group
    m_errorMonitor->SetDesiredWarning("BestPractices-deprecated-extension", 2);
    vk::CreateDevice(this->Gpu(), &dev_info, NULL, &local_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, SpecialUseExtensions) {
    TEST_DESCRIPTION("Create a device with a 'specialuse' extension.");

    AddRequiredExtensions(VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBestPracticesFramework());

    VkDevice local_device;
    VkDeviceCreateInfo dev_info = {};
    VkDeviceQueueCreateInfo queue_info = {};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.pNext = NULL;
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    float qp = 1;
    queue_info.pQueuePriorities = &qp;
    dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev_info.pNext = nullptr;
    dev_info.queueCreateInfoCount = 1;
    dev_info.pQueueCreateInfos = &queue_info;
    dev_info.enabledLayerCount = 0;
    dev_info.ppEnabledLayerNames = NULL;
    dev_info.enabledExtensionCount = m_device_extension_names.size();
    dev_info.ppEnabledExtensionNames = m_device_extension_names.data();

    m_errorMonitor->SetDesiredWarning("BestPractices-specialuse-extension");
    vk::CreateDevice(this->Gpu(), &dev_info, NULL, &local_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, CmdClearAttachmentTest) {
    TEST_DESCRIPTION("Test for validating usage of vkCmdClearAttachments");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    // Main thing we care about for this test is that the VkImage obj we're
    // clearing matches Color Attachment of FB
    //  Also pass down other dummy params to keep driver and paramchecker happy
    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 1.0;
    color_attachment.clearValue.color.float32[1] = 1.0;
    color_attachment.clearValue.color.float32[2] = 1.0;
    color_attachment.clearValue.color.float32[3] = 1.0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};

    // Call for full-sized FB Color attachment prior to issuing a Draw
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-DrawState-ClearCmdBeforeDraw");

    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &color_attachment, 1, &clear_rect);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, CmdClearAttachmentTestSecondary) {
    TEST_DESCRIPTION("Test for validating usage of vkCmdClearAttachments with secondary command buffers");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    m_command_buffer.Begin();

    vkt::CommandBuffer secondary_full_clear(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    vkt::CommandBuffer secondary_small_clear(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    VkCommandBufferInheritanceInfo inherit_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO};
    begin_info.pInheritanceInfo = &inherit_info;
    inherit_info.subpass = 0;
    inherit_info.renderPass = m_renderPassBeginInfo.renderPass;

    // Main thing we care about for this test is that the VkImage obj we're
    // clearing matches Color Attachment of FB
    //  Also pass down other dummy params to keep driver and paramchecker happy
    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 1.0;
    color_attachment.clearValue.color.float32[1] = 1.0;
    color_attachment.clearValue.color.float32[2] = 1.0;
    color_attachment.clearValue.color.float32[3] = 1.0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect_small = {{{0, 0}, {m_width - 1u, m_height - 1u}}, 0, 1};
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    {
        // Small clears which don't cover the render area should not trigger the warning.
        vk::CmdClearAttachments(m_command_buffer.handle(), 1, &color_attachment, 1, &clear_rect_small);
        // Call for full-sized FB Color attachment prior to issuing a Draw
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-DrawState-ClearCmdBeforeDraw");
        // This test may also trigger other warnings
        m_errorMonitor->SetAllowedFailureMsg("BestPractices-AMD-VkCommandBuffer-AvoidSecondaryCmdBuffers");

        vk::CmdClearAttachments(m_command_buffer.handle(), 1, &color_attachment, 1, &clear_rect);
        m_errorMonitor->VerifyFound();
    }
    m_command_buffer.EndRenderPass();

    secondary_small_clear.Begin(&begin_info);
    secondary_full_clear.Begin(&begin_info);
    vk::CmdClearAttachments(secondary_small_clear.handle(), 1, &color_attachment, 1, &clear_rect_small);
    vk::CmdClearAttachments(secondary_full_clear.handle(), 1, &color_attachment, 1, &clear_rect);
    secondary_small_clear.End();
    secondary_full_clear.End();

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    {
        // Small clears which don't cover the render area should not trigger the warning.
        vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary_small_clear.handle());
        // Call for full-sized FB Color attachment prior to issuing a Draw
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-DrawState-ClearCmdBeforeDraw");
        // This test may also trigger other warnings
        m_errorMonitor->SetAllowedFailureMsg("BestPractices-AMD-VkCommandBuffer-AvoidSecondaryCmdBuffers");

        vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary_full_clear.handle());
        m_errorMonitor->VerifyFound();
    }
    m_command_buffer.EndRenderPass();
}

TEST_F(VkBestPracticesLayerTest, CmdResolveImageTypeMismatch) {
    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent = {32, 1, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;  // guarantee support from sampledImageColorSampleCounts
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    // Note: Some implementations expect color attachment usage for any
    // multisample surface
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.flags = 0;
    vkt::Image srcImage(*m_device, image_create_info, vkt::set_layout);

    image_create_info.imageType = VK_IMAGE_TYPE_1D;
    // Note: Some implementations expect color attachment usage for any
    // multisample surface
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    vkt::Image dstImage(*m_device, image_create_info, vkt::set_layout);

    m_command_buffer.Begin();
    // Need memory barrier to VK_IMAGE_LAYOUT_GENERAL for source and dest?
    // VK_IMAGE_LAYOUT_UNDEFINED = 0,
    // VK_IMAGE_LAYOUT_GENERAL = 1,
    VkImageResolve resolveRegion;
    resolveRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    resolveRegion.srcOffset = {0, 0, 0};
    resolveRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    resolveRegion.dstOffset = {0, 0, 0};
    resolveRegion.extent = {1, 1, 1};

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-vkCmdResolveImage-MismatchedImageType");
    vk::CmdResolveImage(m_command_buffer.handle(), srcImage.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(VkBestPracticesLayerTest, ZeroSizeBlitRegion) {
    TEST_DESCRIPTION("vkCmdBlitImage with a zero area region");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Image image_src(*m_device, 128, 128, 1, VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    image_src.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    vkt::Image image_dst(*m_device, 128, 128, 1, VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    image_dst.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    VkImageBlit blit_region = {};
    blit_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    blit_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    blit_region.srcOffsets[0] = {128, 0, 0};
    blit_region.srcOffsets[1] = {128, 128, 1};
    blit_region.dstOffsets[0] = {0, 128, 0};
    blit_region.dstOffsets[1] = {128, 128, 1};

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredWarning("BestPractices-DrawState-InvalidExtents-src");
    m_errorMonitor->SetDesiredWarning("BestPractices-DrawState-InvalidExtents-dst");
    vk::CmdBlitImage(m_command_buffer.handle(), image_src.handle(), image_src.Layout(), image_dst.handle(), image_dst.Layout(), 1,
                     &blit_region, VK_FILTER_LINEAR);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(VkBestPracticesLayerTest, VtxBufferBadIndex) {
    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    // Don't care about actual data, just need to get to draw to flag error
    vkt::Buffer vbo(*m_device, sizeof(float) * 3, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-vkEndCommandBuffer-VtxIndexOutOfBounds");
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    // VBO idx 1, but no VBO in PSO
    vk::CmdBindVertexBuffers(m_command_buffer.handle(), 1, 1, &vbo.handle(), &kZeroDeviceSize);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    vk::EndCommandBuffer(m_command_buffer.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, CommandBufferReset) {
    TEST_DESCRIPTION("Test for validating usage of vkCreateCommandPool with COMMAND_BUFFER_RESET_BIT");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-vkCreateCommandPool-command-buffer-reset");

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(device(), &pool_create_info, nullptr, &command_pool);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, SecondaryCommandBuffer) {
    TEST_DESCRIPTION("Test for validating usage of vkCreateCommandPool with VK_COMMAND_BUFFER_LEVEL_SECONDARY");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    uint32_t queue_family_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_family_props;
    queue_family_props.resize(queue_family_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_family_count, queue_family_props.data());

    uint32_t queue_family_index = VK_QUEUE_FAMILY_IGNORED;
    const VkQueueFlags sec_cmd_buf_queue_flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
    for (uint32_t i = 0; i < queue_family_count; ++i) {
        if ((queue_family_props[i].queueFlags & sec_cmd_buf_queue_flags) == 0) {
            queue_family_index = i;
            break;
        }
    }

    if (queue_family_index == VK_QUEUE_FAMILY_IGNORED) {
        GTEST_SKIP() << "No queue family found without support for secondary command buffers";
    }

    VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
    pool_create_info.queueFamilyIndex = queue_family_index;
    vkt::CommandPool command_pool(*m_device, pool_create_info);

    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.commandPool = command_pool.handle();
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    alloc_info.commandBufferCount = 1;

    m_errorMonitor->SetDesiredWarning("BestPractices-vkAllocateCommandBuffers-unusable-secondary");
    vk::AllocateCommandBuffers(device(), &alloc_info, &command_buffer);
    m_errorMonitor->VerifyFound();

    vk::FreeCommandBuffers(device(), command_pool.handle(), 1, &command_buffer);
}

TEST_F(VkBestPracticesLayerTest, SimultaneousUse) {
    TEST_DESCRIPTION("Test for validating usage of vkBeginCommandBuffer with SIMULTANEOUS_USE");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-vkBeginCommandBuffer-simultaneous-use");

    m_errorMonitor->SetAllowedFailureMsg("vkBeginCommandBuffer-one-time-submit");

    VkCommandBufferBeginInfo cmd_begin_info{};
    cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    vk::BeginCommandBuffer(m_command_buffer.handle(), &cmd_begin_info);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, SmallAllocation) {
    TEST_DESCRIPTION("Test for small memory allocations");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-vkAllocateMemory-small-allocation");

    // Find appropriate memory type for given reqs
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkPhysicalDeviceMemoryProperties dev_mem_props = m_device->Physical().memory_properties_;

    uint32_t mem_type_index = 0;
    for (mem_type_index = 0; mem_type_index < dev_mem_props.memoryTypeCount; ++mem_type_index) {
        if (mem_props == (mem_props & dev_mem_props.memoryTypes[mem_type_index].propertyFlags)) break;
    }
    EXPECT_LT(mem_type_index, dev_mem_props.memoryTypeCount) << "Could not find a suitable memory type.";

    const uint32_t kSmallAllocationSize = 1024;

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = kSmallAllocationSize;
    alloc_info.memoryTypeIndex = mem_type_index;

    VkDeviceMemory memory;
    vk::AllocateMemory(device(), &alloc_info, nullptr, &memory);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, SmallDedicatedAllocation) {
    TEST_DESCRIPTION("Test for small dedicated memory allocations");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-vkBindImageMemory-small-dedicated-allocation");

    m_errorMonitor->SetAllowedFailureMsg("BestPractices-vkAllocateMemory-small-allocation");

    VkImageCreateInfo image_info =
        vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    // Create a small image with a dedicated allocation
    vkt::Image image(*m_device, image_info, vkt::no_mem);

    vkt::DeviceMemory mem;
    mem.init(*m_device,
             vkt::DeviceMemory::GetResourceAllocInfo(*m_device, image.MemoryRequirements(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
    vk::BindImageMemory(device(), image.handle(), mem.handle(), 0);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, MSImageRequiresMemory) {
    TEST_DESCRIPTION("Test for MS image that requires memory");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-vkCreateRenderPass-image-requires-memory");

    VkAttachmentDescription attachment{};
    attachment.samples = VK_SAMPLE_COUNT_4_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.format = VK_FORMAT_B8G8R8A8_SRGB;
    attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription sd{};

    VkRenderPassCreateInfo rp_info{};
    rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &attachment;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &sd;

    VkRenderPass rp;
    vk::CreateRenderPass(device(), &rp_info, nullptr, &rp);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, AttachmentShouldNotBeTransient) {
    TEST_DESCRIPTION("Test for non-lazy multisampled images");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "BestPractices-vkCreateFramebuffer-attachment-should-not-be-transient");

    m_errorMonitor->SetAllowedFailureMsg("BestPractices-vkAllocateMemory-small-allocation");
    m_errorMonitor->SetAllowedFailureMsg("BestPractices-vkBindImageMemory-small-dedicated-allocation");
    m_errorMonitor->SetAllowedFailureMsg("BestPractices-vkBindImageMemory-non-lazy-transient-image");
    m_errorMonitor->SetAllowedFailureMsg("BestPractices-AMD-vkImage-AvoidGeneral");

    VkAttachmentDescription attachment{};
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription sd{};

    VkRenderPassCreateInfo rp_info = vku::InitStructHelper();
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &attachment;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &sd;
    vkt::RenderPass rp(*m_device, rp_info);

    VkImageCreateInfo image_info = vkt::Image::ImageCreateInfo2D(
        1920, 1080, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);
    vkt::Image image(*m_device, image_info, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    vkt::Framebuffer fb(*m_device, rp.handle(), 1, &image_view.handle(), 1920, 1080);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, TooManyInstancedVertexBuffers) {
    TEST_DESCRIPTION("Test for too many instanced vertex buffers");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "BestPractices-vkCreateGraphicsPipelines-too-many-instanced-vertex-buffers");

    // This test may also trigger the small allocation warnings
    m_errorMonitor->SetAllowedFailureMsg("BestPractices-vkAllocateMemory-small-allocation");
    m_errorMonitor->SetAllowedFailureMsg("BestPractices-vkBindImageMemory-small-dedicated-allocation");

    // This test does not need for the shader to consume the vertex input
    m_errorMonitor->SetAllowedFailureMsg("WARNING-Shader-OutputNotConsumed");

    InitRenderTarget();

    std::vector<VkVertexInputBindingDescription> bindings(2, VkVertexInputBindingDescription{});
    std::vector<VkVertexInputAttributeDescription> attributes(2, VkVertexInputAttributeDescription{});

    bindings[0].binding = 0;
    bindings[0].stride = 4;
    bindings[0].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    attributes[0].binding = 0;
    attributes[0].location = 0;
    attributes[0].format = VK_FORMAT_R32_SFLOAT;

    bindings[1].binding = 1;
    bindings[1].stride = 8;
    bindings[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    attributes[1].binding = 1;
    attributes[1].location = 1;
    attributes[1].format = VK_FORMAT_R32_SFLOAT;

    VkPipelineVertexInputStateCreateInfo vi_state_ci{};
    vi_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vi_state_ci.vertexBindingDescriptionCount = static_cast<uint32_t>(bindings.size());
    vi_state_ci.pVertexBindingDescriptions = bindings.data();
    vi_state_ci.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
    vi_state_ci.pVertexAttributeDescriptions = attributes.data();

    CreatePipelineHelper pipe(*this);
    pipe.vi_ci_ = vi_state_ci;
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, ClearAttachmentsAfterLoad) {
    TEST_DESCRIPTION("Test for clearing attachments after load");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Image image(*m_device, m_width, m_height, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    auto image_view = image.CreateView();

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();
    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &image_view.handle(), m_width, m_height);

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-vkCmdClearAttachments-clear-after-load-color");

    // On tiled renderers, this can also trigger a warning about LOAD_OP_LOAD causing a readback
    m_errorMonitor->SetAllowedFailureMsg("BestPractices-vkCmdBeginRenderPass-attachment-needs-readback");
    m_errorMonitor->SetAllowedFailureMsg("BestPractices-DrawState-ClearCmdBeforeDraw");
    m_errorMonitor->SetAllowedFailureMsg("BestPractices-RenderPass-redundant-store");
    m_errorMonitor->SetAllowedFailureMsg("BestPractices-RenderPass-redundant-clear");
    m_errorMonitor->SetAllowedFailureMsg("BestPractices-RenderPass-inefficient-clear");

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle(), m_width, m_height);

    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 1.0;
    color_attachment.clearValue.color.float32[1] = 1.0;
    color_attachment.clearValue.color.float32[2] = 1.0;
    color_attachment.clearValue.color.float32[3] = 1.0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};

    vk::CmdClearAttachments(m_command_buffer.handle(), 1, &color_attachment, 1, &clear_rect);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, ClearAttachmentsAfterLoadSecondary) {
    TEST_DESCRIPTION("Test for clearing attachments after load with secondary command buffers");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    // On tiled renderers, this can also trigger a warning about LOAD_OP_LOAD causing a readback
    m_errorMonitor->SetAllowedFailureMsg("BestPractices-vkCmdBeginRenderPass-attachment-needs-readback");
    m_errorMonitor->SetAllowedFailureMsg("BestPractices-RenderPass-redundant-store");
    m_errorMonitor->SetAllowedFailureMsg("BestPractices-RenderPass-redundant-clear");
    m_errorMonitor->SetAllowedFailureMsg("BestPractices-RenderPass-inefficient-clear");

    vkt::Image image(*m_device, m_width, m_height, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    auto image_view = image.CreateView();

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL,
                                VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();
    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &image_view.handle(), m_width, m_height);

    CreatePipelineHelper pipe_masked(*this);
    pipe_masked.gp_ci_.renderPass = rp.Handle();
    pipe_masked.cb_attachments_.colorWriteMask = 0;
    pipe_masked.CreateGraphicsPipeline();

    CreatePipelineHelper pipe_writes(*this);
    pipe_writes.gp_ci_.renderPass = rp.Handle();
    pipe_writes.cb_attachments_.colorWriteMask = 0xf;
    pipe_writes.CreateGraphicsPipeline();

    m_command_buffer.Begin();

    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 1.0;
    color_attachment.clearValue.color.float32[1] = 1.0;
    color_attachment.clearValue.color.float32[2] = 1.0;
    color_attachment.clearValue.color.float32[3] = 1.0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};

    VkRenderPassBeginInfo render_pass_begin_info = vku::InitStructHelper();
    render_pass_begin_info.renderPass = rp.Handle();
    render_pass_begin_info.framebuffer = fb.handle();
    // need full clear
    render_pass_begin_info.renderArea.extent.width = m_width;
    render_pass_begin_info.renderArea.extent.height = m_height;

    // Plain clear after load.
    m_command_buffer.BeginRenderPass(render_pass_begin_info);
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-vkCmdClearAttachments-clear-after-load-color");
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-DrawState-ClearCmdBeforeDraw");
    {
        vk::CmdClearAttachments(m_command_buffer.handle(), 1, &color_attachment, 1, &clear_rect);
        m_errorMonitor->VerifyFound();
    }
    m_command_buffer.EndRenderPass();

    // Test that a masked write is ignored before clear
    m_command_buffer.BeginRenderPass(render_pass_begin_info);
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-vkCmdClearAttachments-clear-after-load-color");
    {
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_masked.Handle());
        vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
        vk::CmdClearAttachments(m_command_buffer.handle(), 1, &color_attachment, 1, &clear_rect);
        m_errorMonitor->VerifyFound();
    }
    m_command_buffer.EndRenderPass();

    // Test that an actual write will not trigger the clear warning
    m_command_buffer.BeginRenderPass(render_pass_begin_info);
    {
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_writes.Handle());
        vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
        vk::CmdClearAttachments(m_command_buffer.handle(), 1, &color_attachment, 1, &clear_rect);
    }
    m_command_buffer.EndRenderPass();

    // Try the same thing, but now with secondary command buffers.
    VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    VkCommandBufferInheritanceInfo inherit_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO};
    begin_info.pInheritanceInfo = &inherit_info;
    inherit_info.subpass = 0;
    inherit_info.renderPass = rp.Handle();

    vkt::CommandBuffer secondary_clear(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    vkt::CommandBuffer secondary_draw_masked(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    vkt::CommandBuffer secondary_draw_write(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    secondary_clear.Begin(&begin_info);
    secondary_draw_masked.Begin(&begin_info);
    secondary_draw_write.Begin(&begin_info);

    vk::CmdClearAttachments(secondary_clear.handle(), 1, &color_attachment, 1, &clear_rect);

    vk::CmdBindPipeline(secondary_draw_masked.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_masked.Handle());
    vk::CmdDraw(secondary_draw_masked.handle(), 1, 0, 0, 0);

    vk::CmdBindPipeline(secondary_draw_write.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_writes.Handle());
    vk::CmdDraw(secondary_draw_write.handle(), 1, 0, 0, 0);

    secondary_clear.End();
    secondary_draw_masked.End();
    secondary_draw_write.End();

    // Plain clear after load.
    m_command_buffer.BeginRenderPass(render_pass_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-vkCmdClearAttachments-clear-after-load-color");
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-DrawState-ClearCmdBeforeDraw");
    {
        vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary_clear.handle());
        m_errorMonitor->VerifyFound();
    }
    m_command_buffer.EndRenderPass();

    // Test that a masked write is ignored before clear
    m_command_buffer.BeginRenderPass(render_pass_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "BestPractices-vkCmdClearAttachments-clear-after-load-color");
    {
        vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary_draw_masked.handle());
        vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary_clear.handle());
        m_errorMonitor->VerifyFound();
    }
    m_command_buffer.EndRenderPass();

    // Test that an actual write will not trigger the clear warning
    m_command_buffer.BeginRenderPass(render_pass_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    {
        vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary_draw_write.handle());
        vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary_clear.handle());
    }
    m_command_buffer.EndRenderPass();
}

TEST_F(VkBestPracticesLayerTest, TripleBufferingTest) {
    TEST_DESCRIPTION("Test for usage of triple buffering");

    AddSurfaceExtension();
    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "BestPractices-vkCreateSwapchainKHR-suboptimal-swapchain-image-count");
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), m_device->graphics_queue_node_index_, m_surface.Handle(), &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present";
    }

    bool fifo_present = false;
    for (const auto &present_mode : m_surface_present_modes) {
        if (present_mode == VK_PRESENT_MODE_FIFO_KHR) {
            fifo_present = true;
            break;
        }
    }
    if (!fifo_present) {
        GTEST_SKIP() << "fifo present mode not supported";
    }

    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    VkSwapchainCreateInfoKHR swapchain_create_info = {};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.pNext = 0;
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = 2;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = imageUsage;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = preTransform;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkSwapchainCreateInfoKHR-presentMode-02839");
    m_swapchain.Init(*m_device, swapchain_create_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkSwapchainCreateInfoKHR-presentMode-02839");
    swapchain_create_info.minImageCount = 3;
    m_swapchain.Init(*m_device, swapchain_create_info);
}

TEST_F(VkBestPracticesLayerTest, SwapchainCreationTest) {
    TEST_DESCRIPTION("Test for correct swapchain creation");

    AddSurfaceExtension();
    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());
    RETURN_IF_SKIP(InitSurface());

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    m_surface_composite_alpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
#else
    m_surface_composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
#endif

    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    VkSwapchainCreateInfoKHR swapchain_create_info = {};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.pNext = 0;
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = 3;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = imageUsage;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = preTransform;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    // Test for successful swapchain creation when GetPhysicalDeviceSurfaceCapabilitiesKHR() and
    // GetPhysicalDeviceSurfaceFormatsKHR() are queried as expected and GetPhysicalDeviceSurfacePresentModesKHR() is not called but
    // the present mode is VK_PRESENT_MODE_FIFO_KHR
    vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(Gpu(), m_surface.Handle(), &m_surface_capabilities);

    uint32_t format_count;
    vk::GetPhysicalDeviceSurfaceFormatsKHR(Gpu(), m_surface.Handle(), &format_count, nullptr);
    if (format_count != 0) {
        m_surface_formats.resize(format_count);
        vk::GetPhysicalDeviceSurfaceFormatsKHR(Gpu(), m_surface.Handle(), &format_count, m_surface_formats.data());
    }

    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = m_surface_capabilities.minImageExtent;

    // GetPhysicalDeviceSurfacePresentModesKHR() not called before trying to create a swapchain
    m_errorMonitor->SetDesiredWarning("BestPractices-vkCreateSwapchainKHR-present-mode-no-surface");

    // Warning is thrown any time the present mode is not VK_PRESENT_MODE_FIFO_KHR, but only with ARM BP
    m_errorMonitor->SetAllowedFailureMsg("BestPractices-Arm-vkCreateSwapchainKHR-swapchain-presentmode-not-fifo");

    // present mode might not be supported
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkSwapchainCreateInfoKHR-presentMode-01281");

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkSwapchainCreateInfoKHR-presentMode-02839");
    m_swapchain.Init(*m_device, swapchain_create_info);
    m_errorMonitor->VerifyFound();

    swapchain_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkSwapchainCreateInfoKHR-presentMode-02839");
    m_swapchain.Init(*m_device, swapchain_create_info);
}

TEST_F(VkBestPracticesLayerTest, ExpectedQueryDetails) {
    TEST_DESCRIPTION("Check that GetPhysicalDeviceQueueFamilyProperties is working as expected");

    // Vulkan 1.1 required to test vkGetPhysicalDeviceQueueFamilyProperties2
    app_info_.apiVersion = VK_API_VERSION_1_1;
    // VK_KHR_get_physical_device_properties2 required to test vkGetPhysicalDeviceQueueFamilyProperties2KHR
    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBestPracticesFramework());
    const vkt::PhysicalDevice phys_device_obj(gpu_);

    std::vector<VkQueueFamilyProperties> queue_family_props;

    // Ensure we can find a graphics queue family.
    uint32_t queue_count = 0;
    vk::GetPhysicalDeviceQueueFamilyProperties(phys_device_obj.handle(), &queue_count, nullptr);

    queue_family_props.resize(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(phys_device_obj.handle(), &queue_count, queue_family_props.data());

    // Now  for GetPhysicalDeviceQueueFamilyProperties2
    std::vector<VkQueueFamilyProperties2> queue_family_props2;
    vk::GetPhysicalDeviceQueueFamilyProperties2(phys_device_obj.handle(), &queue_count, nullptr);

    queue_family_props2.resize(queue_count);
    for (uint32_t i = 0; i < queue_count; i++) {
        queue_family_props2[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
    }
    vk::GetPhysicalDeviceQueueFamilyProperties2(phys_device_obj.handle(), &queue_count, queue_family_props2.data());

    // And for GetPhysicalDeviceQueueFamilyProperties2KHR
    vk::GetPhysicalDeviceQueueFamilyProperties2KHR(phys_device_obj.handle(), &queue_count, nullptr);

    queue_family_props2.resize(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties2KHR(phys_device_obj.handle(), &queue_count, queue_family_props2.data());

    vkt::Device device(phys_device_obj.handle());
}

TEST_F(VkBestPracticesLayerTest, MissingQueryDetails) {
    TEST_DESCRIPTION("Check that GetPhysicalDeviceQueueFamilyProperties generates appropriate query warning");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    const vkt::PhysicalDevice phys_device_obj(gpu_);

    std::vector<VkQueueFamilyProperties> queue_family_props(1);
    uint32_t queue_count = static_cast<uint32_t>(queue_family_props.size());

    // might only be a queue_count of 1, so check and then do "real" test to make sure error is detected
    m_errorMonitor->SetUnexpectedError("BestPractices-GetPhysicalDeviceQueueFamilyProperties-CountMismatch");
    vk::GetPhysicalDeviceQueueFamilyProperties(phys_device_obj.handle(), &queue_count, queue_family_props.data());
    m_errorMonitor->VerifyFound();

    if (queue_count > 1) {
        m_errorMonitor->SetDesiredWarning("BestPractices-GetPhysicalDeviceQueueFamilyProperties-CountMismatch");
        vk::GetPhysicalDeviceQueueFamilyProperties(phys_device_obj.handle(), &queue_count, queue_family_props.data());
        m_errorMonitor->VerifyFound();
    }

    // Now get information correctly
    vkt::QueueCreateInfoArray queue_info(phys_device_obj.queue_properties_, true);
    // Only request creation with queuefamilies that have at least one queue
    std::vector<VkDeviceQueueCreateInfo> create_queue_infos;
    auto qci = queue_info.Data();
    for (uint32_t j = 0; j < queue_info.Size(); ++j) {
        if (qci[j].queueCount) {
            create_queue_infos.push_back(qci[j]);
        }
    }

    VkPhysicalDeviceFeatures all_features{};
    VkDeviceCreateInfo device_ci = {};
    device_ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_ci.pNext = nullptr;
    device_ci.queueCreateInfoCount = create_queue_infos.size();
    device_ci.pQueueCreateInfos = create_queue_infos.data();
    device_ci.enabledLayerCount = 0;
    device_ci.ppEnabledLayerNames = NULL;
    device_ci.enabledExtensionCount = 0;
    device_ci.ppEnabledExtensionNames = nullptr;
    device_ci.pEnabledFeatures = &all_features;

    // vkGetPhysicalDeviceFeatures has not been called, so this should produce a warning
    m_errorMonitor->SetDesiredWarning("BestPractices-vkCreateDevice-physical-device-features-not-retrieved");
    VkDevice device;
    vk::CreateDevice(phys_device_obj.handle(), &device_ci, nullptr, &device);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, DepthBiasNoAttachment) {
    TEST_DESCRIPTION("Enable depthBias without a depth attachment");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.rs_state_ci_.depthBiasEnable = VK_TRUE;
    pipe.rs_state_ci_.depthBiasConstantFactor = 1.0f;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());

    m_errorMonitor->SetDesiredWarning("BestPractices-vkCmdDraw-DepthBiasNoAttachment");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(VkBestPracticesLayerTest, CreatePipelineVsFsTypeMismatchArraySize) {
    TEST_DESCRIPTION("Test that an error is produced for mismatched array sizes across the vertex->fragment shader interface");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) out float x[2];
        void main(){
           x[0] = 0; x[1] = 0;
           gl_Position = vec4(1);
        }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) in float x[1];
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(x[0]);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kPerformanceWarningBit, "WARNING-Shader-OutputNotConsumed");
}

TEST_F(VkBestPracticesLayerTest, WorkgroupSizeDeprecated) {
    TEST_DESCRIPTION("SPIR-V 1.6 deprecated WorkgroupSize build-in.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    const char *spv_source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpName %main "main"
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ =
            std::make_unique<VkShaderObj>(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kWarningBit, "BestPractices-SpirvDeprecated_WorkgroupSize");
}

TEST_F(VkBestPracticesLayerTest, ImageExtendedUsageWithoutMutableFormat) {
    TEST_DESCRIPTION("Create image with extended usage bit but not mutable format bit.");
    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    auto image_ci = vkt::Image::ImageCreateInfo2D(256, 256, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image_ci.flags = VK_IMAGE_CREATE_EXTENDED_USAGE_BIT;
    VkImage image;
    m_errorMonitor->SetDesiredWarning("BestPractices-vkCreateImage-CreateFlags");
    vk::CreateImage(device(), &image_ci, nullptr, &image);
    m_errorMonitor->VerifyFound();
}

#if GTEST_IS_THREADSAFE
TEST_F(VkBestPracticesLayerTest, ThreadUpdateDescriptorUpdateAfterBindNoCollision) {
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
#endif  // GTEST_IS_THREADSAFE

TEST_F(VkBestPracticesLayerTest, TransitionFromUndefinedToReadOnly) {
    TEST_DESCRIPTION("Transition image layout from undefined to read only");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM,
                     VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkClearColorValue color_clear_value = {};
    color_clear_value.uint32[0] = 255;
    VkImageSubresourceRange clear_range;
    clear_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clear_range.baseMipLevel = 0;
    clear_range.baseArrayLayer = 0;
    clear_range.layerCount = 1;
    clear_range.levelCount = 1;

    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.srcAccessMask = 0;
    img_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    img_barrier.image = image.handle();
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    m_command_buffer.Begin();

    vk::CmdClearColorImage(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, &color_clear_value, 1, &clear_range);

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdPipelineBarrier-pImageMemoryBarriers-02820");
    m_errorMonitor->SetDesiredWarning("BestPractices-ImageMemoryBarrier-TransitionUndefinedToReadOnly");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(VkBestPracticesLayerTest, SemaphoreSetWhenCountIsZero) {
    TEST_DESCRIPTION("Set semaphore in SubmitInfo but count is 0");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Semaphore semaphore(*m_device);
    VkSemaphore semaphore_handle = semaphore.handle();

    VkSubmitInfo signal_submit_info = vku::InitStructHelper();
    signal_submit_info.signalSemaphoreCount = 0;
    signal_submit_info.pSignalSemaphores = &semaphore_handle;

    m_errorMonitor->SetDesiredInfo("BestPractices-SignalSemaphores-SemaphoreCount");
    vk::QueueSubmit(m_default_queue->handle(), 1, &signal_submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    signal_submit_info.signalSemaphoreCount = 1;
    vk::QueueSubmit(m_default_queue->handle(), 1, &signal_submit_info, VK_NULL_HANDLE);

    VkSubmitInfo wait_submit_info = vku::InitStructHelper();
    wait_submit_info.waitSemaphoreCount = 0;
    wait_submit_info.pWaitSemaphores = &semaphore_handle;

    m_errorMonitor->SetDesiredInfo("BestPractices-WaitSemaphores-SemaphoreCount");
    vk::QueueSubmit(m_default_queue->handle(), 1, &wait_submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

TEST_F(VkBestPracticesLayerTest, OverAllocateFromDescriptorPool) {
    TEST_DESCRIPTION("Attempt to allocate more sets and descriptors than descriptor pool has available.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_SAMPLER;
    ds_type_count.descriptorCount = 2;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.flags = 0;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vkt::DescriptorPool ds_pool(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding_samp = {};
    dsl_binding_samp.binding = 0;
    dsl_binding_samp.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    dsl_binding_samp.descriptorCount = 1;
    dsl_binding_samp.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding_samp.pImmutableSamplers = NULL;

    const vkt::DescriptorSetLayout ds_layout_samp(*m_device, {dsl_binding_samp});

    // Try to allocate 2 sets when pool only has 1 set
    VkDescriptorSet descriptor_sets[2];
    VkDescriptorSetLayout set_layouts[2] = {ds_layout_samp.handle(), ds_layout_samp.handle()};
    VkDescriptorSetAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.descriptorSetCount = 2;
    alloc_info.descriptorPool = ds_pool.handle();
    alloc_info.pSetLayouts = set_layouts;
    m_errorMonitor->SetDesiredWarning("BestPractices-vkAllocateDescriptorSets-EmptyDescriptorPool");
    vk::AllocateDescriptorSets(device(), &alloc_info, descriptor_sets);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, OverAllocateTypeFromDescriptorPool) {
    TEST_DESCRIPTION("Attempt to allocate more sets and descriptors than descriptor pool has available.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_SAMPLER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = vku::InitStructHelper();
    ds_pool_ci.flags = 0;
    ds_pool_ci.maxSets = 2;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vkt::DescriptorPool ds_pool(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding_samp = {};
    dsl_binding_samp.binding = 0;
    dsl_binding_samp.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    dsl_binding_samp.descriptorCount = 1;
    dsl_binding_samp.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding_samp.pImmutableSamplers = NULL;

    const vkt::DescriptorSetLayout ds_layout_samp(*m_device, {dsl_binding_samp});

    // Try to allocate 2 sets when pool only has 1 set
    VkDescriptorSet descriptor_sets[2];
    VkDescriptorSetLayout set_layouts[2] = {ds_layout_samp.handle(), ds_layout_samp.handle()};
    VkDescriptorSetAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.descriptorSetCount = 2;
    alloc_info.descriptorPool = ds_pool.handle();
    alloc_info.pSetLayouts = set_layouts;
    m_errorMonitor->SetDesiredWarning("BestPractices-vkAllocateDescriptorSets-EmptyDescriptorPoolType");
    vk::AllocateDescriptorSets(device(), &alloc_info, descriptor_sets);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, RenderPassClearWithoutLoadOpClear) {
    TEST_DESCRIPTION("Test for clearing a RenderPass with non-zero clearValueCount without any VK_ATTACHMENT_LOAD_OP_CLEAR");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    // Setup necessary objects correctly

    // Bigger size to avoid small allocation best practices warning
    const unsigned int w = 1920;
    const unsigned int h = 1080;

    vkt::Image image(*m_device, w, h, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView image_view = image.CreateView();

    // Setup RenderPass
    VkAttachmentDescription attachment{};
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;  // Specify that we do nothing with the contents of the attached image
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachment.format = VK_FORMAT_R8G8B8A8_UNORM;

    VkAttachmentReference ar{};
    ar.attachment = 0;
    ar.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription spd{};
    spd.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    spd.colorAttachmentCount = 1;
    spd.pColorAttachments = &ar;

    VkRenderPassCreateInfo rp_info = vku::InitStructHelper();
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &attachment;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &spd;

    vkt::RenderPass rp(*m_device, rp_info);
    vkt::Framebuffer fb(*m_device, rp.handle(), 1, &image_view.handle(), w, h);

    m_command_buffer.Begin();

    // Create a useless VkClearValue
    VkClearValue cv{};
    cv.color = VkClearColorValue{};
    std::fill(std::begin(cv.color.float32), std::begin(cv.color.float32) + 4, 0.0f);

    VkRenderPassBeginInfo begin_info = vku::InitStructHelper();
    begin_info.clearValueCount = 1;  // Pass one clearValue, in conflict with attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE
    begin_info.pClearValues = &cv;
    begin_info.renderPass = rp.handle();
    begin_info.renderArea.extent.width = w;
    begin_info.renderArea.extent.height = h;
    begin_info.framebuffer = fb.handle();

    m_errorMonitor->SetDesiredWarning("BestPractices-ClearValueWithoutLoadOpClear");
    m_command_buffer.BeginRenderPass(begin_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(VkBestPracticesLayerTest, RenderPassClearValueCountHigherThanAttachmentCount) {
    TEST_DESCRIPTION(
        "Test for beginning a RenderPass with VkRenderPassBeginInfo.clearValueCount > VkRenderPassCreateInfo.attachmentCount");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    // Setup necessary objects correctly

    // Bigger size to avoid small allocation best practices warning
    const unsigned int w = 1920;
    const unsigned int h = 1080;

    vkt::Image image(*m_device, w, h, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView image_view = image.CreateView();

    // Setup RenderPass
    VkAttachmentDescription attachment{};
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.format = VK_FORMAT_R8G8B8A8_UNORM;

    VkAttachmentReference ar{};
    ar.attachment = 0;
    ar.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription spd{};
    spd.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    spd.colorAttachmentCount = 1;
    spd.pColorAttachments = &ar;

    VkRenderPassCreateInfo rp_info = vku::InitStructHelper();
    rp_info.attachmentCount = 1;  // There is only one attachment
    rp_info.pAttachments = &attachment;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &spd;

    vkt::RenderPass rp(*m_device, rp_info);
    vkt::Framebuffer fb(*m_device, rp.handle(), 1, &image_view.handle(), w, h);

    m_command_buffer.Begin();

    // Create two VkClearValues
    VkClearValue cv[2];

    // Create a useful VkClearValue
    cv[0].color = VkClearColorValue{};
    std::fill(std::begin(cv[0].color.float32), std::begin(cv[0].color.float32) + 4, 0.0f);

    // Create a useless VkClearValue
    cv[1].color = VkClearColorValue{};
    std::fill(std::begin(cv[1].color.float32), std::begin(cv[1].color.float32) + 4, 0.0f);

    VkRenderPassBeginInfo begin_info = vku::InitStructHelper();
    begin_info.clearValueCount = 2;  // Pass 2 clearValues, in conflict with VkRenderPassCreateInfo.attachmentCount == 1 meaning the
                                     // second clearValue will be ignored
    begin_info.pClearValues = cv;
    begin_info.renderPass = rp.handle();
    begin_info.renderArea.extent.width = w;
    begin_info.renderArea.extent.height = h;
    begin_info.framebuffer = fb.handle();

    m_errorMonitor->SetDesiredWarning("BestPractices-ClearValueCountHigherThanAttachmentCount");
    m_command_buffer.BeginRenderPass(begin_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(VkBestPracticesLayerTest, DontCareThenLoad) {
    TEST_DESCRIPTION("Test for storing an attachment with STORE_OP_DONT_CARE then loading with LOAD_OP_LOAD");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    // Setup necessary objects correctly

    const unsigned int w = 100;
    const unsigned int h = 100;

    vkt::Image image(*m_device, w, h, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView image_view = image.CreateView();

    // Setup first RenderPass
    VkAttachmentDescription attachment{};
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;        // Clearing as only modification
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;  // Dont care even though we will load afterwards
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.format = VK_FORMAT_R8G8B8A8_UNORM;

    VkAttachmentReference ar{};
    ar.attachment = 0;
    ar.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription spd{};
    spd.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    spd.colorAttachmentCount = 1;
    spd.pColorAttachments = &ar;

    VkRenderPassCreateInfo rp_info = vku::InitStructHelper();
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &attachment;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &spd;

    vkt::RenderPass rp1(*m_device, rp_info);

    // Setup second RenderPass
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;  // Loading even though was stored with dont care
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    vkt::RenderPass rp2(*m_device, rp_info);
    vkt::Framebuffer fb(*m_device, rp1.handle(), 1, &image_view.handle(), w, h);

    m_command_buffer.Begin();

    // All white
    VkClearValue cv;
    cv.color = VkClearColorValue{};
    std::fill(std::begin(cv.color.float32), std::begin(cv.color.float32) + 4, 1.0f);

    // Begin first renderpass
    m_command_buffer.BeginRenderPass(rp1.handle(), fb.handle(), w, h, 1, &cv);

    m_command_buffer.EndRenderPass();

    // Begin second renderpass
    m_command_buffer.BeginRenderPass(rp2.handle(), fb.handle(), w, h, 0, nullptr);

    m_command_buffer.EndRenderPass();

    m_command_buffer.End();

    m_errorMonitor->SetDesiredWarning("BestPractices-StoreOpDontCareThenLoadOpLoad");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

TEST_F(VkBestPracticesLayerTest, LoadDeprecatedExtension) {
    TEST_DESCRIPTION("Test for loading a vk1.3 deprecated extension with a 1.3 instance on a 1.2 or less device");

    SetTargetApiVersion(VK_API_VERSION_1_3);

    RETURN_IF_SKIP(InitBestPracticesFramework());

    const char *extension = VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME;

    if (!DeviceExtensionSupported(extension)) {
        GTEST_SKIP() << extension << " not supported.";
    }

    VkDeviceQueueCreateInfo qci = vku::InitStructHelper();
    qci.queueFamilyIndex = 0;
    float priority = 1;
    qci.pQueuePriorities = &priority;
    qci.queueCount = 1;

    VkDeviceCreateInfo dev_info = vku::InitStructHelper();
    dev_info.queueCreateInfoCount = 1;
    dev_info.pQueueCreateInfos = &qci;
    dev_info.enabledExtensionCount = 1;
    dev_info.ppEnabledExtensionNames = &extension;

    m_errorMonitor->SetDesiredWarning("BestPractices-deprecated-extension");
    // api version != device version
    m_errorMonitor->SetAllowedFailureMsg("BestPractices-vkCreateDevice-API-version-mismatch");

    VkDevice device = VK_NULL_HANDLE;
    vk::CreateDevice(Gpu(), &dev_info, nullptr, &device);

    if (DeviceValidationVersion() >= VK_API_VERSION_1_3) {
        m_errorMonitor->VerifyFound();
    }

    if (device) vk::DestroyDevice(device, nullptr);
}

TEST_F(VkBestPracticesLayerTest, ExclusiveImageMultiQueueUsage) {
    TEST_DESCRIPTION("Test for using a queue exclusive image on multiple queues");

    RETURN_IF_SKIP(InitBestPracticesFramework());
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
        GTEST_SKIP() << "No separate queue family from graphics queue";
    }

    // Setup necessary objects correctly

    const unsigned int w = 100;
    const unsigned int h = 100;

    vkt::Image image(*m_device, w, h, 1, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView image_view = image.CreateView();

    // Setup RenderPass
    VkAttachmentDescription attachment{};
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;    // Clearing so warning will not trigger on second pass
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;  // Store written image for next queue family
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachment.format = VK_FORMAT_R8G8B8A8_UNORM;

    VkAttachmentReference ar{};
    ar.attachment = 0;
    ar.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription spd{};
    spd.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    spd.colorAttachmentCount = 1;
    spd.pColorAttachments = &ar;

    VkRenderPassCreateInfo rp_info = vku::InitStructHelper();
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &attachment;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &spd;

    vkt::RenderPass rp(*m_device, rp_info);
    vkt::Framebuffer fb(*m_device, rp.handle(), 1, &image_view.handle(), w, h);

    vkt::CommandPool graphics_pool(*m_device, graphics_queue->family_index);

    vkt::CommandBuffer graphics_buffer(*m_device, graphics_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VkClearValue cv;
    cv.color = VkClearColorValue{};
    std::fill(std::begin(cv.color.float32), std::begin(cv.color.float32) + 4, 1.0f);

    VkRenderPassBeginInfo begin_info = vku::InitStructHelper();
    begin_info.clearValueCount = 1;
    begin_info.pClearValues = &cv;
    begin_info.renderPass = rp.handle();
    begin_info.renderArea.extent.width = w;
    begin_info.renderArea.extent.height = h;
    begin_info.framebuffer = fb.handle();

    // Prepare compute

    const char *cs = R"glsl(#version 450
    layout(local_size_x=1, local_size_y=1) in;
    layout(set=0, binding=0, rgba32f) uniform image2D img;
    void main(){
        vec4 v = imageLoad(img, ivec2(gl_GlobalInvocationID.xy));
    }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.dsl_bindings_[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    pipe.dsl_bindings_[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pipe.CreateComputePipeline();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                   VK_IMAGE_LAYOUT_GENERAL);
    pipe.descriptor_set_->UpdateDescriptorSets();

    vkt::CommandPool compute_pool(*m_device, compute_queue->family_index);

    vkt::CommandBuffer compute_buffer(*m_device, compute_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    // Record command buffers without queue transition

    // Record graphics command buffer
    graphics_buffer.Begin();

    graphics_buffer.BeginRenderPass(begin_info);

    graphics_buffer.EndRenderPass();

    graphics_buffer.End();

    graphics_queue->Submit(graphics_buffer);
    graphics_queue->Wait();

    // Record compute command buffer
    compute_buffer.Begin();

    vk::CmdBindPipeline(compute_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());

    vk::CmdBindDescriptorSets(compute_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);

    vk::CmdDispatch(compute_buffer.handle(), w, h, 1);

    compute_buffer.End();

    // Warning should trigger as we are potentially accessing undefined resources
    m_errorMonitor->SetDesiredWarning("BestPractices-ConcurrentUsageOfExclusiveImage");
    compute_queue->Submit(compute_buffer);
    compute_queue->Wait();
    m_errorMonitor->VerifyFound();

    vk::ResetCommandPool(device(), graphics_pool.handle(), 0);
    vk::ResetCommandPool(device(), compute_pool.handle(), 0);

    // Record command buffers with queue transition

    // Queue transition barrier, same for release and acquire
    VkImageMemoryBarrier barrier = vku::InitStructHelper();
    barrier.image = image.handle();
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;  // only matters for release
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;             // only matters for acquire
    barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.srcQueueFamilyIndex = graphics_queue->family_index;
    barrier.dstQueueFamilyIndex = compute_queue->family_index;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    // Record graphics command buffer
    graphics_buffer.Begin();

    graphics_buffer.BeginRenderPass(begin_info);

    graphics_buffer.EndRenderPass();

    vk::CmdPipelineBarrier(graphics_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &barrier);

    graphics_buffer.End();
    graphics_queue->Submit(graphics_buffer);
    graphics_queue->Wait();

    // Record compute command buffer
    compute_buffer.Begin();

    vk::CmdPipelineBarrier(compute_buffer.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &barrier);

    vk::CmdBindPipeline(compute_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());

    vk::CmdBindDescriptorSets(compute_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);

    vk::CmdDispatch(compute_buffer.handle(), w, h, 1);

    compute_buffer.End();

    // Warning shouldn't trigger
    m_errorMonitor->SetDesiredWarning("BestPractices-ConcurrentUsageOfExclusiveImage");
    compute_queue->Submit(compute_buffer);
    compute_queue->Wait();
    m_errorMonitor->Finish();
}

TEST_F(VkBestPracticesLayerTest, ImageMemoryBarrierAccessLayoutCombinations) {
    TEST_DESCRIPTION("Transition image layout from undefined to read only");
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM,
                     VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkClearColorValue color_clear_value = {};
    color_clear_value.uint32[0] = 255;
    VkImageSubresourceRange clear_range;
    clear_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clear_range.baseMipLevel = 0;
    clear_range.baseArrayLayer = 0;
    clear_range.layerCount = 1;
    clear_range.levelCount = 1;

    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.srcAccessMask = 0;
    img_barrier.dstAccessMask = 0;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    img_barrier.image = image.handle();
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    m_command_buffer.Begin();

    vk::CmdClearColorImage(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, &color_clear_value, 1, &clear_range);

    // GENERAL - Any
    // note: the table in PR 2918 originally said that 0 was not allowed, but this was incorrect. See Issue #4735
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.dstAccessMask = 0;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);

    // Every table entry includes an implicit "can be 0"
    img_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.dstAccessMask = 0;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);

    img_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    img_barrier.dstAccessMask = 0;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);

    // PRESENT_SRC_KHR - Must be 0
    img_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    img_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    m_errorMonitor->SetDesiredWarning("BestPractices-ImageBarrierAccessLayout");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);
    m_errorMonitor->VerifyFound();

    img_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    img_barrier.dstAccessMask = 0;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);

    {
        VkImageMemoryBarrier2 img_barrier2 = vku::InitStructHelper();
        img_barrier2.srcAccessMask = 0;
        img_barrier2.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        img_barrier2.image = image.handle();
        img_barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        img_barrier2.subresourceRange.baseArrayLayer = 0;
        img_barrier2.subresourceRange.baseMipLevel = 0;
        img_barrier2.subresourceRange.layerCount = 1;
        img_barrier2.subresourceRange.levelCount = 1;

        VkDependencyInfo dependency_info = vku::InitStructHelper();
        dependency_info.imageMemoryBarrierCount = 1;
        dependency_info.pImageMemoryBarriers = &img_barrier2;

        img_barrier2.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
        img_barrier2.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageMemoryBarrier2-dstAccessMask-03903");
        m_errorMonitor->SetDesiredWarning("BestPractices-ImageBarrierAccessLayout");
        vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);
        m_errorMonitor->VerifyFound();

        // make sure bits above UINT32_MAX are detected
        img_barrier2.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        img_barrier2.dstAccessMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageMemoryBarrier2-dstAccessMask-03907");
        m_errorMonitor->SetDesiredWarning("BestPractices-ImageBarrierAccessLayout");
        vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);
        m_errorMonitor->VerifyFound();

        img_barrier2.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        img_barrier2.dstAccessMask = 0;
        vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

        m_command_buffer.End();
    }
}

TEST_F(VkBestPracticesLayerTest, NonSimultaneousSecondaryMarksPrimary) {
    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    secondary.Begin();
    secondary.End();

    VkCommandBufferBeginInfo cbbi = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
        nullptr,
    };

    m_command_buffer.Begin(&cbbi);
    m_errorMonitor->SetDesiredWarning("BestPractices-vkCmdExecuteCommands-CommandBufferSimultaneousUse");
    vk::CmdExecuteCommands(m_command_buffer.handle(), 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(VkBestPracticesLayerTest, NoCreateSwapchainPresentModes) {
    TEST_DESCRIPTION("With swapchain maintenance 1, CreateSwapchain with VkPresentModesCreateInfoEXT");

    AddSurfaceExtension();
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBestPracticesFramework());

    RETURN_IF_SKIP(InitState());
    RETURN_IF_SKIP(InitSurface());
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkSwapchainCreateInfoKHR-presentMode-02839");
    m_errorMonitor->SetDesiredWarning("BestPractices-vkCreateSwapchainKHR-no-VkSwapchainPresentModesCreateInfoEXT-provided");
    m_swapchain = CreateSwapchain(m_surface.Handle(), VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, PipelineWithoutRenderPassOrRenderingInfo) {
    TEST_DESCRIPTION("Create pipeline with VK_NULL_HANDLE render pass and no VkPipelineRenderingCreateInfo in pNext chain");
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.cb_ci_.attachmentCount = 0;
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->SetDesiredWarning("BestPractices-Pipeline-NoRendering");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, GetQueryPoolResultsWithoutBegin) {
    TEST_DESCRIPTION("Get query pool results without ever beginning the query");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0u, 1u);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    uint32_t data = 0u;
    m_errorMonitor->SetDesiredWarning("BestPractices-QueryPool-Unavailable");
    vk::GetQueryPoolResults(*m_device, query_pool.handle(), 0u, 1u, sizeof(uint32_t), &data, sizeof(uint32_t), 0u);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, NonOptimalResolveFormat) {
    TEST_DESCRIPTION("Create a render pass with a resolve attachment that is not optimal");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    VkSubpassResolvePerformanceQueryEXT performance_query = vku::InitStructHelper();
    VkFormatProperties2 format_properties2 = vku::InitStructHelper(&performance_query);
    vk::GetPhysicalDeviceFormatProperties2(Gpu(), format, &format_properties2);
    if (performance_query.optimal == VK_TRUE) {
        GTEST_SKIP() << "VkSubpassResolvePerformanceQueryEXT::optimal required to be VK_FALSE.";
    }

    VkAttachmentReference color_attachment;
    color_attachment.attachment = 0u;
    color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkAttachmentReference resolve_attachment;
    resolve_attachment.attachment = 1u;
    resolve_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription attachments[2];
    attachments[0] = {};
    attachments[0].format = format;
    attachments[0].samples = VK_SAMPLE_COUNT_2_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[1] = {};
    attachments[1].format = format;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1u;
    subpass.pColorAttachments = &color_attachment;
    subpass.pResolveAttachments = &resolve_attachment;

    VkRenderPassCreateInfo render_pass_ci = vku::InitStructHelper();
    render_pass_ci.attachmentCount = 2u;
    render_pass_ci.pAttachments = attachments;
    render_pass_ci.subpassCount = 1u;
    render_pass_ci.pSubpasses = &subpass;

    VkRenderPass render_pass;
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "BestPractices-vkCreateRenderPass-SubpassResolve-NonOptimalFormat");
    vk::CreateRenderPass(*m_device, &render_pass_ci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, PartialPushConstantSetEnd) {
    TEST_DESCRIPTION("Set only a part of push constants at end of a struct");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    char const *const vsSource = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo { uint x[2]; } constants;
        void main(){
           gl_Position = vec4(constants.x[0] * constants.x[1]);
        }
    )glsl";

    VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj const fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    uint32_t data[2] = {1u, 2u};
    VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(data)};

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {}, {push_constant_range});
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdPushConstants(m_command_buffer.handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t),
                         data);

    m_errorMonitor->SetDesiredWarning("BestPractices-PushConstants");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    vk::CmdPushConstants(m_command_buffer.handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(uint32_t),
                         sizeof(uint32_t), &data[1]);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(VkBestPracticesLayerTest, PartialPushConstantSetMiddle) {
    TEST_DESCRIPTION("Set only a part of push constants in middle of as struct");
    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    char const *const vsSource = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo {
            uint a; // set
            uint b; // not set
            uint c; // set
        } constants;
        void main(){
           gl_Position = vec4(float(constants.a * constants.b * constants.c));
        }
    )glsl";

    VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj const fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    uint32_t data = 1u;
    VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t) * 3};

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {}, {push_constant_range});
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdPushConstants(m_command_buffer.handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t),
                         &data);
    vk::CmdPushConstants(m_command_buffer.handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT,
                         sizeof(uint32_t) * 2, sizeof(uint32_t), &data);

    m_errorMonitor->SetDesiredWarning("BestPractices-PushConstants");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7495
TEST_F(VkBestPracticesLayerTest, IgnoreResolveImageView) {
    TEST_DESCRIPTION("Help warn user when they might have resolveMode set to NONE by accident");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    vkt::Image image(*m_device, image_create_info, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    vkt::Image resolve_image(*m_device, image_create_info, vkt::set_layout);
    vkt::ImageView resolve_image_view = resolve_image.CreateView();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredWarning("BestPractices-VkRenderingInfo-ResolveModeNone");
    m_command_buffer.BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(VkBestPracticesLayerTest, SetSignaledEvent) {
    TEST_DESCRIPTION("Signal event two times");
    RETURN_IF_SKIP(InitBestPractices());

    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    m_errorMonitor->SetDesiredWarning("BestPractices-Event-SignalSignaledEvent");
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(VkBestPracticesLayerTest, SetSignaledEvent2) {
    TEST_DESCRIPTION("Signal event two times using CmdSetEvent2 api");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBestPractices());

    vkt::Event event(*m_device);

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.memoryBarrierCount = 1;
    dep_info.pMemoryBarriers = &barrier;

    m_command_buffer.Begin();
    vk::CmdSetEvent2(m_command_buffer.handle(), event, &dep_info);
    m_errorMonitor->SetDesiredWarning("BestPractices-Event-SignalSignaledEvent");
    vk::CmdSetEvent2(m_command_buffer.handle(), event, &dep_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(VkBestPracticesLayerTest, SetEventSignaledByHost) {
    TEST_DESCRIPTION("Set event that was previously set be the host");
    RETURN_IF_SKIP(InitBestPractices());

    vkt::Event event(*m_device);
    vk::SetEvent(*m_device, event);

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    m_command_buffer.End();
    m_errorMonitor->SetDesiredWarning("BestPractices-Event-SignalSignaledEvent");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkBestPracticesLayerTest, SetSignaledEventMultipleSubmits) {
    TEST_DESCRIPTION("Set event from different submits");
    RETURN_IF_SKIP(InitBestPractices());

    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);

    vkt::CommandBuffer cb2(*m_device, m_command_pool);
    cb2.Begin();
    cb2.SetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    cb2.End();
    m_errorMonitor->SetDesiredWarning("BestPractices-Event-SignalSignaledEvent");
    m_default_queue->Submit(cb2);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(VkBestPracticesLayerTest, SetSignaledEventMultipleSubmits2) {
    TEST_DESCRIPTION("Set event from multiple submits using QueueSubmit2 api");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBestPractices());

    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    m_command_buffer.End();
    m_default_queue->Submit2(m_command_buffer);

    vkt::CommandBuffer cb2(*m_device, m_command_pool);
    cb2.Begin();
    cb2.SetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    cb2.End();
    m_errorMonitor->SetDesiredWarning("BestPractices-Event-SignalSignaledEvent");
    m_default_queue->Submit2(cb2);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(VkBestPracticesLayerTest, SetSignaledEventSecondary) {
    TEST_DESCRIPTION("Set event in the primary command buffer and then one more time in the secondary");
    RETURN_IF_SKIP(InitBestPractices());

    vkt::Event event(*m_device);

    vkt::CommandBuffer secondary_cb(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary_cb.Begin();
    secondary_cb.SetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    secondary_cb.End();

    m_command_buffer.Begin();
    m_command_buffer.SetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    m_errorMonitor->SetDesiredWarning("BestPractices-Event-SignalSignaledEvent");
    m_command_buffer.ExecuteCommands(secondary_cb);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(VkBestPracticesLayerTest, SetSignaledEventSecondary2) {
    TEST_DESCRIPTION("Set event in different secondary command buffers");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBestPractices());

    vkt::Event event(*m_device);

    vkt::CommandBuffer cb(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBufferInheritanceInfo inheritanc_info = vku::InitStructHelper();
    VkCommandBufferBeginInfo cb_begin_info = vku::InitStructHelper();
    cb_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    cb_begin_info.pInheritanceInfo = &inheritanc_info;
    cb.Begin(&cb_begin_info);
    cb.SetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    cb.End();
    const VkCommandBuffer secondary_cbs[2] = {cb.handle(), cb.handle()};

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredWarning("BestPractices-Event-SignalSignaledEvent");
    vk::CmdExecuteCommands(m_command_buffer.handle(), 2, secondary_cbs);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(VkBestPracticesLayerTest, SetSignaledEventSecondary3) {
    TEST_DESCRIPTION("Set event in the secondary command buffer and in the primary from different submissions");
    RETURN_IF_SKIP(InitBestPractices());

    vkt::Event event(*m_device);

    vkt::CommandBuffer secondary_cb(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary_cb.Begin();
    secondary_cb.SetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    secondary_cb.End();

    m_command_buffer.Begin();
    m_command_buffer.ExecuteCommands(secondary_cb);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);

    vkt::CommandBuffer cb2(*m_device, m_command_pool);
    cb2.Begin();
    cb2.SetEvent(event, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    cb2.End();
    m_errorMonitor->SetDesiredWarning("BestPractices-Event-SignalSignaledEvent");
    m_default_queue->Submit(cb2);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(VkBestPracticesLayerTest, PartialPushConstantSetEndCompute) {
    TEST_DESCRIPTION("Set only a part of push constants at end of a struct");

    RETURN_IF_SKIP(InitBestPracticesFramework());
    RETURN_IF_SKIP(InitState());

    char const *const csSource = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo { uint x[2]; } constants;
        layout(set = 0, binding = 0) buffer bar { vec4 r; } res;
        void main(){
           res.r = vec4(constants.x[0] * constants.x[1]);
        }
    )glsl";

    uint32_t data[2] = {1u, 2u};
    VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(data)};

    vkt::Buffer buffer(*m_device, 16u, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                       });
    descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0u, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_}, {push_constant_range});
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0u, 1u,
                              &descriptor_set.set_, 0u, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdPushConstants(m_command_buffer.handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0,
                         sizeof(uint32_t), data);

    m_errorMonitor->SetDesiredWarning("BestPractices-PushConstants");
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();

    vk::CmdPushConstants(m_command_buffer.handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_COMPUTE_BIT, sizeof(uint32_t),
                         sizeof(uint32_t), &data[1]);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);

    m_command_buffer.End();
}
