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

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "generated/vk_function_pointers.h"

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
#include "wayland-client.h"
#endif

class NegativeWsi : public WsiTest {};

TEST_F(NegativeWsi, GetPhysicalDeviceDisplayPropertiesNull) {
    TEST_DESCRIPTION("Call vkGetPhysicalDeviceDisplayPropertiesKHR with null pointer");
    AddRequiredExtensions(VK_KHR_DISPLAY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceDisplayPropertiesKHR-pPropertyCount-parameter");
    vk::GetPhysicalDeviceDisplayPropertiesKHR(Gpu(), nullptr, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, InitSwapchainPotentiallyIncompatibleFlag) {
    TEST_DESCRIPTION("Initialize swapchain with potentially incompatible flags");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_SURFACE_PROTECTED_CAPABILITIES_EXTENSION_NAME);
    AddSurfaceExtension();

    RETURN_IF_SKIP(Init());

    InitRenderTarget();
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.surface = m_surface.Handle();
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
    swapchain_ci.oldSwapchain = 0;

    // "protected" flag support is device defined
    {
        swapchain_ci.flags = VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR;

        // Get surface protected capabilities
        VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper();
        surface_info.surface = swapchain_ci.surface;
        VkSurfaceProtectedCapabilitiesKHR surface_protected_capabilities = vku::InitStructHelper();
        VkSurfaceCapabilities2KHR surface_capabilities = vku::InitStructHelper();
        surface_capabilities.pNext = &surface_protected_capabilities;
        vk::GetPhysicalDeviceSurfaceCapabilities2KHR(Gpu(), &surface_info, &surface_capabilities);

        // Create swapchain, monitor potential validation error
        if (!surface_protected_capabilities.supportsProtected) {
            m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-flags-03187");
            vkt::Swapchain swapchain(*m_device, swapchain_ci);
            m_errorMonitor->VerifyFound();
        } else {
            vkt::Swapchain swapchain(*m_device, swapchain_ci);
        }
    }

    // "split instance bind regions" not supported when there is only one device
    VkImageFormatProperties image_format_props{};
    if (vk::GetPhysicalDeviceImageFormatProperties(
            Gpu(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT, &image_format_props) == VK_SUCCESS) {
        swapchain_ci.flags = VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR;

        m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-physicalDeviceCount-01429");
        vkt::Swapchain swapchain(*m_device, swapchain_ci);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeWsi, BindImageMemorySwapchain) {
    TEST_DESCRIPTION("Invalid bind image with a swapchain");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "This test appears to leave the image created a swapchain in a weird state that leads to 00378 when it "
                        "shouldn't. Requires further investigation.";
    }

    InitRenderTarget();
    RETURN_IF_SKIP(InitSwapchain(VK_IMAGE_USAGE_TRANSFER_SRC_BIT));

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = m_surface_formats[0].format;
    image_create_info.extent.width = m_surface_capabilities.minImageExtent.width;
    image_create_info.extent.height = m_surface_capabilities.minImageExtent.height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageSwapchainCreateInfoKHR image_swapchain_create_info = vku::InitStructHelper();
    image_swapchain_create_info.swapchain = m_swapchain;
    image_create_info.pNext = &image_swapchain_create_info;

    vkt::Image image_from_swapchain(*m_device, image_create_info, vkt::no_mem);

    VkImageMemoryRequirementsInfo2 image_memory_requirements_info = vku::InitStructHelper();
    image_memory_requirements_info.image = image_from_swapchain.handle();
    VkMemoryDedicatedRequirements memory_dedicated_requirements = vku::InitStructHelper();

    VkMemoryRequirements2 mem_reqs = vku::InitStructHelper(&memory_dedicated_requirements);
    vk::GetImageMemoryRequirements2(device(), &image_memory_requirements_info, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.memoryTypeIndex = 0;
    alloc_info.allocationSize = mem_reqs.memoryRequirements.size;
    if (alloc_info.allocationSize == 0) {
        GTEST_SKIP() << "Driver seems to not be returning an valid allocation size and need to end test";
    }

    vkt::DeviceMemory mem;
    bool pass = m_device->Physical().SetMemoryType(mem_reqs.memoryRequirements.memoryTypeBits, &alloc_info, 0);
    // some devices don't give us good memory requirements for the swapchain image
    if (pass) {
        mem.init(*m_device, alloc_info);
        ASSERT_TRUE(mem.initialized());
    }

    VkBindImageMemoryInfo bind_info = vku::InitStructHelper();
    bind_info.image = image_from_swapchain.handle();
    bind_info.memory = VK_NULL_HANDLE;
    bind_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-image-01630");
    m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-pNext-01632");
    vk::BindImageMemory2(device(), 1, &bind_info);
    m_errorMonitor->VerifyFound();

    VkBindImageMemorySwapchainInfoKHR bind_swapchain_info = vku::InitStructHelper();
    bind_swapchain_info.swapchain = VK_NULL_HANDLE;
    bind_swapchain_info.imageIndex = 0;
    bind_info.pNext = &bind_swapchain_info;

    m_errorMonitor->SetDesiredError("UNASSIGNED-GeneralParameterError-RequiredHandle");
    vk::BindImageMemory2(device(), 1, &bind_info);
    m_errorMonitor->VerifyFound();

    bind_info.memory = mem.handle();
    bind_swapchain_info.swapchain = m_swapchain;
    bind_swapchain_info.imageIndex = std::numeric_limits<uint32_t>::max();

    if (mem.initialized()) {
        m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-pNext-01631");
    }
    m_errorMonitor->SetDesiredError("VUID-VkBindImageMemorySwapchainInfoKHR-imageIndex-01644");
    if (memory_dedicated_requirements.requiresDedicatedAllocation) {
        m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-image-01445");
    }
    vk::BindImageMemory2(device(), 1, &bind_info);
    m_errorMonitor->VerifyFound();

    bind_info.memory = VK_NULL_HANDLE;
    bind_swapchain_info.imageIndex = 0;
    vk::BindImageMemory2(device(), 1, &bind_info);
}

TEST_F(NegativeWsi, SwapchainImage) {
    TEST_DESCRIPTION("Swapchain images with invalid parameters");
    const char *vuid = "VUID-VkImageSwapchainCreateInfoKHR-swapchain-00995";

    AddSurfaceExtension();
    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    RETURN_IF_SKIP(InitSwapchain());

    VkImageSwapchainCreateInfoKHR image_swapchain_create_info = vku::InitStructHelper();
    image_swapchain_create_info.swapchain = m_swapchain;

    VkImageCreateInfo image_create_info = vku::InitStructHelper(&image_swapchain_create_info);
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkImageCreateInfo good_create_info = image_create_info;

    // imageType
    image_create_info = good_create_info;
    image_create_info.imageType = VK_IMAGE_TYPE_3D;
    CreateImageTest(*this, &image_create_info, vuid);

    // mipLevels
    image_create_info = good_create_info;
    image_create_info.mipLevels = 2;
    CreateImageTest(*this, &image_create_info, vuid);

    // samples
    image_create_info = good_create_info;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    CreateImageTest(*this, &image_create_info, vuid);

    // tiling
    image_create_info = good_create_info;
    image_create_info.tiling = VK_IMAGE_TILING_LINEAR;
    CreateImageTest(*this, &image_create_info, vuid);

    // initialLayout
    image_create_info = good_create_info;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    CreateImageTest(*this, &image_create_info, vuid);

    // flags
    if (m_device->Physical().Features().sparseBinding) {
        image_create_info = good_create_info;
        image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
        CreateImageTest(*this, &image_create_info, vuid);
    }
}

TEST_F(NegativeWsi, TransferImageToSwapchainLayoutDeviceGroup) {
    TEST_DESCRIPTION("Transfer an image to a swapchain's image with a invalid layout between device group");

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    GTEST_SKIP() << "According to valid usage, VkBindImageMemoryInfo-memory should be NULL. But Android will crash if memory is "
                    "NULL, skipping test";
#endif

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddSurfaceExtension();
    RETURN_IF_SKIP(InitFramework());

    const auto physical_device_group = FindPhysicalDeviceGroup();
    if (!physical_device_group.has_value()) {
        GTEST_SKIP() << "cannot find physical device group that contains selected physical device";
    }

    VkDeviceGroupDeviceCreateInfo create_device_pnext = vku::InitStructHelper();
    create_device_pnext.physicalDeviceCount = physical_device_group->physicalDeviceCount;
    create_device_pnext.pPhysicalDevices = physical_device_group->physicalDevices;
    RETURN_IF_SKIP(InitState(nullptr, &create_device_pnext));
    InitRenderTarget();
    RETURN_IF_SKIP(InitSwapchain(VK_IMAGE_USAGE_TRANSFER_DST_BIT));

    constexpr uint32_t test_extent_value = 10;
    if (m_surface_capabilities.minImageExtent.width < test_extent_value ||
        m_surface_capabilities.minImageExtent.height < test_extent_value) {
        GTEST_SKIP() << "minImageExtent is not large enough";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = m_surface_formats[0].format;
    image_create_info.extent.width = m_surface_capabilities.minImageExtent.width;
    image_create_info.extent.height = m_surface_capabilities.minImageExtent.height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkt::Image src_Image(*m_device, image_create_info, vkt::set_layout);

    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VkImageSwapchainCreateInfoKHR image_swapchain_create_info = vku::InitStructHelper();
    image_swapchain_create_info.swapchain = m_swapchain;
    image_create_info.pNext = &image_swapchain_create_info;

    vkt::Image peer_image(*m_device, image_create_info, vkt::no_mem);

    VkBindImageMemoryDeviceGroupInfo bind_devicegroup_info = vku::InitStructHelper();
    bind_devicegroup_info.deviceIndexCount = 1;
    std::array<uint32_t, 1> deviceIndices = {{0}};
    bind_devicegroup_info.pDeviceIndices = deviceIndices.data();
    bind_devicegroup_info.splitInstanceBindRegionCount = 0;
    bind_devicegroup_info.pSplitInstanceBindRegions = nullptr;

    VkBindImageMemorySwapchainInfoKHR bind_swapchain_info = vku::InitStructHelper(&bind_devicegroup_info);
    bind_swapchain_info.swapchain = m_swapchain;
    bind_swapchain_info.imageIndex = 0;

    VkBindImageMemoryInfo bind_info = vku::InitStructHelper(&bind_swapchain_info);
    bind_info.image = peer_image.handle();
    bind_info.memory = VK_NULL_HANDLE;
    bind_info.memoryOffset = 0;
    vk::BindImageMemory2(device(), 1, &bind_info);

    const auto swapchain_images = m_swapchain.GetImages();

    m_command_buffer.Begin();

    VkImageCopy copy_region = {};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {test_extent_value, test_extent_value, 1};
    vk::CmdCopyImage(m_command_buffer.handle(), src_Image.handle(), VK_IMAGE_LAYOUT_GENERAL, peer_image.handle(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

    m_command_buffer.End();

    // Even though both peer_image and swapchain_images[0] use the same memory and are in an invalid layout,
    // only peer_image is referenced by the command buffer so there should only be one error reported.
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-09600");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();

    // peer_image is a presentable image and controlled by the implementation
}

TEST_F(NegativeWsi, SwapchainImageParams) {
    TEST_DESCRIPTION("Swapchain with invalid implied image creation parameters");
    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);

    AddSurfaceExtension();

    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkDeviceGroupDeviceCreateInfo device_group_ci = vku::InitStructHelper();
    device_group_ci.physicalDeviceCount = 1;
    VkPhysicalDevice pdev = Gpu();
    device_group_ci.pPhysicalDevices = &pdev;
    RETURN_IF_SKIP(InitState(nullptr, &device_group_ci));
    InitRenderTarget();
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkSwapchainCreateInfoKHR good_create_info = vku::InitStructHelper();
    good_create_info.surface = m_surface.Handle();
    good_create_info.minImageCount = m_surface_capabilities.minImageCount;
    good_create_info.imageFormat = m_surface_formats[0].format;
    good_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    good_create_info.imageExtent = m_surface_capabilities.minImageExtent;
    good_create_info.imageArrayLayers = 1;
    good_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    good_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    good_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    good_create_info.compositeAlpha = m_surface_composite_alpha;
    good_create_info.presentMode = m_surface_non_shared_present_mode;
    good_create_info.clipped = VK_FALSE;
    good_create_info.oldSwapchain = 0;

    VkSwapchainCreateInfoKHR create_info_bad_usage = good_create_info;
    bool found_bad_usage = false;
    // Trying to find format+usage combination supported by surface, but not supported by image.
    const std::array<VkImageUsageFlags, 5> kImageUsageFlags = {{
        VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_USAGE_STORAGE_BIT,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
    }};

    for (uint32_t i = 0; i < kImageUsageFlags.size() && !found_bad_usage; ++i) {
        if ((m_surface_capabilities.supportedUsageFlags & kImageUsageFlags[i]) != 0) {
            for (size_t j = 0; j < m_surface_formats.size(); ++j) {
                VkImageFormatProperties image_format_properties = {};
                VkResult image_format_properties_result = vk::GetPhysicalDeviceImageFormatProperties(
                    m_device->Physical().handle(), m_surface_formats[j].format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                    kImageUsageFlags[i], 0, &image_format_properties);

                if (image_format_properties_result != VK_SUCCESS) {
                    create_info_bad_usage.imageFormat = m_surface_formats[j].format;
                    create_info_bad_usage.imageUsage = kImageUsageFlags[i];
                    found_bad_usage = true;
                    break;
                }
            }
        }
    }
    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), m_device->graphics_queue_node_index_, m_surface.Handle(), &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present";
    }

    if (found_bad_usage) {
        m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-imageFormat-01778");
        vkt::Swapchain swapchain(*m_device, create_info_bad_usage);
        m_errorMonitor->VerifyFound();
    } else {
        printf(
            "could not find imageFormat and imageUsage values, supported by "
            "surface but unsupported by image, skipping test\n");
    }

    VkImageFormatProperties props;
    VkResult res = vk::GetPhysicalDeviceImageFormatProperties(Gpu(), good_create_info.imageFormat, VK_IMAGE_TYPE_2D,
                                                              VK_IMAGE_TILING_OPTIMAL, good_create_info.imageUsage,
                                                              VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT, &props);
    if (res != VK_SUCCESS) {
        GTEST_SKIP() << "Swapchain image format does not support SPLIT_INSTANCE_BIND_REGIONS";
    }

    VkSwapchainCreateInfoKHR create_info_bad_flags = good_create_info;
    create_info_bad_flags.flags = VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR;
    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-physicalDeviceCount-01429");
    vkt::Swapchain swapchain(*m_device, create_info_bad_flags);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, SwapchainAcquireImageNoSync) {
    TEST_DESCRIPTION("Test vkAcquireNextImageKHR with VK_NULL_HANDLE semaphore and fence");

    AddSurfaceExtension();

    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    {
        m_errorMonitor->SetDesiredError("VUID-vkAcquireNextImageKHR-semaphore-01780");
        m_swapchain.AcquireNextImage(vkt::no_semaphore, kWaitTimeout);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeWsi, SwapchainAcquireImageSignaledFence) {
    TEST_DESCRIPTION("Test vkAcquireNextImageKHR with signaled fence");

    AddSurfaceExtension();

    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    vkt::Fence fence(*m_device);

    vk::QueueSubmit(m_default_queue->handle(), 0, nullptr, fence.handle());

    vk::WaitForFences(device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);

    {
        m_errorMonitor->SetDesiredError("VUID-vkAcquireNextImageKHR-fence-01287");
        m_swapchain.AcquireNextImage(fence, kWaitTimeout);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeWsi, SwapchainAcquireImageSignaledFence2KHR) {
    TEST_DESCRIPTION("Test vkAcquireNextImage2KHR with signaled fence");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddSurfaceExtension();

    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    vkt::Fence fence(*m_device);

    vk::QueueSubmit(m_default_queue->handle(), 0, nullptr, fence.handle());

    vk::WaitForFences(device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);

    {
        m_errorMonitor->SetDesiredError("VUID-VkAcquireNextImageInfoKHR-fence-01289");
        VkAcquireNextImageInfoKHR acquire_info = vku::InitStructHelper();
        acquire_info.swapchain = m_swapchain;
        acquire_info.timeout = kWaitTimeout;
        acquire_info.semaphore = VK_NULL_HANDLE;
        acquire_info.fence = fence.handle();
        acquire_info.deviceMask = 0x1;

        uint32_t dummy;
        vk::AcquireNextImage2KHR(device(), &acquire_info, &dummy);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeWsi, SwapchainAcquireImageNoSync2KHR) {
    TEST_DESCRIPTION("Test vkAcquireNextImage2KHR with VK_NULL_HANDLE semaphore and fence");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddSurfaceExtension();

    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    {
        m_errorMonitor->SetDesiredError("VUID-VkAcquireNextImageInfoKHR-semaphore-01782");
        VkAcquireNextImageInfoKHR acquire_info = vku::InitStructHelper();
        acquire_info.swapchain = m_swapchain;
        acquire_info.timeout = kWaitTimeout;
        acquire_info.semaphore = VK_NULL_HANDLE;
        acquire_info.fence = VK_NULL_HANDLE;
        acquire_info.deviceMask = 0x1;

        uint32_t dummy;
        vk::AcquireNextImage2KHR(device(), &acquire_info, &dummy);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeWsi, SwapchainAcquireImageNoBinarySemaphore) {
    TEST_DESCRIPTION("Test vkAcquireNextImageKHR with non-binary semaphore");

    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    VkSemaphoreTypeCreateInfo semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper(&semaphore_type_create_info);

    vkt::Semaphore semaphore(*m_device, semaphore_create_info);

    m_errorMonitor->SetDesiredError("VUID-vkAcquireNextImageKHR-semaphore-03265");
    m_swapchain.AcquireNextImage(semaphore, kWaitTimeout);  // result is not needed
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, SwapchainAcquireImageNoBinarySemaphore2KHR) {
    TEST_DESCRIPTION("Test vkAcquireNextImage2KHR with non-binary semaphore");

    TEST_DESCRIPTION("Test vkAcquireNextImage2KHR with VK_NULL_HANDLE semaphore and fence");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    VkSemaphoreTypeCreateInfo semaphore_type_create_info = vku::InitStructHelper();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;

    VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper(&semaphore_type_create_info);

    vkt::Semaphore semaphore(*m_device, semaphore_create_info);

    VkAcquireNextImageInfoKHR acquire_info = vku::InitStructHelper();
    acquire_info.swapchain = m_swapchain;
    acquire_info.timeout = kWaitTimeout;
    acquire_info.semaphore = semaphore.handle();
    acquire_info.deviceMask = 0x1;

    m_errorMonitor->SetDesiredError("VUID-VkAcquireNextImageInfoKHR-semaphore-03266");
    uint32_t image_i;
    vk::AcquireNextImage2KHR(device(), &acquire_info, &image_i);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, SwapchainAcquireTooManyImages) {
    TEST_DESCRIPTION("Acquiring invalid amount of images from the swapchain.");

    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());
    const uint32_t image_count = m_swapchain.GetImageCount();

    VkSurfaceCapabilitiesKHR caps;
    ASSERT_EQ(VK_SUCCESS, vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(Gpu(), m_surface.Handle(), &caps));

    const uint32_t acquirable_count = image_count - caps.minImageCount + 1;
    std::vector<vkt::Fence> fences(acquirable_count);
    for (uint32_t i = 0; i < acquirable_count; ++i) {
        fences[i].Init(*m_device);
        VkResult res{};
        m_swapchain.AcquireNextImage(fences[i], kWaitTimeout, &res);
        ASSERT_TRUE(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR);
    }
    vkt::Fence error_fence(*m_device);

    m_errorMonitor->SetDesiredError("VUID-vkAcquireNextImageKHR-surface-07783");
    // NOTE: timeout MUST be UINT64_MAX to trigger the VUID
    m_swapchain.AcquireNextImage(error_fence, vvl::kU64Max);
    m_errorMonitor->VerifyFound();

    // Cleanup
    vk::WaitForFences(device(), fences.size(), MakeVkHandles<VkFence>(fences).data(), VK_TRUE, kWaitTimeout);
}

TEST_F(NegativeWsi, GetSwapchainImageAndTryDestroy) {
    TEST_DESCRIPTION("Try destroying a swapchain presentable image with vkDestroyImage");

    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());
    const auto images = m_swapchain.GetImages();

    m_errorMonitor->SetDesiredError("VUID-vkDestroyImage-image-04882");
    vk::DestroyImage(device(), images.at(0), nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, SwapchainNotSupported) {
    TEST_DESCRIPTION("Test creating a swapchain when GetPhysicalDeviceSurfaceSupportKHR returns VK_FALSE");

    AddSurfaceExtension();

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    // in "issue" section of VK_KHR_android_surface it talks how querying support is not needed on Android
    // The validation layers currently don't validate this VUID for Android surfaces
    if (std::find(m_instance_extension_names.begin(), m_instance_extension_names.end(), VK_KHR_ANDROID_SURFACE_EXTENSION_NAME) !=
        m_instance_extension_names.end()) {
        GTEST_SKIP() << "Test does not run on Android Surface";
    }
#endif

    RETURN_IF_SKIP(InitFramework());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    std::vector<VkQueueFamilyProperties> queue_families;
    uint32_t count = 0;
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &count, nullptr);
    queue_families.resize(count);
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &count, queue_families.data());

    bool found = false;
    uint32_t qfi = 0;
    for (uint32_t i = 0; i < queue_families.size(); i++) {
        VkBool32 supported = VK_FALSE;
        vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), i, m_surface.Handle(), &supported);
        if (!supported) {
            found = true;
            qfi = i;
            break;
        }
    }

    if (!found) {
        GTEST_SKIP() << "All queues support surface present";
    }
    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_create_info = vku::InitStructHelper();
    queue_create_info.queueFamilyIndex = qfi;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;

    VkDeviceCreateInfo device_create_info = vku::InitStructHelper();
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.enabledExtensionCount = m_device_extension_names.size();
    device_create_info.ppEnabledExtensionNames = m_device_extension_names.data();

    vkt::Device test_device(Gpu(), device_create_info);

    // Initialize extensions manually because we don't use InitState() in this test
    for (const char *device_ext_name : m_device_extension_names) {
        vk::InitDeviceExtension(instance(), test_device, device_ext_name);
    }

    // try creating a swapchain, using surface info queried from the default device
    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.flags = 0;
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-surface-01270");
    m_swapchain.Init(test_device, swapchain_create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, SwapchainAcquireTooManyImages2KHR) {
    TEST_DESCRIPTION("Acquiring invalid amount of images from the swapchain via vkAcquireNextImage2KHR.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());
    const uint32_t image_count = m_swapchain.GetImageCount();

    VkSurfaceCapabilitiesKHR caps;
    ASSERT_EQ(VK_SUCCESS, vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(Gpu(), m_surface.Handle(), &caps));

    const uint32_t acquirable_count = image_count - caps.minImageCount + 1;
    std::vector<vkt::Fence> fences(acquirable_count);
    for (uint32_t i = 0; i < acquirable_count; ++i) {
        fences[i].Init(*m_device);
        VkResult res{};
        m_swapchain.AcquireNextImage(fences[i], kWaitTimeout, &res);
        ASSERT_TRUE(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR);
    }
    vkt::Fence error_fence(*m_device);

    m_errorMonitor->SetDesiredError("VUID-vkAcquireNextImage2KHR-surface-07784");
    VkAcquireNextImageInfoKHR acquire_info = vku::InitStructHelper();

    acquire_info.swapchain = m_swapchain;
    acquire_info.timeout = vvl::kU64Max;  // NOTE: timeout MUST be UINT64_MAX to trigger the VUID
    acquire_info.fence = error_fence.handle();
    acquire_info.deviceMask = 0x1;

    uint32_t image_i;
    vk::AcquireNextImage2KHR(device(), &acquire_info, &image_i);
    m_errorMonitor->VerifyFound();

    // Cleanup
    vk::WaitForFences(device(), fences.size(), MakeVkHandles<VkFence>(fences).data(), VK_TRUE, kWaitTimeout);
}

TEST_F(NegativeWsi, SwapchainImageFormatList) {
    TEST_DESCRIPTION("Test VK_KHR_image_format_list and VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR with swapchains");

    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), m_device->graphics_queue_node_index_, m_surface.Handle(), &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present";
    }

    // To make test use, assume a common surface format
    VkSurfaceFormatKHR valid_surface_format{VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_MAX_ENUM_KHR};
    VkSurfaceFormatKHR other_surface_format{VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_MAX_ENUM_KHR};
    for (VkSurfaceFormatKHR surface_format : m_surface_formats) {
        if (surface_format.format == VK_FORMAT_B8G8R8A8_UNORM) {
            valid_surface_format = surface_format;
            break;
        } else {
            other_surface_format = surface_format;
        }
    }
    if (valid_surface_format.format == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "Test requires VK_FORMAT_B8G8R8A8_UNORM as a supported surface format";
    }

    // Use sampled formats that will always be supported
    // Last format is not compatible with the rest
    const VkFormat formats[4] = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_R8G8B8A8_UINT, VK_FORMAT_R8_UNORM};
    VkImageFormatListCreateInfo format_list = vku::InitStructHelper();
    format_list.viewFormatCount = 3;  // first 3 are compatible
    format_list.pViewFormats = formats;

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper(&format_list);
    swapchain_create_info.flags = 0;
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = valid_surface_format.format;
    swapchain_create_info.imageColorSpace = valid_surface_format.colorSpace;
    swapchain_create_info.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    // No mutable flag
    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-flags-04100");
    m_swapchain.Init(*m_device, swapchain_create_info);
    m_errorMonitor->VerifyFound();
    swapchain_create_info.flags = VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR;

    // Last format is not compatible
    format_list.viewFormatCount = 4;
    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-pNext-04099");
    m_swapchain.Init(*m_device, swapchain_create_info);
    m_errorMonitor->VerifyFound();

    // viewFormatCount of 0
    format_list.viewFormatCount = 0;
    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-flags-03168");
    m_swapchain.Init(*m_device, swapchain_create_info);
    m_errorMonitor->VerifyFound();
    format_list.viewFormatCount = 3;  // valid

    // missing pNext
    swapchain_create_info.pNext = nullptr;
    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-flags-03168");
    m_swapchain.Init(*m_device, swapchain_create_info);
    m_errorMonitor->VerifyFound();
    swapchain_create_info.pNext = &format_list;

    // Another surface format is available and is not in list of viewFormats
    if (other_surface_format.format != VK_FORMAT_UNDEFINED) {
        swapchain_create_info.imageFormat = other_surface_format.format;
        swapchain_create_info.imageColorSpace = other_surface_format.colorSpace;
        m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-flags-03168");
        m_swapchain.Init(*m_device, swapchain_create_info);
        m_errorMonitor->VerifyFound();
        swapchain_create_info.imageFormat = valid_surface_format.format;
        swapchain_create_info.imageColorSpace = valid_surface_format.colorSpace;
    }

    m_swapchain.Init(*m_device, swapchain_create_info);
}

TEST_F(NegativeWsi, SwapchainMinImageCountNonShared) {
    TEST_DESCRIPTION("Use invalid minImageCount for non shared swapchain creation");
    AddSurfaceExtension();

    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();
    if (m_surface_capabilities.minImageCount <= 1) {
        GTEST_SKIP() << "minImageCount is not at least 2";
    }

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), m_device->graphics_queue_node_index_, m_surface.Handle(), &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present";
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = 1;  // invalid
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-presentMode-02839");
    m_swapchain.Init(*m_device, swapchain_create_info);
    m_errorMonitor->VerifyFound();

    // Sanity check
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    m_swapchain.Init(*m_device, swapchain_create_info);
}

