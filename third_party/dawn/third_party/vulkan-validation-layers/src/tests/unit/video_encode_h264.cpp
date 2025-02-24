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

class NegativeVideoEncodeH264 : public VkVideoLayerTest {};

TEST_F(NegativeVideoEncodeH264, ProfileMissingCodecInfo) {
    TEST_DESCRIPTION("Test missing codec-specific structure in profile definition");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support";
    }

    VkVideoProfileInfoKHR profile = *config.Profile();
    profile.pNext = nullptr;

    // Missing codec-specific info for H.264 encode profile
    m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-07181");
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(Gpu(), &profile, config.Caps());
    m_errorMonitor->VerifyFound();

    // For encode this VU is also relevant for quality level property queries
    auto quality_level_info = vku::InitStruct<VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR>();
    quality_level_info.pVideoProfile = &profile;

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR-pVideoProfile-08259");
    m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-07181");
    vk::GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(Gpu(), &quality_level_info, config.EncodeQualityLevelProps());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeH264, CapabilityQueryMissingChain) {
    TEST_DESCRIPTION("vkGetPhysicalDeviceVideoCapabilitiesKHR - missing return structures in chain");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support";
    }

    auto caps = vku::InitStruct<VkVideoCapabilitiesKHR>();
    auto encode_caps = vku::InitStruct<VkVideoEncodeCapabilitiesKHR>();
    auto encode_h264_caps = vku::InitStruct<VkVideoEncodeH264CapabilitiesKHR>();

    // Missing encode caps struct for encode profile
    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07186");
    caps.pNext = &encode_h264_caps;
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(Gpu(), config.Profile(), &caps);
    m_errorMonitor->VerifyFound();

    // Missing H.264 encode caps struct for H.264 encode profile
    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07187");
    caps.pNext = &encode_caps;
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(Gpu(), config.Profile(), &caps);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeH264, QualityLevelPropsMissingChain) {
    TEST_DESCRIPTION("vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR - missing return structures in chain");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support";
    }

    auto quality_level_info = vku::InitStruct<VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR>();
    auto quality_level_props = vku::InitStruct<VkVideoEncodeQualityLevelPropertiesKHR>();

    quality_level_info.pVideoProfile = config.Profile();

    // Missing codec-specific output structure for H.264 encode profile
    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR-pQualityLevelInfo-08257");
    vk::GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(Gpu(), &quality_level_info, &quality_level_props);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeH264, CreateSessionInvalidMaxLevel) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid H.264 maxLevelIdc for encode session");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    auto h264_create_info = vku::InitStruct<VkVideoEncodeH264SessionCreateInfoKHR>();
    h264_create_info.pNext = create_info.pNext;
    create_info.pNext = &h264_create_info;

    auto unsupported_level = static_cast<int32_t>(config.EncodeCapsH264()->maxLevelIdc) + 1;
    h264_create_info.maxLevelIdc = static_cast<StdVideoH264LevelIdc>(unsupported_level);

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pVideoProfile-08251");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeH264, CreateSessionParamsMissingCodecInfo) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - missing codec-specific chained structure");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support";
    }

    VideoContext context(m_device, config);

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    auto other_codec_info = vku::InitStruct<VkVideoEncodeH265SessionParametersCreateInfoKHR>();
    other_codec_info.maxStdVPSCount = 1;
    other_codec_info.maxStdSPSCount = 1;
    other_codec_info.maxStdPPSCount = 1;
    create_info.pNext = &other_codec_info;
    create_info.videoSession = context.Session();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07210");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeH264, CreateSessionParamsExceededCapacity) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - H.264 encode parameter set capacity exceeded");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigEncodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support";
    }

    VideoContext context(m_device, config);

    auto h264_ci = vku::InitStruct<VkVideoEncodeH264SessionParametersCreateInfoKHR>();
    auto h264_ai = vku::InitStruct<VkVideoEncodeH264SessionParametersAddInfoKHR>();

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

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04839");
    h264_ci.maxStdSPSCount = 2;
    h264_ci.maxStdPPSCount = 8;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04840");
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

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04839");
    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 8;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04840");
    h264_ci.maxStdSPSCount = 4;
    h264_ci.maxStdPPSCount = 7;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    create_info.videoSessionParametersTemplate = params;
    h264_ci.pParametersAddInfo = nullptr;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04839");
    h264_ci.maxStdSPSCount = 2;
    h264_ci.maxStdPPSCount = 8;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04840");
    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 5;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideoEncodeH264, SessionParamsAddInfoUniqueness) {
    TEST_DESCRIPTION("VkVideoEncodeH264SessionParametersAddInfoKHR - parameter set uniqueness");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigEncodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support";
    }

    VideoContext context(m_device, config);

    auto h264_ci = vku::InitStruct<VkVideoEncodeH264SessionParametersCreateInfoKHR>();
    auto h264_ai = vku::InitStruct<VkVideoEncodeH264SessionParametersAddInfoKHR>();

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

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264SessionParametersAddInfoKHR-None-04837");
    sps_list[0].seq_parameter_set_id = 3;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    sps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264SessionParametersAddInfoKHR-None-04838");
    pps_list[0].seq_parameter_set_id = 2;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    pps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    h264_ci.pParametersAddInfo = nullptr;
    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264SessionParametersAddInfoKHR-None-04837");
    sps_list[0].seq_parameter_set_id = 3;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    sps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264SessionParametersAddInfoKHR-None-04838");
    pps_list[0].seq_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    pps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideoEncodeH264, UpdateSessionParamsConflictingKeys) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - H.264 encode conflicting parameter set keys");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigEncodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support";
    }

    VideoContext context(m_device, config);

    auto h264_ci = vku::InitStruct<VkVideoEncodeH264SessionParametersCreateInfoKHR>();
    auto h264_ai = vku::InitStruct<VkVideoEncodeH264SessionParametersAddInfoKHR>();

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

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07226");
    sps_list2[1].seq_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    sps_list2[1].seq_parameter_set_id = 5;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07227");
    pps_list2[2].seq_parameter_set_id = 1;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    pps_list2[2].seq_parameter_set_id = 4;
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideoEncodeH264, UpdateSessionParamsExceededCapacity) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - H.264 encode parameter set capacity exceeded");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigEncodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support";
    }

    VideoContext context(m_device, config);

    auto h264_ci = vku::InitStruct<VkVideoEncodeH264SessionParametersCreateInfoKHR>();
    auto h264_ai = vku::InitStruct<VkVideoEncodeH264SessionParametersAddInfoKHR>();

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

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-06441");
    h264_ai.stdSPSCount = 2;
    h264_ai.stdPPSCount = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-06442");
    h264_ai.stdSPSCount = 1;
    h264_ai.stdPPSCount = 4;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideoEncodeH264, GetEncodedSessionParamsMissingCodecInfo) {
    TEST_DESCRIPTION("vkGetEncodedVideoSessionParametersKHR - missing codec-specific information");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigEncodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support";
    }

    VideoContext context(m_device, config);

    auto get_info = vku::InitStruct<VkVideoEncodeSessionParametersGetInfoKHR>();
    get_info.videoSessionParameters = context.SessionParams();
    size_t data_size = 0;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08262");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeH264, GetEncodedSessionParams) {
    TEST_DESCRIPTION("vkGetEncodedVideoSessionParametersKHR - H.264 specific parameters are invalid");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support";
    }

    VideoContext context(m_device, config);

    auto h264_info = vku::InitStruct<VkVideoEncodeH264SessionParametersGetInfoKHR>();
    auto get_info = vku::InitStruct<VkVideoEncodeSessionParametersGetInfoKHR>(&h264_info);
    get_info.videoSessionParameters = context.SessionParams();
    size_t data_size = 0;

    // Need to request writing at least one parameter set
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264SessionParametersGetInfoKHR-writeStdSPS-08279");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    // Trying to request non-existent SPS
    h264_info = vku::InitStructHelper();
    h264_info.writeStdSPS = VK_TRUE;
    h264_info.stdSPSId = 1;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08263");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    // Trying to request non-existent PPS
    h264_info = vku::InitStructHelper();
    h264_info.writeStdPPS = VK_TRUE;
    h264_info.stdPPSId = 1;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08264");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    h264_info.stdSPSId = 1;
    h264_info.stdPPSId = 0;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08264");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    // Trying to request both non-existent SPS and PPS
    h264_info = vku::InitStructHelper();
    h264_info.writeStdSPS = VK_TRUE;
    h264_info.writeStdPPS = VK_TRUE;
    h264_info.stdSPSId = 1;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08263");
    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08264");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeH264, BeginCodingRequiresSessionParams) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - H.264 encode requires session parameters");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoBeginCodingInfoKHR begin_info = context.Begin();
    begin_info.videoSessionParameters = VK_NULL_HANDLE;

    cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-videoSession-07249");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();
    cb.End();
}

