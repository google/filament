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

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/render_pass_helper.h"
#include "../framework/barrier_queue_family.h"

#include "utils/vk_layer_utils.h"

// A reasonable well supported default VkImageCreateInfo for image creation
VkImageCreateInfo ImageTest::DefaultImageInfo() {
    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.flags = 0;                          // assumably any is supported
    ci.imageType = VK_IMAGE_TYPE_2D;       // any is supported
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;  // has mandatory support for all usages
    ci.extent = {64, 64, 1};               // limit is 256 for 3D, or 4096
    ci.mipLevels = 1;                      // any is supported
    ci.arrayLayers = 1;                    // limit is 256
    ci.samples = VK_SAMPLE_COUNT_1_BIT;    // needs to be 1 if TILING_LINEAR
    // if VK_IMAGE_TILING_LINEAR imageType must be 2D, usage must be TRANSFER, and levels layers samplers all 1
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;  // depends on format
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    return ci;
}

class PositiveImage : public ImageTest {};

TEST_F(PositiveImage, OwnershipTranfersImage) {
    TEST_DESCRIPTION("Valid image ownership transfers that shouldn't create errors");
    RETURN_IF_SKIP(Init());

    vkt::Queue *no_gfx_queue = m_device->QueueWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT);
    if (!no_gfx_queue) {
        GTEST_SKIP() << "Required queue not present (non-graphics non-compute capable required)";
    }

    vkt::CommandPool no_gfx_pool(*m_device, no_gfx_queue->family_index, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer no_gfx_cb(*m_device, no_gfx_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    // Create an "exclusive" image owned by the graphics queue.
    VkFlags image_use = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, image_use);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    auto image_subres = image.SubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
    auto image_barrier = image.ImageMemoryBarrier(0, 0, image.Layout(), image.Layout(), image_subres);
    image_barrier.srcQueueFamilyIndex = m_device->graphics_queue_node_index_;
    image_barrier.dstQueueFamilyIndex = no_gfx_queue->family_index;

    ValidOwnershipTransfer(m_errorMonitor, m_default_queue, m_command_buffer, no_gfx_queue, no_gfx_cb,
                           VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, nullptr, &image_barrier);

    // Change layouts while changing ownership
    image_barrier.srcQueueFamilyIndex = no_gfx_queue->family_index;
    image_barrier.dstQueueFamilyIndex = m_device->graphics_queue_node_index_;
    image_barrier.oldLayout = image.Layout();
    // Make sure the new layout is different from the old
    if (image_barrier.oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        image_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    } else {
        image_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    ValidOwnershipTransfer(m_errorMonitor, no_gfx_queue, no_gfx_cb, m_default_queue, m_command_buffer,
                           VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, nullptr, &image_barrier);
}

TEST_F(PositiveImage, AliasedMemoryTracking) {
    TEST_DESCRIPTION(
        "Create a buffer, allocate memory, bind memory, destroy the buffer, create an image, and bind the same memory to it");

    RETURN_IF_SKIP(Init());

    VkDeviceSize buff_size = 256;
    auto buffer = std::make_unique<vkt::Buffer>(*m_device, vkt::Buffer::CreateInfo(buff_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
                                                vkt::no_mem);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;  // mandatory format
    image_create_info.extent.width = 64;                  // at least 4096x4096 is supported
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    const auto buffer_memory_requirements = buffer->MemoryRequirements();
    const auto image_memory_requirements = image.MemoryRequirements();

    vkt::DeviceMemory mem;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = (std::max)(buffer_memory_requirements.size, image_memory_requirements.size);
    bool has_memtype =
        m_device->Physical().SetMemoryType(buffer_memory_requirements.memoryTypeBits & image_memory_requirements.memoryTypeBits,
                                           &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!has_memtype) {
        GTEST_SKIP() << "Failed to find a host visible memory type for both a buffer and an image";
    }
    mem.init(*m_device, alloc_info);

    auto pData = mem.Map();
    std::memset(pData, 0xCADECADE, static_cast<size_t>(buff_size));
    mem.Unmap();

    buffer->BindMemory(mem, 0);

    // NOW, destroy the buffer. Obviously, the resource no longer occupies this
    // memory. In fact, it was never used by the GPU.
    // Just be sure, wait for idle.
    buffer.reset(nullptr);
    m_device->Wait();

    // VALIDATION FAILURE:
    image.BindMemory(mem, 0);
}

TEST_F(PositiveImage, CreateImageViewFollowsParameterCompatibilityRequirements) {
    TEST_DESCRIPTION("Verify that creating an ImageView with valid usage does not generate validation errors.");
    RETURN_IF_SKIP(Init());
    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image_ci.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    image.CreateView();
}

TEST_F(PositiveImage, BasicUsage) {
    TEST_DESCRIPTION("Verify that we can create a view with usage INPUT_ATTACHMENT");
    RETURN_IF_SKIP(Init());
    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    vkt::ImageView view = image.CreateView();
}

TEST_F(PositiveImage, FormatCompatibility) {
    TEST_DESCRIPTION("Test format compatibility");
    AddRequiredExtensions(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkFormat format = VK_FORMAT_R12X4G12X4_UNORM_2PACK16;

    VkImageFormatListCreateInfo format_list = vku::InitStructHelper();
    format_list.viewFormatCount = 1;
    format_list.pViewFormats = &format;

    VkImageCreateInfo image_create_info = vku::InitStructHelper(&format_list);
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.flags = 0;
    vkt::Image image(*m_device, image_create_info);
}

TEST_F(PositiveImage, MultpilePNext) {
    TEST_DESCRIPTION(
        "Use VkImageFormatListCreateInfo and VkImageCompressionControlEXT to make sure internal "
        "DispatchGetPhysicalDeviceImageFormatProperties2 pass them along");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_IMAGE_COMPRESSION_CONTROL_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const VkFormat view_format = VK_FORMAT_R8G8B8A8_UINT;

    VkImageFormatListCreateInfo format_list = vku::InitStructHelper();
    format_list.viewFormatCount = 1;
    format_list.pViewFormats = &view_format;

    VkImageCompressionControlEXT image_compression = vku::InitStructHelper(&format_list);
    image_compression.compressionControlPlaneCount = 0;
    image_compression.flags = VK_IMAGE_COMPRESSION_DEFAULT_EXT;

    VkImageCreateInfo image_create_info = vku::InitStructHelper(&image_compression);
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.flags = 0;

    vkt::Image(*m_device, image_create_info);
}

TEST_F(PositiveImage, FramebufferFrom3DImage) {
    TEST_DESCRIPTION("Validate creating a framebuffer from a 3D image.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

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

    VkImageViewCreateInfo dsvci = vku::InitStructHelper();
    dsvci.image = image.handle();
    dsvci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    dsvci.format = VK_FORMAT_B8G8R8A8_UNORM;
    dsvci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    dsvci.subresourceRange.baseMipLevel = 0;
    dsvci.subresourceRange.layerCount = 4;
    dsvci.subresourceRange.baseArrayLayer = 0;
    dsvci.subresourceRange.levelCount = 1;
    vkt::ImageView view(*m_device, dsvci);

    VkFramebufferCreateInfo fci = vku::InitStructHelper();
    fci.renderPass = m_renderPass;
    fci.attachmentCount = 1;
    fci.pAttachments = &view.handle();
    fci.width = 32;
    fci.height = 32;
    fci.layers = 4;
    vkt::Framebuffer fb(*m_device, fci);
}

TEST_F(PositiveImage, ExtendedUsageWithDifferentFormatViews) {
    TEST_DESCRIPTION("Create views with different formats of an image with extended usage");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags =
        VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT | VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT;
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_BC3_UNORM_BLOCK;
    image_ci.extent = {
        64,  // width
        64,  // height
        1,   // depth
    };
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage =
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageFormatProperties image_properties;
    VkResult err = vk::GetPhysicalDeviceImageFormatProperties(Gpu(), image_ci.format, image_ci.imageType, image_ci.tiling,
                                                              image_ci.usage, image_ci.flags, &image_properties);
    // Test not supported by driver
    if (err != VK_SUCCESS) {
        GTEST_SKIP() << "Image format not valid for format, type, tiling, usage and flags combination.";
    }

    vkt::Image image(*m_device, image_ci);
    ASSERT_TRUE(image.handle() != VK_NULL_HANDLE);

    // Since the format is compatible with all image's usage, there's no need to restrict usage
    VkImageViewCreateInfo iv_ci = vku::InitStructHelper();
    iv_ci.image = image.handle();
    iv_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    iv_ci.format = VK_FORMAT_R32G32B32A32_UINT;
    iv_ci.subresourceRange.layerCount = 1;
    iv_ci.subresourceRange.baseMipLevel = 0;
    iv_ci.subresourceRange.levelCount = 1;
    iv_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkt::ImageView view(*m_device, iv_ci);
    ASSERT_TRUE(view.handle() != VK_NULL_HANDLE);

    // Since usage is inherited from the image, we need to restrict the usage to a subset
    // Compressed images do not support storage, but we want to sample from the compressed
    VkImageViewUsageCreateInfo ivu_ci = vku::InitStructHelper();
    ivu_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    iv_ci.pNext = &ivu_ci;
    vkt::ImageView view2(*m_device, iv_ci);
    ASSERT_TRUE(view2.handle() != VK_NULL_HANDLE);
}

TEST_F(PositiveImage, ImageCompressionControl) {
    TEST_DESCRIPTION("Checks for creating fixed rate compression image.");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredExtensions(VK_EXT_IMAGE_COMPRESSION_CONTROL_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imageCompressionControl);
    RETURN_IF_SKIP(Init());

    // Query possible image format with vkGetPhysicalDeviceImageFormatProperties2KHR
    VkPhysicalDeviceImageFormatInfo2 image_format_info = vku::InitStructHelper();
    image_format_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_format_info.tiling = VK_IMAGE_TILING_LINEAR;
    image_format_info.type = VK_IMAGE_TYPE_2D;
    image_format_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    VkImageCompressionPropertiesEXT compression_properties = vku::InitStructHelper();
    VkImageFormatProperties2 image_format_properties = vku::InitStructHelper(&compression_properties);

    vk::GetPhysicalDeviceImageFormatProperties2(Gpu(), &image_format_info, &image_format_properties);

    auto image_ci = vkt::Image::CreateInfo();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.tiling = VK_IMAGE_TILING_LINEAR;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    // Create with disabled image compression
    {
        VkImageCompressionControlEXT compressionControl = vku::InitStructHelper();
        compressionControl.flags = VK_IMAGE_COMPRESSION_DISABLED_EXT;
        image_ci.pNext = &compressionControl;

        vkt::Image image(*m_device, image_ci);
    }

    // Create with default image compression, without fixed rate
    {
        VkImageCompressionControlEXT compressionControl = vku::InitStructHelper();
        compressionControl.flags = VK_IMAGE_COMPRESSION_DEFAULT_EXT;
        image_ci.pNext = &compressionControl;

        vkt::Image image(*m_device, image_ci);
    }

    // Create with fixed rate compression image
    {
        VkImageCompressionControlEXT compressionControl = vku::InitStructHelper();
        compressionControl.flags = VK_IMAGE_COMPRESSION_FIXED_RATE_DEFAULT_EXT;
        image_ci.pNext = &compressionControl;

        vkt::Image image(*m_device, image_ci);
    }

    // Create with fixed rate compression image
    if (compression_properties.imageCompressionFixedRateFlags != VK_IMAGE_COMPRESSION_FIXED_RATE_NONE_EXT) {
        VkImageCompressionFixedRateFlagsEXT supported_compression_fixed_rate = VK_IMAGE_COMPRESSION_FIXED_RATE_NONE_EXT;

        for (uint32_t index = 0; index < 32; index++) {
            if ((compression_properties.imageCompressionFixedRateFlags & (1 << index)) != 0) {
                supported_compression_fixed_rate = (1 << index);
                break;
            }
        }

        ASSERT_TRUE(supported_compression_fixed_rate != VK_IMAGE_COMPRESSION_FIXED_RATE_NONE_EXT);

        VkImageCompressionFixedRateFlagsEXT fixedRageFlags = supported_compression_fixed_rate;
        VkImageCompressionControlEXT compressionControl = vku::InitStructHelper();
        compressionControl.flags = VK_IMAGE_COMPRESSION_FIXED_RATE_EXPLICIT_EXT;
        compressionControl.pFixedRateFlags = &fixedRageFlags;
        compressionControl.compressionControlPlaneCount = 1;
        image_ci.pNext = &compressionControl;

        vkt::Image image(*m_device, image_ci);
    }
}

TEST_F(PositiveImage, Create3DImageView) {
    TEST_DESCRIPTION("Create a 3D image view");

    RETURN_IF_SKIP(Init());

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.extent = {32, 32, 8};
    ci.mipLevels = 6;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkt::Image image(*m_device, ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_VIEW_TYPE_3D);
}

TEST_F(PositiveImage, SlicedCreateInfo) {
    TEST_DESCRIPTION("Test VkImageViewSlicedCreateInfoEXT");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_IMAGE_SLICED_VIEW_OF_3D_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imageSlicedViewOf3D);
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.extent = {32, 32, 8};
    ci.mipLevels = 6;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkt::Image image(*m_device, ci, vkt::set_layout);

    VkImageViewSlicedCreateInfoEXT sliced_info = vku::InitStructHelper();

    VkImageViewCreateInfo ivci = vku::InitStructHelper(&sliced_info);
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_3D;
    ivci.format = ci.format;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.layerCount = 1;

    auto get_effective_depth = [&]() -> uint32_t {
        return GetEffectiveExtent(ci, ivci.subresourceRange.aspectMask, ivci.subresourceRange.baseMipLevel).depth;
    };

    {
        sliced_info.sliceCount = VK_REMAINING_3D_SLICES_EXT;
        sliced_info.sliceOffset = 0;
        ivci.subresourceRange.baseMipLevel = 0;
        ASSERT_TRUE(get_effective_depth() == 8);

        vkt::ImageView image_view(*m_device, ivci);
    }

    {
        sliced_info.sliceCount = VK_REMAINING_3D_SLICES_EXT;
        sliced_info.sliceOffset = 1;
        ivci.subresourceRange.baseMipLevel = 0;
        ASSERT_TRUE(get_effective_depth() == 8);

        vkt::ImageView image_view(*m_device, ivci);
    }

    {
        sliced_info.sliceCount = 8;
        sliced_info.sliceOffset = 0;
        ivci.subresourceRange.baseMipLevel = 0;
        ASSERT_TRUE(get_effective_depth() == 8);

        vkt::ImageView image_view(*m_device, ivci);
    }

    {
        sliced_info.sliceCount = 4;
        sliced_info.sliceOffset = 4;
        ivci.subresourceRange.baseMipLevel = 0;
        ASSERT_TRUE(get_effective_depth() == 8);

        vkt::ImageView image_view(*m_device, ivci);
    }

    {
        sliced_info.sliceCount = 4;
        sliced_info.sliceOffset = 0;
        ivci.subresourceRange.baseMipLevel = 1;
        ASSERT_TRUE(get_effective_depth() == 4);

        vkt::ImageView image_view(*m_device, ivci);
    }

    {
        sliced_info.sliceCount = 2;
        sliced_info.sliceOffset = 0;
        ivci.subresourceRange.baseMipLevel = 2;
        ASSERT_TRUE(get_effective_depth() == 2);

        vkt::ImageView image_view(*m_device, ivci);
    }

    {
        sliced_info.sliceCount = VK_REMAINING_3D_SLICES_EXT;
        sliced_info.sliceOffset = 0;
        ivci.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        ivci.subresourceRange.baseMipLevel = 5;
        ASSERT_TRUE(get_effective_depth() == 1);

        vkt::ImageView image_view(*m_device, ivci);
    }

    {
        sliced_info.sliceCount = VK_REMAINING_3D_SLICES_EXT;
        sliced_info.sliceOffset = 0;
        ivci.subresourceRange.levelCount = 1;
        ivci.subresourceRange.baseMipLevel = 5;
        ASSERT_TRUE(get_effective_depth() == 1);

        vkt::ImageView image_view(*m_device, ivci);
    }
}

TEST_F(PositiveImage, BlitRemainingArrayLayers) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    VkFormat f_color = VK_FORMAT_R32_SFLOAT;  // Need features ..BLIT_SRC_BIT & ..BLIT_DST_BIT
    if (!FormatFeaturesAreSupported(Gpu(), f_color, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
        GTEST_SKIP() << "No blit feature format support";
    }

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = f_color;
    ci.extent = {64, 64, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 4;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image image(*m_device, ci, vkt::set_layout);

    VkImageBlit blitRegion = {};
    blitRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 2, VK_REMAINING_ARRAY_LAYERS};
    blitRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, VK_REMAINING_ARRAY_LAYERS};
    blitRegion.srcOffsets[0] = {0, 0, 0};
    blitRegion.srcOffsets[1] = {16, 16, 1};
    blitRegion.dstOffsets[0] = {32, 32, 0};
    blitRegion.dstOffsets[1] = {64, 64, 1};

    m_command_buffer.Begin();

    vk::CmdBlitImage(m_command_buffer.handle(), image.handle(), image.Layout(), image.handle(), image.Layout(), 1, &blitRegion,
                     VK_FILTER_NEAREST);

    blitRegion.dstSubresource.layerCount = 2;  // same as VK_REMAINING_ARRAY_LAYERS
    vk::CmdBlitImage(m_command_buffer.handle(), image.handle(), image.Layout(), image.handle(), image.Layout(), 1, &blitRegion,
                     VK_FILTER_NEAREST);
}

TEST_F(PositiveImage, BlockTexelViewCompatibleMultipleLayers) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_6_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance6);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceMaintenance6PropertiesKHR maintenance6_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(maintenance6_props);
    if (!maintenance6_props.blockTexelViewCompatibleMultipleLayers) {
        GTEST_SKIP() << "blockTexelViewCompatibleMultipleLayers is not enabled";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT | VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 4;
    image_create_info.arrayLayers = 2;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VkFormatProperties image_fmt;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), image_create_info.format, &image_fmt);
    if (!vkt::Image::IsCompatible(*m_device, image_create_info.usage, image_fmt.optimalTilingFeatures)) {
        GTEST_SKIP() << "Image usage and format not compatible on device";
    }
    vkt::Image image(*m_device, image_create_info, vkt::set_layout);

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    ivci.format = VK_FORMAT_R16G16B16A16_UNORM;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.layerCount = 2;
    vkt::ImageView view(*m_device, ivci);
}

