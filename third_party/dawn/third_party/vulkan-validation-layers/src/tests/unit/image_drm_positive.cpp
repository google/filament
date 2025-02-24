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

void ImageDrmTest::InitBasicImageDrm() {
    SetTargetApiVersion(VK_API_VERSION_1_2);  // required extension added here
    AddRequiredExtensions(VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::samplerYcbcrConversion);
    RETURN_IF_SKIP(Init());
}

std::vector<uint64_t> ImageDrmTest::GetFormatModifier(VkFormat format, VkFormatFeatureFlags2 features, uint32_t plane_count) {
    std::vector<uint64_t> mods;
    VkDrmFormatModifierPropertiesListEXT mod_props = vku::InitStructHelper();
    VkFormatProperties2 format_props = vku::InitStructHelper(&mod_props);
    vk::GetPhysicalDeviceFormatProperties2(Gpu(), format, &format_props);
    if (mod_props.drmFormatModifierCount == 0) {
        return mods;
    }

    std::vector<VkDrmFormatModifierPropertiesEXT> mod_props_length(mod_props.drmFormatModifierCount);
    mod_props.pDrmFormatModifierProperties = mod_props_length.data();
    vk::GetPhysicalDeviceFormatProperties2(Gpu(), format, &format_props);

    for (uint32_t i = 0; i < mod_props.drmFormatModifierCount; ++i) {
        auto &mod = mod_props.pDrmFormatModifierProperties[i];
        if (((mod.drmFormatModifierTilingFeatures & features) == features) && (plane_count == mod.drmFormatModifierPlaneCount)) {
            mods.push_back(mod.drmFormatModifier);
        }
    }

    return mods;
}

class PositiveImageDrm : public ImageDrmTest {};

TEST_F(PositiveImageDrm, Basic) {
    // See https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/2610
    TEST_DESCRIPTION("Create image and imageView using VK_EXT_image_drm_format_modifier");
    RETURN_IF_SKIP(InitBasicImageDrm());

    // we just hope that one of these formats supports modifiers
    // for more detailed checking, we could also check multi-planar formats.
    auto format_list = {
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_FORMAT_B8G8R8A8_SRGB,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_R8G8B8A8_SRGB,
    };

    for (auto format : format_list) {
        std::vector<uint64_t> mods =
            GetFormatModifier(format, VK_FORMAT_FEATURE_TRANSFER_DST_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
        if (mods.empty()) {
            continue;
        }

        // create image
        VkImageCreateInfo ci = vku::InitStructHelper();
        ci.flags = 0;
        ci.imageType = VK_IMAGE_TYPE_2D;
        ci.format = format;
        ci.extent = {128, 128, 1};
        ci.mipLevels = 1;
        ci.arrayLayers = 1;
        ci.samples = VK_SAMPLE_COUNT_1_BIT;
        ci.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
        ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VkImageDrmFormatModifierListCreateInfoEXT mod_list = vku::InitStructHelper();
        mod_list.pDrmFormatModifiers = mods.data();
        mod_list.drmFormatModifierCount = mods.size();
        ci.pNext = &mod_list;

        VkImage image;
        VkResult err = vk::CreateImage(device(), &ci, nullptr, &image);
        ASSERT_EQ(VK_SUCCESS, err);

        // bind memory
        VkPhysicalDeviceMemoryProperties phys_mem_props;
        vk::GetPhysicalDeviceMemoryProperties(Gpu(), &phys_mem_props);
        VkMemoryRequirements mem_reqs;
        vk::GetImageMemoryRequirements(device(), image, &mem_reqs);
        VkDeviceMemory mem_obj = VK_NULL_HANDLE;
        VkMemoryPropertyFlagBits mem_props = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        for (uint32_t type = 0; type < phys_mem_props.memoryTypeCount; type++) {
            if ((mem_reqs.memoryTypeBits & (1 << type)) &&
                ((phys_mem_props.memoryTypes[type].propertyFlags & mem_props) == mem_props)) {
                VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
                alloc_info.allocationSize = mem_reqs.size;
                alloc_info.memoryTypeIndex = type;
                ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &alloc_info, nullptr, &mem_obj));
                break;
            }
        }

        ASSERT_NE((VkDeviceMemory)VK_NULL_HANDLE, mem_obj);
        ASSERT_EQ(VK_SUCCESS, vk::BindImageMemory(device(), image, mem_obj, 0));

        // create image view
        VkImageViewCreateInfo ivci = {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            nullptr,
            0,
            image,
            VK_IMAGE_VIEW_TYPE_2D,
            format,
            {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
             VK_COMPONENT_SWIZZLE_IDENTITY},
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
        };

        CreateImageViewTest(*this, &ivci);

        // for more detailed checking, we could export the image to dmabuf
        // and then import it again (using VkImageDrmFormatModifierExplicitCreateInfoEXT)

        vk::FreeMemory(device(), mem_obj, nullptr);
        vk::DestroyImage(device(), image, nullptr);
    }
}