TEST_F(NegativeVideoEncodeH264, BeginCodingMissingGopRemainingFrames) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - Encode H.264 useGopRemainingFrames missing when required");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeH264()), [](const VideoConfig& config) {
        return config.EncodeCapsH264()->requiresGopRemainingFrames == VK_TRUE;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires an H.264 encode profile with rate control and requiresGopRemainingFrames == VK_TRUE";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto gop_remaining_frames_info = vku::InitStruct<VkVideoEncodeH264GopRemainingFrameInfoKHR>();

    auto rc_layer = vku::InitStruct<VkVideoEncodeRateControlLayerInfoKHR>();
    rc_layer.averageBitrate = 32000;
    rc_layer.maxBitrate = 32000;
    rc_layer.frameRateNumerator = 15;
    rc_layer.frameRateDenominator = 1;

    auto rc_info_h264 = vku::InitStruct<VkVideoEncodeH264RateControlInfoKHR>();
    rc_info_h264.flags = VK_VIDEO_ENCODE_H264_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_info_h264.gopFrameCount = 15;

    auto rc_info = vku::InitStruct<VkVideoEncodeRateControlInfoKHR>(&rc_info_h264);
    rc_info.rateControlMode = config.GetAnySupportedRateControlMode();
    rc_info.layerCount = 1;
    rc_info.pLayers = &rc_layer;
    rc_info.virtualBufferSizeInMs = 1000;
    rc_info.initialVirtualBufferSizeInMs = 0;

    VkVideoBeginCodingInfoKHR begin_info = context.Begin();
    begin_info.pNext = &rc_info;

    cb.Begin();

    // Test with VkVideoEncodeH264GopRemainingFrameInfoKHR missing from pNext chain
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-pBeginInfo-08255");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();

    // Test with useGopRemainingFrames set to VK_FALSE
    gop_remaining_frames_info.useGopRemainingFrames = VK_FALSE;
    gop_remaining_frames_info.pNext = begin_info.pNext;
    begin_info.pNext = &gop_remaining_frames_info;
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-pBeginInfo-08255");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideoEncodeH264, RateControlConstantQpNonZero) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - constantQp must be zero for H.264 if rate control mode is not DISABLED");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeH264()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));

    VideoEncodeInfo encode_info = context.EncodeFrame();
    encode_info.CodecInfo().encode_h264.slice_info.constantQp = 1;

    if (config.EncodeCapsH264()->maxQp < 1) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-constantQp-08270");
    }
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-constantQp-08269");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeH264, RateControlConstantQpNotInCapRange) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - constantQp must be within H.264 minQp/maxQp caps");

    RETURN_IF_SKIP(Init());

    VideoConfig config =
        GetConfig(GetConfigsWithRateControl(GetConfigsEncodeH264(), VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with DISABLED rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config);
    rc_info->rateControlMode = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));

    VideoEncodeInfo encode_info = context.EncodeFrame();

    encode_info.CodecInfo().encode_h264.slice_info.constantQp = config.EncodeCapsH264()->minQp - 1;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-constantQp-08270");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    encode_info.CodecInfo().encode_h264.slice_info.constantQp = config.EncodeCapsH264()->maxQp + 1;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-constantQp-08270");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeH264, RateControlConstantQpPerSliceMismatch) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - constantQp must match across H.264 slices "
        "if VK_VIDEO_ENCODE_H264_CAPABILITY_PER_SLICE_CONSTANT_QP_BIT_KHR is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(
        GetConfigsWithRateControl(GetConfigsEncodeH264(), VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR),
        [](const VideoConfig& config) {
            return config.EncodeCapsH264()->maxSliceCount > 1 &&
                   (config.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_PER_SLICE_CONSTANT_QP_BIT_KHR) == 0;
        }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with DISABLED rate control, "
                        "support for multiple slices but no support for per-slice constant QP";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config);
    rc_info->rateControlMode = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));

    VideoEncodeInfo encode_info = context.EncodeFrame();
    std::vector<VkVideoEncodeH264NaluSliceInfoKHR> slices(2, encode_info.CodecInfo().encode_h264.slice_info);
    encode_info.CodecInfo().encode_h264.picture_info.naluSliceEntryCount = 2;
    encode_info.CodecInfo().encode_h264.picture_info.pNaluSliceEntries = slices.data();

    slices[0].constantQp = config.EncodeCapsH264()->minQp;
    slices[1].constantQp = config.EncodeCapsH264()->maxQp;

    if (slices[0].constantQp == slices[1].constantQp) {
        slices[1].constantQp += 1;
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-constantQp-08270");
    }

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-naluSliceEntryCount-08302");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-naluSliceEntryCount-08312");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-constantQp-08271");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeH264, RateControlLayerCountMismatch) {
    TEST_DESCRIPTION(
        "vkCmdBegin/ControlVideoCodingKHR - when using more than one rate control layer "
        "the layer count must match the H.264 temporal layer count");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithMultiLayerRateControl(GetConfigsEncodeH264()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with multi-layer rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config, true).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));

    cb.Begin();

    rc_info.CodecInfo().encode_h264.temporalLayerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-07022");
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    if (config.EncodeCapsH264()->maxTemporalLayerCount > 2) {
        rc_info.CodecInfo().encode_h264.temporalLayerCount = 3;
        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-07022");
        cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
        m_errorMonitor->VerifyFound();
    }

    cb.BeginVideoCoding(context.Begin());

    rc_info.CodecInfo().encode_h264.temporalLayerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-07022");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    if (config.EncodeCapsH264()->maxTemporalLayerCount > 2) {
        rc_info.CodecInfo().encode_h264.temporalLayerCount = 3;
        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-07022");
        cb.ControlVideoCoding(context.Control().RateControl(rc_info));
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeH264, RateControlHrdCompliance) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - H.264 HRD compliant rate control depends on capability");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeH264()), [](const VideoConfig& config) {
        return (config.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_HRD_COMPLIANCE_BIT_KHR) == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode without HRD compliance support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    auto rc_info = VideoEncodeRateControlInfo(config, true).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    rc_info.CodecInfo().encode_h264.flags = VK_VIDEO_ENCODE_H264_RATE_CONTROL_ATTEMPT_HRD_COMPLIANCE_BIT_KHR;

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264RateControlInfoKHR-flags-08280");
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    m_errorMonitor->VerifyFound();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264RateControlInfoKHR-flags-08280");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeH264, RateControlInvalidCodecInfo) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - invalid H.264 rate control info");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeH264()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    auto rc_info = VideoEncodeRateControlInfo(config, true).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    const char* vuid = nullptr;

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    // Reference pattern without regular GOP flag
    vuid = "VUID-VkVideoEncodeH264RateControlInfoKHR-flags-08281";

    rc_info.CodecInfo().encode_h264.gopFrameCount = 8;

    rc_info.CodecInfo().encode_h264.flags = VK_VIDEO_ENCODE_H264_RATE_CONTROL_REFERENCE_PATTERN_FLAT_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_h264.flags |= VK_VIDEO_ENCODE_H264_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h264.flags = VK_VIDEO_ENCODE_H264_RATE_CONTROL_REFERENCE_PATTERN_DYADIC_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_h264.flags |= VK_VIDEO_ENCODE_H264_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info);

    // Conflicting reference pattern flags
    vuid = "VUID-VkVideoEncodeH264RateControlInfoKHR-flags-08282";

    rc_info.CodecInfo().encode_h264.flags = VK_VIDEO_ENCODE_H264_RATE_CONTROL_REGULAR_GOP_BIT_KHR |
                                            VK_VIDEO_ENCODE_H264_RATE_CONTROL_REFERENCE_PATTERN_FLAT_BIT_KHR |
                                            VK_VIDEO_ENCODE_H264_RATE_CONTROL_REFERENCE_PATTERN_DYADIC_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    // Regular GOP flag requires non-zero GOP size
    vuid = "VUID-VkVideoEncodeH264RateControlInfoKHR-flags-08283";

    rc_info.CodecInfo().encode_h264.flags = VK_VIDEO_ENCODE_H264_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h264.gopFrameCount = 0;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    // Invalid IDR period
    vuid = "VUID-VkVideoEncodeH264RateControlInfoKHR-idrPeriod-08284";

    rc_info.CodecInfo().encode_h264.gopFrameCount = 8;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h264.idrPeriod = 4;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_h264.idrPeriod = 8;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h264.gopFrameCount = 9;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_h264.gopFrameCount = 1;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h264.idrPeriod = 0;
    rc_test_utils.TestRateControlInfo(rc_info);

    // Invalid consecutive B frame count
    vuid = "VUID-VkVideoEncodeH264RateControlInfoKHR-consecutiveBFrameCount-08285";

    rc_info.CodecInfo().encode_h264.gopFrameCount = 4;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h264.consecutiveBFrameCount = 4;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_h264.consecutiveBFrameCount = 3;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h264.gopFrameCount = 1;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_h264.consecutiveBFrameCount = 0;
    rc_test_utils.TestRateControlInfo(rc_info);
}

