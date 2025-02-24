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

class NegativeVideoEncodeAV1 : public VkVideoLayerTest {};

TEST_F(NegativeVideoEncodeAV1, ProfileMissingCodecInfo) {
    TEST_DESCRIPTION("Test missing codec-specific structure in profile definition");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    VkVideoProfileInfoKHR profile = *config.Profile();
    profile.pNext = nullptr;

    // Missing codec-specific info for AV1 encode profile
    m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-10262");
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(Gpu(), &profile, config.Caps());
    m_errorMonitor->VerifyFound();

    // For encode this VU is also relevant for quality level property queries
    auto quality_level_info = vku::InitStruct<VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR>();
    quality_level_info.pVideoProfile = &profile;

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR-pVideoProfile-08259");
    m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-10262");
    vk::GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(Gpu(), &quality_level_info, config.EncodeQualityLevelProps());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeAV1, CapabilityQueryMissingChain) {
    TEST_DESCRIPTION("vkGetPhysicalDeviceVideoCapabilitiesKHR - missing return structures in chain");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    auto caps = vku::InitStruct<VkVideoCapabilitiesKHR>();
    auto encode_caps = vku::InitStruct<VkVideoEncodeCapabilitiesKHR>();
    auto encode_av1_caps = vku::InitStruct<VkVideoEncodeAV1CapabilitiesKHR>();

    // Missing encode caps struct for encode profile
    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07186");
    caps.pNext = &encode_av1_caps;
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(Gpu(), config.Profile(), &caps);
    m_errorMonitor->VerifyFound();

    // Missing AV1 encode caps struct for AV1 encode profile
    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-10263");
    caps.pNext = &encode_caps;
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(Gpu(), config.Profile(), &caps);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeAV1, QualityLevelPropsMissingChain) {
    TEST_DESCRIPTION("vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR - missing return structures in chain");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    auto quality_level_info = vku::InitStruct<VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR>();
    auto quality_level_props = vku::InitStruct<VkVideoEncodeQualityLevelPropertiesKHR>();

    quality_level_info.pVideoProfile = config.Profile();

    // Missing codec-specific output structure for AV1 encode profile
    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR-pQualityLevelInfo-10305");
    vk::GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(Gpu(), &quality_level_info, &quality_level_props);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeAV1, CreateSessionVideoEncodeAV1NotEnabled) {
    TEST_DESCRIPTION("vkCreateVideoSession - AV1 encode is specified but videoEncodeAV1 was not enabled");

    ForceDisableFeature(vkt::Feature::videoEncodeAV1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pVideoProfile-10269");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeAV1, CreateSessionInvalidMaxLevel) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid AV1 maxLevel for encode session");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    auto av1_create_info = vku::InitStruct<VkVideoEncodeAV1SessionCreateInfoKHR>();
    av1_create_info.pNext = create_info.pNext;
    create_info.pNext = &av1_create_info;

    auto unsupported_level = static_cast<int32_t>(config.EncodeCapsAV1()->maxLevel) + 1;
    av1_create_info.maxLevel = static_cast<StdVideoAV1Level>(unsupported_level);

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pVideoProfile-10270");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeAV1, CreateSessionParamsMissingCodecInfo) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - missing codec-specific chained structure");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    VideoContext context(m_device, config);

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    auto other_codec_info = vku::InitStruct<VkVideoEncodeH264SessionParametersCreateInfoKHR>();
    other_codec_info.maxStdSPSCount = 1;
    other_codec_info.maxStdPPSCount = 1;
    create_info.pNext = &other_codec_info;
    create_info.videoSession = context.Session();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-10279");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeAV1, CreateSessionParamsTemplateNotAllowed) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - AV1 encode does not allow using parameters templates");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    VideoContext context(DeviceObj(), config);

    StdVideoAV1SequenceHeader seq_header{};
    auto av1_ci = vku::InitStruct<VkVideoEncodeAV1SessionParametersCreateInfoKHR>();
    av1_ci.pStdSequenceHeader = &seq_header;

    VkVideoSessionParametersKHR params, params2;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &av1_ci;
    create_info.videoSession = context.Session();

    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);

    create_info.videoSessionParametersTemplate = params;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-10278");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideoEncodeAV1, CreateSessionParamsFilmGrain) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - encoding with film grain is not support");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    VideoContext context(m_device, config);

    auto av1_seq_header = config.CreateAV1SequenceHeader();
    av1_seq_header.flags.film_grain_params_present = 1;

    auto av1_ci = vku::InitStruct<VkVideoEncodeAV1SessionParametersCreateInfoKHR>();
    av1_ci.pStdSequenceHeader = &av1_seq_header;

    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &av1_ci;
    create_info.videoSession = context.Session();

    VkVideoSessionParametersKHR params;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeAV1SessionParametersCreateInfoKHR-pStdSequenceHeader-10288");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeAV1, CreateSessionParamsTooManyOperatingPoints) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - too many operating points");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    VideoContext context(m_device, config);

    auto av1_seq_header = config.CreateAV1SequenceHeader();
    av1_seq_header.flags.reduced_still_picture_header = 0;
    std::vector<StdVideoEncodeAV1OperatingPointInfo> av1_op_points(config.EncodeCapsAV1()->maxOperatingPoints + 1);

    auto av1_ci = vku::InitStruct<VkVideoEncodeAV1SessionParametersCreateInfoKHR>();
    av1_ci.pStdSequenceHeader = &av1_seq_header;
    av1_ci.stdOperatingPointCount = static_cast<uint32_t>(av1_op_points.size());
    av1_ci.pStdOperatingPoints = av1_op_points.data();

    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &av1_ci;
    create_info.videoSession = context.Session();

    VkVideoSessionParametersKHR params;
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-10280");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeAV1, UpdateSessionParams) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - not supported for AV1 encode");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    VideoContext context(DeviceObj(), config);

    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.updateSequenceCount = 1;

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-10281");
    vk::UpdateVideoSessionParametersKHR(device(), context.SessionParams(), &update_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeAV1, BeginCodingRequiresSessionParams) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - AV1 encode requires session parameters");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoBeginCodingInfoKHR begin_info = context.Begin();
    begin_info.videoSessionParameters = VK_NULL_HANDLE;

    cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-videoSession-10283");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, BeginCodingMissingGopRemainingFrames) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - Encode AV1 useGopRemainingFrames missing when required");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeAV1()), [](const VideoConfig& config) {
        return config.EncodeCapsAV1()->requiresGopRemainingFrames == VK_TRUE;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires an AV1 encode profile with rate control and requiresGopRemainingFrames == VK_TRUE";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto gop_remaining_frames_info = vku::InitStruct<VkVideoEncodeAV1GopRemainingFrameInfoKHR>();

    auto rc_layer = vku::InitStruct<VkVideoEncodeRateControlLayerInfoKHR>();
    rc_layer.averageBitrate = 32000;
    rc_layer.maxBitrate = 32000;
    rc_layer.frameRateNumerator = 15;
    rc_layer.frameRateDenominator = 1;

    auto rc_info_av1 = vku::InitStruct<VkVideoEncodeAV1RateControlInfoKHR>();
    rc_info_av1.flags = VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_info_av1.gopFrameCount = 15;

    auto rc_info = vku::InitStruct<VkVideoEncodeRateControlInfoKHR>(&rc_info_av1);
    rc_info.rateControlMode = config.GetAnySupportedRateControlMode();
    rc_info.layerCount = 1;
    rc_info.pLayers = &rc_layer;
    rc_info.virtualBufferSizeInMs = 1000;
    rc_info.initialVirtualBufferSizeInMs = 0;

    VkVideoBeginCodingInfoKHR begin_info = context.Begin();
    begin_info.pNext = &rc_info;

    cb.Begin();

    // Test with VkVideoEncodeAV1GopRemainingFrameInfoKHR missing from pNext chain
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-pBeginInfo-10282");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();

    // Test with useGopRemainingFrames set to VK_FALSE
    gop_remaining_frames_info.useGopRemainingFrames = VK_FALSE;
    gop_remaining_frames_info.pNext = begin_info.pNext;
    begin_info.pNext = &gop_remaining_frames_info;
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-pBeginInfo-10282");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, RateControlConstantQIndexNonZero) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - constantQIndex must be zero for AV1 if rate control mode is not DISABLED");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeAV1()));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with rate control";
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
    encode_info.CodecInfo().encode_av1.picture_info.constantQIndex = 1;

    if (config.EncodeCapsAV1()->maxQIndex < 1) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-constantQIndex-10321");
    }
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-constantQIndex-10320");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, RateControlConstantQIndexNotInCapRange) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - constantQIndex must be within AV1 minQIndex/maxQIndex caps");

    RETURN_IF_SKIP(Init());

    // Find an AV1 encode config with a non-zero minQIndex, if possible
    VideoConfig config = GetConfig(
        FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeAV1(), VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR),
                      [](const VideoConfig& config) { return config.EncodeCapsAV1()->minQIndex > 0; }));
    // If couldn't find any config with a non-zero minQIndex, settle with any config that supports DISABLED rate control
    if (!config) {
        config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeAV1(), VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR));
    }
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with DISABLED rate control";
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

    if (config.EncodeCapsAV1()->minQIndex > 0) {
        encode_info.CodecInfo().encode_av1.picture_info.constantQIndex = config.EncodeCapsAV1()->minQIndex - 1;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-constantQIndex-10321");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    encode_info.CodecInfo().encode_av1.picture_info.constantQIndex = config.EncodeCapsAV1()->maxQIndex + 1;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-constantQIndex-10321");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, RateControlLayerCountMismatch) {
    TEST_DESCRIPTION(
        "vkCmdBegin/ControlVideoCodingKHR - when using more than one rate control layer "
        "the layer count must match the AV1 temporal layer count");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithMultiLayerRateControl(GetConfigsEncodeAV1()));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with multi-layer rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config, true).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));

    cb.Begin();

    rc_info.CodecInfo().encode_av1.temporalLayerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-10351");
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    if (config.EncodeCapsAV1()->maxTemporalLayerCount > 2) {
        rc_info.CodecInfo().encode_av1.temporalLayerCount = 3;
        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-10351");
        cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
        m_errorMonitor->VerifyFound();
    }

    cb.BeginVideoCoding(context.Begin());

    rc_info.CodecInfo().encode_av1.temporalLayerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-10351");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    if (config.EncodeCapsAV1()->maxTemporalLayerCount > 2) {
        rc_info.CodecInfo().encode_av1.temporalLayerCount = 3;
        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-10351");
        cb.ControlVideoCoding(context.Control().RateControl(rc_info));
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, RateControlTemporalLayerCount) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - temporalLayerCount is greater than maxTemporalLayerCount");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeAV1()));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config, true).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    rc_info.CodecInfo().encode_av1.temporalLayerCount = config.EncodeCapsAV1()->maxTemporalLayerCount + 1;

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeAV1RateControlInfoKHR-temporalLayerCount-10299");
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeAV1RateControlInfoKHR-temporalLayerCount-10299");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, RateControlInvalidCodecInfo) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - invalid AV1 rate control info");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeAV1()));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    auto rc_info = VideoEncodeRateControlInfo(config, true).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    const char* vuid = nullptr;

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    // Reference pattern without regular GOP flag
    vuid = "VUID-VkVideoEncodeAV1RateControlInfoKHR-flags-10294";

    rc_info.CodecInfo().encode_av1.gopFrameCount = 8;

    rc_info.CodecInfo().encode_av1.flags = VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REFERENCE_PATTERN_FLAT_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_av1.flags |= VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_av1.flags = VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REFERENCE_PATTERN_DYADIC_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_av1.flags |= VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info);

    // Conflicting reference pattern flags
    vuid = "VUID-VkVideoEncodeAV1RateControlInfoKHR-flags-10295";

    rc_info.CodecInfo().encode_av1.flags = VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REGULAR_GOP_BIT_KHR |
                                           VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REFERENCE_PATTERN_FLAT_BIT_KHR |
                                           VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REFERENCE_PATTERN_DYADIC_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    // Regular GOP flag requires non-zero GOP size
    vuid = "VUID-VkVideoEncodeAV1RateControlInfoKHR-flags-10296";

    rc_info.CodecInfo().encode_av1.flags = VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_av1.gopFrameCount = 0;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    // Invalid key frame period
    vuid = "VUID-VkVideoEncodeAV1RateControlInfoKHR-keyFramePeriod-10297";

    rc_info.CodecInfo().encode_av1.gopFrameCount = 8;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_av1.keyFramePeriod = 4;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_av1.keyFramePeriod = 8;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_av1.gopFrameCount = 9;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_av1.gopFrameCount = 1;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_av1.keyFramePeriod = 0;
    rc_test_utils.TestRateControlInfo(rc_info);

    // Invalid consecutive bipredictive frame count
    vuid = "VUID-VkVideoEncodeAV1RateControlInfoKHR-consecutiveBipredictiveFrameCount-10298";

    rc_info.CodecInfo().encode_av1.gopFrameCount = 4;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_av1.consecutiveBipredictiveFrameCount = 4;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_av1.consecutiveBipredictiveFrameCount = 3;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_av1.gopFrameCount = 1;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_av1.consecutiveBipredictiveFrameCount = 0;
    rc_test_utils.TestRateControlInfo(rc_info);
}

