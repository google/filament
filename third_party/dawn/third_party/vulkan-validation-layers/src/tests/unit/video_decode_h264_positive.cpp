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

class PositiveVideoDecodeH264 : public VkVideoLayerTest {};

TEST_F(PositiveVideoDecodeH264, Basic) {
    TEST_DESCRIPTION("Tests basic H.264/AVC video decode use case for framework verification purposes");

    RETURN_IF_SKIP(Init());

    const uint32_t dpb_slots = 3;
    const uint32_t active_refs = 2;

    VideoConfig config =
        GetConfig(GetConfigsWithReferences(GetConfigsWithDpbSlots(GetConfigsDecodeH264(), dpb_slots), active_refs));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support with 3 DPB slots and 2 active references";
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
    cb.DecodeVideo(context.DecodeReferenceFrame(1).AddReferenceFrame(0));
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0));
    cb.DecodeVideo(context.DecodeReferenceFrame(2).AddReferenceFrame(0));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).InvalidateSlot(1).AddResource(-1, 1).AddResource(2, 2));
    cb.DecodeVideo(context.DecodeFrame(1));
    cb.DecodeVideo(context.DecodeReferenceFrame(1).AddReferenceFrame(0).AddReferenceFrame(2));
    cb.DecodeVideo(context.DecodeReferenceFrame(2).AddReferenceFrame(1));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();
}

TEST_F(PositiveVideoDecodeH264, Interlaced) {
    TEST_DESCRIPTION("Tests basic H.264/AVC interlaced video decode use case for framework verification purposes");

    RETURN_IF_SKIP(Init());

    const uint32_t dpb_slots = 2;
    const uint32_t active_refs = 2;

    VideoConfig config =
        GetConfig(GetConfigsWithReferences(GetConfigsWithDpbSlots(GetConfigsDecodeH264Interlaced(), dpb_slots), active_refs));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 interlaced decode support with 2 DPB slots and 2 active references";
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
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(-1, 1));
    cb.ControlVideoCoding(context.Control().Reset());
    cb.DecodeVideo(context.DecodeReferenceTopField(0));
    cb.DecodeVideo(context.DecodeReferenceBottomField(0).AddReferenceTopField(0));
    cb.DecodeVideo(context.DecodeFrame(1).AddReferenceBottomField(0));
    cb.DecodeVideo(context.DecodeReferenceFrame(1).AddReferenceBothFields(0));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().InvalidateSlot(0).AddResource(-1, 0).AddResource(1, 1));
    cb.DecodeVideo(context.DecodeFrame(0));
    cb.DecodeVideo(context.DecodeReferenceFrame(0).AddReferenceFrame(1));
    cb.DecodeVideo(context.DecodeFrame(1).AddReferenceFrame(0));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();
}

TEST_F(PositiveVideoDecodeH264, InterlacedPartialInvalidation) {
    TEST_DESCRIPTION("Tests H.264/AVC interlaced video decode with partial DPB slot picture reference invalidation");

    RETURN_IF_SKIP(Init());

    const uint32_t dpb_slots = 3;
    const uint32_t active_refs = 2;

    VideoConfig config =
        GetConfig(GetConfigsWithReferences(GetConfigsWithDpbSlots(GetConfigsDecodeH264Interlaced(), dpb_slots), active_refs));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 interlaced decode support with 3 DPB slots and 2 active references";
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
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(-1, 1));
    cb.ControlVideoCoding(context.Control().Reset());
    cb.DecodeVideo(context.DecodeReferenceTopField(0));
    cb.DecodeVideo(context.DecodeBottomField(0).AddReferenceTopField(0));
    cb.DecodeVideo(context.DecodeReferenceFrame(1).AddReferenceTopField(0));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(-1, 2));
    cb.DecodeVideo(context.DecodeReferenceBottomField(2).AddReferenceTopField(0).AddReferenceFrame(1));
    cb.DecodeVideo(context.DecodeTopField(2).AddReferenceBottomField(2));
    cb.DecodeVideo(context.DecodeFrame(0).AddReferenceBottomField(2));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();
}

TEST_F(PositiveVideoDecodeH264, InlineSessionParams) {
    TEST_DESCRIPTION(
        "vkCmdBeginVideoCodingKHR - H.264 decode does not require session parameters when videoMaintenance2 is enabled (no VUID "
        "07247)");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance2);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support";
    }
    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto std_sps = config.CreateH264SPS(0);
    auto std_pps = config.CreateH264PPS(0, 0);

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().SetSessionParams(VK_NULL_HANDLE));
    cb.DecodeVideo(context.DecodeFrame().InlineParamsH264(&std_sps, &std_pps));
    cb.EndVideoCoding(context.End());
    cb.End();
}