TEST_F(NegativeVideoEncodeH264, RateControlQpRange) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - H.264 rate control layer QP out of bounds");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeH264()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    const char* expected_min_qp_vuid = "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMinQp-08286";
    const char* allowed_min_qp_vuid = "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMinQp-08288";

    // minQp.qpI not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.minQp.qpI -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qp_vuid, {allowed_min_qp_vuid});
    }

    // minQp.qpP not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.minQp.qpP -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qp_vuid, {allowed_min_qp_vuid});
    }

    // minQp.qpB not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.minQp.qpB -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qp_vuid, {allowed_min_qp_vuid});
    }

    // out of bounds minQp should be ignored if useMinQp is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.useMinQp = VK_FALSE;
        rc_layer.CodecInfo().encode_h264.minQp.qpI -= 1;
        rc_layer.CodecInfo().encode_h264.minQp.qpP -= 1;
        rc_layer.CodecInfo().encode_h264.minQp.qpB -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }

    const char* expected_max_qp_vuid = "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMaxQp-08287";
    const char* allowed_max_qp_vuid = "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMaxQp-08289";

    // maxQp.qpI not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.maxQp.qpI += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qp_vuid, {allowed_max_qp_vuid});
    }

    // maxQp.qpP not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.maxQp.qpP += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qp_vuid, {allowed_max_qp_vuid});
    }

    // maxQp.qpB not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.maxQp.qpB += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qp_vuid, {allowed_max_qp_vuid});
    }

    // out of bounds maxQp should be ignored if useMaxQp is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.useMaxQp = VK_FALSE;
        rc_layer.CodecInfo().encode_h264.maxQp.qpI += 1;
        rc_layer.CodecInfo().encode_h264.maxQp.qpP += 1;
        rc_layer.CodecInfo().encode_h264.maxQp.qpB += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }
}

