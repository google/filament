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
#include "generated/enum_flag_bits.h"

class NegativeVideoEncode : public VkVideoLayerTest {};

TEST_F(NegativeVideoEncode, EncodeQualityLevelPropsUnsupportedProfile) {
    TEST_DESCRIPTION("vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR - unsupported profile");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigEncodeInvalid();
    if (!config) {
        GTEST_SKIP() << "Test requires encode support";
    }

    auto quality_level_info = vku::InitStruct<VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR>();
    quality_level_info.pVideoProfile = config.Profile();

    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR-pVideoProfile-08259");
    vk::GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(Gpu(), &quality_level_info, config.EncodeQualityLevelProps());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncode, EncodeQualityLevelPropsProfileNotEncode) {
    TEST_DESCRIPTION("vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR - profile is not an encode profile");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigDecode();
    if (!config || !GetConfigEncode()) {
        GTEST_SKIP() << "Test requires decode and encode support";
    }

    auto quality_level_info = vku::InitStruct<VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR>();
    quality_level_info.pVideoProfile = config.Profile();

    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR-pVideoProfile-08260");
    vk::GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(Gpu(), &quality_level_info, config.EncodeQualityLevelProps());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncode, EncodeQualityLevelPropsInvalidQualityLevel) {
    TEST_DESCRIPTION("vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR - invalid quality level");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires encode support";
    }

    auto quality_level_info = vku::InitStruct<VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR>();
    quality_level_info.pVideoProfile = config.Profile();
    quality_level_info.qualityLevel = config.EncodeCaps()->maxQualityLevels;

    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR-qualityLevel-08261");
    vk::GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(Gpu(), &quality_level_info, config.EncodeQualityLevelProps());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncode, CreateSessionInvalidEncodeReferencePictureFormat) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid encode reference picture format");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-referencePictureFormat-06814");
    create_info.referencePictureFormat = VK_FORMAT_D16_UNORM;
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncode, CreateSessionInvalidEncodePictureFormat) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid encode picture format");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pictureFormat-04854");
    create_info.pictureFormat = VK_FORMAT_D16_UNORM;
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncode, CreateSessionParamsIncompatibleTemplateEncodeQualityLevel) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - mismatch in encode quality level for template");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigWithMultiEncodeQualityLevelParams(GetConfigsEncode());
    if (!config) {
        GTEST_SKIP() << "Test requires an encode profile with support for parameters objects and at least two quality levels";
    }

    VideoContext context(m_device, config);

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    auto quality_level_info = vku::InitStruct<VkVideoEncodeQualityLevelInfoKHR>();
    quality_level_info.pNext = create_info.pNext;
    create_info.pNext = &quality_level_info;
    create_info.videoSession = context.Session();

    // Expect to fail to create parameters object with max encode quality level
    // with template using encode quality level 0
    create_info.videoSessionParametersTemplate = context.SessionParams();
    quality_level_info.qualityLevel = 1;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-08310");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    // Expect to succeed to create parameters object with explicit encode quality level 0 against the same template
    quality_level_info.qualityLevel = 0;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);

    // Expect to succeed to create parameters object with highest encode quality level index but no template
    create_info.videoSessionParametersTemplate = VK_NULL_HANDLE;
    quality_level_info.qualityLevel = config.EncodeCaps()->maxQualityLevels - 1;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);

    // Expect to fail to create parameters object with encode quality level 0
    // with template using highest encode quality level index
    create_info.videoSessionParametersTemplate = params;
    quality_level_info.qualityLevel = 0;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-08310");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    // Expect to fail the same with implicit encode quality level 0
    create_info.pNext = quality_level_info.pNext;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-08310");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideoEncode, CreateSessionParamsInvalidEncodeQualityLevel) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - invalid encode quality level");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigWithParams(GetConfigsEncode());
    if (!config) {
        GTEST_SKIP() << "Test requires an encode profile with session parameters";
    }

    VideoContext context(m_device, config);

    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    auto quality_level_info = vku::InitStruct<VkVideoEncodeQualityLevelInfoKHR>();
    quality_level_info.qualityLevel = config.EncodeCaps()->maxQualityLevels;
    quality_level_info.pNext = create_info.pNext;
    create_info.pNext = &quality_level_info;
    create_info.videoSession = context.Session();

    VkVideoSessionParametersKHR params;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeQualityLevelInfoKHR-qualityLevel-08311");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncode, GetEncodedSessionParamsRequiresEncodeProfile) {
    TEST_DESCRIPTION("vkGetEncodedVideoSessionParametersKHR - video session parameters object must use encode profile");

    RETURN_IF_SKIP(Init());
    VideoConfig decode_config = GetConfigWithParams(GetConfigsDecode());
    VideoConfig encode_config = GetConfigEncode();
    if (!decode_config || !encode_config) {
        GTEST_SKIP() << "Test requires video encode support and a decode profile with session parameters";
    }

    VideoContext decode_context(m_device, decode_config);
    VideoContext encode_context(m_device, encode_config);

    auto get_info = vku::InitStruct<VkVideoEncodeSessionParametersGetInfoKHR>();
    get_info.videoSessionParameters = decode_context.SessionParams();
    size_t data_size = 0;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08359");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncode, BeginCodingSlotInactive) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - referenced DPB slot is inactive");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile with reference picture support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
    cb.EndVideoCoding(context.End());
    cb.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
    cb.ControlVideoCoding(context.Control().Reset());
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.EncodeInput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR, 0, 1));
    cb.EncodeVideo(context.EncodeReferenceFrame(0));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().InvalidateSlot(0));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
    cb.EndVideoCoding(context.End());
    cb.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeVideoEncode, BeginCodingInvalidSlotResourceAssociation) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - referenced DPB slot is not associated with the specified resource");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile with reference picture support and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
    cb.ControlVideoCoding(context.Control().Reset());
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.EncodeInput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR, 0, 1));
    cb.EncodeVideo(context.EncodeReferenceFrame(0));
    cb.EndVideoCoding(context.End());

    cb.BeginVideoCoding(context.Begin().AddResource(0, 1));
    cb.EndVideoCoding(context.End());
    cb.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-pPictureResource-07265");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeVideoEncode, BeginCodingMissingEncodeDpbUsage) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - reference picture resource missing VIDEO_ENCODE_DPB usage");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsEncode(), 1));
    if (!config) {
        GTEST_SKIP() << "Test requires an encode profile with reference picture support";
    }

    if (config.DpbFormatProps()->imageUsageFlags == VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR) {
        GTEST_SKIP() << "Test requires reference format to support at least one more usage besides ENCODE_DPB";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoPictureResourceInfoKHR res = context.Dpb()->Picture(0);

    auto view_usage_ci = vku::InitStruct<VkImageViewUsageCreateInfo>();
    view_usage_ci.usage = config.DpbFormatProps()->imageUsageFlags ^ VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR;

    auto image_view_ci = vku::InitStruct<VkImageViewCreateInfo>(&view_usage_ci);
    image_view_ci.image = context.Dpb()->Image();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    image_view_ci.format = config.DpbFormatProps()->format;
    image_view_ci.components = config.DpbFormatProps()->componentMapping;
    image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    vkt::ImageView image_view(*m_device, image_view_ci);

    res.imageViewBinding = image_view.handle();

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-slotIndex-07246");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideoEncode, ControlEncodeQualityLevelInvalid) {
    TEST_DESCRIPTION("vkCmdControlVideoCodingKHR - invalid encode quality level");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto quality_level_info = vku::InitStruct<VkVideoEncodeQualityLevelInfoKHR>();
    quality_level_info.qualityLevel = config.EncodeCaps()->maxQualityLevels;
    auto control_info = vku::InitStruct<VkVideoCodingControlInfoKHR>(&quality_level_info);
    control_info.flags = VK_VIDEO_CODING_CONTROL_ENCODE_QUALITY_LEVEL_BIT_KHR;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeQualityLevelInfoKHR-qualityLevel-08311");
    cb.ControlVideoCoding(control_info);
    m_errorMonitor->VerifyFound();
    cb.EndVideoCoding(context.End());
    cb.End();

    m_device->Wait();
}

