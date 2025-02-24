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

class NegativeVideoDecodeH264 : public VkVideoLayerTest {};

TEST_F(NegativeVideoDecodeH264, ProfileMissingCodecInfo) {
    TEST_DESCRIPTION("Test missing codec-specific structure in profile definition");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support";
    }

    VkVideoProfileInfoKHR profile = *config.Profile();
    profile.pNext = nullptr;

    // Missing codec-specific info for H.264 decode profile
    m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-07179");
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(Gpu(), &profile, config.Caps());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoDecodeH264, CapabilityQueryMissingChain) {
    TEST_DESCRIPTION("vkGetPhysicalDeviceVideoCapabilitiesKHR - missing return structures in chain");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support";
    }

    auto caps = vku::InitStruct<VkVideoCapabilitiesKHR>();
    auto decode_caps = vku::InitStruct<VkVideoDecodeCapabilitiesKHR>();
    auto decode_h264_caps = vku::InitStruct<VkVideoDecodeH264CapabilitiesKHR>();

    // Missing decode caps struct for decode profile
    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07183");
    caps.pNext = &decode_h264_caps;
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(Gpu(), config.Profile(), &caps);
    m_errorMonitor->VerifyFound();

    // Missing H.264 decode caps struct for H.264 decode profile
    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07184");
    caps.pNext = &decode_caps;
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(Gpu(), config.Profile(), &caps);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoDecodeH264, CreateSessionParamsMissingCodecInfo) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - missing codec-specific chained structure");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support";
    }

    VideoContext context(m_device, config);

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    auto other_codec_info = vku::InitStruct<VkVideoDecodeH265SessionParametersCreateInfoKHR>();
    other_codec_info.maxStdVPSCount = 1;
    other_codec_info.maxStdSPSCount = 1;
    other_codec_info.maxStdPPSCount = 1;
    create_info.pNext = &other_codec_info;
    create_info.videoSession = context.Session();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07203");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoDecodeH264, CreateSessionParamsExceededCapacity) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - H.264 decode parameter set capacity exceeded");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigDecodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support";
    }

    VideoContext context(m_device, config);

    auto h264_ci = vku::InitStruct<VkVideoDecodeH264SessionParametersCreateInfoKHR>();
    auto h264_ai = vku::InitStruct<VkVideoDecodeH264SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params, params2;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h264_ci;
    create_info.videoSession = context.Session();

    h264_ci.pParametersAddInfo = &h264_ai;

    std::vector<StdVideoH264SequenceParameterSet> sps_list{context.CreateH264SPS(1), context.CreateH264SPS(2),
                                                           context.CreateH264SPS(3)};

    std::vector<StdVideoH264PictureParameterSet> pps_list{
        context.CreateH264PPS(1, 1), context.CreateH264PPS(1, 4), context.CreateH264PPS(2, 1),
        context.CreateH264PPS(2, 2), context.CreateH264PPS(3, 1), context.CreateH264PPS(3, 3),
    };

    h264_ai.stdSPSCount = (uint32_t)sps_list.size();
    h264_ai.pStdSPSs = sps_list.data();
    h264_ai.stdPPSCount = (uint32_t)pps_list.size();
    h264_ai.pStdPPSs = pps_list.data();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07204");
    h264_ci.maxStdSPSCount = 2;
    h264_ci.maxStdPPSCount = 8;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07205");
    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 5;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 6;
    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    create_info.videoSessionParametersTemplate = params;
    sps_list[1].seq_parameter_set_id = 4;
    pps_list[1].seq_parameter_set_id = 4;
    pps_list[5].seq_parameter_set_id = 4;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07204");
    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 8;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07205");
    h264_ci.maxStdSPSCount = 4;
    h264_ci.maxStdPPSCount = 7;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    create_info.videoSessionParametersTemplate = params;
    h264_ci.pParametersAddInfo = nullptr;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07204");
    h264_ci.maxStdSPSCount = 2;
    h264_ci.maxStdPPSCount = 8;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07205");
    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 5;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideoDecodeH264, SessionParamsAddInfoUniqueness) {
    TEST_DESCRIPTION("VkVideoDecodeH264SessionParametersAddInfoKHR - parameter set uniqueness");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigDecodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support";
    }

    VideoContext context(m_device, config);

    auto h264_ci = vku::InitStruct<VkVideoDecodeH264SessionParametersCreateInfoKHR>();
    auto h264_ai = vku::InitStruct<VkVideoDecodeH264SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h264_ci;
    create_info.videoSession = context.Session();

    h264_ci.maxStdSPSCount = 10;
    h264_ci.maxStdPPSCount = 20;
    h264_ci.pParametersAddInfo = &h264_ai;

    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.pNext = &h264_ai;
    update_info.updateSequenceCount = 1;

    std::vector<StdVideoH264SequenceParameterSet> sps_list{context.CreateH264SPS(1), context.CreateH264SPS(2),
                                                           context.CreateH264SPS(3)};

    std::vector<StdVideoH264PictureParameterSet> pps_list{
        context.CreateH264PPS(1, 1), context.CreateH264PPS(1, 4), context.CreateH264PPS(2, 1),
        context.CreateH264PPS(2, 2), context.CreateH264PPS(3, 1), context.CreateH264PPS(3, 3),
    };

    h264_ai.stdSPSCount = (uint32_t)sps_list.size();
    h264_ai.pStdSPSs = sps_list.data();
    h264_ai.stdPPSCount = (uint32_t)pps_list.size();
    h264_ai.pStdPPSs = pps_list.data();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH264SessionParametersAddInfoKHR-None-04825");
    sps_list[0].seq_parameter_set_id = 3;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    sps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH264SessionParametersAddInfoKHR-None-04826");
    pps_list[0].seq_parameter_set_id = 2;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    pps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    h264_ci.pParametersAddInfo = nullptr;
    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH264SessionParametersAddInfoKHR-None-04825");
    sps_list[0].seq_parameter_set_id = 3;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    sps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH264SessionParametersAddInfoKHR-None-04826");
    pps_list[0].seq_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    pps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideoDecodeH264, UpdateSessionParamsConflictingKeys) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - H.264 decode conflicting parameter set keys");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigDecodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support";
    }

    VideoContext context(m_device, config);

    auto h264_ci = vku::InitStruct<VkVideoDecodeH264SessionParametersCreateInfoKHR>();
    auto h264_ai = vku::InitStruct<VkVideoDecodeH264SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h264_ci;
    create_info.videoSession = context.Session();

    h264_ci.maxStdSPSCount = 10;
    h264_ci.maxStdPPSCount = 20;
    h264_ci.pParametersAddInfo = &h264_ai;

    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.pNext = &h264_ai;
    update_info.updateSequenceCount = 1;

    std::vector<StdVideoH264SequenceParameterSet> sps_list{context.CreateH264SPS(1), context.CreateH264SPS(2),
                                                           context.CreateH264SPS(3)};

    std::vector<StdVideoH264PictureParameterSet> pps_list{
        context.CreateH264PPS(1, 1), context.CreateH264PPS(1, 4), context.CreateH264PPS(2, 1),
        context.CreateH264PPS(2, 2), context.CreateH264PPS(3, 1), context.CreateH264PPS(3, 3),
    };

    h264_ai.stdSPSCount = (uint32_t)sps_list.size();
    h264_ai.pStdSPSs = sps_list.data();
    h264_ai.stdPPSCount = (uint32_t)pps_list.size();
    h264_ai.pStdPPSs = pps_list.data();

    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    std::vector<StdVideoH264SequenceParameterSet> sps_list2{context.CreateH264SPS(4), context.CreateH264SPS(5)};

    std::vector<StdVideoH264PictureParameterSet> pps_list2{context.CreateH264PPS(1, 3), context.CreateH264PPS(3, 2),
                                                           context.CreateH264PPS(4, 1), context.CreateH264PPS(5, 2)};

    h264_ai.stdSPSCount = (uint32_t)sps_list2.size();
    h264_ai.pStdSPSs = sps_list2.data();
    h264_ai.stdPPSCount = (uint32_t)pps_list2.size();
    h264_ai.pStdPPSs = pps_list2.data();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07216");
    sps_list2[1].seq_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    sps_list2[1].seq_parameter_set_id = 5;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07218");
    pps_list2[2].seq_parameter_set_id = 1;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    pps_list2[2].seq_parameter_set_id = 4;
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideoDecodeH264, UpdateSessionParamsExceededCapacity) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - H.264 decode parameter set capacity exceeded");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigDecodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support";
    }

    VideoContext context(m_device, config);

    auto h264_ci = vku::InitStruct<VkVideoDecodeH264SessionParametersCreateInfoKHR>();
    auto h264_ai = vku::InitStruct<VkVideoDecodeH264SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h264_ci;
    create_info.videoSession = context.Session();

    h264_ci.maxStdSPSCount = 4;
    h264_ci.maxStdPPSCount = 9;
    h264_ci.pParametersAddInfo = &h264_ai;

    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.pNext = &h264_ai;
    update_info.updateSequenceCount = 1;

    std::vector<StdVideoH264SequenceParameterSet> sps_list{context.CreateH264SPS(1), context.CreateH264SPS(2),
                                                           context.CreateH264SPS(3)};

    std::vector<StdVideoH264PictureParameterSet> pps_list{
        context.CreateH264PPS(1, 1), context.CreateH264PPS(1, 4), context.CreateH264PPS(2, 1),
        context.CreateH264PPS(2, 2), context.CreateH264PPS(3, 1), context.CreateH264PPS(3, 3),
    };

    h264_ai.stdSPSCount = (uint32_t)sps_list.size();
    h264_ai.pStdSPSs = sps_list.data();
    h264_ai.stdPPSCount = (uint32_t)pps_list.size();
    h264_ai.pStdPPSs = pps_list.data();

    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    std::vector<StdVideoH264SequenceParameterSet> sps_list2{context.CreateH264SPS(4), context.CreateH264SPS(5)};

    std::vector<StdVideoH264PictureParameterSet> pps_list2{context.CreateH264PPS(1, 3), context.CreateH264PPS(3, 2),
                                                           context.CreateH264PPS(4, 1), context.CreateH264PPS(5, 2)};

    h264_ai.pStdSPSs = sps_list2.data();
    h264_ai.pStdPPSs = pps_list2.data();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07217");
    h264_ai.stdSPSCount = 2;
    h264_ai.stdPPSCount = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07219");
    h264_ai.stdSPSCount = 1;
    h264_ai.stdPPSCount = 4;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideoDecodeH264, BeginCodingRequiresSessionParams) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - H.264 decode requires session parameters");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoBeginCodingInfoKHR begin_info = context.Begin();
    begin_info.videoSessionParameters = VK_NULL_HANDLE;

    cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-videoSession-07247");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();
    cb.End();
}

