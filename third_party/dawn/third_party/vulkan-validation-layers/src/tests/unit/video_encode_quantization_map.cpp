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

class NegativeVideoEncodeQuantizationMap : public VkVideoLayerTest {};

TEST_F(NegativeVideoEncodeQuantizationMap, CreateSessionVideoQuantizationMapNotEnabled) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - quantization map flag is specified but videoEncodeQuantizationMap was not enabled");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    ForceDisableFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    for (VkVideoSessionCreateFlagBitsKHR flag : {VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                                 VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR}) {
        create_info.flags = flag;

        m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoSessionCreateInfoKHR-flags-10267");
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoSessionCreateInfoKHR-flags-10268");
        m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-flags-10264");
        vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeVideoEncodeQuantizationMap, CreateSessionRequiresEncodeProfile) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - cannot use quantization map flags without encode profile");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    for (VkVideoSessionCreateFlagBitsKHR flag : {VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                                 VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR}) {
        create_info.flags = flag;

        m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-flags-10265");
        vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeVideoEncodeQuantizationMap, CreateSessionQuantMapFlagsMutuallyExclusive) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - cannot allow both QUANTIZATION_DELTA and EMPHASIS maps at the same time");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR) &&
               (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR);
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with support for both QUANTIZATION_DELTA and EMPHASIS map";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();
    create_info.flags = VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR |
                        VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-flags-10266");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeQuantizationMap, CreateSessionQuantDeltaMapUnsupported) {
    TEST_DESCRIPTION(
        "vkCreateVideoSessionKHR - VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR "
        "requires VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR) == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires an encode profile without QUANTIZATION_DELTA_MAP support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.flags = VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR;
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-flags-10267");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeQuantizationMap, CreateSessionEmphasisMapUnsupported) {
    TEST_DESCRIPTION(
        "vkCreateVideoSessionKHR - VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR "
        "requires VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR) == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires an encode profile without EMPHASIS_MAP support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.flags = VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR;
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-flags-10268");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeQuantizationMap, CreateSessionParamsQuantMapIncompatSession) {
    TEST_DESCRIPTION(
        "vkCreateVideoSessionParametersKHR - QUANTIZATION_MAP_COMPATIBLE_BIT requires session allowing quantization maps");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    ASSERT_TRUE(config) << "Support for videoEncodeQuantizationMap implies at least one supported encode profile";

    VideoContext context(m_device, config);

    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.flags = VK_VIDEO_SESSION_PARAMETERS_CREATE_QUANTIZATION_MAP_COMPATIBLE_BIT_KHR;
    create_info.videoSession = context.Session();

    VkVideoSessionParametersKHR params;
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-flags-10271");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeQuantizationMap, CreateSessionParamsQuantMapTexelSize) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - missing or invalid quantizationMapTexelSize");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    struct TestConfig {
        VkVideoSessionCreateFlagBitsKHR flag;
        VideoConfig config;
        const std::vector<vku::safe_VkVideoFormatPropertiesKHR>& map_props;
        const char* vuid;
    };

    std::vector<TestConfig> tests;
    if (delta_config) {
        tests.push_back({VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR, delta_config,
                         delta_config.SupportedQuantDeltaMapProps(), "VUID-VkVideoSessionParametersCreateInfoKHR-flags-10273"});
    }
    if (emphasis_config) {
        tests.push_back({VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR, emphasis_config,
                         emphasis_config.SupportedQuantDeltaMapProps(), "VUID-VkVideoSessionParametersCreateInfoKHR-flags-10274"});
    }

    for (const auto& test : tests) {
        VideoConfig config = test.config;

        config.SessionCreateInfo()->flags |= test.flag;
        VideoContext context(m_device, config);

        // VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR missing from pNext chain
        {
            VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
            create_info.flags = VK_VIDEO_SESSION_PARAMETERS_CREATE_QUANTIZATION_MAP_COMPATIBLE_BIT_KHR;
            create_info.videoSession = context.Session();

            VkVideoSessionParametersKHR params;
            m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-flags-10272");
            vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
            m_errorMonitor->VerifyFound();
        }

        {
            // Find an invalid quantizaitonMapTexelSize by using the maximum width + 1
            VkExtent2D invalid_texel_size = {0, 0};
            for (const auto& map_props : test.map_props) {
                auto texel_size = config.GetQuantMapTexelSize(map_props);
                if (invalid_texel_size.width < texel_size.width) {
                    invalid_texel_size.width = texel_size.width + 1;
                    invalid_texel_size.height = texel_size.height;
                }
            }

            VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
            create_info.flags = VK_VIDEO_SESSION_PARAMETERS_CREATE_QUANTIZATION_MAP_COMPATIBLE_BIT_KHR;
            create_info.videoSession = context.Session();

            VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR map_create_info = vku::InitStructHelper();
            map_create_info.quantizationMapTexelSize = invalid_texel_size;
            map_create_info.pNext = create_info.pNext;
            create_info.pNext = &map_create_info;

            VkVideoSessionParametersKHR params;
            m_errorMonitor->SetDesiredError(test.vuid);
            vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
            m_errorMonitor->VerifyFound();
        }
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideoEncodeQuantizationMap, CreateSessionParamsIncompatibleTemplateQuantMap) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - template must match in quantization map compatibility");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    std::vector<std::tuple<VideoConfig, VkVideoSessionCreateFlagBitsKHR, const VkVideoFormatPropertiesKHR*>> tests = {};
    if (delta_config) {
        tests.emplace_back(delta_config, VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                           delta_config.QuantDeltaMapProps());
    }
    if (emphasis_config) {
        tests.emplace_back(emphasis_config, VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR,
                           emphasis_config.EmphasisMapProps());
    }

    for (auto& [config, flag, map_props] : tests) {
        const auto texel_size = config.GetQuantMapTexelSize(*map_props);

        config.SessionCreateInfo()->flags |= flag;
        VideoContext context(m_device, config);

        // Template is not QUANIZATION_MAP_COMPATIBLE but QUANIZATION_MAP_COMPATIBLE is requested
        {
            auto template_params = context.CreateSessionParams();

            VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR map_create_info = vku::InitStructHelper();
            map_create_info.quantizationMapTexelSize = texel_size;
            VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
            map_create_info.pNext = create_info.pNext;
            create_info.pNext = &map_create_info;
            create_info.flags = VK_VIDEO_SESSION_PARAMETERS_CREATE_QUANTIZATION_MAP_COMPATIBLE_BIT_KHR;
            create_info.videoSessionParametersTemplate = template_params;
            create_info.videoSession = context.Session();

            VkVideoSessionParametersKHR params2 = VK_NULL_HANDLE;
            m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-10275");
            vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
            m_errorMonitor->VerifyFound();
        }

        // Template is QUANIZATION_MAP_COMPATIBLE but QUANIZATION_MAP_COMPATIBLE is not requested
        {
            auto template_params = context.CreateSessionParamsWithQuantMapTexelSize(texel_size);

            VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
            create_info.videoSessionParametersTemplate = template_params;
            create_info.videoSession = context.Session();

            VkVideoSessionParametersKHR params2 = VK_NULL_HANDLE;
            m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-10277");
            vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
            m_errorMonitor->VerifyFound();
        }
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideoEncodeQuantizationMap, CreateSessionParamsIncompatibleTemplateQuantMapTexelSize) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - quantizationMapTexelSize mismatches template");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    auto get_alt_texel_size_format = [](const std::vector<vku::safe_VkVideoFormatPropertiesKHR>& props) -> uint32_t {
        const auto texel_size = VideoConfig::GetQuantMapTexelSize(props[0]);
        for (uint32_t i = 1; i < props.size(); i++) {
            const auto alt_texel_size = VideoConfig::GetQuantMapTexelSize(props[i]);
            if ((alt_texel_size.width != texel_size.width || alt_texel_size.height != texel_size.height)) {
                return i;
            }
        }
        return 0;
    };

    uint32_t delta_alt_format_index = 0;
    VideoConfig delta_config = GetConfig(FilterConfigs(GetConfigsEncode(), [&](const auto& config) {
        if (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR) {
            const auto& props = config.SupportedQuantDeltaMapProps();
            if (delta_alt_format_index == 0) {
                delta_alt_format_index = get_alt_texel_size_format(props);
                if (delta_alt_format_index != 0) {
                    return true;
                }
            }
        }
        return false;
    }));

    uint32_t emphasis_alt_format_index = 0;
    VideoConfig emphasis_config = GetConfig(FilterConfigs(GetConfigsEncode(), [&](const auto& config) {
        if (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR) {
            const auto& props = config.SupportedEmphasisMapProps();
            if (emphasis_alt_format_index == 0) {
                emphasis_alt_format_index = get_alt_texel_size_format(props);
                if (emphasis_alt_format_index != 0) {
                    return true;
                }
            }
        }
        return false;
    }));

    if (!delta_config && !emphasis_config) {
        GTEST_SKIP() << "Test requires a video profile that has QUANTIZATION_DELTA or EMPHASIS support, with more than two "
                        "supported texel sizes";
    }

    struct TestConfig {
        VideoConfig config;
        VkVideoSessionCreateFlagBitsKHR flag;
        VkExtent2D texel_sizes[2];
    };

    std::vector<TestConfig> tests;
    if (delta_config) {
        const auto& map_props = delta_config.SupportedQuantDeltaMapProps();
        tests.emplace_back(TestConfig{delta_config,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      {
                                          delta_config.GetQuantMapTexelSize(map_props[0]),
                                          delta_config.GetQuantMapTexelSize(map_props[delta_alt_format_index]),
                                      }});
    }
    if (emphasis_config) {
        const auto& map_props = emphasis_config.SupportedEmphasisMapProps();
        tests.emplace_back(TestConfig{emphasis_config,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR,
                                      {
                                          delta_config.GetQuantMapTexelSize(map_props[0]),
                                          delta_config.GetQuantMapTexelSize(map_props[emphasis_alt_format_index]),
                                      }});
    }

    for (auto& [config, flag, texel_sizes] : tests) {
        config.SessionCreateInfo()->flags |= flag;
        VideoContext context(m_device, config);

        auto template_params = context.CreateSessionParamsWithQuantMapTexelSize(texel_sizes[0]);

        VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR map_create_info = vku::InitStructHelper();
        map_create_info.quantizationMapTexelSize = texel_sizes[1];
        VkVideoSessionParametersCreateInfoKHR create_info2 = *config.SessionParamsCreateInfo();
        map_create_info.pNext = create_info2.pNext;
        create_info2.pNext = &map_create_info;
        create_info2.flags = VK_VIDEO_SESSION_PARAMETERS_CREATE_QUANTIZATION_MAP_COMPATIBLE_BIT_KHR;
        create_info2.videoSessionParametersTemplate = template_params;
        create_info2.videoSession = context.Session();

        VkVideoSessionParametersKHR params2 = VK_NULL_HANDLE;
        m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-10276");
        vk::CreateVideoSessionParametersKHR(device(), &create_info2, nullptr, &params2);
        m_errorMonitor->VerifyFound();
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideoEncodeQuantizationMap, EncodeQuantMapTypeMismatch) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - quantization map type specified at encode time does not match video session");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig base_config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR) &&
               (config.SupportedQuantDeltaMapProps().size() > 0) &&
               (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR) &&
               (config.SupportedEmphasisMapProps().size() > 0);
    }));
    if (!base_config) {
        GTEST_SKIP() << "Test requires an encode profile with support for both QUANTIZATION_DELTA and EMPHASIS maps";
    }

    struct TestConfig {
        VkVideoEncodeFlagBitsKHR encode_flag;
        VkVideoSessionCreateFlagBitsKHR session_create_flag;
        const VkVideoFormatPropertiesKHR* map_props;
        const char* vuid;
    };

    const std::vector<TestConfig> tests = {
        {
            VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
            VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
            base_config.EmphasisMapProps(),
            "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10311",
        },
        {VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR, VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR,
         base_config.QuantDeltaMapProps(), "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10312"},
    };

    for (const auto& test : tests) {
        VideoConfig config = base_config;
        config.SessionCreateInfo()->flags |= test.session_create_flag;
        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        const auto texel_size = config.GetQuantMapTexelSize(*test.map_props);
        auto params = context.CreateSessionParamsWithQuantMapTexelSize(texel_size);

        VideoEncodeQuantizationMap quantization_map(m_device, config, *test.map_props);

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().SetSessionParams(params));

        if (test.encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        m_errorMonitor->SetDesiredError(test.vuid);
        cb.EncodeVideo(context.EncodeFrame().QuantizationMap(test.encode_flag, texel_size, quantization_map));
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }
}

