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

class NegativeVideoEncodeH265 : public VkVideoLayerTest {};

TEST_F(NegativeVideoEncodeH265, ProfileMissingCodecInfo) {
    TEST_DESCRIPTION("Test missing codec-specific structure in profile definition");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    VkVideoProfileInfoKHR profile = *config.Profile();
    profile.pNext = nullptr;

    // Missing codec-specific info for H.265 encode profile
    m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-07182");
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(Gpu(), &profile, config.Caps());
    m_errorMonitor->VerifyFound();

    // For encode this VU is also relevant for quality level property queries
    auto quality_level_info = vku::InitStruct<VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR>();
    quality_level_info.pVideoProfile = &profile;

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR-pVideoProfile-08259");
    m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-07182");
    vk::GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(Gpu(), &quality_level_info, config.EncodeQualityLevelProps());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeH265, CapabilityQueryMissingChain) {
    TEST_DESCRIPTION("vkGetPhysicalDeviceVideoCapabilitiesKHR - missing return structures in chain");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    auto caps = vku::InitStruct<VkVideoCapabilitiesKHR>();
    auto encode_caps = vku::InitStruct<VkVideoEncodeCapabilitiesKHR>();
    auto encode_h265_caps = vku::InitStruct<VkVideoEncodeH265CapabilitiesKHR>();

    // Missing encode caps struct for encode profile
    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07186");
    caps.pNext = &encode_h265_caps;
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(Gpu(), config.Profile(), &caps);
    m_errorMonitor->VerifyFound();

    // Missing H.265 encode caps struct for H.265 encode profile
    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07188");
    caps.pNext = &encode_caps;
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(Gpu(), config.Profile(), &caps);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeH265, QualityLevelPropsMissingChain) {
    TEST_DESCRIPTION("vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR - missing return structures in chain");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    auto quality_level_info = vku::InitStruct<VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR>();
    auto quality_level_props = vku::InitStruct<VkVideoEncodeQualityLevelPropertiesKHR>();

    quality_level_info.pVideoProfile = config.Profile();

    // Missing codec-specific output structure for H.265 encode profile
    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR-pQualityLevelInfo-08258");
    vk::GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(Gpu(), &quality_level_info, &quality_level_props);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeH265, CreateSessionInvalidMaxLevel) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid H.265 maxLevelIdc for encode session");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    auto h265_create_info = vku::InitStruct<VkVideoEncodeH265SessionCreateInfoKHR>();
    h265_create_info.pNext = create_info.pNext;
    create_info.pNext = &h265_create_info;

    auto unsupported_level = static_cast<int32_t>(config.EncodeCapsH265()->maxLevelIdc) + 1;
    h265_create_info.maxLevelIdc = static_cast<StdVideoH265LevelIdc>(unsupported_level);

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pVideoProfile-08252");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeH265, CreateSessionParamsMissingCodecInfo) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - missing codec-specific chained structure");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    VideoContext context(m_device, config);

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    auto other_codec_info = vku::InitStruct<VkVideoEncodeH264SessionParametersCreateInfoKHR>();
    other_codec_info.maxStdSPSCount = 1;
    other_codec_info.maxStdPPSCount = 1;
    create_info.pNext = &other_codec_info;
    create_info.videoSession = context.Session();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07211");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeH265, CreateSessionParamsExceededCapacity) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - H.265 encode parameter set capacity exceeded");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigEncodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    VideoContext context(m_device, config);

    auto h265_ci = vku::InitStruct<VkVideoEncodeH265SessionParametersCreateInfoKHR>();
    auto h265_ai = vku::InitStruct<VkVideoEncodeH265SessionParametersAddInfoKHR>();

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

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04841");
    h265_ci.maxStdVPSCount = 1;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 8;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04842");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 3;
    h265_ci.maxStdPPSCount = 9;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04843");
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

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04841");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 5;
    h265_ci.maxStdPPSCount = 10;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04842");
    h265_ci.maxStdVPSCount = 3;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 9;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04843");
    h265_ci.maxStdVPSCount = 3;
    h265_ci.maxStdSPSCount = 5;
    h265_ci.maxStdPPSCount = 8;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    create_info.videoSessionParametersTemplate = params;
    h265_ci.pParametersAddInfo = nullptr;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04841");
    h265_ci.maxStdVPSCount = 1;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 7;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04842");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 3;
    h265_ci.maxStdPPSCount = 7;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04843");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 6;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideoEncodeH265, CreateUpdateSessionParamsInvalidTileColumnsRows) {
    TEST_DESCRIPTION("vkCreate/UpdateVideoSessionParametersKHR - H.265 encode PPS contains invalid tile columns/rows");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigEncodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    VideoContext context(m_device, config);

    auto h265_ci = vku::InitStruct<VkVideoEncodeH265SessionParametersCreateInfoKHR>();
    auto h265_ai = vku::InitStruct<VkVideoEncodeH265SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h265_ci;
    create_info.videoSession = context.Session();

    h265_ci.pParametersAddInfo = &h265_ai;

    auto h265_vps = context.CreateH265VPS(1);
    auto h265_sps = context.CreateH265SPS(1, 1);

    std::vector<StdVideoH265PictureParameterSet> h265_pps_list{context.CreateH265PPS(1, 1, 1), context.CreateH265PPS(1, 1, 2),
                                                               context.CreateH265PPS(1, 1, 3), context.CreateH265PPS(1, 1, 4),
                                                               context.CreateH265PPS(1, 1, 5), context.CreateH265PPS(1, 1, 6)};

    h265_ci.maxStdVPSCount = 1;
    h265_ci.maxStdSPSCount = 1;
    h265_ci.maxStdPPSCount = (uint32_t)h265_pps_list.size();

    // Configure some of the PPS entries with various out-of-bounds tile row/column counts

    // Let PPS #2 have out-of-bounds num_tile_columns_minus1
    h265_pps_list[1].num_tile_columns_minus1 = (uint8_t)config.EncodeCapsH265()->maxTiles.width;
    h265_pps_list[1].num_tile_rows_minus1 = (uint8_t)config.EncodeCapsH265()->maxTiles.height / 2;

    // Let PPS #3 use the max limits
    h265_pps_list[2].num_tile_columns_minus1 = (uint8_t)config.EncodeCapsH265()->maxTiles.width - 1;
    h265_pps_list[2].num_tile_rows_minus1 = (uint8_t)config.EncodeCapsH265()->maxTiles.height - 1;

    // Let PPS #5 have out-of-bounds num_tile_rows_minus1
    h265_pps_list[4].num_tile_columns_minus1 = (uint8_t)config.EncodeCapsH265()->maxTiles.width / 2;
    h265_pps_list[4].num_tile_rows_minus1 = (uint8_t)config.EncodeCapsH265()->maxTiles.height;

    // Let PPS #6 have out-of-bounds num_tile_columns_minus1 and num_tile_rows_minus1
    h265_pps_list[5].num_tile_columns_minus1 = (uint8_t)config.EncodeCapsH265()->maxTiles.width;
    h265_pps_list[5].num_tile_rows_minus1 = (uint8_t)config.EncodeCapsH265()->maxTiles.height;

    h265_ai.stdVPSCount = 1;
    h265_ai.pStdVPSs = &h265_vps;
    h265_ai.stdSPSCount = 1;
    h265_ai.pStdSPSs = &h265_sps;
    h265_ai.stdPPSCount = (uint32_t)h265_pps_list.size();
    h265_ai.pStdPPSs = h265_pps_list.data();

    // Try first all of them together
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-08319");
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-08320");
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-08319");
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-08320");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    // Then individual invalid ones one-by-one
    h265_ai.stdPPSCount = 1;

    h265_ai.pStdPPSs = &h265_pps_list[1];
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-08319");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    h265_ai.pStdPPSs = &h265_pps_list[4];
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-08320");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    h265_ai.pStdPPSs = &h265_pps_list[5];
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-08319");
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-08320");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    // Successfully create object with an entry with the max limits
    h265_ai.pStdPPSs = &h265_pps_list[2];
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);

    // But then try to update with invalid ones
    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.pNext = &h265_ai;
    update_info.updateSequenceCount = 1;

    h265_ai.stdVPSCount = 0;
    h265_ai.stdSPSCount = 0;
    h265_ai.stdPPSCount = 1;

    h265_ai.pStdPPSs = &h265_pps_list[1];
    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-08321");
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    h265_ai.pStdPPSs = &h265_pps_list[4];
    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-08322");
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    h265_ai.pStdPPSs = &h265_pps_list[5];
    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-08321");
    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-08322");
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideoEncodeH265, SessionParamsAddInfoUniqueness) {
    TEST_DESCRIPTION("VkVideoEncodeH265SessionParametersAddInfoKHR - parameter set uniqueness");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigEncodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    auto h265_ci = vku::InitStruct<VkVideoEncodeH265SessionParametersCreateInfoKHR>();
    auto h265_ai = vku::InitStruct<VkVideoEncodeH265SessionParametersAddInfoKHR>();

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

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-None-06438");
    vps_list[0].vps_video_parameter_set_id = 2;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    vps_list[0].vps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-None-06439");
    sps_list[0].sps_video_parameter_set_id = 2;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    sps_list[0].sps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-None-06440");
    pps_list[0].pps_seq_parameter_set_id = 2;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    pps_list[0].pps_seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    h265_ci.pParametersAddInfo = nullptr;
    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-None-06438");
    vps_list[0].vps_video_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    vps_list[0].vps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-None-06439");
    sps_list[0].sps_video_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    sps_list[0].sps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-None-06440");
    pps_list[0].pps_seq_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    pps_list[0].pps_seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideoEncodeH265, UpdateSessionParamsConflictingKeys) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - H.265 encode conflicting parameter set keys");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigEncodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    auto h265_ci = vku::InitStruct<VkVideoEncodeH265SessionParametersCreateInfoKHR>();
    auto h265_ai = vku::InitStruct<VkVideoEncodeH265SessionParametersAddInfoKHR>();

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

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07228");
    vps_list2[0].vps_video_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    vps_list2[0].vps_video_parameter_set_id = 3;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07229");
    sps_list2[0].sps_seq_parameter_set_id = 3;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    sps_list2[0].sps_seq_parameter_set_id = 2;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07230");
    pps_list2[1].pps_pic_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    pps_list2[1].pps_pic_parameter_set_id = 4;
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideoEncodeH265, UpdateSessionParamsExceededCapacity) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - H.265 encode parameter set capacity exceeded");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigEncodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    auto h265_ci = vku::InitStruct<VkVideoEncodeH265SessionParametersCreateInfoKHR>();
    auto h265_ai = vku::InitStruct<VkVideoEncodeH265SessionParametersAddInfoKHR>();

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

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-06443");
    h265_ai.stdVPSCount = 1;
    h265_ai.stdSPSCount = 1;
    h265_ai.stdPPSCount = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-06444");
    h265_ai.stdVPSCount = 0;
    h265_ai.stdSPSCount = 2;
    h265_ai.stdPPSCount = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-06445");
    h265_ai.stdVPSCount = 0;
    h265_ai.stdSPSCount = 1;
    h265_ai.stdPPSCount = 3;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideoEncodeH265, GetEncodedSessionParamsMissingCodecInfo) {
    TEST_DESCRIPTION("vkGetEncodedVideoSessionParametersKHR - missing codec-specific information");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigEncodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    VideoContext context(m_device, config);

    auto get_info = vku::InitStruct<VkVideoEncodeSessionParametersGetInfoKHR>();
    get_info.videoSessionParameters = context.SessionParams();
    size_t data_size = 0;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08265");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeH265, GetEncodedSessionParams) {
    TEST_DESCRIPTION("vkGetEncodedVideoSessionParametersKHR - H.265 specific parameters are invalid");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    VideoContext context(m_device, config);

    auto h265_info = vku::InitStruct<VkVideoEncodeH265SessionParametersGetInfoKHR>();
    auto get_info = vku::InitStruct<VkVideoEncodeSessionParametersGetInfoKHR>(&h265_info);
    get_info.videoSessionParameters = context.SessionParams();
    size_t data_size = 0;

    // Need to request writing at least one parameter set
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265SessionParametersGetInfoKHR-writeStdVPS-08290");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    // Trying to request non-existent VPS
    h265_info = vku::InitStructHelper();
    h265_info.writeStdVPS = VK_TRUE;
    h265_info.stdVPSId = 1;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08266");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    // Trying to request non-existent SPS
    h265_info = vku::InitStructHelper();
    h265_info.writeStdSPS = VK_TRUE;
    h265_info.stdSPSId = 1;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08267");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    h265_info.stdVPSId = 1;
    h265_info.stdSPSId = 0;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08267");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    // Trying to request non-existent PPS
    h265_info = vku::InitStructHelper();
    h265_info.writeStdPPS = VK_TRUE;
    h265_info.stdPPSId = 1;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08268");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    h265_info.stdSPSId = 1;
    h265_info.stdPPSId = 0;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08268");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    h265_info.stdVPSId = 1;
    h265_info.stdSPSId = 0;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08268");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    // Trying to request non-existent VPS, SPS, and PPS
    h265_info = vku::InitStructHelper();
    h265_info.writeStdVPS = VK_TRUE;
    h265_info.writeStdSPS = VK_TRUE;
    h265_info.writeStdPPS = VK_TRUE;
    h265_info.stdVPSId = 1;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08266");
    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08267");
    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08268");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    // Trying to request only non-existent SPS and PPS
    h265_info.stdVPSId = 0;
    h265_info.stdSPSId = 1;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08267");
    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08268");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeH265, BeginCodingRequiresSessionParams) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - H.265 encode requires session parameters");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoBeginCodingInfoKHR begin_info = context.Begin();
    begin_info.videoSessionParameters = VK_NULL_HANDLE;

    cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-videoSession-07250");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();
    cb.End();
}

