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

class NegativeVideoDecode : public VkVideoLayerTest {};

TEST_F(NegativeVideoDecode, CreateSessionInvalidDecodeReferencePictureFormat) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid decode reference picture format");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-referencePictureFormat-04852");
    create_info.referencePictureFormat = VK_FORMAT_D16_UNORM;
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoDecode, CreateSessionInvalidDecodePictureFormat) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid decode picture format");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pictureFormat-04853");
    create_info.pictureFormat = VK_FORMAT_D16_UNORM;
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoDecode, BeginCodingSlotInactive) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - referenced DPB slot is inactive");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    if (!config) {
        GTEST_SKIP() << "Test requires a video decode profile with reference picture support";
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
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, 0, 1));
    cb.DecodeVideo(context.DecodeReferenceFrame(0));
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

TEST_F(NegativeVideoDecode, BeginCodingInvalidSlotResourceAssociation) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - referenced DPB slot is not associated with the specified resource");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires a video decode profile with reference picture support and 2 DPB slots";
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
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, 0, 1));
    cb.DecodeVideo(context.DecodeReferenceFrame(0));
    cb.EndVideoCoding(context.End());

    cb.BeginVideoCoding(context.Begin().AddResource(0, 1));
    cb.EndVideoCoding(context.End());
    cb.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-pPictureResource-07265");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeVideoDecode, BeginCodingMissingDecodeDpbUsage) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - reference picture resource missing VIDEO_DECODE_DPB usage");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsDecode(), 1));
    if (!config) {
        GTEST_SKIP() << "Test requires a decode profile with reference picture support";
    }

    if (config.DpbFormatProps()->imageUsageFlags == VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR) {
        GTEST_SKIP() << "Test requires reference format to support at least one more usage besides DECODE_DPB";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoPictureResourceInfoKHR res = context.Dpb()->Picture(0);

    auto view_usage_ci = vku::InitStruct<VkImageViewUsageCreateInfo>();
    view_usage_ci.usage = config.DpbFormatProps()->imageUsageFlags ^ VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR;

    auto image_view_ci = vku::InitStruct<VkImageViewCreateInfo>(&view_usage_ci);
    image_view_ci.image = context.Dpb()->Image();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    image_view_ci.format = config.DpbFormatProps()->format;
    image_view_ci.components = config.DpbFormatProps()->componentMapping;
    image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    vkt::ImageView image_view(*m_device, image_view_ci);

    res.imageViewBinding = image_view.handle();

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-slotIndex-07245");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeSessionNotDecode) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - bound video session was not created with decode operation");

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

    vkt::CommandBuffer& cb = encode_context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(encode_context.Begin());

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-commandBuffer-cmdpool");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-None-08249");
    cb.DecodeVideo(decode_context.DecodeFrame());
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(encode_context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeSessionUninitialized) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - session uninitialized");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.DecodeVideo(context.DecodeFrame());
    cb.EndVideoCoding(context.End());
    cb.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-None-07011");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeVideoDecode, DecodeProtectedNoFaultBitstreamBuffer) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - protectedNoFault tests for bitstream buffer");

    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(Init());
    if (IsProtectedNoFaultSupported()) {
        GTEST_SKIP() << "Test requires protectedMemory support without protectedNoFault support";
    }

    VideoConfig config = GetConfigWithProtectedContent(GetConfigsDecode());
    if (!config) {
        GTEST_SKIP() << "Test requires a video decode profile with protected content support";
    }

    const bool use_protected = true;

    VideoContext unprotected_context(m_device, config);
    unprotected_context.CreateAndBindSessionMemory();
    unprotected_context.CreateResources(use_protected /* bitstream */, false /* DPB */, false /* dst image */);

    vkt::CommandBuffer& unprotected_cb = unprotected_context.CmdBuffer();

    VideoContext protected_context(m_device, config, use_protected);
    protected_context.CreateAndBindSessionMemory();
    protected_context.CreateResources(false /* bitstream */, use_protected /* DPB */, use_protected /* dst image */);

    vkt::CommandBuffer& protected_cb = protected_context.CmdBuffer();

    unprotected_cb.Begin();
    unprotected_cb.BeginVideoCoding(unprotected_context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-commandBuffer-07136");
    unprotected_cb.DecodeVideo(unprotected_context.DecodeFrame());
    m_errorMonitor->VerifyFound();

    unprotected_cb.EndVideoCoding(unprotected_context.End());
    unprotected_cb.End();

    protected_cb.Begin();
    protected_cb.BeginVideoCoding(protected_context.Begin());

    protected_cb.DecodeVideo(protected_context.DecodeFrame());

    protected_cb.EndVideoCoding(protected_context.End());
    protected_cb.End();
}