TEST_F(NegativeWsi, SwapchainMinImageCountShared) {
    TEST_DESCRIPTION("Use invalid minImageCount for shared swapchain creation");

    AddRequiredExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME);
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), m_device->graphics_queue_node_index_, m_surface.Handle(), &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present";
    }

    VkPresentModeKHR shared_present_mode = m_surface_non_shared_present_mode;
    for (size_t i = 0; i < m_surface_present_modes.size(); i++) {
        const VkPresentModeKHR present_mode = m_surface_present_modes[i];
        if ((present_mode == VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR) ||
            (present_mode == VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR)) {
            shared_present_mode = present_mode;
            break;
        }
    }
    if (shared_present_mode == m_surface_non_shared_present_mode) {
        GTEST_SKIP() << "Cannot find supported shared present mode";
    }

    VkSharedPresentSurfaceCapabilitiesKHR shared_present_capabilities = vku::InitStructHelper();
    VkSurfaceCapabilities2KHR capabilities = vku::InitStructHelper(&shared_present_capabilities);
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper();
    surface_info.surface = m_surface.Handle();
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(Gpu(), &surface_info, &capabilities);

    // This was recently added to CTS, but some drivers might not correctly advertise the flag
    if ((shared_present_capabilities.sharedPresentSupportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == 0) {
        GTEST_SKIP() << "Driver was suppose to support VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT";
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = 2;  // invalid
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // implementations must support
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-minImageCount-01383");
    m_swapchain.Init(*m_device, swapchain_create_info);
    m_errorMonitor->VerifyFound();

    // Sanity check
    swapchain_create_info.minImageCount = 1;
    m_swapchain.Init(*m_device, swapchain_create_info);
}

TEST_F(NegativeWsi, SwapchainUsageNonShared) {
    TEST_DESCRIPTION("Use invalid imageUsage for non-shared swapchain creation");
    AddSurfaceExtension();

    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), m_device->graphics_queue_node_index_, m_surface.Handle(), &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present";
    }

    // No implementation should support depth/stencil for swapchain
    if ((m_surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) {
        GTEST_SKIP() << "Test has supported usage already the test is using";
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-presentMode-01427");
    m_swapchain.Init(*m_device, swapchain_create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, SwapchainUsageShared) {
    TEST_DESCRIPTION("Use invalid imageUsage for shared swapchain creation");

    AddRequiredExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME);
    AddSurfaceExtension();

    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), m_device->graphics_queue_node_index_, m_surface.Handle(), &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present";
    }

    VkPresentModeKHR shared_present_mode = m_surface_non_shared_present_mode;
    for (size_t i = 0; i < m_surface_present_modes.size(); i++) {
        const VkPresentModeKHR present_mode = m_surface_present_modes[i];
        if ((present_mode == VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR) ||
            (present_mode == VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR)) {
            shared_present_mode = present_mode;
            break;
        }
    }
    if (shared_present_mode == m_surface_non_shared_present_mode) {
        GTEST_SKIP() << "Cannot find supported shared present mode";
    }

    VkSharedPresentSurfaceCapabilitiesKHR shared_present_capabilities = vku::InitStructHelper();
    VkSurfaceCapabilities2KHR capabilities = vku::InitStructHelper(&shared_present_capabilities);
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper();
    surface_info.surface = m_surface.Handle();
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(Gpu(), &surface_info, &capabilities);

    // No implementation should support depth/stencil for swapchain
    if ((shared_present_capabilities.sharedPresentSupportedUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) {
        GTEST_SKIP() << "Test has supported usage already the test is using";
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = 1;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-imageUsage-01384");
    m_swapchain.Init(*m_device, swapchain_create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, SwapchainPresentShared) {
    TEST_DESCRIPTION("Present shared presentable image without Acquire to generate failure.");

    AddRequiredExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME);
    AddSurfaceExtension();

    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), m_device->graphics_queue_node_index_, m_surface.Handle(), &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present";
    }

    VkPresentModeKHR shared_present_mode = m_surface_non_shared_present_mode;
    for (size_t i = 0; i < m_surface_present_modes.size(); i++) {
        const VkPresentModeKHR present_mode = m_surface_present_modes[i];
        if ((present_mode == VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR) ||
            (present_mode == VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR)) {
            shared_present_mode = present_mode;
            break;
        }
    }
    if (shared_present_mode == m_surface_non_shared_present_mode) {
        GTEST_SKIP() << "Cannot find supported shared present mode";
    }

    VkSharedPresentSurfaceCapabilitiesKHR shared_present_capabilities = vku::InitStructHelper();
    VkSurfaceCapabilities2KHR capabilities = vku::InitStructHelper(&shared_present_capabilities);
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper();
    surface_info.surface = m_surface.Handle();
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(Gpu(), &surface_info, &capabilities);

    // This was recently added to CTS, but some drivers might not correctly advertise the flag
    if ((shared_present_capabilities.sharedPresentSupportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == 0) {
        GTEST_SKIP() << "Driver was suppose to support VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT";
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = 1;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // implementations must support
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    m_swapchain.Init(*m_device, swapchain_create_info);

    const auto images = m_swapchain.GetImages();
    uint32_t image_index = 0;

    // Try to Present without Acquire...
    m_errorMonitor->SetDesiredError("VUID-VkPresentInfoKHR-pImageIndices-01430");
    m_default_queue->Present(m_swapchain, image_index, vkt::no_semaphore);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, DeviceMask) {
    TEST_DESCRIPTION("Invalid deviceMask.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddSurfaceExtension();

    RETURN_IF_SKIP(InitFramework());

    const auto physical_device_group = FindPhysicalDeviceGroup();
    if (!physical_device_group.has_value()) {
        GTEST_SKIP() << "cannot find physical device group that contains selected physical device";
    }

    VkDeviceGroupDeviceCreateInfo create_device_pnext = vku::InitStructHelper();
    create_device_pnext.physicalDeviceCount = physical_device_group->physicalDeviceCount;
    create_device_pnext.pPhysicalDevices = physical_device_group->physicalDevices;
    RETURN_IF_SKIP(InitState(nullptr, &create_device_pnext));
    InitRenderTarget();
    RETURN_IF_SKIP(InitSwapchain());

    // Test VkMemoryAllocateFlagsInfo
    VkMemoryAllocateFlagsInfo alloc_flags_info = vku::InitStructHelper();
    alloc_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_MASK_BIT;
    alloc_flags_info.deviceMask = 0xFFFFFFFF;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags_info);
    alloc_info.memoryTypeIndex = 0;
    alloc_info.allocationSize = 1024;

    VkDeviceMemory mem;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateFlagsInfo-deviceMask-00675");
    vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
    m_errorMonitor->VerifyFound();

    alloc_flags_info.deviceMask = 0;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateFlagsInfo-deviceMask-00676");
    vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
    m_errorMonitor->VerifyFound();

    uint32_t pdev_group_count = 0;
    VkResult err = vk::EnumeratePhysicalDeviceGroups(instance(), &pdev_group_count, nullptr);
    // TODO: initialization can be removed once https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/4138 merges
    std::vector<VkPhysicalDeviceGroupProperties> group_props(pdev_group_count,
                                                             {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    err = vk::EnumeratePhysicalDeviceGroups(instance(), &pdev_group_count, &group_props[0]);

    auto tgt = Gpu();
    bool test_run = false;
    for (uint32_t i = 0; i < pdev_group_count; i++) {
        if ((group_props[i].physicalDeviceCount > 1) && !test_run) {
            for (uint32_t j = 0; j < group_props[i].physicalDeviceCount; j++) {
                if (tgt == group_props[i].physicalDevices[j]) {
                    void *data;
                    VkDeviceMemory mi_mem;
                    alloc_flags_info.deviceMask = 3;
                    err = vk::AllocateMemory(device(), &alloc_info, NULL, &mi_mem);
                    if (VK_SUCCESS == err) {
                        m_errorMonitor->SetDesiredError("VUID-vkMapMemory-memory-00683");
                        vk::MapMemory(device(), mi_mem, 0, 1024, 0, &data);
                        m_errorMonitor->VerifyFound();
                        vk::FreeMemory(device(), mi_mem, nullptr);
                    }
                    test_run = true;
                    break;
                }
            }
        }
    }

    // Test VkDeviceGroupCommandBufferBeginInfo
    VkDeviceGroupCommandBufferBeginInfo dev_grp_cmd_buf_info = vku::InitStructHelper();
    dev_grp_cmd_buf_info.deviceMask = 0xFFFFFFFF;
    VkCommandBufferBeginInfo cmd_buf_info = vku::InitStructHelper(&dev_grp_cmd_buf_info);

    m_command_buffer.Reset();
    m_errorMonitor->SetDesiredError("VUID-VkDeviceGroupCommandBufferBeginInfo-deviceMask-00106");
    vk::BeginCommandBuffer(m_command_buffer.handle(), &cmd_buf_info);
    m_errorMonitor->VerifyFound();

    dev_grp_cmd_buf_info.deviceMask = 0;
    m_command_buffer.Reset();
    m_errorMonitor->SetDesiredError("VUID-VkDeviceGroupCommandBufferBeginInfo-deviceMask-00107");
    vk::BeginCommandBuffer(m_command_buffer.handle(), &cmd_buf_info);
    m_errorMonitor->VerifyFound();

    // Test VkDeviceGroupRenderPassBeginInfo
    dev_grp_cmd_buf_info.deviceMask = 0x00000001;
    m_command_buffer.Reset();
    vk::BeginCommandBuffer(m_command_buffer.handle(), &cmd_buf_info);

    VkDeviceGroupRenderPassBeginInfo dev_grp_rp_info = vku::InitStructHelper();
    dev_grp_rp_info.deviceMask = 0xFFFFFFFF;
    m_renderPassBeginInfo.pNext = &dev_grp_rp_info;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceGroupRenderPassBeginInfo-deviceMask-00905");
    m_errorMonitor->SetDesiredError("VUID-VkDeviceGroupRenderPassBeginInfo-deviceMask-00907");
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();

    dev_grp_rp_info.deviceMask = 0;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceGroupRenderPassBeginInfo-deviceMask-00906");
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();

    dev_grp_rp_info.deviceMask = 0x00000001;
    dev_grp_rp_info.deviceRenderAreaCount = physical_device_group->physicalDeviceCount + 1;
    std::vector<VkRect2D> device_render_areas(dev_grp_rp_info.deviceRenderAreaCount, m_renderPassBeginInfo.renderArea);
    dev_grp_rp_info.pDeviceRenderAreas = device_render_areas.data();

    m_errorMonitor->SetDesiredError("VUID-VkDeviceGroupRenderPassBeginInfo-deviceRenderAreaCount-00908");
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();

    // Test vk::CmdSetDeviceMask()
    vk::CmdSetDeviceMask(m_command_buffer.handle(), 0x00000001);

    dev_grp_rp_info.deviceRenderAreaCount = physical_device_group->physicalDeviceCount;
    vk::CmdBeginRenderPass(m_command_buffer.handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetDeviceMask-deviceMask-00108");
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetDeviceMask-deviceMask-00110");
    m_errorMonitor->SetDesiredError("VUID-vkCmdSetDeviceMask-deviceMask-00111");
    vk::CmdSetDeviceMask(m_command_buffer.handle(), 0xFFFFFFFF);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetDeviceMask-deviceMask-00109");
    vk::CmdSetDeviceMask(m_command_buffer.handle(), 0);
    m_errorMonitor->VerifyFound();

    vkt::Semaphore semaphore(*m_device), semaphore2(*m_device);
    vkt::Fence fence(*m_device);

    // Test VkAcquireNextImageInfoKHR
    uint32_t imageIndex;
    VkAcquireNextImageInfoKHR acquire_next_image_info = vku::InitStructHelper();
    acquire_next_image_info.semaphore = semaphore.handle();
    acquire_next_image_info.swapchain = m_swapchain;
    acquire_next_image_info.fence = fence.handle();
    acquire_next_image_info.deviceMask = 0xFFFFFFFF;

    m_errorMonitor->SetDesiredError("VUID-VkAcquireNextImageInfoKHR-deviceMask-01290");
    vk::AcquireNextImage2KHR(device(), &acquire_next_image_info, &imageIndex);
    m_errorMonitor->VerifyFound();

    // NOTE: We cannot wait on fence in this test because all of the acquire calls fail.

    acquire_next_image_info.semaphore = semaphore2.handle();
    acquire_next_image_info.deviceMask = 0;

    m_errorMonitor->SetDesiredError("VUID-VkAcquireNextImageInfoKHR-deviceMask-01291");
    vk::AcquireNextImage2KHR(device(), &acquire_next_image_info, &imageIndex);
    m_errorMonitor->VerifyFound();

    // Test VkDeviceGroupSubmitInfo
    VkDeviceGroupSubmitInfo device_group_submit_info = vku::InitStructHelper();
    device_group_submit_info.commandBufferCount = 1;
    std::array<uint32_t, 1> command_buffer_device_masks = {{0xFFFFFFFF}};
    device_group_submit_info.pCommandBufferDeviceMasks = command_buffer_device_masks.data();

    VkSubmitInfo submit_info = vku::InitStructHelper(&device_group_submit_info);
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffer.handle();

    m_command_buffer.Reset();
    vk::BeginCommandBuffer(m_command_buffer.handle(), &cmd_buf_info);
    vk::EndCommandBuffer(m_command_buffer.handle());
    m_errorMonitor->SetDesiredError("VUID-VkDeviceGroupSubmitInfo-pCommandBufferDeviceMasks-00086");
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeWsi, DisplayPlaneSurface) {
    TEST_DESCRIPTION("Create and use VkDisplayKHR objects to test VkDisplaySurfaceCreateInfoKHR.");

    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_DISPLAY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());

    uint32_t plane_prop_count = 0;
    vk::GetPhysicalDeviceDisplayPlanePropertiesKHR(Gpu(), &plane_prop_count, nullptr);
    if (plane_prop_count == 0) {
        GTEST_SKIP() << "Test requires at least 1 supported display plane property";
    }
    std::vector<VkDisplayPlanePropertiesKHR> display_plane_props(plane_prop_count);
    vk::GetPhysicalDeviceDisplayPlanePropertiesKHR(Gpu(), &plane_prop_count, display_plane_props.data());
    // using plane 0 for rest of test
    VkDisplayKHR current_display = display_plane_props[0].currentDisplay;
    if (current_display == VK_NULL_HANDLE) {
        GTEST_SKIP() << "VkDisplayPlanePropertiesKHR[0].currentDisplay is not attached to device";
    }

    uint32_t mode_prop_count = 0;
    vk::GetDisplayModePropertiesKHR(Gpu(), current_display, &mode_prop_count, nullptr);
    if (plane_prop_count == 0) {
        GTEST_SKIP() << "test requires at least 1 supported display mode property";
    }
    std::vector<VkDisplayModePropertiesKHR> display_mode_props(mode_prop_count);
    vk::GetDisplayModePropertiesKHR(Gpu(), current_display, &mode_prop_count, display_mode_props.data());

    uint32_t plane_count;
    m_errorMonitor->SetDesiredError("VUID-vkGetDisplayPlaneSupportedDisplaysKHR-planeIndex-01249");
    vk::GetDisplayPlaneSupportedDisplaysKHR(Gpu(), plane_prop_count, &plane_count, nullptr);
    m_errorMonitor->VerifyFound();
    ASSERT_EQ(VK_SUCCESS, vk::GetDisplayPlaneSupportedDisplaysKHR(Gpu(), 0, &plane_count, nullptr));
    if (plane_count == 0) {
        GTEST_SKIP() << "test requires at least 1 supported display plane";
    }
    std::vector<VkDisplayKHR> supported_displays(plane_count);
    plane_count = 1;
    ASSERT_EQ(VK_SUCCESS, vk::GetDisplayPlaneSupportedDisplaysKHR(Gpu(), 0, &plane_count, supported_displays.data()));
    if (supported_displays[0] != current_display) {
        GTEST_SKIP() << "Current VkDisplayKHR used is not supported";
    }

    VkDisplayModeKHR display_mode;
    VkDisplayModeParametersKHR display_mode_parameters = {{0, 0}, 0};
    VkDisplayModeCreateInfoKHR display_mode_info = {VK_STRUCTURE_TYPE_DISPLAY_MODE_CREATE_INFO_KHR, nullptr, 0,
                                                    display_mode_parameters};
    m_errorMonitor->SetDesiredError("VUID-VkDisplayModeParametersKHR-width-01990");
    m_errorMonitor->SetDesiredError("VUID-VkDisplayModeParametersKHR-height-01991");
    m_errorMonitor->SetDesiredError("VUID-VkDisplayModeParametersKHR-refreshRate-01992");
    vk::CreateDisplayModeKHR(Gpu(), current_display, &display_mode_info, nullptr, &display_mode);
    m_errorMonitor->VerifyFound();
    // Use the first good parameter queried
    display_mode_info.parameters = display_mode_props[0].parameters;
    VkResult result = vk::CreateDisplayModeKHR(Gpu(), current_display, &display_mode_info, nullptr, &display_mode);
    if (result != VK_SUCCESS) {
        GTEST_SKIP() << "test failed to create a display mode with vkCreateDisplayModeKHR";
    }

    VkDisplayPlaneCapabilitiesKHR plane_capabilities;
    ASSERT_EQ(VK_SUCCESS, vk::GetDisplayPlaneCapabilitiesKHR(Gpu(), display_mode, 0, &plane_capabilities));

    VkSurfaceKHR surface;
    VkDisplaySurfaceCreateInfoKHR display_surface_info = vku::InitStructHelper();
    display_surface_info.flags = 0;
    display_surface_info.displayMode = display_mode;
    display_surface_info.planeIndex = 0;
    display_surface_info.planeStackIndex = 0;
    display_surface_info.transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    display_surface_info.imageExtent = {8, 8};
    display_surface_info.globalAlpha = 1.0f;

    // Test if the device doesn't support the bits
    if ((plane_capabilities.supportedAlpha & VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR) == 0) {
        display_surface_info.alphaMode = VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR;
        m_errorMonitor->SetDesiredError("VUID-VkDisplaySurfaceCreateInfoKHR-alphaMode-01255");
        vk::CreateDisplayPlaneSurfaceKHR(instance(), &display_surface_info, nullptr, &surface);
        m_errorMonitor->VerifyFound();
    }
    if ((plane_capabilities.supportedAlpha & VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_PREMULTIPLIED_BIT_KHR) == 0) {
        display_surface_info.alphaMode = VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_PREMULTIPLIED_BIT_KHR;
        m_errorMonitor->SetDesiredError("VUID-VkDisplaySurfaceCreateInfoKHR-alphaMode-01255");
        vk::CreateDisplayPlaneSurfaceKHR(instance(), &display_surface_info, nullptr, &surface);
        m_errorMonitor->VerifyFound();
    }

    display_surface_info.globalAlpha = 2.0f;
    display_surface_info.alphaMode = VK_DISPLAY_PLANE_ALPHA_GLOBAL_BIT_KHR;
    if ((plane_capabilities.supportedAlpha & VK_DISPLAY_PLANE_ALPHA_GLOBAL_BIT_KHR) == 0) {
        m_errorMonitor->SetDesiredError("VUID-VkDisplaySurfaceCreateInfoKHR-alphaMode-01255");
    }
    m_errorMonitor->SetDesiredError("VUID-VkDisplaySurfaceCreateInfoKHR-alphaMode-01254");
    vk::CreateDisplayPlaneSurfaceKHR(instance(), &display_surface_info, nullptr, &surface);
    m_errorMonitor->VerifyFound();

    display_surface_info.alphaMode = VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR;
    display_surface_info.planeIndex = plane_prop_count;
    m_errorMonitor->SetDesiredError("VUID-VkDisplaySurfaceCreateInfoKHR-planeIndex-01252");
    vk::CreateDisplayPlaneSurfaceKHR(instance(), &display_surface_info, nullptr, &surface);
    m_errorMonitor->VerifyFound();
    display_surface_info.planeIndex = 0;  // restore to good value

    uint32_t bad_size = m_device->Physical().limits_.maxImageDimension2D + 1;
    display_surface_info.imageExtent = {bad_size, bad_size};
    // one for height and width
    m_errorMonitor->SetDesiredError("VUID-VkDisplaySurfaceCreateInfoKHR-width-01256");
    m_errorMonitor->SetDesiredError("VUID-VkDisplaySurfaceCreateInfoKHR-width-01256");
    vk::CreateDisplayPlaneSurfaceKHR(instance(), &display_surface_info, nullptr, &surface);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, WarningSwapchainCreateInfoPreTransform) {
    TEST_DESCRIPTION("Print warning when preTransform doesn't match curretTransform");

    AddSurfaceExtension();

    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitRenderTarget();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "WARNING-Swapchain-PreTransform");
    m_errorMonitor->SetUnexpectedError("VUID-VkSwapchainCreateInfoKHR-preTransform-01279");
    m_swapchain = CreateSwapchain(m_surface.Handle(), VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, DeviceGroupSubmitInfoSemaphoreCount) {
    TEST_DESCRIPTION("Test semaphoreCounts in DeviceGroupSubmitInfo");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    const auto physical_device_group = FindPhysicalDeviceGroup();
    if (!physical_device_group.has_value()) {
        GTEST_SKIP() << "cannot find physical device group that contains selected physical device";
    }

    VkDeviceGroupDeviceCreateInfo create_device_pnext = vku::InitStructHelper();
    create_device_pnext.physicalDeviceCount = physical_device_group->physicalDeviceCount;
    create_device_pnext.pPhysicalDevices = physical_device_group->physicalDevices;
    RETURN_IF_SKIP(InitState(nullptr, &create_device_pnext));

    VkDeviceGroupCommandBufferBeginInfo dev_grp_cmd_buf_info = vku::InitStructHelper();
    dev_grp_cmd_buf_info.deviceMask = 0x1;
    VkCommandBufferBeginInfo cmd_buf_info = vku::InitStructHelper(&dev_grp_cmd_buf_info);

    vkt::Semaphore semaphore(*m_device);

    VkDeviceGroupSubmitInfo device_group_submit_info = vku::InitStructHelper();
    device_group_submit_info.commandBufferCount = 1;
    uint32_t command_buffer_device_masks = 0;
    device_group_submit_info.pCommandBufferDeviceMasks = &command_buffer_device_masks;

    VkSubmitInfo submit_info = vku::InitStructHelper(&device_group_submit_info);
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffer.handle();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore.handle();

    m_command_buffer.Reset();
    vk::BeginCommandBuffer(m_command_buffer.handle(), &cmd_buf_info);
    vk::EndCommandBuffer(m_command_buffer.handle());
    m_errorMonitor->SetDesiredError("VUID-VkDeviceGroupSubmitInfo-signalSemaphoreCount-00084");
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();

    VkSubmitInfo signal_submit_info = vku::InitStructHelper();
    signal_submit_info.signalSemaphoreCount = 1;
    signal_submit_info.pSignalSemaphores = &semaphore.handle();
    vk::QueueSubmit(m_default_queue->handle(), 1, &signal_submit_info, VK_NULL_HANDLE);

    VkPipelineStageFlags waitMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    submit_info.pWaitDstStageMask = &waitMask;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = nullptr;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceGroupSubmitInfo-waitSemaphoreCount-00082");
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    submit_info.waitSemaphoreCount = 0;
    submit_info.commandBufferCount = 0;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceGroupSubmitInfo-commandBufferCount-00083");
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Need to wait for semaphore to not be in use before destroying it
    m_default_queue->Wait();
}

TEST_F(NegativeWsi, SwapchainAcquireImageWithSignaledSemaphore) {
    TEST_DESCRIPTION("Test vkAcquireNextImageKHR with signaled semaphore");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddSurfaceExtension();

    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    vkt::Semaphore semaphore(*m_device);
    m_default_queue->Submit(vkt::no_cmd, vkt::Signal(semaphore));
    m_default_queue->Wait();

    VkAcquireNextImageInfoKHR acquire_info = vku::InitStructHelper();
    acquire_info.swapchain = m_swapchain;
    acquire_info.timeout = kWaitTimeout;
    acquire_info.semaphore = semaphore.handle();
    acquire_info.fence = VK_NULL_HANDLE;
    acquire_info.deviceMask = 0x1;

    uint32_t dummy;
    m_errorMonitor->SetDesiredError("VUID-vkAcquireNextImageKHR-semaphore-01286");
    m_swapchain.AcquireNextImage(semaphore, kWaitTimeout);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-VkAcquireNextImageInfoKHR-semaphore-01288");
    vk::AcquireNextImage2KHR(device(), &acquire_info, &dummy);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, SwapchainAcquireImageWithPendingSemaphoreWait) {
    TEST_DESCRIPTION("Test vkAcquireNextImageKHR with pending semaphore wait operation");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddSurfaceExtension();

    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    vkt::Semaphore semaphore(*m_device);
    m_default_queue->Submit(vkt::no_cmd, vkt::Signal(semaphore));

    // Add a wait, but don't let it finish.
    m_default_queue->Submit(vkt::no_cmd, vkt::Wait(semaphore, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT));

    m_errorMonitor->SetDesiredError("VUID-vkAcquireNextImageKHR-semaphore-01779");
    m_swapchain.AcquireNextImage(semaphore, kWaitTimeout);
    m_errorMonitor->VerifyFound();

    VkAcquireNextImageInfoKHR acquire_info = vku::InitStructHelper();
    acquire_info.swapchain = m_swapchain;
    acquire_info.timeout = kWaitTimeout;
    acquire_info.semaphore = semaphore.handle();
    acquire_info.fence = VK_NULL_HANDLE;
    acquire_info.deviceMask = 0x1;

    uint32_t dummy;
    m_errorMonitor->SetDesiredError("VUID-VkAcquireNextImageInfoKHR-semaphore-01781");
    vk::AcquireNextImage2KHR(device(), &acquire_info, &dummy);
    m_errorMonitor->VerifyFound();

    // finish the wait
    m_default_queue->Wait();

    // now it should be possible to acquire
    m_swapchain.AcquireNextImage(semaphore, kWaitTimeout);
}

TEST_F(NegativeWsi, DisplayPresentInfoSrcRect) {
    TEST_DESCRIPTION("Test layout tracking on imageless framebuffers");
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
    InitRenderTarget();

    vkt::Semaphore image_acquired(*m_device);
    const uint32_t current_buffer = m_swapchain.AcquireNextImage(image_acquired, kWaitTimeout);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();

    uint32_t swapchain_width = m_surface_capabilities.minImageExtent.width;
    uint32_t swapchain_height = m_surface_capabilities.minImageExtent.height;

    VkDisplayPresentInfoKHR display_present_info = vku::InitStructHelper();
    display_present_info.srcRect.extent.width = swapchain_width + 1;  // Invalid
    display_present_info.srcRect.extent.height = swapchain_height;
    display_present_info.dstRect.extent.width = swapchain_width;
    display_present_info.dstRect.extent.height = swapchain_height;

    m_errorMonitor->SetDesiredError("VUID-VkDisplayPresentInfoKHR-srcRect-01257");
    m_errorMonitor->SetDesiredError("VUID-VkPresentInfoKHR-pImageIndices-01430");
    m_default_queue->Present(m_swapchain, current_buffer, image_acquired, &display_present_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, LeakASwapchain) {
    TEST_DESCRIPTION("Leak a VkSwapchainKHR.");
    // Because this test intentionally leaks swapchains & surfaces, we need to disable leak checking because drivers may leak memory
    // that cannot be cleaned up from this test.
#if defined(VVL_ENABLE_ASAN)
    auto leak_sanitizer_disabler = __lsan::ScopedDisabler();
#endif

    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    if (!IsPlatformMockICD()) {
        // This test leaks a swapchain (on purpose) and should not be run on a real driver
        GTEST_SKIP() << "This test only runs on the mock ICD";
    }

    SurfaceContext surface_context{};
    vkt::Surface surface{};
    if (CreateSurface(surface_context, surface) != VK_SUCCESS) {
        GTEST_SKIP() << "Cannot create required surface";
    }

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), m_device->graphics_queue_node_index_, surface.Handle(), &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present, skipping test";
    }

    SurfaceInformation info = GetSwapchainInfo(surface.Handle());

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.surface = surface.Handle();
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

    VkSwapchainKHR swapchain_handle = VK_NULL_HANDLE;
    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &swapchain_handle);

    // Warn about the surface/swapchain not being destroyed
    m_errorMonitor->SetDesiredError("VUID-vkDestroyInstance-instance-00629");
    m_errorMonitor->SetDesiredError("VUID-vkDestroyDevice-device-05137");
    ShutdownFramework();  // Destroy Instance/Device
    m_errorMonitor->VerifyFound();

    surface.DestroyExplicitly();
}

TEST_F(NegativeWsi, PresentIdWait) {
    TEST_DESCRIPTION("Test present wait extension");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_PRESENT_WAIT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PRESENT_ID_EXTENSION_NAME);
    AddSurfaceExtension();
    AddRequiredFeature(vkt::Feature::presentId);
    AddRequiredFeature(vkt::Feature::presentWait);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    SurfaceContext surface_context;
    vkt::Surface surface2;
    ASSERT_EQ(VK_SUCCESS, CreateSurface(surface_context, surface2));
    vkt::Swapchain swapchain2 =
        CreateSwapchain(surface2.Handle(), VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);
    ASSERT_TRUE(swapchain2.initialized());

    auto images = m_swapchain.GetImages();
    auto images2 = swapchain2.GetImages();

    uint32_t image_indices[2];
    vkt::Fence fence(*m_device);
    vkt::Fence fence2(*m_device);
    VkFence fence_handles[2];
    fence_handles[0] = fence.handle();
    fence_handles[1] = fence2.handle();

    image_indices[0] = m_swapchain.AcquireNextImage(fence, kWaitTimeout);
    image_indices[1] = swapchain2.AcquireNextImage(fence2, kWaitTimeout);
    vk::WaitForFences(device(), 2, fence_handles, true, kWaitTimeout);
    SetImageLayoutPresentSrc(images[image_indices[0]]);
    SetImageLayoutPresentSrc(images2[image_indices[1]]);

    VkSwapchainKHR swap_chains[2] = {m_swapchain, swapchain2};
    uint64_t present_ids[2] = {};
    present_ids[0] = 4;  // Try setting 3 later
    VkPresentIdKHR present_id = vku::InitStructHelper();
    present_id.swapchainCount = 2;
    present_id.pPresentIds = present_ids;
    VkPresentInfoKHR present = vku::InitStructHelper(&present_id);
    present.pSwapchains = swap_chains;
    present.pImageIndices = image_indices;
    present.swapchainCount = 2;

    // Submit a clean present to establish presentIds
    vk::QueuePresentKHR(m_default_queue->handle(), &present);

    vk::ResetFences(device(), 2, fence_handles);
    image_indices[0] = m_swapchain.AcquireNextImage(fence, kWaitTimeout);
    image_indices[1] = swapchain2.AcquireNextImage(fence2, kWaitTimeout);
    vk::WaitForFences(device(), 2, fence_handles, true, kWaitTimeout);
    SetImageLayoutPresentSrc(images[image_indices[0]]);
    SetImageLayoutPresentSrc(images2[image_indices[1]]);

    // presentIds[0] = 3 (smaller than 4), presentIds[1] = 5 (wait for this after swapchain 2 is retired)
    present_ids[0] = 3;
    present_ids[1] = 5;
    m_errorMonitor->SetDesiredError("VUID-VkPresentIdKHR-presentIds-04999");
    vk::QueuePresentKHR(m_default_queue->handle(), &present);
    m_errorMonitor->VerifyFound();

    // Errors should prevent previous and future vkQueuePresents from actually happening so ok to re-use images
    present_id.swapchainCount = 0;
    m_errorMonitor->SetDesiredError("VUID-VkPresentIdKHR-swapchainCount-arraylength");
    vk::QueuePresentKHR(m_default_queue->handle(), &present);
    m_errorMonitor->VerifyFound();

    present_id.swapchainCount = 1;
    present_ids[0] = 5;
    m_errorMonitor->SetDesiredError("VUID-VkPresentIdKHR-swapchainCount-04998");
    vk::QueuePresentKHR(m_default_queue->handle(), &present);
    m_errorMonitor->VerifyFound();

    // Retire swapchain2
    vkt::Swapchain swapchain3 =
        CreateSwapchain(surface2.Handle(), VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, swapchain2);
    present_id.swapchainCount = 2;
    m_errorMonitor->SetDesiredError("VUID-vkWaitForPresentKHR-swapchain-04997");
    vk::WaitForPresentKHR(device(), swapchain2, 5, kWaitTimeout);
    m_errorMonitor->VerifyFound();

    swapchain2.destroy();
    swapchain3.destroy();
}