TEST_F(NegativeVideoEncodeH265, BeginCodingMissingGopRemainingFrames) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - Encode H.265 useGopRemainingFrames missing when required");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeH265()), [](const VideoConfig& config) {
        return config.EncodeCapsH265()->requiresGopRemainingFrames == VK_TRUE;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires an H.265 encode profile with rate control and requiresGopRemainingFrames == VK_TRUE";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto gop_remaining_frames_info = vku::InitStruct<VkVideoEncodeH265GopRemainingFrameInfoKHR>();

    auto rc_layer = vku::InitStruct<VkVideoEncodeRateControlLayerInfoKHR>();
    rc_layer.averageBitrate = 32000;
    rc_layer.maxBitrate = 32000;
    rc_layer.frameRateNumerator = 15;
    rc_layer.frameRateDenominator = 1;

    auto rc_info_h265 = vku::InitStruct<VkVideoEncodeH265RateControlInfoKHR>();
    rc_info_h265.flags = VK_VIDEO_ENCODE_H264_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_info_h265.gopFrameCount = 15;

    auto rc_info = vku::InitStruct<VkVideoEncodeRateControlInfoKHR>(&rc_info_h265);
    rc_info.rateControlMode = config.GetAnySupportedRateControlMode();
    rc_info.layerCount = 1;
    rc_info.pLayers = &rc_layer;
    rc_info.virtualBufferSizeInMs = 1000;
    rc_info.initialVirtualBufferSizeInMs = 0;

    VkVideoBeginCodingInfoKHR begin_info = context.Begin();
    begin_info.pNext = &rc_info;

    cb.Begin();

    // Test with VkVideoEncodeH265GopRemainingFrameInfoKHR missing from pNext chain
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-pBeginInfo-08256");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();

    // Test with useGopRemainingFrames set to VK_FALSE
    gop_remaining_frames_info.useGopRemainingFrames = VK_FALSE;
    gop_remaining_frames_info.pNext = begin_info.pNext;
    begin_info.pNext = &gop_remaining_frames_info;
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-pBeginInfo-08256");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideoEncodeH265, RateControlConstantQpNonZero) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - constantQp must be zero for H.265 if rate control mode is not DISABLED");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeH265()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with rate control";
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
    encode_info.CodecInfo().encode_h265.slice_segment_info.constantQp = 1;

    if (config.EncodeCapsH265()->maxQp < 1) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-constantQp-08273");
    }
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-constantQp-08272");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeH265, RateControlConstantQpNotInCapRange) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - constantQp must be within H.265 minQp/maxQp caps");

    RETURN_IF_SKIP(Init());

    VideoConfig config =
        GetConfig(GetConfigsWithRateControl(GetConfigsEncodeH265(), VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with DISABLED rate control";
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

    encode_info.CodecInfo().encode_h265.slice_segment_info.constantQp = config.EncodeCapsH265()->minQp - 1;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-constantQp-08273");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    encode_info.CodecInfo().encode_h265.slice_segment_info.constantQp = config.EncodeCapsH265()->maxQp + 1;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-constantQp-08273");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeH265, RateControlConstantQpPerSliceSegmentMismatch) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - constantQp must match across H.265 slice segments "
        "if VK_VIDEO_ENCODE_H265_CAPABILITY_PER_SLICE_SEGMENT_CONSTANT_QP_BIT_KHR is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(
        GetConfigsWithRateControl(GetConfigsEncodeH265(), VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR),
        [](const VideoConfig& config) {
            return config.EncodeCapsH265()->maxSliceSegmentCount > 1 &&
                   (config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_PER_SLICE_SEGMENT_CONSTANT_QP_BIT_KHR) == 0;
        }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with DISABLED rate control, "
                        "support for multiple slice segments but no support for per-slice-segment constant QP";
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
    std::vector<VkVideoEncodeH265NaluSliceSegmentInfoKHR> slice_segments(2, encode_info.CodecInfo().encode_h265.slice_segment_info);
    encode_info.CodecInfo().encode_h265.picture_info.naluSliceSegmentEntryCount = 2;
    encode_info.CodecInfo().encode_h265.picture_info.pNaluSliceSegmentEntries = slice_segments.data();

    slice_segments[0].constantQp = config.EncodeCapsH265()->minQp;
    slice_segments[1].constantQp = config.EncodeCapsH265()->maxQp;

    if (slice_segments[0].constantQp == slice_segments[1].constantQp) {
        slice_segments[1].constantQp += 1;
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-constantQp-08273");
    }

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08324");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-naluSliceSegmentEntryCount-08307");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-naluSliceSegmentEntryCount-08313");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-constantQp-08274");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeH265, RateControlLayerCountMismatch) {
    TEST_DESCRIPTION(
        "vkCmdBegin/ControlVideoCodingKHR - when using more than one rate control layer "
        "the layer count must match the H.265 sub-layer count");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithMultiLayerRateControl(GetConfigsEncodeH265()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with multi-layer rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config, true).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));

    cb.Begin();

    rc_info.CodecInfo().encode_h265.subLayerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-07025");
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    if (config.EncodeCapsH265()->maxSubLayerCount > 2) {
        rc_info.CodecInfo().encode_h265.subLayerCount = 3;
        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-07025");
        cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
        m_errorMonitor->VerifyFound();
    }

    cb.BeginVideoCoding(context.Begin());

    rc_info.CodecInfo().encode_h265.subLayerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-07025");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    if (config.EncodeCapsH265()->maxSubLayerCount > 2) {
        rc_info.CodecInfo().encode_h265.subLayerCount = 3;
        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-07025");
        cb.ControlVideoCoding(context.Control().RateControl(rc_info));
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeH265, RateControlHrdCompliance) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - H.265 HRD compliant rate control depends on capability");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeH265()), [](const VideoConfig& config) {
        return (config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_HRD_COMPLIANCE_BIT_KHR) == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode without HRD compliance support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    auto rc_info = VideoEncodeRateControlInfo(config, true).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    rc_info.CodecInfo().encode_h265.flags = VK_VIDEO_ENCODE_H265_RATE_CONTROL_ATTEMPT_HRD_COMPLIANCE_BIT_KHR;

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265RateControlInfoKHR-flags-08291");
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    m_errorMonitor->VerifyFound();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265RateControlInfoKHR-flags-08291");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeH265, RateControlInvalidCodecInfo) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - invalid H.265 rate control info");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeH265()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    auto rc_info = VideoEncodeRateControlInfo(config, true).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    const char* vuid = nullptr;

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    // Reference pattern without regular GOP flag
    vuid = "VUID-VkVideoEncodeH265RateControlInfoKHR-flags-08292";

    rc_info.CodecInfo().encode_h265.gopFrameCount = 8;

    rc_info.CodecInfo().encode_h265.flags = VK_VIDEO_ENCODE_H265_RATE_CONTROL_REFERENCE_PATTERN_FLAT_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_h265.flags |= VK_VIDEO_ENCODE_H265_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h265.flags = VK_VIDEO_ENCODE_H265_RATE_CONTROL_REFERENCE_PATTERN_DYADIC_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_h265.flags |= VK_VIDEO_ENCODE_H265_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info);

    // Conflicting reference pattern flags
    vuid = "VUID-VkVideoEncodeH265RateControlInfoKHR-flags-08293";

    rc_info.CodecInfo().encode_h265.flags = VK_VIDEO_ENCODE_H265_RATE_CONTROL_REGULAR_GOP_BIT_KHR |
                                            VK_VIDEO_ENCODE_H265_RATE_CONTROL_REFERENCE_PATTERN_FLAT_BIT_KHR |
                                            VK_VIDEO_ENCODE_H265_RATE_CONTROL_REFERENCE_PATTERN_DYADIC_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    // Regular GOP flag requires non-zero GOP size
    vuid = "VUID-VkVideoEncodeH265RateControlInfoKHR-flags-08294";

    rc_info.CodecInfo().encode_h265.flags = VK_VIDEO_ENCODE_H265_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h265.gopFrameCount = 0;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    // Invalid IDR period
    vuid = "VUID-VkVideoEncodeH265RateControlInfoKHR-idrPeriod-08295";

    rc_info.CodecInfo().encode_h265.gopFrameCount = 8;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h265.idrPeriod = 4;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_h265.idrPeriod = 8;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h265.gopFrameCount = 9;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_h265.gopFrameCount = 1;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h265.idrPeriod = 0;
    rc_test_utils.TestRateControlInfo(rc_info);

    // Invalid consecutive B frame count
    vuid = "VUID-VkVideoEncodeH265RateControlInfoKHR-consecutiveBFrameCount-08296";

    rc_info.CodecInfo().encode_h265.gopFrameCount = 4;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h265.consecutiveBFrameCount = 4;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_h265.consecutiveBFrameCount = 3;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h265.gopFrameCount = 1;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_h265.consecutiveBFrameCount = 0;
    rc_test_utils.TestRateControlInfo(rc_info);
}

