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

class PositiveVideo : public VkVideoLayerTest {};

TEST_F(PositiveVideo, VideoCodingScope) {
    TEST_DESCRIPTION("Tests calling functions inside/outside video coding scope");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(m_device, GetConfigDecode());
    context.CreateAndBindSessionMemory();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset());
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(PositiveVideo, MultipleCmdBufs) {
    TEST_DESCRIPTION("Tests submit-time validation with multiple command buffers submitted at once");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandPool cmd_pool(*m_device, config.QueueFamilyIndex(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer cb1(*m_device, cmd_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    vkt::CommandBuffer cb2(*m_device, cmd_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    cb1.Begin();
    cb1.BeginVideoCoding(context.Begin());
    cb1.ControlVideoCoding(context.Control().Reset());
    cb1.EndVideoCoding(context.End());
    cb1.End();

    cb2.Begin();
    vk::CmdPipelineBarrier2KHR(cb2.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    cb2.BeginVideoCoding(context.Begin().AddResource(-1, 0));
    cb2.DecodeVideo(context.DecodeFrame(0));
    cb2.EndVideoCoding(context.End());
    cb2.End();

    std::array cbs = {&cb1, &cb2};
    context.Queue().Submit(cbs);
    m_device->Wait();
}

TEST_F(PositiveVideo, BeginCodingOutOfBoundsSlotIndex) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - referenced DPB slot index is invalid but it should not cause submit time crash");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigs(), 4));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with support for 4 reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 3;
    config.SessionCreateInfo()->maxActiveReferencePictures = 3;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoBeginCodingInfoKHR-slotIndex-04856");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(3, 1).AddResource(-1, 2));
    cb.EndVideoCoding(context.End());
    cb.End();

    context.Queue().Submit(cb);
    m_device->Wait();
}
