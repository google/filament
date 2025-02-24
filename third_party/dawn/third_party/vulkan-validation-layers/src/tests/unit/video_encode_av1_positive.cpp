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

class PositiveVideoEncodeAV1 : public VkVideoLayerTest {};

TEST_F(PositiveVideoEncodeAV1, Basic) {
    TEST_DESCRIPTION("Tests basic AV1 video encode use case for framework verification purposes");

    RETURN_IF_SKIP(Init());

    const uint32_t dpb_slots = 3;
    const uint32_t active_refs = 2;

    VideoConfig config = GetConfig(
        GetConfigsWithReferences(GetConfigsWithDpbSlots(GetConfigsWithRateControl(GetConfigsEncodeAV1()), dpb_slots), active_refs));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with rate control and 3 DPB slots and 2 active references";
    }

    config.SessionCreateInfo()->maxDpbSlots = dpb_slots;
    config.SessionCreateInfo()->maxActiveReferencePictures = active_refs;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config).SetAnyMode();
    for (uint32_t i = 0; i < config.EncodeCaps()->maxRateControlLayers; ++i) {
        auto rc_layer = VideoEncodeRateControlLayerInfo(config);
        rc_layer->averageBitrate = 128000;
        rc_layer->maxBitrate = 128000;
        rc_layer->frameRateNumerator = 30;
        rc_layer->frameRateDenominator = 1;
        rc_info.AddLayer(rc_layer);
    }

    cb.Begin();
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.EncodeInput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR));
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(-1, 1).AddResource(-1, 2));
    cb.ControlVideoCoding(context.Control().Reset().RateControl(rc_info).EncodeQualityLevel(0));
    cb.EncodeVideo(context.EncodeReferenceFrame(0));
    cb.EncodeVideo(context.EncodeFrame(1).AddReferenceFrame(0));
    cb.EncodeVideo(context.EncodeReferenceFrame(1).AddReferenceFrame(0));
    cb.EncodeVideo(context.EncodeFrame(2));
    cb.EncodeVideo(context.EncodeReferenceFrame(2).AddReferenceFrame(0).AddReferenceFrame(1));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.Begin();
    cb.BeginVideoCoding(
        context.Begin().RateControl(rc_info).AddResource(0, 0).InvalidateSlot(1).AddResource(-1, 1).AddResource(2, 2));
    cb.EncodeVideo(context.EncodeFrame(1));
    cb.EncodeVideo(context.EncodeReferenceFrame(1).AddReferenceFrame(0).AddReferenceFrame(2));
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(1));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();
}