TEST_F(NegativeVideoEncode, ControlEncodeQualityLevelRequiresEncodeSession) {
    TEST_DESCRIPTION("vkCmdControlVideoCodingKHR - encode quality level control requires an encode session");

    RETURN_IF_SKIP(Init());

    VideoConfig encode_config = GetConfigEncode();
    VideoConfig decode_config = GetConfigDecode();
    if (!encode_config || !decode_config) {
        GTEST_SKIP() << "Test requires video decode and encode support";
    }

    VideoContext context(m_device, decode_config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto control_info = vku::InitStruct<VkVideoCodingControlInfoKHR>();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    control_info.flags = VK_VIDEO_CODING_CONTROL_ENCODE_QUALITY_LEVEL_BIT_KHR;
    m_errorMonitor->SetDesiredError("VUID-vkCmdControlVideoCodingKHR-pCodingControlInfo-08243");
    cb.ControlVideoCoding(control_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, ControlEncodeQualityLevelMissingChain) {
    TEST_DESCRIPTION("vkCmdControlVideoCodingKHR - missing encode quality level info");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto control_info = vku::InitStruct<VkVideoCodingControlInfoKHR>();
    control_info.flags = VK_VIDEO_CODING_CONTROL_ENCODE_QUALITY_LEVEL_BIT_KHR;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->SetDesiredError("VUID-VkVideoCodingControlInfoKHR-flags-08349");
    cb.ControlVideoCoding(control_info);
    m_errorMonitor->VerifyFound();
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeParamsQualityLevelMismatch) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - quality level of bound parameters object does not match current quality level");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigWithMultiEncodeQualityLevelParams(GetConfigsEncode());
    if (!config) {
        GTEST_SKIP() << "Test requires an encode profile with support for parameters objects and at least two quality levels";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoSessionParametersKHR params_quality_level_1;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    auto quality_level_info = vku::InitStruct<VkVideoEncodeQualityLevelInfoKHR>();
    quality_level_info.pNext = create_info.pNext;
    quality_level_info.qualityLevel = 1;
    create_info.pNext = &quality_level_info;
    create_info.videoSession = context.Session();

    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params_quality_level_1);

    // Submitting command buffer using mismatching parameters object encode quality level is fine as long as
    // there is no encode operation recorded
    cb.Begin();
    cb.BeginVideoCoding(context.Begin().SetSessionParams(params_quality_level_1));
    cb.ControlVideoCoding(context.Control().Reset());
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    context.Queue().Wait();

    // Submitting command buffer with matching parameters object encode quality level is fine even if the
    // quality is modified afterwards.
    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.EncodeVideo(context.EncodeFrame());
    cb.ControlVideoCoding(context.Control().EncodeQualityLevel(1));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    context.Queue().Wait();

    // Submitting command buffer with matching parameters object encode quality level is fine, even if state
    // is not set here
    cb.Begin();
    cb.BeginVideoCoding(context.Begin().SetSessionParams(params_quality_level_1));
    cb.EncodeVideo(context.EncodeFrame());
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    context.Queue().Wait();

    // Submitting command buffer with mismatching parameters object is fine if the quality level is modified
    // to the right one before the encode operation
    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().EncodeQualityLevel(0));
    cb.EncodeVideo(context.EncodeFrame());
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    context.Queue().Wait();

    // Submitting an encode operation with mismatching effective and parameters encode quality levels should fail
    cb.Begin();
    cb.BeginVideoCoding(context.Begin().SetSessionParams(params_quality_level_1));
    cb.EncodeVideo(context.EncodeFrame());
    cb.EndVideoCoding(context.End());
    cb.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-None-08318");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();

    // Same goes when encode quality level state is changed before/after
    // First test cases where command buffer recording time validation is possible
    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.EncodeVideo(context.EncodeFrame());
    cb.ControlVideoCoding(context.Control().EncodeQualityLevel(1));

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-None-08318");
    cb.EncodeVideo(context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    cb.ControlVideoCoding(context.Control().EncodeQualityLevel(0));
    cb.EncodeVideo(context.EncodeFrame());
    cb.EndVideoCoding(context.End());

    cb.BeginVideoCoding(context.Begin().SetSessionParams(params_quality_level_1));
    cb.EncodeVideo(context.EncodeFrame());  // This would only generate an error at submit time
    cb.ControlVideoCoding(context.Control().EncodeQualityLevel(1));
    cb.EncodeVideo(context.EncodeFrame());
    cb.ControlVideoCoding(context.Control().EncodeQualityLevel(0));

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-None-08318");
    cb.EncodeVideo(context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();

    // Then test a case where only submit time validation is possible
    cb.Begin();
    cb.BeginVideoCoding(context.Begin().SetSessionParams(params_quality_level_1));
    cb.EncodeVideo(context.EncodeFrame());
    cb.EndVideoCoding(context.End());
    cb.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-None-08318");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params_quality_level_1, nullptr);
}

TEST_F(NegativeVideoEncode, RateControlRequiresEncodeSession) {
    TEST_DESCRIPTION("vkCmdControlVideoCodingKHR - rate control requires an encode session");

    RETURN_IF_SKIP(Init());

    VideoConfig encode_config = GetConfigEncode();
    VideoConfig decode_config = GetConfigDecode();
    if (!encode_config || !decode_config) {
        GTEST_SKIP() << "Test requires video decode and encode support";
    }

    VideoContext context(m_device, decode_config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto control_info = vku::InitStruct<VkVideoCodingControlInfoKHR>();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    control_info.flags = VK_VIDEO_CODING_CONTROL_ENCODE_RATE_CONTROL_BIT_KHR;
    m_errorMonitor->SetDesiredError("VUID-vkCmdControlVideoCodingKHR-pCodingControlInfo-08243");
    cb.ControlVideoCoding(control_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, RateControlUnsupportedMode) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - rate control mode is not supported");

    RETURN_IF_SKIP(Init());

    // Try to find a config that does not support all rate control modes
    VkVideoEncodeRateControlModeFlagsKHR all_rc_modes = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR |
                                                        VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR |
                                                        VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR;
    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [all_rc_modes](const VideoConfig& config) {
        return (config.EncodeCaps()->rateControlModes & all_rc_modes) < all_rc_modes;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires an encode profile that does not support all rate control modes";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto unsupported_rc_modes = (config.EncodeCaps()->rateControlModes & all_rc_modes) ^ all_rc_modes;

    auto rc_info = VideoEncodeRateControlInfo(config);
    rc_info->rateControlMode =
        static_cast<VkVideoEncodeRateControlModeFlagBitsKHR>(1 << static_cast<uint32_t>(log2(unsupported_rc_modes)));
    if (rc_info->rateControlMode != VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR) {
        rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    }

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08244");
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08244");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, RateControlTooManyLayers) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - rate control layer count exceeds maximum supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config).SetAnyMode();
    for (uint32_t i = 0; i <= config.EncodeCaps()->maxRateControlLayers; ++i) {
        rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    }

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-layerCount-08245");
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-layerCount-08245");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, RateControlNoLayers) {
    TEST_DESCRIPTION(
        "vkCmdBegin/ControlVideoCodingKHR - no rate control layers are allowed when "
        "rate control mode is DEFAULT or DISABLED");

    RETURN_IF_SKIP(Init());

    VideoConfig config =
        GetConfig(GetConfigsWithRateControl(GetConfigsEncode(), VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR));
    if (!config) {
        // If DISABLED is not supported, just use any encode config available
        config = GetConfigEncode();
    }
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config);
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));

    std::vector<VkVideoEncodeRateControlModeFlagBitsKHR> rate_control_modes = {VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DEFAULT_KHR,
                                                                               VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR};

    for (auto rate_control_mode : rate_control_modes) {
        if (config.SupportsRateControlMode(rate_control_mode)) {
            rc_info->rateControlMode = rate_control_mode;

            cb.Begin();

            m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08248");
            cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
            m_errorMonitor->VerifyFound();

            cb.BeginVideoCoding(context.Begin());

            m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08248");
            cb.ControlVideoCoding(context.Control().RateControl(rc_info));
            m_errorMonitor->VerifyFound();

            cb.EndVideoCoding(context.End());
            cb.End();
        }
    }
}

TEST_F(NegativeVideoEncode, RateControlMissingLayers) {
    TEST_DESCRIPTION(
        "vkCmdBegin/ControlVideoCodingKHR - rate control layers are required when "
        "rate control mode is CBR or VBR");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config);

    std::vector<VkVideoEncodeRateControlModeFlagBitsKHR> rate_control_modes = {VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR,
                                                                               VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR};

    for (auto rate_control_mode : rate_control_modes) {
        if (config.SupportsRateControlMode(rate_control_mode)) {
            rc_info->rateControlMode = rate_control_mode;

            cb.Begin();

            m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08275");
            cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
            m_errorMonitor->VerifyFound();

            cb.BeginVideoCoding(context.Begin());

            m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08275");
            cb.ControlVideoCoding(context.Control().RateControl(rc_info));
            m_errorMonitor->VerifyFound();

            cb.EndVideoCoding(context.End());
            cb.End();
        }
    }
}

