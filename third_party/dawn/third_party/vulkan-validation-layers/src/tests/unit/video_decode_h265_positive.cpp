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

class PositiveVideoDecodeH265 : public VkVideoLayerTest {};

TEST_F(PositiveVideoDecodeH265, Basic) {
    TEST_DESCRIPTION("Tests basic H.265/HEVC video decode use case for framework verification purposes");

    RETURN_IF_SKIP(Init());

    const uint32_t dpb_slots = 3;
    const uint32_t active_refs = 2;

    VideoConfig config =
        GetConfig(GetConfigsWithReferences(GetConfigsWithDpbSlots(GetConfigsDecodeH265(), dpb_slots), active_refs));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support with 3 DPB slots and 2 active references";
    }

    config.SessionCreateInfo()->maxDpbSlots = dpb_slots;
    config.SessionCreateInfo()->maxActiveReferencePictures = active_refs;

    VideoContext context(m_device, config);
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

TEST_F(PositiveVideoDecodeH265, InlineSessionParams) {
    TEST_DESCRIPTION(
        "vkCmdBeginVideoCodingKHR - H.265 decode does not require session parameters when videoMaintenance2 is enabled (no VUID "
        "07248)");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance2);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support";
    }
    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto std_vps = config.CreateH265VPS(0);
    auto std_sps = config.CreateH265SPS(0, 0);
    auto std_pps = config.CreateH265PPS(0, 0, 0);

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().SetSessionParams(VK_NULL_HANDLE));
    cb.DecodeVideo(context.DecodeFrame().InlineParamsH265(&std_vps, &std_sps, &std_pps));
    cb.EndVideoCoding(context.End());
    cb.End();
}
