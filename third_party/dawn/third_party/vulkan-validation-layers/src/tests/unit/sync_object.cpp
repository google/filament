/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2020-2024 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>
#include <thread>
#include "utils/cast_utils.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/barrier_queue_family.h"
#include "../framework/render_pass_helper.h"

class NegativeSyncObject : public SyncObjectTest {};

TEST_F(NegativeSyncObject, ImageBarrierSubpassConflicts) {
    TEST_DESCRIPTION("Add a pipeline barrier within a subpass that has conflicting state");
    RETURN_IF_SKIP(Init());

    // A renderpass with a single subpass that declared a self-dependency
    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
    };
    VkSubpassDependency dep = {0,
                               0,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_DEPENDENCY_BY_REGION_BIT};
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 1, subpasses, 1, &dep};
    vkt::RenderPass rp(*m_device, rpci);

    rpci.dependencyCount = 0;
    rpci.pDependencies = nullptr;

    vkt::RenderPass rp_noselfdep(*m_device, rpci);
    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::ImageView imageView = image.CreateView();
    vkt::Framebuffer fb(*m_device, rp.handle(), 1, &imageView.handle());
    vkt::Framebuffer fb_noselfdep(*m_device, rp_noselfdep.handle(), 1, &imageView.handle());

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp_noselfdep.handle(), fb_noselfdep.handle(), 32, 32);
    VkMemoryBarrier mem_barrier = vku::InitStructHelper();
    mem_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-None-07889");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 1,
                           &mem_barrier, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();

    m_command_buffer.BeginRenderPass(rp.handle(), fb.handle(), 32, 32);
    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.image = image.handle();
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    // Mis-match src stage mask
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-None-07889");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
    m_errorMonitor->VerifyFound();

    // // Now mis-match dst stage mask
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-None-07889");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_HOST_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
    m_errorMonitor->VerifyFound();

    // Set srcQueueFamilyIndex to a different value than dstQueueFamilyIndex
    img_barrier.srcQueueFamilyIndex = 0;
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-srcQueueFamilyIndex-01182");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    m_errorMonitor->VerifyFound();

    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    // Mis-match mem barrier src access mask
    mem_barrier = vku::InitStructHelper();
    mem_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-None-07889");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 1, &mem_barrier, 0, nullptr,
                           0, nullptr);
    m_errorMonitor->VerifyFound();

    // Mis-match mem barrier dst access mask. Also set srcAccessMask to 0 which should not cause an error
    mem_barrier.srcAccessMask = 0;
    mem_barrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-None-07889");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 1, &mem_barrier, 0, nullptr,
                           0, nullptr);
    m_errorMonitor->VerifyFound();

    // Mis-match image barrier src access mask
    img_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-None-07889");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    m_errorMonitor->VerifyFound();

    // Mis-match image barrier dst access mask
    img_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-None-07889");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    m_errorMonitor->VerifyFound();

    // Mis-match dependencyFlags
    img_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-None-07889");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0 /* wrong */, 0, nullptr, 0, nullptr, 1, &img_barrier);
    m_errorMonitor->VerifyFound();

    // Send non-zero bufferMemoryBarrierCount
    // Construct a valid BufferMemoryBarrier to avoid any parameter errors
    // First we need a valid buffer to reference
    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    VkBufferMemoryBarrier bmb = vku::InitStructHelper();
    bmb.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    bmb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    bmb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bmb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bmb.buffer = buffer.handle();
    bmb.offset = 0;
    bmb.size = VK_WHOLE_SIZE;
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-bufferMemoryBarrierCount-01178");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &bmb, 0,
                           nullptr);
    m_errorMonitor->VerifyFound();

    // Add image barrier w/ image handle that's not in framebuffer
    vkt::Image lone_image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    img_barrier.image = lone_image.handle();
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-image-04073");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    m_errorMonitor->VerifyFound();

    // Have image barrier with mis-matched layouts
    img_barrier.image = image.handle();
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-oldLayout-01181");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    m_errorMonitor->VerifyFound();

    img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-oldLayout-01181");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
}