TEST_F(NegativeVideoEncodeH265, RateControlQpRange) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - H.265 rate control layer QP out of bounds");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeH265()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    const char* expected_min_qp_vuid = "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMinQp-08297";
    const char* allowed_min_qp_vuid = "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMinQp-08299";

    // minQp.qpI not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.minQp.qpI -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qp_vuid, {allowed_min_qp_vuid});
    }

    // minQp.qpP not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.minQp.qpP -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qp_vuid, {allowed_min_qp_vuid});
    }

    // minQp.qpB not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.minQp.qpB -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qp_vuid, {allowed_min_qp_vuid});
    }

    // out of bounds minQp should be ignored if useMinQp is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.useMinQp = VK_FALSE;
        rc_layer.CodecInfo().encode_h265.minQp.qpI -= 1;
        rc_layer.CodecInfo().encode_h265.minQp.qpP -= 1;
        rc_layer.CodecInfo().encode_h265.minQp.qpB -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }

    const char* expected_max_qp_vuid = "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMaxQp-08298";
    const char* allowed_max_qp_vuid = "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMaxQp-08300";

    // maxQp.qpI not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.maxQp.qpI += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qp_vuid, {allowed_max_qp_vuid});
    }

    // maxQp.qpP not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.maxQp.qpP += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qp_vuid, {allowed_max_qp_vuid});
    }

    // maxQp.qpB not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.maxQp.qpB += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qp_vuid, {allowed_max_qp_vuid});
    }

    // out of bounds maxQp should be ignored if useMaxQp is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.useMaxQp = VK_FALSE;
        rc_layer.CodecInfo().encode_h265.maxQp.qpI += 1;
        rc_layer.CodecInfo().encode_h265.maxQp.qpP += 1;
        rc_layer.CodecInfo().encode_h265.maxQp.qpB += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }
}

