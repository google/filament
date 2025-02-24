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

class NegativeVideoDecodeH265 : public VkVideoLayerTest {};

TEST_F(NegativeVideoDecodeH265, ProfileMissingCodecInfo) {
    TEST_DESCRIPTION("Test missing codec-specific structure in profile definition");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support";
    }

    VkVideoProfileInfoKHR profile = *config.Profile();
    profile.pNext = nullptr;

    // Missing codec-specific info for H.265 decode profile
    m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-07180");
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(Gpu(), &profile, config.Caps());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoDecodeH265, CapabilityQueryMissingChain) {
    TEST_DESCRIPTION("vkGetPhysicalDeviceVideoCapabilitiesKHR - missing return structures in chain");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support";
    }

    auto caps = vku::InitStruct<VkVideoCapabilitiesKHR>();
    auto decode_caps = vku::InitStruct<VkVideoDecodeCapabilitiesKHR>();
    auto decode_h265_caps = vku::InitStruct<VkVideoDecodeH265CapabilitiesKHR>();

    // Missing decode caps struct for decode profile
    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07183");
    caps.pNext = &decode_h265_caps;
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(Gpu(), config.Profile(), &caps);
    m_errorMonitor->VerifyFound();

    // Missing H.265 decode caps struct for H.265 decode profile
    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07185");
    caps.pNext = &decode_caps;
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(Gpu(), config.Profile(), &caps);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoDecodeH265, CreateSessionParamsMissingCodecInfo) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - missing codec-specific chained structure");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support";
    }

    VideoContext context(m_device, config);

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    auto other_codec_info = vku::InitStruct<VkVideoDecodeH264SessionParametersCreateInfoKHR>();
    other_codec_info.maxStdSPSCount = 1;
    other_codec_info.maxStdPPSCount = 1;
    create_info.pNext = &other_codec_info;
    create_info.videoSession = context.Session();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07206");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoDecodeH265, CreateSessionParamsExceededCapacity) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - H.265 decode parameter set capacity exceeded");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigDecodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support";
    }

    VideoContext context(m_device, config);

    auto h265_ci = vku::InitStruct<VkVideoDecodeH265SessionParametersCreateInfoKHR>();
    auto h265_ai = vku::InitStruct<VkVideoDecodeH265SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params, params2;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h265_ci;
    create_info.videoSession = context.Session();

    h265_ci.pParametersAddInfo = &h265_ai;

    std::vector<StdVideoH265VideoParameterSet> vps_list{
        context.CreateH265VPS(1),
        context.CreateH265VPS(2),
    };

    std::vector<StdVideoH265SequenceParameterSet> sps_list{
        context.CreateH265SPS(1, 1),
        context.CreateH265SPS(1, 2),
        context.CreateH265SPS(2, 1),
        context.CreateH265SPS(2, 3),
    };

    std::vector<StdVideoH265PictureParameterSet> pps_list{
        context.CreateH265PPS(1, 1, 1), context.CreateH265PPS(1, 1, 2), context.CreateH265PPS(1, 2, 1),
        context.CreateH265PPS(2, 1, 3), context.CreateH265PPS(2, 3, 1), context.CreateH265PPS(2, 3, 2),
        context.CreateH265PPS(2, 3, 3),
    };

    h265_ai.stdVPSCount = (uint32_t)vps_list.size();
    h265_ai.pStdVPSs = vps_list.data();
    h265_ai.stdSPSCount = (uint32_t)sps_list.size();
    h265_ai.pStdSPSs = sps_list.data();
    h265_ai.stdPPSCount = (uint32_t)pps_list.size();
    h265_ai.pStdPPSs = pps_list.data();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07207");
    h265_ci.maxStdVPSCount = 1;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 8;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07208");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 3;
    h265_ci.maxStdPPSCount = 9;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07209");
    h265_ci.maxStdVPSCount = 3;
    h265_ci.maxStdSPSCount = 5;
    h265_ci.maxStdPPSCount = 5;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 7;
    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    create_info.videoSessionParametersTemplate = params;
    vps_list[1].vps_video_parameter_set_id = 3;
    sps_list[1].sps_video_parameter_set_id = 3;
    pps_list[1].sps_video_parameter_set_id = 3;
    pps_list[5].sps_video_parameter_set_id = 3;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07207");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 5;
    h265_ci.maxStdPPSCount = 10;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07208");
    h265_ci.maxStdVPSCount = 3;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 9;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07209");
    h265_ci.maxStdVPSCount = 3;
    h265_ci.maxStdSPSCount = 5;
    h265_ci.maxStdPPSCount = 8;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    create_info.videoSessionParametersTemplate = params;
    h265_ci.pParametersAddInfo = nullptr;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07207");
    h265_ci.maxStdVPSCount = 1;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 7;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07208");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 3;
    h265_ci.maxStdPPSCount = 7;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07209");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 6;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideoDecodeH265, SessionParamsAddInfoUniqueness) {
    TEST_DESCRIPTION("VkVideoDecodeH265SessionParametersAddInfoKHR - parameter set uniqueness");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigDecodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    auto h265_ci = vku::InitStruct<VkVideoDecodeH265SessionParametersCreateInfoKHR>();
    auto h265_ai = vku::InitStruct<VkVideoDecodeH265SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h265_ci;
    create_info.videoSession = context.Session();

    h265_ci.maxStdVPSCount = 10;
    h265_ci.maxStdSPSCount = 20;
    h265_ci.maxStdPPSCount = 30;
    h265_ci.pParametersAddInfo = &h265_ai;

    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.pNext = &h265_ai;
    update_info.updateSequenceCount = 1;

    std::vector<StdVideoH265VideoParameterSet> vps_list{
        context.CreateH265VPS(1),
        context.CreateH265VPS(2),
    };

    std::vector<StdVideoH265SequenceParameterSet> sps_list{
        context.CreateH265SPS(1, 1),
        context.CreateH265SPS(1, 2),
        context.CreateH265SPS(2, 1),
        context.CreateH265SPS(2, 3),
    };

    std::vector<StdVideoH265PictureParameterSet> pps_list{
        context.CreateH265PPS(1, 1, 1), context.CreateH265PPS(1, 1, 2), context.CreateH265PPS(1, 2, 1),
        context.CreateH265PPS(2, 1, 3), context.CreateH265PPS(2, 3, 1), context.CreateH265PPS(2, 3, 2),
        context.CreateH265PPS(2, 3, 3),
    };

    h265_ai.stdVPSCount = (uint32_t)vps_list.size();
    h265_ai.pStdVPSs = vps_list.data();
    h265_ai.stdSPSCount = (uint32_t)sps_list.size();
    h265_ai.pStdSPSs = sps_list.data();
    h265_ai.stdPPSCount = (uint32_t)pps_list.size();
    h265_ai.pStdPPSs = pps_list.data();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04833");
    vps_list[0].vps_video_parameter_set_id = 2;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    vps_list[0].vps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04834");
    sps_list[0].sps_video_parameter_set_id = 2;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    sps_list[0].sps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04835");
    pps_list[0].pps_seq_parameter_set_id = 2;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    pps_list[0].pps_seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    h265_ci.pParametersAddInfo = nullptr;
    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04833");
    vps_list[0].vps_video_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    vps_list[0].vps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04834");
    sps_list[0].sps_video_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    sps_list[0].sps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04835");
    pps_list[0].pps_seq_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    pps_list[0].pps_seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideoDecodeH265, UpdateSessionParamsConflictingKeys) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - H.265 decode conflicting parameter set keys");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigDecodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    auto h265_ci = vku::InitStruct<VkVideoDecodeH265SessionParametersCreateInfoKHR>();
    auto h265_ai = vku::InitStruct<VkVideoDecodeH265SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h265_ci;
    create_info.videoSession = context.Session();

    h265_ci.maxStdVPSCount = 10;
    h265_ci.maxStdSPSCount = 20;
    h265_ci.maxStdPPSCount = 30;
    h265_ci.pParametersAddInfo = &h265_ai;

    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.pNext = &h265_ai;
    update_info.updateSequenceCount = 1;

    std::vector<StdVideoH265VideoParameterSet> vps_list{
        context.CreateH265VPS(1),
        context.CreateH265VPS(2),
    };

    std::vector<StdVideoH265SequenceParameterSet> sps_list{
        context.CreateH265SPS(1, 1),
        context.CreateH265SPS(1, 2),
        context.CreateH265SPS(2, 1),
        context.CreateH265SPS(2, 3),
    };

    std::vector<StdVideoH265PictureParameterSet> pps_list{
        context.CreateH265PPS(1, 1, 1), context.CreateH265PPS(1, 1, 2), context.CreateH265PPS(1, 2, 1),
        context.CreateH265PPS(2, 1, 3), context.CreateH265PPS(2, 3, 1), context.CreateH265PPS(2, 3, 2),
        context.CreateH265PPS(2, 3, 3),
    };

    h265_ai.stdVPSCount = (uint32_t)vps_list.size();
    h265_ai.pStdVPSs = vps_list.data();
    h265_ai.stdSPSCount = (uint32_t)sps_list.size();
    h265_ai.pStdSPSs = sps_list.data();
    h265_ai.stdPPSCount = (uint32_t)pps_list.size();
    h265_ai.pStdPPSs = pps_list.data();

    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    std::vector<StdVideoH265VideoParameterSet> vps_list2{context.CreateH265VPS(3)};

    std::vector<StdVideoH265SequenceParameterSet> sps_list2{context.CreateH265SPS(2, 2), context.CreateH265SPS(3, 1)};

    std::vector<StdVideoH265PictureParameterSet> pps_list2{context.CreateH265PPS(1, 2, 3), context.CreateH265PPS(2, 3, 4),
                                                           context.CreateH265PPS(3, 1, 2)};

    h265_ai.stdVPSCount = (uint32_t)vps_list2.size();
    h265_ai.pStdVPSs = vps_list2.data();
    h265_ai.stdSPSCount = (uint32_t)sps_list2.size();
    h265_ai.pStdSPSs = sps_list2.data();
    h265_ai.stdPPSCount = (uint32_t)pps_list2.size();
    h265_ai.pStdPPSs = pps_list2.data();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07220");
    vps_list2[0].vps_video_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    vps_list2[0].vps_video_parameter_set_id = 3;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07222");
    sps_list2[0].sps_seq_parameter_set_id = 3;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    sps_list2[0].sps_seq_parameter_set_id = 2;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07224");
    pps_list2[1].pps_pic_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    pps_list2[1].pps_pic_parameter_set_id = 4;
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideoDecodeH265, UpdateSessionParamsExceededCapacity) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - H.265 decode parameter set capacity exceeded");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigDecodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    auto h265_ci = vku::InitStruct<VkVideoDecodeH265SessionParametersCreateInfoKHR>();
    auto h265_ai = vku::InitStruct<VkVideoDecodeH265SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h265_ci;
    create_info.videoSession = context.Session();

    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 5;
    h265_ci.maxStdPPSCount = 9;
    h265_ci.pParametersAddInfo = &h265_ai;

    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.pNext = &h265_ai;
    update_info.updateSequenceCount = 1;

    std::vector<StdVideoH265VideoParameterSet> vps_list{
        context.CreateH265VPS(1),
        context.CreateH265VPS(2),
    };

    std::vector<StdVideoH265SequenceParameterSet> sps_list{
        context.CreateH265SPS(1, 1),
        context.CreateH265SPS(1, 2),
        context.CreateH265SPS(2, 1),
        context.CreateH265SPS(2, 3),
    };

    std::vector<StdVideoH265PictureParameterSet> pps_list{
        context.CreateH265PPS(1, 1, 1), context.CreateH265PPS(1, 1, 2), context.CreateH265PPS(1, 2, 1),
        context.CreateH265PPS(2, 1, 3), context.CreateH265PPS(2, 3, 1), context.CreateH265PPS(2, 3, 2),
        context.CreateH265PPS(2, 3, 3),
    };

    h265_ai.stdVPSCount = (uint32_t)vps_list.size();
    h265_ai.pStdVPSs = vps_list.data();
    h265_ai.stdSPSCount = (uint32_t)sps_list.size();
    h265_ai.pStdSPSs = sps_list.data();
    h265_ai.stdPPSCount = (uint32_t)pps_list.size();
    h265_ai.pStdPPSs = pps_list.data();

    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    std::vector<StdVideoH265VideoParameterSet> vps_list2{context.CreateH265VPS(3)};

    std::vector<StdVideoH265SequenceParameterSet> sps_list2{context.CreateH265SPS(2, 2), context.CreateH265SPS(3, 1)};

    std::vector<StdVideoH265PictureParameterSet> pps_list2{context.CreateH265PPS(1, 2, 3), context.CreateH265PPS(2, 3, 4),
                                                           context.CreateH265PPS(3, 1, 2)};

    h265_ai.stdVPSCount = (uint32_t)vps_list2.size();
    h265_ai.pStdVPSs = vps_list2.data();
    h265_ai.stdSPSCount = (uint32_t)sps_list2.size();
    h265_ai.pStdSPSs = sps_list2.data();
    h265_ai.stdPPSCount = (uint32_t)pps_list2.size();
    h265_ai.pStdPPSs = pps_list2.data();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07221");
    h265_ai.stdVPSCount = 1;
    h265_ai.stdSPSCount = 1;
    h265_ai.stdPPSCount = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07223");
    h265_ai.stdVPSCount = 0;
    h265_ai.stdSPSCount = 2;
    h265_ai.stdPPSCount = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07225");
    h265_ai.stdVPSCount = 0;
    h265_ai.stdSPSCount = 1;
    h265_ai.stdPPSCount = 3;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideoDecodeH265, BeginCodingRequiresSessionParams) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - H.265 decode requires session parameters");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoBeginCodingInfoKHR begin_info = context.Begin();
    begin_info.videoSessionParameters = VK_NULL_HANDLE;

    cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-videoSession-07248");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();
    cb.End();
}