TEST_F(NegativeVideoEncodeH264, RateControlPerPicTypeQp) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - H.264 per picture type min/max QP depends on capability");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeH264()), [](const VideoConfig& config) {
        return (config.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_PER_PICTURE_TYPE_MIN_MAX_QP_BIT_KHR) == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode without per picture type min/max QP support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    const char* expected_min_qp_vuid = "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMinQp-08288";
    const char* allowed_min_qp_vuid = "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMinQp-08286";

    // minQp.qpI does not match the other QP values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.minQp.qpI += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qp_vuid, {allowed_min_qp_vuid});
    }

    // minQp.qpP does not match the other QP values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.minQp.qpP += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qp_vuid, {allowed_min_qp_vuid});
    }

    // minQp.qpB does not match the other QP values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.minQp.qpB += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qp_vuid, {allowed_min_qp_vuid});
    }

    // non-matching QP values in minQp should be ignored if useMinQp is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.useMinQp = VK_FALSE;
        rc_layer.CodecInfo().encode_h264.minQp.qpP += 1;
        rc_layer.CodecInfo().encode_h264.minQp.qpB += 2;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }

    const char* expected_max_qp_vuid = "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMaxQp-08289";
    const char* allowed_max_qp_vuid = "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMaxQp-08287";

    // maxQp.qpI does not match the other QP values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.maxQp.qpI -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qp_vuid, {allowed_max_qp_vuid});
    }

    // maxQp.qpP does not match the other QP values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.maxQp.qpP -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qp_vuid, {allowed_max_qp_vuid});
    }

    // maxQp.qpB does not match the other QP values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.maxQp.qpB -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qp_vuid, {allowed_max_qp_vuid});
    }

    // non-matching QP values in maxQp should be ignored if useMaxQp is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.useMaxQp = VK_FALSE;
        rc_layer.CodecInfo().encode_h264.maxQp.qpP -= 1;
        rc_layer.CodecInfo().encode_h264.maxQp.qpB -= 2;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }
}

TEST_F(NegativeVideoEncodeH264, RateControlMinQpGreaterThanMaxQp) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - H.264 rate control layer minQp > maxQp");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeH264()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    const char* expected_vuid = "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMinQp-08374";
    std::vector<const char*> allowed_vuids = {
        "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMaxQp-08287",
        "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMinQp-08288",
        "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMaxQp-08289",
    };

    // minQp.qpI > maxQp.qpI
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.minQp.qpI = rc_layer.CodecInfo().encode_h264.maxQp.qpI;
        rc_layer.CodecInfo().encode_h264.maxQp.qpI -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_vuid, allowed_vuids);
    }

    // minQp.qpP > maxQp.qpP
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.minQp.qpP = rc_layer.CodecInfo().encode_h264.maxQp.qpP;
        rc_layer.CodecInfo().encode_h264.maxQp.qpP -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_vuid, allowed_vuids);
    }

    // minQp.qpB > maxQp.qpB
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.minQp.qpB = rc_layer.CodecInfo().encode_h264.maxQp.qpB;
        rc_layer.CodecInfo().encode_h264.maxQp.qpB -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_vuid, allowed_vuids);
    }

    // minQp can be equal to maxQp
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.minQp.qpI = rc_layer.CodecInfo().encode_h264.maxQp.qpI;
        rc_layer.CodecInfo().encode_h264.minQp.qpP = rc_layer.CodecInfo().encode_h264.maxQp.qpP;
        rc_layer.CodecInfo().encode_h264.minQp.qpB = rc_layer.CodecInfo().encode_h264.maxQp.qpB;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }

    // minQp can be larger than maxQp if useMinQp or useMaxQp is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        std::swap(rc_layer.CodecInfo().encode_h264.minQp, rc_layer.CodecInfo().encode_h264.maxQp);

        rc_layer.CodecInfo().encode_h264.useMinQp = VK_FALSE;
        rc_layer.CodecInfo().encode_h264.useMaxQp = VK_TRUE;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);

        rc_layer.CodecInfo().encode_h264.useMinQp = VK_TRUE;
        rc_layer.CodecInfo().encode_h264.useMaxQp = VK_FALSE;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }
}

