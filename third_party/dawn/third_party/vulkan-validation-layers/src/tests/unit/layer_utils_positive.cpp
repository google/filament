
/*
 * Copyright (c) 2023-2024 Valve Corporation
 * Copyright (c) 2023-2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "utils/vk_layer_utils.h"

class PositiveLayerUtils : public VkLayerTest {};

// These test check utils in the layer without needing to create a full Vulkan instance

TEST_F(PositiveLayerUtils, GetEffectiveExtent) {
    TEST_DESCRIPTION("Test unlikely GetEffectiveExtent edge cases");

    VkImageCreateInfo ci = vku::InitStructHelper();
    VkExtent3D extent = {};

    // Return zero extent if mip level doesn't exist
    {
        ci.mipLevels = 0;
        ci.extent = {1, 1, 1};
        extent = GetEffectiveExtent(ci, VK_IMAGE_ASPECT_NONE, 1);
        ASSERT_TRUE(extent.width == 0);
        ASSERT_TRUE(extent.height == 0);
        ASSERT_TRUE(extent.depth == 0);

        ci.mipLevels = 1;
        extent = GetEffectiveExtent(ci, VK_IMAGE_ASPECT_NONE, 1);
        ASSERT_TRUE(extent.width == 0);
        ASSERT_TRUE(extent.height == 0);
        ASSERT_TRUE(extent.depth == 0);
    }

    // Check that 0 based extent is respected
    {
        ci.flags = 0;
        ci.imageType = VK_IMAGE_TYPE_3D;
        ci.mipLevels = 2;
        ci.extent = {0, 0, 0};
        extent = GetEffectiveExtent(ci, VK_IMAGE_ASPECT_NONE, 1);
        ASSERT_TRUE(extent.width == 0);
        ASSERT_TRUE(extent.height == 0);
        ASSERT_TRUE(extent.depth == 0);

        ci.extent = {16, 32, 0};
        extent = GetEffectiveExtent(ci, VK_IMAGE_ASPECT_NONE, 1);
        ASSERT_TRUE(extent.width == 8);
        ASSERT_TRUE(extent.height == 16);
        ASSERT_TRUE(extent.depth == 0);
    }

    // Corner sampled images
    {
        ci.flags = VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV;
        ci.imageType = VK_IMAGE_TYPE_3D;
        ci.mipLevels = 2;
        ci.extent = {1, 1, 1};
        extent = GetEffectiveExtent(ci, VK_IMAGE_ASPECT_NONE, 1);
        // The minimum level size is 2x2x2 for 3D corner sampled images.
        ASSERT_TRUE(extent.width == 2);
        ASSERT_TRUE(extent.height == 2);
        ASSERT_TRUE(extent.depth == 2);

        ci.flags = VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV;
        ci.imageType = VK_IMAGE_TYPE_3D;
        ci.mipLevels = 2;
        ci.extent = {4, 8, 16};
        extent = GetEffectiveExtent(ci, VK_IMAGE_ASPECT_NONE, 1);
        ASSERT_TRUE(extent.width == 2);
        ASSERT_TRUE(extent.height == 4);
        ASSERT_TRUE(extent.depth == 8);

        ci.flags = VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV;
        ci.imageType = VK_IMAGE_TYPE_3D;
        ci.mipLevels = 3;
        ci.extent = {8, 16, 32};
        extent = GetEffectiveExtent(ci, VK_IMAGE_ASPECT_NONE, 2);
        ASSERT_TRUE(extent.width == 2);
        ASSERT_TRUE(extent.height == 4);
        ASSERT_TRUE(extent.depth == 8);
    }
}

TEST_F(PositiveLayerUtils, IsOnlyOneValidPlaneAspect) {
    const VkFormat two_plane_format = VK_FORMAT_G8_B8R8_2PLANE_422_UNORM;
    ASSERT_FALSE(IsOnlyOneValidPlaneAspect(two_plane_format, 0));
    ASSERT_FALSE(IsOnlyOneValidPlaneAspect(two_plane_format, VK_IMAGE_ASPECT_COLOR_BIT));
    ASSERT_TRUE(IsOnlyOneValidPlaneAspect(two_plane_format, VK_IMAGE_ASPECT_PLANE_0_BIT));
    ASSERT_FALSE(IsOnlyOneValidPlaneAspect(two_plane_format, VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT));
    ASSERT_FALSE(IsOnlyOneValidPlaneAspect(two_plane_format, VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT));
    ASSERT_FALSE(IsOnlyOneValidPlaneAspect(two_plane_format, VK_IMAGE_ASPECT_PLANE_2_BIT));
}

TEST_F(PositiveLayerUtils, IsValidPlaneAspect) {
    const VkFormat two_plane_format = VK_FORMAT_G8_B8R8_2PLANE_422_UNORM;
    ASSERT_FALSE(IsValidPlaneAspect(two_plane_format, 0));
    ASSERT_FALSE(IsValidPlaneAspect(two_plane_format, VK_IMAGE_ASPECT_COLOR_BIT));
    ASSERT_TRUE(IsValidPlaneAspect(two_plane_format, VK_IMAGE_ASPECT_PLANE_0_BIT));
    ASSERT_TRUE(IsValidPlaneAspect(two_plane_format, VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT));
    ASSERT_FALSE(IsValidPlaneAspect(two_plane_format, VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT));
    ASSERT_FALSE(IsValidPlaneAspect(two_plane_format, VK_IMAGE_ASPECT_PLANE_2_BIT));
}

TEST_F(PositiveLayerUtils, IsMultiplePlaneAspect) {
    ASSERT_FALSE(IsMultiplePlaneAspect(0));
    ASSERT_FALSE(IsMultiplePlaneAspect(VK_IMAGE_ASPECT_COLOR_BIT));
    ASSERT_FALSE(IsMultiplePlaneAspect(VK_IMAGE_ASPECT_PLANE_0_BIT));
    ASSERT_TRUE(IsMultiplePlaneAspect(VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT));
    ASSERT_TRUE(IsMultiplePlaneAspect(VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT));
    ASSERT_TRUE(IsMultiplePlaneAspect(VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT));
    ASSERT_FALSE(IsMultiplePlaneAspect(VK_IMAGE_ASPECT_PLANE_2_BIT));
}

TEST_F(PositiveLayerUtils, IsImageLayoutDepthOnly) {
    ASSERT_TRUE(IsImageLayoutDepthOnly(VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL));
    ASSERT_TRUE(IsImageLayoutDepthOnly(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL));
    ASSERT_TRUE(IsImageLayoutDepthOnly(VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL));
    ASSERT_TRUE(IsImageLayoutDepthOnly(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL));

    constexpr std::array all_layouts_but_depth_only{
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
        // VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        // VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR,
        VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR,
        VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT, VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR,
#ifdef VK_ENABLE_BETA_EXTENSIONS
        VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR,
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
        VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR,
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
        VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR,
#endif
        VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV,
        // VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        // VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_MAX_ENUM};

    for (auto layout : all_layouts_but_depth_only) {
        ASSERT_FALSE(IsImageLayoutDepthOnly(layout));
    }
}

TEST_F(PositiveLayerUtils, IsImageLayoutStencilOnly) {
    ASSERT_TRUE(IsImageLayoutStencilOnly(VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL));
    ASSERT_TRUE(IsImageLayoutStencilOnly(VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL));
    ASSERT_TRUE(IsImageLayoutStencilOnly(VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL));
    ASSERT_TRUE(IsImageLayoutStencilOnly(VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL));

    constexpr std::array all_layouts_but_stencil_only{
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
        // VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
        // VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR,
        VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT,
        VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR,
#ifdef VK_ENABLE_BETA_EXTENSIONS
        VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR,
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
        VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR,
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
        VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR,
#endif
        VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
        // VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
        // VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_MAX_ENUM};

    for (auto layout : all_layouts_but_stencil_only) {
        ASSERT_FALSE(IsImageLayoutStencilOnly(layout));
    }
}
