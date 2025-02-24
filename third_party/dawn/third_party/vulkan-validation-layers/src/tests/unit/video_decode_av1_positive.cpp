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

class PositiveVideoDecodeAV1 : public VkVideoLayerTest {};

TEST_F(PositiveVideoDecodeAV1, Basic) {
    TEST_DESCRIPTION("Tests basic AV1 video decode use case for framework verification purposes");

    RETURN_IF_SKIP(Init());

    const uint32_t dpb_slots = 3;
    const uint32_t active_refs = 2;

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsWithDpbSlots(GetConfigsDecodeAV1(), dpb_slots), active_refs));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 decode support with at least 3 DPB slots and 2 active references";
    }

    config.SessionCreateInfo()->maxDpbSlots = dpb_slots;
    config.SessionCreateInfo()->maxActiveReferencePictures = active_refs;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR));
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(-1, 1).AddResource(-1, 2));
    cb.ControlVideoCoding(context.Control().Reset());
    cb.DecodeVideo(context.DecodeReferenceFrame(0));
    cb.DecodeVideo(context.DecodeFrame(1).AddReferenceFrame(0));
    cb.DecodeVideo(context.DecodeReferenceFrame(1).AddReferenceFrame(0));
    cb.DecodeVideo(context.DecodeFrame(2));
    cb.DecodeVideo(context.DecodeReferenceFrame(2).AddReferenceFrame(0).AddReferenceFrame(1));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).InvalidateSlot(1).AddResource(-1, 1).AddResource(2, 2));
    cb.DecodeVideo(context.DecodeFrame(1));
    cb.DecodeVideo(context.DecodeReferenceFrame(1).AddReferenceFrame(0).AddReferenceFrame(2));
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(1));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();
}

TEST_F(PositiveVideoDecodeAV1, InlineSessionParams) {
    TEST_DESCRIPTION(
        "vkCmdBeginVideoCodingKHR - AV1 decode does not require session parameters when videoMaintenance2 is enabled (no VUID "
        "09261)");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance2);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 decode support";
    }
    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto std_seq_header = config.CreateAV1SequenceHeader();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().SetSessionParams(VK_NULL_HANDLE));
    cb.DecodeVideo(context.DecodeFrame().InlineParamsAV1(&std_seq_header));
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(PositiveVideoDecodeAV1, DistinctWithFilmGrain) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - AV1 works with distinct reconstructed picture if using film grain");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithReferences(GetConfigsDecodeAV1FilmGrain()),
                                                 [](const VideoConfig& config) { return config.SupportsDecodeOutputDistinct(); }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 decode support with reference pictures, film grain, and no distinct mode";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
    cb.DecodeVideo(context.DecodeReferenceFrame(0).ApplyFilmGrain());
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(PositiveVideoDecodeAV1, CoincideWithoutFilmGrain) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - AV1 only requires distinct reconstructed picture if using film grain");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithReferences(GetConfigsDecodeAV1FilmGrain()),
                                                 [](const VideoConfig& config) { return config.SupportsDecodeOutputCoincide(); }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 decode support with reference pictures, film grain, and coincide mode";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07146");
    cb.DecodeVideo(context.DecodeFrame(0).SetDecodeOutput(context.Dpb()->Picture(0)));

    cb.EndVideoCoding(context.End());
    cb.End();
}