TEST_F(NegativeVideoEncodeH264, RateControlStateMismatch) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - H.264 rate control state specified does not match current configuration");

    RETURN_IF_SKIP(Init());

    // Try to find a config that supports both CBR and VBR
    VkVideoEncodeRateControlModeFlagsKHR rc_modes =
        VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR | VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR;
    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeH264(), [rc_modes](const VideoConfig& config) {
        return (config.EncodeCaps()->rateControlModes & rc_modes) == rc_modes;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires an H.264 encode profile that supports both CBR and VBR rate control modes";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    const bool include_codec_info = true;

    auto rc_info = VideoEncodeRateControlInfo(config, include_codec_info);
    rc_info->rateControlMode = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR;
    rc_info->virtualBufferSizeInMs = 3000;
    rc_info->initialVirtualBufferSizeInMs = 1000;

    rc_info.CodecInfo().encode_h264.flags = VK_VIDEO_ENCODE_H264_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_info.CodecInfo().encode_h264.gopFrameCount = 15;
    rc_info.CodecInfo().encode_h264.idrPeriod = 60;
    rc_info.CodecInfo().encode_h264.consecutiveBFrameCount = 2;
    rc_info.CodecInfo().encode_h264.temporalLayerCount = config.EncodeCaps()->maxRateControlLayers;

    for (uint32_t i = 0; i < config.EncodeCaps()->maxRateControlLayers; ++i) {
        auto rc_layer = VideoEncodeRateControlLayerInfo(config, include_codec_info);

        rc_layer->averageBitrate = 32000 / (i + 1);
        rc_layer->maxBitrate = 48000 / (i + 1);
        rc_layer->frameRateNumerator = 30;
        rc_layer->frameRateDenominator = i + 1;

        rc_layer.CodecInfo().encode_h264.useMinQp = VK_TRUE;
        rc_layer.CodecInfo().encode_h264.minQp.qpI = config.ClampH264Qp(0 + i);
        rc_layer.CodecInfo().encode_h264.minQp.qpP = config.ClampH264Qp(5 + i);
        rc_layer.CodecInfo().encode_h264.minQp.qpB = config.ClampH264Qp(10 + i);

        if ((config.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_PER_PICTURE_TYPE_MIN_MAX_QP_BIT_KHR) == 0) {
            rc_layer.CodecInfo().encode_h264.minQp.qpP = rc_layer.CodecInfo().encode_h264.minQp.qpI;
            rc_layer.CodecInfo().encode_h264.minQp.qpB = rc_layer.CodecInfo().encode_h264.minQp.qpI;
        }

        rc_layer.CodecInfo().encode_h264.useMaxFrameSize = VK_TRUE;
        rc_layer.CodecInfo().encode_h264.maxFrameSize.frameISize = 30000 / (i + 1);
        rc_layer.CodecInfo().encode_h264.maxFrameSize.framePSize = 20000 / (i + 1);
        rc_layer.CodecInfo().encode_h264.maxFrameSize.frameBSize = 10000 / (i + 1);

        rc_info.AddLayer(rc_layer);
    }

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset().RateControl(rc_info));
    cb.EndVideoCoding(context.End());
    cb.End();

    context.Queue().Submit(cb);
    m_device->Wait();

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    // flags mismatch
    VkVideoEncodeH264RateControlFlagsKHR flags = VK_VIDEO_ENCODE_H264_RATE_CONTROL_TEMPORAL_LAYER_PATTERN_DYADIC_BIT_KHR;
    std::swap(rc_info.CodecInfo().encode_h264.flags, flags);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.CodecInfo().encode_h264.flags, flags);

    // gopFrameCount mismatch
    uint32_t gop_frame_count = 12;
    std::swap(rc_info.CodecInfo().encode_h264.gopFrameCount, gop_frame_count);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.CodecInfo().encode_h264.gopFrameCount, gop_frame_count);

    // idrPeriod mismatch
    uint32_t idr_period = 30;
    std::swap(rc_info.CodecInfo().encode_h264.idrPeriod, idr_period);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.CodecInfo().encode_h264.idrPeriod, idr_period);

    // consecutiveBFrameCount mismatch
    uint32_t cons_b_frames = 4;
    std::swap(rc_info.CodecInfo().encode_h264.consecutiveBFrameCount, cons_b_frames);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.CodecInfo().encode_h264.consecutiveBFrameCount, cons_b_frames);

    // Layer useMinQp mismatch
    rc_info.Layer(0).CodecInfo().encode_h264.useMinQp = VK_FALSE;
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_h264.useMinQp = VK_TRUE;

    // Layer minQp.qpB mismatch
    rc_info.Layer(0).CodecInfo().encode_h264.minQp.qpB -= 1;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMinQp-08286");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMinQp-08288");
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_h264.minQp.qpB += 1;

    // Layer useMaxQp mismatch
    rc_info.Layer(0).CodecInfo().encode_h264.useMaxQp = VK_TRUE;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMaxQp-08287");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMaxQp-08289");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMinQp-08374");
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_h264.useMaxQp = VK_FALSE;

    // Layer maxQp.qpP mismatch
    rc_info.Layer(0).CodecInfo().encode_h264.maxQp.qpP += 1;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMaxQp-08287");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMaxQp-08289");
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_h264.maxQp.qpP -= 1;

    // Layer useMaxFrameSize mismatch
    rc_info.Layer(0).CodecInfo().encode_h264.useMaxFrameSize = VK_FALSE;
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_h264.useMaxFrameSize = VK_TRUE;

    // Layer maxFrameSize.frameISize mismatch
    uint32_t max_frame_i_size = 12345;
    std::swap(rc_info.Layer(0).CodecInfo().encode_h264.maxFrameSize.frameISize, max_frame_i_size);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.Layer(0).CodecInfo().encode_h264.maxFrameSize.frameISize, max_frame_i_size);
}

