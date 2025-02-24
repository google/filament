/*
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/shader_helper.h"
#include "../framework/pipeline_helper.h"

namespace {
VKAPI_ATTR VkBool32 VKAPI_CALL EmptyDebugReportCallback(VkDebugReportFlagsEXT message_flags, VkDebugReportObjectTypeEXT, uint64_t,
                                                        size_t, int32_t, const char *, const char *message, void *user_data) {
    return VK_FALSE;
}
}  // namespace

class NegativeParent : public ParentTest {};

TEST_F(NegativeParent, FillBuffer) {
    TEST_DESCRIPTION("Test VUID-*-commonparent checks not sharing the same Device");

    RETURN_IF_SKIP(Init());
    auto features = m_device->Physical().Features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    vkt::Buffer buffer(*m_second_device, 4096, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdFillBuffer-commonparent");
    vk::CmdFillBuffer(m_command_buffer.handle(), buffer, 0, VK_WHOLE_SIZE, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeParent, BindBuffer) {
    TEST_DESCRIPTION("Test VUID-*-commonparent checks not sharing the same Device");

    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    auto features = m_device->Physical().Features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements mem_reqs;
    m_errorMonitor->SetDesiredError("VUID-vkGetBufferMemoryRequirements-buffer-parent");
    vk::GetBufferMemoryRequirements(m_second_device->handle(), buffer.handle(), &mem_reqs);
    m_errorMonitor->VerifyFound();
    vk::GetBufferMemoryRequirements(device(), buffer.handle(), &mem_reqs);

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.allocationSize = mem_reqs.size;
    m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    vkt::DeviceMemory memory(*m_second_device, mem_alloc);

    VkBindBufferMemoryInfo bind_buffer_info = vku::InitStructHelper();
    bind_buffer_info.buffer = buffer.handle();
    bind_buffer_info.memory = memory.handle();
    bind_buffer_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredError("VUID-VkBindBufferMemoryInfo-commonparent");
    vk::BindBufferMemory2KHR(device(), 1, &bind_buffer_info);
    m_errorMonitor->VerifyFound();
}

// Some of these commonparent VUs are for "non-ignored parameters", various spot related in spec
// Spec issue - https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/6227
TEST_F(NegativeParent, DISABLED_BindImage) {
    TEST_DESCRIPTION("Test VUID-*-commonparent checks not sharing the same Device");

    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    auto features = m_device->Physical().Features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);

    VkMemoryRequirements mem_reqs;
    m_errorMonitor->SetDesiredError("VUID-vkGetImageMemoryRequirements-image-parent");
    vk::GetImageMemoryRequirements(m_second_device->handle(), image.handle(), &mem_reqs);
    m_errorMonitor->VerifyFound();
    vk::GetImageMemoryRequirements(device(), image.handle(), &mem_reqs);

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.allocationSize = mem_reqs.size;
    m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    vkt::DeviceMemory memory(*m_second_device, mem_alloc);

    VkBindImageMemoryInfo bind_image_info = vku::InitStructHelper();
    bind_image_info.image = image.handle();
    bind_image_info.memory = memory.handle();
    bind_image_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-commonparent");
    vk::BindImageMemory2KHR(device(), 1, &bind_image_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, ImageView) {
    TEST_DESCRIPTION("Test VUID-*-commonparent checks not sharing the same Device");

    RETURN_IF_SKIP(Init());
    auto features = m_device->Physical().Features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);

    VkImageView image_view;
    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_B8G8R8A8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    m_errorMonitor->SetDesiredError("VUID-vkCreateImageView-image-09179");
    vk::CreateImageView(m_second_device->handle(), &ivci, nullptr, &image_view);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, BindPipeline) {
    TEST_DESCRIPTION("Test binding pipeline from another device");

    RETURN_IF_SKIP(Init());
    auto features = m_device->Physical().Features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.setLayoutCount = 0;
    vkt::PipelineLayout pipeline_layout(*m_second_device, pipeline_layout_ci);

    VkShaderObj cs(this, kMinimalShaderGlsl, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    cs.InitFromGLSLTry(m_second_device);

    VkComputePipelineCreateInfo pipeline_ci = vku::InitStructHelper();
    pipeline_ci.layout = pipeline_layout.handle();
    pipeline_ci.stage = cs.GetStageCreateInfo();
    vkt::Pipeline pipeline(*m_second_device, pipeline_ci);

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindPipeline-commonparent");
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.handle());
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeParent, PipelineShaderStageCreateInfo) {
    RETURN_IF_SKIP(Init());
    auto features = m_device->Physical().Features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.setLayoutCount = 0;
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);

    VkShaderObj cs(this, kMinimalShaderGlsl, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    cs.InitFromGLSLTry(m_second_device);

    VkComputePipelineCreateInfo pipeline_ci = vku::InitStructHelper();
    pipeline_ci.layout = pipeline_layout.handle();
    pipeline_ci.stage = cs.GetStageCreateInfo();
    m_errorMonitor->SetDesiredError("UNASSIGNED-VkPipelineShaderStageCreateInfo-module-parent");
    vkt::Pipeline pipeline(*m_device, pipeline_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, RenderPassFramebuffer) {
    TEST_DESCRIPTION("Test RenderPass and Framebuffer");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();  // Renderpass created on first device
    auto features = m_device->Physical().Features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    m_errorMonitor->SetDesiredError("VUID-VkFramebufferCreateInfo-commonparent");
    vkt::Framebuffer fb(*m_second_device, m_renderPass, 0, nullptr, m_width, m_height);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, RenderPassImagelessFramebuffer) {
    TEST_DESCRIPTION("Test RenderPass and Imageless Framebuffer");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework());
    VkPhysicalDeviceImagelessFramebufferFeatures imageless_framebuffer = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(imageless_framebuffer);
    RETURN_IF_SKIP(InitState(nullptr, &imageless_framebuffer));

    InitRenderTarget();
    auto features = m_device->Physical().Features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
    VkFramebufferAttachmentImageInfo framebuffer_attachment_image_info = vku::InitStructHelper();
    framebuffer_attachment_image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebuffer_attachment_image_info.width = 256;
    framebuffer_attachment_image_info.height = 256;
    framebuffer_attachment_image_info.layerCount = 1;
    framebuffer_attachment_image_info.viewFormatCount = 1;
    framebuffer_attachment_image_info.pViewFormats = &format;

    VkFramebufferAttachmentsCreateInfo framebuffer_attachments = vku::InitStructHelper();
    framebuffer_attachments.attachmentImageInfoCount = 1;
    framebuffer_attachments.pAttachmentImageInfos = &framebuffer_attachment_image_info;

    VkFramebufferCreateInfo fb_info = vku::InitStructHelper(&framebuffer_attachments);
    fb_info.renderPass = m_renderPass;
    fb_info.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    fb_info.attachmentCount = 1;
    fb_info.width = m_width;
    fb_info.height = m_height;
    fb_info.layers = 1;
    vkt::Framebuffer fb(*m_device, fb_info);

    auto image_ci = vkt::Image::ImageCreateInfo2D(256, 256, 1, 1, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::Image image(*m_second_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    VkRenderPassAttachmentBeginInfo render_pass_attachment_bi = vku::InitStructHelper();
    render_pass_attachment_bi.attachmentCount = 1;
    render_pass_attachment_bi.pAttachments = &image_view.handle();

    m_renderPassBeginInfo.pNext = &render_pass_attachment_bi;
    m_renderPassBeginInfo.framebuffer = fb.handle();

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-framebuffer-02780");
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeParent, RenderPassCommandBuffer) {
    TEST_DESCRIPTION("Test RenderPass and Framebuffer");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();  // Renderpass created on first device
    auto features = m_device->Physical().Features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    vkt::CommandPool command_pool(*m_second_device, m_device->graphics_queue_node_index_, 0);
    vkt::CommandBuffer command_buffer(*m_second_device, command_pool);

    command_buffer.Begin();
    // one for each the framebuffer and renderpass being different from the CommandBuffer
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-commonparent");
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-commonparent");
    auto subpass_begin_info = vku::InitStruct<VkSubpassBeginInfo>(nullptr, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBeginRenderPass2(command_buffer.handle(), &m_renderPassBeginInfo, &subpass_begin_info);
    m_errorMonitor->VerifyFound();
    command_buffer.End();
}

TEST_F(NegativeParent, FreeCommandBuffer) {
    RETURN_IF_SKIP(Init());
    auto features = m_device->Physical().Features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    vkt::CommandPool command_pool_1(*m_device, m_device->graphics_queue_node_index_, 0);
    vkt::CommandPool command_pool_2(*m_second_device, m_device->graphics_queue_node_index_, 0);

    VkCommandBufferAllocateInfo cb_alloc_info = vku::InitStructHelper();
    cb_alloc_info.commandPool = command_pool_1;
    cb_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cb_alloc_info.commandBufferCount = 1;
    VkCommandBuffer command_buffer;
    vk::AllocateCommandBuffers(device(), &cb_alloc_info, &command_buffer);
    m_errorMonitor->SetDesiredError("VUID-vkFreeCommandBuffers-pCommandBuffers-parent");
    m_errorMonitor->SetDesiredError("VUID-vkFreeCommandBuffers-commandPool-parent");
    vk::FreeCommandBuffers(device(), command_pool_2, 1, &command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, Instance_PhysicalDeviceAndSurface) {
    TEST_DESCRIPTION("Surface from a different instance in vkGetPhysicalDeviceSurfaceSupportKHR");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    vkt::Instance instance2(GetInstanceCreateInfo());

    SurfaceContext surface_context;
    vkt::Surface instance2_surface;
    if (CreateSurface(surface_context, instance2_surface, instance2.Handle()) != VK_SUCCESS) {
        GTEST_SKIP() << "Cannot create required surface from 2nd instance";
    }

    VkBool32 supported = VK_FALSE;
    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceSurfaceSupportKHR-commonparent");
    vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), m_device->graphics_queue_node_index_, instance2_surface.Handle(), &supported);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, Instance_DeviceAndSurface) {
    TEST_DESCRIPTION("Surface from a different instance in vkGetDeviceGroupSurfacePresentModesKHR");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    vkt::Instance instance2(GetInstanceCreateInfo());

    SurfaceContext surface_context;
    vkt::Surface instance2_surface;
    if (CreateSurface(surface_context, instance2_surface, instance2.Handle()) != VK_SUCCESS) {
        GTEST_SKIP() << "Cannot create required surface from 2nd instance";
    }

    VkDeviceGroupPresentModeFlagsKHR flags = 0;
    m_errorMonitor->SetDesiredError("VUID-vkGetDeviceGroupSurfacePresentModesKHR-commonparent");
    vk::GetDeviceGroupSurfacePresentModesKHR(m_device->handle(), instance2_surface.Handle(), &flags);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, Instance_Surface) {
    TEST_DESCRIPTION("Surface from a different instance in vkCreateSwapchainKHR");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    vkt::Instance instance2(GetInstanceCreateInfo());

    SurfaceContext surface_context;
    vkt::Surface instance2_surface;
    if (CreateSurface(surface_context, instance2_surface, instance2.Handle()) != VK_SUCCESS) {
        GTEST_SKIP() << "Cannot create required surface from 2nd instance";
    }

    auto swapchain_ci = vku::InitStruct<VkSwapchainCreateInfoKHR>();
    swapchain_ci.surface = instance2_surface.Handle();
    swapchain_ci.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;
    swapchain_ci.clipped = VK_FALSE;
    swapchain_ci.oldSwapchain = VK_NULL_HANDLE;

    // surface from a different instance
    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-commonparent");
    vkt::Swapchain swapchain(*m_device, swapchain_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, Device_OldSwapchain) {
    TEST_DESCRIPTION("oldSwapchain from a different device in vkCreateSwapchainKHR");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    vkt::Instance instance2(GetInstanceCreateInfo());

    SurfaceContext surface_context;
    vkt::Surface instance2_surface;
    if (CreateSurface(surface_context, instance2_surface, instance2.Handle()) != VK_SUCCESS) {
        GTEST_SKIP() << "Cannot create required surface from 2nd instance";
    }

    VkPhysicalDevice instance2_physical_device = VK_NULL_HANDLE;
    {
        uint32_t gpu_count = 0;
        vk::EnumeratePhysicalDevices(instance2.Handle(), &gpu_count, nullptr);
        assert(gpu_count > 0);
        std::vector<VkPhysicalDevice> physical_devices(gpu_count);
        vk::EnumeratePhysicalDevices(instance2.Handle(), &gpu_count, physical_devices.data());
        instance2_physical_device = physical_devices[0];
    }
    vkt::Device instance2_device(instance2_physical_device, m_device_extension_names);

    auto swapchain_ci = vku::InitStruct<VkSwapchainCreateInfoKHR>();
    swapchain_ci.surface = instance2_surface.Handle();
    swapchain_ci.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;
    swapchain_ci.clipped = VK_FALSE;
    swapchain_ci.oldSwapchain = VK_NULL_HANDLE;

    vkt::Swapchain other_device_swapchain(instance2_device, swapchain_ci);

    // oldSwapchain from a different device
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.oldSwapchain = other_device_swapchain;
    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-commonparent");
    vkt::Swapchain swapchain(*m_device, swapchain_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, Instance_Surface_2) {
    TEST_DESCRIPTION("Surface from a different instance in vkDestroySurfaceKHR");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    vkt::Instance instance2(GetInstanceCreateInfo());

    SurfaceContext surface_context;
    vkt::Surface instance2_surface;
    if (CreateSurface(surface_context, instance2_surface, instance2.Handle()) != VK_SUCCESS) {
        GTEST_SKIP() << "Cannot create required surface from 2nd instance";
    }

    // surface from a different instance
    m_errorMonitor->SetDesiredError("VUID-vkDestroySurfaceKHR-surface-parent");
    vk::DestroySurfaceKHR(instance(), instance2_surface.Handle(), nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, Instance_DebugUtilsMessenger) {
    TEST_DESCRIPTION("VkDebugUtilsMessengerEXT from a different instance in vkDestroyDebugUtilsMessengerEXT");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    vkt::Instance instance2(GetInstanceCreateInfo());

    auto empty_callback = [](const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, DebugUtilsLabelCheckData *data) {};
    DebugUtilsLabelCheckData callback_data{};
    callback_data.callback = empty_callback;

    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
    {
        auto messenger_ci = vku::InitStruct<VkDebugUtilsMessengerCreateInfoEXT>();
        messenger_ci.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        messenger_ci.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        messenger_ci.pfnUserCallback = DebugUtilsCallback;
        messenger_ci.pUserData = &callback_data;
        ASSERT_EQ(VK_SUCCESS, vk::CreateDebugUtilsMessengerEXT(instance2.Handle(), &messenger_ci, nullptr, &messenger));
    }

    // debug utils messenger from a different instance
    m_errorMonitor->SetDesiredError("VUID-vkDestroyDebugUtilsMessengerEXT-messenger-parent");
    vk::DestroyDebugUtilsMessengerEXT(instance(), messenger, nullptr);
    m_errorMonitor->VerifyFound();
    vk::DestroyDebugUtilsMessengerEXT(instance2.Handle(), messenger, nullptr);
}

TEST_F(NegativeParent, Instance_DebugReportCallback) {
    TEST_DESCRIPTION("VkDebugReportCallbackEXT from a different instance in vkDestroyDebugReportCallbackEXT");
    AddRequiredExtensions(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    vkt::Instance instance2(GetInstanceCreateInfo());

    VkDebugReportCallbackEXT callback = VK_NULL_HANDLE;
    {
        auto callback_ci = vku::InitStruct<VkDebugReportCallbackCreateInfoEXT>();
        callback_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT;
        callback_ci.pfnCallback = &EmptyDebugReportCallback;
        ASSERT_EQ(VK_SUCCESS, vk::CreateDebugReportCallbackEXT(instance2.Handle(), &callback_ci, nullptr, &callback));
    }

    // debug report callback from a different instance
    m_errorMonitor->SetDesiredError("VUID-vkDestroyDebugReportCallbackEXT-callback-parent");
    vk::DestroyDebugReportCallbackEXT(instance(), callback, nullptr);
    m_errorMonitor->VerifyFound();
    vk::DestroyDebugReportCallbackEXT(instance2.Handle(), callback, nullptr);
}

TEST_F(NegativeParent, PhysicalDevice_Display) {
    TEST_DESCRIPTION("VkDisplayKHR from a different physical device in vkGetDisplayModePropertiesKHR");
    AddRequiredExtensions(VK_KHR_DISPLAY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::Instance instance2(GetInstanceCreateInfo());

    VkPhysicalDevice instance2_gpu = VK_NULL_HANDLE;
    {
        uint32_t gpu_count = 0;
        vk::EnumeratePhysicalDevices(instance2.Handle(), &gpu_count, nullptr);
        ASSERT_GT(gpu_count, 0u);
        std::vector<VkPhysicalDevice> physical_devices(gpu_count);
        vk::EnumeratePhysicalDevices(instance2.Handle(), &gpu_count, physical_devices.data());
        instance2_gpu = physical_devices[0];
    }
    VkDisplayKHR display = VK_NULL_HANDLE;
    {
        uint32_t display_count = 0;
        ASSERT_EQ(VK_SUCCESS, vk::GetPhysicalDeviceDisplayPropertiesKHR(instance2_gpu, &display_count, nullptr));
        if (display_count == 0) {
            GTEST_SKIP() << "No VkDisplayKHR displays found";
        }
        std::vector<VkDisplayPropertiesKHR> display_props{display_count};
        ASSERT_EQ(VK_SUCCESS, vk::GetPhysicalDeviceDisplayPropertiesKHR(instance2_gpu, &display_count, display_props.data()));
        display = display_props[0].display;
    }
    // display from a different physical device
    uint32_t mode_count = 0;
    m_errorMonitor->SetDesiredError("VUID-vkGetDisplayModePropertiesKHR-display-parent");
    vk::GetDisplayModePropertiesKHR(Gpu(), display, &mode_count, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, PhysicalDevice_RegisterDisplayEvent) {
    TEST_DESCRIPTION("VkDisplayKHR from a different physical device in vkRegisterDisplayEventEXT");
    AddRequiredExtensions(VK_EXT_DISPLAY_CONTROL_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::Instance instance2(GetInstanceCreateInfo());

    VkPhysicalDevice instance2_gpu = VK_NULL_HANDLE;
    {
        uint32_t gpu_count = 0;
        vk::EnumeratePhysicalDevices(instance2.Handle(), &gpu_count, nullptr);
        ASSERT_GT(gpu_count, 0u);
        std::vector<VkPhysicalDevice> physical_devices(gpu_count);
        vk::EnumeratePhysicalDevices(instance2.Handle(), &gpu_count, physical_devices.data());
        instance2_gpu = physical_devices[0];
    }
    VkDisplayKHR display = VK_NULL_HANDLE;
    {
        uint32_t display_count = 0;
        ASSERT_EQ(VK_SUCCESS, vk::GetPhysicalDeviceDisplayPropertiesKHR(instance2_gpu, &display_count, nullptr));
        if (display_count == 0) {
            GTEST_SKIP() << "No VkDisplayKHR displays found";
        }
        std::vector<VkDisplayPropertiesKHR> display_props{display_count};
        ASSERT_EQ(VK_SUCCESS, vk::GetPhysicalDeviceDisplayPropertiesKHR(instance2_gpu, &display_count, display_props.data()));
        display = display_props[0].display;
    }

    VkDisplayEventInfoEXT event_info = vku::InitStructHelper();
    event_info.displayEvent = VK_DISPLAY_EVENT_TYPE_FIRST_PIXEL_OUT_EXT;
    VkFence fence;

    m_errorMonitor->SetDesiredError("VUID-vkRegisterDisplayEventEXT-commonparent");
    vk::RegisterDisplayEventEXT(device(), display, &event_info, nullptr, &fence);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, PhysicalDevice_DisplayMode) {
    TEST_DESCRIPTION("VkDisplayModeKHR from a different physical device in vkGetDisplayPlaneCapabilitiesKHR");
    AddRequiredExtensions(VK_KHR_DISPLAY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::Instance instance2(GetInstanceCreateInfo());

    VkPhysicalDevice instance2_gpu = VK_NULL_HANDLE;
    {
        uint32_t gpu_count = 0;
        vk::EnumeratePhysicalDevices(instance2.Handle(), &gpu_count, nullptr);
        ASSERT_GT(gpu_count, 0u);
        std::vector<VkPhysicalDevice> physical_devices(gpu_count);
        vk::EnumeratePhysicalDevices(instance2.Handle(), &gpu_count, physical_devices.data());
        instance2_gpu = physical_devices[0];
    }
    VkDisplayKHR display = VK_NULL_HANDLE;
    {
        uint32_t plane_count = 0;
        ASSERT_EQ(VK_SUCCESS, vk::GetPhysicalDeviceDisplayPlanePropertiesKHR(instance2_gpu, &plane_count, nullptr));
        if (plane_count == 0) {
            GTEST_SKIP() << "No display planes found";
        }
        std::vector<VkDisplayPlanePropertiesKHR> display_planes(plane_count);
        ASSERT_EQ(VK_SUCCESS, vk::GetPhysicalDeviceDisplayPlanePropertiesKHR(instance2_gpu, &plane_count, display_planes.data()));
        display = display_planes[0].currentDisplay;
        if (display == VK_NULL_HANDLE) {
            GTEST_SKIP() << "Null display";
        }
    }
    VkDisplayModeKHR display_mode = VK_NULL_HANDLE;
    {
        VkDisplayModeParametersKHR display_mode_parameters = {{32, 32}, 30};
        VkDisplayModeCreateInfoKHR display_mode_info = vku::InitStructHelper();
        display_mode_info.parameters = display_mode_parameters;
        vk::CreateDisplayModeKHR(instance2_gpu, display, &display_mode_info, nullptr, &display_mode);
        if (display_mode == VK_NULL_HANDLE) {
            GTEST_SKIP() << "Can't create a VkDisplayMode";
        }
    }
    // display mode from a different physical device
    VkDisplayPlaneCapabilitiesKHR plane_capabilities{};
    m_errorMonitor->SetDesiredError("VUID-vkGetDisplayPlaneCapabilitiesKHR-mode-parent");
    vk::GetDisplayPlaneCapabilitiesKHR(Gpu(), display_mode, 0, &plane_capabilities);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, PipelineExecutableInfo) {
    TEST_DESCRIPTION("Try making calls without pipelineExecutableInfo.");

    AddRequiredExtensions(VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR pipeline_exe_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(pipeline_exe_features);
    RETURN_IF_SKIP(InitState(nullptr, &pipeline_exe_features));
    InitRenderTarget();

    m_second_device = new vkt::Device(gpu_, m_device_extension_names, nullptr, &pipeline_exe_features);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    VkPipelineExecutableInfoKHR pipeline_exe_info = vku::InitStructHelper();
    pipeline_exe_info.pipeline = pipe.Handle();
    pipeline_exe_info.executableIndex = 0;

    VkPipelineInfoKHR pipeline_info = vku::InitStructHelper();
    pipeline_info.pipeline = pipe.Handle();

    uint32_t count;
    m_errorMonitor->SetDesiredError("VUID-vkGetPipelineExecutableStatisticsKHR-pipeline-03273");
    vk::GetPipelineExecutableStatisticsKHR(*m_second_device, &pipeline_exe_info, &count, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkGetPipelineExecutableInternalRepresentationsKHR-pipeline-03277");
    vk::GetPipelineExecutableInternalRepresentationsKHR(*m_second_device, &pipeline_exe_info, &count, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkGetPipelineExecutablePropertiesKHR-pipeline-03271");
    vk::GetPipelineExecutablePropertiesKHR(*m_second_device, &pipeline_info, &count, nullptr);
    m_errorMonitor->VerifyFound();
}

// TODO - https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9176
TEST_F(NegativeParent, DISABLED_PipelineInfoEXT) {
    TEST_DESCRIPTION("Try making calls without pipelineExecutableInfo.");

    AddRequiredExtensions(VK_EXT_PIPELINE_PROPERTIES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDevicePipelinePropertiesFeaturesEXT pipeline_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(pipeline_features);
    RETURN_IF_SKIP(InitState(nullptr, &pipeline_features));
    InitRenderTarget();

    m_second_device = new vkt::Device(gpu_, m_device_extension_names, nullptr, &pipeline_features);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    VkPipelineInfoEXT pipeline_info = vku::InitStructHelper();
    pipeline_info.sType = VK_STRUCTURE_TYPE_PIPELINE_PROPERTIES_IDENTIFIER_EXT;
    pipeline_info.pipeline = pipe.Handle();

    VkBaseOutStructure out_struct;
    out_struct.sType = VK_STRUCTURE_TYPE_PIPELINE_PROPERTIES_IDENTIFIER_EXT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPipelinePropertiesEXT-pipeline-06738");
    vk::GetPipelinePropertiesEXT(*m_second_device, &pipeline_info, &out_struct);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, UpdateDescriptorSetsBuffer) {
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    auto features = m_device->Physical().Features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    vkt::Buffer buffer(*m_second_device, 4096, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                     });
    ds.WriteDescriptorBufferInfo(0, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

    m_errorMonitor->SetDesiredError("VUID-vkUpdateDescriptorSets-pDescriptorWrites-06237");
    ds.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, UpdateDescriptorSetsImage) {
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    auto features = m_device->Physical().Features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    vkt::Image image(*m_second_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::ImageView image_view = image.CreateView();

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                     });
    ds.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

    m_errorMonitor->SetDesiredError("VUID-vkUpdateDescriptorSets-pDescriptorWrites-06239");
    ds.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, UpdateDescriptorSetsSampler) {
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    auto features = m_device->Physical().Features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_second_device, SafeSaneSamplerCreateInfo());

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                     });
    ds.WriteDescriptorImageInfo(0, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);

    m_errorMonitor->SetDesiredError("VUID-vkUpdateDescriptorSets-pDescriptorWrites-06238");
    ds.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, UpdateDescriptorSetsCombinedImageSampler) {
    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    auto features = m_device->Physical().Features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    vkt::Image bad_image(*m_second_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::ImageView bad_image_view = bad_image.CreateView();
    vkt::Sampler bad_sampler(*m_second_device, SafeSaneSamplerCreateInfo());

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                     });

    // TODO - This might involve state tracking in ObjectTracker, but likely will be resolved from
    // https://gitlab.khronos.org/vulkan/vulkan/-/issues/4177
    // m_errorMonitor->SetDesiredError("VUID-vkUpdateDescriptorSets-pDescriptorWrites-06238");
    // ds.WriteDescriptorImageInfo(0, image_view, bad_sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    // ds.UpdateDescriptorSets();
    // m_errorMonitor->VerifyFound();

    ds.Clear();
    m_errorMonitor->SetDesiredError("VUID-vkUpdateDescriptorSets-pDescriptorWrites-06239");
    ds.WriteDescriptorImageInfo(0, bad_image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    ds.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, DescriptorSetLayout) {
    TEST_DESCRIPTION("Create pipeline layout from a descriptor set layout that was created on a different device");
    RETURN_IF_SKIP(Init());

    auto features = m_device->Physical().Features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.setLayoutCount = 1u;
    pipeline_layout_ci.pSetLayouts = &descriptor_set.layout_.handle();

    m_errorMonitor->SetDesiredError("UNASSIGNED-VkPipelineLayoutCreateInfo-pSetLayouts-commonparent");
    VkPipelineLayout handle;
    vk::CreatePipelineLayout(m_second_device->handle(), &pipeline_layout_ci, nullptr, &handle);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, FlushInvalidateMemory) {
    RETURN_IF_SKIP(Init());
    auto features = m_device->Physical().Features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.memoryTypeIndex = 0;
    alloc_info.allocationSize = 64;

    bool pass = m_device->Physical().SetMemoryType(0xFFFFFFFF, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!pass) {
        GTEST_SKIP() << "Host Visible memory not found";
    }

    vkt::DeviceMemory device_memory(*m_device, alloc_info);

    VkMappedMemoryRange memory_range = vku::InitStructHelper();
    memory_range.memory = device_memory;
    memory_range.offset = 0;
    memory_range.size = VK_WHOLE_SIZE;

    void *pData;
    vk::MapMemory(device(), device_memory, 0, VK_WHOLE_SIZE, 0, &pData);

    m_errorMonitor->SetDesiredError("UNASSIGNED-VkMappedMemoryRange-memory-device");
    vk::FlushMappedMemoryRanges(m_second_device->handle(), 1, &memory_range);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("UNASSIGNED-VkMappedMemoryRange-memory-device");
    vk::InvalidateMappedMemoryRanges(m_second_device->handle(), 1, &memory_range);
    m_errorMonitor->VerifyFound();

    vk::UnmapMemory(device(), device_memory);
}

TEST_F(NegativeParent, GetDescriptorSetLayoutSupport) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBuffer);
    RETURN_IF_SKIP(Init());
    auto features = m_device->Physical().Features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, &sampler.handle()};
    const VkDescriptorSetLayoutCreateFlags flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    const auto dslci = vku::InitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, flags, 1u, &binding);
    VkDescriptorSetLayoutSupport support = vku::InitStructHelper();
    m_errorMonitor->SetDesiredError("UNASSIGNED-vkGetDescriptorSetLayoutSupport-pImmutableSamplers-device");
    vk::GetDescriptorSetLayoutSupport(m_second_device->handle(), &dslci, &support);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, BufferView) {
    RETURN_IF_SKIP(Init());
    auto features = m_device->Physical().Features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    vkt::Buffer buffer(*m_second_device, 64, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    m_errorMonitor->SetDesiredError("UNASSIGNED-VkBufferViewCreateInfo-buffer-parent");
    vkt::BufferView buffer_view(*m_device, bvci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, CmdPipelineBarrier) {
    RETURN_IF_SKIP(Init());
    auto features = m_device->Physical().Features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    auto image_ci = vkt::Image::ImageCreateInfo2D(256, 256, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::Image image(*m_second_device, image_ci, vkt::set_layout);

    VkImageSubresource image_sub = vkt::Image::Subresource(VK_IMAGE_ASPECT_COLOR_BIT, 0, 0);
    VkImageSubresourceRange image_sub_range = vkt::Image::SubresourceRange(image_sub);
    VkImageMemoryBarrier image_barriers[] = {image.ImageMemoryBarrier(0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                                                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, image_sub_range)};

    m_errorMonitor->SetDesiredError("UNASSIGNED-vkCmdPipelineBarrier-commandBuffer-commonparent");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, image_barriers);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, CmdPipelineBarrier2) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(Init());
    auto features = m_device->Physical().Features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    vkt::Buffer buffer(*m_second_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    VkBufferMemoryBarrier2 buffer_barrier = vku::InitStructHelper();
    buffer_barrier.srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
    buffer_barrier.srcAccessMask = VK_ACCESS_2_HOST_WRITE_BIT;
    buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    buffer_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    buffer_barrier.srcQueueFamilyIndex = 0;
    buffer_barrier.dstQueueFamilyIndex = 0;
    buffer_barrier.buffer = buffer.handle();
    buffer_barrier.size = VK_WHOLE_SIZE;

    VkDependencyInfo buffer_dependency = vku::InitStructHelper();
    buffer_dependency.bufferMemoryBarrierCount = 1;
    buffer_dependency.pBufferMemoryBarriers = &buffer_barrier;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("UNASSIGNED-vkCmdPipelineBarrier2-commandBuffer-commonparent");
    vk::CmdPipelineBarrier2(m_command_buffer.handle(), &buffer_dependency);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeParent, ShaderObjectDescriptorSetLayout) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(Init());
    auto features = m_device->Physical().Features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    OneOffDescriptorSet descriptor_set(m_second_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    VkDescriptorSetLayout dsl_handle = descriptor_set.layout_.handle();

    VkShaderStageFlagBits stage = VK_SHADER_STAGE_VERTEX_BIT;
    m_errorMonitor->SetDesiredError("UNASSIGNED-VkShaderCreateInfoEXT-pSetLayouts-parent");
    const vkt::Shader vertShader(*m_device, stage, GLSLToSPV(stage, kVertexMinimalGlsl), &dsl_handle);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeParent, MapMemory2) {
    AddRequiredExtensions(VK_KHR_MAP_MEMORY_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    auto features = m_device->Physical().Features();
    m_second_device = new vkt::Device(gpu_, m_device_extension_names, &features, nullptr);

    VkMemoryAllocateInfo memory_info = vku::InitStructHelper();
    memory_info.allocationSize = 64;
    m_device->Physical().SetMemoryType(vvl::kU32Max, &memory_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    vkt::DeviceMemory memory(*m_second_device, memory_info);

    VkMemoryMapInfo map_info = vku::InitStructHelper();
    map_info.memory = memory;
    map_info.offset = 0;
    map_info.size = memory_info.allocationSize;

    uint32_t *pData = nullptr;
    m_errorMonitor->SetDesiredError("UNASSIGNED-VkMemoryMapInfo-memory-parent");
    vk::MapMemory2KHR(device(), &map_info, (void **)&pData);
    m_errorMonitor->VerifyFound();
}
