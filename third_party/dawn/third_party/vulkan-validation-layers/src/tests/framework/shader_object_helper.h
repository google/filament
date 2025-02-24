/*
 * Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#pragma once

#include <vector>
#include <vulkan/vulkan.h>

// Assumes VK_SHADER_CODE_TYPE_SPIRV_EXT as that is how must tests will create the shader
VkShaderCreateInfoEXT ShaderCreateInfo(const std::vector<uint32_t>& spirv, VkShaderStageFlagBits stage,
                                       uint32_t set_layout_count = 0, const VkDescriptorSetLayout* set_layouts = nullptr,
                                       uint32_t pc_range_count = 0, const VkPushConstantRange* pc_ranges = nullptr,
                                       const VkSpecializationInfo* specialization_info = nullptr);

VkShaderCreateInfoEXT ShaderCreateInfoFlag(const std::vector<uint32_t>& spirv, VkShaderStageFlagBits stage,
                                           VkShaderCreateFlagsEXT flags);

VkShaderCreateInfoEXT ShaderCreateInfoLink(const std::vector<uint32_t>& spirv, VkShaderStageFlagBits stage,
                                           VkShaderStageFlags next_stage = 0);