TEST_F(NegativeVideoEncodeH265, RateControlPerPicTypeQp) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - H.265 per picture type min/max QP depends on capability");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeH265()), [](const VideoConfig& config) {
        return (config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_PER_PICTURE_TYPE_MIN_MAX_QP_BIT_KHR) == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode without per picture type min/max QP support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    const char* expected_min_qp_vuid = "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMinQp-08299";
    const char* allowed_min_qp_vuid = "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMinQp-08297";

    // minQp.qpI does not match the other QP values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.minQp.qpI += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qp_vuid, {allowed_min_qp_vuid});
    }

    // minQp.qpP does not match the other QP values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.minQp.qpP += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qp_vuid, {allowed_min_qp_vuid});
    }

    // minQp.qpB does not match the other QP values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.minQp.qpB += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qp_vuid, {allowed_min_qp_vuid});
    }

    // non-matching QP values in minQp should be ignored if useMinQp is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.useMinQp = VK_FALSE;
        rc_layer.CodecInfo().encode_h265.minQp.qpP += 1;
        rc_layer.CodecInfo().encode_h265.minQp.qpB += 2;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }

    const char* expected_max_qp_vuid = "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMaxQp-08300";
    const char* allowed_max_qp_vuid = "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMaxQp-08298";

    // maxQp.qpI does not match the other QP values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.maxQp.qpI -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qp_vuid, {allowed_max_qp_vuid});
    }

    // maxQp.qpP does not match the other QP values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.maxQp.qpP -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qp_vuid, {allowed_max_qp_vuid});
    }

    // maxQp.qpB does not match the other QP values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.maxQp.qpB -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qp_vuid, {allowed_max_qp_vuid});
    }

    // non-matching QP values in maxQp should be ignored if useMaxQp is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.useMaxQp = VK_FALSE;
        rc_layer.CodecInfo().encode_h265.maxQp.qpP -= 1;
        rc_layer.CodecInfo().encode_h265.maxQp.qpB -= 2;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }
}

