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

#include "shader_object_helper.h"
#include <vulkan/utility/vk_struct_helper.hpp>

VkShaderCreateInfoEXT ShaderCreateInfo(const std::vector<uint32_t>& spirv, VkShaderStageFlagBits stage, uint32_t set_layout_count,
                                       const VkDescriptorSetLayout* set_layouts, uint32_t pc_range_count,
                                       const VkPushConstantRange* pc_ranges, const VkSpecializationInfo* specialization_info) {
    VkShaderCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.flags = 0;
    create_info.stage = stage;
    create_info.nextStage = 0;
    create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    create_info.codeSize = spirv.size() * sizeof(uint32_t);
    create_info.pCode = spirv.data();
    create_info.pName = "main";
    create_info.setLayoutCount = set_layout_count;
    create_info.pSetLayouts = set_layouts;
    create_info.pushConstantRangeCount = pc_range_count;
    create_info.pPushConstantRanges = pc_ranges;
    create_info.pSpecializationInfo = specialization_info;
    return create_info;
}

VkShaderCreateInfoEXT ShaderCreateInfoFlag(const std::vector<uint32_t>& spirv, VkShaderStageFlagBits stage,
                                           VkShaderCreateFlagsEXT flags) {
    VkShaderCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.flags = flags;
    create_info.stage = stage;
    create_info.nextStage = 0;
    create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    create_info.codeSize = spirv.size() * sizeof(uint32_t);
    create_info.pCode = spirv.data();
    create_info.pName = "main";
    create_info.setLayoutCount = 0;
    create_info.pSetLayouts = nullptr;
    create_info.pushConstantRangeCount = 0;
    create_info.pPushConstantRanges = nullptr;
    create_info.pSpecializationInfo = nullptr;
    return create_info;
}

VkShaderCreateInfoEXT ShaderCreateInfoLink(const std::vector<uint32_t>& spirv, VkShaderStageFlagBits stage,
                                           VkShaderStageFlags next_stage) {
    VkShaderCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    create_info.stage = stage;
    create_info.nextStage = next_stage;
    create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    create_info.codeSize = spirv.size() * sizeof(uint32_t);
    create_info.pCode = spirv.data();
    create_info.pName = "main";
    return create_info;
}