TEST_F(PositiveImage, ImageFormatListSizeCompatibleUncompressed) {
    AddRequiredExtensions(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_BC3_UNORM_BLOCK, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    const VkFormat formats[1] = {VK_FORMAT_R32G32B32A32_UINT};
    VkImageFormatListCreateInfo format_list = vku::InitStructHelper(nullptr);
    format_list.viewFormatCount = 1;
    format_list.pViewFormats = formats;

    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_BC3_UNORM_BLOCK, VK_IMAGE_USAGE_SAMPLED_BIT);
    image_ci.pNext = &format_list;
    image_ci.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT;

    vkt::Image image(*m_device, image_ci);
}

TEST_F(PositiveImage, ImageFormatListSizeCompatibleCompressed) {
    AddRequiredExtensions(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_BC3_UNORM_BLOCK, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    const VkFormat formats[2] = {VK_FORMAT_BC3_UNORM_BLOCK, VK_FORMAT_BC2_UNORM_BLOCK};
    VkImageFormatListCreateInfo format_list = vku::InitStructHelper(nullptr);
    format_list.viewFormatCount = 2;
    format_list.pViewFormats = formats;

    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_BC3_UNORM_BLOCK, VK_IMAGE_USAGE_SAMPLED_BIT);
    image_ci.pNext = &format_list;
    image_ci.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT;

    vkt::Image image(*m_device, image_ci);
}