TEST_F(NegativeVideoEncodeH265, RateControlMinQpGreaterThanMaxQp) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - H.265 rate control layer minQp > maxQp");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeH265()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    const char* expected_vuid = "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMinQp-08375";
    std::vector<const char*> allowed_vuids = {
        "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMaxQp-08298",
        "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMinQp-08299",
        "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMaxQp-08300",
    };

    // minQp.qpI > maxQp.qpI
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.minQp.qpI = rc_layer.CodecInfo().encode_h265.maxQp.qpI;
        rc_layer.CodecInfo().encode_h265.maxQp.qpI -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_vuid, allowed_vuids);
    }

    // minQp.qpP > maxQp.qpP
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.minQp.qpP = rc_layer.CodecInfo().encode_h265.maxQp.qpP;
        rc_layer.CodecInfo().encode_h265.maxQp.qpP -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_vuid, allowed_vuids);
    }

    // minQp.qpB > maxQp.qpB
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.minQp.qpB = rc_layer.CodecInfo().encode_h265.maxQp.qpB;
        rc_layer.CodecInfo().encode_h265.maxQp.qpB -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_vuid, allowed_vuids);
    }

    // minQp can be equal to maxQp
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.minQp.qpI = rc_layer.CodecInfo().encode_h265.maxQp.qpI;
        rc_layer.CodecInfo().encode_h265.minQp.qpP = rc_layer.CodecInfo().encode_h265.maxQp.qpP;
        rc_layer.CodecInfo().encode_h265.minQp.qpB = rc_layer.CodecInfo().encode_h265.maxQp.qpB;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }

    // minQp can be larger than maxQp if useMinQp or useMaxQp is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        std::swap(rc_layer.CodecInfo().encode_h265.minQp, rc_layer.CodecInfo().encode_h265.maxQp);

        rc_layer.CodecInfo().encode_h265.useMinQp = VK_FALSE;
        rc_layer.CodecInfo().encode_h265.useMaxQp = VK_TRUE;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);

        rc_layer.CodecInfo().encode_h265.useMinQp = VK_TRUE;
        rc_layer.CodecInfo().encode_h265.useMaxQp = VK_FALSE;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }
}

TEST_F(NegativeVideoEncodeH265, RateControlStateMismatch) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - H.265 rate control state specified does not match current configuration");

    RETURN_IF_SKIP(Init());

    // Try to find a config that supports both CBR and VBR
    VkVideoEncodeRateControlModeFlagsKHR rc_modes =
        VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR | VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR;
    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeH265(), [rc_modes](const VideoConfig& config) {
        return (config.EncodeCaps()->rateControlModes & rc_modes) == rc_modes;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires an H.265 encode profile that supports both CBR and VBR rate control modes";
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

    rc_info.CodecInfo().encode_h265.flags = VK_VIDEO_ENCODE_H265_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_info.CodecInfo().encode_h265.gopFrameCount = 15;
    rc_info.CodecInfo().encode_h265.idrPeriod = 60;
    rc_info.CodecInfo().encode_h265.consecutiveBFrameCount = 2;
    rc_info.CodecInfo().encode_h265.subLayerCount = config.EncodeCaps()->maxRateControlLayers;

    for (uint32_t i = 0; i < config.EncodeCaps()->maxRateControlLayers; ++i) {
        auto rc_layer = VideoEncodeRateControlLayerInfo(config, include_codec_info);

        rc_layer->averageBitrate = 32000 / (i + 1);
        rc_layer->maxBitrate = 48000 / (i + 1);
        rc_layer->frameRateNumerator = 30;
        rc_layer->frameRateDenominator = i + 1;

        rc_layer.CodecInfo().encode_h265.useMinQp = VK_TRUE;
        rc_layer.CodecInfo().encode_h265.minQp.qpI = config.ClampH265Qp(0 + i);
        rc_layer.CodecInfo().encode_h265.minQp.qpP = config.ClampH265Qp(5 + i);
        rc_layer.CodecInfo().encode_h265.minQp.qpB = config.ClampH265Qp(10 + i);

        if ((config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_PER_PICTURE_TYPE_MIN_MAX_QP_BIT_KHR) == 0) {
            rc_layer.CodecInfo().encode_h265.minQp.qpP = rc_layer.CodecInfo().encode_h265.minQp.qpI;
            rc_layer.CodecInfo().encode_h265.minQp.qpB = rc_layer.CodecInfo().encode_h265.minQp.qpI;
        }

        rc_layer.CodecInfo().encode_h265.useMaxFrameSize = VK_TRUE;
        rc_layer.CodecInfo().encode_h265.maxFrameSize.frameISize = 30000 / (i + 1);
        rc_layer.CodecInfo().encode_h265.maxFrameSize.framePSize = 20000 / (i + 1);
        rc_layer.CodecInfo().encode_h265.maxFrameSize.frameBSize = 10000 / (i + 1);

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
    VkVideoEncodeH265RateControlFlagsKHR flags = VK_VIDEO_ENCODE_H265_RATE_CONTROL_TEMPORAL_SUB_LAYER_PATTERN_DYADIC_BIT_KHR;
    std::swap(rc_info.CodecInfo().encode_h265.flags, flags);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.CodecInfo().encode_h265.flags, flags);

    // gopFrameCount mismatch
    uint32_t gop_frame_count = 12;
    std::swap(rc_info.CodecInfo().encode_h265.gopFrameCount, gop_frame_count);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.CodecInfo().encode_h265.gopFrameCount, gop_frame_count);

    // idrPeriod mismatch
    uint32_t idr_period = 30;
    std::swap(rc_info.CodecInfo().encode_h265.idrPeriod, idr_period);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.CodecInfo().encode_h265.idrPeriod, idr_period);

    // consecutiveBFrameCount mismatch
    uint32_t cons_b_frames = 4;
    std::swap(rc_info.CodecInfo().encode_h265.consecutiveBFrameCount, cons_b_frames);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.CodecInfo().encode_h265.consecutiveBFrameCount, cons_b_frames);

    // Layer useMinQp mismatch
    rc_info.Layer(0).CodecInfo().encode_h265.useMinQp = VK_FALSE;
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_h265.useMinQp = VK_TRUE;

    // Layer minQp.qpB mismatch
    rc_info.Layer(0).CodecInfo().encode_h265.minQp.qpB -= 1;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMinQp-08297");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMinQp-08299");
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_h265.minQp.qpB += 1;

    // Layer useMaxQp mismatch
    rc_info.Layer(0).CodecInfo().encode_h265.useMaxQp = VK_TRUE;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMaxQp-08298");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMaxQp-08300");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMinQp-08375");
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_h265.useMaxQp = VK_FALSE;

    // Layer maxQp.qpP mismatch
    rc_info.Layer(0).CodecInfo().encode_h265.maxQp.qpP += 1;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMaxQp-08298");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMaxQp-08300");
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_h265.maxQp.qpP -= 1;

    // Layer useMaxFrameSize mismatch
    rc_info.Layer(0).CodecInfo().encode_h265.useMaxFrameSize = VK_FALSE;
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_h265.useMaxFrameSize = VK_TRUE;

    // Layer maxFrameSize.frameISize mismatch
    uint32_t max_frame_i_size = 12345;
    std::swap(rc_info.Layer(0).CodecInfo().encode_h265.maxFrameSize.frameISize, max_frame_i_size);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.Layer(0).CodecInfo().encode_h265.maxFrameSize.frameISize, max_frame_i_size);
}

