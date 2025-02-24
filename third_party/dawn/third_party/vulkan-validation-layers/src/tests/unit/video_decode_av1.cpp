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

class NegativeVideoDecodeAV1 : public VkVideoLayerTest {};

TEST_F(NegativeVideoDecodeAV1, ProfileMissingCodecInfo) {
    TEST_DESCRIPTION("Test missing codec-specific structure in profile definition");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 decode support";
    }

    VkVideoProfileInfoKHR profile = *config.Profile();
    profile.pNext = nullptr;

    // Missing codec-specific info for AV1 decode profile
    m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-09256");
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(Gpu(), &profile, config.Caps());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoDecodeAV1, CapabilityQueryMissingChain) {
    TEST_DESCRIPTION("vkGetPhysicalDeviceVideoCapabilitiesKHR - missing return structures in chain");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 decode support";
    }

    auto caps = vku::InitStruct<VkVideoCapabilitiesKHR>();
    auto decode_caps = vku::InitStruct<VkVideoDecodeCapabilitiesKHR>();
    auto decode_av1_caps = vku::InitStruct<VkVideoDecodeAV1CapabilitiesKHR>();

    // Missing decode caps struct for decode profile
    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07183");
    caps.pNext = &decode_av1_caps;
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(Gpu(), config.Profile(), &caps);
    m_errorMonitor->VerifyFound();

    // Missing AV1 decode caps struct for AV1 decode profile
    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-09257");
    caps.pNext = &decode_caps;
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(Gpu(), config.Profile(), &caps);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoDecodeAV1, CreateSessionParamsMissingCodecInfo) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - missing codec-specific chained structure");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 decode support";
    }

    VideoContext context(DeviceObj(), config);

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    auto other_codec_info = vku::InitStruct<VkVideoDecodeH264SessionParametersCreateInfoKHR>();
    other_codec_info.maxStdSPSCount = 1;
    other_codec_info.maxStdPPSCount = 1;
    create_info.pNext = &other_codec_info;
    create_info.videoSession = context.Session();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-09259");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoDecodeAV1, CreateSessionParamsTemplateNotAllowed) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - AV1 decode does not allow using parameters templates");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigDecodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 decode support";
    }

    VideoContext context(DeviceObj(), config);

    StdVideoAV1SequenceHeader seq_header{};
    auto av1_ci = vku::InitStruct<VkVideoDecodeAV1SessionParametersCreateInfoKHR>();
    av1_ci.pStdSequenceHeader = &seq_header;

    VkVideoSessionParametersKHR params, params2;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &av1_ci;
    create_info.videoSession = context.Session();

    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);

    create_info.videoSessionParametersTemplate = params;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-09258");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideoDecodeAV1, UpdateSessionParams) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - not supported for AV1 decode");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigDecodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 decode support";
    }

    VideoContext context(DeviceObj(), config);

    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.updateSequenceCount = 1;

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-09260");
    vk::UpdateVideoSessionParametersKHR(device(), context.SessionParams(), &update_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoDecodeAV1, BeginCodingRequiresSessionParams) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - AV1 decode requires session parameters");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 decode support";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoBeginCodingInfoKHR begin_info = context.Begin();
    begin_info.videoSessionParameters = VK_NULL_HANDLE;

    cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-videoSession-09261");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();
    cb.End();
}

TEST_F(NegativeVideoDecodeAV1, DecodeMissingInlineSessionParams) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - AV1 decode requires bound params object when not all params are specified inline");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance2);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 decode support";
    }

    // Session has no VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR
    {
        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        StdVideoAV1SequenceHeader std_seq_header{};

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().SetSessionParams(VK_NULL_HANDLE));

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-None-10407");
        cb.DecodeVideo(context.DecodeFrame().InlineParamsAV1(&std_seq_header));
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    // Missing inline sequence header
    {
        config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().SetSessionParams(VK_NULL_HANDLE));

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-None-10407");
        cb.DecodeVideo(context.DecodeFrame().InlineParamsAV1(nullptr));
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }
}

