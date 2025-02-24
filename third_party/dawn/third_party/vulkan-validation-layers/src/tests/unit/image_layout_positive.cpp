/*
 * Copyright (c) 2024-2025 Valve Corporation
 * Copyright (c) 2024-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/render_pass_helper.h"

class PositiveImageLayout : public ImageTest {};

TEST_F(PositiveImageLayout, BarriersAndImageUsage) {
    TEST_DESCRIPTION("Ensure barriers' new and old VkImageLayout are compatible with their images' VkImageUsageFlags");

    RETURN_IF_SKIP(Init());
    auto depth_format = FindSupportedDepthStencilFormat(Gpu());
    InitRenderTarget();

    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    {
        vkt::Image img_color(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        vkt::Image img_ds1(*m_device, 128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        vkt::Image img_ds2(*m_device, 128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        vkt::Image img_xfer_src(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        vkt::Image img_xfer_dst(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
        vkt::Image img_sampled(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
        vkt::Image img_input(*m_device, 128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
        const struct {
            vkt::Image &image_obj;
            VkImageLayout old_layout;
            VkImageLayout new_layout;
        } buffer_layouts[] = {
            // clang-format off
            {img_color,    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         VK_IMAGE_LAYOUT_GENERAL},
            {img_ds1,      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL},
            {img_ds2,      VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  VK_IMAGE_LAYOUT_GENERAL},
            {img_sampled,  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         VK_IMAGE_LAYOUT_GENERAL},
            {img_input,    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         VK_IMAGE_LAYOUT_GENERAL},
            {img_xfer_src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             VK_IMAGE_LAYOUT_GENERAL},
            {img_xfer_dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             VK_IMAGE_LAYOUT_GENERAL},
            // clang-format on
        };
        const uint32_t layout_count = sizeof(buffer_layouts) / sizeof(buffer_layouts[0]);

        m_command_buffer.Begin();
        for (uint32_t i = 0; i < layout_count; ++i) {
            img_barrier.image = buffer_layouts[i].image_obj.handle();
            const VkImageUsageFlags usage = buffer_layouts[i].image_obj.Usage();
            img_barrier.subresourceRange.aspectMask = (usage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                                                          ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)
                                                          : VK_IMAGE_ASPECT_COLOR_BIT;

            img_barrier.oldLayout = buffer_layouts[i].old_layout;
            img_barrier.newLayout = buffer_layouts[i].new_layout;
            vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0,
                                   nullptr, 0, nullptr, 1, &img_barrier);

            img_barrier.oldLayout = buffer_layouts[i].new_layout;
            img_barrier.newLayout = buffer_layouts[i].old_layout;
            vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0,
                                   nullptr, 0, nullptr, 1, &img_barrier);
        }
        m_command_buffer.End();

        img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    }
}

TEST_F(PositiveImageLayout, ImagelessTracking) {
    TEST_DESCRIPTION("Test layout tracking on imageless framebuffers");
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDeviceImagelessFramebufferFeaturesKHR physicalDeviceImagelessFramebufferFeatures = vku::InitStructHelper();
    physicalDeviceImagelessFramebufferFeatures.imagelessFramebuffer = VK_TRUE;
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = vku::InitStructHelper(&physicalDeviceImagelessFramebufferFeatures);

    uint32_t physical_device_group_count = 0;
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, nullptr);

    if (physical_device_group_count == 0) {
        GTEST_SKIP() << "physical_device_group_count is 0";
    }
    std::vector<VkPhysicalDeviceGroupProperties> physical_device_group(physical_device_group_count,
                                                                       {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, physical_device_group.data());
    VkDeviceGroupDeviceCreateInfo create_device_pnext = vku::InitStructHelper();
    create_device_pnext.physicalDeviceCount = physical_device_group[0].physicalDeviceCount;
    create_device_pnext.pPhysicalDevices = physical_device_group[0].physicalDevices;
    create_device_pnext.pNext = &physicalDeviceFeatures2;

    RETURN_IF_SKIP(InitState(nullptr, &create_device_pnext));
    RETURN_IF_SKIP(InitSwapchain(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));

    uint32_t attachmentWidth = m_surface_capabilities.minImageExtent.width;
    uint32_t attachmentHeight = m_surface_capabilities.minImageExtent.height;
    VkFormat attachmentFormat = m_surface_formats[0].format;

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(attachmentFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    // Create an image to use in an imageless framebuffer.  Bind swapchain memory to it.
    VkImageSwapchainCreateInfoKHR image_swapchain_create_info = vku::InitStructHelper();
    image_swapchain_create_info.swapchain = m_swapchain;

    auto image_ci = vkt::Image::ImageCreateInfo2D(attachmentWidth, attachmentHeight, 1, 1, attachmentFormat,
                                                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image_ci.pNext = &image_swapchain_create_info;
    vkt::Image image(*m_device, image_ci, vkt::no_mem);

    VkBindImageMemoryDeviceGroupInfo bind_devicegroup_info = vku::InitStructHelper();
    bind_devicegroup_info.deviceIndexCount = physical_device_group[0].physicalDeviceCount;
    std::array<uint32_t, 8> deviceIndices = {{0}};
    bind_devicegroup_info.pDeviceIndices = deviceIndices.data();
    bind_devicegroup_info.splitInstanceBindRegionCount = 0;
    bind_devicegroup_info.pSplitInstanceBindRegions = nullptr;

    VkBindImageMemorySwapchainInfoKHR bind_swapchain_info = vku::InitStructHelper(&bind_devicegroup_info);
    bind_swapchain_info.swapchain = m_swapchain;
    bind_swapchain_info.imageIndex = 0;

    VkBindImageMemoryInfo bind_info = vku::InitStructHelper(&bind_swapchain_info);
    bind_info.image = image.handle();
    bind_info.memory = VK_NULL_HANDLE;
    bind_info.memoryOffset = 0;

    vk::BindImageMemory2(device(), 1, &bind_info);

    const std::vector<VkImage> swapchain_images = m_swapchain.GetImages();

    vkt::Semaphore image_acquired(*m_device);
    const uint32_t current_buffer = m_swapchain.AcquireNextImage(image_acquired, kWaitTimeout);

    vkt::ImageView imageView = image.CreateView();
    VkFramebufferAttachmentImageInfo framebufferAttachmentImageInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO_KHR,
                                                                       nullptr,
                                                                       0,
                                                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                                       attachmentWidth,
                                                                       attachmentHeight,
                                                                       1,
                                                                       1,
                                                                       &attachmentFormat};
    VkFramebufferAttachmentsCreateInfo framebufferAttachmentsCreateInfo = vku::InitStructHelper();
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 1;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = &framebufferAttachmentImageInfo;
    VkFramebufferCreateInfo framebufferCreateInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                                                     &framebufferAttachmentsCreateInfo,
                                                     VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT,
                                                     rp.Handle(),
                                                     1,
                                                     reinterpret_cast<const VkImageView *>(1),
                                                     attachmentWidth,
                                                     attachmentHeight,
                                                     1};
    vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);

    VkRenderPassAttachmentBeginInfo renderPassAttachmentBeginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO_KHR,
                                                                     nullptr, 1, &imageView.handle()};
    VkRenderPassBeginInfo renderPassBeginInfo =
        vku::InitStruct<VkRenderPassBeginInfo>(&renderPassAttachmentBeginInfo, rp.Handle(), framebuffer.handle(),
                                               VkRect2D{{0, 0}, {attachmentWidth, attachmentHeight}}, 0u, nullptr);

    // RenderPass should change the image layout of both the swapchain image and the aliased image to PRESENT_SRC_KHR
    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(renderPassBeginInfo);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    m_default_queue->Present(m_swapchain, current_buffer, image_acquired);
    m_default_queue->Wait();
}

TEST_F(PositiveImageLayout, Subresource) {
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::CreateInfo();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.extent.width = 64;
    image_ci.extent.height = 64;
    image_ci.mipLevels = 7;
    image_ci.arrayLayers = 6;
    image_ci.format = VK_FORMAT_R8_UINT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    vkt::Image image(*m_device, image_ci);

    m_command_buffer.Begin();
    const auto subresource_range = image.SubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
    auto barrier = image.ImageMemoryBarrier(0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, subresource_range);
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &barrier);
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.subresourceRange.baseMipLevel = 1;
    barrier.subresourceRange.levelCount = 1;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &barrier);
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &barrier);
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.subresourceRange.baseMipLevel = 2;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &barrier);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveImageLayout, DescriptorSubresource) {
    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    // Create image, view, and sampler
    const VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
    auto usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 5, format, usage);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);

    VkImageSubresourceRange view_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 3, 1};
    VkImageSubresourceRange first_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkImageSubresourceRange full_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 5};
    VkImageViewCreateInfo image_view_create_info = vku::InitStructHelper();
    image_view_create_info.image = image.handle();
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = format;
    image_view_create_info.subresourceRange = view_range;

    vkt::ImageView view(*m_device, image_view_create_info);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorImageInfo(0, view, sampler);
    descriptor_set.UpdateDescriptorSets();

    // Create PSO to be used for draw-time errors below
    VkShaderObj fs(this, kFragmentSamplerGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    vkt::CommandBuffer cmd_buf(*m_device, m_command_pool);

    enum TestType {
        kInternal,  // Image layout mismatch is *within* a given command buffer
        kExternal   // Image layout mismatch is with the current state of the image, found at QueueSubmit
    };
    std::array<TestType, 2> test_list = {{kInternal, kExternal}};

    for (TestType test_type : test_list) {
        auto init_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        VkImageMemoryBarrier image_barrier = vku::InitStructHelper();

        cmd_buf.Begin();
        image_barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        image_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        image_barrier.image = image.handle();
        image_barrier.subresourceRange = full_range;
        image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_barrier.newLayout = init_layout;

        vk::CmdPipelineBarrier(cmd_buf.handle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 0,
                               nullptr, 0, nullptr, 1, &image_barrier);

        image_barrier.subresourceRange = first_range;
        image_barrier.oldLayout = init_layout;
        image_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vk::CmdPipelineBarrier(cmd_buf.handle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 0,
                               nullptr, 0, nullptr, 1, &image_barrier);

        image_barrier.subresourceRange = view_range;
        image_barrier.oldLayout = init_layout;
        image_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vk::CmdPipelineBarrier(cmd_buf.handle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 0,
                               nullptr, 0, nullptr, 1, &image_barrier);

        if (test_type == kExternal) {
            // The image layout is external to the command buffer we are recording to test.  Submit to push to instance scope.
            cmd_buf.End();
            m_default_queue->Submit(cmd_buf);
            m_default_queue->Wait();
            cmd_buf.Begin();
        }

        cmd_buf.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(cmd_buf.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        vk::CmdBindDescriptorSets(cmd_buf.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                  &descriptor_set.set_, 0, NULL);

        vk::CmdDraw(cmd_buf.handle(), 1, 0, 0, 0);

        cmd_buf.EndRenderPass();
        cmd_buf.End();

        // Submit cmd buffer
        m_default_queue->Submit(cmd_buf);
        m_default_queue->Wait();
    }
}

TEST_F(PositiveImageLayout, Descriptor3D2DSubresource) {
    TEST_DESCRIPTION("Verify renderpass layout transitions for a 2d ImageView created from a 3d Image.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    // Create image, view, and sampler
    const VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
    auto usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    static const uint32_t kWidth = 128;
    static const uint32_t kHeight = 128;

    VkImageCreateInfo image_ci_3d = vku::InitStructHelper();
    image_ci_3d.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    image_ci_3d.imageType = VK_IMAGE_TYPE_3D;
    image_ci_3d.format = format;
    image_ci_3d.extent.width = kWidth;
    image_ci_3d.extent.height = kHeight;
    image_ci_3d.extent.depth = 8;
    image_ci_3d.mipLevels = 1;
    image_ci_3d.arrayLayers = 1;
    image_ci_3d.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci_3d.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci_3d.usage = usage;
    vkt::Image image_3d(*m_device, image_ci_3d, vkt::set_layout);

    vkt::Image other_image(*m_device, kWidth, kHeight, 1, format, usage);

    // The image view is a 2D slice of the 3D image at depth = 4, which we request by
    // asking for arrayLayer = 4
    VkImageSubresourceRange view_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 4, 1};
    // But, the spec says:
    //    Automatic layout transitions apply to the entire image subresource attached
    //    to the framebuffer. If the attachment view is a 2D or 2D array view of a
    //    3D image, even if the attachment view only refers to a subset of the slices
    //    of the selected mip level of the 3D image, automatic layout transitions apply
    //    to the entire subresource referenced which is the entire mip level in this case.
    VkImageSubresourceRange full_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkt::ImageView view_2d, other_view;
    VkImageViewCreateInfo image_view_create_info = vku::InitStructHelper();
    image_view_create_info.image = image_3d.handle();
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = format;
    image_view_create_info.subresourceRange = view_range;

    view_2d.init(*m_device, image_view_create_info);
    ASSERT_TRUE(view_2d.initialized());

    image_view_create_info.image = other_image.handle();
    image_view_create_info.subresourceRange = full_range;
    other_view.init(*m_device, image_view_create_info);
    ASSERT_TRUE(other_view.initialized());

    std::vector<VkAttachmentDescription> attachments = {
        {0, format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
    };

    std::vector<VkAttachmentReference> color = {
        {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };

    VkSubpassDescription subpass = {
        0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, (uint32_t)color.size(), color.data(), nullptr, nullptr, 0, nullptr};

    std::vector<VkSubpassDependency> deps = {
        {VK_SUBPASS_EXTERNAL, 0,
         (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
          VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
          VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT),
         (VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
          VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT),
         (VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
          VK_ACCESS_TRANSFER_WRITE_BIT),
         (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT), 0},
        {0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
         (VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT), VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
         (VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_MEMORY_READ_BIT), 0},
    };

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                   nullptr,
                                   0,
                                   (uint32_t)attachments.size(),
                                   attachments.data(),
                                   1,
                                   &subpass,
                                   (uint32_t)deps.size(),
                                   deps.data()};
    // Create Sampler
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    vkt::Sampler sampler(*m_device, sampler_ci);

    descriptor_set.WriteDescriptorImageInfo(0, other_view, sampler);
    descriptor_set.UpdateDescriptorSets();

    vkt::RenderPass rp(*m_device, rpci);

    // Create PSO to be used for draw-time errors below
    VkShaderObj fs(this, kFragmentSamplerGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.gp_ci_.renderPass = rp.handle();
    pipe.CreateGraphicsPipeline();

    vkt::CommandBuffer cmd_buf(*m_device, m_command_pool);

    enum TestType {
        kInternal,  // Image layout mismatch is *within* a given command buffer
        kExternal   // Image layout mismatch is with the current state of the image, found at QueueSubmit
    };
    std::array<TestType, 2> test_list = {{kInternal, kExternal}};

    for (TestType test_type : test_list) {
        VkImageMemoryBarrier image_barrier = vku::InitStructHelper();

        vkt::Framebuffer fb(*m_device, rp.handle(), 1, &view_2d.handle(), kWidth, kHeight);

        cmd_buf.Begin();
        image_barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        image_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        image_barrier.image = image_3d.handle();
        image_barrier.subresourceRange = full_range;
        image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        vk::CmdPipelineBarrier(cmd_buf.handle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 0,
                               nullptr, 0, nullptr, 1, &image_barrier);
        image_barrier.image = other_image.handle();
        vk::CmdPipelineBarrier(cmd_buf.handle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 0,
                               nullptr, 0, nullptr, 1, &image_barrier);

        if (test_type == kExternal) {
            // The image layout is external to the command buffer we are recording to test.  Submit to push to instance scope.
            cmd_buf.End();
            m_default_queue->Submit(cmd_buf);
            m_default_queue->Wait();
            cmd_buf.Begin();
        }

        m_renderPassBeginInfo.renderPass = rp.handle();
        m_renderPassBeginInfo.framebuffer = fb.handle();
        m_renderPassBeginInfo.renderArea = {{0, 0}, {kWidth, kHeight}};

        cmd_buf.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(cmd_buf.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        vk::CmdBindDescriptorSets(cmd_buf.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
        vk::CmdDraw(cmd_buf.handle(), 1, 0, 0, 0);

        cmd_buf.EndRenderPass();
        cmd_buf.End();

        // Submit cmd buffer
        m_default_queue->Submit(cmd_buf);
        m_default_queue->Wait();
    }
}

TEST_F(PositiveImageLayout, ArrayLayers) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/1998");
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitRenderTarget());

    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 2, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci);

    // layer 0 now VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    // layer 1 is still VK_IMAGE_LAYOUT_UNDEFINED.
    m_command_buffer.Begin();
    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.srcAccessMask = 0;
    img_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    img_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    img_barrier.image = image;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    vkt::ImageView image_view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D, 0, 1, 0, 1);

    VkShaderObj fs(this, kFragmentSamplerGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    pipe.CreateGraphicsPipeline();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    pipe.descriptor_set_->WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveImageLayout, DescriptorArray) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/1998");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitRenderTarget());

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 0) uniform UBO { uint index; };
        // [0] is bad layout
        // [1] is good layout
        layout(set = 0, binding = 1) uniform sampler2D tex[2];
        layout(location = 0) out vec4 uFragColor;
        void main(){
           uFragColor = texture(tex[index], vec2(0, 0));
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    OneOffDescriptorIndexingSet descriptor_set(m_device,
                                               {
                                                   {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr, 0},
                                                   {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, nullptr,
                                                    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
                                               });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    vkt::Buffer in_buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, kHostVisibleMemProps);
    uint32_t *in_buffer_ptr = (uint32_t *)in_buffer.Memory().Map();
    in_buffer_ptr[0] = 1;
    in_buffer.Memory().Unmap();

    vkt::Image bad_image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image good_image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    good_image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkt::ImageView bad_image_view = bad_image.CreateView(VK_IMAGE_ASPECT_COLOR_BIT);
    vkt::ImageView good_image_view = good_image.CreateView(VK_IMAGE_ASPECT_COLOR_BIT);

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    descriptor_set.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE);
    descriptor_set.WriteDescriptorImageInfo(1, bad_image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(1, good_image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}