TEST_F(NegativeVideoDecode, DecodeProtectedNoFaultDecodeOutput) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - protectedNoFault tests for decode output");

    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(Init());
    if (IsProtectedNoFaultSupported()) {
        GTEST_SKIP() << "Test requires protectedMemory support without protectedNoFault support";
    }

    const bool use_protected = true;

    VideoConfig config = GetConfigWithProtectedContent(GetConfigsDecode());
    if (!config) {
        GTEST_SKIP() << "Test requires a video decode profile with protected content support";
    }

    VideoContext unprotected_context(m_device, config);
    unprotected_context.CreateAndBindSessionMemory();
    unprotected_context.CreateResources(false /* bitstream */, false /* DPB */, use_protected /* dst image */);

    vkt::CommandBuffer& unprotected_cb = unprotected_context.CmdBuffer();

    VideoContext protected_context(m_device, config, use_protected);
    protected_context.CreateAndBindSessionMemory();
    protected_context.CreateResources(use_protected /* bitstream */, use_protected /* DPB */, false /* dst image */);

    vkt::CommandBuffer& protected_cb = protected_context.CmdBuffer();

    unprotected_cb.Begin();
    unprotected_cb.BeginVideoCoding(unprotected_context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-commandBuffer-07147");
    unprotected_cb.DecodeVideo(unprotected_context.DecodeFrame());
    m_errorMonitor->VerifyFound();

    unprotected_cb.EndVideoCoding(unprotected_context.End());
    unprotected_cb.End();

    protected_cb.Begin();
    protected_cb.BeginVideoCoding(protected_context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-commandBuffer-07148");
    protected_cb.DecodeVideo(protected_context.DecodeFrame());
    m_errorMonitor->VerifyFound();

    protected_cb.EndVideoCoding(protected_context.End());
    protected_cb.End();
}

TEST_F(NegativeVideoDecode, DecodeImageLayouts) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - pictures should be in the expected layout");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires a video decode profile with reference picture support and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(-1, 1));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));

    cb.DecodeVideo(context.DecodeFrame(0));

    // Decode output must be in DECODE_DST layout if it is distinct from reconstructed
    if (config.SupportsDecodeOutputDistinct()) {
        vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_GENERAL));
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07252");
        cb.DecodeVideo(context.DecodeFrame(0));
        m_errorMonitor->VerifyFound();
        vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, 0, 1));
    }

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));

    // Decode output must be in DECODE_DPB layout if it coincides with reconstructed
    if (config.SupportsDecodeOutputCoincide()) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07254");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07253");
        vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, 0, 1));
        cb.DecodeVideo(context.DecodeFrame(0).SetDecodeOutput(context.Dpb()->Picture(0)));
        m_errorMonitor->VerifyFound();
    }

    // Reconstructed must be in DECODE_DPB layout
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07253");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07254");
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_GENERAL, 0, 1));
    cb.DecodeVideo(context.DecodeFrame(0));
    m_errorMonitor->VerifyFound();
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, 0, 1));

    // Reference must be in DECODE_DPB layout
    cb.DecodeVideo(context.DecodeReferenceFrame(0));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_GENERAL, 0, 1));
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pPictureResource-07255");
    cb.DecodeVideo(context.DecodeFrame(1).AddReferenceFrame(0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeInvalidResourceLayer) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - out-of-bounds layer index in VkVideoPictureResourceInfoKHR");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoPictureResourceInfoKHR dpb_res = context.Dpb()->Picture(0);
    dpb_res.baseArrayLayer = 5;

    VkVideoPictureResourceInfoKHR dst_res = context.DecodeOutput()->Picture();
    dst_res.baseArrayLayer = 5;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    // Invalid baseArrayLayer in VkVideoDecodeInfoKHR::dstPictureResource
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07141");
    m_errorMonitor->SetDesiredError("VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175");
    cb.DecodeVideo(context.DecodeFrame(0).SetDecodeOutput(dst_res));
    m_errorMonitor->VerifyFound();

    // Invalid baseArrayLayer in VkVideoDecodeInfoKHR::pSetupReferenceSlot
    if (config.SupportsDecodeOutputDistinct()) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07149");
        m_errorMonitor->SetDesiredError("VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175");
        cb.DecodeVideo(context.DecodeFrame(0, &dpb_res));
        m_errorMonitor->VerifyFound();
    } else {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07141");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07149");
        m_errorMonitor->SetDesiredError("VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175");
        m_errorMonitor->SetDesiredError("VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175");
        cb.DecodeVideo(context.DecodeFrame(0, &dpb_res).SetDecodeOutput(dst_res));
        m_errorMonitor->VerifyFound();
    }

    // Invalid baseArrayLayer in VkVideoDecodeInfoKHR::pReferenceSlots
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-slotIndex-07256");
    m_errorMonitor->SetDesiredError("VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175");
    cb.DecodeVideo(context.DecodeFrame(1).AddReferenceFrame(0, &dpb_res));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeQueryTooManyOperations) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - no more queries available to store operation results");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video decode queue to support result status queries";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();
    context.CreateStatusQueryPool(2);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 0, 0);
    cb.DecodeVideo(context.DecodeFrame());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-opCount-07134");
    cb.DecodeVideo(context.DecodeFrame());
    m_errorMonitor->VerifyFound();

    vk::CmdEndQuery(cb.handle(), context.StatusQueryPool(), 0);
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeIncompatBufferProfile) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - bitstream buffer must be compatible with the video profile");

    RETURN_IF_SKIP(Init());

    auto configs = GetConfigsDecode();
    if (configs.size() < 2) {
        GTEST_SKIP() << "Test requires two video decode profiles";
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

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07135");
    cb.DecodeVideo(context1.DecodeFrame().SetBitstream(context2.Bitstream()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeBufferMissingDecodeSrcUsage) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - bitstream buffer missing DECODE_SRC usage");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
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
    create_info.usage = VK_BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkt::Buffer buffer(*m_device, create_info);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeInfoKHR-srcBuffer-07165");
    cb.DecodeVideo(context.DecodeFrame().SetBitstreamBuffer(buffer.handle(), 0, create_info.size));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeBufferOffsetOutOfBounds) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - bitstream buffer offset out of bounds");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoDecodeInfo decode_info = context.DecodeFrame();
    decode_info->srcBufferOffset = decode_info->srcBufferRange;
    decode_info->srcBufferOffset += config.Caps()->minBitstreamBufferOffsetAlignment - 1;
    decode_info->srcBufferOffset &= ~(config.Caps()->minBitstreamBufferOffsetAlignment - 1);

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeInfoKHR-srcBufferOffset-07166");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoDecodeInfoKHR-srcBufferRange-07167");
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeBufferOffsetAlignment) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - bitstream buffer offset needs to be aligned");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(FilterConfigs(
        GetConfigsDecode(), [](const VideoConfig& config) { return config.Caps()->minBitstreamBufferOffsetAlignment > 1; }));
    if (!config) {
        GTEST_SKIP() << "Test requires a video decode profile with minBitstreamBufferOffsetAlignment > 1";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoDecodeInfo decode_info = context.DecodeFrame();
    ++decode_info->srcBufferOffset;
    decode_info->srcBufferRange -= config.Caps()->minBitstreamBufferSizeAlignment;

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07138");
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeBufferRangeOutOfBounds) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - bitstream buffer range out of bounds");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoDecodeInfo decode_info = context.DecodeFrame();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeInfoKHR-srcBufferRange-07167");
    decode_info->srcBufferOffset += config.Caps()->minBitstreamBufferOffsetAlignment;
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeInfoKHR-srcBufferRange-07167");
    decode_info->srcBufferOffset = 0;
    decode_info->srcBufferRange += config.Caps()->minBitstreamBufferSizeAlignment;
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeBufferRangeAlignment) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - bitstream buffer range needs to be aligned");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(FilterConfigs(
        GetConfigsDecode(), [](const VideoConfig& config) { return config.Caps()->minBitstreamBufferSizeAlignment > 1; }));
    if (!config) {
        GTEST_SKIP() << "Test requires a video decode profile with minBitstreamBufferSizeAlignment > 1";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoDecodeInfo decode_info = context.DecodeFrame();
    --decode_info->srcBufferRange;

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07139");
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeInvalidOutputAndSetupCoincide) {
    TEST_DESCRIPTION(
        "vkCmdDecodeVideoKHR - decode output and reconstructed pictures must not match "
        "if VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR is not supported");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(FilterConfigs(GetConfigsWithReferences(GetConfigsDecode()),
                                          [](const VideoConfig& config) { return !config.SupportsDecodeOutputCoincide(); }));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures and no support "
                        "for VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    // It is possible that the DPB uses a different format, so ignore format mismatch errors
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07143");

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07146");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07140");
    cb.DecodeVideo(context.DecodeFrame(0).SetDecodeOutput(context.Dpb()->Picture(0)));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeInvalidOutputAndSetupDistinct) {
    TEST_DESCRIPTION(
        "vkCmdDecodeVideoKHR - decode output and reconstructed pictures must match "
        "if VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR is not supported");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(FilterConfigs(GetConfigsWithReferences(GetConfigsDecode()),
                                          [](const VideoConfig& config) { return !config.SupportsDecodeOutputDistinct(); }));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures and no support "
                        "for VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07141");
    cb.DecodeVideo(context.DecodeFrame(0).SetDecodeOutput(context.DecodeOutput()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeInvalidSetupSlotIndex) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - invalid slot index specified for reconstructed picture");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsDecode(), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with 3 reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    VideoDecodeInfo decode_info = context.DecodeFrame(0);
    auto setup_slot = *decode_info->pSetupReferenceSlot;
    decode_info->pSetupReferenceSlot = &setup_slot;

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeInfoKHR-pSetupReferenceSlot-07168");
    setup_slot.slotIndex = -1;
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07170");
    setup_slot.slotIndex = 2;
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeInvalidRefSlotIndex) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - invalid slot index specified for reference picture");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsDecode(), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with 3 reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeInfoKHR-slotIndex-07171");
    cb.DecodeVideo(context.DecodeFrame(0).AddReferenceFrame(-1, 0));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-slotIndex-07256");
    cb.DecodeVideo(context.DecodeFrame(0).AddReferenceFrame(2, 0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeSetupNull) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - reconstructed picture is required for sessions with DPB slots");

    RETURN_IF_SKIP(Init())

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    VideoDecodeInfo decode_info = context.DecodeFrame(0);
    decode_info->pSetupReferenceSlot = NULL;

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-08376");
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeSetupResourceNull) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - reconstructed picture resource must not be NULL");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    VideoDecodeInfo decode_info = context.DecodeFrame(0);
    auto setup_slot = *decode_info->pSetupReferenceSlot;
    decode_info->pSetupReferenceSlot = &setup_slot;

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeInfoKHR-pSetupReferenceSlot-07169");
    setup_slot.pPictureResource = nullptr;
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeReferenceResourceNull) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - reference picture resource must not be NULL");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures with 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeInfoKHR-pPictureResource-07172");
    cb.DecodeVideo(context.DecodeFrame(1).AddReferenceFrame(0, nullptr));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeIncompatOutputPicProfile) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - decode output picture must be compatible with the video profile");

    RETURN_IF_SKIP(Init());

    VideoConfig configs[2] = {};
    const auto& all_configs = GetConfigsDecode();
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
        GTEST_SKIP() << "Test requires two video profiles with matching decode output format/size";
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

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07142");
    cb.DecodeVideo(context1.DecodeFrame().SetDecodeOutput(context2.DecodeOutput()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeOutputFormatMismatch) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - decode output picture format mismatch");

    RETURN_IF_SKIP(Init());

    uint32_t alt_pic_format_index = 0;
    VideoConfig config = GetConfig(FilterConfigs(GetConfigsDecode(), [&alt_pic_format_index](const VideoConfig& config) {
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
        GTEST_SKIP() << "Test requires a video decode profile with support for two output picture formats";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoConfig config2 = config;
    config2.SetFormatProps({config.SupportedPictureFormatProps()[alt_pic_format_index]}, config.SupportedDpbFormatProps());
    VideoDecodeOutput output(m_device, config2);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07143");
    cb.DecodeVideo(context.DecodeFrame().SetDecodeOutput(output.Picture()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeOutputMissingDecodeDstUsage) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - decode output picture resource missing VIDEO_DECODE_DST usage");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    if (config.PictureFormatProps()->imageUsageFlags == VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR) {
        GTEST_SKIP() << "Test requires output format to support at least one more usage besides DECODE_DST";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto view_usage_ci = vku::InitStruct<VkImageViewUsageCreateInfo>();
    view_usage_ci.usage = config.PictureFormatProps()->imageUsageFlags ^ VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR;

    auto image_view_ci = vku::InitStruct<VkImageViewCreateInfo>(&view_usage_ci);
    image_view_ci.image = context.DecodeOutput()->Image();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    image_view_ci.format = config.PictureFormatProps()->format;
    image_view_ci.components = config.PictureFormatProps()->componentMapping;
    image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    vkt::ImageView image_view(*m_device, image_view_ci);

    VkVideoPictureResourceInfoKHR dst_res = context.DecodeOutput()->Picture();
    dst_res.imageViewBinding = image_view;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07146");
    cb.DecodeVideo(context.DecodeFrame().SetDecodeOutput(dst_res));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeOutputCodedOffsetExtent) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - invalid decode output picture coded offset/extent");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VkVideoPictureResourceInfoKHR dst_res = context.DecodeOutput()->Picture();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07144");
    ++dst_res.codedOffset.x;
    cb.DecodeVideo(context.DecodeFrame().SetDecodeOutput(dst_res));
    --dst_res.codedOffset.x;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07144");
    ++dst_res.codedOffset.y;
    cb.DecodeVideo(context.DecodeFrame().SetDecodeOutput(dst_res));
    --dst_res.codedOffset.y;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07145");
    ++dst_res.codedExtent.width;
    cb.DecodeVideo(context.DecodeFrame().SetDecodeOutput(dst_res));
    --dst_res.codedExtent.width;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07145");
    ++dst_res.codedExtent.height;
    cb.DecodeVideo(context.DecodeFrame().SetDecodeOutput(dst_res));
    --dst_res.codedExtent.height;
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeSetupAndRefCodedOffset) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - invalid reconstructed/reference picture coded offset");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures with 2 DPB slots";
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

    if (!config.SupportsDecodeOutputDistinct()) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07144");
    }
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07149");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07173");
    ++res.codedOffset.x;
    cb.DecodeVideo(context.DecodeFrame(0, &res));
    --res.codedOffset.x;
    m_errorMonitor->VerifyFound();

    if (!config.SupportsDecodeOutputDistinct()) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07144");
    }
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07149");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07173");
    ++res.codedOffset.y;
    cb.DecodeVideo(context.DecodeFrame(0, &res));
    --res.codedOffset.y;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-codedOffset-07257");
    ++res.codedOffset.x;
    cb.DecodeVideo(context.DecodeFrame(1).AddReferenceFrame(0, &res));
    --res.codedOffset.x;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-codedOffset-07257");
    ++res.codedOffset.y;
    cb.DecodeVideo(context.DecodeFrame(1).AddReferenceFrame(0, &res));
    --res.codedOffset.y;
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeSetupResourceNotBound) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - used reconstructed picture resource is not bound");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07149");
    cb.DecodeVideo(context.DecodeFrame(0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeRefResourceNotBoundToDpbSlot) {
    TEST_DESCRIPTION(
        "vkCmdDecodeVideoKHR - used reference picture resource is not bound as a resource "
        "currently associated with the corresponding DPB slot");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures and 3 DPB slots";
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
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(1));
    m_errorMonitor->VerifyFound();

    // Trying to refer to bound reference picture resource, but with incorrect slot index
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(1, 0));
    m_errorMonitor->VerifyFound();

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, 0, 1));
    cb.DecodeVideo(context.DecodeReferenceFrame(1, 0));
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(1, 0));

    // Trying to refer to bound reference picture resource, but with incorrect slot index after
    // the associated DPB slot index has been updated within the video coding scope
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0, 0));
    m_errorMonitor->VerifyFound();

    cb.ControlVideoCoding(context.Control().Reset());

    // Trying to refer to bound reference picture resource after reset
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(1, 0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeTooManyReferences) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - reference picture count exceeds maxActiveReferencePictures");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures and 3 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 3;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(-1, 2));

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-activeReferencePictureCount-07150");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0).AddReferenceFrame(1));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeDuplicateRefResource) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - same reference picture resource is used twice");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode(), 2), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with 2 reference pictures and 3 DPB slots";
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

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07264");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0, &res).AddReferenceFrame(1, &res));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-dpbFrameUseCount-07176");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07264");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0, &res).AddReferenceFrame(0, &res));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeDuplicateFrame) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - same DPB frame reference is used twice");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode(), 2), 3));
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
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(0, 1).AddResource(-1, 2));

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-dpbFrameUseCount-07176");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0, 0).AddReferenceFrame(0, 1));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, ImplicitDeactivation) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - test DPB slot deactivation caused by reference invalidation");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference picture support and 2 DPB slots";
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
    cb.DecodeVideo(context.DecodeReferenceFrame(0));
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