TEST_F(NegativeVideoEncodeAV1, RateControlQIndexRange) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - AV1 rate control layer quantizer index out of bounds");

    RETURN_IF_SKIP(Init());

    // Find an AV1 encode config with a non-zero minQIndex, if possible
    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeAV1()),
                                                 [](const VideoConfig& config) { return config.EncodeCapsAV1()->minQIndex > 0; }));
    // If couldn't find any config with a non-zero minQIndex, settle with any config that supports DISABLED rate control
    if (!config) {
        config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeAV1()));
    }
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    const char* expected_min_qi_vuid = "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMinQIndex-10300";
    const char* allowed_min_qi_vuid = "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMinQIndex-10301";

    if (config.EncodeCapsAV1()->minQIndex > 0) {
        // minQIndex.intraQIndex not in supported range
        {
            auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
            rc_layer.CodecInfo().encode_av1.minQIndex.intraQIndex -= 1;
            rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qi_vuid, {allowed_min_qi_vuid});
        }

        // minQIndex.predictiveQIndex not in supported range
        {
            auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
            rc_layer.CodecInfo().encode_av1.minQIndex.predictiveQIndex -= 1;
            rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qi_vuid, {allowed_min_qi_vuid});
        }

        // minQIndex.bipredictiveQIndex not in supported range
        {
            auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
            rc_layer.CodecInfo().encode_av1.minQIndex.bipredictiveQIndex -= 1;
            rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qi_vuid, {allowed_min_qi_vuid});
        }
    }

    // out of bounds minQIndex should be ignored if useMinQIndex is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.useMinQIndex = VK_FALSE;
        rc_layer.CodecInfo().encode_av1.minQIndex.intraQIndex -= 1;
        rc_layer.CodecInfo().encode_av1.minQIndex.predictiveQIndex -= 1;
        rc_layer.CodecInfo().encode_av1.minQIndex.bipredictiveQIndex -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }

    const char* expected_max_qi_vuid = "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMaxQIndex-10302";
    const char* allowed_max_qi_vuid = "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMaxQIndex-10303";

    // maxQIndex.intraQIndex not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.maxQIndex.intraQIndex += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qi_vuid, {allowed_max_qi_vuid});
    }

    // maxQIndex.predictiveQIndex not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.maxQIndex.predictiveQIndex += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qi_vuid, {allowed_max_qi_vuid});
    }

    // maxQIndex.bipredictiveQIndex not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.maxQIndex.bipredictiveQIndex += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qi_vuid, {allowed_max_qi_vuid});
    }

    // out of bounds maxQIndex should be ignored if useMaxQIndex is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.useMaxQIndex = VK_FALSE;
        rc_layer.CodecInfo().encode_av1.maxQIndex.intraQIndex += 1;
        rc_layer.CodecInfo().encode_av1.maxQIndex.predictiveQIndex += 1;
        rc_layer.CodecInfo().encode_av1.maxQIndex.bipredictiveQIndex += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }
}

TEST_F(NegativeVideoEncodeAV1, RateControlPerRcGroupQIndex) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - AV1 per rate control group min/max quantizer index depends on capability");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeAV1()), [](const VideoConfig& config) {
        return (config.EncodeCapsAV1()->flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_PER_RATE_CONTROL_GROUP_MIN_MAX_Q_INDEX_BIT_KHR) == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode without per rate control group min/max quantizer index support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    const char* expected_min_qi_vuid = "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMinQIndex-10301";
    const char* allowed_min_qi_vuid = "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMinQIndex-10300";

    // minQIndex.intraQIndex does not match the other quantizer index values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.minQIndex.intraQIndex += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qi_vuid, {allowed_min_qi_vuid});
    }

    // minQIndex.predictiveQIndex does not match the other quantizer index values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.minQIndex.predictiveQIndex += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qi_vuid, {allowed_min_qi_vuid});
    }

    // minQIndex.bipredictiveQIndex does not match the other quantizer index values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.minQIndex.bipredictiveQIndex += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qi_vuid, {allowed_min_qi_vuid});
    }

    // non-matching quantizer index values in minQIndex should be ignored if useMinQIndex is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.useMinQIndex = VK_FALSE;
        rc_layer.CodecInfo().encode_av1.minQIndex.predictiveQIndex += 1;
        rc_layer.CodecInfo().encode_av1.minQIndex.bipredictiveQIndex += 2;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }

    const char* expected_max_qi_vuid = "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMaxQIndex-10303";
    const char* allowed_max_qi_vuid = "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMaxQIndex-10302";

    // maxQIndex.intraQIndex does not match the other quantizer index values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.maxQIndex.intraQIndex -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qi_vuid, {allowed_max_qi_vuid});
    }

    // maxQIndex.predictiveQIndex does not match the other quantizer index values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.maxQIndex.predictiveQIndex -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qi_vuid, {allowed_max_qi_vuid});
    }

    // maxQIndex.bipredictiveQIndex does not match the other quantizer index values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.maxQIndex.bipredictiveQIndex -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qi_vuid, {allowed_max_qi_vuid});
    }

    // non-matching quantizer index values in maxQIndex should be ignored if useMaxQIndex is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.useMaxQIndex = VK_FALSE;
        rc_layer.CodecInfo().encode_av1.maxQIndex.predictiveQIndex -= 1;
        rc_layer.CodecInfo().encode_av1.maxQIndex.bipredictiveQIndex -= 2;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }
}