TEST_F(NegativeVideoDecodeH265, DecodeMissingInlineSessionParams) {
    TEST_DESCRIPTION(
        "vkCmdBeginVideoCodingKHR - H.265 decode requires bound params object when not all params are specified inline");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance2);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support";
    }

    StdVideoH265VideoParameterSet std_vps{};
    StdVideoH265SequenceParameterSet std_sps{};
    StdVideoH265PictureParameterSet std_pps{};

    // Session was not created with VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR
    {
        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().SetSessionParams(VK_NULL_HANDLE));

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-None-10403");
        cb.DecodeVideo(context.DecodeFrame().InlineParamsH265(&std_vps, &std_sps, &std_pps));
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

        // Missing inline VPS
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-None-10403");
        cb.DecodeVideo(context.DecodeFrame().InlineParamsH265(nullptr, &std_sps, &std_pps));
        m_errorMonitor->VerifyFound();

        // Missing inline SPS
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-None-10403");
        cb.DecodeVideo(context.DecodeFrame().InlineParamsH265(&std_vps, nullptr, &std_pps));
        m_errorMonitor->VerifyFound();

        // Missing inline PPS
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-None-10403");
        cb.DecodeVideo(context.DecodeFrame().InlineParamsH265(&std_vps, &std_sps, nullptr));
        m_errorMonitor->VerifyFound();

        // Missing inline VPS and SPS
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-None-10403");
        cb.DecodeVideo(context.DecodeFrame().InlineParamsH265(nullptr, nullptr, &std_pps));
        m_errorMonitor->VerifyFound();

        // Missing inline VPS and PPS
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-None-10403");
        cb.DecodeVideo(context.DecodeFrame().InlineParamsH265(nullptr, &std_sps, nullptr));
        m_errorMonitor->VerifyFound();

        // Missing inline SPS and PPS
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-None-10403");
        cb.DecodeVideo(context.DecodeFrame().InlineParamsH265(&std_vps, nullptr, nullptr));
        m_errorMonitor->VerifyFound();

        // Missing all inline parameter sets
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-None-10403");
        cb.DecodeVideo(context.DecodeFrame().InlineParamsH265(nullptr, nullptr, nullptr));
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }
}