TEST_F(NegativeWsi, PresentIdWaitFeatures) {
    TEST_DESCRIPTION("Test present wait extension");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_PRESENT_WAIT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PRESENT_ID_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    const auto images = m_swapchain.GetImages();

    vkt::Fence fence(*m_device);
    const uint32_t image_index = m_swapchain.AcquireNextImage(fence, kWaitTimeout);
    vk::WaitForFences(device(), 1, &fence.handle(), true, kWaitTimeout);

    SetImageLayoutPresentSrc(images[image_index]);

    uint64_t present_id_index = 1;
    VkPresentIdKHR present_id = vku::InitStructHelper();
    present_id.swapchainCount = 1;
    present_id.pPresentIds = &present_id_index;

    m_errorMonitor->SetDesiredError("VUID-VkPresentInfoKHR-pNext-06235");
    m_default_queue->Present(m_swapchain, image_index, vkt::no_semaphore, &present_id);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkWaitForPresentKHR-presentWait-06234");
    vk::WaitForPresentKHR(device(), m_swapchain, 1, kWaitTimeout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, GetSwapchainImagesCountButNotImages) {
    TEST_DESCRIPTION("Test for getting swapchain images count and presenting before getting swapchain images.");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), m_device->graphics_queue_node_index_, m_surface.Handle(), &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present, skipping test";
    }
    InitSwapchainInfo();

    VkImageFormatProperties img_format_props;
    vk::GetPhysicalDeviceImageFormatProperties(Gpu(), m_surface_formats[0].format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0, &img_format_props);
    VkExtent2D img_ext = {std::min(m_surface_capabilities.maxImageExtent.width, img_format_props.maxExtent.width),
                          std::min(m_surface_capabilities.maxImageExtent.height, img_format_props.maxExtent.height)};

    VkSwapchainCreateInfoKHR swapchain_info = vku::InitStructHelper();
    swapchain_info.surface = m_surface.Handle();
    swapchain_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_info.imageFormat = m_surface_formats[0].format;
    swapchain_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_info.imageExtent = img_ext;
    swapchain_info.imageArrayLayers = 1;
    swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_info.preTransform = m_surface_capabilities.currentTransform;
    swapchain_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_info.presentMode = m_surface_present_modes[0];
    swapchain_info.clipped = VK_FALSE;

    m_swapchain.Init(*m_device, swapchain_info);

    // This test initiates image count query, but don't need resulting value
    m_swapchain.GetImageCount();

    m_errorMonitor->SetDesiredError("VUID-VkPresentInfoKHR-pImageIndices-01430");
    m_default_queue->Present(m_swapchain, 0, vkt::no_semaphore);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, SurfaceSupportByPhysicalDevice) {
    TEST_DESCRIPTION("Test if physical device supports surface.");
    AddOptionalExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddOptionalExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
#ifdef VK_USE_PLATFORM_WIN32_KHR
    AddOptionalExtensions(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);
#endif
    AddOptionalExtensions(VK_KHR_DISPLAY_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME);
    AddSurfaceExtension();

    RETURN_IF_SKIP(Init());
    const bool swapchain = IsExtensionsEnabled(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    const bool get_surface_capabilities2 = IsExtensionsEnabled(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    const bool display_surface_counter = IsExtensionsEnabled(VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME);
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const bool full_screen_exclusive = IsExtensionsEnabled(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);
#endif
    RETURN_IF_SKIP(InitSurface());

    uint32_t queueFamilyPropertyCount;
    vk::GetPhysicalDeviceQueueFamilyProperties(Gpu(), &queueFamilyPropertyCount, nullptr);

    VkBool32 supported = VK_FALSE;
    for (uint32_t i = 0; i < queueFamilyPropertyCount; ++i) {
        vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), i, m_surface.Handle(), &supported);
        if (supported) {
            break;
        }
    }
    if (supported) {
        GTEST_SKIP() << "Physical device supports present";
    }

#ifdef VK_USE_PLATFORM_WIN32_KHR
    if (full_screen_exclusive) {
        VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper();
        surface_info.surface = m_surface.Handle();
        VkDeviceGroupPresentModeFlagsKHR flags = VK_DEVICE_GROUP_PRESENT_MODE_LOCAL_BIT_KHR;

        m_errorMonitor->SetDesiredError("VUID-vkGetDeviceGroupSurfacePresentModes2EXT-pSurfaceInfo-06213");
        vk::GetDeviceGroupSurfacePresentModes2EXT(device(), &surface_info, &flags);
        m_errorMonitor->VerifyFound();

        uint32_t count;
        vk::GetPhysicalDeviceSurfacePresentModes2EXT(Gpu(), &surface_info, &count, nullptr);
    }
#endif

    if (swapchain) {
        m_errorMonitor->SetDesiredError("VUID-vkGetDeviceGroupSurfacePresentModesKHR-surface-06212");
        VkDeviceGroupPresentModeFlagsKHR flags = VK_DEVICE_GROUP_PRESENT_MODE_LOCAL_BIT_KHR;
        vk::GetDeviceGroupSurfacePresentModesKHR(device(), m_surface.Handle(), &flags);
        m_errorMonitor->VerifyFound();

        uint32_t count;
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDevicePresentRectanglesKHR-surface-06211");
        vk::GetPhysicalDevicePresentRectanglesKHR(Gpu(), m_surface.Handle(), &count, nullptr);
        m_errorMonitor->VerifyFound();
    }

    if (display_surface_counter) {
        VkSurfaceCapabilities2EXT capabilities = vku::InitStructHelper();

        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceSurfaceCapabilities2EXT-surface-06211");
        vk::GetPhysicalDeviceSurfaceCapabilities2EXT(Gpu(), m_surface.Handle(), &capabilities);
        m_errorMonitor->VerifyFound();
    }

    if (get_surface_capabilities2) {
        VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper();
        surface_info.surface = m_surface.Handle();
        VkSurfaceCapabilities2KHR capabilities = vku::InitStructHelper();

        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pSurfaceInfo-06522");
        vk::GetPhysicalDeviceSurfaceCapabilities2KHR(Gpu(), &surface_info, &capabilities);
        m_errorMonitor->VerifyFound();
    }

    {
        VkSurfaceCapabilitiesKHR capabilities;
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceSurfaceCapabilitiesKHR-surface-06211");
        vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(Gpu(), m_surface.Handle(), &capabilities);
        m_errorMonitor->VerifyFound();
    }

    if (get_surface_capabilities2) {
        VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper();
        surface_info.surface = m_surface.Handle();
        uint32_t count;
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceSurfaceFormats2KHR-pSurfaceInfo-06522");
        vk::GetPhysicalDeviceSurfaceFormats2KHR(Gpu(), &surface_info, &count, nullptr);
        m_errorMonitor->VerifyFound();
    }

    {
        uint32_t count;
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceSurfaceFormatsKHR-surface-06525");
        vk::GetPhysicalDeviceSurfaceFormatsKHR(Gpu(), m_surface.Handle(), &count, nullptr);
        m_errorMonitor->VerifyFound();
    }

    {
        uint32_t count;
        vk::GetPhysicalDeviceSurfacePresentModesKHR(Gpu(), m_surface.Handle(), &count, nullptr);
    }
}