TEST_F(NegativeVideoEncode, RateControlLayerBitrate) {
    TEST_DESCRIPTION("vkCmdControlVideoCodingKHR - test incorrect values for averageBitrate and maxBitrate");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    auto rc_info = VideoEncodeRateControlInfo(config).SetAnyMode();
    for (uint32_t i = 0; i < config.EncodeCaps()->maxRateControlLayers; ++i) {
        rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    }

    const uint32_t layer_index = config.EncodeCaps()->maxRateControlLayers / 2;

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    uint64_t orig_bitrate = rc_info.Layer(layer_index)->averageBitrate;

    // averageBitrate must be greater than 0
    rc_info.Layer(layer_index)->averageBitrate = 0;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-pLayers-08276");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    // averageBitrate must be less than or equal to maxBitrate cap
    rc_info.Layer(layer_index)->averageBitrate = config.EncodeCaps()->maxBitrate + 1;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08278");
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-pLayers-08276");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    rc_info.Layer(layer_index)->averageBitrate = orig_bitrate;
    orig_bitrate = rc_info.Layer(layer_index)->maxBitrate;

    // maxBitrate must be greater than 0
    rc_info.Layer(layer_index)->maxBitrate = 0;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08278");
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-pLayers-08277");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    // maxBitrate must be less than or equal to maxBitrate cap
    rc_info.Layer(layer_index)->maxBitrate = config.EncodeCaps()->maxBitrate + 1;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-pLayers-08277");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    rc_info.Layer(layer_index)->maxBitrate = orig_bitrate;

    if (config.EncodeCaps()->rateControlModes & VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR) {
        rc_info->rateControlMode = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR;

        // averageBitrate must be less than or equal to maxBitrate
        rc_info.Layer(layer_index)->maxBitrate = rc_info.Layer(layer_index)->averageBitrate - 1;
        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08278");
        cb.ControlVideoCoding(context.Control().RateControl(rc_info));
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, RateControlLayerBitrateCBR) {
    TEST_DESCRIPTION("vkCmdControlVideoCodingKHR - test incorrect values for averageBitrate and maxBitrate with CBR");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncode(), VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with CBR rate control mode";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    auto rc_info = VideoEncodeRateControlInfo(config);
    rc_info->rateControlMode = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR;
    for (uint32_t i = 0; i < config.EncodeCaps()->maxRateControlLayers; ++i) {
        rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    }

    const uint32_t layer_index = config.EncodeCaps()->maxRateControlLayers / 2;

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    // averageBitrate must equal maxBitrate for CBR
    rc_info.Layer(layer_index)->maxBitrate = rc_info.Layer(layer_index)->averageBitrate - 1;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08356");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    rc_info.Layer(layer_index)->maxBitrate = rc_info.Layer(layer_index)->averageBitrate;
    rc_info.Layer(layer_index)->averageBitrate -= 1;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08356");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, RateControlLayerBitrateVBR) {
    TEST_DESCRIPTION("vkCmdControlVideoCodingKHR - test incorrect values for averageBitrate and maxBitrate with VBR");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncode(), VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with VBR rate control mode";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    auto rc_info = VideoEncodeRateControlInfo(config);
    rc_info->rateControlMode = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR;
    for (uint32_t i = 0; i < config.EncodeCaps()->maxRateControlLayers; ++i) {
        rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    }

    const uint32_t layer_index = config.EncodeCaps()->maxRateControlLayers / 2;

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    // averageBitrate must be less than or equal to maxBitrate
    rc_info.Layer(layer_index)->maxBitrate = rc_info.Layer(layer_index)->averageBitrate - 1;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08278");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, RateControlLayerFrameRate) {
    TEST_DESCRIPTION("vkCmdControlVideoCodingKHR - test incorrect values for frameRateNumerator and frameRateDenominator");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    auto rc_info = VideoEncodeRateControlInfo(config).SetAnyMode();
    for (uint32_t i = 0; i < config.EncodeCaps()->maxRateControlLayers; ++i) {
        rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    }

    const uint32_t layer_index = config.EncodeCaps()->maxRateControlLayers / 2;

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    // frameRateNumerator must be greater than 0
    rc_info.Layer(layer_index)->frameRateNumerator = 0;
    rc_info.Layer(layer_index)->frameRateDenominator = 1;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlLayerInfoKHR-frameRateNumerator-08350");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    // frameRateDenominator must be greater than 0
    rc_info.Layer(layer_index)->frameRateNumerator = 30;
    rc_info.Layer(layer_index)->frameRateDenominator = 0;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlLayerInfoKHR-frameRateDenominator-08351");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, RateControlVirtualBufferSize) {
    TEST_DESCRIPTION(
        "vkCmdControlVideoCodingKHR - test incorrect values for virtualBufferSizeInMs and initialVirtualBufferSizeInMs");

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

    // virtualBufferSizeInMs must be greater than 0
    rc_info->virtualBufferSizeInMs = 0;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-layerCount-08357");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    // initialVirtualBufferSizeInMs must be less than or equal to virtualBufferSizeInMs
    rc_info->virtualBufferSizeInMs = 1000;
    rc_info->initialVirtualBufferSizeInMs = 1001;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-layerCount-08358");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, RateControlMissingChain) {
    TEST_DESCRIPTION("vkCmdControlVideoCodingKHR - missing rate control info");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto control_info = vku::InitStruct<VkVideoCodingControlInfoKHR>();
    control_info.flags = VK_VIDEO_CODING_CONTROL_ENCODE_RATE_CONTROL_BIT_KHR;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->SetDesiredError("VUID-VkVideoCodingControlInfoKHR-flags-07018");
    cb.ControlVideoCoding(control_info);
    m_errorMonitor->VerifyFound();
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, RateControlStateMismatchNotDefault) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - rate control state not specified but rate control mode is not DEFAULT");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset().RateControl(rc_info));
    cb.EndVideoCoding(context.End());
    cb.End();

    context.Queue().Submit(cb);
    m_device->Wait();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.EndVideoCoding(context.End());
    cb.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-pBeginInfo-08253");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeVideoEncode, RateControlStateMismatch) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - rate control state specified does not match current configuration");

    RETURN_IF_SKIP(Init());

    // Try to find a config that supports both CBR and VBR
    VkVideoEncodeRateControlModeFlagsKHR rc_modes =
        VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR | VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR;
    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [rc_modes](const VideoConfig& config) {
        return (config.EncodeCaps()->rateControlModes & rc_modes) == rc_modes;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires an encode profile that supports both CBR and VBR rate control modes";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config);

    rc_info->rateControlMode = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR;
    rc_info->virtualBufferSizeInMs = 3000;
    rc_info->initialVirtualBufferSizeInMs = 1000;

    for (uint32_t i = 0; i < config.EncodeCaps()->maxRateControlLayers; ++i) {
        auto rc_layer = VideoEncodeRateControlLayerInfo(config);

        rc_layer->averageBitrate = 32000 / (i + 1);
        rc_layer->maxBitrate = 32000 / (i + 1);
        rc_layer->frameRateNumerator = 30;
        rc_layer->frameRateDenominator = i + 1;

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

    // rateControlMode mismatch
    VkVideoEncodeRateControlModeFlagBitsKHR rate_control_mode = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR;
    std::swap(rc_info->rateControlMode, rate_control_mode);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info->rateControlMode, rate_control_mode);

    // layerCount mismatch
    if (rc_info->layerCount > 1) {
        uint32_t layer_count = rc_info->layerCount - 1;
        std::swap(rc_info->layerCount, layer_count);
        rc_test_utils.TestRateControlStateMismatch(rc_info);
        std::swap(rc_info->layerCount, layer_count);
    }

    // virtualBufferSizeInMs mismatch
    uint32_t vb_size = 2500;
    std::swap(rc_info->virtualBufferSizeInMs, vb_size);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info->virtualBufferSizeInMs, vb_size);

    // initialVirtualBufferSizeInMs mismatch
    uint32_t init_vb_size = 500;
    std::swap(rc_info->initialVirtualBufferSizeInMs, init_vb_size);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info->initialVirtualBufferSizeInMs, init_vb_size);

    // Layer averageBitrate/maxBitrate mismatch
    uint64_t max_bitrate = 23456;
    uint64_t avg_bitrate = 7891;
    std::swap(rc_info.Layer(0)->averageBitrate, avg_bitrate);
    std::swap(rc_info.Layer(0)->maxBitrate, max_bitrate);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.Layer(0)->averageBitrate, avg_bitrate);
    std::swap(rc_info.Layer(0)->maxBitrate, max_bitrate);

    // Layer frameRateNumerator/Denominator mismatch
    uint32_t fr_num = 30000;
    uint32_t fr_den = 1001;
    std::swap(rc_info.Layer(0)->frameRateNumerator, fr_num);
    std::swap(rc_info.Layer(0)->frameRateDenominator, fr_den);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.Layer(0)->frameRateNumerator, fr_num);
    std::swap(rc_info.Layer(0)->frameRateDenominator, fr_den);
}