TEST_F(PositiveImageDrm, ExternalMemory) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5649
    TEST_DESCRIPTION(
        "Create image with VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT and VkExternalMemoryImageCreateInfo in the pNext chain");

    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicImageDrm());

    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    std::vector<uint64_t> mods = GetFormatModifier(format, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);
    if (mods.empty()) {
        GTEST_SKIP() << "No valid Format Modifier found";
    }

    VkExternalMemoryImageCreateInfo external_info = vku::InitStructHelper();
    external_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    // handleTypes needs to be assigned to trigger the behavior we want
    assert(external_info.handleTypes);

    VkImageDrmFormatModifierListCreateInfoEXT drm_info = vku::InitStructHelper(&external_info);
    drm_info.drmFormatModifierCount = size32(mods);
    drm_info.pDrmFormatModifiers = mods.data();

    VkImageCreateInfo ci = vku::InitStructHelper(&drm_info);
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = format;
    ci.extent = {128, 128, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    {
        VkPhysicalDeviceImageDrmFormatModifierInfoEXT drm_format_modifier = vku::InitStructHelper();
        drm_format_modifier.sharingMode = ci.sharingMode;
        drm_format_modifier.queueFamilyIndexCount = ci.queueFamilyIndexCount;
        drm_format_modifier.pQueueFamilyIndices = ci.pQueueFamilyIndices;
        VkPhysicalDeviceExternalImageFormatInfo external_image_info = vku::InitStructHelper(&drm_format_modifier);
        external_image_info.handleType = static_cast<VkExternalMemoryHandleTypeFlagBits>(external_info.handleTypes);
        VkPhysicalDeviceImageFormatInfo2 image_info = vku::InitStructHelper(&external_image_info);
        image_info.format = ci.format;
        image_info.type = ci.imageType;
        image_info.tiling = ci.tiling;
        image_info.usage = ci.usage;
        image_info.flags = ci.flags;

        VkExternalImageFormatProperties external_image_properties = vku::InitStructHelper();
        VkImageFormatProperties2 image_properties = vku::InitStructHelper(&external_image_properties);

        if (const auto result = vk::GetPhysicalDeviceImageFormatProperties2(Gpu(), &image_info, &image_properties);
            result != VK_SUCCESS) {
            GTEST_SKIP() << "Unable to create image. VkResult = " << string_VkResult(result);
        }

        if ((external_image_properties.externalMemoryProperties.compatibleHandleTypes & external_info.handleTypes) !=
            external_info.handleTypes) {
            GTEST_SKIP() << "Unable to create image, VkExternalMemoryImageCreateInfo::handleTypes not supported";
        }
    }

    CreateImageTest(*this, &ci);
}

TEST_F(PositiveImageDrm, GetImageSubresourceLayoutPlane) {
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

    VkImageSubresource subresource{VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT, 0, 0};
    VkSubresourceLayout layout{};
    vk::GetImageSubresourceLayout(m_device->handle(), image.handle(), &subresource, &layout);
}