TEST_F(NegativeWsi, SwapchainMaintenance1ExtensionAcquire) {
    TEST_DESCRIPTION("Test swapchain Maintenance1 extensions.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddSurfaceExtension();
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    RETURN_IF_SKIP(Init());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    uint32_t count;

    VkSurfaceKHR surface = m_surface.Handle();
    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.surface = surface;
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = imageUsage;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = preTransform;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    // Query present mode data
    const std::array defined_present_modes{VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR,
                                           VK_PRESENT_MODE_FIFO_RELAXED_KHR};

    vk::GetPhysicalDeviceSurfacePresentModesKHR(Gpu(), surface, &count, nullptr);
    std::vector<VkPresentModeKHR> pdev_surface_present_modes(count);
    vk::GetPhysicalDeviceSurfacePresentModesKHR(Gpu(), surface, &count, pdev_surface_present_modes.data());

    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper();
    VkSurfaceCapabilities2KHR surface_caps = vku::InitStructHelper();
    surface_info.surface = surface;

    // Set a present_mode in VkSurfacePresentModeEXT that's NOT returned by GetPhsyicalDeviceSurfaceCapabilities2KHR
    VkPresentModeKHR mismatched_present_mode = VK_PRESENT_MODE_MAX_ENUM_KHR;
    for (auto item : defined_present_modes) {
        if (std::find(pdev_surface_present_modes.begin(), pdev_surface_present_modes.end(), item) ==
            pdev_surface_present_modes.end()) {
            mismatched_present_mode = item;
            break;
        }
    }

    VkSurfacePresentModeEXT present_mode = vku::InitStructHelper();
    present_mode.presentMode = mismatched_present_mode;

    surface_info.pNext = &present_mode;
    m_errorMonitor->SetDesiredError("VUID-VkSurfacePresentModeEXT-presentMode-07780");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkSurfacePresentModeEXT-presentMode-parameter");
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(Gpu(), &surface_info, &surface_caps);
    m_errorMonitor->VerifyFound();

    VkSurfacePresentModeCompatibilityEXT present_mode_compatibility = vku::InitStructHelper();
    present_mode.presentMode = pdev_surface_present_modes[0];
    surface_caps.pNext = &present_mode_compatibility;
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(Gpu(), &surface_info, &surface_caps);

    std::vector<VkPresentModeKHR> compatible_present_modes(present_mode_compatibility.presentModeCount);
    present_mode_compatibility.pPresentModes = compatible_present_modes.data();
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(Gpu(), &surface_info, &surface_caps);

    VkSurfacePresentScalingCapabilitiesEXT scaling_capabilities = vku::InitStructHelper();
    surface_caps.pNext = &scaling_capabilities;
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(Gpu(), &surface_info, &surface_caps);

    mismatched_present_mode = VK_PRESENT_MODE_MAX_ENUM_KHR;

    VkSwapchainPresentModesCreateInfoEXT present_modes_ci = vku::InitStructHelper();
    swapchain_create_info.pNext = &present_modes_ci;
    present_modes_ci.presentModeCount = 1;
    present_modes_ci.pPresentModes = &mismatched_present_mode;

    // Pick a presentmode that's not in gpspmkhr
    for (auto item : defined_present_modes) {
        if (std::find(pdev_surface_present_modes.begin(), pdev_surface_present_modes.end(), item) ==
            pdev_surface_present_modes.end()) {
            mismatched_present_mode = item;
            break;
        }
    }
    if (mismatched_present_mode != VK_PRESENT_MODE_MAX_ENUM_KHR) {
        // Each entry in QueuePresent->vkPresentInfoKHR->pNext->SwapchainPresentModesCreateInfo->pPresentModes must be one of the
        // VkPresentModeKHR values returned by vkGetPhysicalDeviceSurfacePresentModesKHR for the surface
        m_errorMonitor->SetDesiredError("VUID-VkSwapchainPresentModesCreateInfoEXT-None-07762");
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkSwapchainPresentModesCreateInfoEXT-pPresentModes-07763");
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkSwapchainPresentModesCreateInfoEXT-presentMode-07764");
        m_swapchain.Init(*m_device, swapchain_create_info);
        m_errorMonitor->VerifyFound();
    }

    // The entries in pPresentModes must be a subset of the present modes returned in
    // VkSurfacePresentModeCompatibilityEXT::pPresentModes, given vkSwapchainCreateInfoKHR::presentMode in VkSurfacePresentModeEXT
    mismatched_present_mode = VK_PRESENT_MODE_MAX_ENUM_KHR;
    for (auto item : defined_present_modes) {
        if (std::find(compatible_present_modes.begin(), compatible_present_modes.end(), item) == compatible_present_modes.end()) {
            mismatched_present_mode = item;
            break;
        }
    }
    if (mismatched_present_mode != VK_PRESENT_MODE_MAX_ENUM_KHR) {
        m_errorMonitor->SetDesiredError("VUID-VkSwapchainPresentModesCreateInfoEXT-pPresentModes-07763");
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkSwapchainPresentModesCreateInfoEXT-None-07762");
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkSwapchainPresentModesCreateInfoEXT-presentMode-07764");
        present_modes_ci.pPresentModes = &mismatched_present_mode;
        m_swapchain.Init(*m_device, swapchain_create_info);
        m_errorMonitor->VerifyFound();
    }

    present_modes_ci.presentModeCount = 1;
    present_modes_ci.pPresentModes = present_mode_compatibility.pPresentModes;
    // SwapchainCreateInfo->presentMode has to be in VkSurfacePresentModeCompatibilityEXT->pPresentModes
    if (compatible_present_modes.size() > 1) {
        swapchain_create_info.presentMode = compatible_present_modes[1];

        VkSurfacePresentModeEXT present_mode2 = vku::InitStructHelper();
        present_mode2.presentMode = pdev_surface_present_modes[0];
        VkPhysicalDeviceSurfaceInfo2KHR surface_info2 = vku::InitStructHelper(&present_mode2);
        surface_info2.surface = m_surface.Handle();

        VkSurfacePresentModeCompatibilityEXT present_mode_compatibility2 = vku::InitStructHelper();
        present_mode2.presentMode = swapchain_create_info.presentMode;
        VkSurfaceCapabilities2KHR surface_caps2 = vku::InitStructHelper(&present_mode_compatibility2);
        vk::GetPhysicalDeviceSurfaceCapabilities2KHR(Gpu(), &surface_info2, &surface_caps2);

        swapchain_create_info.minImageCount = surface_caps2.surfaceCapabilities.minImageCount;
        m_errorMonitor->SetDesiredError("VUID-VkSwapchainPresentModesCreateInfoEXT-presentMode-07764");
        m_swapchain.Init(*m_device, swapchain_create_info);
        m_errorMonitor->VerifyFound();
    }
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;

    swapchain_create_info.presentMode = compatible_present_modes[0];
    VkSwapchainPresentScalingCreateInfoEXT present_scaling_info = vku::InitStructHelper();
    present_scaling_info.pNext = swapchain_create_info.pNext;
    swapchain_create_info.pNext = &present_scaling_info;

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityX-07765");
    // Disable validation that prevents testing zero gravity value on platforms that provide support for gravity values.
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityX-07772");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityX-07773");
    present_scaling_info.scalingBehavior = VK_PRESENT_SCALING_ONE_TO_ONE_BIT_EXT;
    present_scaling_info.presentGravityX = 0;
    present_scaling_info.presentGravityY = VK_PRESENT_GRAVITY_MIN_BIT_EXT;
    m_swapchain.Init(*m_device, swapchain_create_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityX-07766");
    // Disable validation that prevents testing zero gravity value on platforms that provide support for gravity values.
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityY-07774");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityY-07775");
    present_scaling_info.scalingBehavior = VK_PRESENT_SCALING_ONE_TO_ONE_BIT_EXT;
    present_scaling_info.presentGravityX = VK_PRESENT_GRAVITY_MIN_BIT_EXT;
    present_scaling_info.presentGravityY = 0;
    m_swapchain.Init(*m_device, swapchain_create_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainPresentScalingCreateInfoEXT-scalingBehavior-07767");
    present_scaling_info.scalingBehavior = VK_PRESENT_SCALING_ONE_TO_ONE_BIT_EXT | VK_PRESENT_SCALING_ASPECT_RATIO_STRETCH_BIT_EXT;
    present_scaling_info.presentGravityX = VK_PRESENT_GRAVITY_MIN_BIT_EXT;
    present_scaling_info.presentGravityY = VK_PRESENT_GRAVITY_MIN_BIT_EXT;

    m_swapchain.Init(*m_device, swapchain_create_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityX-07768");
    present_scaling_info.scalingBehavior = VK_PRESENT_SCALING_ONE_TO_ONE_BIT_EXT;
    present_scaling_info.presentGravityX = VK_PRESENT_GRAVITY_MIN_BIT_EXT | VK_PRESENT_GRAVITY_MAX_BIT_EXT;
    present_scaling_info.presentGravityY = VK_PRESENT_GRAVITY_MIN_BIT_EXT;
    m_swapchain.Init(*m_device, swapchain_create_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityY-07769");
    present_scaling_info.presentGravityX = VK_PRESENT_GRAVITY_MIN_BIT_EXT;
    present_scaling_info.presentGravityY = VK_PRESENT_GRAVITY_MIN_BIT_EXT | VK_PRESENT_GRAVITY_MAX_BIT_EXT;
    m_swapchain.Init(*m_device, swapchain_create_info);
    m_errorMonitor->VerifyFound();

    present_scaling_info.presentGravityX = 0;
    present_scaling_info.presentGravityY = 0;
    // Find scaling cap not in scaling_capabilities.supportedPresentScaling and create a swapchain using that
    if (scaling_capabilities.supportedPresentScaling != 0) {
        const std::array defined_scaling_flag_bits = {VK_PRESENT_SCALING_ONE_TO_ONE_BIT_EXT,
                                                      VK_PRESENT_SCALING_ASPECT_RATIO_STRETCH_BIT_EXT,
                                                      VK_PRESENT_SCALING_STRETCH_BIT_EXT};
        for (auto scaling_flag : defined_scaling_flag_bits) {
            if ((scaling_capabilities.supportedPresentScaling & scaling_flag) == 0) {
                present_scaling_info.scalingBehavior = scaling_flag;
                m_errorMonitor->SetDesiredError("VUID-VkSwapchainPresentScalingCreateInfoEXT-scalingBehavior-07770");
                m_swapchain.Init(*m_device, swapchain_create_info);
                m_errorMonitor->VerifyFound();
                break;
            }
        }
    }

    const std::array defined_gravity_flag_bits = {VK_PRESENT_GRAVITY_MIN_BIT_EXT, VK_PRESENT_GRAVITY_MAX_BIT_EXT,
                                                  VK_PRESENT_GRAVITY_CENTERED_BIT_EXT};
    if (scaling_capabilities.supportedPresentGravityX != 0) {
        for (auto gravity_flag : defined_gravity_flag_bits) {
            if ((scaling_capabilities.supportedPresentGravityX & gravity_flag) == 0) {
                present_scaling_info.presentGravityX = gravity_flag;
                m_errorMonitor->SetDesiredError("VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityX-07772");
                m_swapchain.Init(*m_device, swapchain_create_info);
                m_errorMonitor->VerifyFound();
                break;
            }
        }
    }
    if (scaling_capabilities.supportedPresentGravityY != 0) {
        for (auto gravity_flag : defined_gravity_flag_bits) {
            if ((scaling_capabilities.supportedPresentGravityY & gravity_flag) == 0) {
                present_scaling_info.presentGravityY = gravity_flag;
                m_errorMonitor->SetDesiredError("VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityY-07774");
                m_swapchain.Init(*m_device, swapchain_create_info);
                m_errorMonitor->VerifyFound();
                break;
            }
        }
    }

    // If the swapchain is created with VkSwapchainPresentModesCreateInfoEXT,
    present_mode.presentMode = present_modes_ci.pPresentModes[0];
    surface_caps.pNext = &scaling_capabilities;
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(Gpu(), &surface_info, &surface_caps);

    // presentScaling must be a valid scaling method for the surface
    // as returned in VkSurfacePresentScalingCapabilitiesEXT::supportedPresentScaling,
    // given each present mode in VkSwapchainPresentModesCreateInfoEXT::pPresentModes in VkSurfacePresentModeEXT
    if (scaling_capabilities.supportedPresentScaling != 0) {
        const std::array defined_scaling_flag_bits = {VK_PRESENT_SCALING_ONE_TO_ONE_BIT_EXT,
                                                      VK_PRESENT_SCALING_ASPECT_RATIO_STRETCH_BIT_EXT,
                                                      VK_PRESENT_SCALING_STRETCH_BIT_EXT};
        for (auto scaling_flag : defined_scaling_flag_bits) {
            if ((scaling_capabilities.supportedPresentScaling & scaling_flag) == 0) {
                present_scaling_info.scalingBehavior = scaling_flag;
                m_errorMonitor->SetDesiredError("VUID-VkSwapchainPresentScalingCreateInfoEXT-scalingBehavior-07771");
                m_swapchain.Init(*m_device, swapchain_create_info);
                m_errorMonitor->VerifyFound();
                break;
            }
        }
    }

    // presentGravityX must be a valid x-axis present gravity for the surface
    // as returned in VkSurfacePresentScalingCapabilitiesEXT::supportedPresentGravityX,
    // given each present mode in VkSwapchainPresentModesCreateInfoEXT::pPresentModes in VkSurfacePresentModeEXT
    if (scaling_capabilities.supportedPresentGravityX != 0) {
        for (auto gravity_flag : defined_gravity_flag_bits) {
            if ((scaling_capabilities.supportedPresentGravityX & gravity_flag) == 0) {
                present_scaling_info.presentGravityX = gravity_flag;
                m_errorMonitor->SetDesiredError("VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityX-07773");
                m_swapchain.Init(*m_device, swapchain_create_info);
                m_errorMonitor->VerifyFound();
                break;
            }
        }
    }

    // presentGravityY must be a valid y-axis present gravity for the surface
    // as returned in VkSurfacePresentScalingCapabilitiesEXT::supportedPresentGravityY,
    // given each present mode in VkSwapchainPresentModesCreateInfoEXT::pPresentModes in VkSurfacePresentModeEXT
    if (scaling_capabilities.supportedPresentGravityY != 0) {
        for (auto gravity_flag : defined_gravity_flag_bits) {
            if ((scaling_capabilities.supportedPresentGravityY & gravity_flag) == 0) {
                present_scaling_info.presentGravityY = gravity_flag;
                m_errorMonitor->SetDesiredError("VUID-VkSwapchainPresentScalingCreateInfoEXT-presentGravityY-07775");
                m_swapchain.Init(*m_device, swapchain_create_info);
                m_errorMonitor->VerifyFound();
                break;
            }
        }
    }

    // Create swapchain
    VkPresentModeKHR good_present_mode = m_surface_non_shared_present_mode;
    present_modes_ci.pPresentModes = &good_present_mode;
    swapchain_create_info.pNext = nullptr;
    m_swapchain.Init(*m_device, swapchain_create_info);

    const auto swapchain_images = m_swapchain.GetImages();
    const vkt::Semaphore acquire_semaphore(*m_device);
    m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);

    m_default_queue->Wait();

    uint32_t release_index = static_cast<uint32_t>(swapchain_images.size()) + 2;
    VkReleaseSwapchainImagesInfoEXT release_info = vku::InitStructHelper();
    release_info.swapchain = m_swapchain;
    release_info.imageIndexCount = 1;
    release_info.pImageIndices = &release_index;
    m_errorMonitor->SetDesiredError("VUID-VkReleaseSwapchainImagesInfoEXT-pImageIndices-07785");
    vk::ReleaseSwapchainImagesEXT(device(), &release_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, SwapchainMaintenance1ExtensionCaps) {
    TEST_DESCRIPTION("Test swapchain and surface Maintenance1 extensions.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    AddSurfaceExtension();

    RETURN_IF_SKIP(InitFramework());

    // Add this after check, surfacless checks are done conditionally
    AddOptionalExtensions(VK_GOOGLE_SURFACELESS_QUERY_EXTENSION_NAME);

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    RETURN_IF_SKIP(InitState());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    uint32_t count;

    // Call CreateSwapChain with a VkSwapchainPresentModesCreateInfoEXT struct W/O calling getcompatibleModes/getScalingCaps
    VkSurfaceKHR surface = m_surface.Handle();
    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.surface = surface;
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = imageUsage;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = preTransform;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;
    swapchain_create_info.flags = VK_SWAPCHAIN_CREATE_DEFERRED_MEMORY_ALLOCATION_BIT_EXT;

    VkPresentModeKHR old_present_mode = m_surface_non_shared_present_mode;
    VkSwapchainPresentModesCreateInfoEXT present_modes_ci = vku::InitStructHelper();
    swapchain_create_info.pNext = &present_modes_ci;
    present_modes_ci.presentModeCount = 1;
    present_modes_ci.pPresentModes = &old_present_mode;

    m_swapchain.Init(*m_device, swapchain_create_info);

    const auto swapchain_images = m_swapchain.GetImages();

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = m_surface_formats[0].format;
    image_create_info.extent.width = m_surface_capabilities.minImageExtent.width;
    image_create_info.extent.height = m_surface_capabilities.minImageExtent.height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageSwapchainCreateInfoKHR image_swapchain_create_info = vku::InitStructHelper();
    image_swapchain_create_info.swapchain = m_swapchain;
    image_create_info.pNext = &image_swapchain_create_info;

    vkt::Image image_from_swapchain(*m_device, image_create_info, vkt::no_mem);

    VkBindImageMemoryInfo bind_info = vku::InitStructHelper();
    bind_info.image = image_from_swapchain.handle();
    bind_info.memory = VK_NULL_HANDLE;
    bind_info.memoryOffset = 0;

    VkBindImageMemorySwapchainInfoKHR bind_swapchain_info = vku::InitStructHelper();
    bind_swapchain_info.imageIndex = 0;
    bind_info.pNext = &bind_swapchain_info;
    bind_swapchain_info.swapchain = m_swapchain;

    // SwapchainMaint1 enabled + deferred_memory_alloc but image not acquired:
    m_errorMonitor->SetDesiredError("VUID-VkBindImageMemorySwapchainInfoKHR-swapchain-07756");
    vk::BindImageMemory2(device(), 1, &bind_info);
    m_errorMonitor->VerifyFound();

    vk::GetPhysicalDeviceSurfacePresentModesKHR(Gpu(), m_surface.Handle(), &count, nullptr);
    std::vector<VkPresentModeKHR> present_modes(count);
    vk::GetPhysicalDeviceSurfacePresentModesKHR(Gpu(), m_surface.Handle(), &count, present_modes.data());

    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper();
    VkSurfaceCapabilities2KHR surface_caps = vku::InitStructHelper();
    surface_info.surface = m_surface.Handle();

    VkSurfacePresentModeEXT present_mode = vku::InitStructHelper();
    VkSurfacePresentModeCompatibilityEXT present_mode_compatibility = vku::InitStructHelper();
    present_mode.presentMode = present_modes[0];
    surface_caps.pNext = &present_mode_compatibility;

    // Leave VkSurfacePresentMode off of the pNext chain
    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-07776");
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(Gpu(), &surface_info, &surface_caps);
    m_errorMonitor->VerifyFound();

    surface_info.pNext = &present_mode;
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(Gpu(), &surface_info, &surface_caps);

    std::vector<VkPresentModeKHR> compatible_present_modes(present_mode_compatibility.presentModeCount);
    present_mode_compatibility.pPresentModes = compatible_present_modes.data();
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(Gpu(), &surface_info, &surface_caps);

    VkSurfacePresentScalingCapabilitiesEXT scaling_capabilities = vku::InitStructHelper();
    surface_caps.pNext = &scaling_capabilities;
    surface_info.pNext = nullptr;
    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-07777");
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(Gpu(), &surface_info, &surface_caps);
    m_errorMonitor->VerifyFound();

    if (IsExtensionsEnabled(VK_GOOGLE_SURFACELESS_QUERY_EXTENSION_NAME)) {
        surface_info.pNext = &present_mode;
        surface_info.surface = VK_NULL_HANDLE;
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-07778");
        vk::GetPhysicalDeviceSurfaceCapabilities2KHR(Gpu(), &surface_info, &surface_caps);
        m_errorMonitor->VerifyFound();

        surface_caps.pNext = &present_mode_compatibility;
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-07779");
        vk::GetPhysicalDeviceSurfaceCapabilities2KHR(Gpu(), &surface_info, &surface_caps);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeWsi, SwapchainMaintenance1ExtensionRelease) {
    TEST_DESCRIPTION("Test acquiring swapchain images with Maint1 features.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddSurfaceExtension();
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    RETURN_IF_SKIP(Init());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkSurfaceKHR surface = m_surface.Handle();
    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.surface = surface;
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = imageUsage;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = preTransform;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;
    swapchain_create_info.flags = VK_SWAPCHAIN_CREATE_DEFERRED_MEMORY_ALLOCATION_BIT_EXT;

    VkPresentModeKHR old_present_mode = m_surface_non_shared_present_mode;
    VkSwapchainPresentModesCreateInfoEXT present_modes_ci = vku::InitStructHelper();
    swapchain_create_info.pNext = &present_modes_ci;
    present_modes_ci.presentModeCount = 1;
    present_modes_ci.pPresentModes = &old_present_mode;

    m_swapchain.Init(*m_device, swapchain_create_info);

    vkt::Semaphore acquire_semaphore(*m_device);
    vkt::Semaphore submit_semaphore(*m_device);

    const auto swapchain_images = m_swapchain.GetImages();
    const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);

    const VkImageMemoryBarrier present_transition =
        TransitionToPresent(swapchain_images[image_index], VK_IMAGE_LAYOUT_UNDEFINED, 0);
    m_command_buffer.Begin();
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0,
                           0, nullptr, 0, nullptr, 1, &present_transition);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer, vkt::Wait(acquire_semaphore, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT),
                            vkt::Signal(submit_semaphore));

    vkt::Fence present_fence(*m_device);
    VkFence fences[2] = {present_fence.handle(), present_fence.handle()};

    // PresentFenceInfo swapchaincount not equal to PresentInfo swapchaincount
    VkSwapchainPresentFenceInfoEXT fence_info = vku::InitStructHelper();
    fence_info.swapchainCount = 1 /* swapchain count */ + 1;
    fence_info.pFences = fences;

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainPresentFenceInfoEXT-swapchainCount-07757");
    m_default_queue->Present(m_swapchain, image_index, submit_semaphore, &fence_info);
    m_errorMonitor->VerifyFound();

    const std::vector<VkPresentModeKHR> defined_present_modes{
        VK_PRESENT_MODE_IMMEDIATE_KHR,
        VK_PRESENT_MODE_MAILBOX_KHR,
        VK_PRESENT_MODE_FIFO_KHR,
        VK_PRESENT_MODE_FIFO_RELAXED_KHR,
        VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR,
        VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR,
    };

    VkPresentModeKHR mismatched_present_mode = VK_PRESENT_MODE_MAX_ENUM_KHR;
    for (auto item : defined_present_modes) {
        if (item != old_present_mode) {
            mismatched_present_mode = item;
            break;
        }
    }

    // Each entry in pPresentModes must be a presentation mode specified in VkSwapchainPresentModesCreateInfoEXT::pPresentModes
    // when creating the entry's corresponding swapchain
    VkSwapchainPresentModeInfoEXT present_mode_info = vku::InitStructHelper();
    present_mode_info.swapchainCount = 1;
    present_mode_info.pPresentModes = &mismatched_present_mode;
    m_errorMonitor->SetDesiredError("VUID-VkSwapchainPresentModeInfoEXT-pPresentModes-07761");
    m_default_queue->Present(m_swapchain, image_index, submit_semaphore, &present_mode_info);
    m_errorMonitor->VerifyFound();

    // QueuePresent resets image[index].acquired to false
    VkPresentModeKHR good_present_mode = m_surface_non_shared_present_mode;
    present_mode_info.pPresentModes = &good_present_mode;
    m_default_queue->Present(m_swapchain, image_index, submit_semaphore, &present_mode_info);

    uint32_t release_index = 0;
    VkReleaseSwapchainImagesInfoEXT release_info = vku::InitStructHelper();
    release_info.swapchain = m_swapchain;
    release_info.imageIndexCount = 1;
    release_info.pImageIndices = &release_index;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkReleaseSwapchainImagesInfoEXT-pImageIndices-07785");
    m_errorMonitor->SetDesiredError("VUID-VkReleaseSwapchainImagesInfoEXT-pImageIndices-07786");
    vk::ReleaseSwapchainImagesEXT(device(), &release_info);
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
TEST_F(NegativeWsi, AcquireFullScreenExclusiveModeEXT) {
    TEST_DESCRIPTION("Test vkAcquireFullScreenExclusiveModeEXT.");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    if (!IsPlatformMockICD()) {
        GTEST_SKIP() << "Only run test MockICD due to CI stability";
    }

    InitRenderTarget();
    RETURN_IF_SKIP(InitSwapchain());

    m_errorMonitor->SetDesiredError("VUID-vkAcquireFullScreenExclusiveModeEXT-swapchain-02675");
    vk::AcquireFullScreenExclusiveModeEXT(device(), m_swapchain);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkReleaseFullScreenExclusiveModeEXT-swapchain-02678");
    vk::ReleaseFullScreenExclusiveModeEXT(device(), m_swapchain);
    m_errorMonitor->VerifyFound();

    const POINT pt_zero = {0, 0};

    VkSurfaceFullScreenExclusiveWin32InfoEXT surface_full_screen_exlusive_info_win32 = vku::InitStructHelper();
    surface_full_screen_exlusive_info_win32.hmonitor = MonitorFromPoint(pt_zero, MONITOR_DEFAULTTOPRIMARY);

    VkSurfaceFullScreenExclusiveInfoEXT surface_full_screen_exlusive_info =
        vku::InitStructHelper(&surface_full_screen_exlusive_info_win32);
    surface_full_screen_exlusive_info.fullScreenExclusive = VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT;

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper(&surface_full_screen_exlusive_info);
    swapchain_create_info.flags = 0;
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = m_swapchain;

    vkt::Swapchain swapchain_one(*m_device, swapchain_create_info);

    swapchain_create_info.oldSwapchain = swapchain_one;
    vkt::Swapchain swapchain_two(*m_device, swapchain_create_info);

    m_errorMonitor->SetDesiredError("VUID-vkAcquireFullScreenExclusiveModeEXT-swapchain-02674");
    vk::AcquireFullScreenExclusiveModeEXT(device(), swapchain_one);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkReleaseFullScreenExclusiveModeEXT-swapchain-02677");
    vk::ReleaseFullScreenExclusiveModeEXT(device(), swapchain_one);
    m_errorMonitor->VerifyFound();

    VkResult res = vk::AcquireFullScreenExclusiveModeEXT(device(), swapchain_two);
    if (res == VK_SUCCESS) {
        m_errorMonitor->SetDesiredError("VUID-vkAcquireFullScreenExclusiveModeEXT-swapchain-02676");
        vk::AcquireFullScreenExclusiveModeEXT(device(), swapchain_two);
        m_errorMonitor->VerifyFound();
    }
}
#endif

#if defined(VK_USE_PLATFORM_WIN32_KHR)
TEST_F(NegativeWsi, CreateSwapchainFullscreenExclusive) {
    TEST_DESCRIPTION("Test creating a swapchain with VkSurfaceFullScreenExclusiveWin32InfoEXT");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddSurfaceExtension();
    AddRequiredExtensions(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    if (!IsPlatformMockICD()) {
        GTEST_SKIP() << "Only run test MockICD due to CI stability";
    }

    InitRenderTarget();
    RETURN_IF_SKIP(InitSwapchain());

    VkSurfaceFullScreenExclusiveInfoEXT surface_full_screen_exlusive_info = vku::InitStructHelper();

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper(&surface_full_screen_exlusive_info);
    swapchain_create_info.flags = 0;
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;

    surface_full_screen_exlusive_info.fullScreenExclusive = VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-pNext-02679");
    vkt::Swapchain swapchain(*m_device, swapchain_create_info);
    m_errorMonitor->VerifyFound();
}
#endif

#if defined(VK_USE_PLATFORM_WIN32_KHR)
TEST_F(NegativeWsi, GetPhysicalDeviceSurfaceCapabilities2KHRWithFullScreenEXT) {
    TEST_DESCRIPTION("Test vkAcquireFullScreenExclusiveModeEXT.");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    if (!IsPlatformMockICD()) {
        GTEST_SKIP() << "Only run test MockICD due to CI stability";
    }

    InitRenderTarget();
    RETURN_IF_SKIP(InitSwapchain());

    VkSurfaceFullScreenExclusiveInfoEXT fullscreen_exclusive_info = vku::InitStructHelper();
    fullscreen_exclusive_info.fullScreenExclusive = VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT;
    // no VkSurfaceFullScreenExclusiveWin32InfoEXT in pNext chain
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper(&fullscreen_exclusive_info);
    surface_info.surface = m_surface.Handle();

    VkSurfaceCapabilities2KHR surface_caps = vku::InitStructHelper();
    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceSurfaceInfo2KHR-pNext-02672");
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(m_device->Physical(), &surface_info, &surface_caps);
    m_errorMonitor->VerifyFound();
}
#endif

TEST_F(NegativeWsi, CreatingWin32Surface) {
    TEST_DESCRIPTION("Test creating win32 surface with invalid hwnd");

#ifndef VK_USE_PLATFORM_WIN32_KHR
    GTEST_SKIP() << "test not supported on platform";
#else
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());

    VkWin32SurfaceCreateInfoKHR surface_create_info = vku::InitStructHelper();
    surface_create_info.hinstance = GetModuleHandle(0);
    surface_create_info.hwnd = NULL;  // Invalid

    VkSurfaceKHR surface;
    m_errorMonitor->SetDesiredError("VUID-VkWin32SurfaceCreateInfoKHR-hwnd-01308");
    vk::CreateWin32SurfaceKHR(instance(), &surface_create_info, nullptr, &surface);
    m_errorMonitor->VerifyFound();
#endif
}

TEST_F(NegativeWsi, CreatingWaylandSurface) {
    TEST_DESCRIPTION("Test creating wayland surface with invalid display/surface");

#ifndef VK_USE_PLATFORM_WAYLAND_KHR
    GTEST_SKIP() << "test not supported on platform";
#else
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    wl_display *display = nullptr;
    wl_registry *registry = nullptr;
    wl_surface *surface = nullptr;
    wl_compositor *compositor = nullptr;
    {
        display = wl_display_connect(nullptr);
        if (!display) {
            GTEST_SKIP() << "couldn't create wayland surface";
        }

        auto global = [](void *data, struct wl_registry *registry, uint32_t id, const char *interface, uint32_t version) {
            (void)version;
            const std::string_view interface_str = interface;
            if (interface_str == "wl_compositor") {
                auto compositor = reinterpret_cast<wl_compositor **>(data);
                *compositor = reinterpret_cast<wl_compositor *>(wl_registry_bind(registry, id, &wl_compositor_interface, 1));
            }
        };

        auto global_remove = [](void *data, struct wl_registry *registry, uint32_t id) {
            (void)data;
            (void)registry;
            (void)id;
        };

        registry = wl_display_get_registry(display);
        ASSERT_TRUE(registry != nullptr);

        const wl_registry_listener registry_listener = {global, global_remove};

        wl_registry_add_listener(registry, &registry_listener, &compositor);

        wl_display_dispatch(display);
        ASSERT_TRUE(compositor);

        surface = wl_compositor_create_surface(compositor);
        ASSERT_TRUE(surface);

        const uint32_t version = wl_surface_get_version(surface);
        ASSERT_TRUE(version > 0);  // Ensure we have a valid surface
    }

    // Invalid display
    {
        VkWaylandSurfaceCreateInfoKHR surface_create_info = vku::InitStructHelper();
        surface_create_info.display = nullptr;
        surface_create_info.surface = surface;

        VkSurfaceKHR vulkan_surface;
        m_errorMonitor->SetDesiredError("VUID-VkWaylandSurfaceCreateInfoKHR-display-01304");
        vk::CreateWaylandSurfaceKHR(instance(), &surface_create_info, nullptr, &vulkan_surface);
        m_errorMonitor->VerifyFound();
    }

    // Invalid surface
    {
        VkWaylandSurfaceCreateInfoKHR surface_create_info = vku::InitStructHelper();
        surface_create_info.display = display;
        surface_create_info.surface = nullptr;

        VkSurfaceKHR vulkan_surface;
        m_errorMonitor->SetDesiredError("VUID-VkWaylandSurfaceCreateInfoKHR-surface-01305");
        vk::CreateWaylandSurfaceKHR(instance(), &surface_create_info, nullptr, &vulkan_surface);
        m_errorMonitor->VerifyFound();
    }

    // Cleanup wayland objects
    wl_surface_destroy(surface);
    wl_compositor_destroy(compositor);
    wl_registry_destroy(registry);
    wl_display_disconnect(display);
#endif
}

TEST_F(NegativeWsi, CreatingXcbSurface) {
    TEST_DESCRIPTION("Test creating xcb surface with invalid connection/window");

#ifndef VK_USE_PLATFORM_XCB_KHR
    GTEST_SKIP() << "test not supported on platform";
#else
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    xcb_connection_t *xcb_connection = xcb_connect(nullptr, nullptr);
    ASSERT_TRUE(xcb_connection);

    // NOTE: This is technically an invalid window! (There is no width/height)
    // But there is no robust way to check for a valid window without crashing the app.
    xcb_window_t xcb_window = xcb_generate_id(xcb_connection);
    ASSERT_TRUE(xcb_window != 0);

    // Invalid connection
    {
        VkXcbSurfaceCreateInfoKHR surface_create_info = vku::InitStructHelper();
        surface_create_info.connection = nullptr;
        surface_create_info.window = xcb_window;

        VkSurfaceKHR vulkan_surface;
        m_errorMonitor->SetDesiredError("VUID-VkXcbSurfaceCreateInfoKHR-connection-01310");
        vk::CreateXcbSurfaceKHR(instance(), &surface_create_info, nullptr, &vulkan_surface);
        m_errorMonitor->VerifyFound();
    }

    // Invalid window
    {
        VkXcbSurfaceCreateInfoKHR surface_create_info = vku::InitStructHelper();
        surface_create_info.connection = xcb_connection;
        surface_create_info.window = 0;

        VkSurfaceKHR vulkan_surface;
        m_errorMonitor->SetDesiredError("VUID-VkXcbSurfaceCreateInfoKHR-window-01311");
        vk::CreateXcbSurfaceKHR(instance(), &surface_create_info, nullptr, &vulkan_surface);
        m_errorMonitor->VerifyFound();
    }

    // Cleanup xcb objects
    xcb_destroy_window(xcb_connection, xcb_window);
    xcb_disconnect(xcb_connection);
#endif
}

TEST_F(NegativeWsi, CreatingX11Surface) {
    TEST_DESCRIPTION("Test creating invalid x11 surface");

#ifndef VK_USE_PLATFORM_XLIB_KHR
    GTEST_SKIP() << "test not supported on platform";
#else
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    if (std::getenv("DISPLAY") == nullptr) {
        GTEST_SKIP() << "Test requires working display\n";
    }

    Display *x11_display = XOpenDisplay(nullptr);
    ASSERT_TRUE(x11_display != nullptr);

    const int screen = DefaultScreen(x11_display);

    const Window x11_window = XCreateSimpleWindow(x11_display, RootWindow(x11_display, screen), 0, 0, 128, 128, 1,
                                                  BlackPixel(x11_display, screen), WhitePixel(x11_display, screen));

    // Invalid display
    {
        VkSurfaceKHR vulkan_surface;
        VkXlibSurfaceCreateInfoKHR surface_create_info = vku::InitStructHelper();
        surface_create_info.dpy = nullptr;
        surface_create_info.window = x11_window;
        m_errorMonitor->SetDesiredError("VUID-VkXlibSurfaceCreateInfoKHR-dpy-01313");
        vk::CreateXlibSurfaceKHR(instance(), &surface_create_info, nullptr, &vulkan_surface);
        m_errorMonitor->VerifyFound();
    }

    // Invalid window
    {
        VkSurfaceKHR vulkan_surface;
        VkXlibSurfaceCreateInfoKHR surface_create_info = vku::InitStructHelper();
        surface_create_info.dpy = x11_display;
        m_errorMonitor->SetDesiredError("VUID-VkXlibSurfaceCreateInfoKHR-window-01314");
        vk::CreateXlibSurfaceKHR(instance(), &surface_create_info, nullptr, &vulkan_surface);
        m_errorMonitor->VerifyFound();
    }

    XDestroyWindow(x11_display, x11_window);
    XCloseDisplay(x11_display);
#endif
}

TEST_F(NegativeWsi, PresentImageWithWrongLayout) {
    TEST_DESCRIPTION("Present swapchain image without transitioning it to presentable layout.");

    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    const vkt::Semaphore acquire_semaphore(*m_device);
    const auto swapchain_images = m_swapchain.GetImages();
    const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);

    m_errorMonitor->SetDesiredError("VUID-VkPresentInfoKHR-pImageIndices-01430");
    m_default_queue->Present(m_swapchain, image_index, acquire_semaphore);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, CreatingSwapchainWithExtent) {
    TEST_DESCRIPTION("Create swapchain with extent greater than maxImageExtent of SurfaceCapabilities");

    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-pNext-07781");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkSwapchainCreateInfoKHR-imageFormat-01778");

    VkSurfaceCapabilitiesKHR surface_capabilities;
    vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(Gpu(), m_surface.Handle(), &surface_capabilities);

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent.width = surface_capabilities.maxImageExtent.width + 1;
    swapchain_ci.imageExtent.height = surface_capabilities.maxImageExtent.height;
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;
    swapchain_ci.clipped = VK_FALSE;
    swapchain_ci.oldSwapchain = 0;

    vkt::Swapchain swapchain(*m_device, swapchain_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, SurfaceQueryImageCompressionControlWithoutExtension) {
    TEST_DESCRIPTION("Test querying surface image compression control without extension.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddSurfaceExtension();

    AddRequiredExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_IMAGE_COMPRESSION_CONTROL_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());

    VkImageCompressionPropertiesEXT compression_properties = vku::InitStructHelper();
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper(&compression_properties);
    surface_info.surface = m_surface.Handle();
    uint32_t count;

    // get compression control properties even of VK_EXT_image_compression_control extension is disabled(or is not supported).
    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceSurfaceInfo2KHR-pNext-pNext");
    vk::GetPhysicalDeviceSurfaceFormats2KHR(Gpu(), &surface_info, &count, nullptr);
    m_errorMonitor->VerifyFound();
}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
TEST_F(NegativeWsi, PhysicalDeviceSurfaceCapabilities) {
    TEST_DESCRIPTION("Test pNext in GetPhysicalDeviceSurfaceCapabilities2KHR");

    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);

    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper();
    surface_info.surface = m_surface.Handle();

    VkSurfaceCapabilitiesFullScreenExclusiveEXT capabilities_full_screen_exclusive = vku::InitStructHelper();

    VkSurfaceCapabilities2KHR surface_capabilities = vku::InitStructHelper(&capabilities_full_screen_exclusive);
    surface_capabilities.surfaceCapabilities = m_surface_capabilities;

    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-02671");
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(Gpu(), &surface_info, &surface_capabilities);
    m_errorMonitor->VerifyFound();
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
        //
TEST_F(NegativeWsi, QueuePresentWaitingSameSemaphore) {
    TEST_DESCRIPTION("Submit to queue with waitSemaphore that another queue is already waiting on.");
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME);
    all_queue_count_ = true;
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));

    if (m_device->QueuesWithGraphicsCapability().size() < 2) {
        GTEST_SKIP() << "2 graphics queues are needed";
    }

    uint32_t image_index{0};
    const auto images = m_swapchain.GetImages();

    vkt::Fence fence(*m_device);
    vkt::Semaphore semaphore(*m_device);

    vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, semaphore.handle(), fence.handle(), &image_index);

    fence.Wait(kWaitTimeout);
    SetImageLayoutPresentSrc(images[image_index]);

    vkt::Queue *other = m_device->QueuesWithGraphicsCapability()[1];

    m_default_queue->Submit(vkt::no_cmd, vkt::Wait(semaphore, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT));

    m_errorMonitor->SetDesiredError("VUID-vkQueuePresentKHR-pWaitSemaphores-01294");
    other->Present(m_swapchain, image_index, semaphore);
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
    other->Wait();
}