TEST_F(NegativeVideoDecodeAV1, DecodeInvalidCodecInfo) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - invalid/missing AV1 codec-specific information");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecodeAV1(), 2), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 decode support with 2 reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VideoDecodeInfo decode_info = context.DecodeFrame(0);

    StdVideoDecodeAV1PictureInfo std_picture_info{};
    auto picture_info = vku::InitStruct<VkVideoDecodeAV1PictureInfoKHR>();
    uint32_t tile_offset = 0;
    uint32_t tile_size = (uint32_t)decode_info->srcBufferRange;
    picture_info.pStdPictureInfo = &std_picture_info;

    for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
        picture_info.referenceNameSlotIndices[i] = -1;
    }

    picture_info.tileCount = 1;
    picture_info.pTileOffsets = &tile_offset;
    picture_info.pTileSizes = &tile_size;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    // Missing AV1 picture info
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = nullptr;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-09250");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    // Frame header offset must be within buffer range
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-frameHeaderOffset-09251");
        picture_info.frameHeaderOffset = (uint32_t)decode_info->srcBufferRange;
        cb.DecodeVideo(decode_info);
        picture_info.frameHeaderOffset = 0;
        m_errorMonitor->VerifyFound();
    }

    // Tile offsets must be within buffer range
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pTileOffsets-09252");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pTileOffsets-09253");
        tile_offset = (uint32_t)decode_info->srcBufferRange;
        cb.DecodeVideo(decode_info);
        tile_offset = 0;
        m_errorMonitor->VerifyFound();
    }

    // Tile offset plus size must be within buffer range
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pTileOffsets-09252");
        tile_size = (uint32_t)decode_info->srcBufferRange + 1;
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pTileOffsets-09252");
        tile_offset = 1;
        tile_size = (uint32_t)decode_info->srcBufferRange;
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();

        tile_offset = 0;
        tile_size = (uint32_t)decode_info->srcBufferRange;
    }

    // Film grain cannot be used if the video profile did not enable support for it
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-09249");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-filmGrainSupport-09248");
        std_picture_info.flags.apply_grain = 1;
        cb.DecodeVideo(decode_info);
        std_picture_info.flags.apply_grain = 0;
        m_errorMonitor->VerifyFound();
    }

    // Missing AV1 setup reference info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        decode_info = context.DecodeFrame(1);
        decode_info->pNext = &picture_info;
        decode_info->pSetupReferenceSlot = &slot;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07141");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-09254");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    // Missing AV1 reference info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        decode_info = context.DecodeFrame(1);
        decode_info->pNext = &picture_info;
        decode_info->referenceSlotCount = 1;
        decode_info->pReferenceSlots = &slot;

        picture_info.referenceNameSlotIndices[3] = 0;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-09255");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();

        picture_info.referenceNameSlotIndices[3] = -1;
    }

    // Missing reference in referenceNameSlotIndices to reference slot
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        decode_info = context.DecodeFrame(1);
        decode_info->pNext = &picture_info;
        decode_info->referenceSlotCount = 1;
        decode_info->pReferenceSlots = &slot;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pNext-09255");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-slotIndex-09263");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    // Missing reference slot for DPB slot index referred to by referenceNameSlotIndices
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;

        picture_info.referenceNameSlotIndices[3] = 0;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-referenceNameSlotIndices-09262");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();

        picture_info.referenceNameSlotIndices[3] = -1;
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecodeAV1, DecodeFilmGrainRequiresDistinct) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - AV1 film grain requires distinct reconstructed picture");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsDecodeAV1FilmGrain()));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 decode support with reference pictures and film grain";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    // It is possible that the DPB uses a different format, so ignore format mismatch errors
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07143");

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07140");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07146");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-09249");
    cb.DecodeVideo(context.DecodeFrame(0).ApplyFilmGrain().SetDecodeOutput(context.Dpb()->Picture(0)));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}