TEST_F(NegativeVideoDecodeH264, DecodeMissingInlineSessionParams) {
    TEST_DESCRIPTION(
        "vkCmdBeginVideoCodingKHR - H.264 decode requires bound params object when not all params are specified inline");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance2);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support";
    }

    StdVideoH264SequenceParameterSet std_sps{};
    StdVideoH264PictureParameterSet std_pps{};

    // Session was not created with VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR
    {
        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().SetSessionParams(VK_NULL_HANDLE));

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-None-10400");
        cb.DecodeVideo(context.DecodeFrame().InlineParamsH264(&std_sps, &std_pps));
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    // Missing some inline parameters
    {
        config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().SetSessionParams(VK_NULL_HANDLE));

        // Missing inline SPS
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-None-10400");
        cb.DecodeVideo(context.DecodeFrame().InlineParamsH264(nullptr, &std_pps));
        m_errorMonitor->VerifyFound();

        // Missing inline PPS
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-None-10400");
        cb.DecodeVideo(context.DecodeFrame().InlineParamsH264(&std_sps, nullptr));
        m_errorMonitor->VerifyFound();

        // Missing both inline SPS and PPS
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-None-10400");
        cb.DecodeVideo(context.DecodeFrame().InlineParamsH264(nullptr, nullptr));
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }
}