TEST_F(NegativeVideoEncodeH264, EncodeCapsGenPrefixNalu) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - generating H.264 prefix NALU is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeH264(), [](const VideoConfig& config) {
        return (config.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_GENERATE_PREFIX_NALU_BIT_KHR) == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support without prefix NALU generation support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();
    encode_info.CodecInfo().encode_h264.picture_info.generatePrefixNalu = VK_TRUE;

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264PictureInfoKHR-flags-08304");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeH264, EncodeCapsMaxSliceCount) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - test H.264 maxSliceCount capability");

    RETURN_IF_SKIP(Init());

    // Find the H.264 config with the highest slice count supported
    // so that we don't just check a config with only a single slice supported if possible
    auto configs = GetConfigsEncodeH264();
    VideoConfig config;
    uint32_t max_slice_count = 0;
    for (auto& cfg : configs) {
        if (cfg.EncodeCapsH264()->maxSliceCount > max_slice_count) {
            config = cfg;
            max_slice_count = cfg.EncodeCapsH264()->maxSliceCount;
        }
    }
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();

    const uint32_t slice_count = max_slice_count + 1;
    std::vector<VkVideoEncodeH264NaluSliceInfoKHR> slices(slice_count, encode_info.CodecInfo().encode_h264.slice_info);
    encode_info.CodecInfo().encode_h264.picture_info.naluSliceEntryCount = slice_count;
    encode_info.CodecInfo().encode_h264.picture_info.pNaluSliceEntries = slices.data();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-naluSliceEntryCount-08302");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-naluSliceEntryCount-08312");
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264PictureInfoKHR-naluSliceEntryCount-08301");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeH264, EncodeCapsMoreSlicesThanMBs) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - more H.264 slices are requested than MBs "
        "when VK_VIDEO_ENCODE_H264_CAPABILITY_ROW_UNALIGNED_SLICE_BIT_KHR is supported");

    RETURN_IF_SKIP(Init());

    // Find the H.264 config with the highest slice count supported
    // so that we don't just check a config with only a single slice supported if possible
    auto configs = GetConfigsEncodeH264();
    VideoConfig config;
    uint32_t max_slice_count = 0;
    for (auto& cfg : configs) {
        if ((cfg.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_ROW_UNALIGNED_SLICE_BIT_KHR) != 0 &&
            cfg.EncodeCapsH264()->maxSliceCount > max_slice_count) {
            config = cfg;
            max_slice_count = cfg.EncodeCapsH264()->maxSliceCount;
        }
    }
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode with row unaligned slice supported";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();

    const uint32_t slice_count = config.MaxEncodeH264MBCount() + 1;
    std::vector<VkVideoEncodeH264NaluSliceInfoKHR> slices(slice_count, encode_info.CodecInfo().encode_h264.slice_info);
    encode_info.CodecInfo().encode_h264.picture_info.naluSliceEntryCount = slice_count;
    encode_info.CodecInfo().encode_h264.picture_info.pNaluSliceEntries = slices.data();

    if (slice_count > config.EncodeCapsH264()->maxSliceCount) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH264PictureInfoKHR-naluSliceEntryCount-08301");
    }
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-naluSliceEntryCount-08302");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeH264, EncodeCapsMoreSlicesThanMBRows) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - more H.264 slices are requested than MB rows "
        "when VK_VIDEO_ENCODE_H264_CAPABILITY_ROW_UNALIGNED_SLICE_BIT_KHR is not supported");

    RETURN_IF_SKIP(Init());

    // Find the H.264 config with the highest slice count supported
    // so that we don't just check a config with only a single slice supported if possible
    auto configs = GetConfigsEncodeH264();
    VideoConfig config;
    uint32_t max_slice_count = 0;
    for (auto& cfg : configs) {
        if ((cfg.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_ROW_UNALIGNED_SLICE_BIT_KHR) == 0 &&
            cfg.EncodeCapsH264()->maxSliceCount > max_slice_count) {
            config = cfg;
            max_slice_count = cfg.EncodeCapsH264()->maxSliceCount;
        }
    }
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode without row unaligned slice supported";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();

    const uint32_t slice_count = config.MaxEncodeH264MBRowCount() + 1;
    std::vector<VkVideoEncodeH264NaluSliceInfoKHR> slices(slice_count, encode_info.CodecInfo().encode_h264.slice_info);
    encode_info.CodecInfo().encode_h264.picture_info.naluSliceEntryCount = slice_count;
    encode_info.CodecInfo().encode_h264.picture_info.pNaluSliceEntries = slices.data();

    if (slice_count > config.EncodeCapsH264()->maxSliceCount) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH264PictureInfoKHR-naluSliceEntryCount-08301");
    }
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-naluSliceEntryCount-08312");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeH264, EncodeCapsDifferentSliceTypes) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - using different H.264 slice types is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeH264(), [](const VideoConfig& config) {
        return (config.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_DIFFERENT_SLICE_TYPE_BIT_KHR) == 0 &&
               config.EncodeCapsH264()->maxSliceCount > 1;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with multiple slice support but no different slice types";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();

    const uint32_t slice_count = 2;
    std::vector<VkVideoEncodeH264NaluSliceInfoKHR> slices(slice_count, encode_info.CodecInfo().encode_h264.slice_info);
    std::vector<StdVideoEncodeH264SliceHeader> slice_headers(slice_count, encode_info.CodecInfo().encode_h264.std_slice_header);
    encode_info.CodecInfo().encode_h264.picture_info.naluSliceEntryCount = slice_count;
    encode_info.CodecInfo().encode_h264.picture_info.pNaluSliceEntries = slices.data();

    slices[0].pStdSliceHeader = &slice_headers[0];
    slices[1].pStdSliceHeader = &slice_headers[1];

    slice_headers[0].slice_type = STD_VIDEO_H264_SLICE_TYPE_I;
    slice_headers[1].slice_type = STD_VIDEO_H264_SLICE_TYPE_P;

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264PictureInfoKHR-flags-08315");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeH264, EncodeCapsWeightTable) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - H.264 weight table is required for explicit sample prediction if "
        "VK_VIDEO_ENCODE_H264_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config_p = GetConfig(FilterConfigs(GetConfigsEncodeH264(), [](const VideoConfig& config) {
        return (config.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR) == 0 &&
               config.EncodeCapsH264()->maxPPictureL0ReferenceCount > 0;
    }));
    VideoConfig config_b = GetConfig(FilterConfigs(GetConfigsEncodeH264(), [](const VideoConfig& config) {
        return (config.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR) == 0 &&
               config.EncodeCapsH264()->maxBPictureL0ReferenceCount > 0 && config.EncodeCapsH264()->maxL1ReferenceCount > 0;
    }));
    if (!config_p && !config_b) {
        GTEST_SKIP() << "Test requires an H.264 encode profile without generated weight table support";
    }

    if (config_p) {
        VideoConfig config = config_p;

        // Enable explicit weighted sample prediction for P pictures
        config.EncodeH264PPS()->flags.weighted_pred_flag = 1;

        config.SessionCreateInfo()->maxDpbSlots = 2;
        config.SessionCreateInfo()->maxActiveReferencePictures = 1;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

        VideoEncodeInfo encode_info = context.EncodeFrame(1).AddReferenceFrame(0);

        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264PictureInfoKHR-flags-08314");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (config_b) {
        VideoConfig config = config_b;

        // Enable explicit weighted sample prediction for B pictures
        config.EncodeH264PPS()->weighted_bipred_idc = STD_VIDEO_H264_WEIGHTED_BIPRED_IDC_EXPLICIT;

        config.SessionCreateInfo()->maxDpbSlots = 3;
        config.SessionCreateInfo()->maxActiveReferencePictures = 2;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(-1, 2));

        VideoEncodeInfo encode_info = context.EncodeFrame(2).AddReferenceFrame(0).AddBackReferenceFrame(1);

        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264PictureInfoKHR-flags-08314");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }
}