TEST_F(NegativeVideoEncodeAV1, RateControlMinQIndexGreaterThanMaxQIndex) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - AV1 rate control layer minQIndex > maxQIndex");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeAV1()));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    const char* expected_vuid = "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMinQIndex-10304";
    std::vector<const char*> allowed_vuids = {
        "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMaxQIndex-10302",
        "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMinQIndex-10301",
        "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMaxQIndex-10303",
    };

    // minQIndex.intraQIndex > maxQIndex.intraQIndex
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.minQIndex.intraQIndex = rc_layer.CodecInfo().encode_av1.maxQIndex.intraQIndex;
        rc_layer.CodecInfo().encode_av1.maxQIndex.intraQIndex -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_vuid, allowed_vuids);
    }

    // minQIndex.predictiveQIndex > maxQIndex.predictiveQIndex
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.minQIndex.predictiveQIndex = rc_layer.CodecInfo().encode_av1.maxQIndex.predictiveQIndex;
        rc_layer.CodecInfo().encode_av1.maxQIndex.predictiveQIndex -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_vuid, allowed_vuids);
    }

    // minQIndex.bipredictiveQIndex > maxQIndex.bipredictiveQIndex
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.minQIndex.bipredictiveQIndex = rc_layer.CodecInfo().encode_av1.maxQIndex.bipredictiveQIndex;
        rc_layer.CodecInfo().encode_av1.maxQIndex.bipredictiveQIndex -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_vuid, allowed_vuids);
    }

    // minQIndex can be equal to maxQIndex
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.minQIndex.intraQIndex = rc_layer.CodecInfo().encode_av1.maxQIndex.intraQIndex;
        rc_layer.CodecInfo().encode_av1.minQIndex.predictiveQIndex = rc_layer.CodecInfo().encode_av1.maxQIndex.predictiveQIndex;
        rc_layer.CodecInfo().encode_av1.minQIndex.bipredictiveQIndex = rc_layer.CodecInfo().encode_av1.maxQIndex.bipredictiveQIndex;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }

    // minQIndex can be larger than maxQIndex if useMinQIndex or useMaxQIndex is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        std::swap(rc_layer.CodecInfo().encode_av1.minQIndex, rc_layer.CodecInfo().encode_av1.maxQIndex);

        rc_layer.CodecInfo().encode_av1.useMinQIndex = VK_FALSE;
        rc_layer.CodecInfo().encode_av1.useMaxQIndex = VK_TRUE;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);

        rc_layer.CodecInfo().encode_av1.useMinQIndex = VK_TRUE;
        rc_layer.CodecInfo().encode_av1.useMaxQIndex = VK_FALSE;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }
}

TEST_F(NegativeVideoEncodeAV1, RateControlStateMismatch) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - AV1 rate control state specified does not match current configuration");

    RETURN_IF_SKIP(Init());

    // Try to find a config that supports both CBR and VBR
    VkVideoEncodeRateControlModeFlagsKHR rc_modes =
        VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR | VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR;
    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [rc_modes](const VideoConfig& config) {
        return (config.EncodeCaps()->rateControlModes & rc_modes) == rc_modes;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires an AV1 encode profile that supports both CBR and VBR rate control modes";
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

    rc_info.CodecInfo().encode_av1.flags = VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_info.CodecInfo().encode_av1.gopFrameCount = 15;
    rc_info.CodecInfo().encode_av1.keyFramePeriod = 60;
    rc_info.CodecInfo().encode_av1.consecutiveBipredictiveFrameCount = 2;
    rc_info.CodecInfo().encode_av1.temporalLayerCount = config.EncodeCaps()->maxRateControlLayers;

    for (uint32_t i = 0; i < config.EncodeCaps()->maxRateControlLayers; ++i) {
        auto rc_layer = VideoEncodeRateControlLayerInfo(config, include_codec_info);

        rc_layer->averageBitrate = 32000 / (i + 1);
        rc_layer->maxBitrate = 48000 / (i + 1);
        rc_layer->frameRateNumerator = 30;
        rc_layer->frameRateDenominator = i + 1;

        rc_layer.CodecInfo().encode_av1.useMinQIndex = VK_TRUE;
        rc_layer.CodecInfo().encode_av1.minQIndex.intraQIndex = config.ClampAV1QIndex(0 + i);
        rc_layer.CodecInfo().encode_av1.minQIndex.predictiveQIndex = config.ClampAV1QIndex(5 + i);
        rc_layer.CodecInfo().encode_av1.minQIndex.bipredictiveQIndex = config.ClampAV1QIndex(10 + i);

        if ((config.EncodeCapsAV1()->flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_PER_RATE_CONTROL_GROUP_MIN_MAX_Q_INDEX_BIT_KHR) == 0) {
            rc_layer.CodecInfo().encode_av1.minQIndex.predictiveQIndex = rc_layer.CodecInfo().encode_av1.minQIndex.intraQIndex;
            rc_layer.CodecInfo().encode_av1.minQIndex.bipredictiveQIndex = rc_layer.CodecInfo().encode_av1.minQIndex.intraQIndex;
        }

        rc_layer.CodecInfo().encode_av1.useMaxFrameSize = VK_TRUE;
        rc_layer.CodecInfo().encode_av1.maxFrameSize.intraFrameSize = 30000 / (i + 1);
        rc_layer.CodecInfo().encode_av1.maxFrameSize.predictiveFrameSize = 20000 / (i + 1);
        rc_layer.CodecInfo().encode_av1.maxFrameSize.bipredictiveFrameSize = 10000 / (i + 1);

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
    VkVideoEncodeAV1RateControlFlagsKHR flags = VK_VIDEO_ENCODE_AV1_RATE_CONTROL_TEMPORAL_LAYER_PATTERN_DYADIC_BIT_KHR;
    std::swap(rc_info.CodecInfo().encode_av1.flags, flags);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.CodecInfo().encode_av1.flags, flags);

    // gopFrameCount mismatch
    uint32_t gop_frame_count = 12;
    std::swap(rc_info.CodecInfo().encode_av1.gopFrameCount, gop_frame_count);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.CodecInfo().encode_av1.gopFrameCount, gop_frame_count);

    // keyFramePeriod mismatch
    uint32_t key_frame_period = 30;
    std::swap(rc_info.CodecInfo().encode_av1.keyFramePeriod, key_frame_period);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.CodecInfo().encode_av1.keyFramePeriod, key_frame_period);

    // consecutiveBipredictiveFrameCount mismatch
    uint32_t cons_b_frames = 4;
    std::swap(rc_info.CodecInfo().encode_av1.consecutiveBipredictiveFrameCount, cons_b_frames);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.CodecInfo().encode_av1.consecutiveBipredictiveFrameCount, cons_b_frames);

    // Layer useMinQIndex mismatch
    rc_info.Layer(0).CodecInfo().encode_av1.useMinQIndex = VK_FALSE;
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_av1.useMinQIndex = VK_TRUE;

    // Layer minQIndex.bipredictiveQIndex mismatch
    rc_info.Layer(0).CodecInfo().encode_av1.minQIndex.bipredictiveQIndex -= 1;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMinQIndex-10300");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMinQIndex-10301");
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_av1.minQIndex.bipredictiveQIndex += 1;

    // Layer useMaxQIndex mismatch
    rc_info.Layer(0).CodecInfo().encode_av1.useMaxQIndex = VK_TRUE;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMaxQIndex-10302");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMaxQIndex-10303");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMinQIndex-10304");
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_av1.useMaxQIndex = VK_FALSE;

    // Layer maxQIndex.predictiveQIndex mismatch
    rc_info.Layer(0).CodecInfo().encode_av1.maxQIndex.predictiveQIndex += 1;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMinQIndex-10301");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMaxQIndex-10303");
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_av1.maxQIndex.predictiveQIndex -= 1;

    // Layer useMaxFrameSize mismatch
    rc_info.Layer(0).CodecInfo().encode_av1.useMaxFrameSize = VK_FALSE;
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_av1.useMaxFrameSize = VK_TRUE;

    // Layer maxFrameSize.intraFrameSize mismatch
    uint32_t max_frame_i_size = 12345;
    std::swap(rc_info.Layer(0).CodecInfo().encode_av1.maxFrameSize.intraFrameSize, max_frame_i_size);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.Layer(0).CodecInfo().encode_av1.maxFrameSize.intraFrameSize, max_frame_i_size);
}