TEST_F(PositiveImage, ImageAlignmentControl) {
    AddRequiredExtensions(VK_MESA_IMAGE_ALIGNMENT_CONTROL_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imageAlignmentControl);
    RETURN_IF_SKIP(Init());

    const uint32_t alignment = 0x1;
    VkPhysicalDeviceImageAlignmentControlPropertiesMESA props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(props);
    if (!(props.supportedImageAlignmentMask & alignment)) {
        GTEST_SKIP() << "supportedImageAlignmentMask doesn't support testing alignment";
    }
    VkImageAlignmentControlCreateInfoMESA alignment_control = vku::InitStructHelper();
    alignment_control.maximumRequestedAlignment = alignment;

    VkImageCreateInfo image_create_info = DefaultImageInfo();
    image_create_info.pNext = &alignment_control;
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);
}

TEST_F(PositiveImage, ImageAlignmentControlZero) {
    AddRequiredExtensions(VK_MESA_IMAGE_ALIGNMENT_CONTROL_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imageAlignmentControl);
    RETURN_IF_SKIP(Init());

    VkImageAlignmentControlCreateInfoMESA alignment_control = vku::InitStructHelper();
    alignment_control.maximumRequestedAlignment = 0;  // Should ignore

    VkImageCreateInfo image_create_info = DefaultImageInfo();
    image_create_info.pNext = &alignment_control;
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);
}

