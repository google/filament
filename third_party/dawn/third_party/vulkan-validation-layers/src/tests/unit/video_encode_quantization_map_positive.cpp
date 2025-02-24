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

class PositiveVideoEncodeQuantizationMap : public VkVideoLayerTest {};

TEST_F(PositiveVideoEncodeQuantizationMap, QuantDeltaMap) {
    TEST_DESCRIPTION("Tests video encode quantization delta maps");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    if (!GetConfigWithQuantDeltaMap(GetConfigsEncode())) {
        GTEST_SKIP() << "Test requires at least one encode profile with quantization delta map support";
    }

    // Test all encode configs that support quantization delta maps so that we cover all codecs
    // Furthermore, test all quantization delta map formats so that we cover different formats and texel sizes
    for (auto config : GetConfigsEncode()) {
        if (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR) {
            config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR;

            VideoContext context(m_device, config);
            context.CreateAndBindSessionMemory();
            context.CreateResources();

            vkt::CommandBuffer& cb = context.CmdBuffer();

            cb.Begin();
            cb.BeginVideoCoding(context.Begin());
            cb.ControlVideoCoding(context.Control().Reset());
            cb.EndVideoCoding(context.End());
            cb.End();
            context.Queue().Submit(cb);
            m_device->Wait();

            for (const auto& map_props : config.SupportedQuantDeltaMapProps()) {
                const auto texel_size = config.GetQuantMapTexelSize(map_props);
                auto params = context.CreateSessionParamsWithQuantMapTexelSize(texel_size);
                VideoEncodeQuantizationMap quantization_map(m_device, config, *map_props.ptr());

                cb.Begin();
                cb.BeginVideoCoding(context.Begin().SetSessionParams(params));
                cb.EncodeVideo(context.EncodeFrame().QuantizationMap(VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                                                     texel_size, quantization_map));
                cb.EndVideoCoding(context.End());
                cb.End();
                context.Queue().Submit(cb);
                m_device->Wait();
            }
        }
    }
}

TEST_F(PositiveVideoEncodeQuantizationMap, EmphasisMap) {
    TEST_DESCRIPTION("Tests video encode emphasis maps");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    if (!GetConfigWithEmphasisMap(GetConfigsEncode())) {
        GTEST_SKIP() << "Test requires at least one encode profile with emphasis map support";
    }

    // Test all encode configs that support emphasis maps so that we cover all codecs
    // Furthermore, test all emphasis map formats so that we cover different formats and texel sizes
    for (auto config : GetConfigsEncode()) {
        if (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR) {
            config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR;

            VideoContext context(m_device, config);
            context.CreateAndBindSessionMemory();
            context.CreateResources();

            vkt::CommandBuffer& cb = context.CmdBuffer();

            auto rc_info = VideoEncodeRateControlInfo(config).SetAnyMode();
            auto rc_layer = VideoEncodeRateControlLayerInfo(config);
            rc_layer->averageBitrate = 64000;
            rc_layer->maxBitrate = 64000;
            rc_layer->frameRateNumerator = 30;
            rc_layer->frameRateDenominator = 1;
            rc_info.AddLayer(rc_layer);

            cb.Begin();
            cb.BeginVideoCoding(context.Begin());
            cb.ControlVideoCoding(context.Control().Reset().RateControl(rc_info));
            cb.EndVideoCoding(context.End());
            cb.End();
            context.Queue().Submit(cb);
            m_device->Wait();

            for (const auto& map_props : config.SupportedEmphasisMapProps()) {
                const auto texel_size = config.GetQuantMapTexelSize(map_props);
                auto params = context.CreateSessionParamsWithQuantMapTexelSize(texel_size);
                VideoEncodeQuantizationMap quantization_map(m_device, config, *map_props.ptr());

                cb.Begin();
                cb.BeginVideoCoding(context.Begin().SetSessionParams(params).RateControl(rc_info));
                cb.EncodeVideo(
                    context.EncodeFrame().QuantizationMap(VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR, texel_size, quantization_map));
                cb.EndVideoCoding(context.End());
                cb.End();
                context.Queue().Submit(cb);
                m_device->Wait();
            }
        }
    }
}