TEST_F(NegativeWsi, QueuePresentBinarySemaphoreNotSignaled) {
    TEST_DESCRIPTION("Submit a present operation with a waiting binary semaphore not previously signaled.");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    const auto images = m_swapchain.GetImages();
    for (auto image : images) {
        SetImageLayoutPresentSrc(image);
    }

    vkt::Semaphore semaphore(*m_device);
    const uint32_t image_index = m_swapchain.AcquireNextImage(semaphore, kWaitTimeout);
    m_default_queue->Submit(vkt::no_cmd, vkt::Wait(semaphore));

    // the semaphore has already been waited on
    m_errorMonitor->SetDesiredError("VUID-vkQueuePresentKHR-pWaitSemaphores-03268");
    m_default_queue->Present(m_swapchain, image_index, semaphore);
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

TEST_F(NegativeWsi, QueuePresentDependsOnTimelineWait) {
    TEST_DESCRIPTION("Present semaphore wait has corresponding signal, but that signal depends on timeline wait-before-signal");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    if (!m_second_queue) {
        GTEST_SKIP() << "Two queues are needed";
    }

    const auto images = m_swapchain.GetImages();
    for (auto image : images) {
        SetImageLayoutPresentSrc(image);
    }

    vkt::Semaphore timeline_semaphore(*m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    m_default_queue->Submit(vkt::no_cmd, vkt::TimelineWait(timeline_semaphore, 1));

    vkt::Semaphore acquire_semaphore(*m_device);
    const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);

    vkt::Semaphore binary_semaphore(*m_device);
    m_default_queue->Submit(vkt::no_cmd, vkt::Wait(acquire_semaphore), vkt::Signal(binary_semaphore));

    // the semaphore has already been waited on
    m_errorMonitor->SetDesiredError("VUID-vkQueuePresentKHR-pWaitSemaphores-03268");
    m_default_queue->Present(m_swapchain, image_index, binary_semaphore);
    m_errorMonitor->VerifyFound();

    m_second_queue->Submit(vkt::no_cmd, vkt::TimelineSignal(timeline_semaphore, 1));
    m_device->Wait();
}

TEST_F(NegativeWsi, MissingWaitForImageAcquireSemaphore) {
    TEST_DESCRIPTION("Present immediately after acquire and do not wait on the acquire semaphore.");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());
    const auto swapchain_images = m_swapchain.GetImages();
    for (auto image : swapchain_images) {
        SetImageLayoutPresentSrc(image);
    }

    // Acquire image using a semaphore
    const vkt::Semaphore semaphore(*m_device);
    const uint32_t image_index = m_swapchain.AcquireNextImage(semaphore, kWaitTimeout);

    // Present without waiting on the acquire semaphore
    m_errorMonitor->SetDesiredError("UNASSIGNED-VkPresentInfoKHR-pImageIndices-MissingAcquireWait");
    m_default_queue->Present(m_swapchain, image_index, vkt::no_semaphore);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, MissingWaitForImageAcquireSemaphore_2) {
    TEST_DESCRIPTION("Present after submission. Neither submission nor present waits on the acquire semaphore.");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());
    const auto swapchain_images = m_swapchain.GetImages();
    for (auto image : swapchain_images) {
        SetImageLayoutPresentSrc(image);
    }

    // Acquire image using a semaphore
    const vkt::Semaphore semaphore(*m_device);
    const uint32_t image_index = m_swapchain.AcquireNextImage(semaphore, kWaitTimeout);

    // Dummy submit that signals semaphore that will be waited by the present. Does not wait on the acquire semaphore.
    m_command_buffer.Begin();
    m_command_buffer.End();
    const vkt::Semaphore submit_semaphore(*m_device);
    m_default_queue->Submit(m_command_buffer, vkt::Signal(submit_semaphore));

    // Present waits on submit semaphore. Does not wait on the acquire semaphore.
    m_errorMonitor->SetDesiredError("UNASSIGNED-VkPresentInfoKHR-pImageIndices-MissingAcquireWait");
    m_default_queue->Present(m_swapchain, image_index, submit_semaphore);  // only submit semaphore
    m_errorMonitor->VerifyFound();

    m_default_queue->Wait();
}

