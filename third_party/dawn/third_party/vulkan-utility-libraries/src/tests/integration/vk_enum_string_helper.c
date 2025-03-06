// Copyright 2023 The Khronos Group Inc.
// Copyright 2023 Valve Corporation
// Copyright 2023 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0
#include <vulkan/vk_enum_string_helper.h>

// Ensure vk_enum_string_helper.h can be compiled with a C compiler
const char* string_VkResult_compiles() { return string_VkResult(VK_SUCCESS); }

// Ensure string_VkPipelineStageFlagBits2 is callable by C users
const char* vk_format_feature_2_sampled_image_bit() {
    return string_VkPipelineStageFlagBits2(VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT);
}