TEST_F(NegativeVideoEncodeH264, EncodeCapsPicType) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - Cannot encode H.264 P or B pictures without capability prerequisites");

    RETURN_IF_SKIP(Init());

    VideoConfig config_no_p = GetConfig(FilterConfigs(GetConfigsEncodeH264(), [](const VideoConfig& config) {
        return config.EncodeCapsH264()->maxPPictureL0ReferenceCount == 0;
    }));
    VideoConfig config_no_b = GetConfig(FilterConfigs(GetConfigsEncodeH264(), [](const VideoConfig& config) {
        return config.EncodeCapsH264()->maxBPictureL0ReferenceCount == 0 && config.EncodeCapsH264()->maxL1ReferenceCount == 0;
    }));
    if (!config_no_p && !config_no_b) {
        GTEST_SKIP() << "Test requires an H.264 encode profile without P or B frame support";
    }

    if (config_no_p) {
        VideoConfig config = config_no_p;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin());

        VideoEncodeInfo encode_info = context.EncodeFrame();
        encode_info.CodecInfo().encode_h264.std_picture_info.primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_P;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxPPictureL0ReferenceCount-08340");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (config_no_b) {
        VideoConfig config = config_no_b;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin());

        VideoEncodeInfo encode_info = context.EncodeFrame();
        encode_info.CodecInfo().encode_h264.std_picture_info.primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_B;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxBPictureL0ReferenceCount-08341");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }
}

TEST_F(NegativeVideoEncodeH264, EncodeCapsRefPicType) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - Cannot reference H.264 P or B pictures without capability prerequisites");

    RETURN_IF_SKIP(Init());

    VideoConfig config_no_p = GetConfig(FilterConfigs(GetConfigsEncodeH264(), [](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > 1 && config.Caps()->maxActiveReferencePictures > 0 &&
               config.EncodeCapsH264()->maxPPictureL0ReferenceCount == 0;
    }));
    VideoConfig config_no_b = GetConfig(FilterConfigs(GetConfigsEncodeH264(), [](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > 1 && config.Caps()->maxActiveReferencePictures > 0 &&
               config.EncodeCapsH264()->maxBPictureL0ReferenceCount == 0 && config.EncodeCapsH264()->maxL1ReferenceCount == 0;
    }));
    if (!config_no_p && !config_no_b) {
        GTEST_SKIP() << "Test requires an H.264 encode profile without P or B frame support";
    }

    if (config_no_p) {
        VideoConfig config = config_no_p;

        config.SessionCreateInfo()->maxDpbSlots = 2;
        config.SessionCreateInfo()->maxActiveReferencePictures = 1;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

        VideoEncodeInfo encode_info = context.EncodeFrame(1).AddReferenceFrame(0);
        encode_info.CodecInfo().encode_h264.std_picture_info.primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_P;
        encode_info.CodecInfo().encode_h264.std_reference_info[0].primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_P;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxPPictureL0ReferenceCount-08340");
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxPPictureL0ReferenceCount-08340");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (config_no_b) {
        VideoConfig config = config_no_b;

        config.SessionCreateInfo()->maxDpbSlots = 2;
        config.SessionCreateInfo()->maxActiveReferencePictures = 1;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

        VideoEncodeInfo encode_info = context.EncodeFrame(1).AddReferenceFrame(0);
        encode_info.CodecInfo().encode_h264.std_picture_info.primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_B;
        encode_info.CodecInfo().encode_h264.std_reference_info[0].primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_B;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-flags-08342");
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxBPictureL0ReferenceCount-08341");
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxBPictureL0ReferenceCount-08341");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }
}