TEST_F(NegativeWsi, MissingWaitForImageAcquireFence) {
    TEST_DESCRIPTION("Present immediately after acquire and do not wait on the acquire fence.");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());
    const auto swapchain_images = m_swapchain.GetImages();
    for (auto image : swapchain_images) {
        SetImageLayoutPresentSrc(image);
    }

    // Acquire image using a fence
    const vkt::Fence fence(*m_device);
    const uint32_t image_index = m_swapchain.AcquireNextImage(fence, kWaitTimeout);

    // Present without waiting on the acquire fence
    m_errorMonitor->SetDesiredError("UNASSIGNED-VkPresentInfoKHR-pImageIndices-MissingAcquireWait");
    m_default_queue->Present(m_swapchain, image_index, vkt::no_semaphore);
    m_errorMonitor->VerifyFound();

    // NOTE: this test validates vkQueuePresentKHR.
    // At this point it's fine to wait for the fence to avoid in-use errors during test exit
    // (QueueWaitIdle does not wait for the fence signaled by the non-queue operation - AcquireNextImageKHR).
    vk::WaitForFences(device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);
}

TEST_F(NegativeWsi, MissingWaitForImageAcquireFenceAndSemaphore) {
    TEST_DESCRIPTION("Present immediately after acquire and do not wait on the acquire semaphore or fence.");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());
    const auto swapchain_images = m_swapchain.GetImages();
    for (auto image : swapchain_images) {
        SetImageLayoutPresentSrc(image);
    }

    // Acquire image using a semaphore and fence
    const vkt::Semaphore semaphore(*m_device);
    const vkt::Fence fence(*m_device);
    uint32_t image_index = 0;
    vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, semaphore, fence, &image_index);

    // Present without waiting on the acquire semaphore and fence
    m_errorMonitor->SetDesiredError("UNASSIGNED-VkPresentInfoKHR-pImageIndices-MissingAcquireWait");
    m_default_queue->Present(m_swapchain, image_index, vkt::no_semaphore);
    m_errorMonitor->VerifyFound();

    // NOTE: this test validates vkQueuePresentKHR.
    // At this point it's fine to wait for the fence to avoid in-use errors during test exit
    // (QueueWaitIdle does not wait for the fence signaled by the non-queue operation - AcquireNextImageKHR).
    vk::WaitForFences(device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);
}

