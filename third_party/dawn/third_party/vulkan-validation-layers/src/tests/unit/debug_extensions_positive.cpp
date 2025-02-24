/*
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"

class PositiveDebugExtensions : public VkLayerTest {};

TEST_F(PositiveDebugExtensions, SetDebugUtilsObjectBuffer) {
    TEST_DESCRIPTION("call vkSetDebugUtilsObjectNameEXT on a VkBuffer");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Skipping object naming test with MockICD.";
    }

    DebugUtilsLabelCheckData callback_data;
    auto empty_callback = [](const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, DebugUtilsLabelCheckData *data) {
        data->count++;
    };
    callback_data.count = 0;
    callback_data.callback = empty_callback;

    VkDebugUtilsMessengerCreateInfoEXT callback_create_info = vku::InitStructHelper();
    callback_create_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    callback_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    callback_create_info.pfnUserCallback = DebugUtilsCallback;
    callback_create_info.pUserData = &callback_data;
    VkDebugUtilsMessengerEXT my_messenger = VK_NULL_HANDLE;
    vk::CreateDebugUtilsMessengerEXT(instance(), &callback_create_info, nullptr, &my_messenger);

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    const char* object_name = "buffer_object";

    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_BUFFER;
    name_info.objectHandle = (uint64_t)buffer.handle();
    name_info.pObjectName = object_name;
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    vk::DestroyDebugUtilsMessengerEXT(instance(), my_messenger, nullptr);
}

TEST_F(PositiveDebugExtensions, SetDebugUtilsObjectDevice) {
    TEST_DESCRIPTION("call vkSetDebugUtilsObjectNameEXT on a itself as a VkDevice object");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Skipping object naming test with MockICD.";
    }

    DebugUtilsLabelCheckData callback_data;
    auto empty_callback = [](const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, DebugUtilsLabelCheckData *data) {
        data->count++;
    };
    callback_data.count = 0;
    callback_data.callback = empty_callback;

    VkDebugUtilsMessengerCreateInfoEXT callback_create_info = vku::InitStructHelper();
    callback_create_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    callback_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    callback_create_info.pfnUserCallback = DebugUtilsCallback;
    callback_create_info.pUserData = &callback_data;
    VkDebugUtilsMessengerEXT my_messenger = VK_NULL_HANDLE;
    vk::CreateDebugUtilsMessengerEXT(instance(), &callback_create_info, nullptr, &my_messenger);

    const char* object_name = "device_object";

    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_DEVICE;
    name_info.objectHandle = (uint64_t)device();
    name_info.pObjectName = object_name;
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    vk::DestroyDebugUtilsMessengerEXT(instance(), my_messenger, nullptr);
}

TEST_F(PositiveDebugExtensions, DebugLabelPrimaryCommandBuffer) {
    TEST_DESCRIPTION("Test primary command buffer debug labels");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    m_command_buffer.Begin();
    VkDebugUtilsLabelEXT label = vku::InitStructHelper();
    label.pLabelName = "test";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &label);
    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveDebugExtensions, DebugLabelPrimaryCommandBuffer2) {
    TEST_DESCRIPTION("Test primary command buffer debug labels with multiple submits");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkDebugUtilsLabelEXT label = vku::InitStructHelper();
    label.pLabelName = "test";
    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    cb0.Begin();
    vk::CmdBeginDebugUtilsLabelEXT(cb0, &label);
    cb0.End();
    m_default_queue->Submit(cb0);

    vkt::CommandBuffer cb1(*m_device, m_command_pool);
    cb1.Begin();
    vk::CmdEndDebugUtilsLabelEXT(cb1);
    cb1.End();
    m_default_queue->Submit(cb1);

    m_default_queue->Wait();
}

TEST_F(PositiveDebugExtensions, DebugLabelPrimaryCommandBuffer3) {
    TEST_DESCRIPTION("Test primary command buffer debug labels with multiple submits");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkDebugUtilsLabelEXT label = vku::InitStructHelper();
    label.pLabelName = "test";
    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    cb0.Begin();
    vk::CmdBeginDebugUtilsLabelEXT(cb0, &label);
    cb0.End();

    vkt::CommandBuffer cb1(*m_device, m_command_pool);
    cb1.Begin();
    vk::CmdEndDebugUtilsLabelEXT(cb1);
    cb1.End();

    std::array cbs = {&cb0, &cb1};
    m_default_queue->Submit(cbs);
    m_default_queue->Wait();
}

TEST_F(PositiveDebugExtensions, DebugLabelSecondaryCommandBuffer) {
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    vkt::CommandBuffer cb(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    cb.Begin();
    {
        VkDebugUtilsLabelEXT label = vku::InitStructHelper();
        label.pLabelName = "test";
        vk::CmdBeginDebugUtilsLabelEXT(cb, &label);
        vk::CmdEndDebugUtilsLabelEXT(cb);
    }
    cb.End();
}

TEST_F(PositiveDebugExtensions, SwapchainImagesDebugMarker) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7977");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());

    SurfaceInformation info = GetSwapchainInfo(m_surface.Handle());
    InitSwapchainInfo();

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = info.surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = info.surface_formats[0].format;
    swapchain_create_info.imageColorSpace = info.surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = info.surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = info.surface_composite_alpha;
    swapchain_create_info.presentMode = info.surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

    vkt::Swapchain swapchain(*m_device, swapchain_create_info);
    const auto images = swapchain.GetImages();

    {
        VkDebugMarkerObjectNameInfoEXT name_info = vku::InitStructHelper();
        name_info.object = (uint64_t)images[0];
        name_info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT;
        std::string image_name = "swapchain [0]";
        name_info.pObjectName = image_name.c_str();
        vk::DebugMarkerSetObjectNameEXT(device(), &name_info);
    }

    {
        int tags[2] = {1, 2};
        VkDebugMarkerObjectTagInfoEXT name_info = vku::InitStructHelper();
        name_info.object = (uint64_t)images[0];
        name_info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT;
        name_info.tagName = 1;
        name_info.tagSize = 2;
        name_info.pTag = tags;
        vk::DebugMarkerSetObjectTagEXT(device(), &name_info);
    }
}