TEST_F(NegativeVideoEncode, EncodeSessionNotEncode) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - bound video session was not created with encode operation");

    RETURN_IF_SKIP(Init());

    VideoConfig decode_config = GetConfigDecode();
    VideoConfig encode_config = GetConfigEncode();
    if (!decode_config || !encode_config) {
        GTEST_SKIP() << "Test requires both video decode and video encode support";
    }

    VideoContext decode_context(m_device, decode_config);
    decode_context.CreateAndBindSessionMemory();
    decode_context.CreateResources();

    VideoContext encode_context(m_device, encode_config);
    encode_context.CreateAndBindSessionMemory();
    encode_context.CreateResources();

    vkt::CommandBuffer& cb = decode_context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(decode_context.Begin());

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-commandBuffer-cmdpool");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-None-08250");
    cb.EncodeVideo(encode_context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(decode_context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeSessionUninitialized) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - session uninitialized");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.EncodeVideo(context.EncodeFrame());
    cb.EndVideoCoding(context.End());
    cb.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-None-07012");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeVideoEncode, EncodeProtectedNoFaultBitstreamBuffer) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - protectedNoFault tests for bitstream buffer");

    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(Init());
    if (IsProtectedNoFaultSupported()) {
        GTEST_SKIP() << "Test requires protectedMemory support without protectedNoFault support";
    }

    const bool use_protected = true;

    VideoConfig config = GetConfigWithProtectedContent(GetConfigsEncode());
    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile with protected content support";
    }

    VideoContext unprotected_context(m_device, config);
    unprotected_context.CreateAndBindSessionMemory();
    unprotected_context.CreateResources(use_protected /* bitstream */, false /* DPB */, false /* src image */);

    vkt::CommandBuffer& unprotected_cb = unprotected_context.CmdBuffer();

    VideoContext protected_context(m_device, config, use_protected);
    protected_context.CreateAndBindSessionMemory();
    protected_context.CreateResources(false /* bitstream */, use_protected /* DPB */, use_protected /* src image */);

    vkt::CommandBuffer& protected_cb = protected_context.CmdBuffer();

    unprotected_cb.Begin();
    unprotected_cb.BeginVideoCoding(unprotected_context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-commandBuffer-08202");
    unprotected_cb.EncodeVideo(unprotected_context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    unprotected_cb.EndVideoCoding(unprotected_context.End());
    unprotected_cb.End();

    protected_cb.Begin();
    protected_cb.BeginVideoCoding(protected_context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-commandBuffer-08203");
    protected_cb.EncodeVideo(protected_context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    protected_cb.EndVideoCoding(protected_context.End());
    protected_cb.End();
}

TEST_F(NegativeVideoEncode, EncodeProtectedNoFaultEncodeInput) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - protectedNoFault tests for encode input");

    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(Init());
    if (IsProtectedNoFaultSupported()) {
        GTEST_SKIP() << "Test requires protectedMemory support without protectedNoFault support";
    }

    const bool use_protected = true;

    VideoConfig config = GetConfigWithProtectedContent(GetConfigsEncode());
    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile with protected content support";
    }

    VideoContext unprotected_context(m_device, config);
    unprotected_context.CreateAndBindSessionMemory();
    unprotected_context.CreateResources(false /* bitstream */, false /* DPB */, use_protected /* src image */);

    vkt::CommandBuffer& unprotected_cb = unprotected_context.CmdBuffer();

    VideoContext protected_context(m_device, config, use_protected);
    protected_context.CreateAndBindSessionMemory();
    protected_context.CreateResources(use_protected /* bitstream */, use_protected /* DPB */, false /* src image */);

    vkt::CommandBuffer& protected_cb = protected_context.CmdBuffer();

    unprotected_cb.Begin();
    unprotected_cb.BeginVideoCoding(unprotected_context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-commandBuffer-08211");
    unprotected_cb.EncodeVideo(unprotected_context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    unprotected_cb.EndVideoCoding(unprotected_context.End());
    unprotected_cb.End();

    protected_cb.Begin();
    protected_cb.BeginVideoCoding(protected_context.Begin());

    protected_cb.EncodeVideo(protected_context.EncodeFrame());

    protected_cb.EndVideoCoding(protected_context.End());
    protected_cb.End();
}

TEST_F(NegativeVideoEncode, EncodeImageLayouts) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - pictures should be in the expected layout");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile with reference picture support and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(-1, 1));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.EncodeInput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR));

    cb.EncodeVideo(context.EncodeFrame(0));

    // Encode input must be in ENCODE_SRC layout
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08222");
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.EncodeInput()->LayoutTransition(VK_IMAGE_LAYOUT_GENERAL));
    cb.EncodeVideo(context.EncodeFrame(0));
    m_errorMonitor->VerifyFound();
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.EncodeInput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR));

    // Reconstructed must be in ENCODE_DPB layout
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08223");
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_GENERAL, 0, 1));
    cb.EncodeVideo(context.EncodeFrame(0));
    m_errorMonitor->VerifyFound();
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR, 0, 1));

    // Reference must be in ENCODE_DPB layout
    cb.EncodeVideo(context.EncodeReferenceFrame(0));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_GENERAL, 0, 1));
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pPictureResource-08224");
    cb.EncodeVideo(context.EncodeFrame(1).AddReferenceFrame(0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeInvalidResourceLayer) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - out-of-bounds layer index in VkVideoPictureResourceInfoKHR");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoPictureResourceInfoKHR dpb_res = context.Dpb()->Picture(0);
    dpb_res.baseArrayLayer = 5;

    VkVideoPictureResourceInfoKHR src_res = context.EncodeInput()->Picture();
    src_res.baseArrayLayer = 5;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    // Invalid baseArrayLayer in VkVideoEncodeInfoKHR::srcPictureResource
    m_errorMonitor->SetDesiredError("VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175");
    cb.EncodeVideo(context.EncodeFrame(0).SetEncodeInput(src_res));
    m_errorMonitor->VerifyFound();

    // Invalid baseArrayLayer in VkVideoEncodeInfoKHR::pSetupReferenceSlot
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08215");
    m_errorMonitor->SetDesiredError("VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175");
    cb.EncodeVideo(context.EncodeFrame(0, &dpb_res));
    m_errorMonitor->VerifyFound();

    // Invalid baseArrayLayer in VkVideoEncodeInfoKHR::pReferenceSlots
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pPictureResource-08219");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-slotIndex-08217");
    m_errorMonitor->SetDesiredError("VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175");
    cb.EncodeVideo(context.EncodeFrame(1).AddReferenceFrame(0, &dpb_res));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeQueryTooManyOperations) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - no more queries available to store operation results");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video encode queue to support result status queries";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();
    context.CreateStatusQueryPool(2);
    context.CreateEncodeFeedbackQueryPool(2);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 0, 0);
    cb.EncodeVideo(context.EncodeFrame());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-opCount-07174");
    cb.EncodeVideo(context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    vk::CmdEndQuery(cb.handle(), context.StatusQueryPool(), 0);
    cb.EndVideoCoding(context.End());
    cb.End();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    vk::CmdBeginQuery(cb.handle(), context.EncodeFeedbackQueryPool(), 0, 0);
    cb.EncodeVideo(context.EncodeFrame());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-opCount-07174");
    cb.EncodeVideo(context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    vk::CmdEndQuery(cb.handle(), context.EncodeFeedbackQueryPool(), 0);
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeIncompatBufferProfile) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - bitstream buffer must be compatible with the video profile");

    RETURN_IF_SKIP(Init());

    auto configs = GetConfigsEncode();
    if (configs.size() < 2) {
        GTEST_SKIP() << "Test requires two video encode profiles";
    }

    VideoContext context1(m_device, configs[0]);
    VideoContext context2(m_device, configs[1]);
    context1.CreateAndBindSessionMemory();
    context1.CreateResources();
    context2.CreateAndBindSessionMemory();
    context2.CreateResources();

    vkt::CommandBuffer& cb = context1.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08201");
    cb.EncodeVideo(context1.EncodeFrame().SetBitstream(context2.Bitstream()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeBufferMissingEncodeDstUsage) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - bitstream buffer missing ENCODE_DST usage");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    auto profile_list = vku::InitStruct<VkVideoProfileListInfoKHR>();
    profile_list.profileCount = 1;
    profile_list.pProfiles = config.Profile();

    auto create_info = vku::InitStruct<VkBufferCreateInfo>(&profile_list);
    create_info.flags = 0;
    create_info.size = std::max((VkDeviceSize)4096, config.Caps()->minBitstreamBufferSizeAlignment);
    create_info.usage = VK_BUFFER_USAGE_VIDEO_ENCODE_SRC_BIT_KHR;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkt::Buffer buffer(*m_device, create_info);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeInfoKHR-dstBuffer-08236");
    cb.EncodeVideo(context.EncodeFrame().SetBitstreamBuffer(buffer.handle(), 0, create_info.size));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeBufferOffsetOutOfBounds) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - bitstream buffer offset out of bounds");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();
    encode_info->dstBufferOffset = encode_info->dstBufferRange;
    encode_info->dstBufferOffset += config.Caps()->minBitstreamBufferOffsetAlignment - 1;
    encode_info->dstBufferOffset &= ~(config.Caps()->minBitstreamBufferOffsetAlignment - 1);

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeInfoKHR-dstBufferOffset-08237");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeInfoKHR-dstBufferRange-08238");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeBufferOffsetAlignment) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - bitstream buffer offset needs to be aligned");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(FilterConfigs(
        GetConfigsEncode(), [](const VideoConfig& config) { return config.Caps()->minBitstreamBufferOffsetAlignment > 1; }));
    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile with minBitstreamBufferOffsetAlignment > 1";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();
    ++encode_info->dstBufferOffset;
    encode_info->dstBufferRange -= config.Caps()->minBitstreamBufferSizeAlignment;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08204");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeBufferRangeOutOfBounds) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - bitstream buffer range out of bounds");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeInfoKHR-dstBufferRange-08238");
    encode_info->dstBufferOffset += config.Caps()->minBitstreamBufferOffsetAlignment;
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeInfoKHR-dstBufferRange-08238");
    encode_info->dstBufferOffset = 0;
    encode_info->dstBufferRange += config.Caps()->minBitstreamBufferSizeAlignment;
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeBufferRangeAlignment) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - bitstream buffer range needs to be aligned");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(FilterConfigs(
        GetConfigsEncode(), [](const VideoConfig& config) { return config.Caps()->minBitstreamBufferSizeAlignment > 1; }));
    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile with minBitstreamBufferSizeAlignment > 1";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();
    --encode_info->dstBufferRange;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08205");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeInvalidSetupSlotIndex) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - invalid slot index specified for reconstructed picture");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsEncode(), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with 3 reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    VideoEncodeInfo encode_info = context.EncodeFrame(0);
    auto setup_slot = *encode_info->pSetupReferenceSlot;
    encode_info->pSetupReferenceSlot = &setup_slot;

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeInfoKHR-pSetupReferenceSlot-08239");
    setup_slot.slotIndex = -1;
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08213");
    setup_slot.slotIndex = 2;
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeInvalidRefSlotIndex) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - invalid slot index specified for reference picture");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsEncode(), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with 3 reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeInfoKHR-slotIndex-08241");
    cb.EncodeVideo(context.EncodeFrame(0).AddReferenceFrame(-1, 0));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pPictureResource-08219");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-slotIndex-08217");
    cb.EncodeVideo(context.EncodeFrame(0).AddReferenceFrame(2, 0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeSetupNull) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - reconstructed picture is required for sessions with DPB slots");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    VideoEncodeInfo encode_info = context.EncodeFrame(0);
    encode_info->pSetupReferenceSlot = NULL;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08377");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeSetupResourceNull) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - reconstructed picture resource must not be NULL");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    VideoEncodeInfo encode_info = context.EncodeFrame(0);
    auto setup_slot = *encode_info->pSetupReferenceSlot;
    encode_info->pSetupReferenceSlot = &setup_slot;

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeInfoKHR-pSetupReferenceSlot-08240");
    setup_slot.pPictureResource = nullptr;
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeReferenceResourceNull) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - reference picture resource must not be NULL");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with reference pictures with 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeInfoKHR-pPictureResource-08242");
    cb.EncodeVideo(context.EncodeFrame(1).AddReferenceFrame(0, nullptr));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeIncompatInputPicProfile) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - encode input picture must be compatible with the video profile");

    RETURN_IF_SKIP(Init());

    VideoConfig configs[2] = {};
    const auto& all_configs = GetConfigsEncode();
    for (uint32_t i = 0; i < all_configs.size(); ++i) {
        for (uint32_t j = i + 1; j < all_configs.size(); ++j) {
            const auto& coded_extent1 = all_configs[i].SessionCreateInfo()->maxCodedExtent;
            const auto& coded_extent2 = all_configs[j].SessionCreateInfo()->maxCodedExtent;
            const auto& pic_format1 = *all_configs[i].DpbFormatProps();
            const auto& pic_format2 = *all_configs[j].DpbFormatProps();
            if ((coded_extent1.width == coded_extent2.width) && (coded_extent1.height == coded_extent2.height) &&
                (pic_format1.imageType == pic_format2.imageType) && (pic_format1.imageTiling == pic_format2.imageTiling) &&
                (pic_format1.format == pic_format2.format) && (pic_format1.imageUsageFlags == pic_format2.imageUsageFlags)) {
                configs[0] = all_configs[i];
                configs[1] = all_configs[j];
            }
        }
    }
    if (!configs[0]) {
        GTEST_SKIP() << "Test requires two video profiles with matching encode input format/size";
    }

    VideoContext context1(m_device, configs[0]);
    VideoContext context2(m_device, configs[1]);
    context1.CreateAndBindSessionMemory();
    context1.CreateResources();
    context2.CreateAndBindSessionMemory();
    context2.CreateResources();

    vkt::CommandBuffer& cb = context1.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08206");
    cb.EncodeVideo(context1.EncodeFrame().SetEncodeInput(context2.EncodeInput()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeInputFormatMismatch) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - encode input picture format mismatch");

    RETURN_IF_SKIP(Init());

    uint32_t alt_pic_format_index = 0;
    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [&alt_pic_format_index](const VideoConfig& config) {
        const auto& format_props = config.SupportedPictureFormatProps();
        for (size_t i = 0; i < format_props.size(); ++i) {
            if (format_props[i].format != format_props[0].format && alt_pic_format_index == 0) {
                alt_pic_format_index = i;
                return true;
            }
        }
        return false;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile with support for two input picture formats";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoConfig config2 = config;
    config2.SetFormatProps({config.SupportedPictureFormatProps()[alt_pic_format_index]}, config.SupportedDpbFormatProps());
    VideoEncodeInput output(m_device, config2);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08207");
    cb.EncodeVideo(context.EncodeFrame().SetEncodeInput(output.Picture()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeInputMissingEncodeSrcUsage) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - encode input picture resource missing VIDEO_ENCODE_SRC usage");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    if (config.PictureFormatProps()->imageUsageFlags == VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR) {
        GTEST_SKIP() << "Test requires input format to support at least one more usage besides ENCODE_SRC";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto view_usage_ci = vku::InitStruct<VkImageViewUsageCreateInfo>();
    view_usage_ci.usage = config.PictureFormatProps()->imageUsageFlags ^ VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR;

    auto image_view_ci = vku::InitStruct<VkImageViewCreateInfo>(&view_usage_ci);
    image_view_ci.image = context.EncodeInput()->Image();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    image_view_ci.format = config.PictureFormatProps()->format;
    image_view_ci.components = config.PictureFormatProps()->componentMapping;
    image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    vkt::ImageView image_view(*m_device, image_view_ci);

    VkVideoPictureResourceInfoKHR src_res = context.EncodeInput()->Picture();
    src_res.imageViewBinding = image_view.handle();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08210");
    cb.EncodeVideo(context.EncodeFrame().SetEncodeInput(src_res));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeInputCodedOffsetExtent) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - invalid encode input picture coded offset/extent");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VkVideoPictureResourceInfoKHR src_res = context.EncodeInput()->Picture();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08208");
    ++src_res.codedOffset.x;
    cb.EncodeVideo(context.EncodeFrame().SetEncodeInput(src_res));
    --src_res.codedOffset.x;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08208");
    ++src_res.codedOffset.y;
    cb.EncodeVideo(context.EncodeFrame().SetEncodeInput(src_res));
    --src_res.codedOffset.y;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08209");
    ++src_res.codedExtent.width;
    cb.EncodeVideo(context.EncodeFrame().SetEncodeInput(src_res));
    --src_res.codedExtent.width;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08209");
    ++src_res.codedExtent.height;
    cb.EncodeVideo(context.EncodeFrame().SetEncodeInput(src_res));
    --src_res.codedExtent.height;
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeSetupAndRefCodedOffset) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - invalid reconstructed/reference picture coded offset");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with reference pictures with 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    VkVideoPictureResourceInfoKHR res = context.Dpb()->Picture(0);

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08215");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08214");
    ++res.codedOffset.x;
    cb.EncodeVideo(context.EncodeFrame(0, &res));
    --res.codedOffset.x;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08215");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08214");
    ++res.codedOffset.y;
    cb.EncodeVideo(context.EncodeFrame(0, &res));
    --res.codedOffset.y;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pPictureResource-08219");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-codedOffset-08218");
    ++res.codedOffset.x;
    cb.EncodeVideo(context.EncodeFrame(1).AddReferenceFrame(0, &res));
    --res.codedOffset.x;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pPictureResource-08219");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-codedOffset-08218");
    ++res.codedOffset.y;
    cb.EncodeVideo(context.EncodeFrame(1).AddReferenceFrame(0, &res));
    --res.codedOffset.y;
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeSetupResourceNotBound) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - used reconstructed picture resource is not bound");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08215");
    cb.EncodeVideo(context.EncodeFrame(0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeRefResourceNotBoundToDpbSlot) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - used reference picture resource is not bound as a resource "
        "currently associated with the corresponding DPB slot");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncode()), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with reference pictures and 3 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = config.Caps()->maxDpbSlots;
    config.SessionCreateInfo()->maxActiveReferencePictures = config.Caps()->maxActiveReferencePictures;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 2));

    // Trying to refer to reference picture resource that is not bound at all
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pPictureResource-08219");
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(1));
    m_errorMonitor->VerifyFound();

    // Trying to refer to bound reference picture resource, but with incorrect slot index
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pPictureResource-08219");
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(1, 0));
    m_errorMonitor->VerifyFound();

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.EncodeInput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR, 0, 1));
    cb.EncodeVideo(context.EncodeReferenceFrame(1, 0));
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(1, 0));

    // Trying to refer to bound reference picture resource, but with incorrect slot index after
    // the associated DPB slot index has been updated within the video coding scope
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pPictureResource-08219");
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(0, 0));
    m_errorMonitor->VerifyFound();

    cb.ControlVideoCoding(context.Control().Reset());

    // Trying to refer to bound reference picture resource after reset
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pPictureResource-08219");
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(1, 0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeTooManyReferences) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - reference picture count exceeds maxActiveReferencePictures");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncode()), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with reference pictures and 3 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 3;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(-1, 2));

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-activeReferencePictureCount-08216");
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(0).AddReferenceFrame(1));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeDuplicateRefResource) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - same reference picture resource is used twice");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncode(), 2), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with 2 reference pictures and 3 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 3;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoPictureResourceInfoKHR res = context.Dpb()->Picture(0);

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 2));

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pPictureResource-08219");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pPictureResource-08220");
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(0, &res).AddReferenceFrame(1, &res));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-dpbFrameUseCount-08221");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pPictureResource-08220");
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(0, &res).AddReferenceFrame(0, &res));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeDuplicateFrame) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - same DPB frame reference is used twice");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncode(), 2), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with 2 reference pictures and 3 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 3;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(0, 1).AddResource(-1, 2));

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-dpbFrameUseCount-08221");
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(0, 0).AddReferenceFrame(0, 1));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, ImplicitDeactivation) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - test DPB slot deactivation caused by reference invalidation");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with reference picture support and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    // Setup frame in DPB slot then use it as non-setup reconstructed picture
    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
    cb.ControlVideoCoding(context.Control().Reset());
    cb.EncodeVideo(context.EncodeReferenceFrame(0));
    cb.EncodeVideo(context.EncodeFrame(0));
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