TEST_F(NegativeWsi, SwapchainAcquireImageRetired) {
    TEST_DESCRIPTION("Test vkAcquireNextImageKHR with retired swapchain");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = m_swapchain;

    vkt::Swapchain swapchain(*m_device, swapchain_create_info);
    vkt::Semaphore semaphore(*m_device);

    VkAcquireNextImageInfoKHR acquire_info = vku::InitStructHelper();
    acquire_info.swapchain = m_swapchain;
    acquire_info.timeout = kWaitTimeout;
    acquire_info.semaphore = semaphore.handle();
    acquire_info.fence = VK_NULL_HANDLE;
    acquire_info.deviceMask = 0x1;

    uint32_t dummy;
    m_errorMonitor->SetDesiredError("VUID-vkAcquireNextImageKHR-swapchain-01285");
    m_swapchain.AcquireNextImage(semaphore, kWaitTimeout);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-VkAcquireNextImageInfoKHR-swapchain-01675");
    vk::AcquireNextImage2KHR(device(), &acquire_info, &dummy);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, PresentInfoParameters) {
    TEST_DESCRIPTION("Validate VkPresentInfoKHR implicit VUs");

    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    vkt::Fence fence(*m_device);
    const uint32_t image_index = m_swapchain.AcquireNextImage(fence, kWaitTimeout);
    vk::WaitForFences(device(), 1, &fence.handle(), true, kWaitTimeout);

    VkPresentInfoKHR present = vku::InitStructHelper();
    present.waitSemaphoreCount = 0;
    present.swapchainCount = 0;
    present.pSwapchains = &m_swapchain.handle();
    present.pImageIndices = &image_index;
    // There are 3 because 3 different fields rely on swapchainCount being non-zero
    m_errorMonitor->SetDesiredError("VUID-VkPresentInfoKHR-swapchainCount-arraylength");  // pSwapchains
    m_errorMonitor->SetDesiredError("VUID-VkPresentInfoKHR-swapchainCount-arraylength");  // pImageIndices
    m_errorMonitor->SetDesiredError("VUID-VkPresentInfoKHR-swapchainCount-arraylength");  // pResults
    vk::QueuePresentKHR(m_default_queue->handle(), &present);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, PresentRegionsKHR) {
    TEST_DESCRIPTION("Validate VkPresentRegionsKHR");

    AddRequiredExtensions(VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME);
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    vkt::Fence fence(*m_device);
    const uint32_t image_index = m_swapchain.AcquireNextImage(fence, kWaitTimeout);
    vk::WaitForFences(device(), 1, &fence.handle(), true, kWaitTimeout);

    // Allowed to have zero rectangleCount
    VkPresentRegionKHR region[2] = {{0, nullptr}, {0, nullptr}};

    {
        VkPresentRegionsKHR regions = vku::InitStructHelper();
        regions.swapchainCount = 2;  // swapchainCount doesn't match VkPresentInfoKHR::swapchainCount
        regions.pRegions = region;
        m_errorMonitor->SetDesiredError("VUID-VkPresentRegionsKHR-swapchainCount-01260");
        m_default_queue->Present(m_swapchain, image_index, vkt::no_semaphore, &regions);
        m_errorMonitor->VerifyFound();
    }

    {
        VkPresentRegionsKHR regions = vku::InitStructHelper();
        regions.swapchainCount = 0;  // can't be zero
        regions.pRegions = region;
        m_errorMonitor->SetDesiredError("VUID-VkPresentRegionsKHR-swapchainCount-arraylength");
        m_default_queue->Present(m_swapchain, image_index, vkt::no_semaphore, &regions);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeWsi, UseDestroyedSwapchain) {
    TEST_DESCRIPTION("Draw to images of a destroyed swapchain");
    AddSurfaceExtension();

    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), m_device->graphics_queue_node_index_, m_surface.Handle(), &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present";
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;

    vkt::Swapchain swapchain(*m_device, swapchain_create_info);
    const std::vector<VkImage> swapchain_images = swapchain.GetImages();

    vkt::Fence fence(*m_device);
    vk::ResetFences(device(), 1, &fence.handle());
    const uint32_t index = swapchain.AcquireNextImage(fence, kWaitTimeout);
    vk::WaitForFences(device(), 1, &fence.handle(), VK_TRUE, kWaitTimeout);

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = swapchain_images[index];
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = swapchain_create_info.imageFormat;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    vkt::ImageView image_view(*m_device, ivci);
    VkImageView image_view_handle = image_view.handle();

    VkAttachmentDescription attach[] = {
        {0, swapchain_create_info.imageFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
    };

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 1, subpasses, 0, nullptr};
    vkt::RenderPass rp(*m_device, rpci);
    vkt::Framebuffer fb(*m_device, rp.handle(), 1, &image_view_handle, 1, 1);

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.renderPass = rp.handle();
    pipe.CreateGraphicsPipeline();

    vkt::Swapchain oldSwapchain = std::move(swapchain);
    swapchain_create_info.oldSwapchain = oldSwapchain;
    swapchain.Init(*m_device, swapchain_create_info);
    oldSwapchain.destroy();

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkRenderPassBeginInfo-framebuffer-parameter");
    m_command_buffer.BeginRenderPass(rp.handle(), fb.handle());
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeWsi, ImageCompressionControlSwapchainWithoutFeature) {
    TEST_DESCRIPTION("Use image compression control swapchain pNext without feature enabled");

    AddSurfaceExtension();
    AddRequiredExtensions(VK_EXT_IMAGE_COMPRESSION_CONTROL_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkImageCompressionControlEXT image_compression_control = vku::InitStructHelper();

    VkSwapchainCreateInfoKHR create_info = vku::InitStructHelper(&image_compression_control);
    create_info.surface = m_surface.Handle();
    create_info.minImageCount = m_surface_capabilities.minImageCount;
    create_info.imageFormat = m_surface_formats[0].format;
    create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    create_info.imageExtent = m_surface_capabilities.minImageExtent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    create_info.compositeAlpha = m_surface_composite_alpha;
    create_info.presentMode = m_surface_non_shared_present_mode;
    create_info.clipped = VK_FALSE;
    create_info.oldSwapchain = 0;

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-pNext-06752");
    vkt::Swapchain swapchain(*m_device, create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, PresentDuplicatedSwapchain) {
    TEST_DESCRIPTION("Test presenting with the swapchain specified twice");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), m_device->graphics_queue_node_index_, m_surface.Handle(), &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present";
    }

    SurfaceInformation info = GetSwapchainInfo(m_surface.Handle());
    if (info.surface_capabilities.maxImageCount < 3) {
        GTEST_SKIP() << "Required maxImageCount not supported";
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = info.surface_capabilities.maxImageCount;
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

    m_swapchain.Init(*m_device, swapchain_create_info);
    if (!m_swapchain.initialized()) {
        GTEST_SKIP() << "Failed to create swapchain";
    }

    auto images = m_swapchain.GetImages();

    vkt::Fence fence1(*m_device);
    vkt::Fence fence2(*m_device);

    VkSwapchainKHR swapchains[2] = {m_swapchain, m_swapchain};
    uint32_t image_indices[2];

    VkResult result{};
    image_indices[0] = m_swapchain.AcquireNextImage(fence1, kWaitTimeout, &result);
    if (result != VK_SUCCESS) {
        GTEST_SKIP() << "Failed to acquire image";
    }
    image_indices[1] = m_swapchain.AcquireNextImage(fence2, kWaitTimeout, &result);
    if (result != VK_SUCCESS) {
        GTEST_SKIP() << "Failed to acquire image";
    }

    VkFence fences[2] = {fence1.handle(), fence2.handle()};
    vk::WaitForFences(device(), 2u, fences, VK_TRUE, kWaitTimeout);

    SetImageLayoutPresentSrc(images[image_indices[0]]);
    SetImageLayoutPresentSrc(images[image_indices[1]]);

    VkPresentInfoKHR present_info = vku::InitStructHelper();
    present_info.swapchainCount = 2u;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = image_indices;
    m_errorMonitor->SetDesiredError("VUID-VkPresentInfoKHR-pSwapchain-09231");
    vk::QueuePresentKHR(m_default_queue->handle(), &present_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, IncompatibleImageWithSwapchain) {
    TEST_DESCRIPTION("Use VkImageSwapchainCreateInfoKHR with an image which doesnt match swapchain create parameters");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    VkImageSwapchainCreateInfoKHR image_swapchain_create_info = vku::InitStructHelper();
    image_swapchain_create_info.swapchain = m_swapchain;

    VkImageCreateInfo image_create_info = vku::InitStructHelper(&image_swapchain_create_info);
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = m_surface_formats[0].format;
    image_create_info.extent.width = m_surface_capabilities.minImageExtent.width;
    image_create_info.extent.height = m_surface_capabilities.minImageExtent.height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImage image;
    m_errorMonitor->SetDesiredError("VUID-VkImageSwapchainCreateInfoKHR-swapchain-00995");
    vk::CreateImage(device(), &image_create_info, nullptr, &image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, SwapchainPresentModeInfoImplicit) {
    TEST_DESCRIPTION("VkSwapchainPresentModeInfoEXT implicit VUs");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddSurfaceExtension();
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    VkSwapchainPresentModeInfoEXT present_mode_info = vku::InitStructHelper();
    present_mode_info.swapchainCount = 0;
    present_mode_info.pPresentModes = &present_mode;

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainPresentModeInfoEXT-swapchainCount-arraylength");
    m_default_queue->Present(m_swapchain, 0, vkt::no_semaphore, &present_mode_info);
    m_errorMonitor->VerifyFound();

    present_mode_info.swapchainCount = 1;
    present_mode_info.pPresentModes = nullptr;
    m_errorMonitor->SetDesiredError("VUID-VkSwapchainPresentModeInfoEXT-pPresentModes-parameter");
    m_default_queue->Present(m_swapchain, 0, vkt::no_semaphore, &present_mode_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, NonSupportedPresentMode) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8204");
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());

    uint32_t count;
    vk::GetPhysicalDeviceSurfacePresentModesKHR(Gpu(), m_surface.Handle(), &count, nullptr);
    std::vector<VkPresentModeKHR> present_modes(count);
    vk::GetPhysicalDeviceSurfacePresentModesKHR(Gpu(), m_surface.Handle(), &count, present_modes.data());
    for (auto present_mode : present_modes) {
        if (present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            GTEST_SKIP() << "Need no support for VK_PRESENT_MODE_IMMEDIATE_KHR";
        }
    }

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), m_device->graphics_queue_node_index_, m_surface.Handle(), &supported);
    if (!supported) {
        GTEST_SKIP() << "Surface not supported.";
    }

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
    swapchain_create_info.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-presentMode-01281");
    vkt::Swapchain swapchain(*m_device, swapchain_create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, SwapchainMaintenance1DeferredMemoryFlags) {
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.flags = VK_SWAPCHAIN_CREATE_DEFERRED_MEMORY_ALLOCATION_BIT_EXT;
    swapchain_ci.surface = m_surface.Handle();
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
    swapchain_ci.oldSwapchain = 0;

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-flags-parameter");
    m_swapchain.Init(*m_device, swapchain_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, SurfaceCounters) {
    TEST_DESCRIPTION("Test surfaceCounters are supported");

    AddRequiredExtensions(VK_EXT_DISPLAY_CONTROL_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME);
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkSurfaceCapabilities2EXT surface_capabilities = vku::InitStructHelper();
    vk::GetPhysicalDeviceSurfaceCapabilities2EXT(Gpu(), m_surface.Handle(), &surface_capabilities);

    if (surface_capabilities.supportedSurfaceCounters & VK_SURFACE_COUNTER_VBLANK_BIT_EXT) {
        GTEST_SKIP() << "Device supports VK_SURFACE_COUNTER_VBLANK_BIT_EXT";
    }

    VkSwapchainCounterCreateInfoEXT swapchain_counter_ci = vku::InitStructHelper();
    swapchain_counter_ci.surfaceCounters = VK_SURFACE_COUNTER_VBLANK_BIT_EXT;

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper(&swapchain_counter_ci);
    swapchain_ci.surface = m_surface.Handle();
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
    swapchain_ci.oldSwapchain = 0;

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCounterCreateInfoEXT-surfaceCounters-01244");
    m_swapchain.Init(*m_device, swapchain_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, PresentInfoSwapchainsDifferentPresentModes) {
    TEST_DESCRIPTION("Submit VkPresentInfo where one swapchain has VkSwapchainPresentModesCreateInfoEXT and the other does not");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    SurfaceContext surface_context{};
    vkt::Surface surface2{};
    if (CreateSurface(surface_context, surface2) != VK_SUCCESS) {
        GTEST_SKIP() << "Cannot create required surface";
    }

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
    swapchain_ci.imageArrayLayers = 1u;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;
    swapchain_ci.clipped = VK_FALSE;
    swapchain_ci.oldSwapchain = 0u;

    vkt::Swapchain swapchain1(*m_device, swapchain_ci);
    VkSwapchainPresentModesCreateInfoEXT present_modes_ci = vku::InitStructHelper();
    present_modes_ci.presentModeCount = 1u;
    present_modes_ci.pPresentModes = &swapchain_ci.presentMode;
    swapchain_ci.surface = surface2.Handle();
    swapchain_ci.pNext = &present_modes_ci;
    vkt::Swapchain swapchain2(*m_device, swapchain_ci);

    vkt::Semaphore image_acquired1(*m_device);
    vkt::Semaphore image_acquired2(*m_device);
    const uint32_t image_index1 = swapchain1.AcquireNextImage(image_acquired1, kWaitTimeout);
    const uint32_t image_index2 = swapchain2.AcquireNextImage(image_acquired2, kWaitTimeout);

    const VkImageMemoryBarrier present_transitions[] = {
        TransitionToPresent(swapchain1.GetImages()[image_index1], VK_IMAGE_LAYOUT_UNDEFINED, 0),
        TransitionToPresent(swapchain2.GetImages()[image_index2], VK_IMAGE_LAYOUT_UNDEFINED, 0),
    };
    m_command_buffer.Begin();
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0,
                           0u, nullptr, 0u, nullptr, 2u, present_transitions);
    m_command_buffer.End();

    VkSemaphore acquire_semaphores[] = {image_acquired1.handle(), image_acquired2.handle()};
    VkPipelineStageFlags wait_masks[] = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};

    vkt::Semaphore semaphore(*m_device);

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 2u;
    submit_info.pWaitSemaphores = acquire_semaphores;
    submit_info.pWaitDstStageMask = wait_masks;
    submit_info.commandBufferCount = 1u;
    submit_info.pCommandBuffers = &m_command_buffer.handle();
    submit_info.signalSemaphoreCount = 1u;
    submit_info.pSignalSemaphores = &semaphore.handle();
    vk::QueueSubmit(m_default_queue->handle(), 1u, &submit_info, VK_NULL_HANDLE);

    VkSwapchainKHR swapchains[] = {swapchain1.handle(), swapchain2.handle()};
    uint32_t image_indices[] = {image_index1, image_index2};

    VkPresentInfoKHR present = vku::InitStructHelper();
    present.waitSemaphoreCount = 1u;
    present.pWaitSemaphores = &semaphore.handle();
    present.pSwapchains = swapchains;
    present.pImageIndices = image_indices;
    present.swapchainCount = 2;
    m_errorMonitor->SetDesiredError("VUID-VkPresentInfoKHR-pSwapchains-09199");
    vk::QueuePresentKHR(m_default_queue->handle(), &present);
    m_errorMonitor->VerifyFound();
    vk::DeviceWaitIdle(device());
}

TEST_F(NegativeWsi, ReleaseSwapchainImagesWithoutFeature) {
    TEST_DESCRIPTION("Submit VkPresentInfo where one swapchain has VkSwapchainPresentModesCreateInfoEXT and the other does not");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    vkt::Semaphore acquire_semaphore(*m_device);
    const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);

    VkReleaseSwapchainImagesInfoEXT release_info = vku::InitStructHelper();
    release_info.swapchain = m_swapchain.handle();
    release_info.imageIndexCount = 1u;
    release_info.pImageIndices = &image_index;

    m_errorMonitor->SetDesiredError("VUID-vkReleaseSwapchainImagesEXT-swapchainMaintenance1-10159");
    vk::ReleaseSwapchainImagesEXT(device(), &release_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, SwapchainCreateMissingMaintenanc1Feature) {
    TEST_DESCRIPTION("Submit VkPresentInfo where one swapchain has VkSwapchainPresentModesCreateInfoEXT and the other does not");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkSwapchainPresentModesCreateInfoEXT present_modes_create_info = vku::InitStructHelper();
    present_modes_create_info.presentModeCount = 1u;
    present_modes_create_info.pPresentModes = &m_surface_non_shared_present_mode;

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper(&present_modes_create_info);
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
    swapchain_ci.imageArrayLayers = 1u;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;
    swapchain_ci.clipped = VK_FALSE;
    swapchain_ci.oldSwapchain = 0u;

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-swapchainMaintenance1-10155");
    m_swapchain.Init(*m_device, swapchain_ci);
    m_errorMonitor->VerifyFound();

    VkSwapchainPresentScalingCreateInfoEXT present_scaling_ci = vku::InitStructHelper();
    present_scaling_ci.scalingBehavior = VK_PRESENT_SCALING_ASPECT_RATIO_STRETCH_BIT_EXT;
    swapchain_ci.pNext = &present_scaling_ci;
    m_errorMonitor->SetDesiredError("VUID-VkSwapchainPresentScalingCreateInfoEXT-swapchainMaintenance1-10154");
    m_swapchain.Init(*m_device, swapchain_ci);
    m_errorMonitor->VerifyFound();

    swapchain_ci.pNext = nullptr;
    swapchain_ci.flags = VK_SWAPCHAIN_CREATE_DEFERRED_MEMORY_ALLOCATION_BIT_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-swapchainMaintenance1-10157");
    m_swapchain.Init(*m_device, swapchain_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, ImageCompressionPropertiesSwapchainWithoutExtension) {
    TEST_DESCRIPTION("Use image compression control swapchain pNext without feature enabled");

    AddRequiredExtensions(VK_EXT_IMAGE_COMPRESSION_CONTROL_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    if (DeviceExtensionSupported(Gpu(), nullptr, VK_EXT_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_EXT_image_compression_control_swapchain is supported";
    }
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper();
    surface_info.surface = m_surface.Handle();

    uint32_t surface_format_count;
    vk::GetPhysicalDeviceSurfaceFormats2KHR(Gpu(), &surface_info, &surface_format_count, nullptr);

    std::vector<VkSurfaceFormat2KHR> surface_formats(surface_format_count, vku::InitStruct<VkSurfaceFormat2KHR>());
    VkImageCompressionPropertiesEXT image_compression_control = vku::InitStructHelper();
    surface_formats[0].pNext = &image_compression_control;

    m_errorMonitor->SetDesiredError("VUID-VkSurfaceFormat2KHR-pNext-06750");
    vk::GetPhysicalDeviceSurfaceFormats2KHR(Gpu(), &surface_info, &surface_format_count, surface_formats.data());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, MissingPresentModeFifoLatestReadyFeature) {
    TEST_DESCRIPTION("Create swapchain with unsupported fifo latest ready present mode");

    AddSurfaceExtension();
    AddRequiredExtensions(VK_EXT_PRESENT_MODE_FIFO_LATEST_READY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = VK_PRESENT_MODE_FIFO_LATEST_READY_EXT;
    swapchain_ci.clipped = VK_FALSE;
    swapchain_ci.oldSwapchain = 0;

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-presentModeFifoLatestReady-10161");
    vkt::Swapchain swapchain(*m_device, swapchain_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, MissingPresentModesCreateInfoFifoLatestReadyFeature) {
    TEST_DESCRIPTION("Create swapchain with unsupported fifo latest ready present mode");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddSurfaceExtension();
    AddRequiredExtensions(VK_EXT_PRESENT_MODE_FIFO_LATEST_READY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkSurfacePresentModeCompatibilityEXT present_mode_compatibility = vku::InitStructHelper();
    VkSurfaceCapabilities2KHR surface_caps = vku::InitStructHelper(&present_mode_compatibility);

    VkSurfacePresentModeEXT present_mode = vku::InitStructHelper();
    present_mode.presentMode = VK_PRESENT_MODE_FIFO_LATEST_READY_EXT;
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper(&present_mode);
    surface_info.surface = m_surface.Handle();
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(Gpu(), &surface_info, &surface_caps);
    std::vector<VkPresentModeKHR> presentModes(present_mode_compatibility.presentModeCount);
    present_mode_compatibility.pPresentModes = presentModes.data();
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(Gpu(), &surface_info, &surface_caps);

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_LATEST_READY_EXT;

    if (std::find(presentModes.begin(), presentModes.end(), presentMode) == presentModes.end()) {
        GTEST_SKIP() << "VK_PRESENT_MODE_FIFO_LATEST_READY_EXT is not compatible";
    }

    VkSwapchainPresentModesCreateInfoEXT present_modes_ci = vku::InitStructHelper();
    present_modes_ci.presentModeCount = 1;
    present_modes_ci.pPresentModes = &presentMode;

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper(&present_modes_ci);
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = VK_PRESENT_MODE_FIFO_LATEST_READY_EXT;
    swapchain_ci.clipped = VK_FALSE;
    swapchain_ci.oldSwapchain = 0;

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkSwapchainCreateInfoKHR-presentModeFifoLatestReady-10161");
    m_errorMonitor->SetDesiredError("VUID-VkSwapchainPresentModesCreateInfoEXT-presentModeFifoLatestReady-10160");
    vkt::Swapchain swapchain(*m_device, swapchain_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, InitSwapchainInvalidExtent) {
    TEST_DESCRIPTION("Initialize swapchain with imageExtent width 0");

    AddSurfaceExtension();

    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = {0u, m_surface_capabilities.minImageExtent.height};
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;
    swapchain_ci.clipped = VK_FALSE;
    swapchain_ci.oldSwapchain = 0u;

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-imageExtent-01689");
    vkt::Swapchain swapchain(*m_device, swapchain_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, InitSwapchainPresentScalingInvalidExtent) {
    TEST_DESCRIPTION("Initialize swapchain with present scaling and invalid imageExtent");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    AddSurfaceExtension();

    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkSurfacePresentModeEXT surface_present_mode = vku::InitStructHelper();
    surface_present_mode.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper(&surface_present_mode);
    surface_info.surface = m_surface.Handle();

    VkSurfacePresentScalingCapabilitiesEXT present_scaling_capabilities = vku::InitStructHelper();
    VkSurfaceCapabilities2KHR surfaceCapabilities = vku::InitStructHelper(&present_scaling_capabilities);
    vk::GetPhysicalDeviceSurfaceCapabilities2KHR(gpu_, &surface_info, &surfaceCapabilities);

    if ((present_scaling_capabilities.supportedPresentScaling & VK_PRESENT_SCALING_ASPECT_RATIO_STRETCH_BIT_EXT) == 0) {
        GTEST_SKIP() << "Required scalingBehavior not supported";
    }

    VkImageFormatProperties image_format_properties;
    vk::GetPhysicalDeviceImageFormatProperties(gpu_, m_surface_formats[0].format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0u, &image_format_properties);

    if (present_scaling_capabilities.maxScaledImageExtent.width > image_format_properties.maxExtent.width) {
        GTEST_SKIP() << "VkSurfacePresentScalingCapabilitiesEXT::maxScaledImageExtent.width is higher than "
                        "VkImageFormatProperties::maxExtent.width";
    }

    VkSwapchainPresentScalingCreateInfoEXT present_scaling_ci = vku::InitStructHelper();
    present_scaling_ci.scalingBehavior = VK_PRESENT_SCALING_ASPECT_RATIO_STRETCH_BIT_EXT;
    present_scaling_ci.presentGravityX = 0;
    present_scaling_ci.presentGravityY = 0;

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper(&present_scaling_ci);
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = {present_scaling_capabilities.maxScaledImageExtent.width,
                                m_surface_capabilities.minImageExtent.height};
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;
    swapchain_ci.clipped = VK_FALSE;
    swapchain_ci.oldSwapchain = 0u;

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-pNext-07782");
    vkt::Swapchain swapchain(*m_device, swapchain_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, InitSwapchainUnsupportedCompositeAlpha) {
    TEST_DESCRIPTION("Initialize swapchain with supportedCompositeAlpha that is not supported");

    AddSurfaceExtension();

    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkSurfaceCapabilitiesKHR surface_capabilities;
    vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(gpu_, m_surface.Handle(), &surface_capabilities);

    if (surface_capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) {
        GTEST_SKIP() << "VkSurfaceCapabilitiesKHR::supportedCompositeAlpha includes VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR";
    }

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;
    swapchain_ci.clipped = VK_FALSE;
    swapchain_ci.oldSwapchain = 0u;

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-compositeAlpha-01280");
    vkt::Swapchain swapchain(*m_device, swapchain_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, InitSwapchainUnsupportedImageArrayLayers) {
    TEST_DESCRIPTION("Initialize swapchain with too large image array layers");

    AddSurfaceExtension();

    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkSurfaceCapabilitiesKHR surface_capabilities;
    vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(gpu_, m_surface.Handle(), &surface_capabilities);

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_ci.imageArrayLayers = surface_capabilities.maxImageArrayLayers + 1u;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;
    swapchain_ci.clipped = VK_FALSE;
    swapchain_ci.oldSwapchain = 0u;

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-imageArrayLayers-01275");
    vkt::Swapchain swapchain(*m_device, swapchain_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, InitSwapchainUnsupportedSurfaceFormat) {
    TEST_DESCRIPTION("Initialize swapchain with unsupported surface format");

    AddSurfaceExtension();

    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = VK_FORMAT_R32G32B32A32_UINT;
    swapchain_ci.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchain_ci.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_ci.imageArrayLayers = 1u;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;
    swapchain_ci.clipped = VK_FALSE;
    swapchain_ci.oldSwapchain = 0u;

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-imageFormat-01273");
    vkt::Swapchain swapchain(*m_device, swapchain_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, InitSwapchainMissingQueueFamilyIndices) {
    TEST_DESCRIPTION("Initialize swapchain with invalid queue family indices");

    AddSurfaceExtension();

    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_ci.imageArrayLayers = 1u;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchain_ci.queueFamilyIndexCount = 2u;
    swapchain_ci.pQueueFamilyIndices = nullptr;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;
    swapchain_ci.clipped = VK_FALSE;
    swapchain_ci.oldSwapchain = 0u;

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-imageSharingMode-01277");
    vkt::Swapchain swapchain1(*m_device, swapchain_ci);
    m_errorMonitor->VerifyFound();

    swapchain_ci.queueFamilyIndexCount = 1u;
    swapchain_ci.pQueueFamilyIndices = &m_default_queue->family_index;
    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-imageSharingMode-01278");
    vkt::Swapchain swapchain2(*m_device, swapchain_ci);
    m_errorMonitor->VerifyFound();

    uint32_t queue_family_indices[2] = {m_default_queue->family_index, m_default_queue->family_index};
    swapchain_ci.queueFamilyIndexCount = 2u;
    swapchain_ci.pQueueFamilyIndices = queue_family_indices;
    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-imageSharingMode-01428");
    vkt::Swapchain swapchain3(*m_device, swapchain_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, InitSwapchainInvalidImageCount) {
    TEST_DESCRIPTION("Initialize swapchain with invalid minImageCount");

    AddSurfaceExtension();

    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    if (m_surface_capabilities.maxImageCount == 0) {
        GTEST_SKIP() << "maxImageCount is 0";
    }

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = m_surface_capabilities.maxImageCount + 1u;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_ci.imageArrayLayers = 1u;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;
    swapchain_ci.clipped = VK_FALSE;
    swapchain_ci.oldSwapchain = 0u;

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-minImageCount-01272");
    vkt::Swapchain swapchain(*m_device, swapchain_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, InitSwapchainInvalidOldSwapchain) {
    TEST_DESCRIPTION("Initialize swapchain with invalid oldSwapchain");

    AddSurfaceExtension();

    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_ci.imageArrayLayers = 1u;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;
    swapchain_ci.clipped = VK_FALSE;
    swapchain_ci.oldSwapchain = 0u;

    vkt::Swapchain swapchain1(*m_device, swapchain_ci);

    swapchain_ci.oldSwapchain = swapchain1.handle();
    vkt::Swapchain swapchain2(*m_device, swapchain_ci);

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-oldSwapchain-01933");
    vkt::Swapchain swapchain3(*m_device, swapchain_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeWsi, PresentSignaledFence) {
    TEST_DESCRIPTION("Use a sigled fence in VkSwapchainPresentFenceInfoEXT");
    AddSurfaceExtension();
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    const vkt::Semaphore acquire_semaphore(*m_device);
    const vkt::Semaphore submit_semaphore(*m_device);

    const auto swapchain_images = m_swapchain.GetImages();
    const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);
    const auto present_transition = TransitionToPresent(swapchain_images[image_index], VK_IMAGE_LAYOUT_UNDEFINED, 0);

    m_command_buffer.Begin();
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &present_transition);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer, vkt::Wait(acquire_semaphore), vkt::Signal(submit_semaphore));

    vkt::Fence present_fence(*m_device, VK_FENCE_CREATE_SIGNALED_BIT);
    VkSwapchainPresentFenceInfoEXT present_fence_info = vku::InitStructHelper();
    present_fence_info.swapchainCount = 1;
    present_fence_info.pFences = &present_fence.handle();

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainPresentFenceInfoEXT-pFences-07758");
    m_default_queue->Present(m_swapchain, image_index, submit_semaphore, &present_fence_info);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeWsi, PresentFenceInUse) {
    TEST_DESCRIPTION("Use a fence that is already in use in VkSwapchainPresentFenceInfoEXT");
    AddSurfaceExtension();
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    const vkt::Semaphore acquire_semaphore(*m_device);
    const vkt::Semaphore submit_semaphore(*m_device);

    const auto swapchain_images = m_swapchain.GetImages();
    const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);
    const auto present_transition = TransitionToPresent(swapchain_images[image_index], VK_IMAGE_LAYOUT_UNDEFINED, 0);

    m_command_buffer.Begin();
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &present_transition);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer, vkt::Wait(acquire_semaphore), vkt::Signal(submit_semaphore));

    vkt::Fence present_fence(*m_device);
    VkSwapchainPresentFenceInfoEXT present_fence_info = vku::InitStructHelper();
    present_fence_info.swapchainCount = 1;
    present_fence_info.pFences = &present_fence.handle();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    vk::QueueSubmit(m_default_queue->handle(), 1u, &submit_info, present_fence.handle());

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainPresentFenceInfoEXT-pFences-07759");
    m_default_queue->Present(m_swapchain, image_index, submit_semaphore, &present_fence_info);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeWsi, PresentMismatchedSwapchainCount) {
    TEST_DESCRIPTION("Use a sigled fence in VkSwapchainPresentFenceInfoEXT");
    AddSurfaceExtension();
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkPresentModeKHR present_mode = m_surface_non_shared_present_mode;

    VkSwapchainPresentModesCreateInfoEXT present_modes_ci = vku::InitStructHelper();
    present_modes_ci.presentModeCount = 1u;
    present_modes_ci.pPresentModes = &present_mode;

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper(&present_modes_ci);
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_ci.imageArrayLayers = 1u;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = present_mode;
    swapchain_ci.clipped = VK_FALSE;
    swapchain_ci.oldSwapchain = 0u;

    vkt::Swapchain swapchain(*m_device, swapchain_ci);

    const vkt::Semaphore acquire_semaphore(*m_device);
    const vkt::Semaphore submit_semaphore(*m_device);

    const auto swapchain_images = swapchain.GetImages();
    const uint32_t image_index = swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);
    const auto present_transition = TransitionToPresent(swapchain_images[image_index], VK_IMAGE_LAYOUT_UNDEFINED, 0);

    m_command_buffer.Begin();
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &present_transition);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer, vkt::Wait(acquire_semaphore), vkt::Signal(submit_semaphore));

    VkPresentModeKHR present_modes[2] = {present_mode, present_mode};

    VkSwapchainPresentModeInfoEXT present_mode_info = vku::InitStructHelper();
    present_mode_info.swapchainCount = 2u;
    present_mode_info.pPresentModes = present_modes;

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainPresentModeInfoEXT-swapchainCount-07760");
    m_default_queue->Present(swapchain, image_index, submit_semaphore, &present_mode_info);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeWsi, InvalidRectLayer) {
    TEST_DESCRIPTION("Present with RectLayer that is not valid");
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_ci.imageArrayLayers = 1u;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = m_surface_composite_alpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;
    swapchain_ci.clipped = VK_FALSE;
    swapchain_ci.oldSwapchain = 0u;

    vkt::Swapchain swapchain(*m_device, swapchain_ci);

    const vkt::Semaphore acquire_semaphore(*m_device);
    const vkt::Semaphore submit_semaphore(*m_device);

    const auto swapchain_images = swapchain.GetImages();
    const uint32_t image_index = swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);
    const auto present_transition = TransitionToPresent(swapchain_images[image_index], VK_IMAGE_LAYOUT_UNDEFINED, 0);

    m_command_buffer.Begin();
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &present_transition);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer, vkt::Wait(acquire_semaphore), vkt::Signal(submit_semaphore));

    VkRectLayerKHR rectangle;
    rectangle.offset.x = 0;
    rectangle.offset.y = 0;
    rectangle.extent = swapchain_ci.imageExtent;
    rectangle.layer = 1u;

    VkPresentRegionKHR present_region;
    present_region.rectangleCount = 1u;
    present_region.pRectangles = &rectangle;

    VkPresentRegionsKHR present_regions = vku::InitStructHelper();
    present_regions.swapchainCount = 1u;
    present_regions.pRegions = &present_region;

    m_errorMonitor->SetDesiredError("VUID-VkRectLayerKHR-layer-01262");
    m_default_queue->Present(swapchain, image_index, submit_semaphore, &present_regions);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();

    rectangle.layer = 0u;
    rectangle.offset.x = 1;
    m_errorMonitor->SetDesiredError("VUID-VkRectLayerKHR-offset-04864");
    m_default_queue->Present(swapchain, image_index, submit_semaphore, &present_regions);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeWsi, PresentWithUnsupportedQueue) {
    TEST_DESCRIPTION("Present with a queue family that does nto support presenting");
    AddSurfaceExtension();
    AddRequiredExtensions(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::swapchainMaintenance1);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSwapchain());

    const std::optional<uint32_t> transfer_queue_family_index = m_device->TransferOnlyQueueFamily();
    if (!transfer_queue_family_index) {
        GTEST_SKIP() << "Required transfer queue family not found";
    }

    const uint32_t transfer_qfi = m_device->TransferOnlyQueueFamily().value();

    VkBool32 surface_support;
    vk::GetPhysicalDeviceSurfaceSupportKHR(gpu_, transfer_qfi, m_surface.Handle(), &surface_support);
    if (surface_support == VK_TRUE) {
        GTEST_SKIP() << "Surface is supported";
    }

    const vkt::Semaphore acquire_semaphore(*m_device);
    const vkt::Semaphore submit_semaphore(*m_device);

    const auto swapchain_images = m_swapchain.GetImages();
    const uint32_t image_index = m_swapchain.AcquireNextImage(acquire_semaphore, kWaitTimeout);
    const auto present_transition = TransitionToPresent(swapchain_images[image_index], VK_IMAGE_LAYOUT_UNDEFINED, 0);

    vkt::CommandPool command_pool(*m_device, transfer_qfi);
    vkt::CommandBuffer command_buffer(*m_device, command_pool);

    command_buffer.Begin();
    vk::CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &present_transition);
    command_buffer.End();
    m_device->TransferOnlyQueue()->Submit(command_buffer, vkt::Wait(acquire_semaphore), vkt::Signal(submit_semaphore));

    vkt::Fence present_fence(*m_device);
    VkSwapchainPresentFenceInfoEXT present_fence_info = vku::InitStructHelper();
    present_fence_info.swapchainCount = 1;
    present_fence_info.pFences = &present_fence.handle();

    m_errorMonitor->SetDesiredError("VUID-vkQueuePresentKHR-pSwapchains-01292");
    m_device->TransferOnlyQueue()->Present(m_swapchain, image_index, submit_semaphore, &present_fence_info);
    m_errorMonitor->VerifyFound();
    m_device->TransferOnlyQueue()->Wait();
}

TEST_F(NegativeWsi, UnsupportedCompositeAlpha) {
    TEST_DESCRIPTION("Create swapchain with unsupported fifo latest ready present mode");

    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());
    InitSwapchainInfo();

    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
    if (m_surface_capabilities.supportedCompositeAlpha & compositeAlpha) {
        GTEST_SKIP() << "VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR is supported";
    }

    VkSwapchainCreateInfoKHR swapchain_ci = vku::InitStructHelper();
    swapchain_ci.surface = m_surface.Handle();
    swapchain_ci.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_ci.imageFormat = m_surface_formats[0].format;
    swapchain_ci.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_ci.imageExtent = m_surface_capabilities.minImageExtent;
    swapchain_ci.imageArrayLayers = 1u;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_ci.compositeAlpha = compositeAlpha;
    swapchain_ci.presentMode = m_surface_non_shared_present_mode;
    swapchain_ci.clipped = VK_FALSE;
    swapchain_ci.oldSwapchain = 0;

    m_errorMonitor->SetDesiredError("VUID-VkSwapchainCreateInfoKHR-compositeAlpha-01280");
    vkt::Swapchain swapchain(*m_device, swapchain_ci);
    m_errorMonitor->VerifyFound();
}
