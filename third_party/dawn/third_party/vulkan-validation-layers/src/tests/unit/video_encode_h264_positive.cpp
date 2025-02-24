/*
 * Copyright (c) 2022-2025 The Khronos Group Inc.
 * Copyright (c) 2022-2025 RasterGrid Kft.
 * Modifications Copyright (C) 2024 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/video_objects.h"

class PositiveVideoEncodeH264 : public VkVideoLayerTest {};

TEST_F(PositiveVideoEncodeH264, Basic) {
    TEST_DESCRIPTION("Tests basic H.264/AVC video encode use case for framework verification purposes");

    RETURN_IF_SKIP(Init());

    const uint32_t dpb_slots = 3;
    const uint32_t active_refs = 2;

    VideoConfig config = GetConfig(GetConfigsWithReferences(
        GetConfigsWithDpbSlots(GetConfigsWithRateControl(GetConfigsEncodeH264()), dpb_slots), active_refs));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with rate control and 3 DPB slots and 2 active references";
    }

    config.SessionCreateInfo()->maxDpbSlots = dpb_slots;
    config.SessionCreateInfo()->maxActiveReferencePictures = active_refs;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config).SetAnyMode();
    for (uint32_t i = 0; i < config.EncodeCaps()->maxRateControlLayers; ++i) {
        auto rc_layer = VideoEncodeRateControlLayerInfo(config);
        rc_layer->averageBitrate = 64000;
        rc_layer->maxBitrate = 64000;
        rc_layer->frameRateNumerator = 30;
        rc_layer->frameRateDenominator = 1;
        rc_info.AddLayer(rc_layer);
    }

    cb.Begin();
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.EncodeInput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR));
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(-1, 1).AddResource(-1, 2));
    cb.ControlVideoCoding(context.Control().Reset().RateControl(rc_info).EncodeQualityLevel(0));
    cb.EncodeVideo(context.EncodeReferenceFrame(0));
    cb.EncodeVideo(context.EncodeReferenceFrame(1).AddReferenceFrame(0));
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(0));
    cb.EncodeVideo(context.EncodeReferenceFrame(2).AddReferenceFrame(0));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.Begin();
    cb.BeginVideoCoding(
        context.Begin().RateControl(rc_info).AddResource(0, 0).InvalidateSlot(1).AddResource(-1, 1).AddResource(2, 2));
    cb.EncodeVideo(context.EncodeFrame(1));
    cb.EncodeVideo(context.EncodeReferenceFrame(1).AddReferenceFrame(0).AddReferenceFrame(2));
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(1));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();
}

TEST_F(PositiveVideoEncodeH264, RateControlLayerCount) {
    TEST_DESCRIPTION(
        "vkCmdBeginVideoCodingKHR / vkCmdControlVideoCodingKHR - H.264 temporal layer count must only match "
        "the layer count if the layer count is greater than 1");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeH264()), [](const VideoConfig& config) {
        return config.EncodeCapsH264()->maxTemporalLayerCount > 1;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with rate control and temporal layer support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config, true).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    rc_info.CodecInfo().encode_h264.temporalLayerCount = 2;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(PositiveVideoEncodeH264, GetEncodedSessionParams) {
    TEST_DESCRIPTION("vkGetEncodedVideoSessionParametersKHR - test basic usage");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support";
    }

    VideoContext context(m_device, config);

    auto h264_info = vku::InitStruct<VkVideoEncodeH264SessionParametersGetInfoKHR>();
    h264_info.writeStdSPS = VK_TRUE;
    h264_info.writeStdPPS = VK_TRUE;

    auto get_info = vku::InitStruct<VkVideoEncodeSessionParametersGetInfoKHR>(&h264_info);
    get_info.videoSessionParameters = context.SessionParams();

    auto feedback_info = vku::InitStruct<VkVideoEncodeSessionParametersFeedbackInfoKHR>();
    size_t data_size = 0;

    // Calling without feedback info and data pointer is legal
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);

    std::vector<uint8_t> data_buffer(data_size);

    // Calling without feedback info but data pointer is legal
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, data_buffer.data());

    // Calling with feedback info not including codec-specific feedback info
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, &feedback_info, &data_size, nullptr);
}