TEST_F(NegativeVideoEncode, EncodeInlineQueryOpCount) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - inline query count does not match video codec operation count");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();
    context.CreateEncodeFeedbackQueryPool(2);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08360");
    cb.EncodeVideo(context.EncodeFrame().InlineQuery(context.EncodeFeedbackQueryPool(), 0, 2));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeInlineQueryOutOfBounds) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - inline query firstQuery/queryCount out of bounds");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();
    context.CreateEncodeFeedbackQueryPool(4);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoInlineQueryInfoKHR-queryPool-08372");
    m_errorMonitor->SetDesiredError("VUID-VkVideoInlineQueryInfoKHR-queryPool-08373");
    cb.EncodeVideo(context.EncodeFrame().InlineQuery(context.EncodeFeedbackQueryPool(), 4, 1));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pNext-08360");
    m_errorMonitor->SetDesiredError("VUID-VkVideoInlineQueryInfoKHR-queryPool-08373");
    cb.EncodeVideo(context.EncodeFrame().InlineQuery(context.EncodeFeedbackQueryPool(), 2, 3));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeInlineQueryUnavailable) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - inline queries must be unavailable");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();
    context.CreateEncodeFeedbackQueryPool();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    // Use custom begin info because the default uses VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();

    cb.Begin(&begin_info);
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset());
    cb.EncodeVideo(context.EncodeFrame().InlineQuery(context.EncodeFeedbackQueryPool()));
    cb.EndVideoCoding(context.End());
    cb.End();

    m_command_buffer.Begin(&begin_info);
    vk::CmdResetQueryPool(m_command_buffer.handle(), context.EncodeFeedbackQueryPool(), 0, 1);
    m_command_buffer.End();

    // Will fail as query pool has never been reset before
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08361");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    // Will succeed this time as we reset the query
    context.Queue().Submit(cb);
    m_device->Wait();

    // Will fail again as we did not reset after use
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08361");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    // Will succeed again after reset
    context.Queue().Submit(cb);
    m_device->Wait();
}

