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

#include <gtest/gtest.h>
#include "../framework/layer_validation_tests.h"

class NegativeCopyBufferImage : public VkLayerTest {};

TEST_F(NegativeCopyBufferImage, ImageBufferCopy) {
    TEST_DESCRIPTION("Image to buffer and buffer to image tests");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Verify R8G8B8A8_UINT format is supported for transfer
    bool missing_rgba_support = false;
    VkFormatProperties props = {0, 0, 0};
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), VK_FORMAT_R8G8B8A8_UINT, &props);
    missing_rgba_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
    missing_rgba_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
    missing_rgba_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;

    if (missing_rgba_support) {
        GTEST_SKIP() << "R8G8B8A8_UINT transfer unsupported";
    }

    // 128^2 texels, 64k
    vkt::Image image_64k(*m_device, 128, 128, 1, VK_FORMAT_R8G8B8A8_UINT,
                         VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    image_64k.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    // 64^2 texels, 16k
    vkt::Image image_16k(*m_device, 64, 64, 1, VK_FORMAT_R8G8B8A8_UINT,
                         VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    image_16k.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkBufferUsageFlags transfer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_64k(*m_device, 65536, transfer_usage);  // 64k
    vkt::Buffer buffer_16k(*m_device, 16384, transfer_usage);  // 16k

    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {64, 64, 1};
    region.bufferOffset = 0;

    VkMemoryBarrier mem_barriers[3];
    mem_barriers[0] = vku::InitStructHelper();
    mem_barriers[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barriers[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barriers[1] = vku::InitStructHelper();
    mem_barriers[1].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    mem_barriers[1].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barriers[2] = vku::InitStructHelper();
    mem_barriers[2].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barriers[2].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    // attempt copies before putting command buffer in recording state
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-commandBuffer-recording");
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_64k.handle(), image_64k.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-commandBuffer-recording");
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image_64k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_64k.handle(), 1,
                             &region);
    m_errorMonitor->VerifyFound();

    m_command_buffer.Begin();

    // successful copies
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16k.handle(), 1,
                             &region);

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barriers[2], 0, nullptr, 0, nullptr);
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_16k.handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region);
    region.imageOffset.x = 16;  // 16k copy, offset requires larger image

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barriers[0], 0, nullptr, 0, nullptr);
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image_64k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16k.handle(), 1,
                             &region);
    region.imageExtent.height = 78;  // > 16k copy requires larger buffer & image

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barriers[1], 0, nullptr, 0, nullptr);
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_64k.handle(), image_64k.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region);
    region.imageOffset.x = 0;
    region.imageExtent.height = 64;
    region.bufferOffset = 256;  // 16k copy with buffer offset, requires larger buffer

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 2,
                           &mem_barriers[1], 0, nullptr, 0, nullptr);
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_64k.handle(), 1,
                             &region);

    // image/buffer too small (extent too large) on copy to image
    region.imageExtent = {65, 64, 1};
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-pRegions-00171");  // buffer too small
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_16k.handle(), image_64k.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyBufferToImage-imageSubresource-07971");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-imageSubresource-07970");  // image too small
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_64k.handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();

    // image/buffer too small (offset) on copy to image
    region.imageExtent = {64, 64, 1};
    region.imageOffset = {0, 4, 0};
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-pRegions-00171");  // buffer too small
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_16k.handle(), image_64k.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyBufferToImage-imageSubresource-07971");
    m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyBufferToImage-imageSubresource-07972");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-imageSubresource-07970");  // image too small
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_64k.handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();

    // image/buffer too small on copy to buffer
    region.imageExtent = {64, 64, 1};
    region.imageOffset = {0, 0, 0};
    region.bufferOffset = 4;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-pRegions-00183");  // buffer too small
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image_64k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16k.handle(), 1,
                             &region);
    m_errorMonitor->VerifyFound();

    region.imageExtent = {64, 65, 1};
    region.bufferOffset = 0;
    m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyImageToBuffer-imageSubresource-07972");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-imageSubresource-07970");  // image too small
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_64k.handle(), 1,
                             &region);
    m_errorMonitor->VerifyFound();

    // buffer size OK but rowlength causes loose packing
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-pRegions-00183");
    region.imageExtent = {64, 64, 1};
    region.bufferRowLength = 68;
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16k.handle(), 1,
                             &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkBufferImageCopy-imageExtent-06659");
    region.imageExtent.width = 0;
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_64k.handle(), 1,
                             &region);
    m_errorMonitor->VerifyFound();

    region.imageExtent = {64, 64, 1};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-imageSubresource-09105");  // mis-matched aspect
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16k.handle(), 1,
                             &region);
    m_errorMonitor->VerifyFound();
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // Out-of-range mip levels should fail
    region.imageSubresource.mipLevel = image_16k.CreateInfo().mipLevels + 1;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-imageSubresource-07967");
    m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyImageToBuffer-imageSubresource-07971");
    m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyImageToBuffer-imageSubresource-07972");
    m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyImageToBuffer-imageOffset-09104");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-imageSubresource-07970");  // unavoidable "region exceeds image
                                                                                            // bounds" for non-existent mip
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16k.handle(), 1,
                             &region);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-imageSubresource-07967");
    m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyBufferToImage-imageSubresource-07971");
    m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyBufferToImage-imageSubresource-07972");
    m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyBufferToImage-imageOffset-09104");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-imageSubresource-07970");  // unavoidable "region exceeds image
                                                                                            // bounds" for non-existent mip
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_16k.handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();
    region.imageSubresource.mipLevel = 0;

    // Out-of-range array layers should fail
    region.imageSubresource.baseArrayLayer = image_16k.CreateInfo().arrayLayers;
    region.imageSubresource.layerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-imageSubresource-07968");
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16k.handle(), 1,
                             &region);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-imageSubresource-07968");
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_16k.handle(), image_16k.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();
    region.imageSubresource.baseArrayLayer = 0;

    // Layout mismatch should fail
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-srcImageLayout-00189");
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image_16k.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             buffer_16k.handle(), 1, &region);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-dstImageLayout-00180");
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_16k.handle(), image_16k.handle(),
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, ImageBufferCopyDepthStencil) {
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
    vkt::Buffer buffer_16k(*m_device, 16384, transfer_usage);    // 16k

    VkBufferImageCopy ds_region = {};
    ds_region.bufferOffset = 0;
    ds_region.bufferRowLength = 0;
    ds_region.bufferImageHeight = 0;
    ds_region.imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    ds_region.imageOffset = {0, 0, 0};
    ds_region.imageExtent = {256, 256, 1};

    m_command_buffer.Begin();

    ds_region.bufferOffset = 4;
    m_errorMonitor->SetDesiredError(
        "VUID-vkCmdCopyImageToBuffer-pRegions-00183");  // Extract 4b depth per texel, pack into 256k buffer
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), ds_image_4D_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             buffer_256k.handle(), 1, &ds_region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError(
        "VUID-vkCmdCopyImageToBuffer-pRegions-00183");  // Extract 3b depth per texel, pack (loose) into 128k buffer
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), ds_image_3D_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             buffer_128k.handle(), 1, &ds_region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-pRegions-00183");  // Copy 2b depth per texel, into 128k buffer
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), ds_image_2D.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             buffer_128k.handle(), 1, &ds_region);
    m_errorMonitor->VerifyFound();

    ds_region.bufferOffset = 5;
    ds_region.imageExtent = {64, 64, 1};  // need smaller so offset works
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-srcImage-07978");
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), ds_image_2D.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             buffer_128k.handle(), 1, &ds_region);
    m_errorMonitor->VerifyFound();

    ds_region.imageExtent = {256, 256, 1};
    ds_region.bufferOffset = 0;
    ds_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;

    m_errorMonitor->SetDesiredError(
        "VUID-vkCmdCopyImageToBuffer-pRegions-00183");  // Extract 1b stencil per texel, pack into 64k buffer
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), ds_image_4D_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             buffer_16k.handle(), 1, &ds_region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError(
        "VUID-vkCmdCopyImageToBuffer-pRegions-00183");  // Extract 1b stencil per texel, pack into 64k buffer
    ds_region.bufferRowLength = 260;
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), ds_image_3D_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             buffer_64k.handle(), 1, &ds_region);
    m_errorMonitor->VerifyFound();

    ds_region.bufferRowLength = 0;
    ds_region.bufferOffset = 4;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-pRegions-00183");  // Copy 1b depth per texel, into 64k buffer
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), ds_image_1S.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             buffer_64k.handle(), 1, &ds_region);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, ImageBufferCopyDepthStencil2) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(Init());

    bool missing_ds_support = false;
    VkFormatProperties props = {0, 0, 0};
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), VK_FORMAT_D32_SFLOAT_S8_UINT, &props);
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

    vkt::Buffer buffer_256k(*m_device, 262144, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferImageCopy2 ds_region = vku::InitStructHelper();
    ds_region.bufferOffset = 4;  // Extract 4b depth per texel, pack into 256k buffer// bad
    ds_region.bufferRowLength = 0;
    ds_region.bufferImageHeight = 0;
    ds_region.imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    ds_region.imageOffset = {0, 0, 0};
    ds_region.imageExtent = {256, 256, 1};

    VkCopyImageToBufferInfo2 image_buffer_info = vku::InitStructHelper();
    image_buffer_info.dstBuffer = buffer_256k;
    image_buffer_info.srcImage = ds_image_4D_1S;
    image_buffer_info.srcImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_buffer_info.regionCount = 1;
    image_buffer_info.pRegions = &ds_region;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToBufferInfo2-pRegions-00183");
    vk::CmdCopyImageToBuffer2(m_command_buffer.handle(), &image_buffer_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCopyBufferImage, ImageBufferCopyCompression) {
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceFeatures device_features = {};
    GetPhysicalDeviceFeatures(&device_features);
    if (!(device_features.textureCompressionBC || device_features.textureCompressionETC2 ||
          device_features.textureCompressionASTC_LDR)) {
        GTEST_SKIP() << "No compressed formats supported - block compression tests";
    }
    // Verify transfer support for each compression format used blow
    bool missing_bc_support = false;
    bool missing_etc_support = false;
    bool missing_astc_support = false;

    VkFormatProperties props = {0, 0, 0};
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), VK_FORMAT_BC3_SRGB_BLOCK, &props);
    missing_bc_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
    missing_bc_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
    missing_bc_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;

    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK, &props);
    missing_etc_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
    missing_etc_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
    missing_etc_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;

    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), VK_FORMAT_ASTC_4x4_UNORM_BLOCK, &props);
    missing_astc_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
    missing_astc_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
    missing_astc_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;

    vkt::Image image_16k_4x4comp;   // 128^2 texels as 32^2 compressed (4x4) blocks, 16k
    vkt::Image image_NPOT_4x4comp;  // 130^2 texels as 33^2 compressed (4x4) blocks

    if (device_features.textureCompressionBC && (!missing_bc_support)) {
        image_16k_4x4comp.Init(*m_device, 128, 128, 1, VK_FORMAT_BC3_SRGB_BLOCK, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        image_16k_4x4comp.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
        image_NPOT_4x4comp.Init(*m_device, 130, 130, 1, VK_FORMAT_BC3_SRGB_BLOCK, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        image_NPOT_4x4comp.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    } else if (device_features.textureCompressionETC2 && (!missing_etc_support)) {
        image_16k_4x4comp.Init(*m_device, 128, 128, 1, VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        image_16k_4x4comp.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
        image_NPOT_4x4comp.Init(*m_device, 130, 130, 1, VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        image_NPOT_4x4comp.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    } else if (device_features.textureCompressionASTC_LDR && (!missing_astc_support)) {
        image_16k_4x4comp.Init(*m_device, 128, 128, 1, VK_FORMAT_ASTC_4x4_UNORM_BLOCK, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        image_16k_4x4comp.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
        image_NPOT_4x4comp.Init(*m_device, 130, 130, 1, VK_FORMAT_ASTC_4x4_UNORM_BLOCK, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        image_NPOT_4x4comp.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    } else {
        GTEST_SKIP() << "No compressed formats transfers bits are supported - block compression tests";
    }
    ASSERT_TRUE(image_16k_4x4comp.initialized());

    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {64, 64, 1};
    region.bufferOffset = 0;

    VkMemoryBarrier mem_barrier = vku::InitStructHelper();
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    VkBufferUsageFlags transfer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_64k(*m_device, 65536, transfer_usage);  // 64k
    vkt::Buffer buffer_16k(*m_device, 16384, transfer_usage);  // 16k

    m_command_buffer.Begin();

    // Just fits
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barrier, 0, nullptr, 0, nullptr);
    region.imageExtent = {128, 128, 1};
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image_16k_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16k.handle(), 1,
                             &region);

    // with offset, too big for buffer
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-pRegions-00183");
    region.bufferOffset = 16;
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image_16k_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16k.handle(), 1,
                             &region);
    m_errorMonitor->VerifyFound();
    region.bufferOffset = 0;

    // extents that are not a multiple of compressed block size
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-srcImage-00207");     // extent width not a multiple of block size
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-imageOffset-07747");  // image transfer granularity
    region.imageExtent.width = 66;
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image_NPOT_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16k.handle(),
                             1, &region);
    m_errorMonitor->VerifyFound();
    region.imageExtent.width = 128;

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-srcImage-00208");     // extent height not a multiple of block size
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-imageOffset-07747");  // image transfer granularity
    region.imageExtent.height = 2;
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image_NPOT_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16k.handle(),
                             1, &region);
    m_errorMonitor->VerifyFound();
    region.imageExtent.height = 128;

    // TODO: All available compressed formats are 2D, with block depth of 1. Unable to provoke VU_01277.

    // non-multiple extents are allowed if at the far edge of a non-block-multiple image - these should pass
    region.imageExtent.width = 66;
    region.imageOffset.x = 64;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barrier, 0, nullptr, 0, nullptr);
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image_NPOT_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16k.handle(),
                             1, &region);
    region.imageExtent.width = 16;
    region.imageOffset.x = 0;
    region.imageExtent.height = 2;
    region.imageOffset.y = 128;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barrier, 0, nullptr, 0, nullptr);
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image_NPOT_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16k.handle(),
                             1, &region);
    region.imageOffset = {0, 0, 0};

    // buffer offset must be a multiple of texel block size (16)
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-srcImage-07975");
    region.imageExtent = {64, 64, 1};
    region.bufferOffset = 24;
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image_16k_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16k.handle(), 1,
                             &region);
    m_errorMonitor->VerifyFound();

    // rowlength not a multiple of block width (4)
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-bufferRowLength-09106");
    region.bufferOffset = 0;
    region.bufferRowLength = 130;
    region.bufferImageHeight = 0;
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image_16k_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_64k.handle(), 1,
                             &region);
    m_errorMonitor->VerifyFound();

    // imageheight not a multiple of block height (4)
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-bufferImageHeight-09107");
    region.bufferRowLength = 0;
    region.bufferImageHeight = 130;
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image_16k_4x4comp.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_64k.handle(), 1,
                             &region);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, ImageBufferCopyCompression2) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::textureCompressionBC);
    RETURN_IF_SKIP(Init());

    bool missing_bc_support = false;
    VkFormatProperties props = {0, 0, 0};
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), VK_FORMAT_BC3_SRGB_BLOCK, &props);
    missing_bc_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
    missing_bc_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
    missing_bc_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;
    if (missing_bc_support) {
        GTEST_SKIP() << "Format not supported";
    }

    vkt::Buffer buffer_16k(*m_device, 16384, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    // 128^2 texels as 32^2 compressed (4x4) blocks, 16k
    vkt::Image image_16k_4x4comp(*m_device, 128, 128, 1, VK_FORMAT_BC3_SRGB_BLOCK, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    image_16k_4x4comp.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkBufferImageCopy2 region = vku::InitStructHelper();
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {64, 64, 1};
    // buffer offset must be a multiple of texel block size (16)
    region.bufferOffset = 24;

    m_command_buffer.Begin();

    VkCopyImageToBufferInfo2 image_buffer_info = vku::InitStructHelper();
    image_buffer_info.dstBuffer = buffer_16k;
    image_buffer_info.srcImage = image_16k_4x4comp;
    image_buffer_info.srcImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_buffer_info.regionCount = 1;
    image_buffer_info.pRegions = &region;

    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToBufferInfo2-srcImage-07975");
    vk::CmdCopyImageToBuffer2(m_command_buffer.handle(), &image_buffer_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCopyBufferImage, ImageBufferCopyMultiPlanar) {
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Try to use G8_B8R8_2PLANE_420_UNORM because need 2-plane format for some tests and likely supported due to copy support
    // being required with samplerYcbcrConversion feature
    bool missing_mp_support = false;
    VkFormatProperties props = {0, 0, 0};
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, &props);
    missing_mp_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
    missing_mp_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
    missing_mp_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;
    if (missing_mp_support) {
        GTEST_SKIP() << "VK_FORMAT_G8_B8R8_2PLANE_420_UNORM transfer not supported";
    }

    VkBufferImageCopy mp_region = {};
    mp_region.bufferOffset = 0;
    mp_region.bufferRowLength = 0;
    mp_region.bufferImageHeight = 0;
    mp_region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    mp_region.imageOffset = {0, 0, 0};
    mp_region.imageExtent = {128, 128, 1};

    vkt::Buffer buffer_16k(*m_device, 16384, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    // YUV420 means 1/2 width and height so plane_0 is 128x128 and plane_1 is 64x64 here
    // 128^2 texels in plane_0 and 64^2 texels in plane_1
    vkt::Image image_multi_planar(*m_device, 128, 128, 1, VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    image_multi_planar.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkMemoryBarrier mem_barrier = vku::InitStructHelper();
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    m_command_buffer.Begin();

    // Copies into a mutli-planar image aspect properly
    mp_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barrier, 0, nullptr, 0, nullptr);
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_16k.handle(), image_multi_planar.handle(), VK_IMAGE_LAYOUT_GENERAL,
                             1, &mp_region);

    // uses plane_2 without being 3 planar format
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-dstImage-07981");
    mp_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT;
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_16k.handle(), image_multi_planar.handle(), VK_IMAGE_LAYOUT_GENERAL,
                             1, &mp_region);
    m_errorMonitor->VerifyFound();

    // uses single-plane aspect mask
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-dstImage-07981");
    mp_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_16k.handle(), image_multi_planar.handle(), VK_IMAGE_LAYOUT_GENERAL,
                             1, &mp_region);
    m_errorMonitor->VerifyFound();

    // buffer offset must be a multiple of texel block size for VK_FORMAT_R8G8_UNORM (2)
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-dstImage-07976");
    mp_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
    mp_region.bufferOffset = 5;
    mp_region.imageExtent = {8, 8, 1};
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_16k.handle(), image_multi_planar.handle(), VK_IMAGE_LAYOUT_GENERAL,
                             1, &mp_region);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, ImageLayerCountMismatch) {
    TEST_DESCRIPTION(
        "Try to copy between images with the source subresource having a different layerCount than the destination subresource");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddOptionalExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const bool maintenance1 =
        IsExtensionsEnabled(VK_KHR_MAINTENANCE_1_EXTENSION_NAME) || DeviceValidationVersion() >= VK_API_VERSION_1_1;

    VkFormat image_format = VK_FORMAT_B8G8R8A8_UNORM;
    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), image_format, &format_props);
    if ((format_props.optimalTilingFeatures & (VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) == 0) {
        GTEST_SKIP() << "Transfer for format is not supported";
    }

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 4, image_format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    vkt::Image src_image(*m_device, image_ci, vkt::set_layout);

    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image dst_image(*m_device, image_ci, vkt::set_layout);

    m_command_buffer.Begin();
    VkImageCopy copy_region;
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    // Introduce failure by forcing the dst layerCount to differ from src
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 3};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {1, 1, 1};

    const char *vuid = (maintenance1 == true) ? "VUID-vkCmdCopyImage-srcImage-08793" : "VUID-VkImageCopy-apiVersion-07941";
    m_errorMonitor->SetDesiredError(vuid);
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCopyBufferImage, CompressedImageMip) {
    TEST_DESCRIPTION("Image/Buffer copies for higher mip levels");

    AddOptionalExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::textureCompressionBC);
    RETURN_IF_SKIP(Init());
    bool copy_commands2 = IsExtensionsEnabled(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 6, 1, VK_FORMAT_BC3_SRGB_BLOCK,
                                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);

    image_ci.extent = {31, 32, 1};  // Mips are [31,32] [15,16] [7,8] [3,4], [1,2] [1,1]
    vkt::Image odd_image(*m_device, image_ci, vkt::set_layout);

    VkBufferUsageFlags transfer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_1024(*m_device, 1024, transfer_usage);
    vkt::Buffer buffer_64(*m_device, 64, transfer_usage);
    vkt::Buffer buffer_16(*m_device, 16, transfer_usage);
    vkt::Buffer buffer_8(*m_device, 8, transfer_usage);

    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageOffset = {0, 0, 0};
    region.bufferOffset = 0;

    m_command_buffer.Begin();

    VkMemoryBarrier mem_barriers[3];
    mem_barriers[0] = vku::InitStructHelper();
    mem_barriers[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barriers[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barriers[1] = vku::InitStructHelper();
    mem_barriers[1].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    mem_barriers[1].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barriers[2] = vku::InitStructHelper();
    mem_barriers[2].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barriers[2].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    // Mip level copies that work - 5 levels

    // Mip 0 should fit in 1k buffer - 1k texels @ 1b each
    region.imageExtent = {32, 32, 1};
    region.imageSubresource.mipLevel = 0;
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_1024.handle(), 1, &region);

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barriers[2], 0, nullptr, 0, nullptr);
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_1024.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);

    // Mip 2 should fit in 64b buffer - 64 texels @ 1b each
    region.imageExtent = {8, 8, 1};
    region.imageSubresource.mipLevel = 2;
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_64.handle(), 1, &region);

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 2,
                           &mem_barriers[1], 0, nullptr, 0, nullptr);
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_64.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);

    // Mip 3 should fit in 16b buffer - 16 texels @ 1b each
    region.imageExtent = {4, 4, 1};
    region.imageSubresource.mipLevel = 3;
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16.handle(), 1, &region);

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 2,
                           &mem_barriers[1], 0, nullptr, 0, nullptr);
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_16.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);

    // Mip 4&5 should fit in 16b buffer with no complaint - 4 & 1 texels @ 1b each
    region.imageExtent = {2, 2, 1};
    region.imageSubresource.mipLevel = 4;

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barriers[0], 0, nullptr, 0, nullptr);
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16.handle(), 1, &region);

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 2,
                           &mem_barriers[1], 0, nullptr, 0, nullptr);
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_16.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);

    region.imageExtent = {1, 1, 1};
    region.imageSubresource.mipLevel = 5;

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barriers[0], 0, nullptr, 0, nullptr);
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16.handle(), 1, &region);

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 2,
                           &mem_barriers[1], 0, nullptr, 0, nullptr);
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_16.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);

    // Buffer must accommodate a full compressed block, regardless of texel count
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-pRegions-00183");
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_8.handle(), 1, &region);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-pRegions-00171");
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_8.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    // Copy width < compressed block size, but not the full mip width
    region.imageExtent = {1, 2, 1};
    region.imageSubresource.mipLevel = 4;
    // width not a multiple of compressed block width
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-srcImage-00207");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-imageOffset-07747");  // image transfer granularity
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16.handle(), 1, &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError(
        "VUID-vkCmdCopyBufferToImage-dstImage-00207");  // width not a multiple of compressed block width
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-imageOffset-07738");  // image transfer granularity
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_16.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    // Copy height < compressed block size but not the full mip height
    region.imageExtent = {2, 1, 1};
    m_errorMonitor->SetDesiredError(
        "VUID-vkCmdCopyImageToBuffer-srcImage-00208");  // height not a multiple of compressed block width
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-imageOffset-07747");  // image transfer granularity
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16.handle(), 1, &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError(
        "VUID-vkCmdCopyBufferToImage-dstImage-00208");  // height not a multiple of compressed block width
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-imageOffset-07738");  // image transfer granularity
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_16.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    // Offsets must be multiple of compressed block size
    region.imageOffset = {1, 1, 0};
    region.imageExtent = {1, 1, 1};
    // imageOffset not a multiple of block size
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-srcImage-07274");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-srcImage-07275");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-imageOffset-07747");  // image transfer granularity
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16.handle(), 1, &region);
    m_errorMonitor->VerifyFound();

    // imageOffset not a multiple of block size
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-dstImage-07274");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-dstImage-07275");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-imageOffset-07738");  // image transfer granularity
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_16.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    // Equivalent test using KHR_copy_commands2
    if (copy_commands2) {
        const VkBufferImageCopy2 region2 = {VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
                                            NULL,
                                            region.bufferOffset,
                                            region.bufferRowLength,
                                            region.bufferImageHeight,
                                            region.imageSubresource,
                                            region.imageOffset,
                                            region.imageExtent};
        const VkCopyBufferToImageInfo2 copy_buffer_to_image_info2 = {VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
                                                                     NULL,
                                                                     buffer_16.handle(),
                                                                     image.handle(),
                                                                     VK_IMAGE_LAYOUT_GENERAL,
                                                                     1,
                                                                     &region2};
        // imageOffset not a multiple of block size
        m_errorMonitor->SetDesiredError("VUID-VkCopyBufferToImageInfo2-dstImage-07274");
        m_errorMonitor->SetDesiredError("VUID-VkCopyBufferToImageInfo2-dstImage-07275");
        m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage2-imageOffset-07738");  // image transfer granularity
        vk::CmdCopyBufferToImage2KHR(m_command_buffer.handle(), &copy_buffer_to_image_info2);
        m_errorMonitor->VerifyFound();
    }

    // Offset + extent width = mip width - should succeed
    region.imageOffset = {4, 4, 0};
    region.imageExtent = {3, 4, 1};
    region.imageSubresource.mipLevel = 2;

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barriers[0], 0, nullptr, 0, nullptr);
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), odd_image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16.handle(), 1,
                             &region);

    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 2,
                           &mem_barriers[1], 0, nullptr, 0, nullptr);
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_16.handle(), odd_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region);

    // Offset + extent width < mip width and not a multiple of block width - should fail
    region.imageExtent = {3, 3, 1};
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-srcImage-00208");  // offset+extent not a multiple of block width
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-imageOffset-07747");  // image transfer granularity
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), odd_image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_16.handle(), 1,
                             &region);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-dstImage-00208");  // offset+extent not a multiple of block width
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-imageOffset-07738");  // image transfer granularity
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer_16.handle(), odd_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCopyBufferImage, Compressed) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-Docs/issues/1005");
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
    copy_region.extent = {15, 16, 1};

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01728");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstImage-01732");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcOffset-01783");  // image transfer granularity
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstOffset-01784");  // image transfer granularity
    m_command_buffer.Begin();
    vk::CmdCopyImage(m_command_buffer.handle(), image_bc2.handle(), VK_IMAGE_LAYOUT_GENERAL, image_bc3.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.End();
    m_errorMonitor->VerifyFound();
}