TEST_F(NegativeVideoDecode, DecodeInlineQueryOpCount) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - inline query count does not match video codec operation count");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video queue to support result status queries";
    }

    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();
    context.CreateStatusQueryPool(2);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-08365");
    cb.DecodeVideo(context.DecodeFrame().InlineQuery(context.StatusQueryPool(), 0, 2));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeInlineQueryOutOfBounds) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - inline query firstQuery/queryCount out of bounds");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video queue to support result status queries";
    }

    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();
    context.CreateStatusQueryPool(4);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoInlineQueryInfoKHR-queryPool-08372");
    m_errorMonitor->SetDesiredError("VUID-VkVideoInlineQueryInfoKHR-queryPool-08373");
    cb.DecodeVideo(context.DecodeFrame().InlineQuery(context.StatusQueryPool(), 4, 1));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pNext-08365");
    m_errorMonitor->SetDesiredError("VUID-VkVideoInlineQueryInfoKHR-queryPool-08373");
    cb.DecodeVideo(context.DecodeFrame().InlineQuery(context.StatusQueryPool(), 2, 3));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeInlineQueryUnavailable) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - inline queries must be unavailable");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video queue to support result status queries";
    }

    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();
    context.CreateStatusQueryPool();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    // Use custom begin info because the default uses VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();

    cb.Begin(&begin_info);
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset());
    cb.DecodeVideo(context.DecodeFrame().InlineQuery(context.StatusQueryPool()));
    cb.EndVideoCoding(context.End());
    cb.End();

    m_command_buffer.Begin(&begin_info);
    vk::CmdResetQueryPool(m_command_buffer.handle(), context.StatusQueryPool(), 0, 1);
    m_command_buffer.End();

    // Will fail as query pool has never been reset before
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-08366");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    // Will succeed this time as we reset the query
    context.Queue().Submit(cb);
    m_device->Wait();

    // Will fail again as we did not reset after use
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-08366");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    // Will succeed again after reset
    context.Queue().Submit(cb);
    m_device->Wait();
}

