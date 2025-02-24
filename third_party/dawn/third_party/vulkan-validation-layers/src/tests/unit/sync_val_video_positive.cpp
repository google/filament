/*
 * Copyright (c) 2022-2025 The Khronos Group Inc.
 * Copyright (c) 2022-2025 RasterGrid Kft.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/video_objects.h"

class PositiveSyncValVideo : public VkVideoSyncLayerTest {};

TEST_F(PositiveSyncValVideo, ImageRangeGenYcbcrSubsampling) {
    TEST_DESCRIPTION(
        "Test that subsampled YCbCr image planes are handled correctly "
        "by the image range generation utilities used by sync validation");

    RETURN_IF_SKIP(Init());

    // Test values that require the implementation to handle YCbCr subsampling correctly
    // across planes in order for this test to not hit any asserts
    const VkExtent2D max_coded_extent = {272, 272};
    const VkExtent2D coded_extent = {256, 256};

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsDecode(), [&](const VideoConfig& config) {
        return (config.PictureFormatProps()->format == VK_FORMAT_G8_B8R8_2PLANE_420_UNORM ||
                config.PictureFormatProps()->format == VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM) &&
               config.Caps()->maxCodedExtent.width >= max_coded_extent.width &&
               config.Caps()->maxCodedExtent.height >= max_coded_extent.height;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires decode with 4:2:0 decode picture format support";
    }

    config.UpdateMaxCodedExtent(config.Caps()->maxCodedExtent);

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset());

    // Test with a subregion that would cross the half-extent boundaries of a 4:2:0 subsampled image
    auto decode_info = context.DecodeFrame();
    decode_info->dstPictureResource.codedExtent = coded_extent;
    cb.DecodeVideo(decode_info);

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->MemoryBarrier());

    // Also test with an offset (ignoring other validation violations)
    decode_info->dstPictureResource.codedOffset = {1, 1};
    cb.DecodeVideo(decode_info);

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(PositiveSyncValVideo, DecodeCoincide) {
    TEST_DESCRIPTION("Test video decode in coincide mode without sync hazards");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(FilterConfigs(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 3),
                                          [](const VideoConfig& config) { return !config.SupportsDecodeOutputDistinct(); }));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with references, 3 DPB slots, and coincide mode support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 3;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(2, 2));
    cb.ControlVideoCoding(context.Control().Reset());

    cb.DecodeVideo(context.DecodeReferenceFrame(0));
    cb.DecodeVideo(context.DecodeFrame(1));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(1, 1));
    cb.DecodeVideo(context.DecodeFrame(1));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(0, 1));
    cb.DecodeVideo(context.DecodeReferenceFrame(0));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(0, 1));
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(1, 1));
    cb.DecodeVideo(context.DecodeReferenceFrame(1));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(0, 2));
    cb.DecodeVideo(context.DecodeFrame(0).AddReferenceFrame(1));

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(PositiveSyncValVideo, DecodeDistinct) {
    TEST_DESCRIPTION("Test video decode in distinct mode without sync hazards");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(FilterConfigs(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 4),
                                          [](const VideoConfig& config) { return config.SupportsDecodeOutputDistinct(); }));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with references, 4 DPB slots, and distinct mode support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 4;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(2, 2).AddResource(3, 3));
    cb.ControlVideoCoding(context.Control().Reset());

    cb.DecodeVideo(context.DecodeReferenceFrame(0));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->MemoryBarrier());
    cb.DecodeVideo(context.DecodeFrame(1));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->MemoryBarrier());
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(0, 1));
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->MemoryBarrier());
    cb.DecodeVideo(context.DecodeReferenceFrame(3));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->MemoryBarrier());
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(2, 2));
    cb.DecodeVideo(context.DecodeReferenceFrame(2).AddReferenceFrame(3));

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(PositiveSyncValVideo, Encode) {
    TEST_DESCRIPTION("Test video without sync hazards");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncode()), 4));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with references and 4 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 4;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(2, 2).AddResource(3, 3));
    cb.ControlVideoCoding(context.Control().Reset());

    cb.EncodeVideo(context.EncodeReferenceFrame(0));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Bitstream().MemoryBarrier());
    cb.EncodeVideo(context.EncodeFrame(1));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Bitstream().MemoryBarrier());
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(0, 1));
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(0));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Bitstream().MemoryBarrier());
    cb.EncodeVideo(context.EncodeReferenceFrame(3));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Bitstream().MemoryBarrier());
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(2, 2));
    cb.EncodeVideo(context.EncodeReferenceFrame(2).AddReferenceFrame(3));

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(PositiveSyncValVideo, EncodeQuantizationMap) {
    TEST_DESCRIPTION("Test video encode quantization map without sync hazards");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());

    if ((!delta_config || (QueueFamilyFlags(delta_config.QueueFamilyIndex()) & VK_QUEUE_TRANSFER_BIT) == 0) &&
        (!emphasis_config || (QueueFamilyFlags(emphasis_config.QueueFamilyIndex()) & VK_QUEUE_TRANSFER_BIT) == 0)) {
        GTEST_SKIP() << "Test case requires video encode queue to also support transfer";
    }

    struct TestConfig {
        VideoConfig config;
        VkVideoEncodeFlagBitsKHR encode_flag;
        VkVideoSessionCreateFlagBitsKHR session_create_flag;
        const VkVideoFormatPropertiesKHR* map_props;
    };

    std::vector<TestConfig> tests;
    if (delta_config) {
        tests.emplace_back(TestConfig{delta_config, VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      delta_config.QuantDeltaMapProps()});
    }
    if (emphasis_config) {
        tests.emplace_back(TestConfig{emphasis_config, VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR,
                                      emphasis_config.EmphasisMapProps()});
    }

    for (auto& [config, encode_flag, session_create_flag, map_props] : tests) {
        if (QueueFamilyFlags(config.QueueFamilyIndex()) & VK_QUEUE_TRANSFER_BIT) {
        }

        config.SessionCreateInfo()->flags |= session_create_flag;
        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        const auto texel_size = config.GetQuantMapTexelSize(*map_props);
        auto params = context.CreateSessionParamsWithQuantMapTexelSize(texel_size);

        vkt::CommandBuffer& cb = context.CmdBuffer();

        VideoEncodeQuantizationMap quantization_map(m_device, config, *map_props);

        cb.Begin();

        VkClearColorValue clear_value{};
        VkImageSubresourceRange subres_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        vk::CmdClearColorImage(cb.handle(), quantization_map.Image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_value, 1,
                               &subres_range);

        auto barrier_from_transfer = vku::InitStruct<VkImageMemoryBarrier2>();
        barrier_from_transfer.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier_from_transfer.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier_from_transfer.dstStageMask = VK_PIPELINE_STAGE_2_VIDEO_ENCODE_BIT_KHR;
        barrier_from_transfer.dstAccessMask = VK_ACCESS_2_VIDEO_ENCODE_READ_BIT_KHR;
        barrier_from_transfer.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier_from_transfer.newLayout = VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR;
        barrier_from_transfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier_from_transfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier_from_transfer.image = quantization_map.Image();
        barrier_from_transfer.subresourceRange = subres_range;
        auto dep_info_from_transfer = vku::InitStruct<VkDependencyInfo>();
        dep_info_from_transfer.imageMemoryBarrierCount = 1;
        dep_info_from_transfer.pImageMemoryBarriers = &barrier_from_transfer;
        vk::CmdPipelineBarrier2KHR(cb.handle(), &dep_info_from_transfer);

        cb.BeginVideoCoding(context.Begin().SetSessionParams(params));
        cb.ControlVideoCoding(context.Control().Reset());

        cb.EncodeVideo(context.EncodeFrame().QuantizationMap(encode_flag, texel_size, quantization_map));
        vk::CmdPipelineBarrier2KHR(cb.handle(), context.Bitstream().MemoryBarrier());
        cb.EncodeVideo(context.EncodeFrame().QuantizationMap(encode_flag, texel_size, quantization_map));

        auto barrier_to_transfer = vku::InitStruct<VkImageMemoryBarrier2>();
        barrier_to_transfer.srcStageMask = VK_PIPELINE_STAGE_2_VIDEO_ENCODE_BIT_KHR;
        barrier_to_transfer.srcAccessMask = VK_ACCESS_2_VIDEO_ENCODE_READ_BIT_KHR;
        barrier_to_transfer.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier_to_transfer.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier_to_transfer.oldLayout = VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR;
        barrier_to_transfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier_to_transfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier_to_transfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier_to_transfer.image = quantization_map.Image();
        barrier_to_transfer.subresourceRange = subres_range;
        auto dep_info_to_transfer = vku::InitStruct<VkDependencyInfo>();
        dep_info_to_transfer.imageMemoryBarrierCount = 1;
        dep_info_to_transfer.pImageMemoryBarriers = &barrier_to_transfer;
        vk::CmdPipelineBarrier2KHR(cb.handle(), &dep_info_to_transfer);

        cb.EndVideoCoding(context.End());

        vk::CmdClearColorImage(cb.handle(), quantization_map.Image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_value, 1,
                               &subres_range);

        cb.End();
    }

    if (!delta_config || (QueueFamilyFlags(delta_config.QueueFamilyIndex()) & VK_QUEUE_TRANSFER_BIT) == 0 || !emphasis_config ||
        (QueueFamilyFlags(emphasis_config.QueueFamilyIndex()) & VK_QUEUE_TRANSFER_BIT) == 0) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}