// issue being resolved in https://gitlab.khronos.org/vulkan/vulkan/-/issues/1762
TEST_F(NegativeCopyBufferImage, CompressedMipLevels) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-Docs/issues/1005");
    RETURN_IF_SKIP(Init());

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_BC2_UNORM_BLOCK, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    // Mip 0 - 60 x 60
    // Mip 1 - 30 x 30
    // Mip 2 - 15 x 15
    vkt::Image image_src(*m_device, 60, 60, 3, VK_FORMAT_BC2_UNORM_BLOCK,
                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image_dst(*m_device, 60, 60, 3, VK_FORMAT_BC3_UNORM_BLOCK,
                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    if (!image_src.initialized() || !image_dst.initialized()) {
        GTEST_SKIP() << "Unable to initialize surfaces";
    }

    VkImageCopy copy_region = {};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 2, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    m_command_buffer.Begin();

    copy_region.extent = {16, 16, 1};
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstOffset-00150");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstOffset-00151");
    vk::CmdCopyImage(m_command_buffer.handle(), image_src.handle(), VK_IMAGE_LAYOUT_GENERAL, image_dst.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    copy_region.extent = {15, 15, 1};
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01728");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01729");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcOffset-01783");
    vk::CmdCopyImage(m_command_buffer.handle(), image_src.handle(), VK_IMAGE_LAYOUT_GENERAL, image_dst.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, MiscImageLayer) {
    TEST_DESCRIPTION("Image-related tests that don't belong elsewhere");

    RETURN_IF_SKIP(Init());

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R16G16B16A16_UINT, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required VK_FORMAT_R16G16B16A16_UINT features not supported";
    } else if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R8G8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                           VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required VK_FORMAT_R8G8_UNORM features not supported";
    }

    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_R16G16B16A16_UINT, VK_IMAGE_USAGE_TRANSFER_DST_BIT);  // 64bpp
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::Buffer buffer(*m_device, 128 * 128 * 8, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0);
    VkBufferImageCopy region = {};
    region.bufferRowLength = 128;
    region.bufferImageHeight = 128;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    // layerCount can't be 0 - Expect MISMATCHED_IMAGE_ASPECT
    region.imageSubresource.layerCount = 1;
    region.imageExtent = {4, 4, 1};

    vkt::Image image2(*m_device, 128, 128, 1, VK_FORMAT_R8G8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);  // 16bpp
    image2.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::Buffer buffer2(*m_device, 128 * 128 * 2, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0);
    m_command_buffer.Begin();

    // Image must have offset.z of 0 and extent.depth of 1
    // Introduce failure by setting imageExtent.depth to 0
    region.imageExtent.depth = 0;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-dstImage-07980");
    m_errorMonitor->SetDesiredError("VUID-VkBufferImageCopy-imageExtent-06661");
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();

    region.imageExtent.depth = 1;

    // Image must have offset.z of 0 and extent.depth of 1
    // Introduce failure by setting imageOffset.z to 4
    // Note: Also (unavoidably) triggers 'region exceeds image' #1228
    region.imageOffset.z = 4;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-dstImage-07980");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-imageOffset-09104");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-imageSubresource-07970");
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();

    region.imageOffset.z = 0;
    // BufferOffset must be a multiple of the calling command's VkImage parameter's texel size
    // Introduce failure by setting bufferOffset to 1 and 1/2 texels
    region.bufferOffset = 4;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-dstImage-07975");
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();

    // BufferRowLength must be 0, or greater than or equal to the width member of imageExtent
    region.bufferOffset = 0;
    region.imageExtent.height = 128;
    region.imageExtent.width = 128;
    // Introduce failure by setting bufferRowLength > 0 but less than width
    region.bufferRowLength = 64;
    m_errorMonitor->SetDesiredError("VUID-VkBufferImageCopy-bufferRowLength-09101");
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();

    // BufferImageHeight must be 0, or greater than or equal to the height member of imageExtent
    region.bufferRowLength = 128;
    // Introduce failure by setting bufferRowHeight > 0 but less than height
    region.bufferImageHeight = 64;
    m_errorMonitor->SetDesiredError("VUID-VkBufferImageCopy-bufferImageHeight-09102");
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCopyBufferImage, ImageTypeExtentMismatch) {
    TEST_DESCRIPTION("Image copy tests where format type and extents don't match");
    RETURN_IF_SKIP(Init());

    // Tests are designed to run without Maintenance1 which was promoted in 1.1
    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Tests for 1.0 only";
    }

    // Create 1D image
    VkImageCreateInfo ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
                                                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image_2D(*m_device, ci, vkt::set_layout);

    ci.imageType = VK_IMAGE_TYPE_1D;
    ci.extent.height = 1;
    vkt::Image image_1D(*m_device, ci, vkt::set_layout);

    m_command_buffer.Begin();

    VkImageCopy copy_region;
    copy_region.extent = {32, 1, 1};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    // 1D texture w/ offset.y > 0. Source = VU 09c00124, dest = 09c00130
    copy_region.srcOffset.y = 1;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-00146");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcOffset-00145");   // also y-dim overrun
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-apiVersion-07933");  // not same image type
    vk::CmdCopyImage(m_command_buffer.handle(), image_1D.handle(), VK_IMAGE_LAYOUT_GENERAL, image_2D.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    copy_region.srcOffset.y = 0;
    copy_region.dstOffset.y = 1;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstImage-00152");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstOffset-00151");   // also y-dim overrun
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-apiVersion-07933");  // not same image type
    vk::CmdCopyImage(m_command_buffer.handle(), image_2D.handle(), VK_IMAGE_LAYOUT_GENERAL, image_1D.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    copy_region.dstOffset.y = 0;

    // 1D texture w/ extent.height > 1. Source = VU 09c00124, dest = 09c00130
    copy_region.extent.height = 2;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-00146");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcOffset-00145");   // also y-dim overrun
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-apiVersion-07933");  // not same image type
    vk::CmdCopyImage(m_command_buffer.handle(), image_1D.handle(), VK_IMAGE_LAYOUT_GENERAL, image_2D.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstImage-00152");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstOffset-00151");   // also y-dim overrun
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-apiVersion-07933");  // not same image type
    vk::CmdCopyImage(m_command_buffer.handle(), image_2D.handle(), VK_IMAGE_LAYOUT_GENERAL, image_1D.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    copy_region.extent.height = 1;

    // 1D texture w/ offset.z > 0. Source = VU 09c00df2, dest = 09c00df4
    copy_region.srcOffset.z = 1;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01785");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcOffset-00147");   // also z-dim overrun
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-apiVersion-07933");  // not same image type
    vk::CmdCopyImage(m_command_buffer.handle(), image_1D.handle(), VK_IMAGE_LAYOUT_GENERAL, image_2D.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.srcOffset.z = 0;
    copy_region.dstOffset.z = 1;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstImage-01786");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstOffset-00153");   // also z-dim overrun
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-apiVersion-07933");  // not same image type
    vk::CmdCopyImage(m_command_buffer.handle(), image_2D.handle(), VK_IMAGE_LAYOUT_GENERAL, image_1D.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.dstOffset.z = 0;

    // 1D texture w/ extent.depth > 1. Source = VU 09c00df2, dest = 09c00df4
    copy_region.extent.depth = 2;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01785");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcOffset-00147");   // also z-dim overrun (src)
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstOffset-00153");   // also z-dim overrun (dst)
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-apiVersion-08969");  // 2D needs to be 1 pre-Vulkan 1.1
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-apiVersion-07933");  // not same image type
    vk::CmdCopyImage(m_command_buffer.handle(), image_1D.handle(), VK_IMAGE_LAYOUT_GENERAL, image_2D.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstImage-01786");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcOffset-00147");   // also z-dim overrun (src)
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstOffset-00153");   // also z-dim overrun (dst)
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-apiVersion-08969");  // 2D needs to be 1 pre-Vulkan 1.1
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-apiVersion-07933");  // not same image type
    vk::CmdCopyImage(m_command_buffer.handle(), image_2D.handle(), VK_IMAGE_LAYOUT_GENERAL, image_1D.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, ImageTypeExtentMismatch3D) {
    TEST_DESCRIPTION("Image copy tests where format type and extents don't match");
    RETURN_IF_SKIP(Init());

    // Tests are designed to run without Maintenance1 which was promoted in 1.1
    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Tests for 1.0 only";
    }

    // Create 1D image
    VkImageCreateInfo ci = vkt::Image::ImageCreateInfo2D(32, 1, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
                                                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    ci.extent = {32, 32, 1};
    vkt::Image image_2D(*m_device, ci, vkt::set_layout);

    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.extent.depth = 8;
    vkt::Image image_3D(*m_device, ci, vkt::set_layout);

    m_command_buffer.Begin();

    VkImageCopy copy_region;
    copy_region.extent = {32, 1, 1};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    // 2D texture w/ offset.z > 0. Source = VU 09c00df6, dest = 09c00df8
    copy_region.extent = {16, 16, 1};
    copy_region.srcOffset.z = 4;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01787");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcOffset-00147");   // also z-dim overrun (src)
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-apiVersion-07933");  // not same image type
    vk::CmdCopyImage(m_command_buffer.handle(), image_2D.handle(), VK_IMAGE_LAYOUT_GENERAL, image_3D.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.srcOffset.z = 0;
    copy_region.dstOffset.z = 1;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstImage-01788");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstOffset-00153");   // also z-dim overrun (dst)
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-apiVersion-07933");  // not same image type
    vk::CmdCopyImage(m_command_buffer.handle(), image_3D.handle(), VK_IMAGE_LAYOUT_GENERAL, image_2D.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.dstOffset.z = 0;

    // 3D texture accessing an array layer other than 0. VU 09c0011a
    copy_region.extent = {4, 4, 1};
    copy_region.srcSubresource.baseArrayLayer = 1;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-apiVersion-07932");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcSubresource-07968");  // also 'too many layers'
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-apiVersion-07933");      // not same image type
    vk::CmdCopyImage(m_command_buffer.handle(), image_3D.handle(), VK_IMAGE_LAYOUT_GENERAL, image_2D.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.srcSubresource.baseArrayLayer = 0;

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, ImageTypeExtentMismatchCopyCommands2) {
    TEST_DESCRIPTION("Image copy tests where format type and extents don't match");
    AddRequiredExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Tests are designed to run without Maintenance1 which was promoted in 1.1
    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Tests for 1.0 only";
    }

    VkImageCreateInfo ci = vkt::Image::ImageCreateInfo2D(32, 1, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
                                                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    ci.imageType = VK_IMAGE_TYPE_1D;
    vkt::Image image_1D(*m_device, ci, vkt::set_layout);

    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.extent = {32, 32, 1};
    vkt::Image image_2D(*m_device, ci, vkt::set_layout);

    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.extent = {32, 32, 8};
    vkt::Image image_3D(*m_device, ci, vkt::set_layout);

    m_command_buffer.Begin();

    VkImageCopy2 region2 = vku::InitStructHelper();
    region2.extent = {32, 1, 1};
    region2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region2.srcOffset = {0, 0, 0};
    region2.dstOffset = {0, 0, 0};

    VkCopyImageInfo2 copy_image_info2 = vku::InitStructHelper();
    copy_image_info2.dstImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    copy_image_info2.srcImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    copy_image_info2.regionCount = 1;
    copy_image_info2.pRegions = &region2;

    // 1D texture w/ offset.y > 0. Source = VU 09c00124, dest = 09c00130
    {
        region2.srcOffset.y = 1;
        copy_image_info2.srcImage = image_1D;
        copy_image_info2.dstImage = image_2D;

        m_errorMonitor->SetDesiredError("VUID-VkCopyImageInfo2-srcImage-00146");
        m_errorMonitor->SetDesiredError("VUID-VkCopyImageInfo2-srcOffset-00145");   // also y-dim overrun
        m_errorMonitor->SetDesiredError("VUID-VkCopyImageInfo2-apiVersion-07933");  // not same image type
        vk::CmdCopyImage2KHR(m_command_buffer.handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
        region2.srcOffset.y = 0;
    }

    {
        region2.dstOffset.y = 1;
        copy_image_info2.srcImage = image_2D;
        copy_image_info2.dstImage = image_1D;

        m_errorMonitor->SetDesiredError("VUID-VkCopyImageInfo2-dstImage-00152");
        m_errorMonitor->SetDesiredError("VUID-VkCopyImageInfo2-dstOffset-00151");   // also y-dim overrun
        m_errorMonitor->SetDesiredError("VUID-VkCopyImageInfo2-apiVersion-07933");  // not same image type
        vk::CmdCopyImage2KHR(m_command_buffer.handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
        region2.dstOffset.y = 0;
    }

    // 1D texture w/ extent.height > 1. Source = VU 09c00124, dest = 09c00130
    {
        region2.extent.height = 2;
        copy_image_info2.srcImage = image_1D;
        copy_image_info2.dstImage = image_2D;
        m_errorMonitor->SetDesiredError("VUID-VkCopyImageInfo2-srcImage-00146");
        m_errorMonitor->SetDesiredError("VUID-VkCopyImageInfo2-srcOffset-00145");   // also y-dim overrun
        m_errorMonitor->SetDesiredError("VUID-VkCopyImageInfo2-apiVersion-07933");  // not same image type
        vk::CmdCopyImage2KHR(m_command_buffer.handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();

        copy_image_info2.srcImage = image_2D;
        copy_image_info2.dstImage = image_1D;
        m_errorMonitor->SetDesiredError("VUID-VkCopyImageInfo2-dstImage-00152");
        m_errorMonitor->SetDesiredError("VUID-VkCopyImageInfo2-dstOffset-00151");   // also y-dim overrun
        m_errorMonitor->SetDesiredError("VUID-VkCopyImageInfo2-apiVersion-07933");  // not same image type
        vk::CmdCopyImage2KHR(m_command_buffer.handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeCopyBufferImage, ImageTypeExtentMismatchMaintenance1) {
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_1D;
    ci.format = image_format;
    ci.extent = {32, 1, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    // Create 1D image
    vkt::Image image_1D(*m_device, ci, vkt::set_layout);

    // 2D image
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.extent = {32, 32, 1};
    vkt::Image image_2D(*m_device, ci, vkt::set_layout);

    // 3D image
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.extent = {32, 32, 8};
    vkt::Image image_3D(*m_device, ci, vkt::set_layout);

    // 2D image array
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.extent = {32, 32, 1};
    ci.arrayLayers = 8;
    vkt::Image image_2D_array(*m_device, ci, vkt::set_layout);

    // second 2D image array
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.extent = {32, 32, 1};
    ci.arrayLayers = 8;
    vkt::Image image_2D_array_2(*m_device, ci, vkt::set_layout);

    m_command_buffer.Begin();

    VkImageCopy copy_region;
    copy_region.extent = {32, 1, 1};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-07743");
    vk::CmdCopyImage(m_command_buffer.handle(), image_1D.handle(), VK_IMAGE_LAYOUT_GENERAL, image_2D.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Copy from layer not present
    copy_region.srcSubresource.baseArrayLayer = 4;
    copy_region.srcSubresource.layerCount = 6;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01791");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcSubresource-07968");
    vk::CmdCopyImage(m_command_buffer.handle(), image_2D_array.handle(), VK_IMAGE_LAYOUT_GENERAL, image_3D.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;

    // Copy to layer not present
    copy_region.dstSubresource.baseArrayLayer = 1;
    copy_region.dstSubresource.layerCount = 8;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstImage-01792");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstSubresource-07968");
    vk::CmdCopyImage(m_command_buffer.handle(), image_3D.handle(), VK_IMAGE_LAYOUT_GENERAL, image_2D_array.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.dstSubresource.baseArrayLayer = 0;
    copy_region.dstSubresource.layerCount = 1;

    // both 2D and extent.depth not 1
    // Need two 2D array images to prevent other errors
    copy_region.extent = {4, 1, 2};
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01790");
    vk::CmdCopyImage(m_command_buffer.handle(), image_2D_array.handle(), VK_IMAGE_LAYOUT_GENERAL, image_2D_array_2.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.extent = {32, 1, 1};

    // 2D src / 3D dst and depth not equal to src layerCount
    copy_region.extent = {4, 1, 2};
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01791");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-08793");
    vk::CmdCopyImage(m_command_buffer.handle(), image_2D_array.handle(), VK_IMAGE_LAYOUT_GENERAL, image_3D.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.extent = {32, 1, 1};

    // 3D src / 2D dst and depth not equal to dst layerCount
    copy_region.extent = {4, 1, 2};
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstImage-01792");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-08793");
    vk::CmdCopyImage(m_command_buffer.handle(), image_3D.handle(), VK_IMAGE_LAYOUT_GENERAL, image_2D_array.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.extent = {32, 1, 1};

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, ImageCompressedBlockAlignment) {
    // Image copy tests on compressed images with block alignment errors
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::textureCompressionBC);
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_BC3_SRGB_BLOCK,
                                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    VkImageFormatProperties img_prop = {};
    if (VK_SUCCESS != vk::GetPhysicalDeviceImageFormatProperties(m_device->Physical().handle(), image_ci.format, image_ci.imageType,
                                                                 image_ci.tiling, image_ci.usage, image_ci.flags, &img_prop)) {
        GTEST_SKIP() << "No compressed formats supported";
    }

    vkt::Image image_1(*m_device, image_ci, vkt::set_layout);

    image_ci.extent = {62, 62, 1};  // slightly smaller and not divisible by block size
    vkt::Image image_2(*m_device, image_ci, vkt::set_layout);

    m_command_buffer.Begin();

    VkImageCopy copy_region;
    copy_region.extent = {48, 48, 1};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    // Sanity check
    vk::CmdCopyImage(m_command_buffer.handle(), image_1.handle(), VK_IMAGE_LAYOUT_GENERAL, image_2.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    const char* vuid = nullptr;
    bool ycbcr =
        (IsExtensionsEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME) || (DeviceValidationVersion() >= VK_API_VERSION_1_1));

    // Src, Dest offsets must be multiples of compressed block sizes {4, 4, 1}
    // Image transfer granularity gets set to compressed block size, so an ITG error is also (unavoidably) triggered.
    copy_region.srcOffset = {2, 4, 0};  // source width
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-pRegions-07278");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcOffset-01783");  // srcOffset image transfer granularity
    vk::CmdCopyImage(m_command_buffer.handle(), image_1.handle(), VK_IMAGE_LAYOUT_GENERAL, image_2.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.srcOffset = {12, 1, 0};  // source height
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-pRegions-07279");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcOffset-01783");  // srcOffset image transfer granularity
    vk::CmdCopyImage(m_command_buffer.handle(), image_1.handle(), VK_IMAGE_LAYOUT_GENERAL, image_2.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.srcOffset = {0, 0, 0};

    copy_region.dstOffset = {1, 0, 0};  // dest width
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-pRegions-07281");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstOffset-01784");  // dstOffset image transfer granularity
    vk::CmdCopyImage(m_command_buffer.handle(), image_1.handle(), VK_IMAGE_LAYOUT_GENERAL, image_2.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.dstOffset = {4, 1, 0};  // dest height
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-pRegions-07282");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstOffset-01784");  // dstOffset image transfer granularity
    vk::CmdCopyImage(m_command_buffer.handle(), image_1.handle(), VK_IMAGE_LAYOUT_GENERAL, image_2.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.dstOffset = {0, 0, 0};

    // Copy extent must be multiples of compressed block sizes {4, 4, 1} if not full width/height
    vuid = ycbcr ? "VUID-vkCmdCopyImage-srcImage-01728" : "VUID-vkCmdCopyImage-srcImage-01728";
    copy_region.extent = {62, 60, 1};  // source width
    m_errorMonitor->SetDesiredError(vuid);
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcOffset-01783");  // src extent image transfer granularity
    vk::CmdCopyImage(m_command_buffer.handle(), image_1.handle(), VK_IMAGE_LAYOUT_GENERAL, image_2.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    vuid = ycbcr ? "VUID-vkCmdCopyImage-srcImage-01729" : "VUID-vkCmdCopyImage-srcImage-01729";
    copy_region.extent = {60, 62, 1};  // source height
    m_errorMonitor->SetDesiredError(vuid);
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcOffset-01783");  // src extent image transfer granularity
    vk::CmdCopyImage(m_command_buffer.handle(), image_1.handle(), VK_IMAGE_LAYOUT_GENERAL, image_2.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    vuid = ycbcr ? "VUID-vkCmdCopyImage-dstImage-01732" : "VUID-vkCmdCopyImage-dstImage-01732";
    copy_region.extent = {62, 60, 1};  // dest width
    m_errorMonitor->SetDesiredError(vuid);
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstOffset-01784");  // dst extent image transfer granularity
    vk::CmdCopyImage(m_command_buffer.handle(), image_2.handle(), VK_IMAGE_LAYOUT_GENERAL, image_1.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    vuid = ycbcr ? "VUID-vkCmdCopyImage-dstImage-01733" : "VUID-vkCmdCopyImage-dstImage-01733";
    copy_region.extent = {60, 62, 1};  // dest height
    m_errorMonitor->SetDesiredError(vuid);
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstOffset-01784");  // dst extent image transfer granularity
    vk::CmdCopyImage(m_command_buffer.handle(), image_2.handle(), VK_IMAGE_LAYOUT_GENERAL, image_1.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Note: "VUID-vkCmdCopyImage-srcImage-01730", "VUID-vkCmdCopyImage-dstImage-01734", "VUID-vkCmdCopyImage-srcImage-01730",
    // "VUID-vkCmdCopyImage-dstImage-01734"
    //       There are currently no supported compressed formats with a block depth other than 1,
    //       so impossible to create a 'not a multiple' condition for depth.
    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, ImageSrcSizeExceeded) {
    // Image copy with source region specified greater than src image size
    RETURN_IF_SKIP(Init());

    // Create images with full mip chain
    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.extent = {32, 32, 8};
    ci.mipLevels = 6;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    vkt::Image src_image(*m_device, ci, vkt::set_layout);

    // Dest image with one more mip level
    ci.extent = {64, 64, 16};
    ci.mipLevels = 7;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image dst_image(*m_device, ci, vkt::set_layout);

    m_command_buffer.Begin();

    VkImageCopy copy_region;
    copy_region.extent = {32, 32, 8};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    // Source exceeded in x-dim
    copy_region.srcOffset.x = 4;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcOffset-00144");
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Dest exceeded in x-dim in negative direction (since offset is a signed in)
    copy_region.extent.width = 4;
    copy_region.srcOffset.x = -8;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcOffset-00144");
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.extent.width = 32;

    // Source exceeded in y-dim
    copy_region.srcOffset.x = 0;
    copy_region.extent.height = 48;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcOffset-00145");
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Source exceeded in z-dim
    copy_region.extent = {4, 4, 4};
    copy_region.srcSubresource.mipLevel = 2;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcOffset-00147");
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, ImageDstSizeExceeded) {
    // Image copy with dest region specified greater than dest image size
    RETURN_IF_SKIP(Init());

    // Create images with full mip chain
    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.extent = {32, 32, 8};
    ci.mipLevels = 6;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image dst_image(*m_device, ci, vkt::set_layout);

    // Src image with one more mip level
    ci.extent = {64, 64, 16};
    ci.mipLevels = 7;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    vkt::Image src_image(*m_device, ci, vkt::set_layout);

    m_command_buffer.Begin();

    VkImageCopy copy_region;
    copy_region.extent = {32, 32, 8};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    // Dest exceeded in x-dim
    copy_region.dstOffset.x = 4;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstOffset-00150");
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Dest exceeded in x-dim in negative direction (since offset is a signed in)
    copy_region.extent.width = 4;
    copy_region.dstOffset.x = -8;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstOffset-00150");
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.extent.width = 32;

    copy_region.dstOffset.x = 0;
    copy_region.extent.height = 48;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstOffset-00151");
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Dest exceeded in z-dim
    copy_region.extent = {4, 4, 4};
    copy_region.dstSubresource.mipLevel = 2;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstOffset-00153");
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, ImageZeroSize) {
    TEST_DESCRIPTION("Image Copy with empty regions");
    RETURN_IF_SKIP(Init());

    // Create images with full mip chain
    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.extent = {32, 32, 8};
    ci.mipLevels = 6;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    vkt::Image src_image(*m_device, ci, vkt::set_layout);

    ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image dst_image(*m_device, ci, vkt::set_layout);

    // large enough for image
    vkt::Buffer buffer(*m_device, 16384, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0);

    m_command_buffer.Begin();

    VkImageCopy copy_region;
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    copy_region.extent = {4, 4, 0};
    m_errorMonitor->SetDesiredError("VUID-VkImageCopy-extent-06670");
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    copy_region.extent = {0, 0, 4};
    m_errorMonitor->SetDesiredError("VUID-VkImageCopy-extent-06668");  // width
    m_errorMonitor->SetDesiredError("VUID-VkImageCopy-extent-06669");  // height
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    VkImageSubresourceLayers image_subresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkBufferImageCopy buffer_image_copy;
    buffer_image_copy.bufferRowLength = 0;
    buffer_image_copy.bufferImageHeight = 0;
    buffer_image_copy.imageSubresource = image_subresource;
    buffer_image_copy.imageOffset = {0, 0, 0};
    buffer_image_copy.bufferOffset = 0;

    buffer_image_copy.imageExtent = {4, 0, 1};
    m_errorMonitor->SetDesiredError("VUID-VkBufferImageCopy-imageExtent-06660");
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer.handle(), 1,
                             &buffer_image_copy);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkBufferImageCopy-imageExtent-06660");
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer.handle(), dst_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &buffer_image_copy);
    m_errorMonitor->VerifyFound();

    // depth is now zero
    buffer_image_copy.imageExtent = {4, 1, 0};
    m_errorMonitor->SetDesiredError("VUID-VkBufferImageCopy-imageExtent-06661");
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer.handle(), 1,
                             &buffer_image_copy);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkBufferImageCopy-imageExtent-06661");
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer.handle(), dst_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &buffer_image_copy);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, ImageMultiPlaneSizeExceeded) {
    TEST_DESCRIPTION("Image Copy for multi-planar format that exceed size of plane for both src and dst");

    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    // Try to use VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM because need multi-plane format for some tests and likely supported due to
    // copy support being required with samplerYcbcrConversion feature
    VkFormatProperties props = {0, 0, 0};
    bool missing_format_support = false;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, &props);
    missing_format_support |= (props.bufferFeatures == 0 && props.linearTilingFeatures == 0 && props.optimalTilingFeatures == 0);
    missing_format_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0;
    missing_format_support |= (props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0;

    if (missing_format_support == true) {
        GTEST_SKIP() << "VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM transfer not supported";
    }

    // 128^2 texels in plane_0 and 64^2 texels in plane_1
    vkt::Image src_image(*m_device, 128, 128, 1, VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    src_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::Image dst_image(*m_device, 128, 128, 1, VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    dst_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageCopy copy_region = {};
    copy_region.extent = {64, 64, 1};  // Size of plane 1
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_PLANE_1_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_PLANE_1_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    VkImageCopy original_region = copy_region;

    m_command_buffer.Begin();

    // Should be able to do a 64x64 copy from plane 1 -> Plane 1
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    // Should be able to do a 64x64 copy from plane 0 -> Plane 0
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    VkMemoryBarrier mem_barrier = vku::InitStructHelper();
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    // Should be able to do a 64x64 copy from plane 0 -> Plane 1
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barrier, 0, nullptr, 0, nullptr);
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    // Should be able to do a 64x64 copy from plane 0 -> Plane 1
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barrier, 0, nullptr, 0, nullptr);
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    // Should be able to do a 128x64 copy from plane 0 -> Plane 0
    copy_region.extent = {128, 64, 1};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1,
                           &mem_barrier, 0, nullptr, 0, nullptr);
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    // 128x64 copy from plane 0 -> Plane 1
    copy_region.extent = {128, 64, 1};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstOffset-00150");
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // 128x64 copy from plane 1 -> Plane 0
    copy_region.extent = {128, 64, 1};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcOffset-00144");
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // src exceeded in y-dim from offset
    copy_region = original_region;
    copy_region.srcOffset.y = 4;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcOffset-00145");
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // dst exceeded in y-dim from offset
    copy_region = original_region;
    copy_region.dstOffset.y = 4;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstOffset-00151");
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, ImageFormatSizeMismatch) {
    TEST_DESCRIPTION("two single plane mismatch");
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::samplerYcbcrConversion);
    RETURN_IF_SKIP(Init());

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required VK_FORMAT_R8_UNORM features not supported";
    } else if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R8_UINT, VK_IMAGE_TILING_OPTIMAL,
                                           VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required VK_FORMAT_R8_UINT features not supported";
    } else if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                           VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Format not supported";
    }

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8_UNORM,
                                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image_8b_unorm(*m_device, image_ci, vkt::set_layout);

    image_ci.format = VK_FORMAT_R8_UINT;
    vkt::Image image_8b_uint(*m_device, image_ci, vkt::set_layout);

    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    vkt::Image image_32b_unorm(*m_device, image_ci, vkt::set_layout);

    m_command_buffer.Begin();
    VkImageCopy copy_region;
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {1, 1, 1};

    // Sanity check between two 8bit formats
    vk::CmdCopyImage(m_command_buffer.handle(), image_8b_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL, image_8b_uint.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01548");
    vk::CmdCopyImage(m_command_buffer.handle(), image_8b_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL, image_32b_unorm.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Swap src and dst
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01548");
    vk::CmdCopyImage(m_command_buffer.handle(), image_32b_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL, image_8b_unorm.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, ImageFormatSizeMismatch2) {
    TEST_DESCRIPTION("DstImage is a mismatched plane of a multi-planar format");
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::samplerYcbcrConversion);
    RETURN_IF_SKIP(Init());

    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required VK_FORMAT_R8_UNORM features not supported";
    } else if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R8_UINT, VK_IMAGE_TILING_OPTIMAL,
                                           VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required VK_FORMAT_R8_UINT features not supported";
    } else if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                           VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Format not supported";
    }

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8_UNORM,
                                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image_8b_unorm(*m_device, image_ci, vkt::set_layout);

    image_ci.format = VK_FORMAT_R8_UINT;
    vkt::Image image_8b_uint(*m_device, image_ci, vkt::set_layout);

    image_ci.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    vkt::Image image_8b_16b_420_unorm(*m_device, image_ci, vkt::set_layout);

    m_command_buffer.Begin();
    VkImageCopy copy_region;
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {1, 1, 1};

    // First test single-plane -> multi-plan
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;

    // Plane 0 is VK_FORMAT_R8_UNORM so this should succeed
    vk::CmdCopyImage(m_command_buffer.handle(), image_8b_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL, image_8b_16b_420_unorm.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    image_8b_16b_420_unorm.ImageMemoryBarrier(m_command_buffer, VK_IMAGE_ASPECT_PLANE_0_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                                              VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL);

    // Make sure no false postiives if Compatible format
    vk::CmdCopyImage(m_command_buffer.handle(), image_8b_uint.handle(), VK_IMAGE_LAYOUT_GENERAL, image_8b_16b_420_unorm.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    // Plane 1 is VK_FORMAT_R8G8_UNORM so this should fail
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-None-01549");
    vk::CmdCopyImage(m_command_buffer.handle(), image_8b_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL, image_8b_16b_420_unorm.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Same tests but swap src and dst
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    image_8b_unorm.ImageMemoryBarrier(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_TRANSFER_READ_BIT,
                                      VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_8b_16b_420_unorm.ImageMemoryBarrier(m_command_buffer, VK_IMAGE_ASPECT_PLANE_0_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                                              VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL);

    vk::CmdCopyImage(m_command_buffer.handle(), image_8b_16b_420_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL, image_8b_unorm.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    image_8b_16b_420_unorm.ImageMemoryBarrier(m_command_buffer, VK_IMAGE_ASPECT_PLANE_0_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                                              VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL);

    vk::CmdCopyImage(m_command_buffer.handle(), image_8b_16b_420_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL, image_8b_uint.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-None-01549");
    vk::CmdCopyImage(m_command_buffer.handle(), image_8b_16b_420_unorm.handle(), VK_IMAGE_LAYOUT_GENERAL, image_8b_unorm.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, ImageDepthStencilFormatMismatch) {
    RETURN_IF_SKIP(Init());
    auto depth_format = FindSupportedDepthStencilFormat(Gpu());

    VkFormatProperties properties;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), VK_FORMAT_D32_SFLOAT, &properties);
    if (properties.optimalTilingFeatures == 0) {
        GTEST_SKIP() << "Image format not supported";
    }

    vkt::Image srcImage(*m_device, 32, 32, 1, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    srcImage.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::Image dstImage(*m_device, 32, 32, 1, depth_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    dstImage.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    // Create two images of different types and try to copy between them

    m_command_buffer.Begin();
    VkImageCopy copy_region;
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {1, 1, 1};

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01548");
    vk::CmdCopyImage(m_command_buffer.handle(), srcImage.handle(), VK_IMAGE_LAYOUT_GENERAL, dstImage.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_command_buffer.End();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCopyBufferImage, ImageSampleCountMismatch) {
    TEST_DESCRIPTION("Image copies with sample count mis-matches");

    RETURN_IF_SKIP(Init());

    VkImageFormatProperties image_format_properties;
    vk::GetPhysicalDeviceImageFormatProperties(Gpu(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 0,
                                               &image_format_properties);

    if ((0 == (VK_SAMPLE_COUNT_2_BIT & image_format_properties.sampleCounts)) ||
        (0 == (VK_SAMPLE_COUNT_4_BIT & image_format_properties.sampleCounts))) {
        GTEST_SKIP() << "Image multi-sample support not found";
    }

    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image1(*m_device, image_ci, vkt::set_layout);

    image_ci.samples = VK_SAMPLE_COUNT_2_BIT;
    vkt::Image image2(*m_device, image_ci, vkt::set_layout);

    image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
    vkt::Image image4(*m_device, image_ci, vkt::set_layout);

    m_command_buffer.Begin();

    VkImageCopy copy_region;
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {128, 128, 1};

    // Copy a single sample image to/from a multi-sample image
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-00136");
    vk::CmdCopyImage(m_command_buffer.handle(), image1.handle(), VK_IMAGE_LAYOUT_GENERAL, image4.handle(), VK_IMAGE_LAYOUT_GENERAL,
                     1, &copy_region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-00136");
    vk::CmdCopyImage(m_command_buffer.handle(), image2.handle(), VK_IMAGE_LAYOUT_GENERAL, image1.handle(), VK_IMAGE_LAYOUT_GENERAL,
                     1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Copy between multi-sample images with different sample counts
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-00136");
    vk::CmdCopyImage(m_command_buffer.handle(), image2.handle(), VK_IMAGE_LAYOUT_GENERAL, image4.handle(), VK_IMAGE_LAYOUT_GENERAL,
                     1, &copy_region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-00136");
    vk::CmdCopyImage(m_command_buffer.handle(), image4.handle(), VK_IMAGE_LAYOUT_GENERAL, image2.handle(), VK_IMAGE_LAYOUT_GENERAL,
                     1, &copy_region);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, ImageLayerCount) {
    TEST_DESCRIPTION("Check layerCount in vkCmdCopyImage");
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);

    m_command_buffer.Begin();

    VkImageCopy copy_region;
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 0};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 0};
    copy_region.dstOffset = {32, 32, 0};
    copy_region.extent = {16, 16, 1};

    m_errorMonitor->SetDesiredError("VUID-VkImageSubresourceLayers-layerCount-01700");  // src
    m_errorMonitor->SetDesiredError("VUID-VkImageSubresourceLayers-layerCount-01700");  // dst
    vk::CmdCopyImage(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &copy_region);
    m_errorMonitor->VerifyFound();

    copy_region.srcSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
    copy_region.dstSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
    m_errorMonitor->SetDesiredError("VUID-VkImageSubresourceLayers-layerCount-09243");  // src
    m_errorMonitor->SetDesiredError("VUID-VkImageSubresourceLayers-layerCount-09243");  // dst
    vk::CmdCopyImage(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &copy_region);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, ImageAspectMismatch) {
    TEST_DESCRIPTION("Image copies with aspect mask errors");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    auto ds_format = FindSupportedDepthStencilFormat(Gpu());

    // Add Transfer support for all used formats
    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required VK_FORMAT_R32_SFLOAT features not supported";
    } else if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                           VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required VK_FORMAT_D32_SFLOAT features not supported";
    } else if (!FormatFeaturesAreSupported(Gpu(), ds_format, VK_IMAGE_TILING_OPTIMAL,
                                           VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required Depth/Stencil Format features not supported";
    }

    vkt::Image color_image(*m_device, 128, 128, 1, VK_FORMAT_R32_SFLOAT,
                           VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image depth_image(*m_device, 128, 128, 1, VK_FORMAT_D32_SFLOAT,
                           VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image ds_image(*m_device, 128, 128, 1, ds_format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    color_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    depth_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    ds_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageCopy copy_region;
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    copy_region.dstOffset = {64, 0, 0};
    copy_region.extent = {64, 128, 1};

    // Submitting command before command buffer is in recording state
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-commandBuffer-recording");
    vk::CmdCopyImage(m_command_buffer.handle(), depth_image.handle(), VK_IMAGE_LAYOUT_GENERAL, depth_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    m_command_buffer.Begin();

    // Src and dest aspect masks don't match
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01551");
    vk::CmdCopyImage(m_command_buffer.handle(), ds_image.handle(), VK_IMAGE_LAYOUT_GENERAL, ds_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    // Illegal combinations of aspect bits
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;  // color must be alone
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkImageSubresourceLayers-aspectMask-00167");
    // These aspect/format mismatches are redundant but unavoidable here
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-aspectMask-00142");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01551");
    vk::CmdCopyImage(m_command_buffer.handle(), color_image.handle(), VK_IMAGE_LAYOUT_GENERAL, color_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    // same test for dstSubresource
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;  // color must be alone
    m_errorMonitor->SetDesiredError("VUID-VkImageSubresourceLayers-aspectMask-00167");
    // These aspect/format mismatches are redundant but unavoidable here
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-aspectMask-00143");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01551");
    vk::CmdCopyImage(m_command_buffer.handle(), color_image.handle(), VK_IMAGE_LAYOUT_GENERAL, color_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Metadata aspect is illegal
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkImageSubresourceLayers-aspectMask-00168");
    // These aspect/format mismatches are redundant but unavoidable here
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01551");
    vk::CmdCopyImage(m_command_buffer.handle(), color_image.handle(), VK_IMAGE_LAYOUT_GENERAL, color_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    // same test for dstSubresource
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkImageSubresourceLayers-aspectMask-00168");
    // These aspect/format mismatches are redundant but unavoidable here
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01551");
    vk::CmdCopyImage(m_command_buffer.handle(), color_image.handle(), VK_IMAGE_LAYOUT_GENERAL, color_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Aspect Memory Plane mask is illegal
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkImageSubresourceLayers-aspectMask-02247");
    // These aspect/format mismatches are redundant but unavoidable here
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01551");
    vk::CmdCopyImage(m_command_buffer.handle(), color_image.handle(), VK_IMAGE_LAYOUT_GENERAL, color_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    // Aspect mask doesn't match source image format
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-aspectMask-00142");
    vk::CmdCopyImage(m_command_buffer.handle(), color_image.handle(), VK_IMAGE_LAYOUT_GENERAL, depth_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Aspect mask doesn't match dest image format
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-aspectMask-00143");
    vk::CmdCopyImage(m_command_buffer.handle(), color_image.handle(), VK_IMAGE_LAYOUT_GENERAL, depth_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // Check no performance warnings regarding layout are thrown when copying from and to the same image
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    vk::CmdCopyImage(m_command_buffer.handle(), depth_image.handle(), VK_IMAGE_LAYOUT_GENERAL, depth_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, DepthStencilImageCopyNoGraphicsQueueFlags) {
    TEST_DESCRIPTION(
        "Allocate a command buffer on a queue that does not support graphics and try to issue a depth/stencil image copy to "
        "buffer");

    RETURN_IF_SKIP(Init());

    const std::optional<uint32_t> no_gfx = m_device->NonGraphicsQueueFamily();
    if (!no_gfx) {
        GTEST_SKIP() << "Non-graphics queue family not found";
    }

    // Create Depth image
    const VkFormat ds_format = FindSupportedDepthOnlyFormat(Gpu());
    vkt::Image ds_image(*m_device, 64, 64, 1, ds_format,
                        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    ds_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    // 256k to have more then enough to copy
    vkt::Buffer buffer(*m_device, 262144, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0);

    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {64, 64, 1};
    region.bufferOffset = 0;

    // Create command pool on a non-graphics queue
    vkt::CommandPool command_pool(*m_device, no_gfx.value());

    // Setup command buffer on pool
    vkt::CommandBuffer command_buffer(*m_device, command_pool);
    command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-commandBuffer-07739");
    vk::CmdCopyBufferToImage(command_buffer.handle(), buffer.handle(), ds_image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCopyBufferImage, ImageTransferQueueFlags) {
    TEST_DESCRIPTION(
        "Allocate a command buffer on a queue that does not support graphics/compute and try to issue an invalid image copy to "
        "buffer");

    RETURN_IF_SKIP(Init());

    const std::optional<uint32_t> transfer_qfi = m_device->TransferOnlyQueueFamily();
    if (!transfer_qfi) {
        GTEST_SKIP() << "Transfer-only queue family not found";
    }

    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    // 256k to have more then enough to copy
    vkt::Buffer buffer(*m_device, 262144, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {16, 16, 1};
    region.bufferOffset = 5;

    // Create command pool on a non-graphics queue
    vkt::CommandPool command_pool(*m_device, transfer_qfi.value());

    // Setup command buffer on pool
    vkt::CommandBuffer command_buffer(*m_device, command_pool);
    command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-dstImage-07975");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-commandBuffer-07737");
    vk::CmdCopyBufferToImage(command_buffer.handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &region);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCopyBufferImage, CopyCommands2V13) {
    TEST_DESCRIPTION("Ensure copy_commands2 promotions are validated");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(Init());
    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::Image image2(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM,
                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    image2.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::Buffer dst_buffer(*m_device, 128 * 128, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer src_buffer(*m_device, 128 * 128, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    VkImageCopy2 copy_region = vku::InitStructHelper();
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstOffset = {4, 4, 0};
    copy_region.extent = {1, 1, 1};

    VkCopyImageInfo2 copy_image_info = vku::InitStructHelper();
    copy_image_info.srcImage = image.handle();
    copy_image_info.srcImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    copy_image_info.dstImage = image.handle();
    copy_image_info.dstImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    copy_image_info.regionCount = 1;
    copy_image_info.pRegions = &copy_region;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-aspect-06663");
    vk::CmdCopyImage2(m_command_buffer.handle(), &copy_image_info);
    m_errorMonitor->VerifyFound();

    VkBufferCopy2 copy_buffer = vku::InitStructHelper();
    copy_buffer.dstOffset = 4;
    copy_buffer.size = 4;

    VkCopyBufferInfo2 copy_buffer_info = vku::InitStructHelper();
    copy_buffer_info.srcBuffer = dst_buffer.handle();
    copy_buffer_info.dstBuffer = dst_buffer.handle();
    copy_buffer_info.regionCount = 1;
    copy_buffer_info.pRegions = &copy_buffer;

    m_errorMonitor->SetDesiredError("VUID-VkCopyBufferInfo2-srcBuffer-00118");
    vk::CmdCopyBuffer2(m_command_buffer.handle(), &copy_buffer_info);
    m_errorMonitor->VerifyFound();

    VkBufferImageCopy2 bic_region = vku::InitStructHelper();
    bic_region.bufferRowLength = 128;
    bic_region.bufferImageHeight = 128;
    bic_region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    bic_region.imageExtent = {4, 4, 1};

    VkCopyBufferToImageInfo2 buffer_image_info = vku::InitStructHelper();
    buffer_image_info.srcBuffer = src_buffer.handle();
    buffer_image_info.dstImage = image.handle();
    buffer_image_info.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    buffer_image_info.regionCount = 1;
    buffer_image_info.pRegions = &bic_region;

    m_errorMonitor->SetDesiredError("VUID-VkCopyBufferToImageInfo2-dstImage-00177");
    vk::CmdCopyBufferToImage2(m_command_buffer.handle(), &buffer_image_info);
    m_errorMonitor->VerifyFound();

    VkCopyImageToBufferInfo2 image_buffer_info = vku::InitStructHelper();
    image_buffer_info.dstBuffer = src_buffer.handle();
    image_buffer_info.srcImage = image.handle();
    image_buffer_info.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    image_buffer_info.regionCount = 1;
    image_buffer_info.pRegions = &bic_region;

    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToBufferInfo2-dstBuffer-00191");
    vk::CmdCopyImageToBuffer2(m_command_buffer.handle(), &image_buffer_info);
    m_errorMonitor->VerifyFound();

    VkImageBlit2 blit_region = vku::InitStructHelper();
    blit_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    blit_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.srcOffsets[1] = {31, 31, 1};
    blit_region.dstOffsets[0] = {32, 32, 0};
    blit_region.dstOffsets[1] = {64, 64, 1};

    VkBlitImageInfo2 blit_image_info = vku::InitStructHelper();
    blit_image_info.srcImage = image.handle();
    blit_image_info.srcImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    blit_image_info.dstImage = image.handle();
    blit_image_info.dstImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    blit_image_info.regionCount = 1;
    blit_image_info.pRegions = &blit_region;
    blit_image_info.filter = VK_FILTER_NEAREST;

    m_errorMonitor->SetDesiredError("VUID-VkBlitImageInfo2-dstImage-00224");
    vk::CmdBlitImage2(m_command_buffer.handle(), &blit_image_info);
    m_errorMonitor->VerifyFound();

    VkImageResolve2 resolve_region = vku::InitStructHelper();
    resolve_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    resolve_region.srcOffset = {0, 0, 0};
    resolve_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    resolve_region.dstOffset = {0, 0, 0};
    resolve_region.extent = {1, 1, 1};

    VkResolveImageInfo2 resolve_image_info = vku::InitStructHelper();
    resolve_image_info.srcImage = image.handle();
    resolve_image_info.srcImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    resolve_image_info.dstImage = image2.handle();
    resolve_image_info.dstImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    resolve_image_info.regionCount = 1;
    resolve_image_info.pRegions = &resolve_region;

    m_errorMonitor->SetDesiredError("VUID-VkResolveImageInfo2-srcImage-00257");
    vk::CmdResolveImage2(m_command_buffer.handle(), &resolve_image_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCopyBufferImage, ImageOverlappingMemory) {
    TEST_DESCRIPTION("Validate Copy Image from/to Buffer with overlapping memory");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(Init());

    VkDeviceSize buff_size = 32 * 32 * 4;
    vkt::Buffer buffer(*m_device,
                       vkt::Buffer::CreateInfo(buff_size, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT),
                       vkt::no_mem);
    const auto buffer_memory_requirements = buffer.MemoryRequirements();

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    vkt::Image image(*m_device, image_ci, vkt::no_mem);
    const auto image_memory_requirements = image.MemoryRequirements();

    vkt::DeviceMemory mem;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = (std::max)(buffer_memory_requirements.size, image_memory_requirements.size);
    bool has_memtype = m_device->Physical().SetMemoryType(
        buffer_memory_requirements.memoryTypeBits & image_memory_requirements.memoryTypeBits, &alloc_info, 0);
    if (!has_memtype) {
        GTEST_SKIP() << "Failed to find a memory type for both a buffer and an image";
    }
    mem.init(*m_device, alloc_info);

    buffer.BindMemory(mem, 0);
    image.BindMemory(mem, 0);

    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageOffset = {0, 0, 0};
    region.bufferOffset = 0;

    region.imageExtent = {32, 32, 1};
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-pRegions-00184");
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer.handle(), 1, &region);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-pRegions-00173");
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    VkBufferImageCopy2 bic2_region = vku::InitStructHelper();
    bic2_region.bufferRowLength = 0;
    bic2_region.bufferImageHeight = 0;
    bic2_region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    bic2_region.imageOffset = {0, 0, 0};
    bic2_region.bufferOffset = 0;
    bic2_region.imageExtent = {32, 32, 1};

    VkCopyImageToBufferInfo2 i2b2_info = vku::InitStructHelper();
    i2b2_info.dstBuffer = buffer.handle();
    i2b2_info.pRegions = &bic2_region;
    i2b2_info.regionCount = 1;
    i2b2_info.srcImage = image.handle();
    i2b2_info.srcImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToBufferInfo2-pRegions-00184");
    vk::CmdCopyImageToBuffer2(m_command_buffer.handle(), &i2b2_info);
    m_errorMonitor->VerifyFound();

    VkCopyBufferToImageInfo2 b2i2_info = vku::InitStructHelper();
    b2i2_info.srcBuffer = buffer.handle();
    b2i2_info.pRegions = &bic2_region;
    b2i2_info.regionCount = 1;
    b2i2_info.dstImage = image.handle();
    b2i2_info.dstImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    m_errorMonitor->SetDesiredError("VUID-VkCopyBufferToImageInfo2-pRegions-00173");
    vk::CmdCopyBufferToImage2(m_command_buffer.handle(), &b2i2_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCopyBufferImage, ImageRemainingLayers) {
    TEST_DESCRIPTION("Test copying an image with VkImageSubresourceLayers.layerCount = VK_REMAINING_ARRAY_LAYERS");
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    // Copy from a to b
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    vkt::Image image_a(*m_device, image_ci, vkt::set_layout);

    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image image_b(*m_device, image_ci, vkt::set_layout);

    ASSERT_TRUE(image_a.initialized());
    ASSERT_TRUE(image_b.initialized());

    m_command_buffer.Begin();

    image_a.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    image_b.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkImageCopy copy_region{};
    copy_region.extent = image_ci.extent;
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.baseArrayLayer = 7;
    copy_region.srcSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;  // This value is unsupported by VkImageSubresourceLayer
    copy_region.dstSubresource = copy_region.srcSubresource;

    // These vuids will trigger a special message stating that VK_REMAINING_ARRAY_LAYERS is unsupported
    m_errorMonitor->SetDesiredError("VUID-VkImageSubresourceLayers-layerCount-09243");  // src
    m_errorMonitor->SetDesiredError("VUID-VkImageSubresourceLayers-layerCount-09243");  // dst
    vk::CmdCopyImage(m_command_buffer.handle(), image_a.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image_b.handle(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    const uint32_t buffer_size = 32 * 32 * 4;
    vkt::Buffer buffer(*m_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    VkBufferImageCopy buffer_copy{};
    buffer_copy.bufferImageHeight = image_ci.extent.height;
    buffer_copy.bufferRowLength = image_ci.extent.width;
    buffer_copy.imageExtent = image_ci.extent;
    buffer_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    buffer_copy.imageSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;  // This value is unsupported by VkImageSubresourceLayers
    buffer_copy.imageSubresource.mipLevel = 0;
    buffer_copy.imageSubresource.baseArrayLayer = 5;

    // This error will trigger first stating that the copy is too big for the buffer, because of VK_REMAINING_ARRAY_LAYERS
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-pRegions-00171");
    // This error will trigger second stating that VK_REMAINING_ARRAY_LAYERS is unsupported here
    m_errorMonitor->SetDesiredError("VUID-VkImageSubresourceLayers-layerCount-09243");
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer.handle(), image_b.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &buffer_copy);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, DifferentFormatTexelBlockExtent) {
    TEST_DESCRIPTION("Copy bewteen compress images with different texel block extent.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(Init());

    VkFormat src_format = VK_FORMAT_BC3_UNORM_BLOCK;
    VkFormat dst_format = VK_FORMAT_ASTC_12x12_SRGB_BLOCK;

    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), src_format, &format_properties);
    if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0) {
        GTEST_SKIP() << "Src transfer for format is not supported";
    }
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), dst_format, &format_properties);
    if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0) {
        GTEST_SKIP() << "Dst transfer for format is not supported";
    }

    vkt::Image src_image(*m_device, 32, 32, 1, src_format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    src_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::Image dst_image(*m_device, 32, 32, 1, dst_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    dst_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageCopy region;
    region.extent = {32, 32, 1};
    region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.srcOffset = {0, 0, 0};
    region.dstOffset = {0, 0, 0};
    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-09247");
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1u, &region);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, BufferToCompressedImage) {
    TEST_DESCRIPTION("Copy buffer to compressed image when buffer is larger than image.");
    RETURN_IF_SKIP(Init());

    // Verify format support
    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_BC1_RGBA_SRGB_BLOCK, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    vkt::Buffer buffer(*m_device, 8 * 4 * 2, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageExtent = {8, 4, 1};

    vkt::Image width_image(*m_device, 5, 4, 1, VK_FORMAT_BC1_RGBA_SRGB_BLOCK, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image height_image(*m_device, 8, 3, 1, VK_FORMAT_BC1_RGBA_SRGB_BLOCK, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    if (!width_image.initialized() || (!height_image.initialized())) {
        GTEST_SKIP() << "Unable to initialize surfaces";
    }
    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-imageSubresource-07971");
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer.handle(), width_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-imageOffset-09104");
    m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyBufferToImage-imageSubresource-07970");

    VkResult err;
    VkImageCreateInfo depth_image_create_info = vku::InitStructHelper();
    depth_image_create_info.imageType = VK_IMAGE_TYPE_3D;
    depth_image_create_info.format = VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
    depth_image_create_info.extent = {8, 4, 1};
    depth_image_create_info.mipLevels = 1;
    depth_image_create_info.arrayLayers = 1;
    depth_image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    depth_image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    depth_image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VkImage depth_image = VK_NULL_HANDLE;
    err = vk::CreateImage(m_device->handle(), &depth_image_create_info, NULL, &depth_image);
    ASSERT_EQ(VK_SUCCESS, err);

    VkDeviceMemory mem1;
    VkMemoryRequirements mem_reqs;
    mem_reqs.memoryTypeBits = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;
    mem_alloc.memoryTypeIndex = 1;
    vk::GetImageMemoryRequirements(device(), depth_image, &mem_reqs);
    mem_alloc.allocationSize = mem_reqs.size;
    bool pass = m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    ASSERT_TRUE(pass);
    err = vk::AllocateMemory(device(), &mem_alloc, NULL, &mem1);
    ASSERT_EQ(VK_SUCCESS, err);
    err = vk::BindImageMemory(device(), depth_image, mem1, 0);

    region.imageExtent.depth = 2;
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer.handle(), depth_image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    vk::DestroyImage(device(), depth_image, NULL);
    vk::FreeMemory(device(), mem1, NULL);
    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, SameImage) {
    TEST_DESCRIPTION("use wrong layout copying to the same image.");
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_B8G8R8A8_UNORM,
                                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image src_image(*m_device, image_ci, vkt::set_layout);

    m_command_buffer.Begin();
    VkImageCopy copy_region;
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {2, 2, 0};
    copy_region.extent = {1, 1, 1};

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-09460");
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, src_image.handle(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCopyBufferImage, ImageRemainingArrayLayers) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 4, VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);

    VkImageCopy copy_region = {};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 2, VK_REMAINING_ARRAY_LAYERS};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 3};  // should be 2, not 3
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {4, 4, 0};
    copy_region.extent = {1, 1, 1};

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-08794");
    vk::CmdCopyImage(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &copy_region);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCopyBufferImage, ImageMemory) {
    TEST_DESCRIPTION("Validate 4 invalid image memory VUIDs ");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddOptionalExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const bool copy_commands2 = IsExtensionsEnabled(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);

    // Create a small image with a dedicated allocation
    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image_no_mem(*m_device, image_ci, vkt::no_mem);
    vkt::Image image(*m_device, image_ci);

    VkImageCopy copy_region;
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {4, 4, 1};

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-07966");
    vk::CmdCopyImage(m_command_buffer.handle(), image_no_mem.handle(), VK_IMAGE_LAYOUT_GENERAL, image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstImage-07966");
    vk::CmdCopyImage(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, image_no_mem.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    if (copy_commands2) {
        const VkImageCopy2 copy_region2 = {VK_STRUCTURE_TYPE_IMAGE_COPY_2,
                                           NULL,
                                           copy_region.srcSubresource,
                                           copy_region.srcOffset,
                                           copy_region.dstSubresource,
                                           copy_region.dstOffset,
                                           copy_region.extent};
        VkCopyImageInfo2 copy_image_info2 = {VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2,
                                             NULL,
                                             image_no_mem.handle(),
                                             VK_IMAGE_LAYOUT_GENERAL,
                                             image.handle(),
                                             VK_IMAGE_LAYOUT_GENERAL,
                                             1,
                                             &copy_region2};
        m_errorMonitor->SetDesiredError("VUID-VkCopyImageInfo2-srcImage-07966");
        m_errorMonitor->SetUnexpectedError("doesn't match the previously used layout VK_IMAGE_LAYOUT_GENERAL.");
        vk::CmdCopyImage2KHR(m_command_buffer.handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
        copy_image_info2.srcImage = image.handle();
        copy_image_info2.dstImage = image_no_mem.handle();
        m_errorMonitor->SetDesiredError("VUID-VkCopyImageInfo2-dstImage-07966");
        m_errorMonitor->SetUnexpectedError("doesn't match the previously used layout VK_IMAGE_LAYOUT_GENERAL..");
        vk::CmdCopyImage2KHR(m_command_buffer.handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeCopyBufferImage, ImageMissingUsage) {
    TEST_DESCRIPTION("Test copying from src image without VK_IMAGE_USAGE_TRANSFER_SRC_BIT.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    auto format = FindSupportedDepthStencilFormat(Gpu());
    VkImageStencilUsageCreateInfo stencil_usage_ci = vku::InitStructHelper();
    stencil_usage_ci.stencilUsage = VK_IMAGE_USAGE_SAMPLED_BIT;

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, format, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image sampled_image(*m_device, image_ci, vkt::set_layout);

    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image transfer_image(*m_device, image_ci, vkt::set_layout);

    image_ci.pNext = &stencil_usage_ci;
    image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vkt::Image separate_stencil_sampled_image(*m_device, image_ci, vkt::set_layout);

    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    stencil_usage_ci.stencilUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image separate_stencil_transfer_image(*m_device, image_ci, vkt::set_layout);

    VkImageCopy region;
    region.srcSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1};
    region.srcOffset = {0, 0, 0};
    region.dstSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1};
    region.dstOffset = {0, 0, 0};
    region.extent = {32, 32, 1};

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-aspect-06662");
    vk::CmdCopyImage(m_command_buffer.handle(), sampled_image.handle(), VK_IMAGE_LAYOUT_GENERAL, transfer_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-aspect-06663");
    vk::CmdCopyImage(m_command_buffer.handle(), transfer_image.handle(), VK_IMAGE_LAYOUT_GENERAL, sampled_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-aspect-06664");
    vk::CmdCopyImage(m_command_buffer.handle(), separate_stencil_sampled_image.handle(), VK_IMAGE_LAYOUT_GENERAL,
                     separate_stencil_transfer_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-aspect-06665");
    vk::CmdCopyImage(m_command_buffer.handle(), separate_stencil_transfer_image.handle(), VK_IMAGE_LAYOUT_GENERAL,
                     separate_stencil_sampled_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, OverlappingImage) {
    TEST_DESCRIPTION("Copy a range of an image to another overlapping range of the same image");

    RETURN_IF_SKIP(Init());

    vkt::Image image(*m_device, 64, 64, 1, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    m_command_buffer.Begin();

    VkImageCopy image_copy{};
    image_copy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_copy.srcSubresource.layerCount = 1;
    image_copy.srcOffset = {0, 0, 0};
    image_copy.dstSubresource = image_copy.srcSubresource;
    image_copy.dstOffset = {0, 0, 0};
    image_copy.extent = {64, 64, 1};
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-pRegions-00124");
    vk::CmdCopyImage(m_command_buffer.handle(), image, VK_IMAGE_LAYOUT_GENERAL, image, VK_IMAGE_LAYOUT_GENERAL, 1, &image_copy);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, MinImageTransferGranularity) {
    TEST_DESCRIPTION("Tests for validation of Queue Family property minImageTransferGranularity.");
    RETURN_IF_SKIP(Init());

    auto queue_family_properties = m_device->Physical().queue_properties_;
    auto large_granularity_family =
        std::find_if(queue_family_properties.begin(), queue_family_properties.end(), [](VkQueueFamilyProperties family_properties) {
            VkExtent3D family_granularity = family_properties.minImageTransferGranularity;
            // We need a queue family that supports copy operations and has a large enough minImageTransferGranularity for the tests
            // below to make sense.
            return (family_properties.queueFlags & VK_QUEUE_TRANSFER_BIT || family_properties.queueFlags & VK_QUEUE_GRAPHICS_BIT ||
                    family_properties.queueFlags & VK_QUEUE_COMPUTE_BIT) &&
                   family_granularity.depth >= 4 && family_granularity.width >= 4 && family_granularity.height >= 4;
        });

    if (large_granularity_family == queue_family_properties.end()) {
        GTEST_SKIP() << "No queue family has a large enough granularity for this test to be meaningful";
    }
    const size_t queue_family_index = std::distance(queue_family_properties.begin(), large_granularity_family);
    VkExtent3D granularity = queue_family_properties[queue_family_index].minImageTransferGranularity;
    vkt::CommandPool command_pool(*m_device, queue_family_index, 0);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_3D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = granularity.width * 2;
    image_create_info.extent.height = granularity.height * 2;
    image_create_info.extent.depth = granularity.depth * 2;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.flags = 0;
    vkt::Image src_image(*m_device, image_create_info, vkt::set_layout);

    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image dst_image(*m_device, image_create_info, vkt::set_layout);

    vkt::CommandBuffer command_buffer(*m_device, command_pool);
    command_buffer.Begin();

    VkImageCopy copy_region;
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = granularity;

    // Introduce failure by setting srcOffset to a bad granularity value
    copy_region.srcOffset.y = 3;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcOffset-01783");  // srcOffset image transfer granularity
    vk::CmdCopyImage(command_buffer.handle(), src_image, VK_IMAGE_LAYOUT_GENERAL, dst_image, VK_IMAGE_LAYOUT_GENERAL, 1,
                     &copy_region);
    m_errorMonitor->VerifyFound();

    // Introduce failure by setting extent to a granularity value that is bad
    // for both the source and destination image.
    copy_region.srcOffset.y = 0;
    copy_region.extent.width = 3;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcOffset-01783");  // src extent image transfer granularity
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstOffset-01784");  // dst extent image transfer granularity
    vk::CmdCopyImage(command_buffer.handle(), src_image, VK_IMAGE_LAYOUT_GENERAL, dst_image, VK_IMAGE_LAYOUT_GENERAL, 1,
                     &copy_region);
    m_errorMonitor->VerifyFound();

    // Now do some buffer/image copies
    VkDeviceSize buffer_size = 8 * granularity.height * granularity.width * granularity.depth;
    vkt::Buffer buffer(*m_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageExtent = granularity;
    region.imageOffset = {0, 0, 0};

    // Introduce failure by setting imageExtent to a bad granularity value
    region.imageExtent.width = 3;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-imageOffset-07747");  // image transfer granularity
    vk::CmdCopyImageToBuffer(command_buffer.handle(), src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer.handle(), 1, &region);
    m_errorMonitor->VerifyFound();
    region.imageExtent.width = granularity.width;

    // Introduce failure by setting imageOffset to a bad granularity value
    region.imageOffset.z = 3;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-imageOffset-07738");  // image transfer granularity
    vk::CmdCopyBufferToImage(command_buffer.handle(), buffer.handle(), dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    m_errorMonitor->VerifyFound();

    command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, Extents) {
    TEST_DESCRIPTION("Perform copies across a buffer, provoking out-of-range errors.");

    AddOptionalExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    const bool copy_commands2 = IsExtensionsEnabled(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);

    vkt::Buffer buffer_one(*m_device, 2048, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer buffer_two(*m_device, 2048, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferCopy copy_info = {4096, 256, 256};

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBuffer-srcOffset-00113");
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_one.handle(), buffer_two.handle(), 1, &copy_info);
    m_errorMonitor->VerifyFound();

    // equivalent test using KHR_copy_commands2
    if (copy_commands2) {
        const VkBufferCopy2 copy_info2 = {VK_STRUCTURE_TYPE_BUFFER_COPY_2, NULL, copy_info.srcOffset, copy_info.dstOffset,
                                          copy_info.size};
        const VkCopyBufferInfo2 copy_buffer_info2 = {
            VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2, NULL, buffer_one.handle(), buffer_two.handle(), 1, &copy_info2};
        m_errorMonitor->SetDesiredError("VUID-VkCopyBufferInfo2-srcOffset-00113");
        vk::CmdCopyBuffer2KHR(m_command_buffer.handle(), &copy_buffer_info2);
        m_errorMonitor->VerifyFound();
    }

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBuffer-dstOffset-00114");
    copy_info = {256, 4096, 256};
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_one.handle(), buffer_two.handle(), 1, &copy_info);
    m_errorMonitor->VerifyFound();

    // equivalent test using KHR_copy_commands2
    if (copy_commands2) {
        const VkBufferCopy2 copy_info2 = {VK_STRUCTURE_TYPE_BUFFER_COPY_2, NULL, copy_info.srcOffset, copy_info.dstOffset,
                                          copy_info.size};
        const VkCopyBufferInfo2 copy_buffer_info2 = {
            VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2, NULL, buffer_one.handle(), buffer_two.handle(), 1, &copy_info2};
        m_errorMonitor->SetDesiredError("VUID-VkCopyBufferInfo2-dstOffset-00114");
        vk::CmdCopyBuffer2KHR(m_command_buffer.handle(), &copy_buffer_info2);
        m_errorMonitor->VerifyFound();
    }

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBuffer-size-00115");
    copy_info = {1024, 256, 1280};
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_one.handle(), buffer_two.handle(), 1, &copy_info);
    m_errorMonitor->VerifyFound();

    // equivalent test using KHR_copy_commands2
    if (copy_commands2) {
        const VkBufferCopy2 copy_info2 = {VK_STRUCTURE_TYPE_BUFFER_COPY_2, NULL, copy_info.srcOffset, copy_info.dstOffset,
                                          copy_info.size};
        const VkCopyBufferInfo2 copy_buffer_info2 = {
            VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2_KHR, NULL, buffer_one.handle(), buffer_two.handle(), 1, &copy_info2};
        m_errorMonitor->SetDesiredError("VUID-VkCopyBufferInfo2-size-00115");
        vk::CmdCopyBuffer2KHR(m_command_buffer.handle(), &copy_buffer_info2);
        m_errorMonitor->VerifyFound();
    }

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBuffer-size-00116");
    copy_info = {256, 1024, 1280};
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_one.handle(), buffer_two.handle(), 1, &copy_info);
    m_errorMonitor->VerifyFound();

    // equivalent test using KHR_copy_commands2
    if (copy_commands2) {
        const VkBufferCopy2 copy_info2 = {VK_STRUCTURE_TYPE_BUFFER_COPY_2, NULL, copy_info.srcOffset, copy_info.dstOffset,
                                          copy_info.size};
        const VkCopyBufferInfo2 copy_buffer_info2 = {
            VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2_KHR, NULL, buffer_one.handle(), buffer_two.handle(), 1, &copy_info2};
        m_errorMonitor->SetDesiredError("VUID-VkCopyBufferInfo2-size-00116");
        vk::CmdCopyBuffer2KHR(m_command_buffer.handle(), &copy_buffer_info2);
        m_errorMonitor->VerifyFound();
    }

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBuffer-pRegions-00117");
    copy_info = {256, 512, 512};
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_two.handle(), buffer_two.handle(), 1, &copy_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkBufferCopy-size-01988");
    copy_info = {256, 256, 0};
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer_two.handle(), buffer_two.handle(), 1, &copy_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, CompletelyOverlappingBuffer) {
    TEST_DESCRIPTION("Test copying between buffers with completely overlapping source and destination regions.");
    RETURN_IF_SKIP(Init());

    VkBufferCopy copy_info;
    copy_info.srcOffset = 0;
    copy_info.dstOffset = 0;
    copy_info.size = 256;
    vkt::Buffer buffer(*m_device, copy_info.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0);

    vkt::Buffer buffer_shared_memory(*m_device, buffer.CreateInfo(), vkt::no_mem);
    buffer_shared_memory.BindMemory(buffer.Memory(), 0u);

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBuffer-pRegions-00117");
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer.handle(), buffer.handle(), 1, &copy_info);

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBuffer-pRegions-00117");
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer.handle(), buffer_shared_memory.handle(), 1, &copy_info);

    m_command_buffer.End();

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCopyBufferImage, InterleavedRegions) {
    TEST_DESCRIPTION("Test copying between interleaved source and destination regions.");
    RETURN_IF_SKIP(Init());

    VkBufferCopy copy_infos[4];
    copy_infos[0].srcOffset = 0;
    copy_infos[0].dstOffset = 4;
    copy_infos[0].size = 4;
    copy_infos[1].srcOffset = 8;
    copy_infos[1].dstOffset = 12;
    copy_infos[1].size = 4;
    copy_infos[2].srcOffset = 16;
    copy_infos[2].dstOffset = 20;
    copy_infos[2].size = 4;
    copy_infos[3].srcOffset = 24;
    copy_infos[3].dstOffset = 28;
    copy_infos[3].size = 4;

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0);

    vkt::Buffer buffer_shared_memory(*m_device, buffer.CreateInfo(), vkt::no_mem);
    buffer_shared_memory.BindMemory(buffer.Memory(), 0u);

    m_command_buffer.Begin();

    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer.handle(), buffer.handle(), 4, copy_infos);
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer.handle(), buffer_shared_memory.handle(), 4, copy_infos);

    copy_infos[2].dstOffset = 21;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBuffer-pRegions-00117");
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer.handle(), buffer.handle(), 4, copy_infos);

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBuffer-pRegions-00117");
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer.handle(), buffer_shared_memory.handle(), 4, copy_infos);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, FillBuffer) {
    TEST_DESCRIPTION("Test vkCmdFillBuffer");
    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, 32u, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer invalid_usage_buffer(*m_device, 32u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdFillBuffer-size-00026");
    vk::CmdFillBuffer(m_command_buffer.handle(), buffer.handle(), 0u, 0u, 0u);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdFillBuffer-size-00028");
    vk::CmdFillBuffer(m_command_buffer.handle(), buffer.handle(), 0u, 3u, 0u);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdFillBuffer-dstBuffer-00029");
    vk::CmdFillBuffer(m_command_buffer.handle(), invalid_usage_buffer.handle(), 0u, 4u, 0u);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdFillBuffer-dstOffset-00025");
    vk::CmdFillBuffer(m_command_buffer.handle(), buffer.handle(), 1u, 4u, 0u);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, UpdateBuffer) {
    TEST_DESCRIPTION("Test vkCmdUpdateBuffer");
    RETURN_IF_SKIP(Init());

    const uint32_t large_buffer_size = 131072u;
    vkt::Buffer large_buffer(*m_device, large_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer invalid_usage_buffer(*m_device, 32, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    std::vector<uint8_t> data(large_buffer_size);

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdUpdateBuffer-dataSize-00037");
    vk::CmdUpdateBuffer(m_command_buffer.handle(), large_buffer.handle(), 0u, large_buffer_size, data.data());
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdUpdateBuffer-dataSize-00038");
    vk::CmdUpdateBuffer(m_command_buffer.handle(), large_buffer.handle(), 0u, 5u, data.data());
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdUpdateBuffer-dstOffset-00036");
    vk::CmdUpdateBuffer(m_command_buffer.handle(), large_buffer.handle(), 1u, 4u, data.data());
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdUpdateBuffer-dstBuffer-00034");
    vk::CmdUpdateBuffer(m_command_buffer.handle(), invalid_usage_buffer.handle(), 0u, 4u, data.data());
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, CopyToBufferWithoutMemoryBound) {
    TEST_DESCRIPTION("Copy to dst buffer that has no memory bound");
    RETURN_IF_SKIP(Init());

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_ci.size = 32u;
    vkt::Buffer src_buffer(*m_device, buffer_ci);
    vkt::Buffer dst_buffer(*m_device, buffer_ci, vkt::no_mem);

    m_command_buffer.Begin();

    VkBufferCopy region;
    region.srcOffset = 0u;
    region.dstOffset = 0u;
    region.size = 32u;

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBuffer-dstBuffer-00121");
    vk::CmdCopyBuffer(m_command_buffer, src_buffer.handle(), dst_buffer.handle(), 1u, &region);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, CopyColorToDepthMaintenance8AspectMask) {
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
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_PLANE_0_BIT, 0, 0, 1};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {64, 64, 1};

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-aspectMask-00143");
    vk::CmdCopyImage(m_command_buffer.handle(), color_image, VK_IMAGE_LAYOUT_GENERAL, depth_image, VK_IMAGE_LAYOUT_GENERAL, 1,
                     &copy_region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-aspectMask-00142");
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk::CmdCopyImage(m_command_buffer.handle(), depth_image, VK_IMAGE_LAYOUT_GENERAL, color_image, VK_IMAGE_LAYOUT_GENERAL, 1,
                     &copy_region);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, CopyColorToDepthMaintenacne8DepthStencil) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::maintenance8);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    auto ds_format = FindSupportedDepthStencilFormat(Gpu());

    // Add Transfer support for all used formats
    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required VK_FORMAT_R32_SFLOAT features not supported";
    } else if (!FormatFeaturesAreSupported(Gpu(), ds_format, VK_IMAGE_TILING_OPTIMAL,
                                           VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required VK_FORMAT_D32_SFLOAT features not supported";
    }

    vkt::Image color_image(*m_device, 128, 128, 1, VK_FORMAT_R32_SFLOAT,
                           VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image ds_image(*m_device, 128, 128, 1, ds_format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    color_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    ds_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageCopy copy_region;
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0, 1};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {64, 64, 1};

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01548");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcSubresource-10214");
    vk::CmdCopyImage(m_command_buffer.handle(), color_image, VK_IMAGE_LAYOUT_GENERAL, ds_image, VK_IMAGE_LAYOUT_GENERAL, 1,
                     &copy_region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01548");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstSubresource-10215");
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk::CmdCopyImage(m_command_buffer.handle(), ds_image, VK_IMAGE_LAYOUT_GENERAL, color_image, VK_IMAGE_LAYOUT_GENERAL, 1,
                     &copy_region);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, CopyColorToDepthMaintenacne8Compatible) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::maintenance8);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Add Transfer support for all used formats
    if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required VK_FORMAT_R8G8B8A8_UNORM features not supported";
    } else if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                           VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
        GTEST_SKIP() << "Required VK_FORMAT_D32_SFLOAT features not supported";
    }

    vkt::Image color_image(*m_device, 128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM,
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
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01548");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcSubresource-10211");
    vk::CmdCopyImage(m_command_buffer.handle(), color_image, VK_IMAGE_LAYOUT_GENERAL, depth_image, VK_IMAGE_LAYOUT_GENERAL, 1,
                     &copy_region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01548");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcSubresource-10212");
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk::CmdCopyImage(m_command_buffer.handle(), depth_image, VK_IMAGE_LAYOUT_GENERAL, color_image, VK_IMAGE_LAYOUT_GENERAL, 1,
                     &copy_region);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, MissingQueueGraphicsSupport) {
    TEST_DESCRIPTION("Copy from image with depth aspect when queue does not support graphics");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance8);
    RETURN_IF_SKIP(Init());

    const std::optional<uint32_t> non_graphics_queue_family_index = m_device->QueueFamilyWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT);

    if (!non_graphics_queue_family_index) {
        GTEST_SKIP() << "No suitable queue found.";
    }

    vkt::CommandPool command_pool(*m_device, non_graphics_queue_family_index.value());
    vkt::CommandBuffer command_buffer(*m_device, command_pool);

    vkt::Image src_color_image(*m_device, 32u, 32u, 1u, VK_FORMAT_R16_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    src_color_image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    vkt::Image dst_color_image(*m_device, 32u, 32u, 1u, VK_FORMAT_R16_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    dst_color_image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    vkt::Image src_ds_image(*m_device, 32u, 32u, 1u, VK_FORMAT_D16_UNORM,
                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    src_ds_image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    vkt::Image dst_ds_image(*m_device, 32u, 32u, 1u, VK_FORMAT_D16_UNORM,
                            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    dst_ds_image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_ci.size = 32u * 32u * 2u;
    vkt::Buffer buffer(*m_device, buffer_ci);

    command_buffer.Begin();

    VkImageSubresourceLayers color_image_subresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    VkImageSubresourceLayers ds_image_subresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0u, 0u, 1u};
    VkOffset3D offset = {0, 0, 0};
    VkExtent3D extent = {32u, 32u, 1u};

    VkBufferImageCopy buffer_image_copy = {};
    buffer_image_copy.imageSubresource = ds_image_subresource;
    buffer_image_copy.imageOffset = offset;
    buffer_image_copy.imageExtent = extent;

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-commandBuffer-10216");
    vk::CmdCopyImageToBuffer(command_buffer.handle(), src_ds_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1u,
                             &buffer_image_copy);
    m_errorMonitor->VerifyFound();

    VkImageCopy image_copy;
    image_copy.srcSubresource = ds_image_subresource;
    image_copy.srcOffset = offset;
    image_copy.dstSubresource = color_image_subresource;
    image_copy.dstOffset = offset;
    image_copy.extent = extent;

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-commandBuffer-10217");
    vk::CmdCopyImage(command_buffer.handle(), src_ds_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_color_image,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &image_copy);
    m_errorMonitor->VerifyFound();

    image_copy.srcSubresource = color_image_subresource;
    image_copy.dstSubresource = ds_image_subresource;

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-commandBuffer-10218");
    vk::CmdCopyImage(command_buffer.handle(), src_color_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_ds_image,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &image_copy);
    m_errorMonitor->VerifyFound();

    command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, BufferCopy) {
    RETURN_IF_SKIP(Init());

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 32u;
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vkt::Buffer src_buffer_no_mem(*m_device, buffer_ci, vkt::no_mem);
    vkt::Buffer buffer(*m_device, 32u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer dst_buffer(*m_device, 32u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    VkBufferCopy region;
    region.srcOffset = 0u;
    region.dstOffset = 0u;
    region.size = 32u;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBuffer-srcBuffer-00119");
    vk::CmdCopyBuffer(m_command_buffer.handle(), src_buffer_no_mem.handle(), buffer.handle(), 1u, &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBuffer-dstBuffer-00120");
    vk::CmdCopyBuffer(m_command_buffer.handle(), buffer.handle(), dst_buffer.handle(), 1u, &region);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, BlitInvalidDepth) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance8);
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::ImageCreateInfo2D(32u, 32u, 1u, 4u, VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image src_image(*m_device, image_ci);
    src_image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    vkt::Image dst_image(*m_device, image_ci);
    dst_image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.arrayLayers = 1u;
    image_ci.extent.depth = 4u;
    vkt::Image src_image_3d(*m_device, image_ci);
    src_image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    vkt::Image dst_image_3d(*m_device, image_ci);
    dst_image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    m_command_buffer.Begin();

    VkImageBlit region;
    region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.srcOffsets[0] = {0, 0, 0};
    region.srcOffsets[1] = {32, 32, 1};
    region.dstOffsets[0] = {0, 0, 0};
    region.dstOffsets[1] = {32, 32, 2};

    m_errorMonitor->SetDesiredError("VUID-vkCmdBlitImage-maintenance8-10579");
    vk::CmdBlitImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_image_3d,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    region.srcOffsets[1] = {32, 32, 2};
    region.dstOffsets[1] = {32, 32, 1};
    m_errorMonitor->SetDesiredError("VUID-vkCmdBlitImage-maintenance8-10580");
    vk::CmdBlitImage(m_command_buffer.handle(), src_image_3d.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_image,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, ImageCopyMissingSrcFormatFeature) {
    AddRequiredExtensions(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkFormatProperties formatProps;
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(Gpu(), format, &formatProps);
    formatProps.optimalTilingFeatures &= ~VK_FORMAT_FEATURE_TRANSFER_SRC_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(Gpu(), format, formatProps);

    VkImageFormatProperties img_prop;
    if (VK_SUCCESS != vk::GetPhysicalDeviceImageFormatProperties(
                          m_device->Physical().handle(), format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                          VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 0u, &img_prop)) {
        GTEST_SKIP() << "Format not supported";
    }

    m_command_buffer.Begin();

    vkt::Image src_image(*m_device, 32u, 32u, 1u, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    src_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::Image dst_image(*m_device, 32u, 32u, 1u, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    dst_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::Buffer buffer(*m_device, vkt::Buffer::CreateInfo(32u * 32u * 4u, VK_IMAGE_USAGE_TRANSFER_DST_BIT));

    VkImageCopy region;
    region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.srcOffset = {0, 0, 0};
    region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.dstOffset = {0, 0, 0};
    region.extent = {32u, 32u, 1u};

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01995");
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1u, &region);
    m_errorMonitor->VerifyFound();

    VkBufferImageCopy bufferImageCopy;
    bufferImageCopy.bufferOffset = 0u;
    bufferImageCopy.bufferRowLength = 0u;
    bufferImageCopy.bufferImageHeight = 0u;
    bufferImageCopy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    bufferImageCopy.imageOffset = {0, 0, 0};
    bufferImageCopy.imageExtent = {32u, 32u, 1u};

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-srcImage-01998");
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer.handle(), 1u,
                             &bufferImageCopy);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, ImageCopyMissingDstFormatFeature) {
    AddRequiredExtensions(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkFormatProperties formatProps;
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(Gpu(), format, &formatProps);
    formatProps.optimalTilingFeatures &= ~VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(Gpu(), format, formatProps);

    VkImageFormatProperties img_prop;
    if (VK_SUCCESS != vk::GetPhysicalDeviceImageFormatProperties(
                          m_device->Physical().handle(), format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                          VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 0u, &img_prop)) {
        GTEST_SKIP() << "Format not supported";
    }

    m_command_buffer.Begin();

    vkt::Image src_image(*m_device, 32u, 32u, 1u, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    src_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::Image dst_image(*m_device, 32u, 32u, 1u, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    dst_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::Buffer buffer(*m_device,
                       vkt::Buffer::CreateInfo(32u * 32u * 4u, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT));

    VkImageCopy region;
    region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.srcOffset = {0, 0, 0};
    region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.dstOffset = {0, 0, 0};
    region.extent = {32u, 32u, 1u};

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstImage-01996");
    vk::CmdCopyImage(m_command_buffer.handle(), src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1u, &region);
    m_errorMonitor->VerifyFound();

    VkBufferImageCopy bufferImageCopy;
    bufferImageCopy.bufferOffset = 0u;
    bufferImageCopy.bufferRowLength = 0u;
    bufferImageCopy.bufferImageHeight = 0u;
    bufferImageCopy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    bufferImageCopy.imageOffset = {0, 0, 0};
    bufferImageCopy.imageExtent = {32u, 32u, 1u};

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-dstImage-01997");
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer.handle(), dst_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1u,
                             &bufferImageCopy);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, ImageCopyAspectMismatch) {
    RETURN_IF_SKIP(Init());

    vkt::Image image(*m_device, 32u, 32u, 1u, VK_FORMAT_R16_UNORM,
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::Image ds_image(*m_device, 32u, 32u, 1u, VK_FORMAT_D16_UNORM,
                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    ds_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageCopy region;
    region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.srcOffset = {0, 0, 0};
    region.dstSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0u, 0u, 1u};
    region.dstOffset = {0, 0, 0};
    region.extent = {32u, 32u, 1u};

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkImageCopy-apiVersion-07940");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01551");
    vk::CmdCopyImage(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, ds_image.handle(), VK_IMAGE_LAYOUT_GENERAL,
                     1u, &region);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, CopyToBufferWithoutMemory) {
    RETURN_IF_SKIP(Init());

    vkt::Image image(*m_device, 32u, 32u, 1u, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::Buffer buffer(*m_device, vkt::Buffer::CreateInfo(32u * 32u * 4u, VK_IMAGE_USAGE_TRANSFER_DST_BIT), vkt::no_mem);

    VkBufferImageCopy region;
    region.bufferOffset = 0u;
    region.bufferRowLength = 0u;
    region.bufferImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32u, 32u, 1u};

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-dstBuffer-00192");
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer.handle(), 1u, &region);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, ImageCopyInvalidLayout) {
    RETURN_IF_SKIP(Init());

    vkt::Image image(*m_device, 32u, 32u, 1u, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    vkt::Buffer buffer(*m_device,
                       vkt::Buffer::CreateInfo(32u * 32u * 4u, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT));

    VkBufferImageCopy region;
    region.bufferOffset = 0u;
    region.bufferRowLength = 0u;
    region.bufferImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32u, 32u, 1u};

    m_command_buffer.Begin();
    image.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-srcImageLayout-00189");
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer.handle(), 1u,
                             &region);
    m_errorMonitor->VerifyFound();

    image.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-srcImageLayout-01397");
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, buffer.handle(), 1u,
                             &region);
    m_errorMonitor->VerifyFound();

    image.SetLayout(m_command_buffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyBufferToImage-dstImageLayout-01396");
    vk::CmdCopyBufferToImage(m_command_buffer.handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1u,
                             &region);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, CopySrcImageMissingTransferBit) {
    RETURN_IF_SKIP(Init());

    vkt::Image image(*m_device, 32u, 32u, 1u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::Buffer buffer(*m_device, vkt::Buffer::CreateInfo(32u * 32u * 4u, VK_IMAGE_USAGE_TRANSFER_DST_BIT));

    VkBufferImageCopy region;
    region.bufferOffset = 0u;
    region.bufferRowLength = 0u;
    region.bufferImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32u, 32u, 1u};

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToBuffer-srcImage-00186");
    vk::CmdCopyImageToBuffer(m_command_buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer.handle(), 1u, &region);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, BlitImage) {
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    vkt::Image image_no_mem(*m_device, image_ci, vkt::no_mem);
    vkt::Image image(*m_device, image_ci);
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image image_no_transfer_src(*m_device, image_ci);

    m_command_buffer.Begin();

    VkImageBlit region;
    region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.srcOffsets[0] = {0u, 0u, 0u};
    region.srcOffsets[1] = {32u, 32u, 1u};
    region.dstOffsets[0] = {0u, 0u, 0u};
    region.dstOffsets[1] = {32u, 32u, 1u};

    m_errorMonitor->SetDesiredError("VUID-vkCmdBlitImage-srcImage-00220");
    vk::CmdBlitImage(m_command_buffer.handle(), image_no_mem, VK_IMAGE_LAYOUT_GENERAL, image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1u,
                     &region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBlitImage-srcImage-00219");
    vk::CmdBlitImage(m_command_buffer.handle(), image_no_transfer_src, VK_IMAGE_LAYOUT_GENERAL, image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1u,
                     &region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBlitImage-dstImage-00225");
    vk::CmdBlitImage(m_command_buffer.handle(), image, VK_IMAGE_LAYOUT_GENERAL, image_no_mem.handle(), VK_IMAGE_LAYOUT_GENERAL, 1u,
                     &region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    region.srcOffsets[0] = {0u, 0u, 0u};
    region.srcOffsets[1] = {1u, 1u, 1u};
    region.dstOffsets[0] = {1u, 1u, 0u};
    region.dstOffsets[1] = {1u, 1u, 1u};

    m_errorMonitor->SetDesiredError("VUID-vkCmdBlitImage-srcImage-09459");
    vk::CmdBlitImage(m_command_buffer.handle(), image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image.handle(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u,
                     &region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, BlitSubsampledBit) {
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::fragmentDensityMap);
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    vkt::Image image1(*m_device, image_ci);
    image_ci.flags = VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT;
    vkt::Image image2(*m_device, image_ci);

    m_command_buffer.Begin();

    VkImageBlit region;
    region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.srcOffsets[0] = {0u, 0u, 0u};
    region.srcOffsets[1] = {32u, 32u, 1u};
    region.dstOffsets[0] = {0u, 0u, 0u};
    region.dstOffsets[1] = {32u, 32u, 1u};

    m_errorMonitor->SetDesiredError("VUID-vkCmdBlitImage-dstImage-02545");
    vk::CmdBlitImage(m_command_buffer.handle(), image1.handle(), VK_IMAGE_LAYOUT_GENERAL, image2.handle(), VK_IMAGE_LAYOUT_GENERAL, 1u,
                     &region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, BlitDepthImage) {
    RETURN_IF_SKIP(Init());

    VkFormatProperties props;
    vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), VK_FORMAT_D16_UNORM, &props);
    if ((props.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT) == 0) {
        GTEST_SKIP() << "VK_FORMAT_D16_UNORM blit dst not supported";
    }

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_D16_UNORM,
                                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    vkt::Image image1(*m_device, image_ci);
    image1.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::Image image2(*m_device, image_ci);
    image2.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    m_command_buffer.Begin();

    VkImageBlit region;
    region.srcSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0u, 0u, 1u};
    region.dstSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0u, 0u, 1u};
    region.srcOffsets[0] = {0u, 0u, 0u};
    region.srcOffsets[1] = {32u, 32u, 1u};
    region.dstOffsets[0] = {0u, 0u, 0u};
    region.dstOffsets[1] = {32u, 32u, 1u};

    m_errorMonitor->SetDesiredError("VUID-vkCmdBlitImage-srcImage-00232");
    vk::CmdBlitImage(m_command_buffer.handle(), image1, VK_IMAGE_LAYOUT_GENERAL, image2.handle(), VK_IMAGE_LAYOUT_GENERAL, 1u,
                     &region, VK_FILTER_LINEAR);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, ResolveImage) {
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::Image image_no_mem(*m_device, image_ci, vkt::no_mem);
    image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
    vkt::Image ms_image(*m_device, image_ci);
    vkt::Image ms_image_no_mem(*m_device, image_ci, vkt::no_mem);

    m_command_buffer.Begin();

    VkImageResolve region;
    region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.srcOffset = {0, 0, 0};
    region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.dstOffset = {0, 0, 0};
    region.extent = {1u, 1u, 1u};

    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-srcImage-00256");
    vk::CmdResolveImage(m_command_buffer.handle(), ms_image_no_mem.handle(), VK_IMAGE_LAYOUT_GENERAL, image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1u, &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-srcImageLayout-01400");
    vk::CmdResolveImage(m_command_buffer.handle(), ms_image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, image.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1u, &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-dstImage-00258");
    vk::CmdResolveImage(m_command_buffer.handle(), ms_image.handle(), VK_IMAGE_LAYOUT_GENERAL, image_no_mem.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1u, &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-dstImageLayout-01401");
    vk::CmdResolveImage(m_command_buffer.handle(), ms_image.handle(), VK_IMAGE_LAYOUT_GENERAL, image.handle(),
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1u, &region);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeCopyBufferImage, ResolveSubsampledImage) {
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::fragmentDensityMap);
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    vkt::Image image_no_mem(*m_device, image_ci, vkt::no_mem);
    image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
    image_ci.flags = VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT;
    vkt::Image ms_image(*m_device, image_ci);

    m_command_buffer.Begin();

    VkImageResolve region;
    region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.srcOffset = {0, 0, 0};
    region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.dstOffset = {0, 0, 0};
    region.extent = {1u, 1u, 1u};

    m_errorMonitor->SetDesiredError("VUID-vkCmdResolveImage-dstImage-02546");
    vk::CmdResolveImage(m_command_buffer.handle(), ms_image.handle(), VK_IMAGE_LAYOUT_GENERAL, image.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1u, &region);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeCopyBufferImage, ResolveImageRemainingLayers) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::ImageCreateInfo2D(32u, 32u, 1u, 2u, VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    vkt::Image image(*m_device, image_ci);
    image_ci.arrayLayers = 1u;
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
    vkt::Image ms_image(*m_device, image_ci);
    ms_image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    m_command_buffer.Begin();

    VkImageResolve region;
    region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.srcOffset = {0, 0, 0};
    region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, VK_REMAINING_ARRAY_LAYERS};
    region.dstOffset = {0, 0, 0};
    region.extent = {1u, 1u, 1u};

    m_errorMonitor->SetDesiredError("VUID-VkImageResolve-layerCount-08804");
    vk::CmdResolveImage(m_command_buffer.handle(), ms_image.handle(), VK_IMAGE_LAYOUT_GENERAL, image.handle(),
                        VK_IMAGE_LAYOUT_GENERAL, 1u, &region);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}