TEST_F(NegativeVideoEncodeAV1, EncodeCapsPrimaryRefCdfOnly) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - using AV1 primary reference for CDF-only is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(
        GetConfigsWithReferences(FilterConfigs(GetConfigsEncodeAV1(),
                                               [](const VideoConfig& config) {
                                                   return (config.EncodeCapsAV1()->flags &
                                                           VK_VIDEO_ENCODE_AV1_CAPABILITY_PRIMARY_REFERENCE_CDF_ONLY_BIT_KHR) == 0;
                                               })),
        2));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support without CDF-only primary reference support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1));

    VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
    encode_info.CodecInfo().encode_av1.picture_info.primaryReferenceCdfOnly = VK_TRUE;

    // This may trigger other, prediction mode specific cap VUs, because we do not respect the need
    // to use one or two of the supported reference names corresponding to the used prediction mode
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-predictionMode-10329");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-predictionMode-10331");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-predictionMode-10333");

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeAV1PictureInfoKHR-flags-10289");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, EncodeCapsGenObuExtHeader) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - generating AV1 OBU extension header is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [](const VideoConfig& config) {
        return (config.EncodeCapsAV1()->flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_GENERATE_OBU_EXTENSION_HEADER_BIT_KHR) == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support without OBU extension header generation support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    StdVideoEncodeAV1ExtensionHeader av1_obu_ext_header{};
    VideoEncodeInfo encode_info = context.EncodeFrame();
    encode_info.CodecInfo().encode_av1.picture_info.generateObuExtensionHeader = VK_TRUE;
    encode_info.CodecInfo().encode_av1.std_picture_info.pExtensionHeader = &av1_obu_ext_header;

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeAV1PictureInfoKHR-flags-10292");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, EncodeCapsFrameSizeOverride) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 frame size override is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [](const VideoConfig& config) {
        return ((config.Caps()->minCodedExtent.width < config.Caps()->maxCodedExtent.width) ||
                (config.Caps()->minCodedExtent.height < config.Caps()->maxCodedExtent.height)) &&
               (config.EncodeCapsAV1()->flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_FRAME_SIZE_OVERRIDE_BIT_KHR) == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with no frame size override support";
    }

    config.UpdateMaxCodedExtent(config.Caps()->maxCodedExtent);

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    // Cannot set frame_size_override_flag
    auto encode_info = context.EncodeFrame();
    encode_info.CodecInfo().encode_av1.std_picture_info.flags.frame_size_override_flag = 1;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-flags-10322");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    encode_info.CodecInfo().encode_av1.std_picture_info.flags.frame_size_override_flag = 0;

    // Cannot use smaller coded width
    encode_info->srcPictureResource.codedExtent.width -= 1;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-flags-10323");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    encode_info->srcPictureResource.codedExtent.width += 1;

    // Cannot use smaller coded height
    encode_info->srcPictureResource.codedExtent.height -= 1;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-flags-10324");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    encode_info->srcPictureResource.codedExtent.height += 1;

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, EncodeCapsMotionVectorScaling) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 motion vector scaling is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(
        GetConfigsWithReferences(FilterConfigs(
            GetConfigsEncodeAV1(),
            [](const VideoConfig& config) {
                return ((config.Caps()->minCodedExtent.width < config.Caps()->maxCodedExtent.width) ||
                        (config.Caps()->minCodedExtent.height < config.Caps()->maxCodedExtent.height)) &&
                       (config.EncodeCapsAV1()->flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_MOTION_VECTOR_SCALING_BIT_KHR) == 0;
            })),
        2));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with references but no motion vector scaling support";
    }

    config.UpdateMaxCodedExtent(config.Caps()->maxCodedExtent);

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    // We will use a setup where the encoded picture has an extent of maxCodedExtent
    // but the reference frame has an extent of minCodedExtent
    auto patched_resource = context.Dpb()->Picture(1);
    patched_resource.codedExtent = config.Caps()->minCodedExtent;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, patched_resource));

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-flags-10325");
    cb.EncodeVideo(context.EncodeFrame(0).AddReferenceFrame(1, &patched_resource));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, EncodeCapsMaxTiles) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - TileCols and TileRows must be between (0,0) and maxTiles");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    StdVideoAV1TileInfo tile_info{};
    VideoEncodeInfo encode_info = context.EncodeFrame();
    encode_info.CodecInfo().encode_av1.std_picture_info.pTileInfo = &tile_info;

    // TileCols > 0 is not satisfied
    {
        tile_info.TileCols = 0;
        tile_info.TileRows = 1;

        // We may violate tile size limits here but that's okay
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pTileInfo-10347");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pTileInfo-10348");

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pTileInfo-10343");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // TileRows > 0 is not satisfied
    {
        tile_info.TileCols = 1;
        tile_info.TileRows = 0;

        // We may violate tile size limits here but that's okay
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pTileInfo-10347");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pTileInfo-10348");

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pTileInfo-10344");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // TileCols <= maxTiles.width is not satisfied
    {
        tile_info.TileCols = static_cast<uint8_t>(config.EncodeCapsAV1()->maxTiles.width + 1);
        tile_info.TileRows = 1;

        // We may violate tile size limits here but that's okay
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pTileInfo-10347");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pTileInfo-10348");

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pTileInfo-10345");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // TileRows <= maxTiles.height is not satisfied
    {
        tile_info.TileCols = 1;
        tile_info.TileRows = static_cast<uint8_t>(config.EncodeCapsAV1()->maxTiles.height + 1);

        // We may violate tile size limits here but that's okay
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pTileInfo-10347");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pTileInfo-10348");

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pTileInfo-10346");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, EncodeCapsMinMaxTileSize) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - ceil(codedExtent / (TileCols,TileRows)) must be between minTileSize and maxTileSize");

    RETURN_IF_SKIP(Init());

    auto configs = GetConfigsEncodeAV1();
    if (configs.size() == 0) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    StdVideoAV1TileInfo tile_info{};

    auto test_encode = [&](const VideoConfig& config, const char* expected_tile_size_vuid) {
        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin());

        VideoEncodeInfo encode_info = context.EncodeFrame();
        encode_info.CodecInfo().encode_av1.std_picture_info.pTileInfo = &tile_info;

        m_errorMonitor->SetDesiredError(expected_tile_size_vuid);
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    };

    // Test a case that triggers ceil(codedExtent.width / TileCols) < minTileSize.width
    {
        // Find the config with the largest supported minimum tile width
        uint32_t max_min_tile_width = 0;
        VideoConfig config;
        for (auto& cfg : configs) {
            if (cfg.EncodeCapsAV1()->minTileSize.width > max_min_tile_width) {
                max_min_tile_width = cfg.EncodeCapsAV1()->minTileSize.width;
                config = cfg;
            }
        }

        // Use the smallest possible coded extent
        const VkExtent2D coded_extent = config.Caps()->minCodedExtent;
        config.UpdateMaxCodedExtent(coded_extent);

        // Use more tile columns than what would be the maximum to have a tile width >= minTileSize.width
        tile_info.TileCols = static_cast<uint8_t>(coded_extent.width / config.EncodeCapsAV1()->minTileSize.width + 1);
        tile_info.TileRows = static_cast<uint8_t>(coded_extent.height / config.EncodeCapsAV1()->minTileSize.height);

        // We may violate some tile count VUIDs here but that is fine
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pTileInfo-10345");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pTileInfo-10346");

        test_encode(config, "VUID-vkCmdEncodeVideoKHR-pTileInfo-10347");
    }

    // Test a case that triggers ceil(codedExtent.height / TileRows) < minTileSize.height
    {
        // Find the config with the largest supported minimum tile height
        uint32_t max_min_tile_height = 0;
        VideoConfig config;
        for (auto& cfg : configs) {
            if (cfg.EncodeCapsAV1()->minTileSize.height > max_min_tile_height) {
                max_min_tile_height = cfg.EncodeCapsAV1()->minTileSize.height;
                config = cfg;
            }
        }

        // Use the smallest possible coded extent
        const VkExtent2D coded_extent = config.Caps()->minCodedExtent;
        config.UpdateMaxCodedExtent(coded_extent);

        // Use more tile rows than what would be the maximum to have a tile height >= minTileSize.height
        tile_info.TileCols = static_cast<uint8_t>(coded_extent.width / config.EncodeCapsAV1()->minTileSize.width);
        tile_info.TileRows = static_cast<uint8_t>(coded_extent.height / config.EncodeCapsAV1()->minTileSize.height + 1);

        // We may violate some tile count VUIDs here but that is fine
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pTileInfo-10345");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pTileInfo-10346");

        test_encode(config, "VUID-vkCmdEncodeVideoKHR-pTileInfo-10348");
    }

    // Test a case that triggers ceil(codedExtent.width / TileCols) > maxTileSize.width
    {
        // Find the config with the smallest supported maximum tile width
        uint32_t min_max_tile_width = UINT32_MAX;
        VideoConfig config;
        for (auto& cfg : configs) {
            if (cfg.EncodeCapsAV1()->maxTileSize.width < min_max_tile_width) {
                min_max_tile_width = cfg.EncodeCapsAV1()->maxTileSize.width;
                config = cfg;
            }
        }

        // Use the largest possible coded extent
        const VkExtent2D coded_extent = config.Caps()->maxCodedExtent;
        config.UpdateMaxCodedExtent(coded_extent);

        // Use less tile columns than what would be the minimum to have a tile width <= maxTileSize.width
        tile_info.TileCols = static_cast<uint8_t>(
            (coded_extent.width + config.EncodeCapsAV1()->maxTileSize.width - 1) / config.EncodeCapsAV1()->maxTileSize.width - 1);
        tile_info.TileRows = static_cast<uint8_t>((coded_extent.height + config.EncodeCapsAV1()->maxTileSize.height - 1) /
                                                  config.EncodeCapsAV1()->maxTileSize.height);

        // We can only test this if the determined TileCols is not zero
        if (tile_info.TileCols > 0) {
            test_encode(config, "VUID-vkCmdEncodeVideoKHR-pTileInfo-10347");
        }
    }

    // Test a case that triggers ceil(codedExtent.height / TileRows) > maxTileSize.height
    {
        // Find the config with the smallest supported maximum tile height
        uint32_t min_max_tile_height = UINT32_MAX;
        VideoConfig config;
        for (auto& cfg : configs) {
            if (cfg.EncodeCapsAV1()->maxTileSize.height < min_max_tile_height) {
                min_max_tile_height = cfg.EncodeCapsAV1()->maxTileSize.height;
                config = cfg;
            }
        }

        // Use the largest possible coded extent
        const VkExtent2D coded_extent = config.Caps()->maxCodedExtent;
        config.UpdateMaxCodedExtent(coded_extent);

        // Use less tile rows than what would be the minimum to have a tile height <= maxTileSize.height
        tile_info.TileCols = static_cast<uint8_t>((coded_extent.width + config.EncodeCapsAV1()->maxTileSize.width - 1) /
                                                  config.EncodeCapsAV1()->maxTileSize.width);
        tile_info.TileRows = static_cast<uint8_t>((coded_extent.height + config.EncodeCapsAV1()->maxTileSize.height - 1) /
                                                      config.EncodeCapsAV1()->maxTileSize.height -
                                                  1);

        // We can only test this if the determined TileRows is not zero
        if (tile_info.TileRows > 0) {
            test_encode(config, "VUID-vkCmdEncodeVideoKHR-pTileInfo-10348");
        }
    }
}