TEST_F(NegativeVideoEncodeH264, EncodeCapsBPicInRefList) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - Cannot reference H.264 B pictures in L0/L1 without capability prerequisites");

    RETURN_IF_SKIP(Init());

    VideoConfig config_no_b_in_l0 = GetConfig(FilterConfigs(GetConfigsEncodeH264(), [](const VideoConfig& config) {
        return (config.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_B_FRAME_IN_L0_LIST_BIT_KHR) == 0 &&
               config.EncodeCapsH264()->maxBPictureL0ReferenceCount > 0;
    }));
    VideoConfig config_no_b_in_l1 = GetConfig(FilterConfigs(GetConfigsEncodeH264(), [](const VideoConfig& config) {
        return (config.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_B_FRAME_IN_L1_LIST_BIT_KHR) == 0 &&
               config.EncodeCapsH264()->maxL1ReferenceCount > 0;
    }));
    if (!config_no_b_in_l0 && !config_no_b_in_l1) {
        GTEST_SKIP() << "Test requires an H.264 encode profile without B frame support in L0 or L1";
    }

    if (config_no_b_in_l0) {
        VideoConfig config = config_no_b_in_l0;

        config.SessionCreateInfo()->maxDpbSlots = 2;
        config.SessionCreateInfo()->maxActiveReferencePictures = 1;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

        VideoEncodeInfo encode_info = context.EncodeFrame(1).AddReferenceFrame(0);
        encode_info.CodecInfo().encode_h264.std_reference_info[0].primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_B;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-flags-08342");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (config_no_b_in_l1) {
        VideoConfig config = config_no_b_in_l1;

        config.SessionCreateInfo()->maxDpbSlots = 2;
        config.SessionCreateInfo()->maxActiveReferencePictures = 1;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

        VideoEncodeInfo encode_info = context.EncodeFrame(1).AddBackReferenceFrame(0);
        encode_info.CodecInfo().encode_h264.std_reference_info[0].primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_B;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-flags-08343");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }
}

TEST_F(NegativeVideoEncodeH264, EncodeInvalidCodecInfo) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - invalid/missing H.264 codec-specific information");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncodeH264()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VideoEncodeInfo encode_info = context.EncodeFrame(0);

    StdVideoEncodeH264PictureInfo std_picture_info{};
    StdVideoEncodeH264ReferenceListsInfo std_ref_lists{};
    StdVideoEncodeH264SliceHeader std_slice_header{};
    auto slice_info = vku::InitStruct<VkVideoEncodeH264NaluSliceInfoKHR>();
    auto picture_info = vku::InitStruct<VkVideoEncodeH264PictureInfoKHR>();
    std_picture_info.pRefLists = &std_ref_lists;
    picture_info.pStdPictureInfo = &std_picture_info;
    picture_info.naluSliceEntryCount = 1;
    picture_info.pNaluSliceEntries = &slice_info;
    slice_info.pStdSliceHeader = &std_slice_header;

    for (uint32_t i = 0; i < STD_VIDEO_H264_MAX_NUM_LIST_REF; ++i) {
        std_ref_lists.RefPicList0[i] = STD_VIDEO_H264_NO_REFERENCE_PICTURE;
        std_ref_lists.RefPicList1[i] = STD_VIDEO_H264_NO_REFERENCE_PICTURE;
    }

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    // Missing H.264 picture info
    {
        encode_info = context.EncodeFrame(0);
        encode_info->pNext = nullptr;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08225");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // No matching SPS/PPS
    {
        encode_info = context.EncodeFrame(0);
        encode_info->pNext = &picture_info;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-StdVideoH264SequenceParameterSet-08226");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-StdVideoH264PictureParameterSet-08227");
        std_picture_info.seq_parameter_set_id = 1;
        cb.EncodeVideo(encode_info);
        std_picture_info.seq_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-StdVideoH264PictureParameterSet-08227");
        std_picture_info.pic_parameter_set_id = 1;
        cb.EncodeVideo(encode_info);
        std_picture_info.pic_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();
    }

    // Missing H.264 setup reference info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        encode_info = context.EncodeFrame(0);
        encode_info->pNext = &picture_info;
        encode_info->pSetupReferenceSlot = &slot;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08228");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Missing H.264 reference info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        encode_info = context.EncodeFrame(1);
        encode_info->pNext = &picture_info;
        encode_info->referenceSlotCount = 1;
        encode_info->pReferenceSlots = &slot;

        std_ref_lists.RefPicList0[0] = 0;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08229");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        std_ref_lists.RefPicList0[0] = STD_VIDEO_H264_NO_REFERENCE_PICTURE;
    }

    // Missing H.264 reference list info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        encode_info = context.EncodeFrame(1);
        encode_info->pNext = &picture_info;
        encode_info->referenceSlotCount = 1;
        encode_info->pReferenceSlots = &slot;

        std_picture_info.pRefLists = nullptr;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pNext-08229");
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08352");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        std_picture_info.pRefLists = &std_ref_lists;
    }

    // Missing H.264 L0 or L1 list reference to reference slot
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        encode_info = context.EncodeFrame(1);
        encode_info->pNext = &picture_info;
        encode_info->referenceSlotCount = 1;
        encode_info->pReferenceSlots = &slot;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pNext-08229");
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08353");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Missing reference slot for DPB index referred to by the H.264 L0 or L1 list
    {
        encode_info = context.EncodeFrame(0);
        encode_info->pNext = &picture_info;

        std_ref_lists.RefPicList0[0] = 0;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08339");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        std_ref_lists.RefPicList0[0] = STD_VIDEO_H264_NO_REFERENCE_PICTURE;

        std_ref_lists.RefPicList1[0] = 0;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08339");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        std_ref_lists.RefPicList1[0] = STD_VIDEO_H264_NO_REFERENCE_PICTURE;
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}