TEST_F(NegativeSyncObject, BufferMemoryBarrierNoBuffer) {
    // Try to add a buffer memory barrier with no buffer.
    m_errorMonitor->SetDesiredError("UNASSIGNED-GeneralParameterError-RequiredHandle");

    RETURN_IF_SKIP(Init());
    m_command_buffer.Begin();

    VkBufferMemoryBarrier buf_barrier = vku::InitStructHelper();
    buf_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    buf_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    buf_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buf_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buf_barrier.buffer = VK_NULL_HANDLE;
    buf_barrier.offset = 0;
    buf_barrier.size = VK_WHOLE_SIZE;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0,
                           nullptr, 1, &buf_barrier, 0, nullptr);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, Barriers) {
    TEST_DESCRIPTION("A variety of ways to get VK_INVALID_BARRIER ");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::separateDepthStencilLayouts);
    // Make sure extensions for multi-planar and separate depth stencil images are enabled if possible
    AddOptionalExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework(&kDisableMessageLimit));
    RETURN_IF_SKIP(InitState());
    const bool mp_extensions = IsExtensionsEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    const bool external_memory = IsExtensionsEnabled(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    const bool maintenance2 = IsExtensionsEnabled(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    const bool feedback_loop_layout = IsExtensionsEnabled(VK_EXT_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_EXTENSION_NAME);
    const bool video_decode_queue = IsExtensionsEnabled(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME);
    const bool video_encode_queue = IsExtensionsEnabled(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME);
    const bool video_encode_quantization_map = IsExtensionsEnabled(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);

    vkt::Image color_image(*m_device, m_width, m_height, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    auto color_view = color_image.CreateView();

    // Just using all framebuffer-space pipeline stages in order to get a reasonably large
    //  set of bits that can be used for both src & dst
    VkPipelineStageFlags stage_flags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                       VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // Add all of the gfx mem access bits that correlate to the fb-space pipeline stages
    VkAccessFlags access_flags = VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
                                 VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                 VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                 VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    // Add a token self-dependency for this test to avoid unexpected errors
    rp.AddSubpassDependency(stage_flags, stage_flags, access_flags, access_flags);
    rp.CreateRenderPass();
    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &color_view.handle());

    auto depth_format = FindSupportedDepthStencilFormat(Gpu());

    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t invalid = static_cast<uint32_t>(m_device->Physical().queue_properties_.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (invalid == 1) || (m_device->Physical().queue_properties_[other_family].queueCount == 0) ||
                                 ((m_device->Physical().queue_properties_[other_family].queueFlags & VK_QUEUE_TRANSFER_BIT) == 0);
    std::vector<uint32_t> qf_indices{{submit_family, other_family}};
    if (only_one_family) {
        qf_indices.resize(1);
    }
    BarrierQueueFamilyTestHelper::Context test_context(this, qf_indices);

    // Use image unbound to memory in barrier
    // Use buffer unbound to memory in barrier
    BarrierQueueFamilyTestHelper conc_test(&test_context);
    conc_test.Init(nullptr, false, false);

    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    conc_test(" used with no memory bound. Memory should be bound by calling vkBindImageMemory()",
              " used with no memory bound. Memory should be bound by calling vkBindBufferMemory()");

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    conc_test.buffer_barrier_.buffer = buffer.handle();

    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    conc_test.image_barrier_.image = image.handle();

    // New layout can't be UNDEFINED
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    conc_test("VUID-VkImageMemoryBarrier-newLayout-01198", "");

    // Transition image to color attachment optimal
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    conc_test("");

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle());

    // Can't send buffer memory barrier during a render pass
    m_command_buffer.EndRenderPass();

    // Duplicate barriers that change layout
    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.image = image.handle();
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    VkImageMemoryBarrier img_barriers[2] = {img_barrier, img_barrier};

    // Transitions from UNDEFINED  are valid, even if duplicated
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 2,
                           img_barriers);

    // Duplication of layout transitions (not from undefined) are not valid
    img_barriers[0].oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barriers[0].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barriers[1].oldLayout = img_barriers[0].oldLayout;
    img_barriers[1].newLayout = img_barriers[0].newLayout;
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-oldLayout-01197");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 2,
                           img_barriers);
    m_errorMonitor->VerifyFound();

    if (!external_memory) {
        printf("External memory extension not supported, skipping external queue family subcase\n");
    } else {
        // Transitions to and from EXTERNAL within the same command buffer are valid, if pointless.
        img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        img_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        img_barrier.srcQueueFamilyIndex = submit_family;
        img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
        img_barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        img_barrier.dstAccessMask = 0;
        vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               0, 0, nullptr, 0, nullptr, 1, &img_barrier);
        img_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
        img_barrier.dstQueueFamilyIndex = submit_family;
        img_barrier.srcAccessMask = 0;
        img_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               0, 0, nullptr, 0, nullptr, 1, &img_barrier);
    }

    // Exceed the buffer size
    conc_test.buffer_barrier_.offset = conc_test.buffer_.CreateInfo().size + 1;
    conc_test("", "VUID-VkBufferMemoryBarrier-offset-01187");

    conc_test.buffer_barrier_.offset = 0;
    conc_test.buffer_barrier_.size = conc_test.buffer_.CreateInfo().size + 1;
    // Size greater than total size
    conc_test("", "VUID-VkBufferMemoryBarrier-size-01189");

    conc_test.buffer_barrier_.size = 0;
    // Size is zero
    conc_test("", "VUID-VkBufferMemoryBarrier-size-01188");

    conc_test.buffer_barrier_.size = VK_WHOLE_SIZE;

    // Now exercise barrier aspect bit errors, first DS
    vkt::Image ds_image(*m_device, 128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    conc_test.image_barrier_.image = ds_image.handle();

    // Not having DEPTH or STENCIL set is an error
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-subresourceRange-09601");
    conc_test("VUID-VkImageMemoryBarrier-image-03319");

    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

    conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    conc_test("VUID-VkImageMemoryBarrier-aspectMask-08702");

    conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    conc_test("VUID-VkImageMemoryBarrier-aspectMask-08703");

    conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Having anything other than DEPTH and STENCIL is an error
    conc_test.image_barrier_.subresourceRange.aspectMask =
        VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_COLOR_BIT;
    conc_test("VUID-VkImageMemoryBarrier-subresourceRange-09601");

    // Now test depth-only
    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), VK_FORMAT_D16_UNORM, &format_props);
    if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        vkt::Image d_image(*m_device, 128, 128, 1, VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

        conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        conc_test.image_barrier_.image = d_image.handle();

        // DEPTH bit must be set
        conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
        conc_test("VUID-VkImageMemoryBarrier-subresourceRange-09601");

        // No bits other than DEPTH may be set
        conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_COLOR_BIT;
        conc_test("VUID-VkImageMemoryBarrier-subresourceRange-09601");
    }

    // Now test stencil-only
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), VK_FORMAT_S8_UINT, &format_props);
    if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        vkt::Image s_image(*m_device, 128, 128, 1, VK_FORMAT_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

        conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        conc_test.image_barrier_.image = s_image.handle();

        // Use of COLOR aspect on depth image is error
        conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        // must have the VK_IMAGE_ASPECT_STENCIL_BIT set
        conc_test("VUID-VkImageMemoryBarrier-subresourceRange-09601");
    }

    // Finally test color
    vkt::Image c_image(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    conc_test.image_barrier_.image = c_image.handle();

    // COLOR bit must be set
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-subresourceRange-09601");
    conc_test("VUID-VkImageMemoryBarrier-image-09241");

    // No bits other than COLOR may be set
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-subresourceRange-09601");
    conc_test("VUID-VkImageMemoryBarrier-image-09241");

    // Test multip-planar image
    if (mp_extensions) {
        VkFormatProperties format_properties;
        VkFormat mp_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
        vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), mp_format, &format_properties);
        constexpr VkImageAspectFlags disjoint_sampled = VK_FORMAT_FEATURE_DISJOINT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
        if (disjoint_sampled == (format_properties.optimalTilingFeatures & disjoint_sampled)) {
            VkImageCreateInfo image_create_info = vku::InitStructHelper();
            image_create_info.imageType = VK_IMAGE_TYPE_2D;
            image_create_info.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
            image_create_info.extent.width = 64;
            image_create_info.extent.height = 64;
            image_create_info.extent.depth = 1;
            image_create_info.mipLevels = 1;
            image_create_info.arrayLayers = 1;
            image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
            image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
            image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
            image_create_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;

            VkImage mp_image;
            VkDeviceMemory plane_0_memory;
            VkDeviceMemory plane_1_memory;
            ASSERT_EQ(VK_SUCCESS, vk::CreateImage(device(), &image_create_info, NULL, &mp_image));

            VkImagePlaneMemoryRequirementsInfo image_plane_req = vku::InitStructHelper();
            image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;

            VkImageMemoryRequirementsInfo2 mem_req_info2 = vku::InitStructHelper(&image_plane_req);
            mem_req_info2.image = mp_image;
            VkMemoryRequirements2 mem_req2 = vku::InitStructHelper();
            vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mem_req2);

            // Find a valid memory type index to memory to be allocated from
            VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
            alloc_info.allocationSize = mem_req2.memoryRequirements.size;
            ASSERT_TRUE(m_device->Physical().SetMemoryType(mem_req2.memoryRequirements.memoryTypeBits, &alloc_info, 0));
            ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &alloc_info, NULL, &plane_0_memory));

            image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;
            vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mem_req2);
            alloc_info.allocationSize = mem_req2.memoryRequirements.size;
            ASSERT_TRUE(m_device->Physical().SetMemoryType(mem_req2.memoryRequirements.memoryTypeBits, &alloc_info, 0));
            ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &alloc_info, NULL, &plane_1_memory));

            VkBindImagePlaneMemoryInfo plane_0_memory_info = vku::InitStructHelper();
            plane_0_memory_info.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
            VkBindImagePlaneMemoryInfo plane_1_memory_info = vku::InitStructHelper();
            plane_1_memory_info.planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;

            VkBindImageMemoryInfo bind_image_info[2];
            bind_image_info[0] = vku::InitStructHelper(&plane_0_memory_info);
            bind_image_info[0].image = mp_image;
            bind_image_info[0].memory = plane_0_memory;
            bind_image_info[0].memoryOffset = 0;
            bind_image_info[1] = bind_image_info[0];
            bind_image_info[1].pNext = &plane_1_memory_info;
            bind_image_info[1].memory = plane_1_memory;
            vk::BindImageMemory2KHR(device(), 2, bind_image_info);

            conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            conc_test.image_barrier_.image = mp_image;

            // Test valid usage first
            conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
            conc_test("", "", VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, true);

            conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-subresourceRange-09601");
            conc_test("VUID-VkImageMemoryBarrier-image-01672");

            conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT;
            m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-subresourceRange-09601");
            conc_test("VUID-VkImageMemoryBarrier-image-01672");

            vk::FreeMemory(device(), plane_0_memory, NULL);
            vk::FreeMemory(device(), plane_1_memory, NULL);
            vk::DestroyImage(device(), mp_image, nullptr);
        }
    }

    // A barrier's new and old VkImageLayout must be compatible with an image's VkImageUsageFlags.
    {
        vkt::Image img_color(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        vkt::Image img_ds(*m_device, 128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        vkt::Image img_xfer_src(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        vkt::Image img_xfer_dst(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
        vkt::Image img_sampled(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
        vkt::Image img_input(*m_device, 128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
        struct BadBufferTest {
            vkt::Image &image_obj;
            VkImageLayout bad_layout;
            std::string msg_code;
        };
        // clang-format off
        std::vector<BadBufferTest> bad_buffer_layouts = {
            // images _without_ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
            {img_ds,       VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01208"},
            {img_xfer_src, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01208"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01208"},
            {img_sampled,  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01208"},
            {img_input,    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01208"},
            // images _without_ VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
            {img_color,    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01209"},
            {img_xfer_src, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01209"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01209"},
            {img_sampled,  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01209"},
            {img_input,    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01209"},
            {img_color,    VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier-oldLayout-01210"},
            {img_xfer_src, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier-oldLayout-01210"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier-oldLayout-01210"},
            {img_sampled,  VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier-oldLayout-01210"},
            {img_input,    VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier-oldLayout-01210"},
            // images _without_ VK_IMAGE_USAGE_SAMPLED_BIT or VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
            {img_color,    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01211"},
            {img_ds,       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01211"},
            {img_xfer_src, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01211"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01211"},
            // images _without_ VK_IMAGE_USAGE_TRANSFER_SRC_BIT
            {img_color,    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01212"},
            {img_ds,       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01212"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01212"},
            {img_sampled,  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01212"},
            {img_input,    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01212"},
            // images _without_ VK_IMAGE_USAGE_TRANSFER_DST_BIT
            {img_color,    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01213"},
            {img_ds,       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01213"},
            {img_xfer_src, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01213"},
            {img_sampled,  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01213"},
            {img_input,    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01213"},
            // images _without_ VK_KHR_maintenance2 added layouts
            {img_color,    VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01658"},
            {img_xfer_src, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01658"},
            {img_sampled,  VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01658"},
            {img_input,    VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01658"},
            {img_color,    VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01659"},
            {img_xfer_src, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01659"},
            {img_sampled,  VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01659"},
            {img_input,    VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01659"},
        };
        if (video_decode_queue) {
            // images _without_ VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07120"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07120"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07120"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07120"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07120"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07120"});
            // // images _without_ VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07121"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07121"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07121"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07121"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07121"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07121"});
            // // images _without_ VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07122"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07122"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07122"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07122"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07122"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07122"});
        }
        if (video_encode_queue) {
            // images _without_ VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07123"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07123"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07123"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07123"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07123"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07123"});
            // images _without_ VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07124"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07124"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07124"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07124"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07124"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07124"});
            // images _without_ VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07125"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07125"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07125"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07125"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07125"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR,             "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07125"});
        }
        if (video_encode_quantization_map) {
            // images _without_ VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR,"VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-10287"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR,"VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-10287"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR,"VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-10287"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR,"VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-10287"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR,"VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-10287"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR,"VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-10287"});
        }
        // clang-format on

        for (const auto &test : bad_buffer_layouts) {
            const VkImageLayout bad_layout = test.bad_layout;
            // Skip layouts that require maintenance2 support
            if ((maintenance2 == false) && ((bad_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) ||
                                            (bad_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL))) {
                continue;
            }
            conc_test.image_barrier_.image = test.image_obj.handle();
            const VkImageUsageFlags usage = test.image_obj.Usage();
            conc_test.image_barrier_.subresourceRange.aspectMask = (usage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                                                                       ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)
                                                                       : VK_IMAGE_ASPECT_COLOR_BIT;

            conc_test.image_barrier_.oldLayout = bad_layout;
            conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            conc_test(test.msg_code);

            conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            conc_test.image_barrier_.newLayout = bad_layout;
            conc_test(test.msg_code);
        }

        if (feedback_loop_layout) {
            conc_test.image_barrier_.image = img_color.handle();
            conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;
            conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            conc_test("VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07006");
        }

        conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        conc_test.image_barrier_.image = image.handle();
    }

    // Attempt barrier where srcAccessMask is not supported by srcStageMask
    // Have bit that's supported (transfer write), and another that isn't to verify multi-bit validation
    conc_test.buffer_barrier_.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
    conc_test.buffer_barrier_.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    conc_test.buffer_barrier_.offset = 0;
    conc_test.buffer_barrier_.size = VK_WHOLE_SIZE;
    conc_test("", "VUID-vkCmdPipelineBarrier-pBufferMemoryBarriers-02817");

    // Attempt barrier where dstAccessMask is not supported by dstStageMask
    conc_test.buffer_barrier_.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    conc_test.buffer_barrier_.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    conc_test("", "VUID-vkCmdPipelineBarrier-pBufferMemoryBarriers-02818");

    // Attempt to mismatch barriers/waitEvents calls with incompatible queues
    // Create command pool with incompatible queueflags
    const std::vector<VkQueueFamilyProperties> queue_props = m_device->Physical().queue_properties_;
    const std::optional<uint32_t> queue_family_index = m_device->ComputeOnlyQueueFamily();
    if (!queue_family_index) {
        GTEST_SKIP() << "No compute-only queue found; skipped";
    }

    VkBufferMemoryBarrier buf_barrier = vku::InitStructHelper();
    buf_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    buf_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    buf_barrier.buffer = buffer.handle();
    buf_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buf_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buf_barrier.offset = 0;
    buf_barrier.size = VK_WHOLE_SIZE;

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-srcStageMask-06461");

    vkt::CommandPool command_pool(*m_device, queue_family_index.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer bad_command_buffer(*m_device, command_pool);

    bad_command_buffer.Begin();
    // Set two bits that should both be supported as a bonus positive check
    buf_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    buf_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT;
    vk::CmdPipelineBarrier(bad_command_buffer.handle(), VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &buf_barrier, 0, nullptr);
    m_errorMonitor->VerifyFound();

    // Check for error for trying to wait on pipeline stage not supported by this queue. Specifically since our queue is not a
    // compute queue, vk::CmdWaitEvents cannot have it's source stage mask be VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-06459");
    vkt::Event event(*m_device);
    vk::CmdWaitEvents(bad_command_buffer.handle(), 1, &event.handle(), /*source stage mask*/ VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                      VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();
    bad_command_buffer.End();
}

TEST_F(NegativeSyncObject, Sync2Barriers) {
    TEST_DESCRIPTION("Synchronization2 test for invalid Memory Barriers");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddOptionalExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework(&kDisableMessageLimit));
    RETURN_IF_SKIP(InitState());
    const bool maintenance2 = IsExtensionsEnabled(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    const bool feedback_loop_layout = IsExtensionsEnabled(VK_EXT_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_EXTENSION_NAME);
    const bool video_decode_queue = IsExtensionsEnabled(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME);
    const bool video_encode_queue = IsExtensionsEnabled(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME);
    const bool video_encode_quantization_map = IsExtensionsEnabled(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);

    auto depth_format = FindSupportedDepthStencilFormat(Gpu());

    vkt::Image color_image(*m_device, m_width, m_height, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    auto color_view = color_image.CreateView();

    // Just using all framebuffer-space pipeline stages in order to get a reasonably large
    //  set of bits that can be used for both src & dst
    VkPipelineStageFlags stage_flags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                       VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // Add all of the gfx mem access bits that correlate to the fb-space pipeline stages
    VkAccessFlags access_flags = VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
                                 VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                 VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                 VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    // Add a token self-dependency for this test to avoid unexpected errors
    rp.AddSubpassDependency(stage_flags, stage_flags, access_flags, access_flags);
    rp.CreateRenderPass();
    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &color_view.handle());

    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t invalid = static_cast<uint32_t>(m_device->Physical().queue_properties_.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (invalid == 1) || (m_device->Physical().queue_properties_[other_family].queueCount == 0);
    std::vector<uint32_t> qf_indices{{submit_family, other_family}};
    if (only_one_family) {
        qf_indices.resize(1);
    }
    Barrier2QueueFamilyTestHelper::Context test_context(this, qf_indices);

    // Use image unbound to memory in barrier
    // Use buffer unbound to memory in barrier
    Barrier2QueueFamilyTestHelper conc_test(&test_context);
    conc_test.Init(false, false);

    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    conc_test(" used with no memory bound. Memory should be bound by calling vkBindImageMemory()",
              " used with no memory bound. Memory should be bound by calling vkBindBufferMemory()");

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    conc_test.buffer_barrier_.buffer = buffer.handle();

    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    conc_test.image_barrier_.image = image.handle();

    // New layout can't be PREINITIALIZED
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    conc_test("VUID-VkImageMemoryBarrier2-newLayout-01198", "");

    // Transition image to color attachment optimal
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    conc_test("");

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle());

    // Can't send buffer memory barrier during a render pass
    m_command_buffer.EndRenderPass();

    // Duplicate barriers that change layout
    VkImageMemoryBarrier2 img_barrier = vku::InitStructHelper();
    img_barrier.image = image.handle();
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    img_barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    img_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    VkImageMemoryBarrier2 img_barriers[2] = {img_barrier, img_barrier};

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 2;
    dep_info.pImageMemoryBarriers = img_barriers;
    dep_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    // Transitions from UNDEFINED  are valid, even if duplicated
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dep_info);

    // Duplication of layout transitions (not from undefined) are not valid
    img_barriers[0].oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barriers[0].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barriers[1].oldLayout = img_barriers[0].oldLayout;
    img_barriers[1].newLayout = img_barriers[0].newLayout;
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier2-oldLayout-01197");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dep_info);
    m_errorMonitor->VerifyFound();

    {
        // Transitions to and from EXTERNAL within the same command buffer are valid, if pointless.
        dep_info.imageMemoryBarrierCount = 1;
        dep_info.pImageMemoryBarriers = &img_barrier;
        img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        img_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        img_barrier.srcQueueFamilyIndex = submit_family;
        img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
        img_barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        img_barrier.dstAccessMask = 0;
        img_barrier.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        img_barrier.dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dep_info);

        img_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
        img_barrier.dstQueueFamilyIndex = submit_family;
        img_barrier.srcAccessMask = 0;
        img_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;

        vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dep_info);
    }

    // Exceed the buffer size
    conc_test.buffer_barrier_.offset = conc_test.buffer_.CreateInfo().size + 1;
    conc_test("", "VUID-VkBufferMemoryBarrier2-offset-01187");

    conc_test.buffer_barrier_.offset = 0;
    conc_test.buffer_barrier_.size = conc_test.buffer_.CreateInfo().size + 1;
    // Size greater than total size
    conc_test("", "VUID-VkBufferMemoryBarrier2-size-01189");

    conc_test.buffer_barrier_.size = 0;
    // Size is zero
    conc_test("", "VUID-VkBufferMemoryBarrier2-size-01188");

    conc_test.buffer_barrier_.size = VK_WHOLE_SIZE;

    // Now exercise barrier aspect bit errors, first DS
    vkt::Image ds_image(*m_device, 128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    conc_test.image_barrier_.image = ds_image.handle();

    // Not having DEPTH or STENCIL set is an error
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier2-subresourceRange-09601");
    {
        conc_test("VUID-VkImageMemoryBarrier2-image-03320");

        // Having only one of depth or stencil set for DS image is an error
        conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        conc_test("VUID-VkImageMemoryBarrier2-image-03320");
    }

    // Having anything other than DEPTH and STENCIL is an error
    conc_test.image_barrier_.subresourceRange.aspectMask =
        VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_COLOR_BIT;
    conc_test("VUID-VkImageMemoryBarrier2-subresourceRange-09601");

    // Now test depth-only
    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), VK_FORMAT_D16_UNORM, &format_props);
    if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        vkt::Image d_image(*m_device, 128, 128, 1, VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

        conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        conc_test.image_barrier_.image = d_image.handle();

        // DEPTH bit must be set
        conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
        conc_test("depth-only image formats must have the VK_IMAGE_ASPECT_DEPTH_BIT set.");

        // No bits other than DEPTH may be set
        conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_COLOR_BIT;
        conc_test("depth-only image formats can have only the VK_IMAGE_ASPECT_DEPTH_BIT set.");
    }

    // Now test stencil-only
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), VK_FORMAT_S8_UINT, &format_props);
    if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        vkt::Image s_image(*m_device, 128, 128, 1, VK_FORMAT_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

        conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        conc_test.image_barrier_.image = s_image.handle();

        // Use of COLOR aspect on depth image is error
        conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        conc_test("stencil-only image formats must have the VK_IMAGE_ASPECT_STENCIL_BIT set.");
    }

    // Finally test color
    vkt::Image c_image(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    conc_test.image_barrier_.image = c_image.handle();

    // COLOR bit must be set
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier2-subresourceRange-09601");
    conc_test("VUID-VkImageMemoryBarrier2-image-09241");

    // No bits other than COLOR may be set
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier2-subresourceRange-09601");
    conc_test("VUID-VkImageMemoryBarrier2-image-09241");

    // A barrier's new and old VkImageLayout must be compatible with an image's VkImageUsageFlags.
    {
        vkt::Image img_color(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        vkt::Image img_ds(*m_device, 128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        vkt::Image img_xfer_src(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        vkt::Image img_xfer_dst(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
        vkt::Image img_sampled(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
        vkt::Image img_input(*m_device, 128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
        struct BadBufferTest {
            vkt::Image &image_obj;
            VkImageLayout bad_layout;
            std::string msg_code;
        };
        // clang-format off
        std::vector<BadBufferTest> bad_buffer_layouts = {
            // images _without_ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
            {img_ds,       VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01208"},
            {img_xfer_src, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01208"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01208"},
            {img_sampled,  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01208"},
            {img_input,    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01208"},
            // images _without_ VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
            {img_color,    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01209"},
            {img_xfer_src, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01209"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01209"},
            {img_sampled,  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01209"},
            {img_input,    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01209"},
            {img_color,    VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier2-oldLayout-01210"},
            {img_xfer_src, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier2-oldLayout-01210"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier2-oldLayout-01210"},
            {img_sampled,  VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier2-oldLayout-01210"},
            {img_input,    VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier2-oldLayout-01210"},
            // images _without_ VK_IMAGE_USAGE_SAMPLED_BIT or VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
            {img_color,    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01211"},
            {img_ds,       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01211"},
            {img_xfer_src, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01211"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01211"},
            // images _without_ VK_IMAGE_USAGE_TRANSFER_SRC_BIT
            {img_color,    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01212"},
            {img_ds,       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01212"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01212"},
            {img_sampled,  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01212"},
            {img_input,    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01212"},
            // images _without_ VK_IMAGE_USAGE_TRANSFER_DST_BIT
            {img_color,    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01213"},
            {img_ds,       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01213"},
            {img_xfer_src, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01213"},
            {img_sampled,  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01213"},
            {img_input,    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01213"},
            // images _without_ VK_KHR_maintenance2 added layouts
            {img_color,    VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01658"},
            {img_xfer_src, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01658"},
            {img_sampled,  VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01658"},
            {img_input,    VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01658"},
            {img_color,    VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01659"},
            {img_xfer_src, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01659"},
            {img_sampled,  VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01659"},
            {img_input,    VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01659"},
        };
        if (video_decode_queue) {
            // images _without_ VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07120"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07120"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07120"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07120"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07120"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07120"});
            // images _without_ VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07121"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07121"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07121"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07121"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07121"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07121"});
            // images _without_ VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07122"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07122"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07122"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07122"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07122"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07122"});
        }
        if (video_encode_queue) {
            // images _without_ VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07123"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07123"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07123"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07123"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07123"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07123"});
            // images _without_ VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07124"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07124"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07124"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07124"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07124"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07124"});
            // images _without_ VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07125"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07125"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07125"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07125"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07125"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07125"});
        }
        if (video_encode_quantization_map) {
            // images _without_ VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR
            bad_buffer_layouts.push_back({img_color,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-10287"});
            bad_buffer_layouts.push_back({img_ds,       VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-10287"});
            bad_buffer_layouts.push_back({img_xfer_src, VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-10287"});
            bad_buffer_layouts.push_back({img_xfer_dst, VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-10287"});
            bad_buffer_layouts.push_back({img_sampled,  VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-10287"});
            bad_buffer_layouts.push_back({img_input,    VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR, "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-10287"});
        }
        // clang-format on

        for (const auto &test : bad_buffer_layouts) {
            const VkImageLayout bad_layout = test.bad_layout;
            // Skip layouts that require maintenance2 support
            if ((maintenance2 == false) && ((bad_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) ||
                                            (bad_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL))) {
                continue;
            }
            conc_test.image_barrier_.image = test.image_obj.handle();
            const VkImageUsageFlags usage = test.image_obj.Usage();
            conc_test.image_barrier_.subresourceRange.aspectMask = (usage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                                                                       ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)
                                                                       : VK_IMAGE_ASPECT_COLOR_BIT;

            conc_test.image_barrier_.oldLayout = bad_layout;
            conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            conc_test(test.msg_code);

            conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            conc_test.image_barrier_.newLayout = bad_layout;
            conc_test(test.msg_code);
        }

        if (feedback_loop_layout) {
            conc_test.image_barrier_.image = img_color.handle();
            conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;
            conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            conc_test("VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07006");
        }

        conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        conc_test.image_barrier_.image = image.handle();
    }

    // Attempt barrier where srcAccessMask is not supported by srcStageMask
    // Have lower-order bit that's supported (shader write), but higher-order bit not supported to verify multi-bit validation
    // TODO: synchronization2 has a separate VUID for every access flag. Gotta test them all..
    conc_test.buffer_barrier_.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    conc_test.buffer_barrier_.offset = 0;
    conc_test.buffer_barrier_.size = VK_WHOLE_SIZE;
    conc_test("", "VUID-VkBufferMemoryBarrier2-srcAccessMask-03909");

    // Attempt barrier where dstAccessMask is not supported by dstStageMask
    conc_test.buffer_barrier_.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    conc_test.buffer_barrier_.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    conc_test("", "VUID-VkBufferMemoryBarrier2-dstAccessMask-03911");
}

TEST_F(NegativeSyncObject, DepthStencilImageNonSeparate) {
    TEST_DESCRIPTION("test barrier with depth/stencil image, with wrong aspect mask with not separateDepthStencilLayouts");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());

    InitRenderTarget();

    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t invalid = static_cast<uint32_t>(m_device->Physical().queue_properties_.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (invalid == 1) || (m_device->Physical().queue_properties_[other_family].queueCount == 0);
    std::vector<uint32_t> qf_indices{{submit_family, other_family}};
    if (only_one_family) {
        qf_indices.resize(1);
    }
    BarrierQueueFamilyTestHelper::Context test_context(this, qf_indices);
    BarrierQueueFamilyTestHelper conc_test(&test_context);
    conc_test.Init(nullptr, false, true);

    m_command_buffer.Begin();

    const VkFormat depth_format = FindSupportedDepthStencilFormat(Gpu());
    vkt::Image ds_image(*m_device, 128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    conc_test.image_barrier_.image = ds_image.handle();

    // Not having DEPTH or STENCIL set is an error
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-subresourceRange-09601");
    conc_test("VUID-VkImageMemoryBarrier-image-03320");

    // Having only one of depth or stencil set for DS image is an error
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    conc_test("VUID-VkImageMemoryBarrier-image-03320");
}

TEST_F(NegativeSyncObject, DepthStencilImageNonSeparateSync2) {
    TEST_DESCRIPTION("test barrier with depth/stencil image, with wrong aspect mask with not separateDepthStencilLayouts");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t invalid = static_cast<uint32_t>(m_device->Physical().queue_properties_.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (invalid == 1) || (m_device->Physical().queue_properties_[other_family].queueCount == 0);
    std::vector<uint32_t> qf_indices{{submit_family, other_family}};
    if (only_one_family) {
        qf_indices.resize(1);
    }
    Barrier2QueueFamilyTestHelper::Context test_context(this, qf_indices);
    Barrier2QueueFamilyTestHelper conc_test(&test_context);
    conc_test.Init(false, true);

    m_command_buffer.Begin();

    const VkFormat depth_format = FindSupportedDepthStencilFormat(Gpu());
    vkt::Image ds_image(*m_device, 128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    conc_test.image_barrier_.image = ds_image.handle();

    // Not having DEPTH or STENCIL set is an error
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier2-subresourceRange-09601");
    conc_test("VUID-VkImageMemoryBarrier2-image-03320");

    // Having only one of depth or stencil set for DS image is an error
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    conc_test("VUID-VkImageMemoryBarrier2-image-03320");
}

TEST_F(NegativeSyncObject, BarrierQueueFamily) {
    TEST_DESCRIPTION("Create and submit barriers with invalid queue families");
    SetTargetApiVersion(VK_API_VERSION_1_0);
    RETURN_IF_SKIP(Init());
    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        GTEST_SKIP()
            << "Device has apiVersion greater than 1.0 -- skipping test cases that require external memory to be disabled.";
    }

    // Find queues of two families
    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t queue_family_count = static_cast<uint32_t>(m_device->Physical().queue_properties_.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (queue_family_count == 1) ||
                                 (m_device->Physical().queue_properties_[other_family].queueCount == 0) ||
                                 ((m_device->Physical().queue_properties_[other_family].queueFlags & VK_QUEUE_TRANSFER_BIT) == 0);

    std::vector<uint32_t> qf_indices{{submit_family, other_family}};
    if (only_one_family) {
        qf_indices.resize(1);
    }

    BarrierQueueFamilyTestHelper::Context test_context(this, qf_indices);

    BarrierQueueFamilyTestHelper excl_test(&test_context);
    excl_test.Init(nullptr);  // no queue families means *exclusive* sharing mode.

    excl_test("VUID-VkImageMemoryBarrier-image-09117", "VUID-VkBufferMemoryBarrier-buffer-09095", VK_QUEUE_FAMILY_IGNORED,
              submit_family);
    excl_test("VUID-VkImageMemoryBarrier-image-09118", "VUID-VkBufferMemoryBarrier-buffer-09096", submit_family,
              VK_QUEUE_FAMILY_IGNORED);
    excl_test(submit_family, submit_family);
    excl_test(VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);
}

TEST_F(NegativeSyncObject, BarrierQueueFamilyOneFamily) {
    TEST_DESCRIPTION("Create and submit barriers with invalid queue families");
    SetTargetApiVersion(VK_API_VERSION_1_0);
    RETURN_IF_SKIP(Init());
    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        GTEST_SKIP()
            << "Device has apiVersion greater than 1.0 -- skipping test cases that require external memory to be disabled.";
    }

    // Find queues of two families
    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t queue_family_count = static_cast<uint32_t>(m_device->Physical().queue_properties_.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (queue_family_count == 1) ||
                                 (m_device->Physical().queue_properties_[other_family].queueCount == 0) ||
                                 ((m_device->Physical().queue_properties_[other_family].queueFlags & VK_QUEUE_TRANSFER_BIT) == 0);
    if (only_one_family) {
        GTEST_SKIP() << "Single queue family found";
    }
    std::vector<uint32_t> qf_indices{{submit_family, other_family}};
    BarrierQueueFamilyTestHelper::Context test_context(this, qf_indices);

    std::vector<uint32_t> families = {submit_family, other_family};
    BarrierQueueFamilyTestHelper conc_test(&test_context);
    conc_test.Init(&families);
    {
        // src
        static const char *img_vuid = "VUID-VkImageMemoryBarrier-None-09053";
        static const char *buf_vuid = "VUID-VkBufferMemoryBarrier-None-09050";
        conc_test(img_vuid, buf_vuid, submit_family, VK_QUEUE_FAMILY_IGNORED);
    }
    {
        // dst
        static const char *img_vuid = "VUID-VkImageMemoryBarrier-None-09054";
        static const char *buf_vuid = "VUID-VkBufferMemoryBarrier-None-09051";
        conc_test(img_vuid, buf_vuid, VK_QUEUE_FAMILY_IGNORED, submit_family);
    }
    {
        // neither
        static const char *img_vuid = "VUID-VkImageMemoryBarrier-None-09053";
        static const char *buf_vuid = "VUID-VkBufferMemoryBarrier-None-09050";
        conc_test(img_vuid, buf_vuid, submit_family, submit_family);
    }
    conc_test(VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);
}

TEST_F(NegativeSyncObject, BarrierQueueFamily2) {
    TEST_DESCRIPTION("Create and submit barriers with invalid queue families");
    SetTargetApiVersion(VK_API_VERSION_1_0);
    RETURN_IF_SKIP(Init());

    // Find queues of two families
    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t queue_family_count = static_cast<uint32_t>(m_device->Physical().queue_properties_.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (queue_family_count == 1) ||
                                 (m_device->Physical().queue_properties_[other_family].queueCount == 0) ||
                                 ((m_device->Physical().queue_properties_[other_family].queueFlags & VK_QUEUE_TRANSFER_BIT) == 0);

    if (only_one_family) {
        GTEST_SKIP() << "Single queue family found";
    }
    std::vector<uint32_t> qf_indices{{submit_family, other_family}};
    BarrierQueueFamilyTestHelper::Context test_context(this, qf_indices);

    BarrierQueueFamilyTestHelper excl_test(&test_context);
    excl_test.Init(nullptr);

    // Although other_family does not match submit_family, because the barrier families are
    // equal here, no ownership transfer actually happens, and this barrier is valid by the spec.
    excl_test(other_family, other_family, submit_family);

    // positive test (testing both the index logic and the QFO transfer tracking.
    excl_test(submit_family, other_family, submit_family);
    excl_test(submit_family, other_family, other_family);
    excl_test(other_family, submit_family, other_family);
    excl_test(other_family, submit_family, submit_family);

    // negative testing for QFO transfer tracking
    // Duplicate release in one CB
    excl_test("WARNING-VkImageMemoryBarrier-image-00001", "WARNING-VkBufferMemoryBarrier-buffer-00001", submit_family, other_family,
              submit_family, BarrierQueueFamilyTestHelper::DOUBLE_RECORD);
    // Duplicate pending release
    excl_test("WARNING-VkImageMemoryBarrier-image-00003", "WARNING-VkBufferMemoryBarrier-buffer-00003", submit_family, other_family,
              submit_family);
    // Duplicate acquire in one CB
    excl_test("WARNING-VkImageMemoryBarrier-image-00001", "WARNING-VkBufferMemoryBarrier-buffer-00001", submit_family, other_family,
              other_family, BarrierQueueFamilyTestHelper::DOUBLE_RECORD);
    // No pending release
    excl_test("VUID-vkQueueSubmit-pSubmits-02207", "VUID-vkQueueSubmit-pSubmits-02207", submit_family, other_family, other_family);
    // Duplicate release in two CB
    excl_test("WARNING-VkImageMemoryBarrier-image-00002", "WARNING-VkBufferMemoryBarrier-buffer-00002", submit_family, other_family,
              submit_family, BarrierQueueFamilyTestHelper::DOUBLE_COMMAND_BUFFER);
    // Duplicate acquire in two CB
    excl_test(submit_family, other_family, submit_family);  // need a succesful release
    excl_test("WARNING-VkImageMemoryBarrier-image-00002", "WARNING-VkBufferMemoryBarrier-buffer-00002", submit_family, other_family,
              other_family, BarrierQueueFamilyTestHelper::DOUBLE_COMMAND_BUFFER);
}

TEST_F(NegativeSyncObject, ImageOwnershipTransferQueueMismatch) {
    TEST_DESCRIPTION("Neither src nor dst barrier queue family matches submit queue family");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    std::optional<uint32_t> transfer_only_family = m_device->TransferOnlyQueueFamily();
    std::optional<uint32_t> compute_only_family = m_device->ComputeOnlyQueueFamily();
    if (!transfer_only_family.has_value() || !compute_only_family.has_value()) {
        GTEST_SKIP() << "Transfer-only and compute-only queue family is required";
    }

    vkt::CommandPool release_pool(*m_device, transfer_only_family.value());
    vkt::CommandBuffer release_cb(*m_device, release_pool);
    vkt::CommandBuffer acquire_cb(*m_device, m_command_pool);

    const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, usage);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    // Release image
    VkImageMemoryBarrier2 release_barrier = vku::InitStructHelper();
    release_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    release_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    release_barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
    release_barrier.dstAccessMask = VK_ACCESS_2_NONE;
    release_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    release_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    release_barrier.srcQueueFamilyIndex = compute_only_family.value();  // specify compute family instead of expected transfer
    release_barrier.dstQueueFamilyIndex = m_default_queue->family_index;
    release_barrier.image = image;
    release_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkDependencyInfo release_dep_info = vku::InitStructHelper();
    release_dep_info.imageMemoryBarrierCount = 1;
    release_dep_info.pImageMemoryBarriers = &release_barrier;
    release_cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier2-srcQueueFamilyIndex-10387");
    vk::CmdPipelineBarrier2(release_cb, &release_dep_info);
    m_errorMonitor->VerifyFound();
    release_cb.End();

    // Acquire image
    VkImageMemoryBarrier2 acquire_barrier = vku::InitStructHelper();
    acquire_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    acquire_barrier.srcAccessMask = VK_ACCESS_2_NONE;
    acquire_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    acquire_barrier.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    acquire_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    acquire_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    acquire_barrier.srcQueueFamilyIndex = transfer_only_family.value();
    acquire_barrier.dstQueueFamilyIndex = compute_only_family.value();  // specify compute family instead of expected graphics
    acquire_barrier.image = image;
    acquire_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkDependencyInfo acquire_dep_info = vku::InitStructHelper();
    acquire_dep_info.imageMemoryBarrierCount = 1;
    acquire_dep_info.pImageMemoryBarriers = &acquire_barrier;
    acquire_cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier2-srcQueueFamilyIndex-10387");
    vk::CmdPipelineBarrier2(acquire_cb, &acquire_dep_info);
    m_errorMonitor->VerifyFound();
    acquire_cb.End();
}

TEST_F(NegativeSyncObject, BufferOwnershipTransferQueueMismatch) {
    TEST_DESCRIPTION("Neither src nor dst barrier queue family matches submit queue family");
    RETURN_IF_SKIP(Init());

    std::optional<uint32_t> transfer_only_family = m_device->TransferOnlyQueueFamily();
    std::optional<uint32_t> compute_only_family = m_device->ComputeOnlyQueueFamily();
    if (!transfer_only_family.has_value() || !compute_only_family.has_value()) {
        GTEST_SKIP() << "Transfer-only and compute-only queue family is required";
    }

    vkt::CommandPool release_pool(*m_device, transfer_only_family.value());
    vkt::CommandBuffer release_cb(*m_device, release_pool);
    vkt::CommandBuffer acquire_cb(*m_device, m_command_pool);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    // Release buffer
    VkBufferMemoryBarrier release_barrier = vku::InitStructHelper();
    release_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    release_barrier.dstAccessMask = VK_ACCESS_2_NONE;
    release_barrier.srcQueueFamilyIndex = compute_only_family.value();  // specify compute family instead of expected transfer
    release_barrier.dstQueueFamilyIndex = m_default_queue->family_index;
    release_barrier.buffer = buffer;
    release_barrier.size = 256;
    release_cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-srcQueueFamilyIndex-10388");
    vk::CmdPipelineBarrier(release_cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 1,
                           &release_barrier, 0, nullptr);
    m_errorMonitor->VerifyFound();
    release_cb.End();

    // Acquire image
    VkBufferMemoryBarrier acquire_barrier = vku::InitStructHelper();
    acquire_barrier.srcAccessMask = 0;
    acquire_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    acquire_barrier.srcQueueFamilyIndex = transfer_only_family.value();
    acquire_barrier.dstQueueFamilyIndex = compute_only_family.value();  // specify compute family instead of expected graphics
    acquire_barrier.buffer = buffer;
    acquire_barrier.size = 256;
    acquire_cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-srcQueueFamilyIndex-10388");
    vk::CmdPipelineBarrier(acquire_cb, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 1,
                           &acquire_barrier, 0, nullptr);
    m_errorMonitor->VerifyFound();
    acquire_cb.End();
}

TEST_F(NegativeSyncObject, ConcurrentBufferBarrierNeedsIgnoredQueue) {
    TEST_DESCRIPTION("Barrier for concurrent buffer resource must have VK_QUEUE_FAMILY_IGNORED at least for src or dst");
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    std::optional<uint32_t> transfer_only_family = m_device->TransferOnlyQueueFamily();
    if (!transfer_only_family.has_value()) {
        GTEST_SKIP() << "Transfer-only queue family is required";
    }

    // Create VK_SHARING_MODE_CONCURRENT buffer by specifying queue families
    uint32_t queue_families[2] = {m_default_queue->family_index, transfer_only_family.value()};
    vkt::Buffer buffer;
    buffer.init(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0, nullptr, vvl::make_span(queue_families, 2));

    VkBufferMemoryBarrier barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    // Specify EXTERNAL for both queue families to trigger VU
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
    barrier.buffer = buffer.handle();
    barrier.offset = 0;
    barrier.size = 256;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkBufferMemoryBarrier-None-09049");
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
                           1, &barrier, 0, nullptr);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, ConcurrentImageBarrierNeedsIgnoredQueue) {
    TEST_DESCRIPTION("Barrier for concurrent image resource must have VK_QUEUE_FAMILY_IGNORED at least for src or dst");
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    std::optional<uint32_t> transfer_only_family = m_device->TransferOnlyQueueFamily();
    if (!transfer_only_family.has_value()) {
        GTEST_SKIP() << "Transfer-only queue family is required";
    }

    // Create VK_SHARING_MODE_CONCURRENT image by specifying queue families
    uint32_t queue_families[2] = {m_default_queue->family_index, transfer_only_family.value()};
    const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkImageCreateInfo image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, usage,
                                                               VK_IMAGE_TILING_OPTIMAL, vvl::make_span(queue_families, 2));
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    // Release image
    VkImageMemoryBarrier barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    // Specify EXTERNAL for both queue families to trigger VU
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
    barrier.image = image;
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-None-09052");
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &barrier);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, BufferBarrierWithHostStage) {
    TEST_DESCRIPTION("Buffer barrier includes VK_PIPELINE_STAGE_2_HOST_BIT as srcStageMask or dstStageMask");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    VkBufferMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.buffer = buffer.handle();
    barrier.size = VK_WHOLE_SIZE;
    // source and destination families should be equal if HOST stage is used
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = 0;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.bufferMemoryBarrierCount = 1;
    dependency_info.pBufferMemoryBarriers = &barrier;

    // HOST stage as source
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_HOST_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkBufferMemoryBarrier2-srcStageMask-03851");
    vk::CmdPipelineBarrier2(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // HOST stage as destination
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_HOST_READ_BIT;
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkBufferMemoryBarrier2-srcStageMask-03851");
    vk::CmdPipelineBarrier2(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, ImageBarrierWithHostStage) {
    TEST_DESCRIPTION("Image barrier includes VK_PIPELINE_STAGE_2_HOST_BIT as srcStageMask or dstStageMask");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    VkImageMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.image = image.handle();
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    // source and destination families should be equal if HOST stage is used
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = 0;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = &barrier;

    // HOST stage as source
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_HOST_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier2-srcStageMask-03854");
    vk::CmdPipelineBarrier2(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // HOST stage as destination
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_HOST_READ_BIT;
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier2-srcStageMask-03854");
    vk::CmdPipelineBarrier2(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, BufferBarrierWithHostStageSync1) {
    TEST_DESCRIPTION("Buffer barrier includes VK_PIPELINE_STAGE_HOST_BIT as srcStageMask or dstStageMask");
    RETURN_IF_SKIP(Init());

    if (m_device->Physical().queue_properties_.size() < 2) {
        GTEST_SKIP() << "Two queue families are required";
    }
    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkBufferMemoryBarrier barrier = vku::InitStructHelper();
    barrier.srcQueueFamilyIndex = 0;
    barrier.dstQueueFamilyIndex = 1;  // dstQueueFamilyIndex != srcQueueFamilyIndex
    barrier.buffer = buffer.handle();
    barrier.size = VK_WHOLE_SIZE;

    // HOST stage as source
    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-srcStageMask-09634");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                           1, &barrier, 0, nullptr);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // HOST stage as destination
    barrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_HOST_READ_BIT;
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-srcStageMask-09634");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, nullptr,
                           1, &barrier, 0, nullptr);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, ImageBarrierWithHostStageSync1) {
    TEST_DESCRIPTION("Image barrier includes VK_PIPELINE_STAGE_HOST_BIT as srcStageMask or dstStageMask");
    RETURN_IF_SKIP(Init());

    if (m_device->Physical().queue_properties_.size() < 2) {
        GTEST_SKIP() << "Two queue families are required";
    }
    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    VkImageMemoryBarrier barrier = vku::InitStructHelper();
    barrier.srcQueueFamilyIndex = 0;
    barrier.dstQueueFamilyIndex = 1;  // dstQueueFamilyIndex != srcQueueFamilyIndex
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.image = image.handle();
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    // HOST stage as source
    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-srcStageMask-09633");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &barrier);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();

    // HOST stage as destination
    barrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_HOST_READ_BIT;
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-srcStageMask-09633");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &barrier);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

// TODO - Figure out if test or VU are bad
TEST_F(NegativeSyncObject, BarrierQueueFamilyWithMemExt) {
    TEST_DESCRIPTION("Create and submit barriers with invalid queue families when memory extension is enabled ");
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Find queues of two families
    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t invalid = static_cast<uint32_t>(m_device->Physical().queue_properties_.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (invalid == 1) || (m_device->Physical().queue_properties_[other_family].queueCount == 0);

    std::vector<uint32_t> qf_indices{{submit_family, other_family}};
    if (only_one_family) {
        qf_indices.resize(1);
    }
    BarrierQueueFamilyTestHelper::Context test_context(this, qf_indices);

    BarrierQueueFamilyTestHelper excl_test(&test_context);
    excl_test.Init(nullptr);  // no queue families means *exclusive* sharing mode.

    excl_test("VUID-VkImageMemoryBarrier-image-09118", "VUID-VkBufferMemoryBarrier-buffer-09096", submit_family, invalid);
    excl_test("VUID-VkImageMemoryBarrier-image-09117", "VUID-VkBufferMemoryBarrier-buffer-09095", invalid, submit_family);
    excl_test(submit_family, submit_family);
    excl_test(submit_family, VK_QUEUE_FAMILY_EXTERNAL_KHR);
    excl_test(VK_QUEUE_FAMILY_EXTERNAL_KHR, submit_family);
}

// TODO - Figure out if test or VU are bad
TEST_F(NegativeSyncObject, BarrierQueueFamilyWithMemExt2) {
    TEST_DESCRIPTION("Create and submit barriers with invalid queue families when memory extension is enabled ");
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Find queues of two families
    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t invalid = static_cast<uint32_t>(m_device->Physical().queue_properties_.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (invalid == 1) || (m_device->Physical().queue_properties_[other_family].queueCount == 0);

    if (only_one_family) {
        GTEST_SKIP() << "Single queue family found";
    }
    std::vector<uint32_t> qf_indices{{submit_family, other_family}};
    BarrierQueueFamilyTestHelper::Context test_context(this, qf_indices);

    std::vector<uint32_t> families = {submit_family, other_family};
    BarrierQueueFamilyTestHelper conc_test(&test_context);

    conc_test.Init(&families);
    static const char *img_vuid = "VUID-VkImageMemoryBarrier-None-09053";
    static const char *buf_vuid = "VUID-VkBufferMemoryBarrier-None-09050";
    conc_test(img_vuid, buf_vuid, submit_family, submit_family);
    conc_test(VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);
    conc_test(VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_EXTERNAL_KHR);
    conc_test(VK_QUEUE_FAMILY_EXTERNAL_KHR, VK_QUEUE_FAMILY_IGNORED);

    conc_test("VUID-VkImageMemoryBarrier-None-09053", "VUID-VkBufferMemoryBarrier-None-09050", submit_family,
              VK_QUEUE_FAMILY_IGNORED);
    conc_test("VUID-VkImageMemoryBarrier-None-09054", "VUID-VkBufferMemoryBarrier-None-09051", VK_QUEUE_FAMILY_IGNORED,
              submit_family);
    // This is to flag the errors that would be considered only "unexpected" in the parallel case above
    conc_test(VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_EXTERNAL_KHR);
    conc_test(VK_QUEUE_FAMILY_EXTERNAL_KHR, VK_QUEUE_FAMILY_IGNORED);
}

TEST_F(NegativeSyncObject, ImageBarrierWithBadRange) {
    TEST_DESCRIPTION("VkImageMemoryBarrier with an invalid subresourceRange");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkImageMemoryBarrier img_barrier_template = vku::InitStructHelper();
    img_barrier_template.srcAccessMask = 0;
    img_barrier_template.dstAccessMask = 0;
    img_barrier_template.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier_template.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier_template.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier_template.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    // subresourceRange to be set later for the for the purposes of this test
    img_barrier_template.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier_template.subresourceRange.baseArrayLayer = 0;
    img_barrier_template.subresourceRange.baseMipLevel = 0;
    img_barrier_template.subresourceRange.layerCount = 0;
    img_barrier_template.subresourceRange.levelCount = 0;

    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t invalid = static_cast<uint32_t>(m_device->Physical().queue_properties_.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (invalid == 1) || (m_device->Physical().queue_properties_[other_family].queueCount == 0);
    std::vector<uint32_t> qf_indices{{submit_family, other_family}};
    if (only_one_family) {
        qf_indices.resize(1);
    }
    BarrierQueueFamilyTestHelper::Context test_context(this, qf_indices);

    // Use image unbound to memory in barrier
    // Use buffer unbound to memory in barrier
    BarrierQueueFamilyTestHelper conc_test(&test_context);
    conc_test.Init(nullptr);
    img_barrier_template.image = conc_test.image_.handle();
    conc_test.image_barrier_ = img_barrier_template;
    // Nested scope here confuses clang-format, somehow
    // clang-format off

    // try for vk::CmdPipelineBarrier
    {
        // Try baseMipLevel >= image.mipLevels with VK_REMAINING_MIP_LEVELS
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_REMAINING_MIP_LEVELS, 0, 1};
            conc_test("VUID-VkImageMemoryBarrier-subresourceRange-01486");
        }

        // Try baseMipLevel >= image.mipLevels without VK_REMAINING_MIP_LEVELS
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0, 1};
            m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-subresourceRange-01724");
            conc_test("VUID-VkImageMemoryBarrier-subresourceRange-01486");
        }

        // Try levelCount = 0
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 0, 1};
            conc_test("VUID-VkImageSubresourceRange-levelCount-01720");
        }

        // Try baseMipLevel + levelCount > image.mipLevels
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 2, 0, 1};
            conc_test("VUID-VkImageMemoryBarrier-subresourceRange-01724");
        }

        // Try baseArrayLayer >= image.arrayLayers with VK_REMAINING_ARRAY_LAYERS
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, VK_REMAINING_ARRAY_LAYERS};
            conc_test("VUID-VkImageMemoryBarrier-subresourceRange-01488");
        }

        // Try baseArrayLayer >= image.arrayLayers without VK_REMAINING_ARRAY_LAYERS
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, 1};
            m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-subresourceRange-01725");
            conc_test("VUID-VkImageMemoryBarrier-subresourceRange-01488");
        }

        // Try layerCount = 0
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 0};
            conc_test("VUID-VkImageSubresourceRange-layerCount-01721");
        }

        // Try baseArrayLayer + layerCount > image.arrayLayers
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2};
            conc_test("VUID-VkImageMemoryBarrier-subresourceRange-01725");
        }
    }

    m_command_buffer.Begin();
    // try for vk::CmdWaitEvents
    {
        vkt::Event event(*m_device);

        // Try baseMipLevel >= image.mipLevels with VK_REMAINING_MIP_LEVELS
        {
            m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-subresourceRange-01486");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_REMAINING_MIP_LEVELS, 0, 1};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vk::CmdWaitEvents(m_command_buffer.handle(), 1, &event.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try baseMipLevel >= image.mipLevels without VK_REMAINING_MIP_LEVELS
        {
            m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-subresourceRange-01486");
            m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-subresourceRange-01724");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0, 1};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vk::CmdWaitEvents(m_command_buffer.handle(), 1, &event.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try levelCount = 0
        {
            m_errorMonitor->SetDesiredError("VUID-VkImageSubresourceRange-levelCount-01720");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 0, 1};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vk::CmdWaitEvents(m_command_buffer.handle(), 1, &event.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try baseMipLevel + levelCount > image.mipLevels
        {
            m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-subresourceRange-01724");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 2, 0, 1};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vk::CmdWaitEvents(m_command_buffer.handle(), 1, &event.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try baseArrayLayer >= image.arrayLayers with VK_REMAINING_ARRAY_LAYERS
        {
            m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-subresourceRange-01488");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, VK_REMAINING_ARRAY_LAYERS};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vk::CmdWaitEvents(m_command_buffer.handle(), 1, &event.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try baseArrayLayer >= image.arrayLayers without VK_REMAINING_ARRAY_LAYERS
        {
            m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-subresourceRange-01488");
            m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-subresourceRange-01725");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, 1};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vk::CmdWaitEvents(m_command_buffer.handle(), 1, &event.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try layerCount = 0
        {
            m_errorMonitor->SetDesiredError("VUID-VkImageSubresourceRange-layerCount-01721");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 0};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vk::CmdWaitEvents(m_command_buffer.handle(), 1, &event.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try baseArrayLayer + layerCount > image.arrayLayers
        {
            m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier-subresourceRange-01725");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vk::CmdWaitEvents(m_command_buffer.handle(), 1, &event.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }
    }
    // clang-format on
}

TEST_F(NegativeSyncObject, Sync2BarrierQueueFamily) {
    TEST_DESCRIPTION("Create and submit barriers with invalid queue families with synchronization2");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    // Find queues of two families
    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t invalid = static_cast<uint32_t>(m_device->Physical().queue_properties_.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (invalid == 1) || (m_device->Physical().queue_properties_[other_family].queueCount == 0);

    std::vector<uint32_t> qf_indices{{submit_family, other_family}};
    if (only_one_family) {
        qf_indices.resize(1);
    }
    BarrierQueueFamilyTestHelper::Context test_context(this, qf_indices);
    Barrier2QueueFamilyTestHelper::Context test_context2(this, qf_indices);

    Barrier2QueueFamilyTestHelper excl_test(&test_context2);
    excl_test.Init();  // *exclusive* sharing mode.
    excl_test("VUID-VkImageMemoryBarrier2-image-09118", "VUID-VkBufferMemoryBarrier2-buffer-09096", submit_family, invalid);
    excl_test("VUID-VkImageMemoryBarrier2-image-09117", "VUID-VkBufferMemoryBarrier2-buffer-09095", invalid, submit_family);
    excl_test(submit_family, submit_family);
    excl_test(submit_family, VK_QUEUE_FAMILY_EXTERNAL_KHR);
    excl_test(VK_QUEUE_FAMILY_EXTERNAL_KHR, submit_family);
}

TEST_F(NegativeSyncObject, BufferBarrierQueuesExternalAndForeign) {
    TEST_DESCRIPTION("Test buffer barrier with one family EXTERNAL and another one FOREIGN_EXT");
    AddRequiredExtensions(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    VkBufferMemoryBarrier bmb = vku::InitStructHelper();
    bmb.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    bmb.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    bmb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
    bmb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_FOREIGN_EXT;
    bmb.buffer = buffer.handle();
    bmb.offset = 0;
    bmb.size = VK_WHOLE_SIZE;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-srcQueueFamilyIndex-10388");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &bmb, 0, nullptr);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, BufferBarrierQueuesExternalAndForeign2) {
    TEST_DESCRIPTION("Test buffer barrier with one family EXTERNAL and another one FOREIGN_EXT");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    VkBufferMemoryBarrier2 bmb = vku::InitStructHelper();
    bmb.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    bmb.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    bmb.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    bmb.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    bmb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_FOREIGN_EXT;
    bmb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
    bmb.buffer = buffer.handle();
    bmb.offset = 0;
    bmb.size = VK_WHOLE_SIZE;

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.bufferMemoryBarrierCount = 1;
    dep_info.pBufferMemoryBarriers = &bmb;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier2-srcQueueFamilyIndex-10387");
    vk::CmdPipelineBarrier2(m_command_buffer, &dep_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, BarrierAccessSync2) {
    TEST_DESCRIPTION("Test barrier VkAccessFlagBits2.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    VkMemoryBarrier2 mem_barrier = vku::InitStructHelper();
    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &mem_barrier;

    m_command_buffer.Begin();

    // srcAccessMask and srcStageMask
    mem_barrier.srcAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-03900");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_INDEX_READ_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-03901");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-03902");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-03903");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_UNIFORM_READ_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-03904");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-03905");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-03906");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-03907");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-07454");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-03909");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-03910");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-03911");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-03912");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-03913");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-03914");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-03915");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_HOST_READ_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-03916");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_HOST_WRITE_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-03917");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    // now test dstAccessMask and dstStageMask
    mem_barrier.srcAccessMask = VK_ACCESS_2_HOST_WRITE_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;

    mem_barrier.dstAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-03900");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_INDEX_READ_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-03901");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-03902");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-03903");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_UNIFORM_READ_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-03904");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-03905");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-03906");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-03907");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-07454");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-03909");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-03910");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-03911");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-03912");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-03913");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-03914");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-03915");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_HOST_READ_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-03916");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.dstAccessMask = VK_ACCESS_2_HOST_WRITE_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-03917");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    m_command_buffer.End();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, BarrierAccessSync2RtxMaintenance1) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredFeature(vkt::Feature::rayTracingMaintenance1);
    RETURN_IF_SKIP(Init());

    VkMemoryBarrier2 mem_barrier = vku::InitStructHelper();
    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &mem_barrier;

    m_command_buffer.Begin();

    mem_barrier.srcAccessMask = VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-07272");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    mem_barrier.srcAccessMask = VK_ACCESS_2_HOST_WRITE_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-07272");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, BarrierAccessSync2DescriptorBuffer) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    RETURN_IF_SKIP(Init());

    VkMemoryBarrier2 mem_barrier = vku::InitStructHelper();
    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &mem_barrier;

    m_command_buffer.Begin();

    mem_barrier.srcAccessMask = VK_ACCESS_2_DESCRIPTOR_BUFFER_READ_BIT_EXT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-08118");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    mem_barrier.srcAccessMask = VK_ACCESS_2_HOST_WRITE_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_2_DESCRIPTOR_BUFFER_READ_BIT_EXT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-08118");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, BarrierAccessVideoDecode) {
    TEST_DESCRIPTION("Test barrier with access decode read bit.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    VkMemoryBarrier2 mem_barrier = vku::InitStructHelper();
    mem_barrier.srcAccessMask = VK_ACCESS_2_VIDEO_DECODE_READ_BIT_KHR;
    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &mem_barrier;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-04858");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_VIDEO_DECODE_WRITE_BIT_KHR;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-04859");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_VIDEO_ENCODE_READ_BIT_KHR;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-04860");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    mem_barrier.srcAccessMask = VK_ACCESS_2_VIDEO_ENCODE_WRITE_BIT_KHR;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-srcAccessMask-04861");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    m_command_buffer.End();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, Sync2LayoutFeature) {
    SetTargetApiVersion(VK_API_VERSION_1_3);

    RETURN_IF_SKIP(Init());

    VkImageCreateInfo info = vkt::Image::CreateInfo();
    info.format = VK_FORMAT_B8G8R8A8_UNORM;
    info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    vkt::Image image(*m_device, info, vkt::set_layout);

    m_command_buffer.Begin();
    VkImageMemoryBarrier2 img_barrier = vku::InitStructHelper();
    img_barrier.image = image.handle();
    img_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    img_barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    img_barrier.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;

    VkDependencyInfo dep_info = vku::InitStructHelper();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &img_barrier;
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier2-synchronization2-03848");
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier2-synchronization2-07793");  // oldLayout
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier2-synchronization2-07794");  // newLayout
    vk::CmdPipelineBarrier2(m_command_buffer.handle(), &dep_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, SubmitSignaledFence) {
    RETURN_IF_SKIP(Init());

    VkFenceCreateInfo fenceInfo = vku::InitStructHelper();
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkt::Fence testFence(*m_device, fenceInfo);

    m_command_buffer.Begin();
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-fence-00063");
    m_default_queue->Submit(m_command_buffer, testFence);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeSyncObject, QueueSubmitWaitingSameSemaphore) {
    TEST_DESCRIPTION("Submit to queue with waitSemaphore that another queue is already waiting on.");

    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    all_queue_count_ = true;
    RETURN_IF_SKIP(Init());

    if ((m_second_queue_caps & VK_QUEUE_GRAPHICS_BIT) == 0) {
        GTEST_SKIP() << "2 graphics queues are needed";
    }

    vkt::Semaphore semaphore(*m_device);
    {
        VkPipelineStageFlags stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        m_default_queue->Submit(vkt::no_cmd, vkt::Signal(semaphore));
        m_default_queue->Submit(vkt::no_cmd, vkt::Wait(semaphore, stage_flags));

        m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pWaitSemaphores-00068");
        m_second_queue->Submit(vkt::no_cmd, vkt::Wait(semaphore, stage_flags));
        m_errorMonitor->VerifyFound();
        m_default_queue->Wait();
        m_second_queue->Wait();
    }
    if (m_device->Physical().queue_properties_[m_default_queue->family_index].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
        VkBindSparseInfo signal_bind = vku::InitStructHelper();
        signal_bind.signalSemaphoreCount = 1;
        signal_bind.pSignalSemaphores = &semaphore.handle();

        VkBindSparseInfo wait_bind = vku::InitStructHelper();
        wait_bind.waitSemaphoreCount = 1;
        wait_bind.pWaitSemaphores = &semaphore.handle();

        vk::QueueBindSparse(m_default_queue->handle(), 1, &signal_bind, VK_NULL_HANDLE);
        vk::QueueBindSparse(m_default_queue->handle(), 1, &wait_bind, VK_NULL_HANDLE);

        m_errorMonitor->SetDesiredError("VUID-vkQueueBindSparse-pWaitSemaphores-01116");
        vk::QueueBindSparse(m_second_queue->handle(), 1, &wait_bind, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        m_default_queue->Wait();
        m_second_queue->Wait();
    }

    // sync 2
    {
        VkSemaphoreSubmitInfo signal_sem_info = vku::InitStructHelper();
        signal_sem_info.semaphore = semaphore.handle();
        signal_sem_info.stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        VkSubmitInfo2 signal_submit = vku::InitStructHelper();
        signal_submit.signalSemaphoreInfoCount = 1;
        signal_submit.pSignalSemaphoreInfos = &signal_sem_info;

        VkSemaphoreSubmitInfo wait_sem_info = vku::InitStructHelper();
        wait_sem_info.semaphore = semaphore.handle();
        wait_sem_info.stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        VkSubmitInfo2 wait_submit = vku::InitStructHelper();
        wait_submit.waitSemaphoreInfoCount = 1;
        wait_submit.pWaitSemaphoreInfos = &wait_sem_info;

        vk::QueueSubmit2KHR(m_default_queue->handle(), 1, &signal_submit, VK_NULL_HANDLE);
        vk::QueueSubmit2KHR(m_default_queue->handle(), 1, &wait_submit, VK_NULL_HANDLE);
        m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit2-semaphore-03871");
        vk::QueueSubmit2KHR(m_second_queue->handle(), 1, &wait_submit, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        m_default_queue->Wait();
        m_second_queue->Wait();
    }
}

TEST_F(NegativeSyncObject, QueueSubmit2KHRUsedButSynchronizaion2Disabled) {
    TEST_DESCRIPTION("Using QueueSubmit2KHR when synchronization2 is not enabled");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    bool vulkan_13 = (DeviceValidationVersion() >= VK_API_VERSION_1_3);

    VkSubmitInfo2 submit_info = vku::InitStructHelper();
    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit2-synchronization2-03866");
    vk::QueueSubmit2KHR(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    if (vulkan_13) {
        m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit2-synchronization2-03866");
        m_default_queue->Submit2(vkt::no_cmd);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeSyncObject, WaitEventsDifferentQueueFamilies) {
    TEST_DESCRIPTION("Using CmdWaitEvents with invalid barrier queue families");
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    const std::optional<uint32_t> no_gfx = m_device->QueueFamilyWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT);
    if (!no_gfx) {
        GTEST_SKIP() << "Required queue families not present (non-graphics non-compute capable required)";
    }

    vkt::Event event(*m_device);
    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VkBufferMemoryBarrier BufferMemoryBarrier = vku::InitStructHelper();
    BufferMemoryBarrier.srcAccessMask = 0;
    BufferMemoryBarrier.dstAccessMask = 0;
    BufferMemoryBarrier.buffer = buffer.handle();
    BufferMemoryBarrier.offset = 0;
    BufferMemoryBarrier.size = 256;
    BufferMemoryBarrier.srcQueueFamilyIndex = m_device->graphics_queue_node_index_;
    BufferMemoryBarrier.dstQueueFamilyIndex = no_gfx.value();

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageMemoryBarrier ImageMemoryBarrier = vku::InitStructHelper();
    ImageMemoryBarrier.srcAccessMask = 0;
    ImageMemoryBarrier.dstAccessMask = 0;
    ImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    ImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    ImageMemoryBarrier.image = image.handle();
    ImageMemoryBarrier.srcQueueFamilyIndex = m_device->graphics_queue_node_index_;
    ImageMemoryBarrier.dstQueueFamilyIndex = no_gfx.value();
    ImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    ImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    ImageMemoryBarrier.subresourceRange.layerCount = 1;
    ImageMemoryBarrier.subresourceRange.levelCount = 1;

    m_command_buffer.Begin();
    vk::CmdSetEvent(m_command_buffer.handle(), event.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-srcQueueFamilyIndex-02803");
    vk::CmdWaitEvents(m_command_buffer.handle(), 1, &event.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, nullptr, 1, &BufferMemoryBarrier, 0, nullptr);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-srcQueueFamilyIndex-02803");
    vk::CmdWaitEvents(m_command_buffer.handle(), 1, &event.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, nullptr, 0, nullptr, 1, &ImageMemoryBarrier);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, CmdWaitEvents2DependencyFlags) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);
    VkEvent event_handle = event.handle();

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_VIEW_LOCAL_BIT;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents2-dependencyFlags-10394");
    vk::CmdWaitEvents2KHR(m_command_buffer.handle(), 1, &event_handle, &dependency_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, WaitEvent2HostStage) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    vkt::Event event(*m_device);
    VkEvent event_handle = event.handle();

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &barrier;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents2-dependencyFlags-03844");
    vk::CmdWaitEvents2KHR(m_command_buffer.handle(), 1, &event_handle, &dependency_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, SemaphoreTypeCreateInfoCore) {
    TEST_DESCRIPTION("Invalid usage of VkSemaphoreTypeCreateInfo with a 1.2 core version");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    // Core 1.2 supports timelineSemaphore feature bit but not enabled
    RETURN_IF_SKIP(Init());

    VkSemaphore semaphore;

    VkSemaphoreTypeCreateInfo semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    semaphore_type_create_info.initialValue = 1;

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper(&semaphore_type_create_info);
    semaphore_create_info.flags = 0;

    // timelineSemaphore feature bit not set
    m_errorMonitor->SetDesiredError("VUID-VkSemaphoreTypeCreateInfo-timelineSemaphore-03252");
    vk::CreateSemaphore(device(), &semaphore_create_info, nullptr, &semaphore);
    m_errorMonitor->VerifyFound();

    // Binary semaphore can't be initialValue 0
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
    m_errorMonitor->SetDesiredError("VUID-VkSemaphoreTypeCreateInfo-semaphoreType-03279");
    vk::CreateSemaphore(device(), &semaphore_create_info, nullptr, &semaphore);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, SemaphoreTypeCreateInfoExtension) {
    TEST_DESCRIPTION("Invalid usage of VkSemaphoreTypeCreateInfo with extension");

    SetTargetApiVersion(VK_API_VERSION_1_1);  // before timelineSemaphore was added to core
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    // Enabled extension but not the timelineSemaphore feature bit
    RETURN_IF_SKIP(Init());

    VkSemaphore semaphore;

    VkSemaphoreTypeCreateInfo semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    semaphore_type_create_info.initialValue = 1;

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper(&semaphore_type_create_info);
    semaphore_create_info.flags = 0;

    // timelineSemaphore feature bit not set
    m_errorMonitor->SetDesiredError("VUID-VkSemaphoreTypeCreateInfo-timelineSemaphore-03252");
    vk::CreateSemaphore(device(), &semaphore_create_info, nullptr, &semaphore);
    m_errorMonitor->VerifyFound();

    // Binary semaphore can't be initialValue 0
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
    m_errorMonitor->SetDesiredError("VUID-VkSemaphoreTypeCreateInfo-semaphoreType-03279");
    vk::CreateSemaphore(device(), &semaphore_create_info, nullptr, &semaphore);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, MixedTimelineAndBinarySemaphores) {
    TEST_DESCRIPTION("Submit mixtures of timeline and binary semaphores");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceTimelineSemaphorePropertiesKHR timelineproperties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(timelineproperties);

    VkSemaphoreTypeCreateInfo semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    semaphore_type_create_info.initialValue = 5;

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper(&semaphore_type_create_info);

    VkSemaphore semaphore[2];
    vk::CreateSemaphore(device(), &semaphore_create_info, nullptr, &semaphore[0]);
    // index 1 should be a binary semaphore
    semaphore_create_info.pNext = nullptr;
    vk::CreateSemaphore(device(), &semaphore_create_info, nullptr, &semaphore[1]);
    VkSemaphore extra_binary;
    vk::CreateSemaphore(device(), &semaphore_create_info, nullptr, &extra_binary);

    VkSemaphoreSignalInfo semaphore_signal_info = vku::InitStructHelper();
    semaphore_signal_info.semaphore = semaphore[0];
    semaphore_signal_info.value = 3;
    semaphore_signal_info.value = 10;
    vk::SignalSemaphoreKHR(device(), &semaphore_signal_info);

    VkTimelineSemaphoreSubmitInfo timeline_semaphore_submit_info = vku::InitStructHelper();
    uint64_t signalValue = 20;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 0;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = nullptr;
    // this array needs a length of 2, even though the binary semaphore won't look at the values array
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = &signalValue;

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submit_info = vku::InitStructHelper(&timeline_semaphore_submit_info);
    submit_info.pWaitDstStageMask = &stageFlags;
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = nullptr;
    submit_info.signalSemaphoreCount = 2;
    submit_info.pSignalSemaphores = semaphore;
    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-pNext-03241");
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    uint64_t values[2] = {signalValue, 0 /*ignored*/};
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 2;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = values;
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);

    // the indexes in pWaitSemaphores and pWaitSemaphoreValues should match
    VkSemaphore reversed[2] = {semaphore[1], semaphore[0]};
    uint64_t reversed_values[2] = {vvl::kU64Max /* ignored */, 20};
    VkPipelineStageFlags wait_stages[2] = {VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT};
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = nullptr;
    submit_info.waitSemaphoreCount = 2;
    submit_info.pWaitSemaphores = reversed;
    submit_info.pWaitDstStageMask = wait_stages;
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 0;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = nullptr;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 2;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = reversed_values;
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);

    // if we only signal a binary semaphore we don't need a 'values' array
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 0;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = nullptr;
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 0;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = nullptr;
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = nullptr;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &extra_binary;
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);

    m_default_queue->Wait();
    vk::DestroySemaphore(device(), semaphore[0], nullptr);
    vk::DestroySemaphore(device(), semaphore[1], nullptr);
    vk::DestroySemaphore(device(), extra_binary, nullptr);
}

TEST_F(NegativeSyncObject, QueueSubmitNoTimelineSemaphoreInfo) {
    TEST_DESCRIPTION("Submit a queue with a timeline semaphore but not a VkTimelineSemaphoreSubmitInfo.");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    VkSemaphoreTypeCreateInfo semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper(&semaphore_type_create_info);

    VkSemaphore semaphore;
    ASSERT_EQ(VK_SUCCESS, vk::CreateSemaphore(device(), &semaphore_create_info, nullptr, &semaphore));

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submit_info[2] = {};
    submit_info[0] = vku::InitStructHelper();
    submit_info[0].commandBufferCount = 0;
    submit_info[0].pWaitDstStageMask = &stageFlags;
    submit_info[0].signalSemaphoreCount = 1;
    submit_info[0].pSignalSemaphores = &semaphore;

    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-pWaitSemaphores-03239");
    vk::QueueSubmit(m_default_queue->handle(), 1, submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    VkTimelineSemaphoreSubmitInfo timeline_semaphore_submit_info = vku::InitStructHelper();
    uint64_t signalValue = 1;
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = &signalValue;
    submit_info[0].pNext = &timeline_semaphore_submit_info;

    submit_info[1] = vku::InitStructHelper();
    submit_info[1].commandBufferCount = 0;
    submit_info[1].pWaitDstStageMask = &stageFlags;
    submit_info[1].waitSemaphoreCount = 1;
    submit_info[1].pWaitSemaphores = &semaphore;

    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-pWaitSemaphores-03239");
    vk::QueueSubmit(m_default_queue->handle(), 2, submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    vk::DestroySemaphore(device(), semaphore, nullptr);
}

TEST_F(NegativeSyncObject, QueueSubmitTimelineSemaphoreValue) {
    TEST_DESCRIPTION("Submit a queue with a timeline semaphore using a wrong payload value.");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceTimelineSemaphorePropertiesKHR timelineproperties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(timelineproperties);

    VkSemaphoreTypeCreateInfo semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper(&semaphore_type_create_info);

    vkt::Semaphore semaphore(*m_device, semaphore_create_info);

    VkTimelineSemaphoreSubmitInfo timeline_semaphore_submit_info = vku::InitStructHelper();
    uint64_t signalValue = 1;
    uint64_t waitValue = 3;
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = &signalValue;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = &waitValue;

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submit_info = vku::InitStructHelper(&timeline_semaphore_submit_info);
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore.handle();
    submit_info.pWaitDstStageMask = &stageFlags;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();

    timeline_semaphore_submit_info.signalSemaphoreValueCount = 0;
    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-pNext-03241");
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 0;

    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-pNext-03240");
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    signalValue = 5;
    {
        VkSemaphoreSignalInfo semaphore_signal_info = vku::InitStructHelper();
        semaphore_signal_info.semaphore = semaphore.handle();
        semaphore_signal_info.value = signalValue;
        ASSERT_EQ(VK_SUCCESS, vk::SignalSemaphoreKHR(device(), &semaphore_signal_info));
    }

    timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;

    // Check for re-signalling an already completed value (5)
    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-pSignalSemaphores-03242");
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Submit (6)
    signalValue++;
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);

    // Check against a pending value (6)
    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-pSignalSemaphores-03242");
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    {
        // Double signal with the same value (7)
        signalValue++;
        uint64_t signal_values[2] = {signalValue, signalValue};
        VkSemaphore signal_sems[2] = {semaphore.handle(), semaphore.handle()};

        VkTimelineSemaphoreSubmitInfo tl_info_2 = vku::InitStructHelper();
        tl_info_2.signalSemaphoreValueCount = 2;
        tl_info_2.pSignalSemaphoreValues = signal_values;

        VkSubmitInfo submit_info2 = vku::InitStructHelper(&tl_info_2);
        submit_info2.signalSemaphoreCount = 2;
        submit_info2.pSignalSemaphores = signal_sems;

        m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-pSignalSemaphores-03242");
        vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info2, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }

    // Check if we can test violations of maxTimelineSemaphoreValueDifference
    if (timelineproperties.maxTimelineSemaphoreValueDifference < vvl::kU64Max) {
        uint64_t bigValue = signalValue + timelineproperties.maxTimelineSemaphoreValueDifference + 1;
        timeline_semaphore_submit_info.pSignalSemaphoreValues = &bigValue;

        m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-pSignalSemaphores-03244");
        vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        if (signalValue < vvl::kU64Max) {
            signalValue++;
            timeline_semaphore_submit_info.pSignalSemaphoreValues = &signalValue;
            waitValue = signalValue + timelineproperties.maxTimelineSemaphoreValueDifference + 1;

            submit_info.signalSemaphoreCount = 0;
            timeline_semaphore_submit_info.signalSemaphoreValueCount = 0;
            timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
            timeline_semaphore_submit_info.pWaitSemaphoreValues = &waitValue;

            m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-pWaitSemaphores-03243");
            vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
            m_errorMonitor->VerifyFound();
        }
    }
    m_default_queue->Wait();
}

TEST_F(NegativeSyncObject, QueueBindSparseTimelineSemaphoreValue) {
    TEST_DESCRIPTION("Submit a queue with a timeline semaphore using a wrong payload value.");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    auto index = m_device->graphics_queue_node_index_;
    if ((m_device->Physical().queue_properties_[index].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) == 0) {
        GTEST_SKIP() << "Graphics queue does not have sparse binding bit";
    }

    VkPhysicalDeviceTimelineSemaphorePropertiesKHR timelineproperties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(timelineproperties);

    VkSemaphoreTypeCreateInfo semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper(&semaphore_type_create_info);

    vkt::Semaphore semaphore(*m_device, semaphore_create_info);

    VkTimelineSemaphoreSubmitInfo timeline_semaphore_submit_info = vku::InitStructHelper();
    uint64_t signalValue = 1;
    uint64_t waitValue = 3;
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = &signalValue;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = &waitValue;

    VkBindSparseInfo submit_info = vku::InitStructHelper();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore.handle();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();

    // error for both signal and wait
    m_errorMonitor->SetDesiredError("VUID-VkBindSparseInfo-pWaitSemaphores-03246");
    m_errorMonitor->SetDesiredError("VUID-VkBindSparseInfo-pWaitSemaphores-03246");
    vk::QueueBindSparse(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    submit_info.pNext = &timeline_semaphore_submit_info;

    timeline_semaphore_submit_info.signalSemaphoreValueCount = 0;
    m_errorMonitor->SetDesiredError("VUID-VkBindSparseInfo-pNext-03248");
    vk::QueueBindSparse(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    submit_info.signalSemaphoreCount = 1;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 0;
    submit_info.waitSemaphoreCount = 1;

    m_errorMonitor->SetDesiredError("VUID-VkBindSparseInfo-pNext-03247");
    vk::QueueBindSparse(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    signalValue = 5;
    {
        VkSemaphoreSignalInfo semaphore_signal_info = vku::InitStructHelper();
        semaphore_signal_info.semaphore = semaphore.handle();
        semaphore_signal_info.value = signalValue;
        ASSERT_EQ(VK_SUCCESS, vk::SignalSemaphoreKHR(device(), &semaphore_signal_info));
    }

    timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
    submit_info.waitSemaphoreCount = 1;

    // Check for re-signalling an already completed value (5)
    m_errorMonitor->SetDesiredError("VUID-VkBindSparseInfo-pSignalSemaphores-03249");
    vk::QueueBindSparse(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Submit (6)
    signalValue++;
    vk::QueueBindSparse(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);

    // Check against a pending value (6)
    m_errorMonitor->SetDesiredError("VUID-VkBindSparseInfo-pSignalSemaphores-03249");
    vk::QueueBindSparse(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    {
        // Double signal with the same value (7)
        signalValue++;
        uint64_t signal_values[2] = {signalValue, signalValue};
        VkSemaphore signal_sems[2] = {semaphore.handle(), semaphore.handle()};

        VkTimelineSemaphoreSubmitInfo tl_info_2 = vku::InitStructHelper();
        tl_info_2.signalSemaphoreValueCount = 2;
        tl_info_2.pSignalSemaphoreValues = signal_values;

        VkBindSparseInfo submit_info2 = vku::InitStructHelper(&tl_info_2);
        submit_info2.signalSemaphoreCount = 2;
        submit_info2.pSignalSemaphores = signal_sems;

        m_errorMonitor->SetDesiredError("VUID-VkBindSparseInfo-pSignalSemaphores-03249");
        vk::QueueBindSparse(m_default_queue->handle(), 1, &submit_info2, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }

    // Check if we can test violations of maxTimelineSemaphoreValueDifference
    if (timelineproperties.maxTimelineSemaphoreValueDifference < vvl::kU64Max) {
        uint64_t bigValue = signalValue + timelineproperties.maxTimelineSemaphoreValueDifference + 1;
        timeline_semaphore_submit_info.pSignalSemaphoreValues = &bigValue;

        m_errorMonitor->SetDesiredError("VUID-VkBindSparseInfo-pSignalSemaphores-03251");
        vk::QueueBindSparse(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        if (signalValue < vvl::kU64Max) {
            waitValue = bigValue;

            submit_info.signalSemaphoreCount = 0;
            submit_info.waitSemaphoreCount = 1;
            timeline_semaphore_submit_info.signalSemaphoreValueCount = 0;
            timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
            timeline_semaphore_submit_info.pWaitSemaphoreValues = &waitValue;

            m_errorMonitor->SetDesiredError("VUID-VkBindSparseInfo-pWaitSemaphores-03250");
            vk::QueueBindSparse(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
            m_errorMonitor->VerifyFound();
        }
    }
    m_default_queue->Wait();
}

TEST_F(NegativeSyncObject, Sync2QueueSubmitTimelineSemaphoreValue) {
    TEST_DESCRIPTION("Submit a queue with a timeline semaphore using a wrong payload value.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceTimelineSemaphorePropertiesKHR timelineproperties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(timelineproperties);

    uint64_t value = 5;
    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE, value);

    // Check for re-signalling an already completed value (5)
    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo2-semaphore-03882");
    m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(semaphore, value), vkt::no_fence, true);
    m_errorMonitor->VerifyFound();

    // Submit (6)
    value++;
    m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(semaphore, value), vkt::no_fence, true);

    // Check against a pending value (6)
    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo2-semaphore-03882");
    m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(semaphore, value), vkt::no_fence, true);
    m_errorMonitor->VerifyFound();

    // Double signal with the same value (7)
    value++;
    {
        VkSemaphoreSubmitInfo signal_info = vku::InitStructHelper();
        signal_info.value = value;
        signal_info.semaphore = semaphore.handle();
        signal_info.stageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

        VkSemaphoreSubmitInfo double_signal_info[2];
        double_signal_info[0] = signal_info;
        double_signal_info[1] = signal_info;

        VkSubmitInfo2 submit_info = vku::InitStructHelper();
        submit_info.signalSemaphoreInfoCount = 2;
        submit_info.pSignalSemaphoreInfos = double_signal_info;

        m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo2-semaphore-03882");
        vk::QueueSubmit2KHR(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }

    // Check if we can test violations of maxTimelineSemaphoreValueDifference
    if (value < (value + timelineproperties.maxTimelineSemaphoreValueDifference + 1)) {
        value += timelineproperties.maxTimelineSemaphoreValueDifference + 1;

        m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo2-semaphore-03883");
        m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(semaphore, value), vkt::no_fence, true);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo2-semaphore-03884");
        m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineWait(semaphore, value), vkt::no_fence, true);
        m_errorMonitor->VerifyFound();
    }
    m_default_queue->Wait();
}

TEST_F(NegativeSyncObject, QueueSubmitBinarySemaphoreNotSignaled) {
    TEST_DESCRIPTION("Submit a queue with a waiting binary semaphore not previously signaled.");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper();
    // VUIDs reported change if the extension is enabled, even if the timelineSemaphore feature isn't supported.

    {
        vkt::Semaphore semaphore[3];
        semaphore[0].init(*m_device, semaphore_create_info);
        semaphore[1].init(*m_device, semaphore_create_info);
        semaphore[2].init(*m_device, semaphore_create_info);

        VkPipelineStageFlags stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkSubmitInfo submit_info[3] = {};
        submit_info[0] = vku::InitStructHelper();
        submit_info[0].pWaitDstStageMask = &stage_flags;
        submit_info[0].waitSemaphoreCount = 1;
        submit_info[0].pWaitSemaphores = &semaphore[0].handle();
        submit_info[0].signalSemaphoreCount = 1;
        submit_info[0].pSignalSemaphores = &semaphore[1].handle();
        m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pWaitSemaphores-03238");
        vk::QueueSubmit(m_default_queue->handle(), 1, submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        submit_info[1] = vku::InitStructHelper();
        submit_info[1].pWaitDstStageMask = &stage_flags;
        submit_info[1].waitSemaphoreCount = 1;
        submit_info[1].pWaitSemaphores = &semaphore[1].handle();
        submit_info[1].signalSemaphoreCount = 1;
        submit_info[1].pSignalSemaphores = &semaphore[2].handle();

        m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pWaitSemaphores-03238");
        vk::QueueSubmit(m_default_queue->handle(), 2, &submit_info[0], VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        submit_info[2] = vku::InitStructHelper();
        submit_info[2].signalSemaphoreCount = 1;
        submit_info[2].pSignalSemaphores = &semaphore[0].handle();

        ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info[2], VK_NULL_HANDLE));
        ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit(m_default_queue->handle(), 2, submit_info, VK_NULL_HANDLE));
        m_default_queue->Wait();
    }
    if (m_device->Physical().queue_properties_[m_default_queue->family_index].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
        vkt::Semaphore semaphore[3];
        semaphore[0].init(*m_device, semaphore_create_info);
        semaphore[1].init(*m_device, semaphore_create_info);
        semaphore[2].init(*m_device, semaphore_create_info);

        VkBindSparseInfo bind_info[3] = {};

        bind_info[0] = vku::InitStructHelper();
        bind_info[0].waitSemaphoreCount = 1;
        bind_info[0].pWaitSemaphores = &semaphore[0].handle();
        bind_info[0].signalSemaphoreCount = 1;
        bind_info[0].pSignalSemaphores = &semaphore[1].handle();

        m_errorMonitor->SetDesiredError("VUID-vkQueueBindSparse-pWaitSemaphores-03245");
        vk::QueueBindSparse(m_default_queue->handle(), 1, &bind_info[0], VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        bind_info[1] = vku::InitStructHelper();
        bind_info[1].waitSemaphoreCount = 1;
        bind_info[1].pWaitSemaphores = &semaphore[1].handle();
        bind_info[1].signalSemaphoreCount = 1;
        bind_info[1].pSignalSemaphores = &semaphore[2].handle();

        m_errorMonitor->SetDesiredError("VUID-vkQueueBindSparse-pWaitSemaphores-03245");
        vk::QueueBindSparse(m_default_queue->handle(), 2, bind_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        bind_info[2] = vku::InitStructHelper();
        bind_info[2].signalSemaphoreCount = 1;
        bind_info[2].pSignalSemaphores = &semaphore[0].handle();

        ASSERT_EQ(VK_SUCCESS, vk::QueueBindSparse(m_default_queue->handle(), 1, &bind_info[2], VK_NULL_HANDLE));
        ASSERT_EQ(VK_SUCCESS, vk::QueueBindSparse(m_default_queue->handle(), 2, bind_info, VK_NULL_HANDLE));
        m_default_queue->Wait();
    }

    {
        vkt::Semaphore semaphore[3];
        semaphore[0].init(*m_device, semaphore_create_info);
        semaphore[1].init(*m_device, semaphore_create_info);
        semaphore[2].init(*m_device, semaphore_create_info);

        VkSemaphoreSubmitInfo sem_info[3];
        for (int i = 0; i < 3; i++) {
            sem_info[i] = vku::InitStructHelper();
            sem_info[i].semaphore = semaphore[i].handle();
            sem_info[i].stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        }

        VkSubmitInfo2 submit_info[3] = {};
        submit_info[0] = vku::InitStructHelper();
        submit_info[0].waitSemaphoreInfoCount = 1;
        submit_info[0].pWaitSemaphoreInfos = &sem_info[0];
        submit_info[0].signalSemaphoreInfoCount = 1;
        submit_info[0].pSignalSemaphoreInfos = &sem_info[1];

        m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit2-semaphore-03873");
        vk::QueueSubmit2KHR(m_default_queue->handle(), 1, &submit_info[0], VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        submit_info[1] = vku::InitStructHelper();
        submit_info[1].waitSemaphoreInfoCount = 1;
        submit_info[1].pWaitSemaphoreInfos = &sem_info[1];
        submit_info[1].signalSemaphoreInfoCount = 1;
        submit_info[1].pSignalSemaphoreInfos = &sem_info[2];

        m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit2-semaphore-03873");
        vk::QueueSubmit2KHR(m_default_queue->handle(), 2, submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        submit_info[2] = vku::InitStructHelper();
        submit_info[2].signalSemaphoreInfoCount = 1;
        submit_info[2].pSignalSemaphoreInfos = &sem_info[0];

        ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit2KHR(m_default_queue->handle(), 1, &submit_info[2], VK_NULL_HANDLE));
        ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit2KHR(m_default_queue->handle(), 2, submit_info, VK_NULL_HANDLE));
        m_default_queue->Wait();
    }
}

TEST_F(NegativeSyncObject, QueueSubmitTimelineSemaphoreOutOfOrder) {
    TEST_DESCRIPTION("Submit out-of-order timeline semaphores.");
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed to run this test";
    }
    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE, 5);
    m_default_queue->Submit(vkt::no_cmd, vkt::TimelineWait(semaphore, 10), vkt::TimelineSignal(semaphore, 100));
    m_second_queue->Submit(vkt::no_cmd, vkt::TimelineWait(semaphore, 0), vkt::TimelineSignal(semaphore, 10));
    m_device->Wait();
}

TEST_F(NegativeSyncObject, WaitSemaphoresType) {
    TEST_DESCRIPTION("Wait for a non Timeline Semaphore");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    VkSemaphoreTypeCreateInfo semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper(&semaphore_type_create_info);

    VkSemaphore semaphore[2];
    vk::CreateSemaphore(device(), &semaphore_create_info, nullptr, &(semaphore[0]));

    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
    vk::CreateSemaphore(device(), &semaphore_create_info, nullptr, &(semaphore[1]));

    VkSemaphoreWaitInfo semaphore_wait_info = vku::InitStructHelper();
    semaphore_wait_info.semaphoreCount = 2;
    semaphore_wait_info.pSemaphores = &semaphore[0];
    const uint64_t wait_values[] = {10, 40};
    semaphore_wait_info.pValues = &wait_values[0];

    m_errorMonitor->SetDesiredError("VUID-VkSemaphoreWaitInfo-pSemaphores-03256");
    vk::WaitSemaphoresKHR(device(), &semaphore_wait_info, 10000);
    m_errorMonitor->VerifyFound();

    vk::DestroySemaphore(device(), semaphore[0], nullptr);
    vk::DestroySemaphore(device(), semaphore[1], nullptr);
}

TEST_F(NegativeSyncObject, SignalSemaphoreType) {
    TEST_DESCRIPTION("Signal a non Timeline Semaphore");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    vkt::Semaphore semaphore(*m_device);

    VkSemaphoreSignalInfo semaphore_signal_info = vku::InitStructHelper();
    semaphore_signal_info.semaphore = semaphore.handle();
    semaphore_signal_info.value = 10;

    m_errorMonitor->SetDesiredError("VUID-VkSemaphoreSignalInfo-semaphore-03257");
    vk::SignalSemaphoreKHR(device(), &semaphore_signal_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, SignalSemaphoreValue) {
    TEST_DESCRIPTION("Signal a Timeline Semaphore with invalid values");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceTimelineSemaphorePropertiesKHR timelineproperties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(timelineproperties);

    vkt::Semaphore timeline0(*m_device, VK_SEMAPHORE_TYPE_TIMELINE, 5);
    vkt::Semaphore timeline1(*m_device, VK_SEMAPHORE_TYPE_TIMELINE, 5);

    m_errorMonitor->SetDesiredError("VUID-VkSemaphoreSignalInfo-value-03258");
    timeline0.SignalKHR(3);
    m_errorMonitor->VerifyFound();

    timeline0.SignalKHR(10);
    m_default_queue->Submit(vkt::no_cmd, vkt::TimelineWait(timeline1, 10), vkt::TimelineSignal(timeline0, 20));

    m_errorMonitor->SetDesiredError("VUID-VkSemaphoreSignalInfo-value-03259");
    timeline0.SignalKHR(25);
    m_errorMonitor->VerifyFound();

    timeline0.SignalKHR(15);
    timeline1.SignalKHR(15);

    // Test violations of maxTimelineSemaphoreValueDifference
    if (timelineproperties.maxTimelineSemaphoreValueDifference < vvl::kU64Max) {
        vkt::Semaphore sem(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);

        m_errorMonitor->SetDesiredError("VUID-VkSemaphoreSignalInfo-value-03260");
        sem.SignalKHR(timelineproperties.maxTimelineSemaphoreValueDifference + 1);
        m_errorMonitor->VerifyFound();

        sem.SignalKHR(timelineproperties.maxTimelineSemaphoreValueDifference);
        m_default_queue->Wait();
    }
    // Regression test for value difference validations ran against binary semaphores
    {
        vkt::Semaphore timeline_sem(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
        vkt::Semaphore binary_sem(*m_device);

        uint64_t signalValue = 1;
        uint64_t offendingValue = timelineproperties.maxTimelineSemaphoreValueDifference + 1;

        VkTimelineSemaphoreSubmitInfo timeline_semaphore_submit_info = vku::InitStructHelper();
        timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
        timeline_semaphore_submit_info.pWaitSemaphoreValues = &signalValue;
        // These two assignments are not required by the spec, but would segfault on older versions of validation layers
        timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
        timeline_semaphore_submit_info.pSignalSemaphoreValues = &offendingValue;

        VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkSubmitInfo submit_info = vku::InitStructHelper(&timeline_semaphore_submit_info);
        submit_info.pWaitDstStageMask = &stageFlags;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &timeline_sem.handle();
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &binary_sem.handle();

        vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
        timeline_sem.SignalKHR(signalValue);
        m_default_queue->Wait();
    }
    m_default_queue->Wait();
}

TEST_F(NegativeSyncObject, Sync2SignalSemaphoreValue) {
    TEST_DESCRIPTION("Signal a Timeline Semaphore with invalid values");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceTimelineSemaphorePropertiesKHR timelineproperties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(timelineproperties);

    VkSemaphoreTypeCreateInfo semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    semaphore_type_create_info.initialValue = 5;

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper(&semaphore_type_create_info);

    vkt::Semaphore semaphore[2];
    semaphore[0].init(*m_device, semaphore_create_info);
    semaphore[1].init(*m_device, semaphore_create_info);

    VkSemaphoreSignalInfo semaphore_signal_info = vku::InitStructHelper();
    semaphore_signal_info.semaphore = semaphore[0].handle();
    semaphore_signal_info.value = 10;
    ASSERT_EQ(VK_SUCCESS, vk::SignalSemaphore(device(), &semaphore_signal_info));

    VkSemaphoreSubmitInfo signal_info = vku::InitStructHelper();
    signal_info.semaphore = semaphore[0].handle();

    VkSemaphoreSubmitInfo wait_info = vku::InitStructHelper();
    wait_info.semaphore = semaphore[0].handle();
    wait_info.stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    VkSubmitInfo2 submit_info = vku::InitStructHelper();
    submit_info.signalSemaphoreInfoCount = 1;
    submit_info.pSignalSemaphoreInfos = &signal_info;
    submit_info.waitSemaphoreInfoCount = 1;
    submit_info.pWaitSemaphoreInfos = &wait_info;

    // signal value > wait value
    signal_info.value = 11;
    wait_info.value = 11;
    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo2-semaphore-03881");
    vk::QueueSubmit2KHR(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // signal value == current value
    signal_info.value = 10;
    wait_info.value = 5;
    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo2-semaphore-03882");
    vk::QueueSubmit2KHR(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    signal_info.value = 20;
    wait_info.value = 15;
    wait_info.semaphore = semaphore[1].handle();
    ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit2KHR(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE));

    semaphore_signal_info.value = 25;

    m_errorMonitor->SetDesiredError("VUID-VkSemaphoreSignalInfo-value-03259");
    vk::SignalSemaphore(device(), &semaphore_signal_info);
    m_errorMonitor->VerifyFound();

    semaphore_signal_info.value = 15;
    ASSERT_EQ(VK_SUCCESS, vk::SignalSemaphore(device(), &semaphore_signal_info));
    semaphore_signal_info.semaphore = semaphore[1].handle();
    ASSERT_EQ(VK_SUCCESS, vk::SignalSemaphore(device(), &semaphore_signal_info));

    // Check if we can test violations of maxTimelineSemaphoreValueDifference
    if (timelineproperties.maxTimelineSemaphoreValueDifference < vvl::kU64Max) {
        // Regression test for value difference validations ran against binary semaphores
        semaphore_type_create_info.initialValue = 0;
        vkt::Semaphore timeline_sem(*m_device, semaphore_create_info);

        vkt::Semaphore binary_sem(*m_device);

        wait_info.semaphore = timeline_sem.handle();
        wait_info.value = 1;

        signal_info.semaphore = binary_sem.handle();
        signal_info.value = timelineproperties.maxTimelineSemaphoreValueDifference + 1;

        vk::QueueSubmit2KHR(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);

        semaphore_signal_info.semaphore = timeline_sem.handle();
        semaphore_signal_info.value = 1;
        vk::SignalSemaphore(device(), &semaphore_signal_info);

        m_default_queue->Wait();
    }

    m_default_queue->Wait();
}

TEST_F(NegativeSyncObject, SemaphoreCounterType) {
    TEST_DESCRIPTION("Get payload from a non Timeline Semaphore");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    vkt::Semaphore semaphore(*m_device);

    uint64_t value = 0xdeadbeef;

    m_errorMonitor->SetDesiredError("VUID-vkGetSemaphoreCounterValue-semaphore-03255");
    vk::GetSemaphoreCounterValueKHR(device(), semaphore.handle(), &value);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, EventStageMaskOneCommandBufferPass) {
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    vk::CmdSetEvent(m_command_buffer.handle(), event.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    vk::CmdWaitEvents(m_command_buffer.handle(), 1, &event.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(NegativeSyncObject, EventStageMaskOneCommandBufferFail) {
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    vk::CmdSetEvent(m_command_buffer.handle(), event.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    // wrong srcStageMask
    vk::CmdWaitEvents(m_command_buffer.handle(), 1, &event.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-parameter");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeSyncObject, EventStageMaskTwoCommandBufferPass) {
    RETURN_IF_SKIP(Init());

    vkt::CommandBuffer commandBuffer1(*m_device, m_command_pool);
    vkt::CommandBuffer commandBuffer2(*m_device, m_command_pool);
    vkt::Event event(*m_device);

    commandBuffer1.Begin();
    vk::CmdSetEvent(commandBuffer1.handle(), event.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    commandBuffer1.End();
    m_default_queue->Submit(commandBuffer1);

    commandBuffer2.Begin();
    vk::CmdWaitEvents(commandBuffer2.handle(), 1, &event.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    commandBuffer2.End();
    m_default_queue->Submit(commandBuffer2);

    m_default_queue->Wait();
}

TEST_F(NegativeSyncObject, EventStageMaskTwoCommandBufferFail) {
    RETURN_IF_SKIP(Init());

    vkt::CommandBuffer commandBuffer1(*m_device, m_command_pool);
    vkt::CommandBuffer commandBuffer2(*m_device, m_command_pool);
    vkt::Event event(*m_device);

    commandBuffer1.Begin();
    vk::CmdSetEvent(commandBuffer1.handle(), event.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    commandBuffer1.End();
    m_default_queue->Submit(commandBuffer1);

    commandBuffer2.Begin();
    // wrong srcStageMask
    vk::CmdWaitEvents(commandBuffer2.handle(), 1, &event.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    commandBuffer2.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-parameter");
    m_default_queue->Submit(commandBuffer2);
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

TEST_F(NegativeSyncObject, DetectInterQueueEventUsage) {
    TEST_DESCRIPTION("Sets event on one queue and tries to wait on a different queue (CmdSetEvent/CmdWaitEvents)");
    all_queue_count_ = true;
    RETURN_IF_SKIP(Init());

    if ((m_second_queue_caps & VK_QUEUE_GRAPHICS_BIT) == 0) {
        GTEST_SKIP() << "2 graphics queues are needed";
    }
    const vkt::Event event(*m_device);

    vkt::CommandBuffer cb1(*m_device, m_command_pool);
    cb1.Begin();
    vk::CmdSetEvent(cb1, event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    cb1.End();

    vkt::CommandPool pool2(*m_device, m_second_queue->family_index);
    vkt::CommandBuffer cb2(*m_device, pool2);
    cb2.Begin();
    vk::CmdWaitEvents(cb2, 1, &event.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, nullptr,
                      0, nullptr, 0, nullptr);
    cb2.End();

    m_default_queue->Submit(cb1);
    m_errorMonitor->SetDesiredError("UNASSIGNED-SubmitValidation-WaitEvents-WrongQueue");
    m_second_queue->Submit(cb2);
    m_errorMonitor->VerifyFound();

    m_device->Wait();
}

TEST_F(NegativeSyncObject, DetectInterQueueEventUsage2) {
    TEST_DESCRIPTION("Sets event on one queue and tries to wait on a different queue (CmdSetEvent2/CmdWaitEvents2)");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    all_queue_count_ = true;
    RETURN_IF_SKIP(Init());

    if ((m_second_queue_caps & VK_QUEUE_GRAPHICS_BIT) == 0) {
        GTEST_SKIP() << "2 graphics queues are needed";
    }

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;
    barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_NONE;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &barrier;

    const vkt::Event event(*m_device);

    vkt::CommandBuffer cb1(*m_device, m_command_pool);
    cb1.Begin();
    vk::CmdSetEvent2(cb1, event, &dependency_info);
    cb1.End();

    vkt::CommandPool pool2(*m_device, m_second_queue->family_index);
    vkt::CommandBuffer cb2(*m_device, pool2);
    cb2.Begin();
    vk::CmdWaitEvents2(cb2, 1, &event.handle(), &dependency_info);
    cb2.End();

    m_default_queue->Submit(cb1);
    m_errorMonitor->SetDesiredError("UNASSIGNED-SubmitValidation-WaitEvents-WrongQueue");
    m_second_queue->Submit(cb2);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeSyncObject, SignalSignaledSemaphore) {
    TEST_DESCRIPTION("Call VkQueueSubmit with a semaphore that is already signaled but not waited on by the queue.");
    RETURN_IF_SKIP(Init());

    vkt::Semaphore semaphore(*m_device);

    // Signal semaphore
    vkt::CommandBuffer cb(*m_device, m_command_pool);
    cb.Begin();
    cb.End();
    m_default_queue->Submit(cb, vkt::Signal(semaphore));

    // Signal again
    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pSignalSemaphores-00067");
    m_default_queue->Submit(vkt::no_cmd, vkt::Signal(semaphore));
    m_errorMonitor->VerifyFound();

    m_device->Wait();
}

TEST_F(NegativeSyncObject, PipelineStageConditionalRenderingWithWrongQueue) {
    TEST_DESCRIPTION("Run CmdPipelineBarrier with VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT and wrong VkQueueFlagBits");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::conditionalRendering);
    RETURN_IF_SKIP(Init());

    auto only_transfer_queueFamilyIndex = m_device->TransferOnlyQueueFamily();
    if (!only_transfer_queueFamilyIndex.has_value()) {
        GTEST_SKIP() << "Transfer only queue is not supported";
    }

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::CommandPool commandPool(*m_device, only_transfer_queueFamilyIndex.value());
    vkt::CommandBuffer commandBuffer(*m_device, commandPool);

    commandBuffer.Begin();

    VkImageMemoryBarrier imb = vku::InitStructHelper();
    imb.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    imb.dstAccessMask = VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT;
    imb.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imb.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imb.image = image.handle();
    imb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imb.subresourceRange.baseMipLevel = 0;
    imb.subresourceRange.levelCount = 1;
    imb.subresourceRange.baseArrayLayer = 0;
    imb.subresourceRange.layerCount = 1;

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-srcStageMask-06461");
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-dstStageMask-06462");
    vk::CmdPipelineBarrier(commandBuffer.handle(), VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                           VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT, 0, 0, nullptr, 0, nullptr, 1, &imb);
    m_errorMonitor->VerifyFound();

    commandBuffer.End();
}

TEST_F(NegativeSyncObject, WaitOnNoEvent) {
    RETURN_IF_SKIP(Init());
    VkEvent bad_event = CastToHandle<VkEvent, uintptr_t>(0xbaadbeef);
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-pEvents-parameter");
    vk::CmdWaitEvents(m_command_buffer.handle(), 1, &bad_event, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
                      nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, InvalidDeviceOnlyEvent) {
    TEST_DESCRIPTION("Attempt to use device only event with host commands.");

    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());
    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_subset_features = vku::InitStructHelper();
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = vku::InitStructHelper();
    if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        sync2_features.pNext = &portability_subset_features;
    }
    GetPhysicalDeviceFeatures2(sync2_features);
    if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        if (portability_subset_features.events) {
            GTEST_SKIP() << "VkPhysicalDevicePortabilitySubsetFeaturesKHR::events not supported";
        }
    }
    RETURN_IF_SKIP(InitState(nullptr, &sync2_features));

    VkEventCreateInfo event_ci = vku::InitStructHelper();
    event_ci.flags = VK_EVENT_CREATE_DEVICE_ONLY_BIT;
    vkt::Event ev(*m_device, event_ci);

    m_errorMonitor->SetDesiredError("VUID-vkResetEvent-event-03823");
    vk::ResetEvent(*m_device, ev.handle());
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkSetEvent-event-03941");
    vk::SetEvent(*m_device, ev.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, SetEvent2DependencyFlags) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    m_command_buffer.Begin();

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    vkt::Event event(*m_device);

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetEvent2-dependencyFlags-03825");
    vk::CmdSetEvent2(m_command_buffer.handle(), event.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, SetEvent2HostStage) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    m_command_buffer.Begin();

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &barrier;

    vkt::Event event(*m_device);

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetEvent2-srcStageMask-09391");  // src
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetEvent2-dstStageMask-09392");  // dst
    vk::CmdSetEvent2(m_command_buffer.handle(), event.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, SetEvent2HostStageKHR) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    m_command_buffer.Begin();

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &barrier;

    vkt::Event event(*m_device);

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetEvent2-srcStageMask-09391");  // src
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetEvent2-dstStageMask-09392");  // dst
    vk::CmdSetEvent2KHR(m_command_buffer.handle(), event.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, WaitEventRenderPassHostBit) {
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        GTEST_SKIP() << "VkPhysicalDevicePortabilitySubsetFeaturesKHR::events not supported";
    }

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vkt::Event event(*m_device);

    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-07308");
    vk::CmdWaitEvents(m_command_buffer.handle(), 1, &event.handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                      nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, StageMaskHost) {
    TEST_DESCRIPTION("Test invalid usage of VK_PIPELINE_STAGE_HOST_BIT.");
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);
    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetEvent-stageMask-01149");
    vk::CmdSetEvent(m_command_buffer.handle(), event.handle(), VK_PIPELINE_STAGE_HOST_BIT);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdResetEvent-stageMask-01153");
    vk::CmdResetEvent(m_command_buffer.handle(), event.handle(), VK_PIPELINE_STAGE_HOST_BIT);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();

    vkt::Semaphore semaphore(*m_device);
    // Signal the semaphore so we can wait on it.
    m_default_queue->Submit(vkt::no_cmd, vkt::Signal(semaphore));

    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-pWaitDstStageMask-00078");
    m_default_queue->Submit(vkt::no_cmd, vkt::Wait(semaphore, VK_PIPELINE_STAGE_HOST_BIT));
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

TEST_F(NegativeSyncObject, ResetEventThenSet) {
    TEST_DESCRIPTION("Reset an event then set it after the reset has been submitted.");
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    vk::CmdResetEvent(m_command_buffer.handle(), event.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);

    m_errorMonitor->SetDesiredError("VUID-vkSetEvent-event-09543");
    vk::SetEvent(device(), event.handle());
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

// This test should only be used for manual inspection
// Because a command buffer with vkCmdWaitEvents is submitted with an
// event that is never signaled, the test results in a VK_ERROR_DEVICE_LOST
TEST_F(NegativeSyncObject, DISABLED_WaitEventThenSet) {
#if defined(VVL_ENABLE_TSAN)
    // NOTE: This test in particular has failed sporadically on CI when TSAN is enabled.
    GTEST_SKIP() << "https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5965";
#endif
    TEST_DESCRIPTION("Wait on a event then set it after the wait has been submitted.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework());
    void *pNext = nullptr;
    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_subset_features = vku::InitStructHelper();
    if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        pNext = &portability_subset_features;
        GetPhysicalDeviceFeatures2(portability_subset_features);
        if (!portability_subset_features.events) {
            GTEST_SKIP() << "VkPhysicalDevicePortabilitySubsetFeaturesKHR::events not supported";
        }
    }
    RETURN_IF_SKIP(InitState(nullptr, pNext));

    vkt::Event event(*m_device);

    m_command_buffer.Begin();
    vk::CmdWaitEvents(m_command_buffer.handle(), 1, &event.handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                      0, nullptr, 0, nullptr, 0, nullptr);
    vk::CmdResetEvent(m_command_buffer.handle(), event.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);

    m_errorMonitor->SetDesiredError("VUID-vkSetEvent-event-09543");
    vk::SetEvent(device(), event.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, RenderPassPipelineBarrierGraphicsStage) {
    TEST_DESCRIPTION("Use non-graphics pipeline stage inside a renderpass");
    RETURN_IF_SKIP(Init());

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});
    rp.AddColorAttachment(0);
    rp.AddSubpassDependency();
    rp.CreateRenderPass();

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::ImageView view = image.CreateView();
    vkt::Framebuffer fb(*m_device, rp.Handle(), 1, &view.handle());

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp.Handle(), fb.handle());
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-None-07889");
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-None-07889");
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-None-07892");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
                           0, nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, MemoryBarrierStageNotSupportedByQueue) {
    TEST_DESCRIPTION("Memory barrier uses pipeline stages not supported by the queue family");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    std::optional<uint32_t> transfer_only_family = m_device->TransferOnlyQueueFamily();
    if (!transfer_only_family.has_value()) {
        GTEST_SKIP() << "Transfer-only queue family is required";
    }
    vkt::CommandPool transfer_pool(*m_device, transfer_only_family.value());
    vkt::CommandBuffer transfer_cb(*m_device, transfer_pool);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkMemoryBarrier2 barrier_src_gfx = vku::InitStructHelper();
    barrier_src_gfx.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;  // graphics stage
    barrier_src_gfx.srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    barrier_src_gfx.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    barrier_src_gfx.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;

    VkMemoryBarrier2 barrier_dst_gfx = vku::InitStructHelper();
    barrier_dst_gfx.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    barrier_dst_gfx.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier_dst_gfx.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;  // graphics stage
    barrier_dst_gfx.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;

    VkDependencyInfo dep_info_src_gfx = vku::InitStructHelper();
    dep_info_src_gfx.memoryBarrierCount = 1;
    dep_info_src_gfx.pMemoryBarriers = &barrier_src_gfx;

    VkDependencyInfo dep_info_dst_gfx = vku::InitStructHelper();
    dep_info_dst_gfx.memoryBarrierCount = 1;
    dep_info_dst_gfx.pMemoryBarriers = &barrier_dst_gfx;

    transfer_cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier2-srcStageMask-09673");
    vk::CmdPipelineBarrier2(transfer_cb.handle(), &dep_info_src_gfx);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier2-dstStageMask-09674");
    vk::CmdPipelineBarrier2(transfer_cb.handle(), &dep_info_dst_gfx);
    m_errorMonitor->VerifyFound();
    transfer_cb.End();
}

TEST_F(NegativeSyncObject, BufferBarrierStageNotSupportedByQueue) {
    TEST_DESCRIPTION("Buffer memory barrier without ownership transfer uses pipeline stages not supported by the queue family");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    std::optional<uint32_t> compute_only_family = m_device->ComputeOnlyQueueFamily();
    if (!compute_only_family.has_value()) {
        GTEST_SKIP() << "Compute-only queue family is required";
    }
    vkt::CommandPool compute_pool(*m_device, compute_only_family.value());
    vkt::CommandBuffer compute_cb(*m_device, compute_pool);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferMemoryBarrier2 barrier_src_gfx = vku::InitStructHelper();
    barrier_src_gfx.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;  // graphics stage
    barrier_src_gfx.srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    barrier_src_gfx.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    barrier_src_gfx.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier_src_gfx.buffer = buffer.handle();
    barrier_src_gfx.offset = 0;
    barrier_src_gfx.size = 256;

    VkBufferMemoryBarrier2 barrier_dst_gfx = vku::InitStructHelper();
    barrier_dst_gfx.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    barrier_dst_gfx.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier_dst_gfx.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;  // graphics stage
    barrier_dst_gfx.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    barrier_dst_gfx.buffer = buffer.handle();
    barrier_dst_gfx.offset = 0;
    barrier_dst_gfx.size = 256;

    VkDependencyInfo dep_info_src_gfx = vku::InitStructHelper();
    dep_info_src_gfx.bufferMemoryBarrierCount = 1;
    dep_info_src_gfx.pBufferMemoryBarriers = &barrier_src_gfx;

    VkDependencyInfo dep_info_dst_gfx = vku::InitStructHelper();
    dep_info_dst_gfx.bufferMemoryBarrierCount = 1;
    dep_info_dst_gfx.pBufferMemoryBarriers = &barrier_dst_gfx;

    compute_cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier2-srcStageMask-09675");
    vk::CmdPipelineBarrier2(compute_cb.handle(), &dep_info_src_gfx);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier2-dstStageMask-09676");
    vk::CmdPipelineBarrier2(compute_cb.handle(), &dep_info_dst_gfx);
    m_errorMonitor->VerifyFound();
    compute_cb.End();
}

TEST_F(NegativeSyncObject, BufferOwnershipTransferStageNotSupportedByQueue) {
    TEST_DESCRIPTION("Buffer memory barrier with ownership transfer uses pipeline stages not supported by the queue family");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    // Enable feature to use stage other than ALL_COMMANDS during ownership transfer
    AddRequiredExtensions(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance8);
    RETURN_IF_SKIP(Init());

    std::optional<uint32_t> transfer_only_family = m_device->TransferOnlyQueueFamily();
    if (!transfer_only_family.has_value()) {
        GTEST_SKIP() << "Transfer-only queue family is required";
    }
    vkt::CommandPool transfer_pool(*m_device, transfer_only_family.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer transfer_cb(*m_device, transfer_pool);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    // Acquire operation on transfer queue.
    // The src stage should be a valid transfer stage.
    VkBufferMemoryBarrier2 acquire_barrier = vku::InitStructHelper();
    acquire_barrier.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;  // Not a valid transfer stage
    acquire_barrier.srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    acquire_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    acquire_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    acquire_barrier.srcQueueFamilyIndex = m_default_queue->family_index;
    acquire_barrier.dstQueueFamilyIndex = transfer_only_family.value();
    acquire_barrier.buffer = buffer;
    acquire_barrier.offset = 0;
    acquire_barrier.size = 256;

    VkDependencyInfo acquire_dep_info = vku::InitStructHelper();
    // Use this dependency flag to be able to use src stage other then ALL_COMMAND
    acquire_dep_info.dependencyFlags = VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR;
    acquire_dep_info.bufferMemoryBarrierCount = 1;
    acquire_dep_info.pBufferMemoryBarriers = &acquire_barrier;

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier2-srcStageMask-09675");
    transfer_cb.Begin();
    vk::CmdPipelineBarrier2(transfer_cb, &acquire_dep_info);
    transfer_cb.End();
    m_errorMonitor->VerifyFound();

    // Release operation on transfer queue.
    // The dst stage should be a valid transfer stage.
    VkBufferMemoryBarrier2 release_barrier = vku::InitStructHelper();
    release_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    release_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    release_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;  // Not valid transfer stage
    release_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    release_barrier.srcQueueFamilyIndex = transfer_only_family.value();
    release_barrier.dstQueueFamilyIndex = m_default_queue->family_index;
    release_barrier.buffer = buffer;
    release_barrier.offset = 0;
    release_barrier.size = 256;

    VkDependencyInfo release_dep_info = vku::InitStructHelper();
    // Use this dependency flag to be able to use dst stage other then ALL_COMMAND
    release_dep_info.dependencyFlags = VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR;
    release_dep_info.bufferMemoryBarrierCount = 1;
    release_dep_info.pBufferMemoryBarriers = &release_barrier;

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier2-dstStageMask-09676");
    transfer_cb.Begin();
    vk::CmdPipelineBarrier2(transfer_cb, &release_dep_info);
    transfer_cb.End();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, BarrierOwnershipTransferUseAllStages) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    // Enable extension but do not enable maintenance8 feature
    AddRequiredExtensions(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferMemoryBarrier barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.buffer = buffer;
    barrier.offset = 0;
    barrier.size = 256;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-maintenance8-10206");
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR, 0, nullptr, 1, &barrier, 0,
                           nullptr);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, EventOwnershipTransferUseAllStages) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    // Enable extension but do not enable maintenance8 feature
    AddRequiredExtensions(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.dependencyFlags = VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents2-maintenance8-10205");
    vk::CmdWaitEvents2(m_command_buffer.handle(), 1, &event.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, ImageBarrierStageNotSupportedByQueue) {
    TEST_DESCRIPTION("Image memory barrier without ownership transfer uses pipeline stages not supported by the queue family");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    std::optional<uint32_t> compute_only_family = m_device->ComputeOnlyQueueFamily();
    if (!compute_only_family.has_value()) {
        GTEST_SKIP() << "Compute-only queue family is required";
    }
    vkt::CommandPool compute_pool(*m_device, compute_only_family.value());
    vkt::CommandBuffer compute_cb(*m_device, compute_pool);

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    VkImageMemoryBarrier2 barrier_src_gfx = vku::InitStructHelper();
    barrier_src_gfx.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;  // graphics stage
    barrier_src_gfx.srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    barrier_src_gfx.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    barrier_src_gfx.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier_src_gfx.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier_src_gfx.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier_src_gfx.image = image.handle();
    barrier_src_gfx.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkImageMemoryBarrier2 barrier_dst_gfx = vku::InitStructHelper();
    barrier_dst_gfx.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    barrier_dst_gfx.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier_dst_gfx.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;  // graphics stage
    barrier_dst_gfx.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    barrier_dst_gfx.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier_dst_gfx.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier_dst_gfx.image = image.handle();
    barrier_dst_gfx.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkDependencyInfo dep_info_src_gfx = vku::InitStructHelper();
    dep_info_src_gfx.imageMemoryBarrierCount = 1;
    dep_info_src_gfx.pImageMemoryBarriers = &barrier_src_gfx;

    VkDependencyInfo dep_info_dst_gfx = vku::InitStructHelper();
    dep_info_dst_gfx.imageMemoryBarrierCount = 1;
    dep_info_dst_gfx.pImageMemoryBarriers = &barrier_dst_gfx;

    compute_cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier2-srcStageMask-09675");
    vk::CmdPipelineBarrier2(compute_cb.handle(), &dep_info_src_gfx);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier2-dstStageMask-09676");
    vk::CmdPipelineBarrier2(compute_cb.handle(), &dep_info_dst_gfx);
    m_errorMonitor->VerifyFound();
    compute_cb.End();
}

TEST_F(NegativeSyncObject, TimelineHostSignalAndInUseTracking) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8476");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    VkSemaphore handle = semaphore.handle();

    m_default_queue->Submit(vkt::no_cmd, vkt::TimelineWait(semaphore, 1));
    semaphore.Signal(1);  // signal should not initiate forward progress on the queue thread

    // In the case of regression, this delay gives the queue thread additional time to mark
    // the semaphore as not in use which will fail the following check. If the queue thread
    // was not fast enough the test will pass without detecting regression (false-negative).
    std::this_thread::sleep_for(std::chrono::milliseconds{50});

    m_errorMonitor->SetDesiredError("VUID-vkDestroySemaphore-semaphore-05149");
    semaphore.destroy();
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
    vk::DestroySemaphore(*m_device, handle, nullptr);
}

TEST_F(NegativeSyncObject, TimelineSubmitSignalAndInUseTracking) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8370");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    all_queue_count_ = true;
    RETURN_IF_SKIP(Init());

    if (!m_second_queue) {
        GTEST_SKIP() << "2 queues are needed";
    }

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    VkSemaphore handle = semaphore.handle();

    m_default_queue->Submit(vkt::no_cmd, vkt::TimelineWait(semaphore, 1));
    m_second_queue->Submit(vkt::no_cmd, vkt::TimelineSignal(semaphore, 1));
    // Waiting for the second (signaling) queue should not initiate queue thread forward
    // progress on the default (waiting) queue.
    m_second_queue->Wait();

    // In the case of regression, this delay gives the queue thread additional time to mark
    // the semaphore as not in use which will fail the following check. If the queue thread
    // was not fast enough the test will pass without detecting regression (false-negative).
    std::this_thread::sleep_for(std::chrono::milliseconds{50});

    m_errorMonitor->SetDesiredError("VUID-vkDestroySemaphore-semaphore-05149");
    semaphore.destroy();
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
    vk::DestroySemaphore(*m_device, handle, nullptr);
}

TEST_F(NegativeSyncObject, TimelineCannotFixBinaryWaitBeforeSignal) {
    TEST_DESCRIPTION("Binary signal should be submitted before binary wait. Timeline can't help with ordering");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }

    vkt::Semaphore timeline_semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    vkt::Semaphore binary_semaphore(*m_device);

    // Although timeline wait postpones binary wait, the specification still does not
    // allow to submit binary wait before binary signal
    // https://gitlab.khronos.org/vulkan/vulkan/-/issues/4046
    m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineWait(timeline_semaphore, 1));
    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit2-semaphore-03873");
    m_default_queue->Submit2(vkt::no_cmd, vkt::Wait(binary_semaphore));
    m_errorMonitor->VerifyFound();

    m_second_queue->Submit2(vkt::no_cmd, vkt::Signal(binary_semaphore));
    m_second_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(timeline_semaphore, 1));

    m_default_queue->Wait();
}

TEST_F(NegativeSyncObject, DecreasingTimelineSignals) {
    TEST_DESCRIPTION("Signal timeline value smaller than previous signal");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);

    m_default_queue->Submit(vkt::no_cmd, vkt::TimelineSignal(semaphore, 2));

    // NOTE: VerifyFound goes after Wait because validation is performed by the Queue thread
    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-pSignalSemaphores-03242");
    m_default_queue->Submit(vkt::no_cmd, vkt::TimelineSignal(semaphore, 1));
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, DifferentSignalingOrderThanSubmitOrder) {
    TEST_DESCRIPTION("Timeline values are increasing in submit order but reordered by wait-before-signal at runtime");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    if (!m_second_queue) {
        GTEST_SKIP() << "2 queues are needed";
    }

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);

    m_default_queue->Submit(vkt::no_cmd, vkt::TimelineWait(semaphore, 1));
    m_default_queue->Submit(vkt::no_cmd, vkt::TimelineSignal(semaphore, 2));

    // Signal 3 resolves wait 1 then value 2 is signaled
    // NOTE: VerifyFound goes after Wait because validation is performed by the Queue thread
    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-pSignalSemaphores-03242");
    m_second_queue->Submit(vkt::no_cmd, vkt::TimelineSignal(semaphore, 3));
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, DifferentSignalingOrderThanSubmitOrder2) {
    TEST_DESCRIPTION("Timeline values are increasing in submit order but reordered by wait-before-signal at runtime");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    if (!m_second_queue) {
        GTEST_SKIP() << "2 queues are needed";
    }

    vkt::Semaphore semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);

    m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineWait(semaphore, 1));
    m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(semaphore, 1));

    // Signal 3 resolves wait 1 then value 1 is signaled
    // NOTE: VerifyFound goes after Wait because validation is performed by the Queue thread
    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo2-semaphore-03882");
    m_second_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(semaphore, 3));
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSyncObject, BinarySyncDependsOnTimelineWait) {
    TEST_DESCRIPTION("Binary semaphore signal->wait after timeline wait-before-signal");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }

    vkt::Semaphore timeline_semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    vkt::Semaphore binary_semaphore(*m_device);

    m_default_queue->Submit(vkt::no_cmd, vkt::TimelineWait(timeline_semaphore, 1));
    m_default_queue->Submit(vkt::no_cmd, vkt::Signal(binary_semaphore));

    // There is a matching binary signal for this wait, but that signal depends on another not yet submitted timeline signal
    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pWaitSemaphores-03238");
    m_default_queue->Submit(vkt::no_cmd, vkt::Wait(binary_semaphore));
    m_errorMonitor->VerifyFound();

    m_second_queue->Submit(vkt::no_cmd, vkt::TimelineSignal(timeline_semaphore, 1));
    m_device->Wait();
}

TEST_F(NegativeSyncObject, BinarySyncDependsOnTimelineWait2) {
    TEST_DESCRIPTION("Binary semaphore signal-wait depends on timeline wait-before-signal");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }

    vkt::Semaphore timeline_semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    vkt::Semaphore binary_semaphore(*m_device);

    m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineWait(timeline_semaphore, 1), vkt::Signal(binary_semaphore));

    // There is a matching binary signal for this wait, but that signal depends on another not yet submitted timeline signal
    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit2-semaphore-03873");
    m_default_queue->Submit2(vkt::no_cmd, vkt::Wait(binary_semaphore));
    m_errorMonitor->VerifyFound();

    m_second_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(timeline_semaphore, 1));
    m_device->Wait();
}

TEST_F(NegativeSyncObject, BinarySyncDependsOnTimelineWait3) {
    TEST_DESCRIPTION("Binary semaphore signal-wait depends on timeline wait-before-signal");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }

    vkt::Semaphore timeline_semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    vkt::Semaphore timeline_semaphore2(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    vkt::Semaphore binary_semaphore(*m_device);

    m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineWait(timeline_semaphore, 1), vkt::TimelineSignal(timeline_semaphore2, 1));

    m_default_queue->Submit2(vkt::no_cmd, vkt::TimelineWait(timeline_semaphore2, 1), vkt::Signal(binary_semaphore));

    // There is a matching binary signal for this wait, but that signal depends on another not yet submitted timeline signal
    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit2-semaphore-03873");
    m_default_queue->Submit2(vkt::no_cmd, vkt::Wait(binary_semaphore));
    m_errorMonitor->VerifyFound();

    m_second_queue->Submit2(vkt::no_cmd, vkt::TimelineSignal(timeline_semaphore, 1));
    m_device->Wait();
}

TEST_F(NegativeSyncObject, BinarySyncDependsOnTimelineWait4) {
    TEST_DESCRIPTION("Binary semaphore signal-wait depends on timeline wait-before-signal. Use sparse binding operation");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }
    if (!(m_default_queue_caps & VK_QUEUE_SPARSE_BINDING_BIT)) {
        GTEST_SKIP() << "Graphics queue does not have sparse binding bit";
    }

    vkt::Semaphore timeline_semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    vkt::Semaphore binary_semaphore(*m_device);

    m_default_queue->Submit(vkt::no_cmd, vkt::TimelineWait(timeline_semaphore, 1));
    m_default_queue->Submit(vkt::no_cmd, vkt::Signal(binary_semaphore));

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.waitSemaphoreCount = 1;
    bind_info.pWaitSemaphores = &binary_semaphore.handle();

    m_errorMonitor->SetDesiredError("VUID-vkQueueBindSparse-pWaitSemaphores-03245");
    vk::QueueBindSparse(*m_default_queue, 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    m_second_queue->Submit(vkt::no_cmd, vkt::TimelineSignal(timeline_semaphore, 1));
    m_device->Wait();
}

TEST_F(NegativeSyncObject, BinarySyncDependsOnTimelineWait5) {
    TEST_DESCRIPTION("Binary semaphore signal-wait depends on timeline wait-before-signal. Use sparse binding operation");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }
    if (!(m_default_queue_caps & VK_QUEUE_SPARSE_BINDING_BIT)) {
        GTEST_SKIP() << "Graphics queue does not have sparse binding bit";
    }

    vkt::Semaphore timeline_semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    vkt::Semaphore binary_semaphore(*m_device);

    // Submit timeline signal using sparse submit
    const uint64_t timeline_value = 1;
    VkTimelineSemaphoreSubmitInfo timeline_submit_info = vku::InitStructHelper();
    timeline_submit_info.waitSemaphoreValueCount = 1;
    timeline_submit_info.pWaitSemaphoreValues = &timeline_value;
    VkBindSparseInfo bind_info = vku::InitStructHelper(&timeline_submit_info);
    bind_info.waitSemaphoreCount = 1;
    bind_info.pWaitSemaphores = &timeline_semaphore.handle();
    vk::QueueBindSparse(*m_default_queue, 1, &bind_info, VK_NULL_HANDLE);

    m_default_queue->Submit2(vkt::no_cmd, vkt::Signal(binary_semaphore));

    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit2-semaphore-03873");
    m_default_queue->Submit2(vkt::no_cmd, vkt::Wait(binary_semaphore));
    m_errorMonitor->VerifyFound();

    m_second_queue->Submit(vkt::no_cmd, vkt::TimelineSignal(timeline_semaphore, 1));
    m_device->Wait();
}

TEST_F(NegativeSyncObject, CmdWaitEvents2KHRUsedButSynchronizaion2Disabled) {
    TEST_DESCRIPTION("Using CmdWaitEvents2KHR when synchronization2 is not enabled");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::Event event(*m_device);
    VkEvent event_handle = event.handle();
    VkDependencyInfo dependency_info = vku::InitStructHelper();

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents2-synchronization2-03836");
    vk::CmdWaitEvents2KHR(m_command_buffer.handle(), 1, &event_handle, &dependency_info);
    m_errorMonitor->VerifyFound();

    if (DeviceValidationVersion() >= VK_API_VERSION_1_3) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents2-synchronization2-03836");
        vk::CmdWaitEvents2(m_command_buffer.handle(), 1, &event_handle, &dependency_info);
        m_errorMonitor->VerifyFound();
    }
    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, Sync2FeatureDisabled) {
    TEST_DESCRIPTION("Call sync2 functions when the feature is disabled");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const bool vulkan_13 = (DeviceValidationVersion() >= VK_API_VERSION_1_3);
    bool timestamp = false;

    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_count, NULL);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queue_count, queue_props.data());
    if (queue_props[m_device->graphics_queue_node_index_].timestampValidBits > 0) {
        timestamp = true;
    }

    m_command_buffer.Begin();

    VkDependencyInfo dependency_info = vku::InitStructHelper();

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier2-synchronization2-03848");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    vkt::Event event(*m_device);

    VkPipelineStageFlagBits2 stage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;

    m_errorMonitor->SetDesiredError("VUID-vkCmdResetEvent2-synchronization2-03829");
    vk::CmdResetEvent2KHR(m_command_buffer.handle(), event.handle(), stage);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetEvent2-synchronization2-03824");
    vk::CmdSetEvent2KHR(m_command_buffer.handle(), event.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    if (timestamp) {
        vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_TIMESTAMP, 1);

        m_errorMonitor->SetDesiredError("VUID-vkCmdWriteTimestamp2-synchronization2-03858");
        vk::CmdWriteTimestamp2KHR(m_command_buffer.handle(), stage, query_pool.handle(), 0);
        m_errorMonitor->VerifyFound();
        if (vulkan_13) {
            m_errorMonitor->SetDesiredError("VUID-vkCmdWriteTimestamp2-synchronization2-03858");
            vk::CmdWriteTimestamp2(m_command_buffer.handle(), stage, query_pool.handle(), 0);
            m_errorMonitor->VerifyFound();
        }
    }
    if (vulkan_13) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier2-synchronization2-03848");
        vk::CmdPipelineBarrier2(m_command_buffer.handle(), &dependency_info);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdResetEvent2-synchronization2-03829");
        vk::CmdResetEvent2(m_command_buffer.handle(), event.handle(), stage);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdSetEvent2-synchronization2-03824");
        vk::CmdSetEvent2(m_command_buffer.handle(), event.handle(), &dependency_info);
        m_errorMonitor->VerifyFound();
    }

    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, BufferMemoryBarrierUnbound) {
    RETURN_IF_SKIP(Init());

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.flags = 0u;
    buffer_ci.size = 256u;
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer(*m_device, buffer_ci, vkt::no_mem);

    m_command_buffer.Begin();
    VkBufferMemoryBarrier bmb = vku::InitStructHelper();
    bmb.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    bmb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    bmb.buffer = buffer.handle();
    bmb.offset = 0;
    bmb.size = VK_WHOLE_SIZE;
    m_errorMonitor->SetDesiredError("VUID-VkBufferMemoryBarrier-buffer-01931");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0u, 0u,
                           nullptr, 1u, &bmb, 0u, nullptr);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, BufferMemoryBarrierQueueFamilyExternal) {
    RETURN_IF_SKIP(Init());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed to run this test";
    }

    uint32_t qfi[2] = {m_default_queue->family_index, m_second_queue->family_index};

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.flags = 0u;
    buffer_ci.size = 256u;
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_ci.sharingMode = VK_SHARING_MODE_CONCURRENT;
    buffer_ci.queueFamilyIndexCount = 2u;
    buffer_ci.pQueueFamilyIndices = qfi;
    vkt::Buffer buffer(*m_device, buffer_ci, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    m_command_buffer.Begin();
    VkBufferMemoryBarrier bmb = vku::InitStructHelper();
    bmb.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    bmb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    bmb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
    bmb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bmb.buffer = buffer.handle();
    bmb.offset = 0;
    bmb.size = VK_WHOLE_SIZE;
    m_errorMonitor->SetDesiredError("VUID-VkBufferMemoryBarrier-None-09097");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0u, 0u,
                           nullptr, 1u, &bmb, 0u, nullptr);
    m_errorMonitor->VerifyFound();

    bmb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bmb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
    m_errorMonitor->SetDesiredError("VUID-VkBufferMemoryBarrier-None-09098");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0u, 0u,
                           nullptr, 1u, &bmb, 0u, nullptr);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, ImageMemoryBarrier2QueueFamilyExternal) {
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed to run this test";
    }

    uint32_t queue_families[2] = {m_default_queue->family_index, m_second_queue->family_index};

    VkImageCreateInfo image_ci = vkt::Image::ImageCreateInfo2D(32u, 32u, 1u, 1u, VK_FORMAT_B8G8R8A8_UNORM,
                                                               VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                                               VK_IMAGE_TILING_OPTIMAL, vvl::make_span(queue_families, 2u));
    vkt::Image image(*m_device, image_ci);

    m_command_buffer.Begin();
    VkImageMemoryBarrier2 imb = vku::InitStructHelper();
    imb.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR;
    imb.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    imb.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    imb.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    imb.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imb.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
    imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imb.image = image.handle();
    imb.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.imageMemoryBarrierCount = 1u;
    dependency_info.pImageMemoryBarriers = &imb;

    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier2-None-09119");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier2-None-09120");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, BufferMemoryBarrierQueueFamilyForeign) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.flags = 0u;
    buffer_ci.size = 256u;
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer(*m_device, buffer_ci, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    m_command_buffer.Begin();
    VkBufferMemoryBarrier bmb = vku::InitStructHelper();
    bmb.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    bmb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    bmb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_FOREIGN_EXT;
    bmb.dstQueueFamilyIndex = m_default_queue->family_index;
    bmb.buffer = buffer.handle();
    bmb.offset = 0;
    bmb.size = VK_WHOLE_SIZE;

    m_errorMonitor->SetDesiredError("VUID-VkBufferMemoryBarrier-srcQueueFamilyIndex-09099");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0u, 0u,
                           nullptr, 1u, &bmb, 0u, nullptr);
    m_errorMonitor->VerifyFound();

    bmb.srcQueueFamilyIndex = m_default_queue->family_index;
    bmb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_FOREIGN_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkBufferMemoryBarrier-dstQueueFamilyIndex-09100");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0u, 0u,
                           nullptr, 1u, &bmb, 0u, nullptr);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, UnsupportedBarrierAccessMaskConditionalRendering) {
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredExtensions(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::conditionalRendering);
    RETURN_IF_SKIP(Init());

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = 0u;
    barrier.srcAccessMask = 0u;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1u;
    dependency_info.pMemoryBarriers = &barrier;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-03918");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, UnsupportedBarrierAccessMaskFragmentDensityMap) {
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::fragmentDensityMap);
    RETURN_IF_SKIP(Init());

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = 0u;
    barrier.srcAccessMask = 0u;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_FRAGMENT_DENSITY_MAP_READ_BIT_EXT;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1u;
    dependency_info.pMemoryBarriers = &barrier;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-03919");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, UnsupportedBarrierAccessMaskXfb) {
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    RETURN_IF_SKIP(Init());

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = 0u;
    barrier.srcAccessMask = 0u;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1u;
    dependency_info.pMemoryBarriers = &barrier;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-03920");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    barrier.dstAccessMask = VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-04747");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, UnsupportedBarrierAccessMaskBlendOperationAdvanced) {
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredExtensions(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::advancedBlendCoherentOperations);
    RETURN_IF_SKIP(Init());

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = 0u;
    barrier.srcAccessMask = 0u;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1u;
    dependency_info.pMemoryBarriers = &barrier;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkMemoryBarrier2-dstAccessMask-03926");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, ImageMemoryBarrier2QueueFamilyForeign) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_ci =
        vkt::Image::ImageCreateInfo2D(32u, 32u, 1u, 1u, VK_FORMAT_B8G8R8A8_UNORM,
                                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL);
    vkt::Image image(*m_device, image_ci);

    m_command_buffer.Begin();
    VkImageMemoryBarrier2 imb = vku::InitStructHelper();
    imb.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR;
    imb.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    imb.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    imb.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    imb.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imb.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_FOREIGN_EXT;
    imb.dstQueueFamilyIndex = m_default_queue->family_index;
    imb.image = image.handle();
    imb.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.imageMemoryBarrierCount = 1u;
    dependency_info.pImageMemoryBarriers = &imb;

    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-09121");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    imb.srcQueueFamilyIndex = m_default_queue->family_index;
    imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_FOREIGN_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryBarrier2-dstQueueFamilyIndex-09122");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, UnsupportedPipelineBarrierStages) {
    AddOptionalExtensions(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-dstStageMask-04091");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                           VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT, 0u, 0u, nullptr, 0u, nullptr, 0u, nullptr);
    m_errorMonitor->VerifyFound();

    if (IsExtensionsEnabled(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME)) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-dstStageMask-04092");
        vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT, 0u, 0u, nullptr, 0u, nullptr, 0u, nullptr);
        m_errorMonitor->VerifyFound();
    }

    if (IsExtensionsEnabled(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME)) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-dstStageMask-04093");
        vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT, 0u, 0u, nullptr, 0u, nullptr, 0u, nullptr);
        m_errorMonitor->VerifyFound();
    }

    if (IsExtensionsEnabled(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME)) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-dstStageMask-04094");
        vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT, 0u, 0u, nullptr, 0u, nullptr, 0u, nullptr);
        m_errorMonitor->VerifyFound();
    }

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-dstStageMask-03937");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0u, 0u, 0u, nullptr, 0u, nullptr, 0u,
                           nullptr);
    m_errorMonitor->VerifyFound();

    if (IsExtensionsEnabled(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-dstStageMask-07318");
        vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, 0u, 0u, nullptr, 0u, nullptr, 0u,
                               nullptr);
        m_errorMonitor->VerifyFound();
    }

    m_command_buffer.End();
}

TEST_F(NegativeSyncObject, UnsupportedBufferMemoryBarrier2Stages) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredExtensions(VK_HUAWEI_SUBPASS_SHADING_EXTENSION_NAME);
    AddRequiredExtensions(VK_HUAWEI_INVOCATION_MASK_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    VkBufferMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.buffer = buffer.handle();
    barrier.size = VK_WHOLE_SIZE;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_SUBPASS_SHADER_BIT_HUAWEI;

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.bufferMemoryBarrierCount = 1u;
    dependency_info.pBufferMemoryBarriers = &barrier;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkBufferMemoryBarrier2-dstStageMask-04957");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    barrier.dstStageMask = VK_PIPELINE_STAGE_2_INVOCATION_MASK_BIT_HUAWEI;
    m_errorMonitor->SetDesiredError("VUID-VkBufferMemoryBarrier2-dstStageMask-04995");
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}
