// Copyright 2023-2025 The Khronos Group Inc.
// Copyright 2023-2025 Valve Corporation
// Copyright 2023-2025 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0
#include <vulkan/utility/vk_format_utils.h>

bool check_format_utils() {
    vkuGetPlaneIndex(VK_IMAGE_ASPECT_PLANE_1_BIT);
    vkuFormatHasGreen(VK_FORMAT_R8G8B8A8_UNORM);
    vkuFormatTexelBlockSize(VK_FORMAT_ASTC_5x4_SRGB_BLOCK);
    struct VKU_FORMAT_INFO f = vkuGetFormatInfo(VK_FORMAT_R8G8B8A8_SRGB);
    if (f.component_count != 4) {
        return false;
    }
    return true;
}