TEST_F(NegativeVideoEncodeH265, EncodeCapsDifferentSliceSegmentTypes) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - using different H.265 slice segment types is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeH265(), [](const VideoConfig& config) {
        return (config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_DIFFERENT_SLICE_SEGMENT_TYPE_BIT_KHR) == 0 &&
               config.EncodeCapsH265()->maxSliceSegmentCount > 1;
    }));
    if (!config) {
        GTEST_SKIP()
            << "Test requires H.265 encode support with multiple slice segment support but no different slice segment types";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();

    const uint32_t slice_segment_count = 2;
    std::vector<VkVideoEncodeH265NaluSliceSegmentInfoKHR> slice_segments(slice_segment_count,
                                                                         encode_info.CodecInfo().encode_h265.slice_segment_info);
    std::vector<StdVideoEncodeH265SliceSegmentHeader> slice_segment_headers(
        slice_segment_count, encode_info.CodecInfo().encode_h265.std_slice_segment_header);
    encode_info.CodecInfo().encode_h265.picture_info.naluSliceSegmentEntryCount = slice_segment_count;
    encode_info.CodecInfo().encode_h265.picture_info.pNaluSliceSegmentEntries = slice_segments.data();

    slice_segments[0].pStdSliceSegmentHeader = &slice_segment_headers[0];
    slice_segments[1].pStdSliceSegmentHeader = &slice_segment_headers[1];

    slice_segment_headers[0].slice_type = STD_VIDEO_H265_SLICE_TYPE_I;
    slice_segment_headers[1].slice_type = STD_VIDEO_H265_SLICE_TYPE_P;

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08324");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-naluSliceSegmentEntryCount-08307");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-naluSliceSegmentEntryCount-08313");
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08317");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeH265, EncodeCapsMultipleTilesPerSliceSegment) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - encoding multiple H.265 tiles per slice segment is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeH265(), [](const VideoConfig& config) {
        return (config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_MULTIPLE_TILES_PER_SLICE_SEGMENT_BIT_KHR) == 0 &&
               (config.EncodeCapsH265()->maxTiles.width > 1 || config.EncodeCapsH265()->maxTiles.height > 1);
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with multiple tile but no multiple tiles per slice segment support";
    }

    auto pps = config.EncodeH265PPS();
    pps->num_tile_columns_minus1 = (uint8_t)config.EncodeCapsH265()->maxTiles.width - 1;
    pps->num_tile_rows_minus1 = (uint8_t)config.EncodeCapsH265()->maxTiles.height - 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08323");
    cb.EncodeVideo(context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeH265, EncodeCapsMultipleSliceSegmentsPerTile) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - encoding multiple H.265 slcie segments per tile is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeH265(), [](const VideoConfig& config) {
        return (config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_MULTIPLE_SLICE_SEGMENTS_PER_TILE_BIT_KHR) == 0 &&
               config.EncodeCapsH265()->maxSliceSegmentCount > 1;
    }));
    if (!config) {
        GTEST_SKIP()
            << "Test requires H.265 encode support with multiple slice segment but no multiple slice segments per tile support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();

    const uint32_t slice_segment_count = 2;
    std::vector<VkVideoEncodeH265NaluSliceSegmentInfoKHR> slice_segments(slice_segment_count,
                                                                         encode_info.CodecInfo().encode_h265.slice_segment_info);
    encode_info.CodecInfo().encode_h265.picture_info.naluSliceSegmentEntryCount = slice_segment_count;
    encode_info.CodecInfo().encode_h265.picture_info.pNaluSliceSegmentEntries = slice_segments.data();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-naluSliceSegmentEntryCount-08307");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-naluSliceSegmentEntryCount-08313");
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08324");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeH265, EncodeCapsMaxSliceSegmentCount) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - test H.265 maxSliceSegmentCount capability");

    RETURN_IF_SKIP(Init());

    // Find the H.265 config with the highest slice segment count supported
    // so that we don't just check a config with only a single slice segment supported if possible
    auto configs = GetConfigsEncodeH265();
    VideoConfig config;
    uint32_t max_slice_segment_count = 0;
    for (auto& cfg : configs) {
        if (cfg.EncodeCapsH265()->maxSliceSegmentCount > max_slice_segment_count) {
            config = cfg;
            max_slice_segment_count = cfg.EncodeCapsH265()->maxSliceSegmentCount;
        }
    }
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();

    const uint32_t slice_segment_count = max_slice_segment_count + 1;
    std::vector<VkVideoEncodeH265NaluSliceSegmentInfoKHR> slice_segments(slice_segment_count,
                                                                         encode_info.CodecInfo().encode_h265.slice_segment_info);
    encode_info.CodecInfo().encode_h265.picture_info.naluSliceSegmentEntryCount = slice_segment_count;
    encode_info.CodecInfo().encode_h265.picture_info.pNaluSliceSegmentEntries = slice_segments.data();

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08324");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-naluSliceSegmentEntryCount-08307");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-naluSliceSegmentEntryCount-08313");
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265PictureInfoKHR-naluSliceSegmentEntryCount-08306");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeH265, EncodeCapsMoreSliceSegmentsThanCTBs) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - more H.265 slice segments are requested than CTBs "
        "when VK_VIDEO_ENCODE_H265_CAPABILITY_ROW_UNALIGNED_SLICE_SEGMENT_BIT_KHR is supported");

    RETURN_IF_SKIP(Init());

    // Find the H.265 config with the highest slice segment count supported
    // so that we don't just check a config with only a single slice segment supported if possible
    auto configs = GetConfigsEncodeH265();
    VideoConfig config;
    uint32_t max_slice_segment_count = 0;
    for (auto& cfg : configs) {
        if ((cfg.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_ROW_UNALIGNED_SLICE_SEGMENT_BIT_KHR) != 0 &&
            cfg.EncodeCapsH265()->maxSliceSegmentCount > max_slice_segment_count) {
            config = cfg;
            max_slice_segment_count = cfg.EncodeCapsH265()->maxSliceSegmentCount;
        }
    }
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode with row unaligned slice segment supported";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();

    const uint32_t slice_segment_count = config.MaxEncodeH265CTBCount() + 1;
    std::vector<VkVideoEncodeH265NaluSliceSegmentInfoKHR> slice_segments(slice_segment_count,
                                                                         encode_info.CodecInfo().encode_h265.slice_segment_info);
    encode_info.CodecInfo().encode_h265.picture_info.naluSliceSegmentEntryCount = slice_segment_count;
    encode_info.CodecInfo().encode_h265.picture_info.pNaluSliceSegmentEntries = slice_segments.data();

    if (slice_segment_count > config.EncodeCapsH265()->maxSliceSegmentCount) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265PictureInfoKHR-naluSliceSegmentEntryCount-08306");
    }
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08324");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-naluSliceSegmentEntryCount-08307");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeH265, EncodeCapsMoreSliceSegmentsThanCTBRows) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - more H.265 slice segments are requested than CTB rows "
        "when VK_VIDEO_ENCODE_H265_CAPABILITY_ROW_UNALIGNED_SLICE_SEGMENT_BIT_KHR is not supported");

    RETURN_IF_SKIP(Init());

    // Find the H.265 config with the highest slice segment count supported
    // so that we don't just check a config with only a single slice segment supported if possible
    auto configs = GetConfigsEncodeH265();
    VideoConfig config;
    uint32_t max_slice_segment_count = 0;
    for (auto& cfg : configs) {
        if ((cfg.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_ROW_UNALIGNED_SLICE_SEGMENT_BIT_KHR) == 0 &&
            cfg.EncodeCapsH265()->maxSliceSegmentCount > max_slice_segment_count) {
            config = cfg;
            max_slice_segment_count = cfg.EncodeCapsH265()->maxSliceSegmentCount;
        }
    }
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode without row unaligned slice segment supported";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();

    const uint32_t slice_segment_count = config.MaxEncodeH265CTBRowCount() + 1;
    std::vector<VkVideoEncodeH265NaluSliceSegmentInfoKHR> slice_segments(slice_segment_count,
                                                                         encode_info.CodecInfo().encode_h265.slice_segment_info);
    encode_info.CodecInfo().encode_h265.picture_info.naluSliceSegmentEntryCount = slice_segment_count;
    encode_info.CodecInfo().encode_h265.picture_info.pNaluSliceSegmentEntries = slice_segments.data();

    if (slice_segment_count > config.EncodeCapsH265()->maxSliceSegmentCount) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265PictureInfoKHR-naluSliceSegmentEntryCount-08306");
    }
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08324");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-naluSliceSegmentEntryCount-08313");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeH265, EncodeCapsWeightTable) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - H.265 weight table is required for explicit sample prediction if "
        "VK_VIDEO_ENCODE_H265_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config_p = GetConfig(FilterConfigs(GetConfigsEncodeH265(), [](const VideoConfig& config) {
        return (config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR) == 0 &&
               config.EncodeCapsH265()->maxPPictureL0ReferenceCount > 0;
    }));
    VideoConfig config_b = GetConfig(FilterConfigs(GetConfigsEncodeH265(), [](const VideoConfig& config) {
        return (config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR) == 0 &&
               config.EncodeCapsH265()->maxBPictureL0ReferenceCount > 0 && config.EncodeCapsH265()->maxL1ReferenceCount > 0;
    }));
    if (!config_p && !config_b) {
        GTEST_SKIP() << "Test requires an H.265 encode profile without generated weight table support";
    }

    if (config_p) {
        VideoConfig config = config_p;

        // Enable explicit weighted sample prediction for P pictures
        config.EncodeH265PPS()->flags.weighted_pred_flag = 1;

        config.SessionCreateInfo()->maxDpbSlots = 2;
        config.SessionCreateInfo()->maxActiveReferencePictures = 1;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

        VideoEncodeInfo encode_info = context.EncodeFrame(1).AddReferenceFrame(0);

        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08316");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (config_b) {
        VideoConfig config = config_b;

        // Enable explicit weighted sample prediction for B pictures
        config.EncodeH265PPS()->flags.weighted_bipred_flag = 1;

        config.SessionCreateInfo()->maxDpbSlots = 3;
        config.SessionCreateInfo()->maxActiveReferencePictures = 2;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(-1, 2));

        VideoEncodeInfo encode_info = context.EncodeFrame(2).AddReferenceFrame(0).AddBackReferenceFrame(1);
        encode_info.CodecInfo().encode_h265.std_picture_info.pic_type = STD_VIDEO_H265_PICTURE_TYPE_B;
        encode_info.CodecInfo().encode_h265.std_slice_segment_header.slice_type = STD_VIDEO_H265_SLICE_TYPE_B;

        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08316");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }
}