TEST_F(NegativeVideoEncodeAV1, EncodeInvalidCodecInfo) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - invalid/missing AV1 codec-specific information");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncodeAV1()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    // Missing AV1 picture info
    {
        auto encode_info = context.EncodeFrame(0);
        encode_info->pNext = nullptr;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-10317");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // INTRA_ONLY prediction mode used with AV1 frame type other than KEY_FRAME or INTRA_ONLY_FRAME
    {
        auto encode_info = context.EncodeFrame(1).AddReferenceFrame(0);

        encode_info.CodecInfo().encode_av1.picture_info.predictionMode = VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_INTRA_ONLY_KHR;
        encode_info.CodecInfo().encode_av1.std_picture_info.frame_type = STD_VIDEO_AV1_FRAME_TYPE_INTER;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10326");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Non-INTRA_ONLY prediction mode used with AV1 frame type KEY_FRAME or INTRA_ONLY_FRAME
    {
        auto encode_info = context.EncodeFrame(1).AddReferenceFrame(0);

        encode_info.CodecInfo().encode_av1.std_picture_info.frame_type = STD_VIDEO_AV1_FRAME_TYPE_INTRA_ONLY;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pStdPictureInfo-10327");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        encode_info.CodecInfo().encode_av1.std_picture_info.frame_type = STD_VIDEO_AV1_FRAME_TYPE_KEY;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pStdPictureInfo-10327");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Missing AV1 setup reference info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        auto encode_info = context.EncodeFrame(0);
        encode_info->pSetupReferenceSlot = &slot;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10318");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Missing AV1 reference info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        auto encode_info = context.EncodeFrame(1).AddReferenceFrame(0);
        encode_info->referenceSlotCount = 1;
        encode_info->pReferenceSlots = &slot;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-10319");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Missing reference in referenceNameSlotIndices to reference slot
    {
        auto encode_info = context.EncodeFrame(1).AddReferenceFrame(0);

        for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
            encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
        }

        // This will also trigger the VU related to primary_ref_frame DPB slot index
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeAV1PictureInfoKHR-pStdPictureInfo-10291");

        // This may trigger other, prediction mode specific cap VUs, because we do not respect the need
        // to use one or two of the supported reference names corresponding to the used prediction mode
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-predictionMode-10329");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-predictionMode-10331");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-predictionMode-10333");

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-slotIndex-10335");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Missing reference slot for DPB slot index referred to by referenceNameSlotIndices
    {
        auto encode_info = context.EncodeFrame(1).AddReferenceFrame(0);
        encode_info->referenceSlotCount = 0;
        encode_info->pReferenceSlots = nullptr;

        for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
            m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-referenceNameSlotIndices-10334");
        }

        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // AV1 segmentation is not supported but segmentation_enabled is not zero
    {
        auto encode_info = context.EncodeFrame(0);
        encode_info.CodecInfo().encode_av1.std_picture_info.flags.segmentation_enabled = 1;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pStdPictureInfo-10349");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // AV1 segmentation is not supported but pSegmentation is not NULL
    {
        StdVideoAV1Segmentation segmentation_info{};
        auto encode_info = context.EncodeFrame(0);
        encode_info.CodecInfo().encode_av1.std_picture_info.pSegmentation = &segmentation_info;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pStdPictureInfo-10350");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, EncodeInvalidCodecInfoPrimaryRefCdfOnly) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - invalid/missing AV1 codec-specific information "
        "when primary reference is used only as CDF reference");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(
        GetConfigsWithReferences(FilterConfigs(GetConfigsEncodeAV1(),
                                               [](const VideoConfig& config) {
                                                   return (config.EncodeCapsAV1()->flags &
                                                           VK_VIDEO_ENCODE_AV1_CAPABILITY_PRIMARY_REFERENCE_CDF_ONLY_BIT_KHR) != 0;
                                               })),
        2));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with reference pictures, 2 DPB slots, "
                        "and CDF-only primary reference support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1));

    // Invalid primary_ref_frame
    {
        VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
        encode_info.CodecInfo().encode_av1.picture_info.primaryReferenceCdfOnly = VK_TRUE;
        encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame = VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR;

        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeAV1PictureInfoKHR-primaryReferenceCdfOnly-10290");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Invalid primary reference DPB slot index
    {
        VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
        encode_info.CodecInfo().encode_av1.picture_info.primaryReferenceCdfOnly = VK_TRUE;
        const uint8_t primary_ref_frame = encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame;
        encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[primary_ref_frame] = -1;

        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeAV1PictureInfoKHR-pStdPictureInfo-10291");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, EncodeInvalidPrimaryRefFrameDpbSlotIndex) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - AV1 primary_ref_frame specifies a valid reference name with no associated DPB slot index");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncodeAV1()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1));

    VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
    const uint8_t primary_ref_frame = encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame;
    encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[primary_ref_frame] = -1;

    // This may trigger other, prediction mode specific cap VUs, because we do not respect the need
    // to use one or two of the supported reference names corresponding to the used prediction mode
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-predictionMode-10329");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-predictionMode-10331");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-predictionMode-10333");

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeAV1PictureInfoKHR-pStdPictureInfo-10291");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, EncodeInvalidCodecInfoGenObuExtHeader) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - invalid/missing AV1 codec-specific information "
        "when OBU extension header generation is requested");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [](const VideoConfig& config) {
        return (config.EncodeCapsAV1()->flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_GENERATE_OBU_EXTENSION_HEADER_BIT_KHR) != 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with OBU extension header generation support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();
    encode_info.CodecInfo().encode_av1.picture_info.generateObuExtensionHeader = VK_TRUE;
    encode_info.CodecInfo().encode_av1.std_picture_info.pExtensionHeader = nullptr;

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeAV1PictureInfoKHR-generateObuExtensionHeader-10293");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, EncodeSingleReferenceNotSupported) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 single reference prediction mode not supported");

    RETURN_IF_SKIP(Init());

    // Single reference prediction requires at least one active reference picture
    const uint32_t min_ref_count = 1;

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [&](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > min_ref_count && config.Caps()->maxActiveReferencePictures >= min_ref_count &&
               config.EncodeCapsAV1()->maxSingleReferenceCount == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with no single reference prediction mode support";
    }

    config.SessionCreateInfo()->maxDpbSlots = min_ref_count + 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = min_ref_count;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(1, 1));

    VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
    encode_info.CodecInfo().encode_av1.picture_info.predictionMode = VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_SINGLE_REFERENCE_KHR;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxSingleReferenceCount-10328");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, EncodeInvalidSingleReference) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - Unsupported AV1 single reference name");

    RETURN_IF_SKIP(Init());

    // Single reference prediction requires at least one active reference picture
    const uint32_t min_ref_count = 1;

    // We need to find an unsupported AV1 reference name
    const uint32_t ref_name_mask = 0x7F;

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [&](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > min_ref_count && config.Caps()->maxActiveReferencePictures >= min_ref_count &&
               config.EncodeCapsAV1()->maxSingleReferenceCount > 0 &&
               (config.EncodeCapsAV1()->singleReferenceNameMask & ref_name_mask) != ref_name_mask;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with single reference prediction mode support";
    }

    config.SessionCreateInfo()->maxDpbSlots = min_ref_count + 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = min_ref_count;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(1, 1));

    VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
    encode_info.CodecInfo().encode_av1.picture_info.predictionMode = VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_SINGLE_REFERENCE_KHR;

    // Clear all supported reference name indices, leaving only the unsupported ones on
    for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
        if ((config.EncodeCapsAV1()->singleReferenceNameMask & (1 << i)) != 0) {
            encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
        }
    }

    // This may trigger the VU related to primary_ref_frame DPB slot index
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeAV1PictureInfoKHR-pStdPictureInfo-10291");

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10329");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, EncodeInvalidSingleReferenceCdfOnly) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - Unsupported CDF-only AV1 single reference name");

    RETURN_IF_SKIP(Init());

    // Single reference prediction requires at least one active reference picture
    // The CDF-only reference can also refer to the same reference slot, only the reference name has to differ
    const uint32_t min_ref_count = 1;

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [&](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > min_ref_count && config.Caps()->maxActiveReferencePictures >= min_ref_count &&
               config.EncodeCapsAV1()->maxSingleReferenceCount > 0 &&
               (config.EncodeCapsAV1()->flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_PRIMARY_REFERENCE_CDF_ONLY_BIT_KHR) != 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with single reference prediction mode "
                        "and CDF-only primary reference support";
    }

    config.SessionCreateInfo()->maxDpbSlots = min_ref_count + 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = min_ref_count;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(1, 1));

    VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
    encode_info.CodecInfo().encode_av1.picture_info.predictionMode = VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_SINGLE_REFERENCE_KHR;

    // Clear all supported reference name indices, leaving only the unsupported ones on
    uint8_t supported_ref_idx = 0;
    for (uint8_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
        if ((config.EncodeCapsAV1()->singleReferenceNameMask & (1 << i)) != 0) {
            encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
        } else {
            // Remember one of the supported reference names as we will use that as CDF-only reference
            supported_ref_idx = i;
        }
    }

    // Use the supported reference name as CDF-only reference
    encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[supported_ref_idx] = 1;
    encode_info.CodecInfo().encode_av1.picture_info.primaryReferenceCdfOnly = VK_TRUE;
    encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame = supported_ref_idx;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10329");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, EncodeUnidirectionalCompoundNotSupported) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 unidirectional compound prediction mode not supported");

    RETURN_IF_SKIP(Init());

    // Unidirectional compound prediction requires at least one active reference picture
    // No need for two pictures as both reference names can point to the same picture
    const uint32_t min_ref_count = 1;

    // We need to find an unsupported AV1 reference name
    // NOTE: Unidirectional compound prediction does not support ALTREF2_FRAME in any combination
    const uint32_t ref_name_mask = 0x5F;

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [&](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > min_ref_count && config.Caps()->maxActiveReferencePictures >= min_ref_count &&
               config.EncodeCapsAV1()->maxUnidirectionalCompoundReferenceCount == 0 &&
               (config.EncodeCapsAV1()->unidirectionalCompoundReferenceNameMask & ref_name_mask) != ref_name_mask;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with no unidirectional compound prediction mode support";
    }

    config.SessionCreateInfo()->maxDpbSlots = min_ref_count + 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = min_ref_count;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(1, 1));

    VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
    encode_info.CodecInfo().encode_av1.picture_info.predictionMode =
        VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_UNIDIRECTIONAL_COMPOUND_KHR;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxUnidirectionalCompoundReferenceCount-10330");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, EncodeInvalidUnidirectionalCompound) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - Unsupported AV1 unidirectional compound reference names");

    RETURN_IF_SKIP(Init());

    // Unidirectional compound prediction requires at least one active reference picture
    // No need for two pictures as both reference names can point to the same picture
    const uint32_t min_ref_count = 1;

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [&](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > min_ref_count && config.Caps()->maxActiveReferencePictures >= min_ref_count &&
               config.EncodeCapsAV1()->maxUnidirectionalCompoundReferenceCount > 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with unidirectional compound prediction mode support";
    }

    config.SessionCreateInfo()->maxDpbSlots = min_ref_count + 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = min_ref_count;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(1, 1));

    VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
    encode_info.CodecInfo().encode_av1.picture_info.predictionMode =
        VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_UNIDIRECTIONAL_COMPOUND_KHR;

    // Unidirectional compound supports the following combinations
    //   (0,1) - LAST_FRAME + LAST2_FRAME
    //   (0,2) - LAST_FRAME + LAST3_FRAME
    //   (0,3) - LAST_FRAME + GOLDEN_FRAME
    //   (4,6) - BWDREF_FRAME + ALTREF_FRAME
    // Test with cases that fail these conditions in some way
    std::vector<uint32_t> clear_masks = {
        0x11,  // No LAST_FRAME or BWDREF_FRAME
        0x41,  // No LAST_FRAME or ALTREF_FRAME
        0x1E,  // No LAST2_FRAME, LAST3_FRAME, GOLDEN_FRAME, or BWDREF_FRAME
        0x4E,  // No LAST2_FRAME, LAST3_FRAME, GOLDEN_FRAME, or ALTREF_FRAME
    };
    for (auto clear_mask : clear_masks) {
        for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
            if ((clear_mask & (1 << i)) != 0) {
                encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
            }
        }

        // This may trigger the VU related to primary_ref_frame DPB slot index
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeAV1PictureInfoKHR-pStdPictureInfo-10291");

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10331");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, EncodeInvalidUnidirectionalCompoundCdfOnly) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - Unsupported CDF-only AV1 unidirectional compound reference names");

    RETURN_IF_SKIP(Init());

    // Unidirectional compound prediction requires at least one active reference picture
    // No need for two pictures as both reference names can point to the same picture
    // The CDF-only reference can also refer to the same reference slot, only the reference name has to differ
    const uint32_t min_ref_count = 1;

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [&](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > min_ref_count && config.Caps()->maxActiveReferencePictures >= min_ref_count &&
               config.EncodeCapsAV1()->maxUnidirectionalCompoundReferenceCount > 0 &&
               (config.EncodeCapsAV1()->flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_PRIMARY_REFERENCE_CDF_ONLY_BIT_KHR) != 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with unidirectional compound prediction mode "
                        "and CDF-only primary reference support";
    }

    config.SessionCreateInfo()->maxDpbSlots = min_ref_count + 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = min_ref_count;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(1, 1));

    // Unidirectional compound supports the following combinations
    //   (0,1) - LAST_FRAME + LAST2_FRAME
    //   (0,2) - LAST_FRAME + LAST3_FRAME
    //   (0,3) - LAST_FRAME + GOLDEN_FRAME
    //   (4,6) - BWDREF_FRAME + ALTREF_FRAME

    const uint8_t last_frame_idx = STD_VIDEO_AV1_REFERENCE_NAME_LAST_FRAME - 1;
    const std::array<uint8_t, 3> last_frame_pair_indices = {STD_VIDEO_AV1_REFERENCE_NAME_LAST2_FRAME - 1,
                                                            STD_VIDEO_AV1_REFERENCE_NAME_LAST3_FRAME - 1,
                                                            STD_VIDEO_AV1_REFERENCE_NAME_GOLDEN_FRAME - 1};
    const uint8_t bwdref_frame_idx = STD_VIDEO_AV1_REFERENCE_NAME_BWDREF_FRAME - 1;
    const uint8_t altref_frame_idx = STD_VIDEO_AV1_REFERENCE_NAME_ALTREF_FRAME - 1;

    if ((config.EncodeCapsAV1()->unidirectionalCompoundReferenceNameMask & (1 << last_frame_idx)) != 0) {
        // If LAST_FRAME is supported, we clear all group 2 reference names and set it as CDF-only reference,
        VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
        encode_info.CodecInfo().encode_av1.picture_info.predictionMode =
            VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_UNIDIRECTIONAL_COMPOUND_KHR;

        for (uint8_t i = bwdref_frame_idx; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
            encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
        }
        encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[last_frame_idx] = 1;
        encode_info.CodecInfo().encode_av1.picture_info.primaryReferenceCdfOnly = VK_TRUE;
        encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame = last_frame_idx;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10331");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        // then we clear all except LAST_FRAME and enable one of its pairs (if supported) as CDF-only reference
        for (uint8_t i = last_frame_idx + 1; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
            encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
        }

        for (auto other_frame_idx : last_frame_pair_indices) {
            if ((config.EncodeCapsAV1()->unidirectionalCompoundReferenceNameMask & (1 << other_frame_idx)) != 0) {
                encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[other_frame_idx] = 1;
                encode_info.CodecInfo().encode_av1.picture_info.primaryReferenceCdfOnly = VK_TRUE;
                encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame = other_frame_idx;

                m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10331");
                cb.EncodeVideo(encode_info);
                m_errorMonitor->VerifyFound();

                encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[other_frame_idx] = -1;
            }
        }
    }

    if ((config.EncodeCapsAV1()->unidirectionalCompoundReferenceNameMask & (1 << bwdref_frame_idx)) != 0) {
        // If BWDREF_FRAME is supported, we clear all group 1 reference names and set it as CDF-only reference
        VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
        encode_info.CodecInfo().encode_av1.picture_info.predictionMode =
            VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_UNIDIRECTIONAL_COMPOUND_KHR;

        for (uint8_t i = last_frame_idx; i < bwdref_frame_idx; ++i) {
            encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
        }
        encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[bwdref_frame_idx] = 1;
        encode_info.CodecInfo().encode_av1.picture_info.primaryReferenceCdfOnly = VK_TRUE;
        encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame = bwdref_frame_idx;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10331");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    if ((config.EncodeCapsAV1()->unidirectionalCompoundReferenceNameMask & (1 << altref_frame_idx)) != 0) {
        // If ALTREF_FRAME is supported, we clear all group 1 reference names and set it as CDF-only reference
        VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
        encode_info.CodecInfo().encode_av1.picture_info.predictionMode =
            VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_UNIDIRECTIONAL_COMPOUND_KHR;

        for (uint8_t i = last_frame_idx; i < bwdref_frame_idx; ++i) {
            encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
        }
        encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[altref_frame_idx] = 1;
        encode_info.CodecInfo().encode_av1.picture_info.primaryReferenceCdfOnly = VK_TRUE;
        encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame = altref_frame_idx;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10331");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, EncodeBidirectionalCompoundNotSupported) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 bidirectional compound prediction mode not supported");

    RETURN_IF_SKIP(Init());

    // Bidirectional compound prediction requires at least one active reference picture
    // No need for two pictures as both reference names can point to the same picture
    const uint32_t min_ref_count = 1;

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [&](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > min_ref_count && config.Caps()->maxActiveReferencePictures >= min_ref_count &&
               config.EncodeCapsAV1()->maxBidirectionalCompoundReferenceCount == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with no bidirectional compound prediction mode support";
    }

    config.SessionCreateInfo()->maxDpbSlots = min_ref_count + 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = min_ref_count;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(1, 1));

    VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
    encode_info.CodecInfo().encode_av1.picture_info.predictionMode = VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_BIDIRECTIONAL_COMPOUND_KHR;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxBidirectionalCompoundReferenceCount-10332");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, EncodeInvalidBidirectionalCompound) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - Unsupported AV1 bidirectional compound reference names");

    RETURN_IF_SKIP(Init());

    // Bidirectional compound prediction requires at least one active reference picture
    // No need for two pictures as both reference names can point to the same picture
    const uint32_t min_ref_count = 1;

    // We need to find an unsupported AV1 reference name
    const uint32_t ref_name_mask = 0x7F;

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [&](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > min_ref_count && config.Caps()->maxActiveReferencePictures >= min_ref_count &&
               config.EncodeCapsAV1()->maxBidirectionalCompoundReferenceCount > 0 &&
               (config.EncodeCapsAV1()->bidirectionalCompoundReferenceNameMask & ref_name_mask) != ref_name_mask;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with bidirectional compound prediction mode support";
    }

    config.SessionCreateInfo()->maxDpbSlots = min_ref_count + 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = min_ref_count;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(1, 1));

    const uint32_t bwdref_frame_idx = STD_VIDEO_AV1_REFERENCE_NAME_BWDREF_FRAME - 1;

    // Clear all supported reference name indices from group 1
    {
        VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
        encode_info.CodecInfo().encode_av1.picture_info.predictionMode =
            VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_BIDIRECTIONAL_COMPOUND_KHR;

        for (uint32_t i = 0; i < bwdref_frame_idx; ++i) {
            if ((config.EncodeCapsAV1()->bidirectionalCompoundReferenceNameMask & (1 << i)) != 0) {
                encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
            }
        }

        // This may trigger the VU related to primary_ref_frame DPB slot index
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeAV1PictureInfoKHR-pStdPictureInfo-10291");

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10333");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Clear all supported reference name indices from group 2
    {
        VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
        encode_info.CodecInfo().encode_av1.picture_info.predictionMode =
            VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_BIDIRECTIONAL_COMPOUND_KHR;

        for (uint32_t i = bwdref_frame_idx; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
            if ((config.EncodeCapsAV1()->bidirectionalCompoundReferenceNameMask & (1 << i)) != 0) {
                encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
            }
        }

        // This may trigger the VU related to primary_ref_frame DPB slot index
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeAV1PictureInfoKHR-pStdPictureInfo-10291");

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10333");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, EncodeInvalidBidirectionalCompoundCdfOnly) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - Unsupported CDF-only AV1 bidirectional compound reference names");

    RETURN_IF_SKIP(Init());

    // Bidirectional compound prediction requires at least one active reference picture
    // No need for two pictures as both reference names can point to the same picture
    // The CDF-only reference can also refer to the same reference slot, only the reference name has to differ
    const uint32_t min_ref_count = 1;

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [&](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > min_ref_count && config.Caps()->maxActiveReferencePictures >= min_ref_count &&
               config.EncodeCapsAV1()->maxBidirectionalCompoundReferenceCount > 0 &&
               (config.EncodeCapsAV1()->flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_PRIMARY_REFERENCE_CDF_ONLY_BIT_KHR) != 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with bidirectional compound prediction mode "
                        "and CDF-only primary reference support";
    }

    config.SessionCreateInfo()->maxDpbSlots = min_ref_count + 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = min_ref_count;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(1, 1));

    const uint8_t bwdref_frame_idx = STD_VIDEO_AV1_REFERENCE_NAME_BWDREF_FRAME - 1;

    // Clear all supported reference name indices from group 1
    {
        VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
        encode_info.CodecInfo().encode_av1.picture_info.predictionMode =
            VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_BIDIRECTIONAL_COMPOUND_KHR;

        uint8_t supported_group1_ref_idx = 0;
        for (uint8_t i = 0; i < bwdref_frame_idx; ++i) {
            if ((config.EncodeCapsAV1()->bidirectionalCompoundReferenceNameMask & (1 << i)) != 0) {
                encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
            } else {
                // Remember one of the supported group 1 reference names as we will use that as CDF-only reference
                supported_group1_ref_idx = i;
            }
        }

        // Use the supported group 1 reference name as CDF-only reference
        encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[supported_group1_ref_idx] = 1;
        encode_info.CodecInfo().encode_av1.picture_info.primaryReferenceCdfOnly = VK_TRUE;
        encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame = supported_group1_ref_idx;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10333");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Clear all supported reference name indices from group 2
    {
        VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
        encode_info.CodecInfo().encode_av1.picture_info.predictionMode =
            VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_BIDIRECTIONAL_COMPOUND_KHR;

        uint8_t supported_group2_ref_idx = 0;
        for (uint8_t i = bwdref_frame_idx; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
            if ((config.EncodeCapsAV1()->bidirectionalCompoundReferenceNameMask & (1 << i)) != 0) {
                encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
            } else {
                // Remember one of the supported group 2 reference names as we will use that as CDF-only reference
                supported_group2_ref_idx = i;
            }
        }

        // Use the supported group 2 reference name as CDF-only reference
        encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[supported_group2_ref_idx] = 1;
        encode_info.CodecInfo().encode_av1.picture_info.primaryReferenceCdfOnly = VK_TRUE;
        encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame = supported_group2_ref_idx;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10333");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, EncodeObuExtHeaderPictureSetupMismatch) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - AV1 OBU extension header must be specified for both or neither of picture and setup info");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncodeAV1())));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));

    // Picture info has AV1 OBU extension header info but the setup reference info does not
    {
        StdVideoEncodeAV1ExtensionHeader av1_obu_ext_header{};
        auto encode_info = context.EncodeFrame(0);
        encode_info.CodecInfo().encode_av1.std_picture_info.pExtensionHeader = &av1_obu_ext_header;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10338");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Setup reference info has AV1 OBU extension header info but the picture info does not
    {
        StdVideoEncodeAV1ExtensionHeader av1_obu_ext_header{};
        auto encode_info = context.EncodeFrame(0);
        encode_info.CodecInfo().encode_av1.std_setup_reference_info.pExtensionHeader = &av1_obu_ext_header;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10338");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, EncodeInvalidTemporalId) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 temporal ID exceeds maxTemporalLayerCount");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncodeAV1()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    // Picture temporal_id >= maxTemporalLayerCount
    {
        StdVideoEncodeAV1ExtensionHeader av1_obu_ext_header{};
        auto encode_info = context.EncodeFrame(0);
        encode_info.CodecInfo().encode_av1.std_picture_info.pExtensionHeader = &av1_obu_ext_header;
        encode_info.CodecInfo().encode_av1.std_setup_reference_info.pExtensionHeader = &av1_obu_ext_header;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pExtensionHeader-10336");
        av1_obu_ext_header.temporal_id = static_cast<uint8_t>(config.EncodeCapsAV1()->maxTemporalLayerCount + 1);
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pExtensionHeader-10336");
        av1_obu_ext_header.temporal_id = static_cast<uint8_t>(config.EncodeCapsAV1()->maxTemporalLayerCount);
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        av1_obu_ext_header.temporal_id = static_cast<uint8_t>(config.EncodeCapsAV1()->maxTemporalLayerCount - 1);
        cb.EncodeVideo(encode_info);
    }

    // Reference temporal_id >= maxTemporalLayerCount
    {
        StdVideoEncodeAV1ExtensionHeader av1_obu_ext_header{};
        auto encode_info = context.EncodeFrame(1).AddReferenceFrame(0);
        encode_info.CodecInfo().encode_av1.std_reference_info[0].pExtensionHeader = &av1_obu_ext_header;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pExtensionHeader-10341");
        av1_obu_ext_header.temporal_id = static_cast<uint8_t>(config.EncodeCapsAV1()->maxTemporalLayerCount + 1);
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pExtensionHeader-10341");
        av1_obu_ext_header.temporal_id = static_cast<uint8_t>(config.EncodeCapsAV1()->maxTemporalLayerCount);
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        av1_obu_ext_header.temporal_id = static_cast<uint8_t>(config.EncodeCapsAV1()->maxTemporalLayerCount - 1);
        cb.EncodeVideo(encode_info);
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, EncodePictureSetupTemporalIdMismatch) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - temporal_id mismatch between picture and setup info");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(FilterConfigs(
        GetConfigsEncodeAV1(), [](const VideoConfig& config) { return config.EncodeCapsAV1()->maxTemporalLayerCount > 1; }))));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with DPB slots and multiple temporal layers";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));

    StdVideoEncodeAV1ExtensionHeader picture_av1_obu_ext_header{};
    StdVideoEncodeAV1ExtensionHeader setup_av1_obu_ext_header{};
    auto encode_info = context.EncodeFrame(0);
    encode_info.CodecInfo().encode_av1.std_picture_info.pExtensionHeader = &picture_av1_obu_ext_header;
    encode_info.CodecInfo().encode_av1.std_setup_reference_info.pExtensionHeader = &setup_av1_obu_ext_header;

    picture_av1_obu_ext_header.temporal_id = 0;
    setup_av1_obu_ext_header.temporal_id = 1;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10339");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, EncodeInvalidSpatialId) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 spatial ID exceeds maxSpatialLayerCount");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncodeAV1()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    // Picture spatial_id >= maxSpatialLayerCount
    {
        StdVideoEncodeAV1ExtensionHeader av1_obu_ext_header{};
        auto encode_info = context.EncodeFrame(0);
        encode_info.CodecInfo().encode_av1.std_picture_info.pExtensionHeader = &av1_obu_ext_header;
        encode_info.CodecInfo().encode_av1.std_setup_reference_info.pExtensionHeader = &av1_obu_ext_header;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pExtensionHeader-10337");
        av1_obu_ext_header.spatial_id = static_cast<uint8_t>(config.EncodeCapsAV1()->maxSpatialLayerCount + 1);
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pExtensionHeader-10337");
        av1_obu_ext_header.spatial_id = static_cast<uint8_t>(config.EncodeCapsAV1()->maxSpatialLayerCount);
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        av1_obu_ext_header.spatial_id = static_cast<uint8_t>(config.EncodeCapsAV1()->maxSpatialLayerCount - 1);
        cb.EncodeVideo(encode_info);
    }

    // Reference spatial_id >= maxSpatialLayerCount
    {
        StdVideoEncodeAV1ExtensionHeader av1_obu_ext_header{};
        auto encode_info = context.EncodeFrame(1).AddReferenceFrame(0);
        encode_info.CodecInfo().encode_av1.std_reference_info[0].pExtensionHeader = &av1_obu_ext_header;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pExtensionHeader-10342");
        av1_obu_ext_header.spatial_id = static_cast<uint8_t>(config.EncodeCapsAV1()->maxSpatialLayerCount + 1);
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pExtensionHeader-10342");
        av1_obu_ext_header.spatial_id = static_cast<uint8_t>(config.EncodeCapsAV1()->maxSpatialLayerCount);
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        av1_obu_ext_header.spatial_id = static_cast<uint8_t>(config.EncodeCapsAV1()->maxSpatialLayerCount - 1);
        cb.EncodeVideo(encode_info);
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, EncodePictureSetupSpatialIdMismatch) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - spatial_id mismatch between picture and setup info");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(FilterConfigs(
        GetConfigsEncodeAV1(), [](const VideoConfig& config) { return config.EncodeCapsAV1()->maxSpatialLayerCount > 1; }))));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with DPB slots and multiple spatial layers";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));

    StdVideoEncodeAV1ExtensionHeader picture_av1_obu_ext_header{};
    StdVideoEncodeAV1ExtensionHeader setup_av1_obu_ext_header{};
    auto encode_info = context.EncodeFrame(0);
    encode_info.CodecInfo().encode_av1.std_picture_info.pExtensionHeader = &picture_av1_obu_ext_header;
    encode_info.CodecInfo().encode_av1.std_setup_reference_info.pExtensionHeader = &setup_av1_obu_ext_header;

    picture_av1_obu_ext_header.spatial_id = 0;
    setup_av1_obu_ext_header.spatial_id = 1;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10340");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeAV1, CreateBufferVideoEncodeAV1NotEnabled) {
    TEST_DESCRIPTION("vkCreateBuffer - AV1 encode is specified but videoEncodeAV1 was not enabled");

    ForceDisableFeature(vkt::Feature::videoEncodeAV1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    VkBuffer buffer = VK_NULL_HANDLE;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.usage = VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR;
    buffer_ci.size = 2048;

    VkVideoProfileListInfoKHR video_profiles = vku::InitStructHelper();
    video_profiles.profileCount = 1;
    video_profiles.pProfiles = config.Profile();
    buffer_ci.pNext = &video_profiles;

    m_errorMonitor->SetDesiredError("VUID-VkBufferCreateInfo-pNext-10249");
    vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeAV1, CreateImageVideoEncodeAV1NotEnabled) {
    TEST_DESCRIPTION("vkCreateImage - AV1 encode is specified but videoEncodeAV1 was not enabled");

    ForceDisableFeature(vkt::Feature::videoEncodeAV1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    VkImage image = VK_NULL_HANDLE;
    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = config.PictureFormatProps()->imageType;
    image_ci.format = config.PictureFormatProps()->format;
    image_ci.extent = {config.MaxCodedExtent().width, config.MaxCodedExtent().height, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = config.PictureFormatProps()->imageTiling;
    image_ci.usage = config.PictureFormatProps()->imageUsageFlags;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkVideoProfileListInfoKHR video_profiles = vku::InitStructHelper();
    video_profiles.profileCount = 1;
    video_profiles.pProfiles = config.Profile();
    image_ci.pNext = &video_profiles;

    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-pNext-10250");
    vk::CreateImage(device(), &image_ci, nullptr, &image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeAV1, CreateQueryPoolVideoEncodeAV1NotEnabled) {
    TEST_DESCRIPTION("vkCreateQueryPool - AV1 encode is specified but videoEncodeAV1 was not enabled");

    ForceDisableFeature(vkt::Feature::videoEncodeAV1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    auto encode_feedback_info = vku::InitStruct<VkQueryPoolVideoEncodeFeedbackCreateInfoKHR>(config.Profile());
    encode_feedback_info.encodeFeedbackFlags = VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BYTES_WRITTEN_BIT_KHR;

    VkQueryPool query_pool = VK_NULL_HANDLE;
    auto create_info = vku::InitStruct<VkQueryPoolCreateInfo>(&encode_feedback_info);
    create_info.queryType = VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR;
    create_info.queryCount = 1;

    m_errorMonitor->SetDesiredError("VUID-VkQueryPoolCreateInfo-pNext-10248");
    vk::CreateQueryPool(device(), &create_info, nullptr, &query_pool);
    m_errorMonitor->VerifyFound();
}