TEST_F(PositiveImage, RemainingMipLevels2DViewOf3D) {
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_IMAGE_2D_VIEW_OF_3D_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT | VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT;
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.extent = {32, 32, 2};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkt::Image image(*m_device, image_ci, vkt::set_layout);

    vkt::ImageView view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D, 0, VK_REMAINING_MIP_LEVELS, 0, 1);
}

TEST_F(PositiveImage, RemainingMipLevelsBlockTexelView) {
    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT | VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 2;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VkFormatProperties image_fmt;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), image_create_info.format, &image_fmt);
    if (!vkt::Image::IsCompatible(*m_device, image_create_info.usage, image_fmt.optimalTilingFeatures)) {
        GTEST_SKIP() << "Image usage and format not compatible on device";
    }
    vkt::Image image(*m_device, image_create_info, vkt::set_layout);

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    ivci.format = VK_FORMAT_R16G16B16A16_UNORM;
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, 1};
    CreateImageViewTest(*this, &ivci);
}

TEST_F(PositiveImage, BlitMaintenance8) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::maintenance8);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkFormat format = VK_FORMAT_R32_SFLOAT;  // Need features ..BLIT_SRC_BIT & ..BLIT_DST_BIT

    if (!FormatFeaturesAreSupported(Gpu(), format, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
        GTEST_SKIP() << "No blit feature format support";
    }

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.format = format;
    ci.extent = {64, 64, 8};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image image_3d(*m_device, ci, vkt::set_layout);

    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.extent.depth = 1;
    ci.arrayLayers = 4;
    vkt::Image image_2d(*m_device, ci, vkt::set_layout);

    // src is 2D with multiple layers
    VkImageBlit blit_region = {};
    blit_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1};
    blit_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.srcOffsets[1] = {16, 16, 1};
    blit_region.dstOffsets[0] = {32, 32, 0};
    blit_region.dstOffsets[1] = {64, 64, 1};

    m_command_buffer.Begin();
    vk::CmdBlitImage(m_command_buffer.handle(), image_2d, image_2d.Layout(), image_3d, image_3d.Layout(), 1, &blit_region,
                     VK_FILTER_NEAREST);
    m_command_buffer.End();
}