TEST_F(NegativeVideoEncode, EncodeInlineQueryType) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - inline query type is invalid");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    auto create_info = vku::InitStruct<VkQueryPoolCreateInfo>();
    create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, create_info);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-queryPool-08363");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-queryType-08362");
    cb.EncodeVideo(context.EncodeFrame().InlineQuery(query_pool.handle()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeInlineQueryProfileMismatch) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - inline query must have been created with the same profile");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    auto configs = GetConfigsEncode();
    if (configs.size() < 2) {
        GTEST_SKIP() << "Test requires support for at least two video encode profiles";
    }

    configs[0].SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;

    VideoContext context1(m_device, configs[0]);
    VideoContext context2(m_device, configs[1]);
    context1.CreateAndBindSessionMemory();
    context1.CreateResources();
    context2.CreateEncodeFeedbackQueryPool();

    vkt::CommandBuffer& cb = context1.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-queryPool-08363");
    cb.EncodeVideo(context1.EncodeFrame().InlineQuery(context2.EncodeFeedbackQueryPool()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, EncodeInlineQueryIncompatibleQueueFamily) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - result status queries require queue family support");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    uint32_t queue_family_index = VK_QUEUE_FAMILY_IGNORED;
    for (uint32_t qfi = 0; qfi < QueueFamilyCount(); ++qfi) {
        if (!QueueFamilySupportsResultStatusOnlyQueries(qfi)) {
            queue_family_index = qfi;
            break;
        }
    }

    if (queue_family_index == VK_QUEUE_FAMILY_IGNORED) {
        GTEST_SKIP() << "Test requires a queue family with no support for result status queries";
    }

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();
    context.CreateStatusQueryPool();

    vkt::CommandPool cmd_pool(*m_device, queue_family_index, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer cb(*m_device, cmd_pool);

    cb.Begin();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-cmdpool");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07231");
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-commandBuffer-cmdpool");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-queryType-08364");
    cb.EncodeVideo(context.EncodeFrame().InlineQuery(context.StatusQueryPool()));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEndVideoCodingKHR-commandBuffer-cmdpool");
    cb.EndVideoCoding(context.End());

    cb.End();
}

