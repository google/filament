/*
 * Copyright (c) 2023-2025 The Khronos Group Inc.
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
 * Copyright (c) 2023-2025 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"

class NegativeHostImageCopy : public HostImageCopyTest {};

TEST_F(NegativeHostImageCopy, ImageLayout) {
    TEST_DESCRIPTION("Bad Image Layout");
    RETURN_IF_SKIP(InitHostImageCopyTest());

    VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);

    std::vector<uint8_t> pixels(width * height * 4);

    VkMemoryToImageCopy region_to_image = vku::InitStructHelper();
    region_to_image.pHostPointer = pixels.data();
    region_to_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_to_image.imageExtent = {width, height, 1};

    VkCopyMemoryToImageInfo copy_to_image = vku::InitStructHelper();
    copy_to_image.dstImage = image;
    copy_to_image.dstImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    copy_to_image.regionCount = 1;
    copy_to_image.pRegions = &region_to_image;

    VkImageToMemoryCopy region_from_image = vku::InitStructHelper();
    region_from_image.pHostPointer = pixels.data();
    region_from_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_from_image.imageExtent = {width, height, 1};

    VkCopyImageToMemoryInfo copy_from_image = vku::InitStructHelper();
    copy_from_image.srcImage = image;
    copy_from_image.srcImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    copy_from_image.regionCount = 1;
    copy_from_image.pRegions = &region_from_image;

    // Bad image layout
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-dstImageLayout-09059");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-srcImageLayout-09064");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, TransferUsageFlag) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);

    // Missing transfer usage
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    vkt::Image image_no_transfer(*m_device, image_ci);
    image_no_transfer.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    std::vector<uint8_t> pixels(width * height * 4);

    VkMemoryToImageCopy region_to_image = vku::InitStructHelper();
    region_to_image.pHostPointer = pixels.data();
    region_to_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_to_image.imageExtent = {width, height, 1};

    VkCopyMemoryToImageInfo copy_to_image = vku::InitStructHelper();
    copy_to_image.dstImage = image;
    copy_to_image.dstImageLayout = layout;
    copy_to_image.regionCount = 1;
    copy_to_image.pRegions = &region_to_image;

    VkImageToMemoryCopy region_from_image = vku::InitStructHelper();
    region_from_image.pHostPointer = pixels.data();
    region_from_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_from_image.imageExtent = {width, height, 1};

    VkCopyImageToMemoryInfo copy_from_image = vku::InitStructHelper();
    copy_from_image.srcImage = image;
    copy_from_image.srcImageLayout = layout;
    copy_from_image.regionCount = 1;
    copy_from_image.pRegions = &region_from_image;

    copy_to_image.dstImage = image_no_transfer;
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-dstImage-09113");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();

    copy_from_image.srcImage = image_no_transfer;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-srcImage-09113");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, ImageOffset) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);

    std::vector<uint8_t> pixels(width * height * 4);

    VkMemoryToImageCopy region_to_image = vku::InitStructHelper();
    region_to_image.pHostPointer = pixels.data();
    region_to_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_to_image.imageExtent = {width, height, 1};

    VkCopyMemoryToImageInfo copy_to_image = vku::InitStructHelper();
    copy_to_image.dstImage = image;
    copy_to_image.dstImageLayout = layout;
    copy_to_image.regionCount = 1;
    copy_to_image.pRegions = &region_to_image;

    VkImageToMemoryCopy region_from_image = vku::InitStructHelper();
    region_from_image.pHostPointer = pixels.data();
    region_from_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_from_image.imageExtent = {width, height, 1};

    VkCopyImageToMemoryInfo copy_from_image = vku::InitStructHelper();
    copy_from_image.srcImage = image;
    copy_from_image.srcImageLayout = layout;
    copy_from_image.regionCount = 1;
    copy_from_image.pRegions = &region_from_image;

    // Memcpy with imageOffset x, y, or z == 0
    copy_to_image.flags = VK_HOST_IMAGE_COPY_MEMCPY;
    region_to_image.imageOffset.x = 1;
    // If ImageExtent.width is left at width, offset will exceed width of image (07971). Setting it to width-1 will not match
    // image dimensions (09115). Pick the one with MEMCPY flag set and test for both here.
    region_to_image.imageExtent.width = width - 1;
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-imageOffset-09114");
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-dstImage-09115");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();

    copy_from_image.flags = VK_HOST_IMAGE_COPY_MEMCPY;
    region_from_image.imageOffset.x = 1;
    region_from_image.imageExtent.width = width - 1;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-imageOffset-09114");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-srcImage-09115");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, AspectMask) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);

    std::vector<uint8_t> pixels(width * height * 4);

    VkMemoryToImageCopy region_to_image = vku::InitStructHelper();
    region_to_image.pHostPointer = pixels.data();
    region_to_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_to_image.imageExtent = {width, height, 1};

    VkCopyMemoryToImageInfo copy_to_image = vku::InitStructHelper();
    copy_to_image.dstImage = image;
    copy_to_image.dstImageLayout = layout;
    copy_to_image.regionCount = 1;
    copy_to_image.pRegions = &region_to_image;

    VkImageToMemoryCopy region_from_image = vku::InitStructHelper();
    region_from_image.pHostPointer = pixels.data();
    region_from_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_from_image.imageExtent = {width, height, 1};

    VkCopyImageToMemoryInfo copy_from_image = vku::InitStructHelper();
    copy_from_image.srcImage = image;
    copy_from_image.srcImageLayout = layout;
    copy_from_image.regionCount = 1;
    copy_from_image.pRegions = &region_from_image;

    // Bad aspectMask
    region_to_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-imageSubresource-09105");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-imageSubresource-09105");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, CopyImageToFromMemoryNoMemory) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);

    std::vector<uint8_t> pixels(width * height * 4);

    VkMemoryToImageCopy region_to_image = vku::InitStructHelper();
    region_to_image.pHostPointer = pixels.data();
    region_to_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_to_image.imageExtent = {width, height, 1};

    VkCopyMemoryToImageInfo copy_to_image = vku::InitStructHelper();
    copy_to_image.dstImage = image;
    copy_to_image.dstImageLayout = layout;
    copy_to_image.regionCount = 1;
    copy_to_image.pRegions = &region_to_image;

    VkImageToMemoryCopy region_from_image = vku::InitStructHelper();
    region_from_image.pHostPointer = pixels.data();
    region_from_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_from_image.imageExtent = {width, height, 1};

    VkCopyImageToMemoryInfo copy_from_image = vku::InitStructHelper();
    copy_from_image.srcImage = image;
    copy_from_image.srcImageLayout = layout;
    copy_from_image.regionCount = 1;
    copy_from_image.pRegions = &region_from_image;

    vkt::Image image_no_mem(*m_device, image_ci, vkt::no_mem);
    copy_to_image.dstImage = image_no_mem;
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-dstImage-07966");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();

    copy_from_image.srcImage = image_no_mem;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-srcImage-07966");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, ImageSubresource) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);

    std::vector<uint8_t> pixels(width * height * 4);

    VkMemoryToImageCopy region_to_image = vku::InitStructHelper();
    region_to_image.pHostPointer = pixels.data();
    region_to_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_to_image.imageExtent = {width, height, 1};

    VkCopyMemoryToImageInfo copy_to_image = vku::InitStructHelper();
    copy_to_image.dstImage = image;
    copy_to_image.dstImageLayout = layout;
    copy_to_image.regionCount = 1;
    copy_to_image.pRegions = &region_to_image;

    VkImageToMemoryCopy region_from_image = vku::InitStructHelper();
    region_from_image.pHostPointer = pixels.data();
    region_from_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_from_image.imageExtent = {width, height, 1};

    VkCopyImageToMemoryInfo copy_from_image = vku::InitStructHelper();
    copy_from_image.srcImage = image;
    copy_from_image.srcImageLayout = layout;
    copy_from_image.regionCount = 1;
    copy_from_image.pRegions = &region_from_image;

    // Bad mipLevel - also throws off multiple size calculations, causing other errors
    // Also get 07970 - pRegions must be contained within the specified dstSubresource of dstImage
    // Also get 07971 - imageOffset.x and (imageExtent.width + imageOffset.x) both >= 0 and <= imageSubresource width
    // Also get 07972 - imageOffset.y and (imageExtent.height + imageOffset.y) both >= 0 and <= imageSubresource height
    // Also get 09104 - imageOffset.z and (imageExtent.depth + imageOffset.z) both >= 0 and <= imageSubresource depth
    region_to_image.imageSubresource.mipLevel = image_ci.mipLevels + 1;
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyMemoryToImageInfo-imageSubresource-07970");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyMemoryToImageInfo-imageSubresource-07971");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyMemoryToImageInfo-imageSubresource-07972");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyMemoryToImageInfo-imageOffset-09104");
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-imageSubresource-07967");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();

    region_from_image.imageSubresource.mipLevel = image_ci.mipLevels + 1;
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToMemoryInfo-imageSubresource-07970");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToMemoryInfo-imageSubresource-07971");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToMemoryInfo-imageSubresource-07972");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToMemoryInfo-imageOffset-09104");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-imageSubresource-07967");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
    region_to_image.imageSubresource.mipLevel = 0;
    region_from_image.imageSubresource.mipLevel = 0;

    // baseArrayLayer + layerCount > arrayLayers
    region_to_image.imageSubresource.baseArrayLayer = image_ci.arrayLayers;
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-imageSubresource-07968");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.imageSubresource.baseArrayLayer = image_ci.arrayLayers;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-imageSubresource-07968");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
    region_to_image.imageSubresource.baseArrayLayer = 0;
    region_from_image.imageSubresource.baseArrayLayer = 0;

    // Extent bigger than image - Can't get 07970 without getting 07971, 07972 or 09104, so test both 07971 and 07970 here
    region_to_image.imageExtent.width = width + 1;
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-imageSubresource-07971");
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-imageSubresource-07970");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.imageExtent.width = width + 1;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-imageSubresource-07971");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-imageSubresource-07970");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
    region_to_image.imageExtent.width = width;
    region_from_image.imageExtent.width = width;

    // imageOffset.y and (imageExtent.height + imageOffset.y) both >= 0 and <= imageSubresource height
    region_to_image.imageOffset.y = -1;
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-imageSubresource-07972");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyMemoryToImageInfo-imageSubresource-07970");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.imageOffset.y = -1;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-imageSubresource-07972");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToMemoryInfo-imageSubresource-07970");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, ImageExtent) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);

    std::vector<uint8_t> pixels(width * height * 4);

    VkMemoryToImageCopy region_to_image = vku::InitStructHelper();
    region_to_image.pHostPointer = pixels.data();
    region_to_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_to_image.imageExtent = {width, height, 1};

    VkCopyMemoryToImageInfo copy_to_image = vku::InitStructHelper();
    copy_to_image.dstImage = image;
    copy_to_image.dstImageLayout = layout;
    copy_to_image.regionCount = 1;
    copy_to_image.pRegions = &region_to_image;

    VkImageToMemoryCopy region_from_image = vku::InitStructHelper();
    region_from_image.pHostPointer = pixels.data();
    region_from_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_from_image.imageExtent = {width, height, 1};

    VkCopyImageToMemoryInfo copy_from_image = vku::InitStructHelper();
    copy_from_image.srcImage = image;
    copy_from_image.srcImageLayout = layout;
    copy_from_image.regionCount = 1;
    copy_from_image.pRegions = &region_from_image;

    // Use 3D image to avoid 07980
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    vkt::Image image_3d(*m_device, image_ci);
    image_3d.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // imageOffset.z and (imageExtent.depth + imageOffset.z) both >= 0 and <= imageSubresource height
    copy_to_image.dstImage = image_3d;
    region_to_image.imageOffset.z = 1;
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-imageOffset-09104");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyMemoryToImageInfo-imageSubresource-07970");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.imageOffset.z = 1;
    copy_from_image.srcImage = image_3d;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-imageOffset-09104");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToMemoryInfo-imageSubresource-07970");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
    region_to_image.imageOffset.z = 0;
    region_from_image.imageOffset.z = 0;

    // imageSubresource.baseArrayLayer must be 0 and imageSubresource.layerCount must be 1
    region_to_image.imageSubresource.baseArrayLayer = 1;
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-dstImage-07983");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyMemoryToImageInfo-imageSubresource-07968");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.imageSubresource.baseArrayLayer = 1;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-srcImage-07983");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToMemoryInfo-imageSubresource-07968");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
    region_to_image.imageSubresource.baseArrayLayer = 0;
    region_from_image.imageSubresource.baseArrayLayer = 0;

    // imageExtent.depth must not be 0
    region_to_image.imageExtent.depth = 0;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryToImageCopy-imageExtent-06661");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.imageExtent.depth = 0;
    m_errorMonitor->SetDesiredError("VUID-VkImageToMemoryCopy-imageExtent-06661");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, Image1D) {
    TEST_DESCRIPTION("Use VK_EXT_host_image_copy to copy from images to memory and vice versa");
    RETURN_IF_SKIP(InitHostImageCopyTest());

    VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);

    std::vector<uint8_t> pixels(width * height * 4);

    VkMemoryToImageCopy region_to_image = vku::InitStructHelper();
    region_to_image.pHostPointer = pixels.data();
    region_to_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_to_image.imageExtent = {width, height, 1};

    VkCopyMemoryToImageInfo copy_to_image = vku::InitStructHelper();
    copy_to_image.dstImage = image;
    copy_to_image.dstImageLayout = layout;
    copy_to_image.regionCount = 1;
    copy_to_image.pRegions = &region_to_image;

    VkImageToMemoryCopy region_from_image = vku::InitStructHelper();
    region_from_image.pHostPointer = pixels.data();
    region_from_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_from_image.imageExtent = {width, height, 1};

    VkCopyImageToMemoryInfo copy_from_image = vku::InitStructHelper();
    copy_from_image.srcImage = image;
    copy_from_image.srcImageLayout = layout;
    copy_from_image.regionCount = 1;
    copy_from_image.pRegions = &region_from_image;

    // Image type 1D
    // imageOffset.y must be 0 and imageExtent.height must be 1
    image_ci.imageType = VK_IMAGE_TYPE_1D;
    image_ci.extent.height = 1;
    vkt::Image image_1d(*m_device, image_ci, vkt::set_layout);
    image_1d.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.extent.height = height;
    copy_to_image.dstImage = image_1d;
    region_to_image.imageOffset.y = 1;
    region_to_image.imageExtent.height = 1;
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-dstImage-07979");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyMemoryToImageInfo-imageSubresource-07972");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyMemoryToImageInfo-imageSubresource-07970");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    copy_from_image.srcImage = image_1d;
    region_from_image.imageOffset.y = 1;
    region_from_image.imageExtent.height = 1;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-srcImage-07979");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToMemoryInfo-imageSubresource-07972");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToMemoryInfo-imageSubresource-07970");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
    region_to_image.imageOffset.y = 0;
    region_from_image.imageOffset.y = 0;

    // imageOffset.z must be 0 and imageExtent.depth must be 1
    region_to_image.imageOffset.z = 1;
    region_to_image.imageExtent.depth = 1;
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-dstImage-07980");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyMemoryToImageInfo-imageOffset-09104");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyMemoryToImageInfo-imageSubresource-07970");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.imageOffset.z = 1;
    region_from_image.imageExtent.depth = 1;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-srcImage-07980");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToMemoryInfo-imageOffset-09104");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToMemoryInfo-imageSubresource-07970");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, Image1DMultiSampled) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    const VkPhysicalDeviceLimits &dev_limits = m_device->Physical().limits_;
    if ((dev_limits.sampledImageColorSampleCounts & VK_SAMPLE_COUNT_2_BIT) == 0) {
        GTEST_SKIP() << "VK_SAMPLE_COUNT_2_BIT not supported";
    }

    VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);

    image_ci.samples = VK_SAMPLE_COUNT_2_BIT;
    vkt::Image image_samplecount(*m_device, image_ci);
    image_samplecount.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    std::vector<uint8_t> pixels(width * height * 4);

    VkMemoryToImageCopy region_to_image = vku::InitStructHelper();
    region_to_image.pHostPointer = pixels.data();
    region_to_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_to_image.imageExtent = {width, height, 1};

    VkCopyMemoryToImageInfo copy_to_image = vku::InitStructHelper();
    copy_to_image.dstImage = image;
    copy_to_image.dstImageLayout = layout;
    copy_to_image.regionCount = 1;
    copy_to_image.pRegions = &region_to_image;

    VkImageToMemoryCopy region_from_image = vku::InitStructHelper();
    region_from_image.pHostPointer = pixels.data();
    region_from_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_from_image.imageExtent = {width, height, 1};

    VkCopyImageToMemoryInfo copy_from_image = vku::InitStructHelper();
    copy_from_image.srcImage = image;
    copy_from_image.srcImageLayout = layout;
    copy_from_image.regionCount = 1;
    copy_from_image.pRegions = &region_from_image;

    // Can't use sampled image
    copy_to_image.dstImage = image_samplecount;
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-dstImage-07973");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();

    copy_from_image.srcImage = image_samplecount;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-srcImage-07973");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, CompressedFormat) {
    TEST_DESCRIPTION("Use VK_EXT_host_image_copy to copy from images to memory and vice versa");
    AddRequiredFeature(vkt::Feature::textureCompressionBC);
    RETURN_IF_SKIP(InitHostImageCopyTest());

    VkImageFormatProperties img_prop = {};
    if (VK_SUCCESS != vk::GetPhysicalDeviceImageFormatProperties(m_device->Physical().handle(), VK_FORMAT_BC3_SRGB_BLOCK,
                                                                 image_ci.imageType, image_ci.tiling, image_ci.usage,
                                                                 image_ci.flags, &img_prop)) {
        GTEST_SKIP() << "Image format properties not supported";
    }

    VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);

    std::vector<uint8_t> pixels(width * height * 4);

    VkMemoryToImageCopy region_to_image = vku::InitStructHelper();
    region_to_image.pHostPointer = pixels.data();
    region_to_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_to_image.imageExtent = {width, height, 1};

    VkCopyMemoryToImageInfo copy_to_image = vku::InitStructHelper();
    copy_to_image.dstImage = image;
    copy_to_image.dstImageLayout = layout;
    copy_to_image.regionCount = 1;
    copy_to_image.pRegions = &region_to_image;

    VkImageToMemoryCopy region_from_image = vku::InitStructHelper();
    region_from_image.pHostPointer = pixels.data();
    region_from_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_from_image.imageExtent = {width, height, 1};

    VkCopyImageToMemoryInfo copy_from_image = vku::InitStructHelper();
    copy_from_image.srcImage = image;
    copy_from_image.srcImageLayout = layout;
    copy_from_image.regionCount = 1;
    copy_from_image.pRegions = &region_from_image;

    image_ci.format = VK_FORMAT_BC3_SRGB_BLOCK;
    image_ci.mipLevels = 6;
    vkt::Image image_compressed(*m_device, image_ci, vkt::set_layout);
    image_compressed.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // imageOffset not a multiple of block size
    region_to_image.imageOffset = {1, 1, 0};
    region_to_image.imageExtent = {1, 1, 1};
    region_to_image.imageSubresource.mipLevel = 4;
    copy_to_image.dstImage = image_compressed;
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-dstImage-07274");
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-dstImage-07275");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.imageOffset = {1, 1, 0};
    region_from_image.imageExtent = {1, 1, 1};
    region_from_image.imageSubresource.mipLevel = 4;
    copy_from_image.srcImage = image_compressed;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-srcImage-07274");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-srcImage-07275");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
    region_to_image.imageOffset = {0, 0, 0};
    region_from_image.imageOffset = {0, 0, 0};

    // width not a multiple of compressed block width
    region_to_image.imageExtent = {1, 2, 1};
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-dstImage-00207");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.imageExtent = {1, 2, 1};
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-srcImage-00207");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();

    // Copy height < compressed block size but not the full mip height
    region_to_image.imageExtent = {2, 1, 1};
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-dstImage-00208");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.imageExtent = {2, 1, 1};
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-srcImage-00208");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();

    region_to_image.imageSubresource.mipLevel = 0;
    region_from_image.imageSubresource.mipLevel = 0;
    region_to_image.imageExtent = {width, height, 1};
    region_from_image.imageExtent = {width, height, 1};

    // memoryRowLength not a multiple of block width (4)
    region_to_image.memoryRowLength = 130;
    region_to_image.memoryImageHeight = 0;
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-memoryRowLength-09106");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.memoryRowLength = 130;
    region_from_image.memoryImageHeight = 0;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-memoryRowLength-09106");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();

    // memoryImageHeight not a multiple of block height (4)
    region_to_image.memoryRowLength = 0;
    region_to_image.memoryImageHeight = 130;
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-memoryImageHeight-09107");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.memoryRowLength = 0;
    region_from_image.memoryImageHeight = 130;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-memoryImageHeight-09107");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();

    region_to_image.memoryImageHeight = 0;
    region_from_image.memoryImageHeight = 0;

    // memoryRowLength divided by the texel block extent width and then multiplied by the texel block size of the image must
    // be less than or equal to 2^31-1
    region_to_image.memoryRowLength = 0x20000000;
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-memoryRowLength-09108");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.memoryRowLength = 0x20000000;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-memoryRowLength-09108");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, DepthStencil) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    if (!CopyLayoutSupported(copy_src_layouts, copy_dst_layouts, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)) {
        GTEST_SKIP() << "Depth Stencil layout not supported";
    }

    auto stencil_format = FindSupportedDepthStencilFormat(Gpu());
    if (!(m_device->FormatFeaturesOptimal(stencil_format) & VK_FORMAT_FEATURE_2_HOST_IMAGE_TRANSFER_BIT)) {
        GTEST_SKIP() << "Device does not support host image on depth format";
    }

    VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);

    std::vector<uint8_t> pixels(width * height * 4);

    VkMemoryToImageCopy region_to_image = vku::InitStructHelper();
    region_to_image.pHostPointer = pixels.data();
    region_to_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_to_image.imageExtent = {width, height, 1};

    VkCopyMemoryToImageInfo copy_to_image = vku::InitStructHelper();
    copy_to_image.dstImage = image;
    copy_to_image.dstImageLayout = layout;
    copy_to_image.regionCount = 1;
    copy_to_image.pRegions = &region_to_image;

    VkImageToMemoryCopy region_from_image = vku::InitStructHelper();
    region_from_image.pHostPointer = pixels.data();
    region_from_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_from_image.imageExtent = {width, height, 1};

    VkCopyImageToMemoryInfo copy_from_image = vku::InitStructHelper();
    copy_from_image.srcImage = image;
    copy_from_image.srcImageLayout = layout;
    copy_from_image.regionCount = 1;
    copy_from_image.pRegions = &region_from_image;

    image_ci.format = stencil_format;
    image_ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    vkt::Image image_stencil(*m_device, image_ci, vkt::set_layout);
    image_stencil.SetLayout((VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT),
                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

    // Stencil, no VK_IMAGE_USAGE_HOST_TRANSFER_BIT
    region_to_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    copy_to_image.dstImage = image_stencil;
    copy_to_image.dstImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-dstImage-09111");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    copy_from_image.srcImage = image_stencil;
    copy_from_image.srcImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-srcImage-09111");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();

    VkImageStencilUsageCreateInfo stencil_usage_ci = vku::InitStructHelper();
    stencil_usage_ci.stencilUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    image_ci.pNext = &stencil_usage_ci;
    vkt::Image image_separate_stencil(*m_device, image_ci, vkt::set_layout);
    image_separate_stencil.SetLayout((VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT),
                                     VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

    // Seperate stencil, no VK_IMAGE_USAGE_HOST_TRANSFER_BIT
    region_to_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    copy_to_image.dstImage = image_separate_stencil;
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-dstImage-09112");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    copy_from_image.srcImage = image_separate_stencil;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-srcImage-09112");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();

    VkImageFormatProperties img_prop = {};
    if (VK_SUCCESS == vk::GetPhysicalDeviceImageFormatProperties(
                          m_device->Physical().handle(), stencil_format, image_ci.imageType, image_ci.tiling,
                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_HOST_TRANSFER_BIT, image_ci.flags,
                          &img_prop)) {
        // The aspectMask member of imageSubresource must only have a single bit set
        image_ci.format = stencil_format;
        image_ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_HOST_TRANSFER_BIT;
        image_ci.pNext = nullptr;
        vkt::Image image_stencil2(*m_device, image_ci, vkt::set_layout);
        image_stencil2.SetLayout((VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT),
                                 VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
        copy_to_image.dstImage = image_stencil2;
        region_to_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        m_errorMonitor->SetDesiredError("VUID-VkMemoryToImageCopy-aspectMask-09103");
        vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
        m_errorMonitor->VerifyFound();
        copy_from_image.srcImage = image_stencil2;
        region_from_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        m_errorMonitor->SetDesiredError("VUID-VkImageToMemoryCopy-aspectMask-09103");
        vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeHostImageCopy, MultiPlanar) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);

    std::vector<uint8_t> pixels(width * height * 4);

    VkMemoryToImageCopy region_to_image = vku::InitStructHelper();
    region_to_image.pHostPointer = pixels.data();
    region_to_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_to_image.imageExtent = {width, height, 1};

    VkCopyMemoryToImageInfo copy_to_image = vku::InitStructHelper();
    copy_to_image.dstImage = image;
    copy_to_image.dstImageLayout = layout;
    copy_to_image.regionCount = 1;
    copy_to_image.pRegions = &region_to_image;

    VkImageToMemoryCopy region_from_image = vku::InitStructHelper();
    region_from_image.pHostPointer = pixels.data();
    region_from_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_from_image.imageExtent = {width, height, 1};

    VkCopyImageToMemoryInfo copy_from_image = vku::InitStructHelper();
    copy_from_image.srcImage = image;
    copy_from_image.srcImageLayout = layout;
    copy_from_image.regionCount = 1;
    copy_from_image.pRegions = &region_from_image;

    VkImageFormatProperties img_prop = {};
    if (VK_SUCCESS == vk::GetPhysicalDeviceImageFormatProperties(
                          m_device->Physical().handle(), VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, VK_IMAGE_TYPE_2D,
                          VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_HOST_TRANSFER_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0,
                          &img_prop)) {
        // imageSubresource.aspectMask must be VK_IMAGE_ASPECT_PLANE_0_BIT or VK_IMAGE_ASPECT_PLANE_1_BIT
        vkt::Image image_multi_planar2(*m_device, 128, 128, 1, VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
                                       VK_IMAGE_USAGE_HOST_TRANSFER_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        image_multi_planar2.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);
        region_to_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT;
        copy_to_image.dstImage = image_multi_planar2;
        m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-dstImage-07981");
        vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
        m_errorMonitor->VerifyFound();
        region_from_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT;
        copy_from_image.srcImage = image_multi_planar2;
        m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-srcImage-07981");
        vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
        m_errorMonitor->VerifyFound();
    }

    if (VK_SUCCESS == vk::GetPhysicalDeviceImageFormatProperties(
                          m_device->Physical().handle(), VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, VK_IMAGE_TYPE_2D,
                          VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_HOST_TRANSFER_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0,
                          &img_prop)) {
        // imageSubresource.aspectMask must be VK_IMAGE_ASPECT_PLANE_0_BIT, VK_IMAGE_ASPECT_PLANE_1_BIT, or
        // VK_IMAGE_ASPECT_PLANE_2_BIT
        vkt::Image image_multi_planar3(*m_device, 128, 128, 1, VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
                                       VK_IMAGE_USAGE_HOST_TRANSFER_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        image_multi_planar3.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);
        region_to_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_to_image.dstImage = image_multi_planar3;
        m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-dstImage-07981");
        vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
        m_errorMonitor->VerifyFound();
        region_from_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_from_image.srcImage = image_multi_planar3;
        m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-srcImage-07981");
        vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeHostImageCopy, NonSupportedLayout) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    if (CopyLayoutSupported(copy_src_layouts, copy_dst_layouts, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)) {
        GTEST_SKIP() << "Need Transfer src layout to not be supported";
    }

    VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);

    std::vector<uint8_t> pixels(width * height * 4);

    VkMemoryToImageCopy region_to_image = vku::InitStructHelper();
    region_to_image.pHostPointer = pixels.data();
    region_to_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_to_image.imageExtent = {width, height, 1};

    VkCopyMemoryToImageInfo copy_to_image = vku::InitStructHelper();
    copy_to_image.dstImage = image;
    copy_to_image.dstImageLayout = layout;
    copy_to_image.regionCount = 1;
    copy_to_image.pRegions = &region_to_image;

    VkImageToMemoryCopy region_from_image = vku::InitStructHelper();
    region_from_image.pHostPointer = pixels.data();
    region_from_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_from_image.imageExtent = {width, height, 1};

    VkCopyImageToMemoryInfo copy_from_image = vku::InitStructHelper();
    copy_from_image.srcImage = image;
    copy_from_image.srcImageLayout = layout;
    copy_from_image.regionCount = 1;
    copy_from_image.pRegions = &region_from_image;

    // layout must be one of the image layouts returned in VkPhysicalDeviceHostImageCopyPropertiesEXT::pCopySrcLayouts
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    copy_to_image.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-dstImageLayout-09060");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    copy_from_image.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-srcImageLayout-09065");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
    copy_to_image.dstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_from_image.srcImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

TEST_F(NegativeHostImageCopy, ImageExtent2) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);

    std::vector<uint8_t> pixels(width * height * 4);

    VkMemoryToImageCopy region_to_image = vku::InitStructHelper();
    region_to_image.pHostPointer = pixels.data();
    region_to_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_to_image.imageExtent = {width, height, 1};

    VkCopyMemoryToImageInfo copy_to_image = vku::InitStructHelper();
    copy_to_image.dstImage = image;
    copy_to_image.dstImageLayout = layout;
    copy_to_image.regionCount = 1;
    copy_to_image.pRegions = &region_to_image;

    VkImageToMemoryCopy region_from_image = vku::InitStructHelper();
    region_from_image.pHostPointer = pixels.data();
    region_from_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_from_image.imageExtent = {width, height, 1};

    VkCopyImageToMemoryInfo copy_from_image = vku::InitStructHelper();
    copy_from_image.srcImage = image;
    copy_from_image.srcImageLayout = layout;
    copy_from_image.regionCount = 1;
    copy_from_image.pRegions = &region_from_image;

    // memoryRowLength must be 0, or greater than or equal to the width member of imageExtent
    region_to_image.memoryRowLength = region_to_image.imageExtent.width - 4;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryToImageCopy-memoryRowLength-09101");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.memoryRowLength = region_from_image.imageExtent.width - 4;
    m_errorMonitor->SetDesiredError("VUID-VkImageToMemoryCopy-memoryRowLength-09101");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
    region_to_image.memoryRowLength = 0;
    region_from_image.memoryRowLength = 0;

    // memoryImageHeight must be 0, or greater than or equal to the height member of imageExtent
    region_to_image.memoryImageHeight = region_to_image.imageExtent.height - 4;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryToImageCopy-memoryImageHeight-09102");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.memoryImageHeight = region_from_image.imageExtent.height - 4;
    m_errorMonitor->SetDesiredError("VUID-VkImageToMemoryCopy-memoryImageHeight-09102");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
    region_to_image.memoryImageHeight = 0;
    region_from_image.memoryImageHeight = 0;

    // imageExtent.width must not be 0
    region_to_image.imageExtent.width = 0;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryToImageCopy-imageExtent-06659");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.imageExtent.width = 0;
    m_errorMonitor->SetDesiredError("VUID-VkImageToMemoryCopy-imageExtent-06659");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
    region_to_image.imageExtent.width = width;
    region_from_image.imageExtent.width = width;

    // imageExtent.height must not be 0
    region_to_image.imageExtent.height = 0;
    m_errorMonitor->SetDesiredError("VUID-VkMemoryToImageCopy-imageExtent-06660");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.imageExtent.height = 0;
    m_errorMonitor->SetDesiredError("VUID-VkImageToMemoryCopy-imageExtent-06660");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
    region_to_image.imageExtent.height = height;
    region_from_image.imageExtent.height = height;

    // When VK_HOST_IMAGE_COPY_MEMCPY is in flags, memoryRowLength and memoryImageHeight must be zero
    region_to_image.memoryRowLength = width;
    copy_to_image.flags = VK_HOST_IMAGE_COPY_MEMCPY;
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-flags-09393");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();
    region_from_image.memoryImageHeight = height;
    copy_from_image.flags = VK_HOST_IMAGE_COPY_MEMCPY;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-flags-09394");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, CopyImageToImageFeature) {
    RETURN_IF_SKIP(InitHostImageCopyTest());
    VkFormat no_hic_feature_format =
        FindFormatWithoutFeatures2(Gpu(), image_ci.tiling, VK_FORMAT_FEATURE_2_HOST_IMAGE_TRANSFER_BIT);
    if (no_hic_feature_format == VK_FORMAT_UNDEFINED) {
        GTEST_SKIP() << "Can't find format with no support";
    }

    // If VK_IMAGE_USAGE_HOST_TRANSFER_BIT, then format features must have VK_FORMAT_FEATURE_2_HOST_IMAGE_TRANSFER_BIT
    image_ci.format = no_hic_feature_format;
    // Can't use vkt::Image because it does error checking
    VkImage image_no_feature;
    // Any invalid usage will get 02251
    m_errorMonitor->SetUnexpectedError("VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251");
    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-imageCreateFormatFeatures-09048");
    vk::CreateImage(device(), &image_ci, NULL, &image_no_feature);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, CopyImageToImageIdenticalCreateInfo) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    if (!CopyLayoutSupported(copy_src_layouts, copy_dst_layouts, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)) {
        GTEST_SKIP() << "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL not supported";
    }

    vkt::Image image1(*m_device, image_ci, vkt::set_layout);
    // srcImage and dstImage must have been created with identical image creation parameters
    image_ci.extent.width = image_ci.extent.width / 2;
    vkt::Image image2(*m_device, image_ci, vkt::set_layout);

    VkImageCopy2 image_copy_2 = vku::InitStructHelper();
    image_copy_2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.extent = {image_ci.extent.width, height, 1};

    VkCopyImageToImageInfo copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy_2;
    copy_image_to_image.srcImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImage = image1;
    copy_image_to_image.srcImage = image2;

    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcImage-09069");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, CopyImageToImageUsageFlagsStencil) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    if (!CopyLayoutSupported(copy_src_layouts, copy_dst_layouts, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)) {
        GTEST_SKIP() << "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL not supported";
    }
    VkImageCopy2 image_copy_2 = vku::InitStructHelper();
    image_copy_2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.extent = {width, height, 1};
    VkCopyImageToImageInfo copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy_2;

    // Note that because the images need to be identical to avoid 09069, we'll go ahead and test for src and dst errors in one call

    auto stencil_format = FindSupportedDepthStencilFormat(Gpu());
    image_ci.format = stencil_format;
    image_ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    vkt::Image image_stencil1(*m_device, image_ci, vkt::set_layout);
    image_stencil1.SetLayout((VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT),
                             VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
    vkt::Image image_stencil2(*m_device, image_ci, vkt::set_layout);
    image_stencil2.SetLayout((VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT),
                             VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

    // Stencil, no VK_IMAGE_USAGE_HOST_TRANSFER_BIT
    image_copy_2.srcSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    copy_image_to_image.dstImage = image_stencil1;
    copy_image_to_image.dstImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    image_copy_2.dstSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    copy_image_to_image.srcImage = image_stencil2;
    copy_image_to_image.srcImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstImage-09111");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcImage-09111");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();

    // Seperate stencil, no VK_IMAGE_USAGE_HOST_TRANSFER_BIT
    VkImageStencilUsageCreateInfo stencil_usage_ci = vku::InitStructHelper();
    stencil_usage_ci.stencilUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    image_ci.pNext = &stencil_usage_ci;

    vkt::Image image_separate_stencil1(*m_device, image_ci, vkt::set_layout);
    image_separate_stencil1.SetLayout((VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT),
                                      VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
    vkt::Image image_separate_stencil2(*m_device, image_ci, vkt::set_layout);
    image_separate_stencil2.SetLayout((VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT),
                                      VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

    image_copy_2.dstSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    copy_image_to_image.dstImage = image_separate_stencil1;
    image_copy_2.srcSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    copy_image_to_image.srcImage = image_separate_stencil2;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstImage-09112");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcImage-09112");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, CopyImageToImageUsageFlagsNonStencil) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    auto image_ci_no_transfer = vkt::Image::ImageCreateInfo2D(width, height, 1, 1, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    // Missing transfer usage
    vkt::Image image1(*m_device, image_ci_no_transfer, vkt::set_layout);
    vkt::Image image2(*m_device, image_ci_no_transfer, vkt::set_layout);
    image1.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    image2.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    VkImageCopy2 image_copy_2 = vku::InitStructHelper();
    image_copy_2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.extent = {width, height, 1};

    VkCopyImageToImageInfo copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy_2;
    copy_image_to_image.srcImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImage = image1;
    copy_image_to_image.srcImage = image2;

    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstImage-09113");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcImage-09113");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, CopyImageToImageOffset) {
    RETURN_IF_SKIP(InitHostImageCopyTest());
    vkt::Image image1(*m_device, image_ci, vkt::set_layout);
    vkt::Image image2(*m_device, image_ci, vkt::set_layout);

    VkImageCopy2 image_copy_2 = vku::InitStructHelper();
    image_copy_2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.dstOffset.x = 1;
    image_copy_2.srcOffset.x = 1;
    // If ImageExtent.width is left at width, offset will exceed width of image (07971). Setting it to width-1 will not match
    // image dimensions (09115). Pick the one with MEMCPY flag set (09115) and test for both here.
    image_copy_2.extent = {width = 1, height, 1};

    VkCopyImageToImageInfo copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy_2;
    copy_image_to_image.srcImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImage = image1;
    copy_image_to_image.srcImage = image2;
    // Memcpy with imageOffset x, y, or z == 0
    copy_image_to_image.flags = VK_HOST_IMAGE_COPY_MEMCPY;

    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstOffset-09114");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcOffset-09114");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcImage-09115");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstImage-09115");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, CopyImageToImageNoMemory) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    vkt::Image image1(*m_device, image_ci, vkt::no_mem);
    vkt::Image image2(*m_device, image_ci, vkt::no_mem);

    VkImageCopy2 image_copy_2 = vku::InitStructHelper();
    image_copy_2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.extent = {width, height, 1};

    VkCopyImageToImageInfo copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy_2;
    copy_image_to_image.srcImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImage = image1;
    copy_image_to_image.srcImage = image2;

    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstImage-07966");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcImage-07966");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, CopyImageToImageMipLevels) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    vkt::Image image1(*m_device, image_ci, vkt::set_layout);
    vkt::Image image2(*m_device, image_ci, vkt::set_layout);

    VkImageCopy2 image_copy_2 = vku::InitStructHelper();
    image_copy_2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.extent = {width, height, 1};
    // Bad mipLevel - also throws off multiple size calculations, causing other errors
    // Also get 07970 - pRegions must be contained within the specified dstSubresource of dstImage
    // Also get 07971 - imageOffset.x and (imageExtent.width + imageOffset.x) both >= 0 and <= imageSubresource width
    // Also get 07972 - imageOffset.y and (imageExtent.height + imageOffset.y) both >= 0 and <= imageSubresource height
    // Also get 09104 - imageOffset.z and (imageExtent.depth + imageOffset.z) both >= 0 and <= imageSubresource depth
    image_copy_2.dstSubresource.mipLevel = image_ci.mipLevels + 1;
    image_copy_2.srcSubresource.mipLevel = image_ci.mipLevels + 1;

    VkCopyImageToImageInfo copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy_2;
    copy_image_to_image.srcImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImage = image1;
    copy_image_to_image.srcImage = image2;

    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-srcSubresource-07970");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-srcSubresource-07971");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-srcSubresource-07972");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-srcOffset-09104");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-dstSubresource-07970");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-dstSubresource-07971");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-dstSubresource-07972");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-dstOffset-09104");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcSubresource-07967");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstSubresource-07967");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, CopyImageToImageLayers) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    vkt::Image image1(*m_device, image_ci, vkt::set_layout);
    vkt::Image image2(*m_device, image_ci, vkt::set_layout);

    VkImageCopy2 image_copy_2 = vku::InitStructHelper();
    image_copy_2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.extent = {width, height, 1};
    // baseArrayLayer + layerCount > arrayLayers
    image_copy_2.srcSubresource.baseArrayLayer = image_ci.arrayLayers;
    image_copy_2.dstSubresource.baseArrayLayer = image_ci.arrayLayers;

    VkCopyImageToImageInfo copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy_2;
    copy_image_to_image.srcImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImage = image1;
    copy_image_to_image.srcImage = image2;

    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcSubresource-07968");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstSubresource-07968");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, CopyImageToImageLargeExtent) {
    TEST_DESCRIPTION("Use VK_EXT_host_image_copy to copy from an image to another image");
    RETURN_IF_SKIP(InitHostImageCopyTest());

    vkt::Image image1(*m_device, image_ci, vkt::set_layout);
    vkt::Image image2(*m_device, image_ci, vkt::set_layout);

    VkImageCopy2 image_copy_2 = vku::InitStructHelper();
    image_copy_2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    // Extent bigger than image - Can't get 07970 without getting 07971, 07972 or 09104, so test both 07971 and 07970 here
    image_copy_2.extent = {width + 1, height, 1};
    VkCopyImageToImageInfo copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy_2;
    copy_image_to_image.srcImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImage = image1;
    copy_image_to_image.srcImage = image2;

    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcSubresource-07970");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcSubresource-07971");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstSubresource-07970");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstSubresource-07971");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, CopyImageToImageOffsetSmall) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    vkt::Image image1(*m_device, image_ci, vkt::set_layout);
    vkt::Image image2(*m_device, image_ci, vkt::set_layout);

    VkImageCopy2 image_copy_2 = vku::InitStructHelper();
    image_copy_2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.extent = {width, height, 1};
    // imageOffset.y and (imageExtent.height + imageOffset.y) both >= 0 and <= imageSubresource height
    // Also get 07970 - pRegions must be contained within the specified dstSubresource of dstImage
    image_copy_2.srcOffset.y = -1;
    image_copy_2.dstOffset.y = -1;

    VkCopyImageToImageInfo copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy_2;
    copy_image_to_image.srcImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImage = image1;
    copy_image_to_image.srcImage = image2;

    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcSubresource-07972");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstSubresource-07972");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-srcSubresource-07970");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-dstSubresource-07970");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, CopyImageToImagDepth) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    // Use 3D image to avoid 07980
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    vkt::Image image1(*m_device, image_ci, vkt::set_layout);
    vkt::Image image2(*m_device, image_ci, vkt::set_layout);

    VkImageCopy2 image_copy_2 = vku::InitStructHelper();
    image_copy_2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.extent = {width, height, 1};
    // imageOffset.z and (imageExtent.depth + imageOffset.z) both >= 0 and <= imageSubresource height
    // Also get 07970 - pRegions must be contained within the specified dstSubresource of dstImage
    image_copy_2.srcOffset.z = 1;
    image_copy_2.dstOffset.z = 1;

    VkCopyImageToImageInfo copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy_2;
    copy_image_to_image.srcImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImage = image1;
    copy_image_to_image.srcImage = image2;

    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstOffset-09104");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcOffset-09104");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-srcSubresource-07970");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-dstSubresource-07970");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, CopyImageToImagDepthZero) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    // Use 3D image to avoid 07980
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    vkt::Image image1(*m_device, image_ci, vkt::set_layout);
    vkt::Image image2(*m_device, image_ci, vkt::set_layout);

    VkImageCopy2 image_copy_2 = vku::InitStructHelper();
    image_copy_2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    // extent.depth must not be 0
    image_copy_2.extent = {width, height, 0};

    VkCopyImageToImageInfo copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy_2;
    copy_image_to_image.srcImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImage = image1;
    copy_image_to_image.srcImage = image2;

    m_errorMonitor->SetDesiredError("VUID-VkImageCopy2-extent-06670");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, CopyImageToImagDepthLayer) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    // Use 3D image to avoid 07980
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    vkt::Image image1(*m_device, image_ci, vkt::set_layout);
    vkt::Image image2(*m_device, image_ci, vkt::set_layout);

    VkImageCopy2 image_copy_2 = vku::InitStructHelper();
    // 3D image, Subresource.baseArrayLayer must be 0 and Subresource.layerCount must be 1
    // Also get Subresource.baseArrayLayer + Subresource.layerCount <= CreateInfo.arrayLayers
    image_copy_2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1};
    image_copy_2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1};
    image_copy_2.extent = {width, height, 1};

    VkCopyImageToImageInfo copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy_2;
    copy_image_to_image.srcImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImage = image1;
    copy_image_to_image.srcImage = image2;

    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcImage-07983");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstImage-07983");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-srcSubresource-07968");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-dstSubresource-07968");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, CopyImageToImag1D) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    image_ci.imageType = VK_IMAGE_TYPE_1D;
    image_ci.extent.height = 1;
    vkt::Image image1(*m_device, image_ci, vkt::set_layout);
    vkt::Image image2(*m_device, image_ci, vkt::set_layout);

    VkImageCopy2 image_copy_2 = vku::InitStructHelper();
    image_copy_2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    // Offset.y must be 0 and extent.height must be 1
    image_copy_2.extent = {width, 1, 1};
    image_copy_2.srcOffset.y = 1;
    image_copy_2.dstOffset.y = 1;

    VkCopyImageToImageInfo copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy_2;
    copy_image_to_image.srcImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImage = image1;
    copy_image_to_image.srcImage = image2;

    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcImage-07979");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstImage-07979");
    // Also get 07970 - pRegions must be contained within the specified subresource of image
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-srcSubresource-07970");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-dstSubresource-07970");
    // Also get 07972 - offset.y and (extent.height + offset.y) both >= 0 and <= image subresource height
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-srcSubresource-07972");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-dstSubresource-07972");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, CopyImageToImag1DDpeth) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    image_ci.imageType = VK_IMAGE_TYPE_1D;
    image_ci.extent.height = 1;
    vkt::Image image1(*m_device, image_ci, vkt::set_layout);
    vkt::Image image2(*m_device, image_ci, vkt::set_layout);

    VkImageCopy2 image_copy_2 = vku::InitStructHelper();
    image_copy_2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    // imageOffset.z must be 0 and imageExtent.depth must be 1
    image_copy_2.extent = {width, 1, 1};
    image_copy_2.srcOffset.z = 1;
    image_copy_2.dstOffset.z = 1;

    VkCopyImageToImageInfo copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy_2;
    copy_image_to_image.srcImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImage = image1;
    copy_image_to_image.srcImage = image2;

    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstImage-07980");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcImage-07980");
    // Also get 07970 - pRegions must be contained within the specified subresource of image
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-srcSubresource-07970");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-dstSubresource-07970");
    // Also get 09104 - imageOffset.z and (imageExtent.depth + imageOffset.z) both >= 0 and <= imageSubresource depth
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-srcOffset-09104");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-dstOffset-09104");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, CopyImageToImageAspectMask) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    vkt::Image image1(*m_device, image_ci, vkt::set_layout);
    vkt::Image image2(*m_device, image_ci, vkt::set_layout);

    VkImageCopy2 image_copy_2 = vku::InitStructHelper();
    // Bad aspectMask
    image_copy_2.srcSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    image_copy_2.dstSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    image_copy_2.extent = {width, height, 1};
    VkCopyImageToImageInfo copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy_2;
    copy_image_to_image.srcImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImage = image1;
    copy_image_to_image.srcImage = image2;

    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcSubresource-09105");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstSubresource-09105");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, CopyImageToImageAspectMask2) {
    TEST_DESCRIPTION("Use VK_EXT_host_image_copy to copy from an image to another image");
    RETURN_IF_SKIP(InitHostImageCopyTest());

    vkt::Image image1(*m_device, image_ci, vkt::set_layout);
    vkt::Image image2(*m_device, image_ci, vkt::set_layout);

    VkImageCopy2 image_copy_2 = vku::InitStructHelper();
    image_copy_2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.extent = {width, height, 1};
    VkCopyImageToImageInfo copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy_2;
    copy_image_to_image.srcImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImage = image1;
    copy_image_to_image.srcImage = image2;

    // Multiple aspect mask bits
    image_copy_2.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;
    image_copy_2.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkImageSubresourceLayers-aspectMask-00167");
    // Also get aspect not present in image
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-srcSubresource-09105");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
    image_copy_2.extent = {width, height, 1};

    // Can't include METADATA
    image_copy_2.srcSubresource.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkImageSubresourceLayers-aspectMask-00168");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();

    // No aspect plane bits
    image_copy_2.srcSubresource.aspectMask = VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageSubresourceLayers-aspectMask-parameter");
    m_errorMonitor->SetDesiredError("VUID-VkImageSubresourceLayers-aspectMask-02247");
    // Also get aspect not present in image
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-srcSubresource-09105");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, CopyImageToImagePlanar) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    VkImageFormatProperties img_prop = {};
    VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkImageCopy2 image_copy_2 = vku::InitStructHelper();
    image_copy_2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.extent = {width, height, 1};
    VkCopyImageToImageInfo copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy_2;
    copy_image_to_image.srcImageLayout = layout;
    copy_image_to_image.dstImageLayout = layout;

    if (VK_SUCCESS == vk::GetPhysicalDeviceImageFormatProperties(
                          m_device->Physical().handle(), VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, VK_IMAGE_TYPE_2D,
                          VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_HOST_TRANSFER_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0,
                          &img_prop)) {
        // imageSubresource.aspectMask must be VK_IMAGE_ASPECT_PLANE_0_BIT or VK_IMAGE_ASPECT_PLANE_1_BIT
        vkt::Image image_multi_twoplane1(*m_device, 128, 128, 1, VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
                                         VK_IMAGE_USAGE_HOST_TRANSFER_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        vkt::Image image_multi_twoplane2(*m_device, 128, 128, 1, VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
                                         VK_IMAGE_USAGE_HOST_TRANSFER_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        image_multi_twoplane1.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);
        image_multi_twoplane2.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);
        image_copy_2.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT;
        image_copy_2.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT;
        copy_image_to_image.dstImage = image_multi_twoplane1;
        copy_image_to_image.srcImage = image_multi_twoplane2;
        m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcImage-07981");
        m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstImage-07981");
        vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
        m_errorMonitor->VerifyFound();
    }

    if (VK_SUCCESS == vk::GetPhysicalDeviceImageFormatProperties(
                          m_device->Physical().handle(), VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, VK_IMAGE_TYPE_2D,
                          VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_HOST_TRANSFER_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0,
                          &img_prop)) {
        // imageSubresource.aspectMask must be VK_IMAGE_ASPECT_PLANE_0_BIT, VK_IMAGE_ASPECT_PLANE_1_BIT, or
        // VK_IMAGE_ASPECT_PLANE_2_BIT
        vkt::Image image_multi_threeplane1(*m_device, 128, 128, 1, VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
                                           VK_IMAGE_USAGE_HOST_TRANSFER_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        vkt::Image image_multi_threeplane2(*m_device, 128, 128, 1, VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
                                           VK_IMAGE_USAGE_HOST_TRANSFER_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        image_multi_threeplane1.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);
        image_multi_threeplane2.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);
        image_copy_2.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_copy_2.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_image_to_image.dstImage = image_multi_threeplane1;
        copy_image_to_image.srcImage = image_multi_threeplane2;
        m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcImage-07981");
        m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstImage-07981");
        vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeHostImageCopy, CopyImageToImageLayout) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    vkt::Image image1(*m_device, image_ci, vkt::set_layout);
    vkt::Image image2(*m_device, image_ci, vkt::set_layout);

    VkImageCopy2 image_copy_2 = vku::InitStructHelper();
    image_copy_2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.extent = {width, height, 1};
    VkCopyImageToImageInfo copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy_2;
    copy_image_to_image.srcImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    copy_image_to_image.dstImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    copy_image_to_image.dstImage = image1;
    copy_image_to_image.srcImage = image2;

    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcImageLayout-09070");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstImageLayout-09071");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
}
TEST_F(NegativeHostImageCopy, CopyImageToImageProperties) {
    RETURN_IF_SKIP(InitHostImageCopyTest());
    if (CopyLayoutSupported(copy_src_layouts, copy_dst_layouts, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)) {
        GTEST_SKIP() << "Properties supported";
    }

    vkt::Image image1(*m_device, image_ci, vkt::set_layout);
    vkt::Image image2(*m_device, image_ci, vkt::set_layout);
    // layout must be one of the image layouts returned in VkPhysicalDeviceHostImageCopyPropertiesEXT::pCopySrcLayouts
    image1.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    image2.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    VkImageCopy2 image_copy_2 = vku::InitStructHelper();
    image_copy_2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.extent = {width, height, 1};
    VkCopyImageToImageInfo copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy_2;
    copy_image_to_image.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    copy_image_to_image.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    copy_image_to_image.dstImage = image1;
    copy_image_to_image.srcImage = image2;

    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcImageLayout-09072");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstImageLayout-09073");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, CopyImageToImageZeroExtent) {
    TEST_DESCRIPTION("Use VK_EXT_host_image_copy to copy from an image to another image");
    RETURN_IF_SKIP(InitHostImageCopyTest());

    vkt::Image image1(*m_device, image_ci, vkt::set_layout);
    vkt::Image image2(*m_device, image_ci, vkt::set_layout);

    VkImageCopy2 image_copy_2 = vku::InitStructHelper();
    image_copy_2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    // Neither width not height can be zero
    image_copy_2.extent = {0, 0, 1};
    VkCopyImageToImageInfo copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy_2;
    copy_image_to_image.srcImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImage = image1;
    copy_image_to_image.srcImage = image2;

    m_errorMonitor->SetDesiredError("VUID-VkImageCopy2-extent-06668");
    m_errorMonitor->SetDesiredError("VUID-VkImageCopy2-extent-06669");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, CopyImageToImageZeroLayer) {
    TEST_DESCRIPTION("Use VK_EXT_host_image_copy to copy from an image to another image");
    RETURN_IF_SKIP(InitHostImageCopyTest());

    vkt::Image image1(*m_device, image_ci, vkt::set_layout);
    vkt::Image image2(*m_device, image_ci, vkt::set_layout);

    VkImageCopy2 image_copy_2 = vku::InitStructHelper();
    image_copy_2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 0};
    image_copy_2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.extent = {width, height, 1};
    VkCopyImageToImageInfo copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy_2;
    copy_image_to_image.srcImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImage = image1;
    copy_image_to_image.srcImage = image2;

    m_errorMonitor->SetDesiredError("VUID-VkImageSubresourceLayers-layerCount-01700");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, CopyImageToImageCompressed) {
    AddRequiredFeature(vkt::Feature::textureCompressionBC);
    RETURN_IF_SKIP(InitHostImageCopyTest());

    VkImageFormatProperties img_prop = {};
    if (VK_SUCCESS != vk::GetPhysicalDeviceImageFormatProperties(m_device->Physical().handle(), VK_FORMAT_BC3_SRGB_BLOCK,
                                                                 image_ci.imageType, image_ci.tiling, image_ci.usage,
                                                                 image_ci.flags, &img_prop)) {
        GTEST_SKIP() << "Image format properties not supported";
    }

    image_ci.format = VK_FORMAT_BC3_SRGB_BLOCK;
    image_ci.mipLevels = 6;
    vkt::Image image_compressed1(*m_device, image_ci, vkt::set_layout);
    vkt::Image image_compressed2(*m_device, image_ci, vkt::set_layout);
    image_compressed1.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    image_compressed2.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // imageOffset not a multiple of block size
    VkImageCopy2 image_copy_2 = vku::InitStructHelper();
    image_copy_2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 4, 0, 1};
    image_copy_2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 4, 0, 1};
    image_copy_2.dstOffset = {1, 1, 0};
    image_copy_2.srcOffset = {1, 1, 0};
    image_copy_2.extent = {1, 1, 1};

    VkCopyImageToImageInfo copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy_2;
    copy_image_to_image.srcImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    copy_image_to_image.dstImage = image_compressed1;
    copy_image_to_image.srcImage = image_compressed2;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcImage-07274");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstImage-07274");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcImage-07275");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstImage-07275");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
    image_copy_2.dstOffset = {0, 0, 0};
    image_copy_2.srcOffset = {0, 0, 0};

    // width not a multiple of compressed block width
    image_copy_2.extent = {1, 2, 1};
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcImage-00207");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstImage-00207");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();

    // Copy height < compressed block size but not the full mip height
    image_copy_2.extent = {2, 1, 1};
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcImage-00208");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstImage-00208");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
    image_copy_2.extent = {width, height, 1};
    image_copy_2.srcSubresource.mipLevel = 0;
    image_copy_2.dstSubresource.mipLevel = 0;
}

TEST_F(NegativeHostImageCopy, CopyImageToFromMemorySubsampled) {
    TEST_DESCRIPTION("Use VK_EXT_fragment_density_map with VK_EXT_host_image_copy");
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    RETURN_IF_SKIP(InitHostImageCopyTest());

    VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);

    std::vector<uint8_t> pixels(width * height * 4);

    VkMemoryToImageCopy region_to_image = vku::InitStructHelper();
    region_to_image.pHostPointer = pixels.data();
    region_to_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_to_image.imageExtent = {width, height, 1};

    VkCopyMemoryToImageInfo copy_to_image = vku::InitStructHelper();
    copy_to_image.dstImage = image;
    copy_to_image.dstImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    copy_to_image.regionCount = 1;
    copy_to_image.pRegions = &region_to_image;

    VkImageToMemoryCopy region_from_image = vku::InitStructHelper();
    region_from_image.pHostPointer = pixels.data();
    region_from_image.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region_from_image.imageExtent = {width, height, 1};

    VkCopyImageToMemoryInfo copy_from_image = vku::InitStructHelper();
    copy_from_image.srcImage = image;
    copy_from_image.srcImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    copy_from_image.regionCount = 1;
    copy_from_image.pRegions = &region_from_image;

    image_ci.flags = VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT;
    vkt::Image image_subsampled(*m_device, image_ci, vkt::set_layout);
    image_subsampled.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    copy_to_image.dstImage = image_subsampled;
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-dstImage-07969");
    vk::CopyMemoryToImageEXT(*m_device, &copy_to_image);
    m_errorMonitor->VerifyFound();

    copy_from_image.srcImage = image_subsampled;
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-srcImage-07969");
    vk::CopyImageToMemoryEXT(*m_device, &copy_from_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, CopyImageToImageSubsampled) {
    TEST_DESCRIPTION("Use VK_EXT_fragment_density_map with VK_EXT_host_image_copy");
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    RETURN_IF_SKIP(InitHostImageCopyTest());

    VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkImageCopy2 image_copy_2 = vku::InitStructHelper();
    image_copy_2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy_2.extent = {width, height, 1};

    image_ci.flags = VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT;

    vkt::Image image_subsampled1(*m_device, image_ci, vkt::set_layout);
    vkt::Image image_subsampled2(*m_device, image_ci, vkt::set_layout);
    image_subsampled1.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    image_subsampled2.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    VkCopyImageToImageInfo copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy_2;
    copy_image_to_image.srcImageLayout = layout;
    copy_image_to_image.dstImageLayout = layout;
    copy_image_to_image.dstImage = image_subsampled1;
    copy_image_to_image.srcImage = image_subsampled2;

    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcImage-07969");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstImage-07969");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, TransitionImageLayout) {
    TEST_DESCRIPTION("Use VK_EXT_host_image_copy to transition image layouts");
    RETURN_IF_SKIP(InitHostImageCopyTest());
    vkt::Image image(*m_device, image_ci, vkt::set_layout);

    VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkHostImageLayoutTransitionInfo transition_info = vku::InitStructHelper();
    transition_info.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    transition_info.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    transition_info.subresourceRange = range;
    transition_info.image = image;

    // Bad baseMipLevel
    transition_info.subresourceRange.baseMipLevel = 1;
    transition_info.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    m_errorMonitor->SetDesiredError("VUID-VkHostImageLayoutTransitionInfo-subresourceRange-01486");
    vk::TransitionImageLayoutEXT(*m_device, 1, &transition_info);
    m_errorMonitor->VerifyFound();
    transition_info.subresourceRange.baseMipLevel = 0;

    // Bad baseMipLevel + levelCount
    transition_info.subresourceRange.levelCount = 2;
    m_errorMonitor->SetDesiredError("VUID-VkHostImageLayoutTransitionInfo-subresourceRange-01724");
    vk::TransitionImageLayoutEXT(*m_device, 1, &transition_info);
    m_errorMonitor->VerifyFound();
    transition_info.subresourceRange.levelCount = 1;

    // Bad baseArrayLayer
    // Also has to get baseArrayLayer + layerCount > arrayLayers, so test both
    transition_info.subresourceRange.baseArrayLayer = 1;
    m_errorMonitor->SetDesiredError("VUID-VkHostImageLayoutTransitionInfo-subresourceRange-01725");
    m_errorMonitor->SetDesiredError("VUID-VkHostImageLayoutTransitionInfo-subresourceRange-01488");
    vk::TransitionImageLayoutEXT(*m_device, 1, &transition_info);
    m_errorMonitor->VerifyFound();
    transition_info.subresourceRange.baseArrayLayer = 0;

    // Bad aspectMask
    transition_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkHostImageLayoutTransitionInfo-image-09241");
    vk::TransitionImageLayoutEXT(*m_device, 1, &transition_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, TransitionImageLayout2) {
    RETURN_IF_SKIP(InitHostImageCopyTest());
    vkt::Image image(*m_device, image_ci, vkt::set_layout);

    VkHostImageLayoutTransitionInfo transition_info = vku::InitStructHelper();
    transition_info.oldLayout = VK_IMAGE_LAYOUT_GENERAL;  // wrong
    transition_info.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    transition_info.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    transition_info.image = image;

    m_errorMonitor->SetDesiredError("VUID-VkHostImageLayoutTransitionInfo-oldLayout-09229");
    vk::TransitionImageLayoutEXT(*m_device, 1, &transition_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, TransitionImageLayoutNoMemory) {
    RETURN_IF_SKIP(InitHostImageCopyTest());
    vkt::Image image(*m_device, image_ci, vkt::no_mem);

    VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkHostImageLayoutTransitionInfo transition_info = vku::InitStructHelper();
    transition_info.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    transition_info.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    transition_info.subresourceRange = range;
    transition_info.image = image;

    m_errorMonitor->SetDesiredError("VUID-VkHostImageLayoutTransitionInfo-image-01932");
    vk::TransitionImageLayoutEXT(*m_device, 1, &transition_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, TransitionImageLayoutUsage) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkHostImageLayoutTransitionInfo transition_info = vku::InitStructHelper();
    transition_info.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    transition_info.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    transition_info.subresourceRange = range;

    auto image_ci_no_transfer = vkt::Image::ImageCreateInfo2D(width, height, 1, 1, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    // Missing transfer usage
    vkt::Image image_no_transfer(*m_device, image_ci_no_transfer, vkt::set_layout);
    image_no_transfer.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    transition_info.image = image_no_transfer;
    m_errorMonitor->SetDesiredError("VUID-VkHostImageLayoutTransitionInfo-image-09055");
    vk::TransitionImageLayoutEXT(*m_device, 1, &transition_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, TransitionImageLayoutMultiPlanar) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    VkImageFormatProperties img_prop = {};
    if (VK_SUCCESS != vk::GetPhysicalDeviceImageFormatProperties(
                          m_device->Physical().handle(), VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, VK_IMAGE_TYPE_2D,
                          VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_HOST_TRANSFER_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                          VK_IMAGE_CREATE_DISJOINT_BIT, &img_prop)) {
        GTEST_SKIP() << "Image Format Properties not supported";
    }

    VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkHostImageLayoutTransitionInfo transition_info = vku::InitStructHelper();
    transition_info.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    transition_info.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    transition_info.subresourceRange = range;

    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    transition_info.image = image;

    // imageSubresource.aspectMask must be VK_IMAGE_ASPECT_PLANE_0_BIT or VK_IMAGE_ASPECT_PLANE_1_BIT or
    // VK_IMAGE_ASPECT_COLOR_BIT
    VkDeviceMemory plane_0_memory;
    VkDeviceMemory plane_1_memory;
    auto image_ci_multi_planar =
        vkt::Image::ImageCreateInfo2D(width, height, 1, 1, VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    // Need a multi planar, disjoint image
    image_ci_multi_planar.usage = VK_IMAGE_USAGE_HOST_TRANSFER_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_ci_multi_planar.flags = VK_IMAGE_CREATE_DISJOINT_BIT;
    vkt::Image image_multi_planar(*m_device, image_ci_multi_planar, vkt::no_mem);
    VkImagePlaneMemoryRequirementsInfo image_plane_req = vku::InitStructHelper();
    image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
    VkImageMemoryRequirementsInfo2 mem_req_info2 = vku::InitStructHelper(&image_plane_req);
    mem_req_info2.image = image_multi_planar;
    VkMemoryRequirements2 mem_req2 = vku::InitStructHelper();
    vk::GetImageMemoryRequirements2(device(), &mem_req_info2, &mem_req2);
    // Find a valid memory type index to memory to be allocated from
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = mem_req2.memoryRequirements.size;
    m_device->Physical().SetMemoryType(mem_req2.memoryRequirements.memoryTypeBits, &alloc_info, 0);
    vk::AllocateMemory(device(), &alloc_info, NULL, &plane_0_memory);

    image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;
    vk::GetImageMemoryRequirements2(device(), &mem_req_info2, &mem_req2);
    alloc_info.allocationSize = mem_req2.memoryRequirements.size;
    m_device->Physical().SetMemoryType(mem_req2.memoryRequirements.memoryTypeBits, &alloc_info, 0);
    vk::AllocateMemory(device(), &alloc_info, NULL, &plane_1_memory);

    VkBindImagePlaneMemoryInfo plane_0_memory_info = vku::InitStructHelper();
    plane_0_memory_info.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
    VkBindImagePlaneMemoryInfo plane_1_memory_info = vku::InitStructHelper();
    plane_1_memory_info.planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;

    VkBindImageMemoryInfo bind_image_info[2]{};
    bind_image_info[0] = vku::InitStructHelper(&plane_0_memory_info);
    bind_image_info[0].image = image_multi_planar;
    bind_image_info[0].memory = plane_0_memory;
    bind_image_info[0].memoryOffset = 0;
    bind_image_info[1] = bind_image_info[0];
    bind_image_info[1].pNext = &plane_1_memory_info;
    bind_image_info[1].memory = plane_1_memory;
    vk::BindImageMemory2(device(), 2, bind_image_info);

    // Now transition the layout
    image_multi_planar.SetLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    transition_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    transition_info.image = image_multi_planar;
    m_errorMonitor->SetDesiredError("VUID-VkHostImageLayoutTransitionInfo-image-01672");
    vk::TransitionImageLayoutEXT(*m_device, 1, &transition_info);
    m_errorMonitor->VerifyFound();
    vk::FreeMemory(device(), plane_0_memory, nullptr);
    vk::FreeMemory(device(), plane_1_memory, nullptr);
    transition_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    transition_info.image = image;
}

TEST_F(NegativeHostImageCopy, TransitionImageLayoutNotSupported) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    if (CopyLayoutSupported(copy_src_layouts, copy_dst_layouts, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)) {
        GTEST_SKIP() << "Need Transfer src layout to not be supported";
    }

    VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkHostImageLayoutTransitionInfo transition_info = vku::InitStructHelper();
    transition_info.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    transition_info.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    transition_info.subresourceRange = range;

    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    transition_info.image = image;

    // layout must be one of the image layouts returned in VkPhysicalDeviceHostImageCopyPropertiesEXT::pCopySrcLayouts
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    transition_info.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    m_errorMonitor->SetDesiredError("VUID-VkHostImageLayoutTransitionInfo-oldLayout-09230");
    vk::TransitionImageLayoutEXT(*m_device, 1, &transition_info);
    m_errorMonitor->VerifyFound();
    transition_info.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // layout must be one of the image layouts returned in VkPhysicalDeviceHostImageCopyPropertiesEXT::pCopyDstLayouts
    transition_info.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    m_errorMonitor->SetDesiredError("VUID-VkHostImageLayoutTransitionInfo-newLayout-09057");
    vk::TransitionImageLayoutEXT(*m_device, 1, &transition_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, TransitionImageLayoutDepthStencil) {
    // need separateDepthStencilLayouts disabled
    RETURN_IF_SKIP(InitHostImageCopyTest());

    if (!CopyLayoutSupported(copy_src_layouts, copy_dst_layouts, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) ||
        !CopyLayoutSupported(copy_src_layouts, copy_dst_layouts, VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL) ||
        !CopyLayoutSupported(copy_src_layouts, copy_dst_layouts, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)) {
        GTEST_SKIP() << "Depth/Stencil image layout not supported";
    }

    auto stencil_format = FindSupportedDepthStencilFormat(Gpu());
    if (!(m_device->FormatFeaturesOptimal(stencil_format) & VK_FORMAT_FEATURE_2_HOST_IMAGE_TRANSFER_BIT)) {
        GTEST_SKIP() << "Device does not support host image on depth format";
    }

    VkImageFormatProperties img_prop = {};
    if (VK_SUCCESS != vk::GetPhysicalDeviceImageFormatProperties(
                          m_device->Physical().handle(), stencil_format, image_ci.imageType, image_ci.tiling,
                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_HOST_TRANSFER_BIT, image_ci.flags,
                          &img_prop)) {
        GTEST_SKIP() << "Image Format Properties not supported";
    }

    VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkHostImageLayoutTransitionInfo transition_info = vku::InitStructHelper();
    transition_info.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    transition_info.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    transition_info.subresourceRange = range;

    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    transition_info.image = image;

    image_ci.format = stencil_format;
    image_ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_HOST_TRANSFER_BIT;
    vkt::Image image_stencil(*m_device, image_ci, vkt::set_layout);
    image_stencil.SetLayout((VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT),
                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
    transition_info.image = image_stencil;
    transition_info.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    transition_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    // Will also get error for aspect != color
    m_errorMonitor->SetUnexpectedError("VUID-VkHostImageLayoutTransitionInfo-image-09241");
    m_errorMonitor->SetDesiredError("VUID-VkHostImageLayoutTransitionInfo-image-03320");
    vk::TransitionImageLayoutEXT(*m_device, 1, &transition_info);
    m_errorMonitor->VerifyFound();

    // subresourceRange includes VK_IMAGE_ASPECT_DEPTH_BIT, oldLayout and newLayout must not be one of
    // VK_IMAGE_LAYOUT_STENCIL_*_OPTIMAL
    transition_info.subresourceRange.aspectMask = (VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT);
    transition_info.newLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    // Will also get error for aspect != color
    m_errorMonitor->SetUnexpectedError("VUID-VkHostImageLayoutTransitionInfo-image-09241");
    m_errorMonitor->SetDesiredError("VUID-VkHostImageLayoutTransitionInfo-aspectMask-08702");
    vk::TransitionImageLayoutEXT(*m_device, 1, &transition_info);
    m_errorMonitor->VerifyFound();

    // subresourceRange includes VK_IMAGE_ASPECT_STENCIL_BIT, oldLayout and newLayout must not be one of
    // VK_IMAGE_LAYOUT_DEPTH_*_OPTIMAL
    transition_info.subresourceRange.aspectMask = (VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT);
    transition_info.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    // Will also get error for aspect != color
    m_errorMonitor->SetUnexpectedError("VUID-VkHostImageLayoutTransitionInfo-image-09241");
    m_errorMonitor->SetDesiredError("VUID-VkHostImageLayoutTransitionInfo-aspectMask-08703");
    vk::TransitionImageLayoutEXT(*m_device, 1, &transition_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, Features) {
    TEST_DESCRIPTION("Use VK_EXT_host_image_copy routines without enabling the hostImageCopy feature");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-10245");
    image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_HOST_TRANSFER_BIT);
    vkt::Image image(*m_device, image_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeHostImageCopy, ImageMemoryOverlap) {
    TEST_DESCRIPTION("Copy with host memory and image memory overlapping");
    RETURN_IF_SKIP(InitHostImageCopyTest());

    image_ci.mipLevels = 4;
    VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL;
    vkt::Image image(*m_device, image_ci, kHostVisibleMemProps);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);

    VkDeviceAddress *data = (VkDeviceAddress *)image.Memory().Map();

    VkImageToMemoryCopy region = vku::InitStructHelper();
    region.pHostPointer = data;
    region.memoryRowLength = 0;
    region.memoryImageHeight = 0;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32, 32, 1};
    const uint32_t copy_size = (32 * 32 * 4) / 8;  // 64 bit pointer
    VkCopyImageToMemoryInfo copy_image_to_memory = vku::InitStructHelper();
    copy_image_to_memory.srcImage = image.handle();
    copy_image_to_memory.srcImageLayout = layout;
    copy_image_to_memory.regionCount = 1;
    copy_image_to_memory.pRegions = &region;

    // Start of copy overlaps
    m_errorMonitor->SetDesiredError("VUID-VkImageToMemoryCopy-pRegions-09067");
    vk::CopyImageToMemoryEXT(*m_device, &copy_image_to_memory);
    m_errorMonitor->VerifyFound();

    // End of copy overlaps
    region.pHostPointer = data - (copy_size / 2);
    m_errorMonitor->SetDesiredError("VUID-VkImageToMemoryCopy-pRegions-09067");
    vk::CopyImageToMemoryEXT(*m_device, &copy_image_to_memory);
    m_errorMonitor->VerifyFound();

    // Extent fits, but rowLength * imageHeight doesn't
    region.pHostPointer = data - copy_size - 1;
    region.memoryRowLength = 48;
    region.memoryImageHeight = 32;
    m_errorMonitor->SetDesiredError("VUID-VkImageToMemoryCopy-pRegions-09067");
    vk::CopyImageToMemoryEXT(*m_device, &copy_image_to_memory);
    m_errorMonitor->VerifyFound();

    VkMemoryToImageCopy region2 = vku::InitStructHelper();
    region2.pHostPointer = data;
    region2.memoryRowLength = 0;
    region2.memoryImageHeight = 0;
    region2.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region2.imageOffset = {0, 0, 0};
    region2.imageExtent = {32, 32, 1};

    VkCopyMemoryToImageInfo copy_memory_to_image = vku::InitStructHelper();
    copy_memory_to_image.dstImage = image.handle();
    copy_memory_to_image.dstImageLayout = layout;
    copy_memory_to_image.regionCount = 1;
    copy_memory_to_image.pRegions = &region2;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryToImageCopy-pRegions-09062");
    vk::CopyMemoryToImageEXT(*m_device, &copy_memory_to_image);
    m_errorMonitor->VerifyFound();

    image.Memory().Unmap();
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8946
TEST_F(NegativeHostImageCopy, DISABLED_ImageMemoryOverlapCompressed) {
    RETURN_IF_SKIP(InitHostImageCopyTest());

    VkImageFormatProperties img_prop = {};
    if (VK_SUCCESS != vk::GetPhysicalDeviceImageFormatProperties(m_device->Physical().handle(), VK_FORMAT_BC3_SRGB_BLOCK,
                                                                 image_ci.imageType, image_ci.tiling, image_ci.usage,
                                                                 image_ci.flags, &img_prop)) {
        GTEST_SKIP() << "Image format properties not supported";
    }

    image_ci.format = VK_FORMAT_BC3_SRGB_BLOCK;
    image_ci.extent = {4, 4, 1};
    VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL;
    vkt::Image image(*m_device, image_ci, kHostVisibleMemProps);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, layout);

    VkDeviceAddress *data = (VkDeviceAddress *)image.Memory().Map();

    VkMemoryToImageCopy region2 = vku::InitStructHelper();
    region2.pHostPointer = data;
    region2.memoryRowLength = 0;
    region2.memoryImageHeight = 0;
    region2.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region2.imageOffset = {0, 0, 0};
    region2.imageExtent = {4, 4, 1};

    VkCopyMemoryToImageInfo copy_memory_to_image = vku::InitStructHelper();
    copy_memory_to_image.dstImage = image.handle();
    copy_memory_to_image.dstImageLayout = layout;
    copy_memory_to_image.regionCount = 1;
    copy_memory_to_image.pRegions = &region2;

    m_errorMonitor->SetDesiredError("VUID-VkMemoryToImageCopy-pRegions-09062");
    vk::CopyMemoryToImageEXT(*m_device, &copy_memory_to_image);
    m_errorMonitor->VerifyFound();

    image.Memory().Unmap();
}

TEST_F(NegativeHostImageCopy, ImageMemorySparseUnbound) {
    TEST_DESCRIPTION("Copy with host memory and image memory overlapping");
    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(InitHostImageCopyTest());

    image_ci.usage = VK_IMAGE_USAGE_HOST_TRANSFER_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_ci.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
    VkImageFormatProperties img_prop;
    if (VK_SUCCESS != GPDIFPHelper(Gpu(), &image_ci, &img_prop)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    vkt::Image image(*m_device, image_ci, vkt::no_mem);

    const uint32_t buffer_size = width * height * 4;
    std::vector<uint8_t> data(buffer_size);

    VkImageToMemoryCopy region = vku::InitStructHelper();
    region.pHostPointer = data.data();
    region.memoryRowLength = 0;
    region.memoryImageHeight = 0;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    VkCopyImageToMemoryInfo copy_image_to_memory = vku::InitStructHelper();
    copy_image_to_memory.srcImage = image.handle();
    copy_image_to_memory.srcImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    copy_image_to_memory.regionCount = 1;
    copy_image_to_memory.pRegions = &region;

    // LAYOUT_UNDEFINED will not be allowed, but image has no memory
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToMemoryInfo-srcImageLayout-09065");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToMemoryInfo-srcImage-09109");
    vk::CopyImageToMemoryEXT(*m_device, &copy_image_to_memory);
    m_errorMonitor->VerifyFound();

    VkMemoryToImageCopy region2 = vku::InitStructHelper();
    region2.pHostPointer = data.data();
    region2.memoryRowLength = 0;
    region2.memoryImageHeight = 0;
    region2.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region2.imageOffset = {0, 0, 0};
    region2.imageExtent = {width, height, 1};

    VkCopyMemoryToImageInfo copy_memory_to_image = vku::InitStructHelper();
    copy_memory_to_image.dstImage = image.handle();
    copy_memory_to_image.dstImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    copy_memory_to_image.regionCount = 1;
    copy_memory_to_image.pRegions = &region2;

    // LAYOUT_UNDEFINED will not be allowed, but image has no memory
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyMemoryToImageInfo-dstImageLayout-09059");
    m_errorMonitor->SetDesiredError("VUID-VkCopyMemoryToImageInfo-dstImage-09109");
    vk::CopyMemoryToImageEXT(*m_device, &copy_memory_to_image);
    m_errorMonitor->VerifyFound();

    // Images have to be identical, and some drivers have no memory for both USAGE_HOST_TRANSFER and SPARSE_BINDING
    // so check for both src and dst in one call
    vkt::Image image2(*m_device, image_ci, vkt::no_mem);

    VkImageCopy2 image_copy = vku::InitStructHelper();
    image_copy.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy.extent = {32, 32, 1};
    VkCopyImageToImageInfo copy_image_to_image = vku::InitStructHelper();
    copy_image_to_image.regionCount = 1;
    copy_image_to_image.pRegions = &image_copy;
    copy_image_to_image.srcImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    copy_image_to_image.dstImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    copy_image_to_image.srcImage = image;
    copy_image_to_image.dstImage = image2;
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-srcImageLayout-09072");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-srcImage-09109");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyImageToImageInfo-dstImageLayout-09071");
    m_errorMonitor->SetDesiredError("VUID-VkCopyImageToImageInfo-dstImage-09109");
    vk::CopyImageToImageEXT(*m_device, &copy_image_to_image);
    m_errorMonitor->VerifyFound();
}
