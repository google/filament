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

class PositiveVideoEncode : public VkVideoLayerTest {};

TEST_F(PositiveVideoEncode, ProfileIndependentResources) {
    TEST_DESCRIPTION("Test video profile independent resources with encode");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with references";
    }

    config.EnableProfileIndependentResources();
    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
    cb.EncodeVideo(context.EncodeReferenceFrame(0));
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(PositiveVideoEncode, RateControlVirtualBufferSize) {
    TEST_DESCRIPTION(
        "vkCmdControlVideoCodingKHR - test valid values for "
        "virtualBufferSizeInMs and initialVirtualBufferSizeInMs");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    auto rc_info = VideoEncodeRateControlInfo(config).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    // initialVirtualBufferSizeInMs can be less than virtualBufferSizeInMs
    rc_info->virtualBufferSizeInMs = 1000;
    rc_info->initialVirtualBufferSizeInMs = 0;
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));

    // initialVirtualBufferSizeInMs can be equal to virtualBufferSizeInMs
    rc_info->virtualBufferSizeInMs = 1000;
    rc_info->initialVirtualBufferSizeInMs = 1000;
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));

    cb.EndVideoCoding(context.End());
    cb.End();
}