TEST_F(NegativeVideoEncodeQuantizationMap, EncodeMissingQuantMapUsage) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - quantization map image usage corresponding to encode flag is missing");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    if ((!delta_config ||
         delta_config.QuantDeltaMapProps()->imageUsageFlags == VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR) &&
        (!emphasis_config ||
         emphasis_config.EmphasisMapProps()->imageUsageFlags == VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR)) {
        GTEST_SKIP() << "Test requires quantization map format to support at least one more usage besides quantization map";
    }

    struct TestConfig {
        VideoConfig config;
        VkVideoEncodeFlagBitsKHR encode_flag;
        VkVideoSessionCreateFlagBitsKHR session_create_flag;
        VkImageUsageFlagBits image_usage;
        const VkVideoFormatPropertiesKHR* map_props;
        const char* vs_create_vuid;
    };

    std::vector<TestConfig> tests = {};
    if (delta_config &&
        delta_config.QuantDeltaMapProps()->imageUsageFlags != VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR) {
        tests.push_back({delta_config, VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
                         VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                         VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR, delta_config.QuantDeltaMapProps(),
                         "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10311"});
    }
    if (emphasis_config &&
        emphasis_config.EmphasisMapProps()->imageUsageFlags != VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR) {
        tests.push_back({delta_config, VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR,
                         VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR,
                         VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR, delta_config.EmphasisMapProps(),
                         "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10312"});
    }

    for (const auto& test : tests) {
        VideoConfig config = test.config;

        config.SessionCreateInfo()->flags |= test.session_create_flag;
        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        const auto texel_size = config.GetQuantMapTexelSize(*test.map_props);
        auto params = context.CreateSessionParamsWithQuantMapTexelSize(texel_size);

        VideoEncodeQuantizationMap quantization_map(m_device, config, *test.map_props);

        auto view_usage_ci = vku::InitStruct<VkImageViewUsageCreateInfo>();
        view_usage_ci.usage = test.map_props->imageUsageFlags ^ test.image_usage;

        auto image_view_ci = vku::InitStruct<VkImageViewCreateInfo>(&view_usage_ci);
        image_view_ci.image = quantization_map.Image();
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        image_view_ci.format = test.map_props->format;
        image_view_ci.components = test.map_props->componentMapping;
        image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

        vkt::ImageView image_view(*m_device, image_view_ci);

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().SetSessionParams(params));

        if (test.encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        m_errorMonitor->SetDesiredError(test.vs_create_vuid);
        cb.EncodeVideo(context.EncodeFrame().QuantizationMap(test.encode_flag, texel_size, image_view));
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (!delta_config ||
        delta_config.QuantDeltaMapProps()->imageUsageFlags == VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR ||
        !emphasis_config ||
        emphasis_config.EmphasisMapProps()->imageUsageFlags == VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideoEncodeQuantizationMap, EncodeIncompatQuantMapProfile) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - quantization map must be compatible with the video profile");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    auto delta_configs = FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR) &&
               (config.SupportedQuantDeltaMapProps().size() > 0);
    });
    auto emphasis_configs = FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR) &&
               (config.SupportedEmphasisMapProps().size() > 0);
    });

    if (delta_configs.size() < 2 && emphasis_configs.size() < 2) {
        GTEST_SKIP()
            << "Test requires at least two profiles that support QUANTIZATION_DELTA, or two profiles that support EMPHASIS";
    }

    struct TestConfig {
        VideoConfig configs[2];
        const VkVideoFormatPropertiesKHR* map_props[2];
        VkVideoEncodeFlagBitsKHR encode_flag;
        VkVideoSessionCreateFlagBitsKHR session_create_flag;
    };

    std::vector<TestConfig> tests;
    if (delta_configs.size() >= 2) {
        tests.emplace_back(TestConfig{{delta_configs[0], delta_configs[1]},
                                      {
                                          delta_configs[0].QuantDeltaMapProps(),
                                          delta_configs[1].QuantDeltaMapProps(),
                                      },
                                      VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR});
    }
    if (emphasis_configs.size() >= 2) {
        tests.emplace_back(TestConfig{{emphasis_configs[0], emphasis_configs[1]},
                                      {
                                          emphasis_configs[0].EmphasisMapProps(),
                                          emphasis_configs[1].EmphasisMapProps(),
                                      },
                                      VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR});
    }

    for (auto& [configs, map_props, encode_flag, session_create_flag] : tests) {
        configs[0].SessionCreateInfo()->flags |= session_create_flag;
        VideoContext context(m_device, configs[0]);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        const VkExtent2D texel_sizes[2] = {configs[0].GetQuantMapTexelSize(*map_props[0]),
                                           configs[1].GetQuantMapTexelSize(*map_props[1])};
        auto params = context.CreateSessionParamsWithQuantMapTexelSize(texel_sizes[0]);

        VideoEncodeQuantizationMap quantization_map(m_device, configs[1], *map_props[1]);

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().SetSessionParams(params));

        if (encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        if (texel_sizes[0].width != texel_sizes[1].width || texel_sizes[0].height != texel_sizes[1].height) {
            // If the two profile's texel sizes do not match, then we may have additional VUs triggered
            // related to the texel sizes and extents of the quantization map
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pNext-10316");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeQuantizationMapInfoKHR-quantizationMapExtent-10352");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeQuantizationMapInfoKHR-quantizationMapExtent-10353");
        }

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10310");
        cb.EncodeVideo(context.EncodeFrame().QuantizationMap(encode_flag, texel_sizes[1], quantization_map));
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (delta_configs.size() < 2 || emphasis_configs.size() < 2) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideoEncodeQuantizationMap, EncodeQuantMapNotAllowed) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - quantization map flag is passed against session that was not created to allow it");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    struct TestConfig {
        VideoConfig config;
        VkVideoEncodeFlagBitsKHR encode_flag;
        const char* vuid;
    };

    std::vector<TestConfig> tests;
    if (delta_config) {
        tests.emplace_back(TestConfig{delta_config, VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10306"});
    }
    if (emphasis_config) {
        tests.emplace_back(
            TestConfig{emphasis_config, VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR, "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10307"});
    }

    for (const auto& [config, encode_flag, vuid] : tests) {
        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        auto params = context.CreateSessionParams();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        VideoEncodeInfo encode_info = context.EncodeFrame();
        encode_info->flags |= encode_flag;

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().SetSessionParams(params));

        if (encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        // VkVideoEncodeQuantizationMapInfoKHR is not chained so we allow related errors
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10309");
        m_errorMonitor->SetDesiredError(vuid);
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideoEncodeQuantizationMap, EncodeQuantMapMissing) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - quantization map missing");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    struct TestConfig {
        VideoConfig config;
        VkVideoEncodeFlagBitsKHR encode_flag;
        VkVideoSessionCreateFlagBitsKHR session_create_flag;
        const VkVideoFormatPropertiesKHR* map_props;
    };

    std::vector<TestConfig> tests;
    if (delta_config) {
        tests.emplace_back(TestConfig{delta_config, VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      delta_config.QuantDeltaMapProps()});
    }
    if (emphasis_config) {
        tests.emplace_back(TestConfig{emphasis_config, VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR,
                                      emphasis_config.EmphasisMapProps()});
    }

    for (auto& [config, encode_flag, session_create_flag, map_props] : tests) {
        config.SessionCreateInfo()->flags |= session_create_flag;
        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        const auto texel_size = config.GetQuantMapTexelSize(*map_props);
        auto params = context.CreateSessionParamsWithQuantMapTexelSize(texel_size);

        vkt::CommandBuffer& cb = context.CmdBuffer();

        VideoEncodeInfo encode_info = context.EncodeFrame();
        encode_info->flags |= encode_flag;

        // No VkVideoEncodeQuantizationMapInfoKHR in pNext
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().SetSessionParams(params));

        if (encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10309");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        VkVideoEncodeQuantizationMapInfoKHR quantization_map_info = vku::InitStructHelper();
        quantization_map_info.quantizationMapExtent.width =
            (encode_info->srcPictureResource.codedExtent.width + texel_size.width - 1) / texel_size.width;
        quantization_map_info.quantizationMapExtent.height =
            (encode_info->srcPictureResource.codedExtent.height + texel_size.height - 1) / texel_size.height;
        quantization_map_info.pNext = encode_info->pNext;
        encode_info->pNext = &quantization_map_info;

        // VkVideoEncodeQuantizationMapInfoKHR::quantizationMap = VK_NULL_HANDLE
        quantization_map_info.quantizationMap = VK_NULL_HANDLE;

        if (encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10309");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideoEncodeQuantizationMap, EncodeParamsNotQuantMapCompatible) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - video session parameters object is not quantization map compatible");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    struct TestConfig {
        VideoConfig config;
        VkVideoEncodeFlagBitsKHR encode_flag;
        VkVideoSessionCreateFlagBitsKHR session_create_flag;
        const VkVideoFormatPropertiesKHR* map_props;
    };

    std::vector<TestConfig> tests;
    if (delta_config) {
        tests.emplace_back(TestConfig{delta_config, VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      delta_config.QuantDeltaMapProps()});
    }
    if (emphasis_config) {
        tests.emplace_back(TestConfig{emphasis_config, VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR,
                                      emphasis_config.EmphasisMapProps()});
    }

    for (auto& [config, encode_flag, session_create_flag, map_props] : tests) {
        config.SessionCreateInfo()->flags |= session_create_flag;
        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        VideoEncodeQuantizationMap quantization_map(m_device, config, *map_props);

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin());

        if (encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-10315");
        cb.EncodeVideo(
            context.EncodeFrame().QuantizationMap(encode_flag, config.GetQuantMapTexelSize(*map_props), quantization_map));
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideoEncodeQuantizationMap, EncodeQuantMapExtentCodedExtentMismatch) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - quantizationMapExtent inconsistent with the coded extent");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    struct TestConfig {
        VideoConfig config;
        VkVideoEncodeFlagBitsKHR encode_flag;
        VkVideoSessionCreateFlagBitsKHR session_create_flag;
        const VkVideoFormatPropertiesKHR* map_props;
    };

    std::vector<TestConfig> tests;
    if (delta_config) {
        tests.emplace_back(TestConfig{delta_config, VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      delta_config.QuantDeltaMapProps()});
    }
    if (emphasis_config) {
        tests.emplace_back(TestConfig{emphasis_config, VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR,
                                      emphasis_config.EmphasisMapProps()});
    }

    for (auto& [config, encode_flag, session_create_flag, map_props] : tests) {
        config.SessionCreateInfo()->flags |= session_create_flag;
        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        const auto texel_size = config.GetQuantMapTexelSize(*map_props);
        auto params = context.CreateSessionParamsWithQuantMapTexelSize(texel_size);

        vkt::CommandBuffer& cb = context.CmdBuffer();

        VideoEncodeQuantizationMap quantization_map(m_device, config, *map_props);
        VideoEncodeInfo encode_info = context.EncodeFrame().QuantizationMap(encode_flag, texel_size, quantization_map);

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().SetSessionParams(params));

        if (encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        encode_info.QuantizationMapInfo().quantizationMapExtent.width -= 1;
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-10316");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
        encode_info.QuantizationMapInfo().quantizationMapExtent.width += 1;

        if (encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        encode_info.QuantizationMapInfo().quantizationMapExtent.height -= 1;
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-10316");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
        encode_info.QuantizationMapInfo().quantizationMapExtent.height += 1;

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideoEncodeQuantizationMap, EncodeQuantMapExtentViewExtentMismatch) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - quantizationMapExtent inconsistent with the image view extent");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    struct TestConfig {
        VideoConfig config;
        VkVideoEncodeFlagBitsKHR encode_flag;
        VkVideoSessionCreateFlagBitsKHR session_create_flag;
        const VkVideoFormatPropertiesKHR* map_props;
    };

    std::vector<TestConfig> tests;
    if (delta_config) {
        tests.emplace_back(TestConfig{delta_config, VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      delta_config.QuantDeltaMapProps()});
    }
    if (emphasis_config) {
        tests.emplace_back(TestConfig{emphasis_config, VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR,
                                      emphasis_config.EmphasisMapProps()});
    }

    for (auto& [config, encode_flag, session_create_flag, map_props] : tests) {
        config.SessionCreateInfo()->flags |= session_create_flag;
        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        const auto texel_size = config.GetQuantMapTexelSize(*map_props);
        auto params = context.CreateSessionParamsWithQuantMapTexelSize(texel_size);

        vkt::CommandBuffer& cb = context.CmdBuffer();

        VideoEncodeQuantizationMap quantization_map(m_device, config, *map_props);
        VideoEncodeInfo encode_info = context.EncodeFrame().QuantizationMap(encode_flag, texel_size, quantization_map);

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().SetSessionParams(params));

        // quantizationMapExtent is not consistent with the codedExtent
        if (encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        // This will certainly cause quantizationMapExtent to have a mismatch with codedExtent
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pNext-10316");

        encode_info.QuantizationMapInfo().quantizationMapExtent.width += 1;
        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeQuantizationMapInfoKHR-quantizationMapExtent-10352");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
        encode_info.QuantizationMapInfo().quantizationMapExtent.width -= 1;

        if (encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        // This will certainly cause quantizationMapExtent to have a mismatch with codedExtent
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pNext-10316");

        encode_info.QuantizationMapInfo().quantizationMapExtent.height += 1;
        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeQuantizationMapInfoKHR-quantizationMapExtent-10353");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
        encode_info.QuantizationMapInfo().quantizationMapExtent.height -= 1;

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideoEncodeQuantizationMap, EncodeWithEmphasisMapIncompatibleRateControlMode) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - emphasis map used with incompatible rate control mode");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig config =
        GetConfigWithEmphasisMap(GetConfigsWithRateControl(GetConfigsEncode(), VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR));
    if (!config) {
        // If DISABLED is not supported, just use any encode config with emphasis map support available
        config = GetConfigWithEmphasisMap(GetConfigsEncode());
    }
    if (!config) {
        GTEST_SKIP() << "Test requires emphasis map support";
    }

    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR;
    VideoContext context(m_device, config);

    const auto texel_size = config.EmphasisMapTexelSize();
    auto params = context.CreateSessionParamsWithQuantMapTexelSize(texel_size);

    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoEncodeQuantizationMap quantization_map(m_device, config, *config.EmphasisMapProps());

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().SetSessionParams(params));

    // Test with DEFAULT rate control mode
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
    cb.EncodeVideo(context.EncodeFrame().QuantizationMap(VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR, texel_size, quantization_map));
    m_errorMonitor->VerifyFound();

    // If supported, also test with DISABLED rate control mode
    if (config.EncodeCaps()->rateControlModes & VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR) {
        auto rc_info = VideoEncodeRateControlInfo(config);
        rc_info->rateControlMode = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR;
        cb.ControlVideoCoding(context.Control().RateControl(rc_info));

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        cb.EncodeVideo(
            context.EncodeFrame().QuantizationMap(VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR, texel_size, quantization_map));
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeQuantizationMap, EncodeQuantMapImageLayout) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - quantization map is not in the correct image layout at execution time");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    struct TestConfig {
        VkVideoEncodeFlagBitsKHR encode_flag;
        VkVideoSessionCreateFlagBitsKHR session_create_flag;
        VideoConfig config;
        const VkVideoFormatPropertiesKHR* map_props;
    };

    std::vector<TestConfig> tests;
    if (delta_config) {
        tests.emplace_back(TestConfig{VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR, delta_config,
                                      delta_config.QuantDeltaMapProps()});
    }
    if (emphasis_config) {
        tests.emplace_back(TestConfig{VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR, emphasis_config,
                                      emphasis_config.EmphasisMapProps()});
    }

    for (const auto& test : tests) {
        VideoConfig config = test.config;

        config.SessionCreateInfo()->flags |= test.session_create_flag;
        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        const auto texel_size = config.GetQuantMapTexelSize(*test.map_props);
        auto params = context.CreateSessionParamsWithQuantMapTexelSize(texel_size);

        VideoEncodeQuantizationMap quantization_map(m_device, config, *test.map_props);

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        // quantization map must be in VK_IMAGE_LAYOUT_ENCODE_QUANTIZATION_MAP_KHR
        vk::CmdPipelineBarrier2KHR(cb.handle(), quantization_map.LayoutTransition(VK_IMAGE_LAYOUT_GENERAL, 0, 1));
        cb.BeginVideoCoding(context.Begin().SetSessionParams(params));

        if (test.encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-10314");
        cb.EncodeVideo(context.EncodeFrame().QuantizationMap(test.encode_flag, texel_size, quantization_map));
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideoEncodeQuantizationMap, EncodeProtectedNoFaultQuantMap) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - protectedNoFault tests for quantization map");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(Init());
    if (IsProtectedNoFaultSupported()) {
        GTEST_SKIP() << "Test requires protectedMemory support without protectedNoFault support";
    }

    const bool use_protected = true;

    VideoConfig delta_config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return (config.Caps()->flags & VK_VIDEO_CAPABILITY_PROTECTED_CONTENT_BIT_KHR) &&
               (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR) &&
               (config.SupportedQuantDeltaMapProps().size() > 0);
    }));
    VideoConfig emphasis_config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return (config.Caps()->flags & VK_VIDEO_CAPABILITY_PROTECTED_CONTENT_BIT_KHR) &&
               (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR) &&
               (config.SupportedEmphasisMapProps().size() > 0);
    }));
    if (!delta_config && !emphasis_config) {
        GTEST_SKIP() << "Test requires an encode profile that supports protected content and quantization maps";
    }

    struct TestConfig {
        VideoConfig config;
        VkVideoEncodeFlagBitsKHR encode_flag;
        VkVideoSessionCreateFlagBitsKHR session_create_flag;
        const VkVideoFormatPropertiesKHR* map_props;
    };

    std::vector<TestConfig> tests;
    if (delta_config) {
        tests.emplace_back(TestConfig{delta_config, VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      delta_config.QuantDeltaMapProps()});
    }
    if (emphasis_config) {
        tests.emplace_back(TestConfig{emphasis_config, VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR,
                                      emphasis_config.EmphasisMapProps()});
    }

    for (auto& [config, encode_flag, session_create_flag, map_props] : tests) {
        config.SessionCreateInfo()->flags |= session_create_flag;

        VideoContext unprotected_context(m_device, config);
        unprotected_context.CreateAndBindSessionMemory();
        unprotected_context.CreateResources(false /* bitstream */, false /* DPB */, false /* src_image */);

        vkt::CommandBuffer& unprotected_cb = unprotected_context.CmdBuffer();

        VideoContext protected_context(m_device, config, use_protected);
        protected_context.CreateAndBindSessionMemory();
        protected_context.CreateResources(use_protected /* bitstream */, use_protected /* DPB */, use_protected /* src_image */);

        vkt::CommandBuffer& protected_cb = protected_context.CmdBuffer();

        const auto texel_size = config.GetQuantMapTexelSize(*map_props);
        auto unprotected_params = unprotected_context.CreateSessionParamsWithQuantMapTexelSize(texel_size);
        auto protected_params = protected_context.CreateSessionParamsWithQuantMapTexelSize(texel_size);

        VideoEncodeQuantizationMap unprotected_quantization_map(m_device, config, *map_props, false);
        VideoEncodeQuantizationMap protected_quantization_map(m_device, config, *map_props, true);

        unprotected_cb.Begin();
        unprotected_cb.BeginVideoCoding(unprotected_context.Begin().SetSessionParams(unprotected_params));

        if (encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-10313");
        unprotected_cb.EncodeVideo(
            unprotected_context.EncodeFrame().QuantizationMap(encode_flag, texel_size, protected_quantization_map));
        m_errorMonitor->VerifyFound();

        unprotected_cb.EndVideoCoding(unprotected_context.End());
        unprotected_cb.End();

        protected_cb.Begin();
        protected_cb.BeginVideoCoding(protected_context.Begin().SetSessionParams(protected_params));

        if (encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        protected_cb.EncodeVideo(
            protected_context.EncodeFrame().QuantizationMap(encode_flag, texel_size, unprotected_quantization_map));
        m_errorMonitor->VerifyFound();

        protected_cb.EndVideoCoding(protected_context.End());
        protected_cb.End();
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideoEncodeQuantizationMap, CreateImageVideoEncodeQuantizationMapNotEnabled) {
    TEST_DESCRIPTION("vkCreateImage - quantization map usage is specified but videoEncodeQuantizationMap was not enabled");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    ForceDisableFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    std::vector<std::tuple<VideoConfig, VkImageUsageFlagBits, const VkVideoFormatPropertiesKHR*>> quantization_map_usages;
    if (delta_config) {
        quantization_map_usages.emplace_back(delta_config, VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                             delta_config.QuantDeltaMapProps());
    }
    if (emphasis_config) {
        quantization_map_usages.emplace_back(emphasis_config, VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR,
                                             emphasis_config.EmphasisMapProps());
    }

    for (const auto& [config, usage, props] : quantization_map_usages) {
        VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
        profile_list.profileCount = 1;
        profile_list.pProfiles = config.Profile();

        VkImageCreateInfo image_ci = vku::InitStructHelper();
        image_ci.flags = 0;
        image_ci.pNext = &profile_list;
        image_ci.extent = {
            config.EncodeQuantizationMapCaps()->maxQuantizationMapExtent.width,
            config.EncodeQuantizationMapCaps()->maxQuantizationMapExtent.height,
            1,
        };
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_ci.usage = usage;
        image_ci.imageType = props->imageType;
        image_ci.format = props->format;
        image_ci.tiling = props->imageTiling;

        VkImage image = VK_NULL_HANDLE;
        m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-10251");
        vk::CreateImage(device(), &image_ci, nullptr, &image);
        m_errorMonitor->VerifyFound();
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideoEncodeQuantizationMap, CreateImageQuantDeltaMapUnsupportedProfile) {
    TEST_DESCRIPTION("vkCreateImage - quantization delta map image created against profile without cap");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR) == 0;
    }));
    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    if (!config || !delta_config) {
        GTEST_SKIP() << "Test requires an encode profile without quantization delta map support";
    }

    const VkVideoFormatPropertiesKHR* format_props = delta_config.QuantDeltaMapProps();

    VideoContext context(m_device, config);

    VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
    profile_list.profileCount = 1;
    profile_list.pProfiles = config.Profile();

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = 0;
    image_ci.pNext = &profile_list;
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = format_props->format;
    image_ci.extent = {1, 1, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = format_props->imageTiling;
    image_ci.usage = VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR;
    image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImage image = VK_NULL_HANDLE;

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251");
    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-10255");
    vk::CreateImage(device(), &image_ci, nullptr, &image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeQuantizationMap, CreateImageEmphasisMapUnsupportedProfile) {
    TEST_DESCRIPTION("vkCreateImage - emphasis map image created against profile without cap");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR) == 0;
    }));
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    if (!config || !emphasis_config) {
        GTEST_SKIP() << "Test requires an encode profile without emphasis map support";
    }

    const VkVideoFormatPropertiesKHR* format_props = emphasis_config.EmphasisMapProps();

    VideoContext context(m_device, config);

    VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
    profile_list.profileCount = 1;
    profile_list.pProfiles = config.Profile();

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = 0;
    image_ci.pNext = &profile_list;
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = format_props->format;
    image_ci.extent = {1, 1, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = format_props->imageTiling;
    image_ci.usage = VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR;
    image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImage image = VK_NULL_HANDLE;

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251");
    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-10256");
    vk::CreateImage(device(), &image_ci, nullptr, &image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeQuantizationMap, CreateImageQuantMapProfileIndependent) {
    TEST_DESCRIPTION("vkCreateImage - quantization maps cannot be profile independent");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    std::vector<std::tuple<VideoConfig, VkImageUsageFlagBits, const VkVideoFormatPropertiesKHR*>> quantization_map_usages;
    if (delta_config) {
        quantization_map_usages.emplace_back(delta_config, VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                             delta_config.QuantDeltaMapProps());
    }
    if (emphasis_config) {
        quantization_map_usages.emplace_back(emphasis_config, VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR,
                                             emphasis_config.EmphasisMapProps());
    }

    for (const auto& [config, usage, props] : quantization_map_usages) {
        VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
        profile_list.profileCount = 1;
        profile_list.pProfiles = config.Profile();

        VkImageCreateInfo image_ci = vku::InitStructHelper();
        image_ci.flags = VK_IMAGE_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR;
        image_ci.pNext = &profile_list;
        image_ci.extent = {
            config.EncodeQuantizationMapCaps()->maxQuantizationMapExtent.width,
            config.EncodeQuantizationMapCaps()->maxQuantizationMapExtent.height,
            1,
        };
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_ci.usage = usage;
        image_ci.imageType = props->imageType;
        image_ci.format = props->format;
        image_ci.tiling = props->imageTiling;

        VkImage image = VK_NULL_HANDLE;
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251");
        m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-flags-08331");
        vk::CreateImage(device(), &image_ci, nullptr, &image);
        m_errorMonitor->VerifyFound();
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideoEncodeQuantizationMap, CreateImageQuantMapInvalidParameters) {
    TEST_DESCRIPTION("vkCreateImage - image parameters are incompatible with quantization maps");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    VideoConfig decode_config = GetConfigDecode();

    std::vector<std::tuple<VideoConfig, VkImageUsageFlagBits, const VkVideoFormatPropertiesKHR*>> quantization_map_usages;
    if (delta_config) {
        quantization_map_usages.emplace_back(delta_config, VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                             delta_config.QuantDeltaMapProps());
    }
    if (emphasis_config) {
        quantization_map_usages.emplace_back(emphasis_config, VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR,
                                             emphasis_config.EmphasisMapProps());
    }

    for (const auto& [config, usage, props] : quantization_map_usages) {
        VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
        VkVideoProfileInfoKHR profiles[] = {*config.Profile(), *config.Profile()};
        profile_list.profileCount = 1;
        profile_list.pProfiles = profiles;

        VkImageCreateInfo image_ci = vku::InitStructHelper();
        image_ci.pNext = &profile_list;
        image_ci.flags = 0;
        image_ci.imageType = VK_IMAGE_TYPE_2D;
        image_ci.format = props->format;
        image_ci.extent = {1, 1, 1};
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.tiling = props->imageTiling;
        image_ci.usage = usage;
        image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VkImage image = VK_NULL_HANDLE;

        image_ci.imageType = VK_IMAGE_TYPE_1D;
        // Implementations are not expected to support non-2D quantization maps
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251");
        m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-10252");
        vk::CreateImage(device(), &image_ci, nullptr, &image);
        m_errorMonitor->VerifyFound();

        image_ci.imageType = VK_IMAGE_TYPE_2D;
        image_ci.samples = VK_SAMPLE_COUNT_2_BIT;
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-samples-02257");
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-samples-02258");
        m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-10253");
        vk::CreateImage(device(), &image_ci, nullptr, &image);
        m_errorMonitor->VerifyFound();

        // Try no profile list
        image_ci.pNext = nullptr;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-10254");
        vk::CreateImage(device(), &image_ci, nullptr, &image);
        m_errorMonitor->VerifyFound();

        // Try profileCount != 1
        image_ci.pNext = &profile_list;
        profile_list.profileCount = 0;
        m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-10254");
        vk::CreateImage(device(), &image_ci, nullptr, &image);
        m_errorMonitor->VerifyFound();

        profile_list.profileCount = 2;
        m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-10254");
        vk::CreateImage(device(), &image_ci, nullptr, &image);
        m_errorMonitor->VerifyFound();

        // Try decode profile, if available
        profile_list.profileCount = 1;
        if (decode_config) {
            profiles[0] = *decode_config.Profile();
            // Implementations probably don't support quantization map formats with decode profile
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251");
            m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-10254");
            vk::CreateImage(device(), &image_ci, nullptr, &image);
            m_errorMonitor->VerifyFound();
        }

        // Try extent > maxQuantizationMapExtent
        profiles[0] = *config.Profile();
        image_ci.extent.width = config.EncodeQuantizationMapCaps()->maxQuantizationMapExtent.width + 1;
        m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-10257");
        vk::CreateImage(device(), &image_ci, nullptr, &image);
        m_errorMonitor->VerifyFound();

        image_ci.extent.width = 1;
        image_ci.extent.height = config.EncodeQuantizationMapCaps()->maxQuantizationMapExtent.height + 1;
        m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-10258");
        vk::CreateImage(device(), &image_ci, nullptr, &image);
        m_errorMonitor->VerifyFound();
    };

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideoEncodeQuantizationMap, CreateImageViewQuantMapInvalidViewType) {
    TEST_DESCRIPTION("vkCreateImageView - view type not compatible with quantization maps");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    std::vector<std::tuple<VideoConfig, VkImageUsageFlagBits, const VkVideoFormatPropertiesKHR*>> quantization_map_usages;
    if (delta_config) {
        quantization_map_usages.emplace_back(delta_config, VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                             delta_config.QuantDeltaMapProps());
    }
    if (emphasis_config) {
        quantization_map_usages.emplace_back(emphasis_config, VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR,
                                             emphasis_config.EmphasisMapProps());
    }

    for (const auto& [config, usage, props] : quantization_map_usages) {
        VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
        profile_list.profileCount = 1;
        profile_list.pProfiles = config.Profile();

        VkImageCreateInfo image_ci = vku::InitStructHelper();
        image_ci.pNext = &profile_list;
        image_ci.imageType = props->imageType;
        image_ci.format = props->format;
        image_ci.extent = {1, 1, 1};
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.tiling = props->imageTiling;
        image_ci.usage = props->imageUsageFlags;
        image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        vkt::Image image(*m_device, image_ci);
        VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
        image_view_ci.image = image.handle();
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_1D;
        image_view_ci.format = image_ci.format;
        image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_ci.subresourceRange.levelCount = 1;
        image_view_ci.subresourceRange.layerCount = 1;

        m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageViewCreateInfo-subResourceRange-01021");
        m_errorMonitor->SetDesiredError("VUID-VkImageViewCreateInfo-image-10261");
        VkImageView image_view = VK_NULL_HANDLE;
        vk::CreateImageView(device(), &image_view_ci, nullptr, &image_view);
        m_errorMonitor->VerifyFound();
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideoEncodeQuantizationMap, CreateImageViewQuantMapUnsupportedFormatFeatures) {
    TEST_DESCRIPTION("vkCreateImageView - quantization map view format features unsupported");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    std::vector<std::tuple<VideoConfig, VkImageUsageFlagBits, const VkVideoFormatPropertiesKHR*, const char*>>
        quantization_map_usages;
    if (delta_config) {
        quantization_map_usages.emplace_back(delta_config, VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                             delta_config.QuantDeltaMapProps(), "VUID-VkImageViewCreateInfo-usage-10259");
    }
    if (emphasis_config) {
        quantization_map_usages.emplace_back(emphasis_config, VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR,
                                             emphasis_config.EmphasisMapProps(), "VUID-VkImageViewCreateInfo-usage-10260");
    }

    for (auto& [config, usage, props, vuid] : quantization_map_usages) {
        VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
        profile_list.profileCount = 1;
        profile_list.pProfiles = config.Profile();

        VkImageCreateInfo image_ci = vku::InitStructHelper();
        image_ci.pNext = &profile_list;
        image_ci.flags = 0;
        image_ci.imageType = props->imageType;
        image_ci.format = props->format;
        image_ci.extent = {1, 1, 1};
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.tiling = props->imageTiling;
        image_ci.usage = props->imageUsageFlags;
        image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        vkt::Image image(*m_device, image_ci);
        VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
        image_view_ci.image = image.handle();
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        // We will just use a definitely not supported view format here because we anyway
        // did not create the image with MUTABLE, so no need to worry about video format compatibility
        image_view_ci.format = VK_FORMAT_D32_SFLOAT;
        image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_ci.subresourceRange.levelCount = 1;
        image_view_ci.subresourceRange.layerCount = 1;

        // The image is not created with MUTABLE, so we need to allow this VU.
        // This should also bypass view format compatibility checks
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageViewCreateInfo-image-01762");
        m_errorMonitor->SetDesiredError(vuid);
        VkImageView image_view = VK_NULL_HANDLE;
        vk::CreateImageView(device(), &image_view_ci, nullptr, &image_view);
        m_errorMonitor->VerifyFound();
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}