TEST_F(NegativeVideoDecodeH264, DecodeTooManyReferencesInterlaced) {
    TEST_DESCRIPTION(
        "vkCmdDecodeVideoKHR - reference picture count exceeds maxActiveReferencePictures"
        " (specific test for H.264 interlaced with both top and bottom field referenced)");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecodeH264Interlaced(), 2), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with 2 reference pictures and 3 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 3;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(-1, 2));

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-activeReferencePictureCount-07150");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0).AddReferenceBothFields(1));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-activeReferencePictureCount-07150");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceBothFields(1).AddReferenceFrame(0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecodeH264, DecodeDuplicateRefResourceInterlaced) {
    TEST_DESCRIPTION(
        "vkCmdDecodeVideoKHR - same reference picture resource is used twice "
        "with one referring to the top field and another referring to the bottom field");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsDecodeH264Interlaced(), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with 2 reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07264");
    cb.DecodeVideo(context.DecodeFrame(1).AddReferenceTopField(0, 0).AddReferenceBottomField(0, 0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecodeH264, DecodeDuplicateFrameFieldInterlaced) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - same DPB frame, top field, or bottom field reference is used twice");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecodeH264Interlaced(), 4), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with 4 reference pictures and 3 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 3;
    config.SessionCreateInfo()->maxActiveReferencePictures = 4;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(0, 1).AddResource(-1, 2));

    // Same DPB frame is used twice
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-dpbFrameUseCount-07176");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0, 0).AddReferenceFrame(0, 1));
    m_errorMonitor->VerifyFound();

    // Same DPB top field is used twice
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-dpbTopFieldUseCount-07177");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceTopField(0, 0).AddReferenceTopField(0, 1));
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-dpbTopFieldUseCount-07177");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceTopField(0, 0).AddReferenceBothFields(0, 1));
    m_errorMonitor->VerifyFound();

    // Same DPB bottom field is used twice
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-dpbBottomFieldUseCount-07178");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceBottomField(0, 0).AddReferenceBottomField(0, 1));
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-dpbBottomFieldUseCount-07178");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceBottomField(0, 0).AddReferenceBothFields(0, 1));
    m_errorMonitor->VerifyFound();

    // Same DPB top & bottom field is used twice
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-dpbTopFieldUseCount-07177");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-dpbBottomFieldUseCount-07178");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceBothFields(0, 0).AddReferenceBothFields(0, 1));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecodeH264, ImplicitDeactivationInterlaced) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - test DPB slot deactivation caused by H.264 interlaced reference invalidation");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecodeH264Interlaced()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires an H.264 interlaced decode profile with reference picture support and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    {
        // Setup frame in DPB slot then use it as non-setup reconstructed picture for a top field
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceFrame(0));
        cb.DecodeVideo(context.DecodeTopField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to include the DPB slot expecting reference picture association at begin-time
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    {
        // Setup frame in DPB slot then use it as non-setup reconstructed picture for a bottom field
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceFrame(0));
        cb.DecodeVideo(context.DecodeTopField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to include the DPB slot expecting reference picture association at begin-time
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    {
        // Setup top field in DPB slot then use it as non-setup reconstructed picture for a frame
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceTopField(0));
        cb.DecodeVideo(context.DecodeFrame(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to include the DPB slot expecting reference picture association at begin-time
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    {
        // Setup top field in DPB slot then use it as non-setup reconstructed picture for a top field
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceTopField(0));
        cb.DecodeVideo(context.DecodeTopField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to include the DPB slot expecting reference picture association at begin-time
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    {
        // Setup bottom field in DPB slot then use it as non-setup reconstructed picture for a frame
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceBottomField(0));
        cb.DecodeVideo(context.DecodeFrame(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to include the DPB slot expecting reference picture association at begin-time
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    {
        // Setup bottom field in DPB slot then use it as non-setup reconstructed picture for a bottom field
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceTopField(0));
        cb.DecodeVideo(context.DecodeTopField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to include the DPB slot expecting reference picture association at begin-time
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }
}

TEST_F(NegativeVideoDecodeH264, DecodeRefPicKindMismatch) {
    TEST_DESCRIPTION(
        "vkCmdDecodeVideoKHR - H.264 reference picture kind (frame, top field, bottom field) mismatch "
        "between actual DPB slot contents and specified reference pictures");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecodeH264Interlaced()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires an H.264 interlaced decode profile with reference picture support and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    {
        // Setup frame in DPB slot
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceFrame(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to reference DPB slot as top field
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceTopField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07267");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();

        // Try to reference DPB slot as bottom field
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceBottomField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07268");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    {
        // Setup top field in DPB slot
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceTopField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to reference DPB slot as frame
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceFrame(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07266");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();

        // Try to reference DPB slot as bottom field
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceBottomField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07268");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    {
        // Setup bottom field in DPB slot
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceBottomField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to reference DPB slot as frame
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceFrame(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07266");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();

        // Try to reference DPB slot as top field
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceTopField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07267");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }
}

TEST_F(NegativeVideoDecodeH264, InvalidationOnlyInterlaced) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - test H.264 interlaced reference invalidation without implicit DPB slot deactivation");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecodeH264Interlaced()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires an H.264 interlaced decode profile with reference picture support and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    {
        // Setup top and bottom field in DPB slot then use it as non-setup reconstructed picture for a top field
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceTopField(0));
        cb.DecodeVideo(context.DecodeReferenceBottomField(0));
        cb.DecodeVideo(context.DecodeTopField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to reference DPB slot as top field
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceTopField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07267");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    {
        // Setup top and bottom field in DPB slot then use it as non-setup reconstructed picture for a bottom field
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceTopField(0));
        cb.DecodeVideo(context.DecodeReferenceBottomField(0));
        cb.DecodeVideo(context.DecodeBottomField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to reference DPB slot as bottom field
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceBottomField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07268");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }
}

TEST_F(NegativeVideoDecodeH264, DecodeInvalidCodecInfo) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - invalid/missing H.264 codec-specific information");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecodeH264(), 2), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support with 2 reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VideoDecodeInfo decode_info = context.DecodeFrame(0);

    StdVideoDecodeH264PictureInfo std_picture_info{};
    auto picture_info = vku::InitStruct<VkVideoDecodeH264PictureInfoKHR>();
    uint32_t slice_offset = 0;
    picture_info.pStdPictureInfo = &std_picture_info;
    picture_info.sliceCount = 1;
    picture_info.pSliceOffsets = &slice_offset;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    // Missing H.264 picture info
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = nullptr;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-07152");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    // Decode output must be a frame if session does not support interlaced frames
    {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07259");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-None-07258");
        cb.DecodeVideo(context.DecodeTopField(0));
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07259");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-None-07258");
        cb.DecodeVideo(context.DecodeBottomField(0));
        m_errorMonitor->VerifyFound();
    }

    // Slice offsets must be within buffer range
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pSliceOffsets-07153");
        slice_offset = (uint32_t)decode_info->srcBufferRange;
        cb.DecodeVideo(decode_info);
        slice_offset = 0;
        m_errorMonitor->VerifyFound();
    }

    // No matching SPS/PPS
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-StdVideoH264SequenceParameterSet-07154");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-StdVideoH264PictureParameterSet-07155");
        std_picture_info.seq_parameter_set_id = 1;
        cb.DecodeVideo(decode_info);
        std_picture_info.seq_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-StdVideoH264PictureParameterSet-07155");
        std_picture_info.pic_parameter_set_id = 1;
        cb.DecodeVideo(decode_info);
        std_picture_info.pic_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();
    }

    // Missing H.264 setup reference info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;
        decode_info->pSetupReferenceSlot = &slot;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07141");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07156");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    // Reconstructed picture must be a frame if session does not support interlaced frames
    {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07261");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07259");
        cb.DecodeVideo(context.DecodeReferenceTopField(0).SetFrame(true /* override decode output */));
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07261");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07259");
        cb.DecodeVideo(context.DecodeReferenceBottomField(0).SetFrame(true /* override decode output */));
        m_errorMonitor->VerifyFound();
    }

    // Missing H.264 reference info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        decode_info = context.DecodeFrame(1);
        decode_info->pNext = &picture_info;
        decode_info->referenceSlotCount = 1;
        decode_info->pReferenceSlots = &slot;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-07157");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    // Reference picture must be a frame if session does not support interlaced frames
    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07260");
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceTopField(0));
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07260");
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceBottomField(0));
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07260");
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceBothFields(0));
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecodeH264, DecodeFieldFrameMismatch) {
    TEST_DESCRIPTION(
        "vkCmdDecodeVideoKHR - H.264 interlaced field/frame mismatch between "
        "decode output picture and reconstructed picture");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsDecodeH264Interlaced()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 interlaced decode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    // Decode output is frame but reconstructed is top field
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07261");
    cb.DecodeVideo(context.DecodeReferenceTopField(0).SetFrame(true /* override decode output */));
    m_errorMonitor->VerifyFound();

    // Decode output is frame but reconstructed is bottom field
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07261");
    cb.DecodeVideo(context.DecodeReferenceBottomField(0).SetFrame(true /* override decode output */));
    m_errorMonitor->VerifyFound();

    // Decode output is top field but reconstructed is frame
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07262");
    cb.DecodeVideo(context.DecodeReferenceFrame(0).SetTopField(true /* override decode output */));
    m_errorMonitor->VerifyFound();

    // Decode output is top field but reconstructed is bottom field
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07262");
    cb.DecodeVideo(context.DecodeBottomField(0).SetTopField(true /* override decode output */));
    m_errorMonitor->VerifyFound();

    // Decode output is bottom field but reconstructed is frame
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07263");
    cb.DecodeVideo(context.DecodeReferenceFrame(0).SetBottomField(true /* override decode output */));
    m_errorMonitor->VerifyFound();

    // Decode output is bottom field but reconstructed is top field
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07263");
    cb.DecodeVideo(context.DecodeReferenceTopField(0).SetBottomField(true /* override decode output */));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecodeH264, DecodeInlineSessionParamsMismatch) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - H.264 decode inline parameter set ID mismatch");

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

    StdVideoH264SequenceParameterSet std_sps{};
    StdVideoH264PictureParameterSet std_pps{};

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().SetSessionParams(VK_NULL_HANDLE));

    // No matching SPS
    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-10401");
        std_sps.seq_parameter_set_id = 1;
        cb.DecodeVideo(context.DecodeFrame().InlineParamsH264(&std_sps, &std_pps));
        std_sps.seq_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();
    }

    // No matching PPS
    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-10402");
        std_pps.seq_parameter_set_id = 1;
        cb.DecodeVideo(context.DecodeFrame().InlineParamsH264(&std_sps, &std_pps));
        std_pps.seq_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-10402");
        std_pps.pic_parameter_set_id = 1;
        cb.DecodeVideo(context.DecodeFrame().InlineParamsH264(&std_sps, &std_pps));
        std_pps.pic_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}
