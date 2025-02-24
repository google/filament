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

class NegativeImageDrm : public ImageDrmTest {};

TEST_F(NegativeImageDrm, Basic) {
    RETURN_IF_SKIP(InitBasicImageDrm());

    std::vector<uint64_t> mods = GetFormatModifier(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    if (mods.empty()) {
        GTEST_SKIP() << "No valid Format Modifier found";
    }

    VkImageCreateInfo image_info = vku::InitStructHelper();
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.arrayLayers = 1;
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.mipLevels = 1;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageFormatProperties2 image_format_prop = vku::InitStructHelper();
    VkPhysicalDeviceImageFormatInfo2 image_format_info = vku::InitStructHelper();
    image_format_info.format = image_info.format;
    image_format_info.tiling = image_info.tiling;
    image_format_info.type = image_info.imageType;
    image_format_info.usage = image_info.usage;
    VkPhysicalDeviceImageDrmFormatModifierInfoEXT drm_format_mod_info = vku::InitStructHelper();
    drm_format_mod_info.drmFormatModifier = mods[0];
    drm_format_mod_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_format_info.pNext = (void *)&drm_format_mod_info;
    vk::GetPhysicalDeviceImageFormatProperties2(m_device->Physical().handle(), &image_format_info, &image_format_prop);

    {
        VkImageFormatProperties dummy_props;
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceImageFormatProperties-tiling-02248");
        vk::GetPhysicalDeviceImageFormatProperties(m_device->Physical().handle(), image_info.format, image_info.imageType,
                                                   image_info.tiling, image_info.usage, image_info.flags, &dummy_props);
        m_errorMonitor->VerifyFound();
    }

    VkSubresourceLayout fake_plane_layout = {0, 0, 0, 0, 0};

    VkImageDrmFormatModifierExplicitCreateInfoEXT drm_format_mod_explicit = vku::InitStructHelper();
    drm_format_mod_explicit.drmFormatModifierPlaneCount = 1;
    drm_format_mod_explicit.pPlaneLayouts = &fake_plane_layout;

    // No pNext
    CreateImageTest(*this, &image_info, "VUID-VkImageCreateInfo-tiling-02261");

    // Having wrong size, arrayPitch and depthPitch in VkSubresourceLayout
    fake_plane_layout.size = 1;
    fake_plane_layout.arrayPitch = 1;
    fake_plane_layout.depthPitch = 1;

    image_info.pNext = (void *)&drm_format_mod_explicit;
    m_errorMonitor->SetDesiredError("VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-size-02267");
    m_errorMonitor->SetDesiredError("VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-arrayPitch-02268");
    CreateImageTest(*this, &image_info, "VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-depthPitch-02269");
}

TEST_F(NegativeImageDrm, Basic2) {
    RETURN_IF_SKIP(InitBasicImageDrm());
    std::vector<uint64_t> mods = GetFormatModifier(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    if (mods.empty()) {
        GTEST_SKIP() << "No valid Format Modifier found";
    }

    VkImageCreateInfo image_info = vku::InitStructHelper();
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.arrayLayers = 1;
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.mipLevels = 1;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageFormatProperties2 image_format_prop = vku::InitStructHelper();
    VkPhysicalDeviceImageFormatInfo2 image_format_info = vku::InitStructHelper();
    image_format_info.format = image_info.format;
    image_format_info.tiling = image_info.tiling;
    image_format_info.type = image_info.imageType;
    image_format_info.usage = image_info.usage;
    VkPhysicalDeviceImageDrmFormatModifierInfoEXT drm_format_mod_info = vku::InitStructHelper();
    drm_format_mod_info.drmFormatModifier = mods[0];
    drm_format_mod_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_format_info.pNext = (void *)&drm_format_mod_info;
    vk::GetPhysicalDeviceImageFormatProperties2(m_device->Physical().handle(), &image_format_info, &image_format_prop);

    VkSubresourceLayout fake_plane_layout = {0, 0, 0, 0, 0};

    VkImageDrmFormatModifierListCreateInfoEXT drm_format_mod_list = vku::InitStructHelper();
    drm_format_mod_list.drmFormatModifierCount = mods.size();
    drm_format_mod_list.pDrmFormatModifiers = mods.data();

    VkImageDrmFormatModifierExplicitCreateInfoEXT drm_format_mod_explicit = vku::InitStructHelper();
    drm_format_mod_explicit.drmFormatModifierPlaneCount = 1;
    drm_format_mod_explicit.pPlaneLayouts = &fake_plane_layout;

    image_info.pNext = (void *)&drm_format_mod_explicit;

    VkPhysicalDeviceImageDrmFormatModifierInfoEXT drm_format_modifier = vku::InitStructHelper();
    drm_format_modifier.drmFormatModifier = mods[1];
    image_format_info.pNext = &drm_format_modifier;
    VkResult result =
        vk::GetPhysicalDeviceImageFormatProperties2(m_device->Physical().handle(), &image_format_info, &image_format_prop);
    if (result == VK_ERROR_FORMAT_NOT_SUPPORTED) {
        GTEST_SKIP() << "Format VK_FORMAT_R8G8B8A8_UNORM not supported with format modifiers";
    }
    VkImage image = VK_NULL_HANDLE;
    // Postive check if only 1
    image_info.pNext = (void *)&drm_format_mod_list;
    vk::CreateImage(device(), &image_info, nullptr, &image);
    vk::DestroyImage(device(), image, nullptr);

    image_info.pNext = (void *)&drm_format_mod_explicit;
    vk::CreateImage(device(), &image_info, nullptr, &image);
    vk::DestroyImage(device(), image, nullptr);

    // Having both in pNext
    drm_format_mod_explicit.pNext = (void *)&drm_format_mod_list;
    CreateImageTest(*this, &image_info, "VUID-VkImageCreateInfo-tiling-02261");

    // Only 1 pNext but wrong tiling
    image_info.pNext = (void *)&drm_format_mod_list;
    image_info.tiling = VK_IMAGE_TILING_LINEAR;
    CreateImageTest(*this, &image_info, "VUID-VkImageCreateInfo-pNext-02262");
}

TEST_F(NegativeImageDrm, ImageFormatInfo) {
    TEST_DESCRIPTION("Validate VkPhysicalDeviceImageFormatInfo2.");
    RETURN_IF_SKIP(InitBasicImageDrm());

    VkPhysicalDeviceImageDrmFormatModifierInfoEXT image_drm_format_modifier = vku::InitStructHelper();

    VkPhysicalDeviceImageFormatInfo2 image_format_info = vku::InitStructHelper(&image_drm_format_modifier);
    image_format_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_format_info.type = VK_IMAGE_TYPE_2D;
    image_format_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_format_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_format_info.flags = 0;

    VkImageFormatProperties2 image_format_properties = vku::InitStructHelper();
    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceImageFormatInfo2-tiling-02249");
    vk::GetPhysicalDeviceImageFormatProperties2(Gpu(), &image_format_info, &image_format_properties);
    m_errorMonitor->VerifyFound();

    image_format_info.pNext = nullptr;
    image_format_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceImageFormatInfo2-tiling-02249");
    vk::GetPhysicalDeviceImageFormatProperties2(Gpu(), &image_format_info, &image_format_properties);
    m_errorMonitor->VerifyFound();

    VkImageFormatListCreateInfo format_list = vku::InitStructHelper(&image_drm_format_modifier);
    format_list.viewFormatCount = 0;  // Invalid
    image_format_info.pNext = &format_list;
    image_format_info.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceImageFormatInfo2-tiling-02313");
    vk::GetPhysicalDeviceImageFormatProperties2(Gpu(), &image_format_info, &image_format_properties);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageDrm, GetImageSubresourceLayoutPlane) {
    TEST_DESCRIPTION("Try to get image subresource layout for drm image plane 3 when it only has 2");
    RETURN_IF_SKIP(InitBasicImageDrm());

    VkFormat format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    std::vector<uint64_t> mods = GetFormatModifier(format, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT, 2);
    if (mods.empty()) {
        GTEST_SKIP() << "No valid Format Modifier found";
    }

    VkImageDrmFormatModifierListCreateInfoEXT list_create_info = vku::InitStructHelper();
    list_create_info.drmFormatModifierCount = mods.size();
    list_create_info.pDrmFormatModifiers = mods.data();
    VkImageCreateInfo create_info = vku::InitStructHelper(&list_create_info);
    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.format = format;
    create_info.extent.width = 64;
    create_info.extent.height = 64;
    create_info.extent.depth = 1;
    create_info.mipLevels = 1;
    create_info.arrayLayers = 1;
    create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    create_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    for (uint64_t mod : mods) {
        VkPhysicalDeviceImageDrmFormatModifierInfoEXT drm_format_modifier = vku::InitStructHelper();
        drm_format_modifier.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        drm_format_modifier.drmFormatModifier = mod;
        VkPhysicalDeviceImageFormatInfo2 image_info = vku::InitStructHelper(&drm_format_modifier);
        image_info.format = format;
        image_info.type = create_info.imageType;
        image_info.tiling = create_info.tiling;
        image_info.usage = create_info.usage;
        image_info.flags = create_info.flags;
        VkImageFormatProperties2 image_properties = vku::InitStructHelper();
        if (vk::GetPhysicalDeviceImageFormatProperties2(Gpu(), &image_info, &image_properties) != VK_SUCCESS) {
            // Works with Mesa, Pixel 7 doesn't support this combo
            GTEST_SKIP() << "Required formats/features not supported";
        }
    }

    vkt::Image image(*m_device, create_info, vkt::no_mem);
    if (image.initialized() == false) {
        GTEST_SKIP() << "Failed to create image.";
    }

    // Try to get layout for plane 3 when we only have 2
    VkImageSubresource subresource{};
    subresource.aspectMask = VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT;
    VkSubresourceLayout layout{};
    m_errorMonitor->SetDesiredError("VUID-vkGetImageSubresourceLayout-tiling-09433");
    vk::GetImageSubresourceLayout(m_device->handle(), image.handle(), &subresource, &layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageDrm, DeviceImageMemoryRequirements) {
    TEST_DESCRIPTION("Validate usage of VkDeviceImageMemoryRequirementsKHR.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicImageDrm());

    VkSubresourceLayout planeLayout = {0, 0, 0, 0, 0};
    VkImageDrmFormatModifierExplicitCreateInfoEXT drm_format_modifier_create_info = vku::InitStructHelper();
    drm_format_modifier_create_info.drmFormatModifierPlaneCount = 1;
    drm_format_modifier_create_info.pPlaneLayouts = &planeLayout;

    VkImageCreateInfo image_create_info = vku::InitStructHelper(&drm_format_modifier_create_info);
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.arrayLayers = 1;

    VkDeviceImageMemoryRequirementsKHR device_image_memory_requirements = vku::InitStructHelper();
    device_image_memory_requirements.pCreateInfo = &image_create_info;
    device_image_memory_requirements.planeAspect = VK_IMAGE_ASPECT_COLOR_BIT;

    VkMemoryRequirements2 memory_requirements = vku::InitStructHelper();

    m_errorMonitor->SetDesiredError("VUID-VkDeviceImageMemoryRequirements-pCreateInfo-06776");
    vk::GetDeviceImageMemoryRequirementsKHR(device(), &device_image_memory_requirements, &memory_requirements);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageDrm, ImageSubresourceRangeAspectMask) {
    TEST_DESCRIPTION("Test creating Image with invalid VkImageSubresourceRange aspectMask.");
    RETURN_IF_SKIP(InitBasicImageDrm());

    VkFormat mp_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    if (!FormatFeaturesAreSupported(Gpu(), mp_format, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    vkt::Image image(*m_device, 32, 32, 1, mp_format, VK_IMAGE_USAGE_SAMPLED_BIT);

    vkt::SamplerYcbcrConversion conversion(*m_device, mp_format);
    auto conversion_info = conversion.ConversionInfo();
    VkImageViewCreateInfo ivci = vku::InitStructHelper(&conversion_info);
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = mp_format;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT;

    m_errorMonitor->SetUnexpectedError("VUID-VkImageViewCreateInfo-subresourceRange-09594");
    CreateImageViewTest(*this, &ivci, "VUID-VkImageSubresourceRange-aspectMask-02278");
}

TEST_F(NegativeImageDrm, MutableFormat) {
    TEST_DESCRIPTION("use VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT with no VkImageFormatListCreateInfo.");
    RETURN_IF_SKIP(InitBasicImageDrm());

    std::vector<uint64_t> mods = GetFormatModifier(VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
    if (mods.empty()) {
        GTEST_SKIP() << "No valid Format Modifier found";
    }

    VkImageDrmFormatModifierListCreateInfoEXT mod_list = vku::InitStructHelper();
    mod_list.pDrmFormatModifiers = mods.data();
    mod_list.drmFormatModifierCount = mods.size();

    VkImageCreateInfo image_info = vku::InitStructHelper(&mod_list);
    image_info.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.arrayLayers = 1;
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.mipLevels = 1;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    CreateImageTest(*this, &image_info, "VUID-VkImageCreateInfo-tiling-02353");

    VkImageFormatListCreateInfo format_list = vku::InitStructHelper();
    format_list.viewFormatCount = 0;
    mod_list.pNext = &format_list;
    CreateImageTest(*this, &image_info, "VUID-VkImageCreateInfo-tiling-02353");
}

TEST_F(NegativeImageDrm, CompressionControl) {
    TEST_DESCRIPTION("mix VK_EXT_image_compression_control with DRM.");
    AddRequiredExtensions(VK_EXT_IMAGE_COMPRESSION_CONTROL_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicImageDrm());

    VkImageCompressionControlEXT compression_control = vku::InitStructHelper();
    compression_control.flags = VK_IMAGE_COMPRESSION_DEFAULT_EXT;
    compression_control.compressionControlPlaneCount = 1;
    compression_control.pFixedRateFlags = nullptr;

    VkSubresourceLayout fake_plane_layout = {0, 0, 0, 0, 0};
    VkImageDrmFormatModifierExplicitCreateInfoEXT drm_format_mod_explicit = vku::InitStructHelper(&compression_control);
    drm_format_mod_explicit.drmFormatModifierPlaneCount = 1;
    drm_format_mod_explicit.pPlaneLayouts = &fake_plane_layout;

    VkImageCreateInfo image_info = vku::InitStructHelper(&drm_format_mod_explicit);
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.arrayLayers = 1;
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.mipLevels = 1;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    CreateImageTest(*this, &image_info, "VUID-VkImageCreateInfo-pNext-06746");
}

TEST_F(NegativeImageDrm, GetImageDrmFormatModifierProperties) {
    TEST_DESCRIPTION("Use vkGetImageDrmFormatModifierPropertiesEXT");
    RETURN_IF_SKIP(InitBasicImageDrm());

    VkImageCreateInfo image_info = vku::InitStructHelper();
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.extent = {128, 128, 1};
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;  // not DRM tiling
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkt::Image image(*m_device, image_info);

    VkImageDrmFormatModifierPropertiesEXT props = vku::InitStructHelper();
    m_errorMonitor->SetDesiredError("VUID-vkGetImageDrmFormatModifierPropertiesEXT-image-02272");
    vk::GetImageDrmFormatModifierPropertiesEXT(device(), image.handle(), &props);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkGetImageDrmFormatModifierPropertiesEXT-image-parameter");
    VkImage bad_image = CastFromUint64<VkImage>(0xFFFFEEEE);
    vk::GetImageDrmFormatModifierPropertiesEXT(device(), bad_image, &props);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageDrm, PhysicalDeviceImageDrmFormatModifierInfo) {
    TEST_DESCRIPTION("Use vkPhysicalDeviceImageDrmFormatModifierInfo with VK_SHARING_MODE_CONCURRENT");
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicImageDrm());

    VkPhysicalDeviceImageDrmFormatModifierInfoEXT drm_format_modifier = vku::InitStructHelper();
    drm_format_modifier.sharingMode = VK_SHARING_MODE_CONCURRENT;

    VkPhysicalDeviceExternalImageFormatInfo external_image_info = vku::InitStructHelper(&drm_format_modifier);
    external_image_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    VkPhysicalDeviceImageFormatInfo2 image_info = vku::InitStructHelper(&external_image_info);
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.type = VK_IMAGE_TYPE_2D;
    image_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_info.flags = 0;

    VkImageFormatProperties2 image_properties = vku::InitStructHelper();

    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceImageDrmFormatModifierInfoEXT-sharingMode-02315");
    vk::GetPhysicalDeviceImageFormatProperties2(Gpu(), &image_info, &image_properties);
    m_errorMonitor->VerifyFound();

    drm_format_modifier.queueFamilyIndexCount = 2;
    drm_format_modifier.pQueueFamilyIndices = nullptr;
    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceImageDrmFormatModifierInfoEXT-sharingMode-02314");
    vk::GetPhysicalDeviceImageFormatProperties2(Gpu(), &image_info, &image_properties);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageDrm, PhysicalDeviceImageDrmFormatModifierInfoQuery) {
    TEST_DESCRIPTION("Use vkPhysicalDeviceImageDrmFormatModifierInfo with VK_SHARING_MODE_CONCURRENT");
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicImageDrm());

    uint32_t queue_family_property_count = 0;
    vk::GetPhysicalDeviceQueueFamilyProperties2(Gpu(), &queue_family_property_count, nullptr);
    if (queue_family_property_count < 2) {
        GTEST_SKIP() << "pQueueFamilyPropertyCount is not 2 or more";
    }
    uint32_t queue_family_indices[2] = {0, 1};

    VkPhysicalDeviceImageDrmFormatModifierInfoEXT drm_format_modifier = vku::InitStructHelper();
    drm_format_modifier.sharingMode = VK_SHARING_MODE_CONCURRENT;
    drm_format_modifier.queueFamilyIndexCount = 2;
    drm_format_modifier.pQueueFamilyIndices = queue_family_indices;

    VkPhysicalDeviceExternalImageFormatInfo external_image_info = vku::InitStructHelper(&drm_format_modifier);
    external_image_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    VkPhysicalDeviceImageFormatInfo2 image_info = vku::InitStructHelper(&external_image_info);
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.type = VK_IMAGE_TYPE_2D;
    image_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_info.flags = 0;

    VkImageFormatProperties2 image_properties = vku::InitStructHelper();

    // Count too large
    queue_family_indices[0] = queue_family_property_count + 1;
    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceImageDrmFormatModifierInfoEXT-sharingMode-02316");
    vk::GetPhysicalDeviceImageFormatProperties2(Gpu(), &image_info, &image_properties);
    m_errorMonitor->VerifyFound();

    // Not unique indices
    queue_family_indices[0] = 0;
    queue_family_indices[1] = 0;
    drm_format_modifier.queueFamilyIndexCount = queue_family_property_count;
    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceImageDrmFormatModifierInfoEXT-sharingMode-02316");
    vk::GetPhysicalDeviceImageFormatProperties2(Gpu(), &image_info, &image_properties);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageDrm, MultiPlanarGetImageMemoryRequirements) {
    RETURN_IF_SKIP(InitBasicImageDrm());

    VkFormat format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    std::vector<uint64_t> mods = GetFormatModifier(format, VK_FORMAT_FEATURE_DISJOINT_BIT, 3);
    if (mods.empty()) {
        GTEST_SKIP() << "No valid Format Modifier found";
    }

    auto list_create_info = vku::InitStruct<VkImageDrmFormatModifierListCreateInfoEXT>();
    list_create_info.drmFormatModifierCount = mods.size();
    list_create_info.pDrmFormatModifiers = mods.data();
    auto create_info = vku::InitStruct<VkImageCreateInfo>(&list_create_info);
    create_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;
    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.format = format;
    create_info.extent.width = 64;
    create_info.extent.height = 64;
    create_info.extent.depth = 1;
    create_info.mipLevels = 1;
    create_info.arrayLayers = 1;
    create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    create_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    for (uint64_t mod : mods) {
        auto drm_format_modifier = vku::InitStruct<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>();
        drm_format_modifier.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        drm_format_modifier.drmFormatModifier = mod;
        auto image_info = vku::InitStruct<VkPhysicalDeviceImageFormatInfo2>(&drm_format_modifier);
        image_info.format = format;
        image_info.type = create_info.imageType;
        image_info.tiling = create_info.tiling;
        image_info.usage = create_info.usage;
        image_info.flags = create_info.flags;
        auto image_properties = vku::InitStruct<VkImageFormatProperties2>();
        if (vk::GetPhysicalDeviceImageFormatProperties2(Gpu(), &image_info, &image_properties) != VK_SUCCESS) {
            GTEST_SKIP() << "Required formats/features not supported";
        }
    }

    vkt::Image image(*m_device, create_info, vkt::no_mem);
    if (image.initialized() == false) {
        GTEST_SKIP() << "Failed to create image.";
    }

    auto image_plane_req = vku::InitStruct<VkImagePlaneMemoryRequirementsInfo>();
    auto mem_req_info2 = vku::InitStruct<VkImageMemoryRequirementsInfo2>(&image_plane_req);
    mem_req_info2.image = image;

    auto mem_reqs2 = vku::InitStruct<VkMemoryRequirements2>();

    // should be VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT
    image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkImagePlaneMemoryRequirementsInfo-planeAspect-02282");
    vk::GetImageMemoryRequirements2(device(), &mem_req_info2, &mem_reqs2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageDrm, MultiPlanarBindMemory) {
    RETURN_IF_SKIP(InitBasicImageDrm());

    VkFormat format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    std::vector<uint64_t> mods = GetFormatModifier(format, VK_FORMAT_FEATURE_DISJOINT_BIT, 3);
    if (mods.empty()) {
        GTEST_SKIP() << "No valid Format Modifier found";
    }

    auto list_create_info = vku::InitStruct<VkImageDrmFormatModifierListCreateInfoEXT>();
    list_create_info.drmFormatModifierCount = mods.size();
    list_create_info.pDrmFormatModifiers = mods.data();
    auto create_info = vku::InitStruct<VkImageCreateInfo>(&list_create_info);
    create_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;
    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.format = format;
    create_info.extent.width = 64;
    create_info.extent.height = 64;
    create_info.extent.depth = 1;
    create_info.mipLevels = 1;
    create_info.arrayLayers = 1;
    create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    create_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    for (uint64_t mod : mods) {
        auto drm_format_modifier = vku::InitStruct<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>();
        drm_format_modifier.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        drm_format_modifier.drmFormatModifier = mod;
        auto image_info = vku::InitStruct<VkPhysicalDeviceImageFormatInfo2>(&drm_format_modifier);
        image_info.format = format;
        image_info.type = create_info.imageType;
        image_info.tiling = create_info.tiling;
        image_info.usage = create_info.usage;
        image_info.flags = create_info.flags;
        auto image_properties = vku::InitStruct<VkImageFormatProperties2>();
        if (vk::GetPhysicalDeviceImageFormatProperties2(Gpu(), &image_info, &image_properties) != VK_SUCCESS) {
            GTEST_SKIP() << "Required formats/features not supported";
        }
    }

    vkt::Image image(*m_device, create_info, vkt::no_mem);
    if (image.initialized() == false) {
        GTEST_SKIP() << "Failed to create image.";
    }

    // Allocate & bind memory
    auto image_plane_req = vku::InitStruct<VkImagePlaneMemoryRequirementsInfo>();
    auto mem_req_info2 = vku::InitStruct<VkImageMemoryRequirementsInfo2>(&image_plane_req);
    mem_req_info2.image = image;
    auto mem_reqs2 = vku::InitStruct<VkMemoryRequirements2>();

    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    auto alloc_info = vku::InitStruct<VkMemoryAllocateInfo>();

    // Plane 0
    image_plane_req.planeAspect = VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT;
    vk::GetImageMemoryRequirements2(device(), &mem_req_info2, &mem_reqs2);
    uint32_t mem_type = 0;
    auto phys_mem_props2 = vku::InitStruct<VkPhysicalDeviceMemoryProperties2>();
    vk::GetPhysicalDeviceMemoryProperties2(Gpu(), &phys_mem_props2);
    for (mem_type = 0; mem_type < phys_mem_props2.memoryProperties.memoryTypeCount; mem_type++) {
        if ((mem_reqs2.memoryRequirements.memoryTypeBits & (1 << mem_type)) &&
            ((phys_mem_props2.memoryProperties.memoryTypes[mem_type].propertyFlags & mem_props) == mem_props)) {
            alloc_info.memoryTypeIndex = mem_type;
            break;
        }
    }
    alloc_info.allocationSize = mem_reqs2.memoryRequirements.size;
    vkt::DeviceMemory p0_mem(*m_device, alloc_info);

    // Plane 1 & 2 use same memory type
    image_plane_req.planeAspect = VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT;
    vk::GetImageMemoryRequirements2(device(), &mem_req_info2, &mem_reqs2);
    alloc_info.allocationSize = mem_reqs2.memoryRequirements.size;
    vkt::DeviceMemory p1_mem(*m_device, alloc_info);

    image_plane_req.planeAspect = VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT;
    vk::GetImageMemoryRequirements2(device(), &mem_req_info2, &mem_reqs2);
    alloc_info.allocationSize = mem_reqs2.memoryRequirements.size;
    vkt::DeviceMemory p2_mem(*m_device, alloc_info);

    // Set up 3-plane binding
    VkBindImageMemoryInfo bind_info[3];
    VkBindImagePlaneMemoryInfo plane_info[3];
    for (int plane = 0; plane < 3; plane++) {
        plane_info[plane] = vku::InitStruct<VkBindImagePlaneMemoryInfo>();
        bind_info[plane] = vku::InitStruct<VkBindImageMemoryInfo>(&plane_info[plane]);
        bind_info[plane].image = image;
        bind_info[plane].memoryOffset = 0;
    }
    bind_info[0].memory = p0_mem.handle();
    bind_info[1].memory = p1_mem.handle();
    bind_info[2].memory = p2_mem.handle();
    plane_info[0].planeAspect = VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT;
    plane_info[1].planeAspect = VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT;
    plane_info[2].planeAspect = VK_IMAGE_ASPECT_PLANE_2_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkBindImagePlaneMemoryInfo-planeAspect-02284");
    vk::BindImageMemory2(device(), 3, bind_info);
    m_errorMonitor->VerifyFound();
}