TEST_F(NegativeVideoEncode, CreateQueryPoolMissingEncodeFeedbackInfo) {
    TEST_DESCRIPTION("vkCreateQueryPool - missing VkQueryPoolVideoEncodeFeedbackCreateInfoKHR");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    auto create_info = vku::InitStruct<VkQueryPoolCreateInfo>(config.Profile());
    create_info.queryType = VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR;
    create_info.queryCount = 1;

    VkQueryPool query_pool;
    m_errorMonitor->SetDesiredError("VUID-VkQueryPoolCreateInfo-queryType-07906");
    vk::CreateQueryPool(device(), &create_info, nullptr, &query_pool);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncode, CreateQueryPoolEncodeFeedbackProfile) {
    TEST_DESCRIPTION("vkCreateQueryPool - require encode profile for ENCODE_FEEDBACK query");

    RETURN_IF_SKIP(Init());

    VideoConfig decode_config = GetConfigDecode();
    VideoConfig encode_config = GetConfigEncode();
    if (!decode_config || !encode_config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    auto encode_feedback_info = vku::InitStruct<VkQueryPoolVideoEncodeFeedbackCreateInfoKHR>(decode_config.Profile());
    encode_feedback_info.encodeFeedbackFlags = VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BYTES_WRITTEN_BIT_KHR;

    auto create_info = vku::InitStruct<VkQueryPoolCreateInfo>(&encode_feedback_info);
    create_info.queryType = VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR;
    create_info.queryCount = 1;

    VkQueryPool query_pool;
    m_errorMonitor->SetDesiredError("VUID-VkQueryPoolCreateInfo-queryType-07133");
    vk::CreateQueryPool(device(), &create_info, nullptr, &query_pool);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncode, CreateQueryPoolUnsupportedEncodeFeedback) {
    TEST_DESCRIPTION("vkCreateQueryPool - missing VkQueryPoolVideoEncodeFeedbackCreateInfoKHR");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return config.EncodeCaps()->supportedEncodeFeedbackFlags != AllVkVideoEncodeFeedbackFlagBitsKHR;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile that does not support all encode feedback flags";
    }

    auto encode_feedback_info = vku::InitStruct<VkQueryPoolVideoEncodeFeedbackCreateInfoKHR>(config.Profile());
    encode_feedback_info.encodeFeedbackFlags = AllVkVideoEncodeFeedbackFlagBitsKHR;

    auto create_info = vku::InitStruct<VkQueryPoolCreateInfo>(&encode_feedback_info);
    create_info.queryType = VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR;
    create_info.queryCount = 1;

    VkQueryPool query_pool;
    m_errorMonitor->SetDesiredError("VUID-VkQueryPoolCreateInfo-queryType-07907");
    vk::CreateQueryPool(device(), &create_info, nullptr, &query_pool);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncode, BeginQueryEncodeFeedbackProfileMismatch) {
    TEST_DESCRIPTION("vkCmdBeginQuery - encode feedback query must have been created with the same profile");

    RETURN_IF_SKIP(Init());

    auto configs = GetConfigsEncode();
    if (configs.size() < 2) {
        GTEST_SKIP() << "Test requires support for at least two video encode profiles";
    }

    VideoContext context1(m_device, configs[0]);
    VideoContext context2(m_device, configs[1]);
    context1.CreateAndBindSessionMemory();
    context2.CreateEncodeFeedbackQueryPool();

    vkt::CommandBuffer& cb = context1.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryType-07130");
    vk::CmdBeginQuery(cb.handle(), context2.EncodeFeedbackQueryPool(), 0, 0);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.End();
}

TEST_F(NegativeVideoEncode, BeginQueryEncodeFeedbackNoBoundVideoSession) {
    TEST_DESCRIPTION("vkCmdBeginQuery - encode feedback query requires bound video session");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateEncodeFeedbackQueryPool();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryType-07129");
    vk::CmdBeginQuery(cb.handle(), context.EncodeFeedbackQueryPool(), 0, 0);
    m_errorMonitor->VerifyFound();

    cb.End();
}