TEST_F(NegativeVideoDecode, DecodeInlineQueryType) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - inline query type is invalid");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video queue to support result status queries";
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

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-queryPool-08368");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-queryType-08367");
    cb.DecodeVideo(context.DecodeFrame().InlineQuery(query_pool.handle()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeInlineQueryProfileMismatch) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - inline query must have been created with the same profile");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    auto configs = GetConfigsDecode();
    if (configs.size() < 2) {
        GTEST_SKIP() << "Test requires support for at least two video decode profiles";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(configs[0].QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video queue to support result status queries";
    }

    configs[0].SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;

    VideoContext context1(m_device, configs[0]);
    VideoContext context2(m_device, configs[1]);
    context1.CreateAndBindSessionMemory();
    context1.CreateResources();
    context2.CreateStatusQueryPool();

    vkt::CommandBuffer& cb = context1.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-queryPool-08368");
    cb.DecodeVideo(context1.DecodeFrame().InlineQuery(context2.StatusQueryPool()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.End();
}

TEST_F(NegativeVideoDecode, DecodeInlineQueryIncompatibleQueueFamily) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - result status queries require queue family support");

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

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
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

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-commandBuffer-cmdpool");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-queryType-08369");
    cb.DecodeVideo(context.DecodeFrame().InlineQuery(context.StatusQueryPool()));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEndVideoCodingKHR-commandBuffer-cmdpool");
    cb.EndVideoCoding(context.End());

    cb.End();
}