TEST_F(NegativeVideoEncodeH265, EncodeCapsPicType) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - Cannot encode H.265 P or B pictures without capability prerequisites");

    RETURN_IF_SKIP(Init());

    VideoConfig config_no_p = GetConfig(FilterConfigs(GetConfigsEncodeH265(), [](const VideoConfig& config) {
        return config.EncodeCapsH265()->maxPPictureL0ReferenceCount == 0;
    }));
    VideoConfig config_no_b = GetConfig(FilterConfigs(GetConfigsEncodeH265(), [](const VideoConfig& config) {
        return config.EncodeCapsH265()->maxBPictureL0ReferenceCount == 0 && config.EncodeCapsH265()->maxL1ReferenceCount == 0;
    }));
    if (!config_no_p && !config_no_b) {
        GTEST_SKIP() << "Test requires an H.265 encode profile without P or B frame support";
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
        encode_info.CodecInfo().encode_h265.std_picture_info.pic_type = STD_VIDEO_H265_PICTURE_TYPE_P;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxPPictureL0ReferenceCount-08345");
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
        encode_info.CodecInfo().encode_h265.std_picture_info.pic_type = STD_VIDEO_H265_PICTURE_TYPE_B;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxBPictureL0ReferenceCount-08346");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }
}

TEST_F(NegativeVideoEncodeH265, EncodeCapsRefPicType) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - Cannot reference H.265 P or B pictures without capability prerequisites");

    RETURN_IF_SKIP(Init());

    VideoConfig config_no_p = GetConfig(FilterConfigs(GetConfigsEncodeH265(), [](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > 1 && config.Caps()->maxActiveReferencePictures > 0 &&
               config.EncodeCapsH265()->maxPPictureL0ReferenceCount == 0;
    }));
    VideoConfig config_no_b = GetConfig(FilterConfigs(GetConfigsEncodeH265(), [](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > 1 && config.Caps()->maxActiveReferencePictures > 0 &&
               config.EncodeCapsH265()->maxBPictureL0ReferenceCount == 0 && config.EncodeCapsH265()->maxL1ReferenceCount == 0;
    }));
    if (!config_no_p && !config_no_b) {
        GTEST_SKIP() << "Test requires an H.265 encode profile without P or B frame support";
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
        encode_info.CodecInfo().encode_h265.std_picture_info.pic_type = STD_VIDEO_H265_PICTURE_TYPE_P;
        encode_info.CodecInfo().encode_h265.std_reference_info[0].pic_type = STD_VIDEO_H265_PICTURE_TYPE_P;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxPPictureL0ReferenceCount-08345");
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxPPictureL0ReferenceCount-08345");
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
        encode_info.CodecInfo().encode_h265.std_picture_info.pic_type = STD_VIDEO_H265_PICTURE_TYPE_B;
        encode_info.CodecInfo().encode_h265.std_reference_info[0].pic_type = STD_VIDEO_H265_PICTURE_TYPE_B;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-flags-08347");
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxBPictureL0ReferenceCount-08346");
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxBPictureL0ReferenceCount-08346");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }
}