TEST_F(PositiveVideoEncodeAV1, RateControlLayerCount) {
    TEST_DESCRIPTION(
        "vkCmdBeginVideoCodingKHR / vkCmdControlVideoCodingKHR - AV1 temporal layer count must only match "
        "the layer count if the layer count is greater than 1");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeAV1()), [](const VideoConfig& config) {
        return config.EncodeCapsAV1()->maxTemporalLayerCount > 1;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with rate control and temporal layer support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config, true).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    rc_info.CodecInfo().encode_av1.temporalLayerCount = 2;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(PositiveVideoEncodeAV1, FrameSizeOverride) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 frame size override should be allowed when supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [](const VideoConfig& config) {
        return ((config.Caps()->minCodedExtent.width < config.Caps()->maxCodedExtent.width) ||
                (config.Caps()->minCodedExtent.height < config.Caps()->maxCodedExtent.height)) &&
               (config.EncodeCapsAV1()->flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_FRAME_SIZE_OVERRIDE_BIT_KHR) != 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with frame size override support";
    }

    config.UpdateMaxCodedExtent(config.Caps()->maxCodedExtent);

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    // We will use a smaller resolution than the max
    auto encode_info = context.EncodeFrame();
    encode_info.CodecInfo().encode_av1.std_picture_info.flags.frame_size_override_flag = 1;
    encode_info->srcPictureResource.codedExtent = config.Caps()->minCodedExtent;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.EncodeVideo(encode_info);
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(PositiveVideoEncodeAV1, MotionVectorScaling) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 motion vector scaling should be allowed when supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(
        GetConfigsWithReferences(FilterConfigs(
            GetConfigsEncodeAV1(),
            [](const VideoConfig& config) {
                return ((config.Caps()->minCodedExtent.width < config.Caps()->maxCodedExtent.width) ||
                        (config.Caps()->minCodedExtent.height < config.Caps()->maxCodedExtent.height)) &&
                       (config.EncodeCapsAV1()->flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_MOTION_VECTOR_SCALING_BIT_KHR) != 0;
            })),
        2));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with references and motion vector scaling support";
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
    cb.EncodeVideo(context.EncodeFrame(0).AddReferenceFrame(1, &patched_resource));
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(PositiveVideoEncodeAV1, SingleReference) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 single reference prediction");

    RETURN_IF_SKIP(Init());

    // Single reference prediction requires at least one active reference picture
    const uint32_t min_ref_count = 1;

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [&](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > min_ref_count && config.Caps()->maxActiveReferencePictures >= min_ref_count &&
               config.EncodeCapsAV1()->maxSingleReferenceCount > 0;
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

    // Test all supported reference names
    for (uint8_t ref_name_idx = 0; ref_name_idx < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++ref_name_idx) {
        if ((config.EncodeCapsAV1()->singleReferenceNameMask & (1 << ref_name_idx)) != 0) {
            for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
                encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
            }
            encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[ref_name_idx] = 1;
            encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame = ref_name_idx;

            cb.EncodeVideo(encode_info);
        }
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(PositiveVideoEncodeAV1, UnidirectionalCompound) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 unidirectional compound prediction");

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
    std::vector<std::pair<uint8_t, uint8_t>> ref_name_pairs = {
        std::make_pair<uint8_t, uint8_t>(0, 1),  // LAST_FRAME + LAST2_FRAME
        std::make_pair<uint8_t, uint8_t>(0, 2),  // LAST_FRAME + LAST3_FRAME
        std::make_pair<uint8_t, uint8_t>(0, 3),  // LAST_FRAME + GOLDEN_FRAME
        std::make_pair<uint8_t, uint8_t>(4, 6),  // BWDREF_FRAME + ALTREF_FRAME
    };

    // Test all supported reference name combinations
    for (auto ref_name_pair : ref_name_pairs) {
        const uint32_t ref_name_mask = (1 << ref_name_pair.first) | (1 << ref_name_pair.second);
        if ((config.EncodeCapsAV1()->unidirectionalCompoundReferenceNameMask & ref_name_mask) == ref_name_mask) {
            for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
                encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
            }
            encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[ref_name_pair.first] = 1;
            encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[ref_name_pair.second] = 1;
            encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame = ref_name_pair.first;

            cb.EncodeVideo(encode_info);
        }
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(PositiveVideoEncodeAV1, BidirectionalCompound) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 bidirectional compound prediction");

    RETURN_IF_SKIP(Init());

    // Bidirectional compound prediction requires at least one active reference picture
    // No need for two pictures as both reference names can point to the same picture
    const uint32_t min_ref_count = 1;

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [&](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > min_ref_count && config.Caps()->maxActiveReferencePictures >= min_ref_count &&
               config.EncodeCapsAV1()->maxBidirectionalCompoundReferenceCount > 0;
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

    VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
    encode_info.CodecInfo().encode_av1.picture_info.predictionMode = VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_BIDIRECTIONAL_COMPOUND_KHR;

    // Test all supported reference name combinations (one from group 1 and one from group 2)
    const uint8_t bwdref_frame_idx = STD_VIDEO_AV1_REFERENCE_NAME_BWDREF_FRAME - 1;
    for (uint8_t ref_name_1 = 0; ref_name_1 < bwdref_frame_idx; ref_name_1++) {
        for (uint8_t ref_name_2 = bwdref_frame_idx; ref_name_2 < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ref_name_2++) {
            const uint32_t ref_name_mask = (1 << ref_name_1) | (1 << ref_name_2);
            if ((config.EncodeCapsAV1()->bidirectionalCompoundReferenceNameMask & ref_name_mask) == ref_name_mask) {
                for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
                    encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
                }
                encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[ref_name_1] = 1;
                encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[ref_name_2] = 1;
                encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame = ref_name_1;

                cb.EncodeVideo(encode_info);
            }
        }
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(PositiveVideoEncodeAV1, GetEncodedSessionParams) {
    TEST_DESCRIPTION("vkGetEncodedVideoSessionParametersKHR - test basic usage");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    VideoContext context(m_device, config);

    auto get_info = vku::InitStruct<VkVideoEncodeSessionParametersGetInfoKHR>();
    get_info.videoSessionParameters = context.SessionParams();

    auto feedback_info = vku::InitStruct<VkVideoEncodeSessionParametersFeedbackInfoKHR>();
    size_t data_size = 0;

    // Calling without feedback info and data pointer is legal
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);

    std::vector<uint8_t> data_buffer(data_size);

    // Calling without feedback info but data pointer is legal
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, data_buffer.data());

    // Calling with feedback info not including codec-specific feedback info
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, &feedback_info, &data_size, nullptr);
}