TEST_F(PositiveImageDrm, MutableFormat) {
    TEST_DESCRIPTION("use VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT with no VkImageFormatListCreateInfo .");
    RETURN_IF_SKIP(InitBasicImageDrm());

    std::vector<uint64_t> mods = GetFormatModifier(VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
    if (mods.empty()) {
        GTEST_SKIP() << "No valid Format Modifier found";
    }

    VkImageDrmFormatModifierListCreateInfoEXT mod_list = vku::InitStructHelper();
    mod_list.pDrmFormatModifiers = mods.data();
    mod_list.drmFormatModifierCount = mods.size();

    VkFormat formats = VK_FORMAT_R8G8B8A8_SNORM;
    VkImageFormatListCreateInfo format_list = vku::InitStructHelper(&mod_list);
    format_list.viewFormatCount = 1;
    format_list.pViewFormats = &formats;

    VkImageCreateInfo image_info = vku::InitStructHelper(&format_list);
    image_info.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.extent = {128, 128, 1};
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    CreateImageTest(*this, &image_info);
}

TEST_F(PositiveImageDrm, GetImageDrmFormatModifierProperties) {
    TEST_DESCRIPTION("Use vkGetImageDrmFormatModifierPropertiesEXT");
    RETURN_IF_SKIP(InitBasicImageDrm());

    std::vector<uint64_t> mods = GetFormatModifier(VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
    if (mods.empty()) {
        GTEST_SKIP() << "No valid Format Modifier found";
    }

    VkImageDrmFormatModifierListCreateInfoEXT mod_list = vku::InitStructHelper();
    mod_list.pDrmFormatModifiers = mods.data();
    mod_list.drmFormatModifierCount = mods.size();

    VkImageCreateInfo image_info = vku::InitStructHelper(&mod_list);
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.extent = {128, 128, 1};
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkt::Image image(*m_device, image_info);

    VkImageDrmFormatModifierPropertiesEXT props = vku::InitStructHelper();
    vk::GetImageDrmFormatModifierPropertiesEXT(device(), image.handle(), &props);
}

TEST_F(PositiveImageDrm, PhysicalDeviceImageDrmFormatModifierInfoExclusive) {
    TEST_DESCRIPTION("Use vkPhysicalDeviceImageDrmFormatModifierInfo with VK_SHARING_MODE_EXCLUSIVE");
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicImageDrm());

    VkPhysicalDeviceImageDrmFormatModifierInfoEXT drm_format_modifier = vku::InitStructHelper();
    drm_format_modifier.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkPhysicalDeviceExternalImageFormatInfo external_image_info = vku::InitStructHelper(&drm_format_modifier);
    external_image_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    VkPhysicalDeviceImageFormatInfo2 image_info = vku::InitStructHelper(&external_image_info);
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.type = VK_IMAGE_TYPE_2D;
    image_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_info.flags = 0;

    VkExternalImageFormatProperties external_image_properties = vku::InitStructHelper();
    VkImageFormatProperties2 image_properties = vku::InitStructHelper(&external_image_properties);

    vk::GetPhysicalDeviceImageFormatProperties2(Gpu(), &image_info, &image_properties);
}

TEST_F(PositiveImageDrm, PhysicalDeviceImageDrmFormatModifierInfoConcurrent) {
    TEST_DESCRIPTION("Use vkPhysicalDeviceImageDrmFormatModifierInfo with VK_SHARING_MODE_CONCURRENT");
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicImageDrm());

    uint32_t queue_family_property_count = 0;
    vk::GetPhysicalDeviceQueueFamilyProperties2(Gpu(), &queue_family_property_count, nullptr);
    if (queue_family_property_count < 2) {
        GTEST_SKIP() << "pQueueFamilyPropertyCount is not 2 or more";
    }
    std::vector<VkQueueFamilyProperties2> queue_family_props(queue_family_property_count);
    vk::GetPhysicalDeviceQueueFamilyProperties2(Gpu(), &queue_family_property_count, nullptr);

    VkPhysicalDeviceImageDrmFormatModifierInfoEXT drm_format_modifier = vku::InitStructHelper();
    drm_format_modifier.sharingMode = VK_SHARING_MODE_CONCURRENT;
    drm_format_modifier.queueFamilyIndexCount = 2;
    uint32_t queue_family_indices[2] = {0, 1};
    drm_format_modifier.pQueueFamilyIndices = queue_family_indices;

    VkPhysicalDeviceExternalImageFormatInfo external_image_info = vku::InitStructHelper(&drm_format_modifier);
    external_image_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    VkPhysicalDeviceImageFormatInfo2 image_info = vku::InitStructHelper(&external_image_info);
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.type = VK_IMAGE_TYPE_2D;
    image_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_info.flags = 0;

    VkExternalImageFormatProperties external_image_properties = vku::InitStructHelper();
    VkImageFormatProperties2 image_properties = vku::InitStructHelper(&external_image_properties);

    vk::GetPhysicalDeviceImageFormatProperties2(Gpu(), &image_info, &image_properties);
}