TEST_F(NegativeVideoDecodeH265, DecodeInvalidCodecInfo) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - invalid/missing H.265 codec-specific information");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecodeH265()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support with reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VideoDecodeInfo decode_info = context.DecodeFrame(0);

    StdVideoDecodeH265PictureInfo std_picture_info{};
    auto picture_info = vku::InitStruct<VkVideoDecodeH265PictureInfoKHR>();
    uint32_t slice_segment_offset = 0;
    picture_info.pStdPictureInfo = &std_picture_info;
    picture_info.sliceSegmentCount = 1;
    picture_info.pSliceSegmentOffsets = &slice_segment_offset;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    // Missing H.265 picture info
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = nullptr;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-07158");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    // Slice offsets must be within buffer range
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pSliceSegmentOffsets-07159");
        slice_segment_offset = (uint32_t)decode_info->srcBufferRange;
        cb.DecodeVideo(decode_info);
        slice_segment_offset = 0;
        m_errorMonitor->VerifyFound();
    }

    // No matching VPS/SPS/PPS
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-StdVideoH265VideoParameterSet-07160");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-StdVideoH265SequenceParameterSet-07161");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-StdVideoH265PictureParameterSet-07162");
        std_picture_info.sps_video_parameter_set_id = 1;
        cb.DecodeVideo(decode_info);
        std_picture_info.sps_video_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-StdVideoH265SequenceParameterSet-07161");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-StdVideoH265PictureParameterSet-07162");
        std_picture_info.pps_seq_parameter_set_id = 1;
        cb.DecodeVideo(decode_info);
        std_picture_info.pps_seq_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-StdVideoH265PictureParameterSet-07162");
        std_picture_info.pps_pic_parameter_set_id = 1;
        cb.DecodeVideo(decode_info);
        std_picture_info.pps_pic_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();
    }

    // Missing H.265 setup reference info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;
        decode_info->pSetupReferenceSlot = &slot;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07141");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07163");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    // Missing H.265 reference info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        decode_info = context.DecodeFrame(1);
        decode_info->pNext = &picture_info;
        decode_info->referenceSlotCount = 1;
        decode_info->pReferenceSlots = &slot;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-07164");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecodeH265, DecodeInlineSessionParamsMismatch) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - H.265 decode inline parameter set ID mismatch");

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

    StdVideoH265VideoParameterSet std_vps{};
    StdVideoH265SequenceParameterSet std_sps{};
    StdVideoH265PictureParameterSet std_pps{};

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().SetSessionParams(VK_NULL_HANDLE));

    // No matching VPS
    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-10404");
        std_vps.vps_video_parameter_set_id = 1;
        cb.DecodeVideo(context.DecodeFrame().InlineParamsH265(&std_vps, &std_sps, &std_pps));
        std_vps.vps_video_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();
    }

    // No matching SPS
    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-10405");
        std_sps.sps_video_parameter_set_id = 1;
        cb.DecodeVideo(context.DecodeFrame().InlineParamsH265(&std_vps, &std_sps, &std_pps));
        std_sps.sps_video_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-10405");
        std_sps.sps_seq_parameter_set_id = 1;
        cb.DecodeVideo(context.DecodeFrame().InlineParamsH265(&std_vps, &std_sps, &std_pps));
        std_sps.sps_seq_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();
    }

    // No matching PPS
    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-10406");
        std_pps.sps_video_parameter_set_id = 1;
        cb.DecodeVideo(context.DecodeFrame().InlineParamsH265(&std_vps, &std_sps, &std_pps));
        std_pps.sps_video_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-10406");
        std_pps.pps_seq_parameter_set_id = 1;
        cb.DecodeVideo(context.DecodeFrame().InlineParamsH265(&std_vps, &std_sps, &std_pps));
        std_pps.pps_seq_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-10406");
        std_pps.pps_pic_parameter_set_id = 1;
        cb.DecodeVideo(context.DecodeFrame().InlineParamsH265(&std_vps, &std_sps, &std_pps));
        std_pps.pps_pic_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}