TEST_F(NegativeVideoEncodeH265, EncodeCapsBPicInRefList) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - Cannot reference H.265 B pictures in L0/L1 without capability prerequisites");

    RETURN_IF_SKIP(Init());

    VideoConfig config_no_b_in_l0 = GetConfig(FilterConfigs(GetConfigsEncodeH265(), [](const VideoConfig& config) {
        return (config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_B_FRAME_IN_L0_LIST_BIT_KHR) == 0 &&
               config.EncodeCapsH265()->maxBPictureL0ReferenceCount > 0;
    }));
    VideoConfig config_no_b_in_l1 = GetConfig(FilterConfigs(GetConfigsEncodeH265(), [](const VideoConfig& config) {
        return (config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_B_FRAME_IN_L1_LIST_BIT_KHR) == 0 &&
               config.EncodeCapsH265()->maxL1ReferenceCount > 0;
    }));
    if (!config_no_b_in_l0 && !config_no_b_in_l1) {
        GTEST_SKIP() << "Test requires an H.265 encode profile without B frame support in L0 or L1";
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
        encode_info.CodecInfo().encode_h265.std_reference_info[0].pic_type = STD_VIDEO_H265_PICTURE_TYPE_B;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-flags-08347");
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
        encode_info.CodecInfo().encode_h265.std_reference_info[0].pic_type = STD_VIDEO_H265_PICTURE_TYPE_B;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-flags-08348");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }
}

TEST_F(NegativeVideoEncodeH265, EncodeInvalidCodecInfo) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - invalid/missing H.265 codec-specific information");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncodeH265()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VideoEncodeInfo encode_info = context.EncodeFrame(0);

    StdVideoEncodeH265PictureInfo std_picture_info{};
    StdVideoEncodeH265ReferenceListsInfo std_ref_lists{};
    StdVideoEncodeH265SliceSegmentHeader std_slice_segment_header{};
    auto slice_segment_info = vku::InitStruct<VkVideoEncodeH265NaluSliceSegmentInfoKHR>();
    auto picture_info = vku::InitStruct<VkVideoEncodeH265PictureInfoKHR>();
    std_picture_info.pRefLists = &std_ref_lists;
    picture_info.pStdPictureInfo = &std_picture_info;
    picture_info.naluSliceSegmentEntryCount = 1;
    picture_info.pNaluSliceSegmentEntries = &slice_segment_info;
    slice_segment_info.pStdSliceSegmentHeader = &std_slice_segment_header;

    for (uint32_t i = 0; i < STD_VIDEO_H265_MAX_NUM_LIST_REF; ++i) {
        std_ref_lists.RefPicList0[i] = STD_VIDEO_H265_NO_REFERENCE_PICTURE;
        std_ref_lists.RefPicList1[i] = STD_VIDEO_H265_NO_REFERENCE_PICTURE;
    }

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    // Missing H.265 picture info
    {
        encode_info = context.EncodeFrame(0);
        encode_info->pNext = nullptr;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08230");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // No matching VPS/SPS/PPS
    {
        encode_info = context.EncodeFrame(0);
        encode_info->pNext = &picture_info;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-StdVideoH265VideoParameterSet-08231");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-StdVideoH265SequenceParameterSet-08232");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-StdVideoH265PictureParameterSet-08233");
        std_picture_info.sps_video_parameter_set_id = 1;
        cb.EncodeVideo(encode_info);
        std_picture_info.sps_video_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-StdVideoH265SequenceParameterSet-08232");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-StdVideoH265PictureParameterSet-08233");
        std_picture_info.pps_seq_parameter_set_id = 1;
        cb.EncodeVideo(encode_info);
        std_picture_info.pps_seq_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-StdVideoH265PictureParameterSet-08233");
        std_picture_info.pps_pic_parameter_set_id = 1;
        cb.EncodeVideo(encode_info);
        std_picture_info.pps_pic_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();
    }

    // Missing H.265 setup reference info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        encode_info = context.EncodeFrame(0);
        encode_info->pNext = &picture_info;
        encode_info->pSetupReferenceSlot = &slot;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08234");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Missing H.265 reference info
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

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08235");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        std_ref_lists.RefPicList0[0] = STD_VIDEO_H265_NO_REFERENCE_PICTURE;
    }

    // Missing H.265 reference list info
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

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pNext-08235");
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08354");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        std_picture_info.pRefLists = &std_ref_lists;
    }

    // Missing H.265 L0 or L1 list reference to reference slot
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        encode_info = context.EncodeFrame(1);
        encode_info->pNext = &picture_info;
        encode_info->referenceSlotCount = 1;
        encode_info->pReferenceSlots = &slot;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pNext-08235");
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08355");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Missing reference slot for DPB index referred to by the H.265 L0 or L1 list
    {
        encode_info = context.EncodeFrame(0);
        encode_info->pNext = &picture_info;

        std_ref_lists.RefPicList0[0] = 0;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08344");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        std_ref_lists.RefPicList0[0] = STD_VIDEO_H264_NO_REFERENCE_PICTURE;

        std_ref_lists.RefPicList1[0] = 0;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08344");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        std_ref_lists.RefPicList1[0] = STD_VIDEO_H264_NO_REFERENCE_PICTURE;
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}
