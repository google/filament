/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"

class PositiveCopyBufferImage : public VkLayerTest {};

TEST_F(PositiveCopyBufferImage, ImageRemainingLayersMaintenance5) {
    TEST_DESCRIPTION(
        "Test copying an image with VkImageSubresourceLayers.layerCount = VK_REMAINING_ARRAY_LAYERS using VK_KHR_maintenance5");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.extent = {32, 32, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 8;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;

    // Copy from a to b
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    vkt::Image image_a(*m_device, ci, vkt::set_layout);

    ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image image_b(*m_device, ci, vkt::set_layout);

    m_command_buffer.Begin();
    image_a.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    image_b.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkImageCopy copy_region{};
    copy_region.extent = ci.extent;
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.baseArrayLayer = 2;
    copy_region.srcSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
    copy_region.dstSubresource = copy_region.srcSubresource;

    vk::CmdCopyImage(m_command_buffer.handle(), image_a.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image_b.handle(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

    // layerCount can explicitly list value
    copy_region.dstSubresource.layerCount = 6;
    vk::CmdCopyImage(m_command_buffer.handle(), image_a.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image_b.handle(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, ImageTypeExtentMismatchMaintenance5) {
    TEST_DESCRIPTION("Test copying an image with extent mismatch using VK_KHR_maintenance5");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image_2D(*m_device, image_ci, vkt::set_layout);

    image_ci.imageType = VK_IMAGE_TYPE_1D;
    image_ci.extent.height = 1;
    vkt::Image image_1D(*m_device, image_ci, vkt::set_layout);

    VkImageCopy copy_region;
    copy_region.extent = {32, 1, 1};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    m_command_buffer.Begin();
    vk::CmdCopyImage(m_command_buffer.handle(), image_1D.handle(), VK_IMAGE_LAYOUT_GENERAL, image_2D.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, ImageLayerCount) {
    TEST_DESCRIPTION("Check layerCount in vkCmdCopyImage");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    m_command_buffer.Begin();

    VkImageCopy copyRegion;
    copyRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_REMAINING_ARRAY_LAYERS};
    copyRegion.srcOffset = {0, 0, 0};
    copyRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_REMAINING_ARRAY_LAYERS};
    copyRegion.dstOffset = {32, 32, 0};
    copyRegion.extent = {16, 16, 1};
    vk::CmdCopyImage(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &copyRegion);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, BufferToRemaingImageLayers) {
    TEST_DESCRIPTION("Test vkCmdCopyBufferToImage2 with VK_REMAINING_ARRAY_LAYERS");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, 32u * 32u * 4u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkBufferImageCopy2 region = vku::InitStructHelper();
    region.bufferOffset = 0u;
    region.bufferRowLength = 0u;
    region.bufferImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, VK_REMAINING_ARRAY_LAYERS};
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32u, 32u, 1u};

    VkCopyBufferToImageInfo2 copy_buffer_to_image = vku::InitStructHelper();
    copy_buffer_to_image.srcBuffer = buffer.handle();
    copy_buffer_to_image.dstImage = image.handle();
    copy_buffer_to_image.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    copy_buffer_to_image.regionCount = 1u;
    copy_buffer_to_image.pRegions = &region;

    m_command_buffer.Begin();
    vk::CmdCopyBufferToImage2KHR(m_command_buffer.handle(), &copy_buffer_to_image);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, ImageOverlappingMemory) {
    TEST_DESCRIPTION("Validate Copy Image from/to Buffer with overlapping memory");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(Init());

    auto image_ci =
        vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_LINEAR);

    VkDeviceSize buff_size = 32 * 32 * 4;
    vkt::Buffer buffer(*m_device,
                       vkt::Buffer::CreateInfo(buff_size, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT),
                       vkt::no_mem);
    auto buffer_memory_requirements = buffer.MemoryRequirements();

    vkt::Image image(*m_device, image_ci, vkt::no_mem);
    auto image_memory_requirements = image.MemoryRequirements();

    vkt::DeviceMemory mem;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = buffer_memory_requirements.size + image_memory_requirements.size;
    bool has_memtype = m_device->Physical().SetMemoryType(
        buffer_memory_requirements.memoryTypeBits & image_memory_requirements.memoryTypeBits, &alloc_info, 0);
    if (!has_memtype) {
        GTEST_SKIP() << "Failed to find a memory type for both a buffer and an image";
    }
    mem.init(*m_device, alloc_info);

    buffer.BindMemory(mem, 0);
    image.BindMemory(mem, buffer_memory_requirements.size);

    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageOffset = {0, 0, 0};
    region.bufferOffset = 0;

    region.imageExtent = {32, 32, 1};
    m_command_buffer.Begin();
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer.handle(), 1, &region);
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, ImageOverlappingMemoryCompressed) {
    TEST_DESCRIPTION("Validate Copy Image from/to Buffer with overlapping memory");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(Init());

    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), VK_FORMAT_BC3_UNORM_BLOCK, &format_properties);
    if (((format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0) ||
        ((format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0)) {
        GTEST_SKIP() << "VK_FORMAT_BC3_UNORM_BLOCK with linear tiling not supported";
    }

    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageOffset = {0, 0, 0};
    region.bufferOffset = 0;
    region.imageExtent = {32, 32, 1};

    auto image_ci =
        vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_BC3_UNORM_BLOCK,
                                      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_LINEAR);

    // 1 byte per texel
    VkDeviceSize buff_size = 32 * 32 * 1;
    vkt::Buffer buffer(*m_device,
                       vkt::Buffer::CreateInfo(buff_size, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT),
                       vkt::no_mem);
    auto buffer_memory_requirements = buffer.MemoryRequirements();

    vkt::Image image(*m_device, image_ci, vkt::no_mem);
    auto image_memory_requirements = image.MemoryRequirements();

    vkt::DeviceMemory mem;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = buffer_memory_requirements.size + image_memory_requirements.size;
    bool has_memtype = m_device->Physical().SetMemoryType(
        buffer_memory_requirements.memoryTypeBits & image_memory_requirements.memoryTypeBits, &alloc_info, 0);
    if (!has_memtype) {
        GTEST_SKIP() << "Failed to find a memory type for both a buffer and an image";
    }

    mem.init(*m_device, alloc_info);

    buffer.BindMemory(mem, 0);
    image.BindMemory(mem, buffer_memory_requirements.size);

    m_command_buffer.Begin();
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer.handle(), 1, &region);
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, UncompressedToCompressedImage) {
    TEST_DESCRIPTION("Image copies between compressed and uncompressed images");
    RETURN_IF_SKIP(Init());

    // Verify format support
    // Size-compatible (64-bit) formats. Uncompressed is 64 bits per texel, compressed is 64 bits per 4x4 block (or 4bpt).
    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R16G16B16A16_UINT, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT) ||
        !FormatFeaturesAreSupported(Gpu(), VK_FORMAT_BC1_RGBA_SRGB_BLOCK, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    // Size = 10 * 10 * 64 = 6400
    vkt::Image uncomp_10x10t_image(*m_device, 10, 10, 1, VK_FORMAT_R16G16B16A16_UINT,
                                   VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    // Size = 40 * 40 * 4  = 6400
    vkt::Image comp_10x10b_40x40t_image(*m_device, 40, 40, 1, VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
                                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    if (!uncomp_10x10t_image.initialized() || !comp_10x10b_40x40t_image.initialized()) {
        GTEST_SKIP() << "Unable to initialize surfaces";
    }

    // Both copies represent the same number of bytes. Bytes Per Texel = 1 for bc6, 16 for uncompressed
    // Copy compressed to uncompressed
    VkImageCopy copy_region = {};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    m_command_buffer.Begin();

    // Copy from uncompressed to compressed
    copy_region.extent = {10, 10, 1};  // Dimensions in (uncompressed) texels
    vk::CmdCopyImage(m_command_buffer.handle(), uncomp_10x10t_image.handle(), VK_IMAGE_LAYOUT_GENERAL,
                     comp_10x10b_40x40t_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    // The next copy swaps source and dest s.t. we need an execution barrier on for the prior source and an access barrier for
    // prior dest
    VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.image = comp_10x10b_40x40t_image.handle();
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &image_barrier);

    // And from compressed to uncompressed
    copy_region.extent = {40, 40, 1};  // Dimensions in (compressed) texels
    vk::CmdCopyImage(m_command_buffer.handle(), comp_10x10b_40x40t_image.handle(), VK_IMAGE_LAYOUT_GENERAL,
                     uncomp_10x10t_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, UncompressedToCompressedImage2) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-Docs/issues/593");
    RETURN_IF_SKIP(Init());

    // Size-compatible (64-bit) formats. Uncompressed is 64 bits per texel, compressed is 64 bits per 4x4 block (or 4bpt).
    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R16G16B16A16_UINT, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT) ||
        !FormatFeaturesAreSupported(Gpu(), VK_FORMAT_BC1_RGBA_SRGB_BLOCK, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    // Size = 3 * 3 * 64 = 576
    vkt::Image uncomp_image(*m_device, 3, 3, 1, VK_FORMAT_R16G16B16A16_UINT,
                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    // Size = 12 * 12 * 4  = 576
    vkt::Image comp_image(*m_device, 12, 12, 1, VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
                          VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    if (!uncomp_image.initialized() || !comp_image.initialized()) {
        GTEST_SKIP() << "Unable to initialize surfaces";
    }

    VkImageCopy copy_region = {};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {3, 3, 1};

    m_command_buffer.Begin();
    vk::CmdCopyImage(m_command_buffer.handle(), uncomp_image.handle(), VK_IMAGE_LAYOUT_GENERAL, comp_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, Compressed) {
    RETURN_IF_SKIP(Init());

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_BC2_UNORM_BLOCK, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT) ||
        !FormatFeaturesAreSupported(Gpu(), VK_FORMAT_BC3_UNORM_BLOCK, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    vkt::Image image_bc2(*m_device, 60, 60, 1, VK_FORMAT_BC2_UNORM_BLOCK,
                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image_bc3(*m_device, 60, 60, 1, VK_FORMAT_BC3_UNORM_BLOCK,
                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    if (!image_bc2.initialized() || !image_bc3.initialized()) {
        GTEST_SKIP() << "Unable to initialize surfaces";
    }

    VkImageCopy copy_region = {};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {16, 16, 1};

    m_command_buffer.Begin();
    vk::CmdCopyImage(m_command_buffer.handle(), image_bc2.handle(), VK_IMAGE_LAYOUT_GENERAL, image_bc3.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, ImageSubresource) {
    RETURN_IF_SKIP(Init());

    VkImageUsageFlags usage =
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 2, 5, format, usage);
    vkt::Image image(*m_device, image_ci);

    VkImageSubresourceLayers src_layer{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageSubresourceLayers dst_layer{VK_IMAGE_ASPECT_COLOR_BIT, 1, 3, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkExtent3D full_extent{128 / 2, 128 / 2, 1};  // <-- image type is 2D
    VkImageCopy region = {src_layer, zero_offset, dst_layer, zero_offset, full_extent};
    auto init_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    auto src_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    auto dst_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    auto final_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    m_command_buffer.Begin();

    auto cb = m_command_buffer.handle();

    VkImageSubresourceRange src_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkImageMemoryBarrier image_barriers[2];

    image_barriers[0] = vku::InitStructHelper();
    image_barriers[0].srcAccessMask = 0;
    image_barriers[0].dstAccessMask = 0;
    image_barriers[0].image = image.handle();
    image_barriers[0].subresourceRange = src_range;
    image_barriers[0].oldLayout = init_layout;
    image_barriers[0].newLayout = dst_layout;

    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           image_barriers);
    VkClearColorValue clear_color{};
    vk::CmdClearColorImage(cb, image.handle(), dst_layout, &clear_color, 1, &src_range);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    m_command_buffer.Begin();

    image_barriers[0].oldLayout = dst_layout;
    image_barriers[0].newLayout = src_layout;

    VkImageSubresourceRange dst_range{VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 3, 1};
    image_barriers[1] = vku::InitStructHelper();
    image_barriers[1].srcAccessMask = 0;
    image_barriers[1].dstAccessMask = 0;
    image_barriers[1].image = image.handle();
    image_barriers[1].subresourceRange = dst_range;
    image_barriers[1].oldLayout = init_layout;
    image_barriers[1].newLayout = dst_layout;

    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 2,
                           image_barriers);

    vk::CmdCopyImage(cb, image.handle(), src_layout, image.handle(), dst_layout, 1, &region);

    image_barriers[0].oldLayout = src_layout;
    image_barriers[0].newLayout = final_layout;
    image_barriers[1].oldLayout = dst_layout;
    image_barriers[1].newLayout = final_layout;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 2,
                           image_barriers);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

// Being worked on if this is valid or not
// https://gitlab.khronos.org/vulkan/vulkan/-/issues/4159
TEST_F(PositiveCopyBufferImage, DISABLED_CopyCompressed1DImage) {
    TEST_DESCRIPTION("Copy a 1D image with compressed format");
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_1D;
    image_ci.format = VK_FORMAT_R16G16B16A16_UNORM;
    image_ci.extent = {256u, 1u, 1u};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (!ImageFormatIsSupported(instance(), Gpu(), image_ci, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT)) {
        GTEST_SKIP() << "image format not supported";
    }
    vkt::Image src_image(*m_device, image_ci, vkt::set_layout);

    image_ci.format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (!ImageFormatIsSupported(instance(), Gpu(), image_ci, VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "image format not supported";
    }
    vkt::Image dst_image(*m_device, image_ci, vkt::set_layout);

    m_command_buffer.Begin();
    src_image.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    dst_image.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkImageCopy image_copy;
    image_copy.srcSubresource = {1u, 0u, 0u, 1u};
    image_copy.srcOffset = {0, 0, 0};
    image_copy.dstSubresource = {1u, 0u, 0u, 1u};
    image_copy.dstOffset = {0, 0, 0};
    image_copy.extent = {16u, 1u, 1u};

    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &image_copy);
    m_command_buffer.End();
}

// Being worked on if this is valid or not
// https://gitlab.khronos.org/vulkan/vulkan/-/issues/4159
TEST_F(PositiveCopyBufferImage, DISABLED_CopyCompressed1DToCompressed2D) {
    TEST_DESCRIPTION("Copy a 1D compressed format to a 2D compressed format");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.imageType = VK_IMAGE_TYPE_1D;
    image_ci.extent = {256u, 1u, 1u};
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (!ImageFormatIsSupported(instance(), Gpu(), image_ci, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT)) {
        GTEST_SKIP() << "image format not supported";
    }
    vkt::Image src_image(*m_device, image_ci, vkt::set_layout);

    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.extent = {32u, 32u, 1u};
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (!ImageFormatIsSupported(instance(), Gpu(), image_ci, VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "image format not supported";
    }
    vkt::Image dst_image(*m_device, image_ci, vkt::set_layout);

    m_command_buffer.Begin();
    src_image.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    dst_image.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkImageCopy image_copy;
    image_copy.srcSubresource = {1u, 0u, 0u, 1u};
    image_copy.srcOffset = {0, 0, 0};
    image_copy.dstSubresource = {1u, 0u, 0u, 1u};
    image_copy.dstOffset = {0, 0, 0};
    image_copy.extent = {32u, 4u, 1u};

    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &image_copy);
    m_command_buffer.End();
}

// Being worked on if this is valid or not
// https://gitlab.khronos.org/vulkan/vulkan/-/issues/4159
TEST_F(PositiveCopyBufferImage, DISABLED_CopyBufferTo1DCompressedImage) {
    TEST_DESCRIPTION("Copy a buffer to 1D image with compressed format");
    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_1D;
    image_ci.format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    image_ci.extent = {64u, 1u, 1u};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    if (!ImageFormatIsSupported(instance(), Gpu(), image_ci, VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "image format not supported";
    }
    vkt::Image dst_image(*m_device, image_ci, vkt::set_layout);

    VkBufferImageCopy buffer_image_copy;
    buffer_image_copy.bufferOffset = 0u;
    buffer_image_copy.bufferRowLength = 64u;
    buffer_image_copy.bufferImageHeight = 4u;
    buffer_image_copy.imageSubresource = {1u, 0u, 0u, 1u};
    buffer_image_copy.imageOffset = {0, 0, 0};
    buffer_image_copy.imageExtent = {64u, 4u, 1u};

    m_command_buffer.Begin();
    dst_image.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer, dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u,
                             &buffer_image_copy);
    m_command_buffer.End();
}

// Being worked on if this is valid or not
// https://gitlab.khronos.org/vulkan/vulkan/-/issues/4159
TEST_F(PositiveCopyBufferImage, DISABLED_CopyCompress2DTo1D) {
    TEST_DESCRIPTION("Copy a compressed 2D image to a compressed 1D image");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = 0u;
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    image_ci.extent = {64u, 64u, 1u};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (!ImageFormatIsSupported(instance(), Gpu(), image_ci, VK_IMAGE_USAGE_TRANSFER_SRC_BIT)) {
        GTEST_SKIP() << "image format not supported";
    }
    vkt::Image src_image(*m_device, image_ci, vkt::set_layout);

    image_ci.imageType = VK_IMAGE_TYPE_1D;
    image_ci.extent = {1024u, 1u, 1u};
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (!ImageFormatIsSupported(instance(), Gpu(), image_ci, VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "image format not supported";
    }
    vkt::Image dst_image(*m_device, image_ci, vkt::set_layout);

    m_command_buffer.Begin();
    src_image.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    dst_image.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkImageCopy image_copy;
    image_copy.srcSubresource = {1u, 0u, 0u, 1u};
    image_copy.srcOffset = {0, 0, 0};
    image_copy.dstSubresource = {1u, 0u, 0u, 1u};
    image_copy.dstOffset = {0, 0, 0};
    image_copy.extent = {64u, 4u, 1u};

    vk::CmdCopyImage(m_command_buffer.handle(), src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_image,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &image_copy);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, BufferCopiesStressTest) {
    TEST_DESCRIPTION("Do many buffer copies, make sure perf is good");

    RETURN_IF_SKIP(Init());

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_ci.size = 1024;
    vkt::Buffer src_buffer(*m_device, buffer_ci, vkt::no_mem);
    vkt::Buffer dst_buffer(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), src_buffer.handle(), &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    buffer_mem_alloc.allocationSize *= 2;

    vkt::DeviceMemory buffer_mem(*m_device, buffer_mem_alloc);
    src_buffer.BindMemory(buffer_mem, 0);
    dst_buffer.BindMemory(buffer_mem, 1024);

    constexpr VkDeviceSize copy_size = 1024 / 4;
    std::vector<VkBufferCopy> copy_info_list(4);
    copy_info_list[3].srcOffset = 0;
    copy_info_list[3].dstOffset = 0;
    copy_info_list[3].size = copy_size;

    copy_info_list[2].srcOffset = copy_size;
    copy_info_list[2].dstOffset = copy_size;
    copy_info_list[2].size = copy_size;

    copy_info_list[1].srcOffset = 2 * copy_size;
    copy_info_list[1].dstOffset = 2 * copy_size;
    copy_info_list[1].size = copy_size;

    copy_info_list[0].srcOffset = 3 * copy_size;
    copy_info_list[0].dstOffset = 3 * copy_size;
    copy_info_list[0].size = copy_size;

    const size_t size = 10000;
    copy_info_list.resize(copy_info_list.size() + size);
    for (size_t i = 0; i < size; ++i) {
        copy_info_list[i + 4] = copy_info_list[i % 4];
    }

    m_command_buffer.Begin();

    vk::CmdCopyBuffer(m_command_buffer, src_buffer.handle(), dst_buffer.handle(), size32(copy_info_list), copy_info_list.data());

    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, CopyColorToDepthMaintenacne8) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::maintenance8);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Add Transfer support for all used formats
    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required VK_FORMAT_R32_SFLOAT features not supported";
    } else if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                           VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required VK_FORMAT_D32_SFLOAT features not supported";
    }

    vkt::Image color_image(*m_device, 128, 128, 1, VK_FORMAT_R32_SFLOAT,
                           VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image depth_image(*m_device, 128, 128, 1, VK_FORMAT_D32_SFLOAT,
                           VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    color_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    depth_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageCopy copy_region;
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {64, 64, 1};

    m_command_buffer.Begin();
    vk::CmdCopyImage(m_command_buffer.handle(), color_image, VK_IMAGE_LAYOUT_GENERAL, depth_image, VK_IMAGE_LAYOUT_GENERAL, 1,
                     &copy_region);
    m_command_buffer.End();
}

TEST_F(PositiveCopyBufferImage, ImageBufferCopyDepthStencil) {
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Verify all needed Depth/Stencil formats are supported
    bool missing_ds_support = false;
    VkFormatProperties props = {0, 0, 0};
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), VK_FORMAT_D32_SFLOAT_S8_UINT, &props);
    missing_ds_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
    missing_ds_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
    missing_ds_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), VK_FORMAT_D24_UNORM_S8_UINT, &props);
    missing_ds_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
    missing_ds_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
    missing_ds_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), VK_FORMAT_D16_UNORM, &props);
    missing_ds_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
    missing_ds_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
    missing_ds_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), VK_FORMAT_S8_UINT, &props);
    missing_ds_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
    missing_ds_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
    missing_ds_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;

    if (missing_ds_support) {
        GTEST_SKIP() << "Depth / Stencil formats unsupported";
    }

    // 256^2 texels, 512kb (256k depth, 64k stencil, 192k pack)
    vkt::Image ds_image_4D_1S(
        *m_device, 256, 256, 1, VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    ds_image_4D_1S.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    // 256^2 texels, 256kb (192k depth, 64k stencil)
    vkt::Image ds_image_3D_1S(
        *m_device, 256, 256, 1, VK_FORMAT_D24_UNORM_S8_UINT,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    ds_image_3D_1S.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    // 256^2 texels, 128k (128k depth)
    vkt::Image ds_image_2D(
        *m_device, 256, 256, 1, VK_FORMAT_D16_UNORM,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    ds_image_2D.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    // 256^2 texels, 64k (64k stencil)
    vkt::Image ds_image_1S(
        *m_device, 256, 256, 1, VK_FORMAT_S8_UINT,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    ds_image_1S.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkBufferUsageFlags transfer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_256k(*m_device, 262144, transfer_usage);  // 256k
    vkt::Buffer buffer_128k(*m_device, 131072, transfer_usage);  // 128k
    vkt::Buffer buffer_64k(*m_device, 65536, transfer_usage);    // 64k

    VkBufferImageCopy ds_region = {};
    ds_region.bufferOffset = 0;
    ds_region.bufferRowLength = 0;
    ds_region.bufferImageHeight = 0;
    ds_region.imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    ds_region.imageOffset = {0, 0, 0};
    ds_region.imageExtent = {256, 256, 1};

    VkMemoryBarrier mem_barrier = vku::InitStructHelper();
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    m_command_buffer.Begin();
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), ds_image_4D_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             buffer_256k.handle(), 1, &ds_region);

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barrier, 0, nullptr, 0, nullptr);
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), ds_image_3D_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             buffer_256k.handle(), 1, &ds_region);

    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), ds_image_2D.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             buffer_128k.handle(), 1, &ds_region);

    // Stencil
    ds_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barrier, 0, nullptr, 0, nullptr);
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), ds_image_4D_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             buffer_64k.handle(), 1, &ds_region);

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barrier, 0, nullptr, 0, nullptr);
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), ds_image_3D_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             buffer_64k.handle(), 1, &ds_region);

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barrier, 0, nullptr, 0, nullptr);
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), ds_image_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             buffer_64k.handle(), 1, &ds_region);
}
