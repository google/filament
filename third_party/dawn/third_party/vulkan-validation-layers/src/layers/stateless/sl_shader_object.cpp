/* Copyright (c) 2023-2024 Nintendo
 * Copyright (c) 2023-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "stateless/stateless_validation.h"

namespace stateless {

bool Device::manual_PreCallValidateCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                    const VkShaderCreateInfoEXT *pCreateInfos,
                                                    const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                                    const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    for (uint32_t i = 0; i < createInfoCount; ++i) {
        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);
        const VkShaderCreateInfoEXT &createInfo = pCreateInfos[i];
        if (createInfo.codeType == VK_SHADER_CODE_TYPE_SPIRV_EXT && SafeModulo(createInfo.codeSize, 4) != 0) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-codeSize-08735", device, create_info_loc.dot(Field::codeSize),
                             "(%" PRIu64 ") is not a multiple of 4.", static_cast<uint64_t>(createInfo.codeSize));
        }

        auto pCode = reinterpret_cast<std::uintptr_t>(createInfo.pCode);
        if (createInfo.codeType == VK_SHADER_CODE_TYPE_BINARY_EXT) {
            if (SafeModulo(pCode, 16 * sizeof(unsigned char)) != 0) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-pCode-08492", device, create_info_loc.dot(Field::codeType),
                                 "is VK_SHADER_CODE_TYPE_BINARY_EXT, but pCodde is not aligned to 16 bytes.");
            }
        } else if (createInfo.codeType == VK_SHADER_CODE_TYPE_SPIRV_EXT) {
            if (SafeModulo(pCode, 4 * sizeof(unsigned char)) != 0) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-pCode-08493", device, create_info_loc.dot(Field::codeType),
                                 "is VK_SHADER_CODE_TYPE_SPIRV_EXT, but pCodde is not aligned to 4 bytes.");
            }
        }

        const VkShaderStageFlags linkedStages = VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT |
                                                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                                                VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_GEOMETRY_BIT |
                                                VK_SHADER_STAGE_FRAGMENT_BIT;
        if ((createInfo.stage & linkedStages) == 0 && (createInfo.flags & VK_SHADER_CREATE_LINK_STAGE_BIT_EXT) != 0) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-flags-08412", device, create_info_loc.dot(Field::flags),
                             "includes VK_SHADER_CREATE_LINK_STAGE_BIT_EXT but the stage is %s.",
                             string_VkShaderStageFlagBits(createInfo.stage));
        }
        if ((createInfo.stage != VK_SHADER_STAGE_COMPUTE_BIT) &&
            ((createInfo.flags & VK_SHADER_CREATE_DISPATCH_BASE_BIT_EXT) != 0)) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-flags-08485", device, create_info_loc.dot(Field::flags),
                             "includes VK_SHADER_CREATE_DISPATCH_BASE_BIT_EXT but the stage is %s.",
                             string_VkShaderStageFlagBits(createInfo.stage));
        }
        if (createInfo.stage != VK_SHADER_STAGE_FRAGMENT_BIT) {
            if ((createInfo.flags & VK_SHADER_CREATE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_EXT) != 0) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-flags-08486", device, create_info_loc.dot(Field::flags),
                                 "includes VK_SHADER_CREATE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_EXT but the stage is %s.",
                                 string_VkShaderStageFlagBits(createInfo.stage));
            }
            if ((createInfo.flags & VK_SHADER_CREATE_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT) != 0) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-flags-08488", device, create_info_loc.dot(Field::flags),
                                 "includes VK_SHADER_CREATE_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT but the stage is %s.",
                                 string_VkShaderStageFlagBits(createInfo.stage));
            }
        }

        if (createInfo.stage != VK_SHADER_STAGE_MESH_BIT_EXT && (createInfo.flags & VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT) != 0) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-flags-08414", device, create_info_loc.dot(Field::flags),
                             "includes VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT but the stage is %s.",
                             string_VkShaderStageFlagBits(createInfo.stage));
        }

        if (createInfo.stage == VK_SHADER_STAGE_VERTEX_BIT) {
            if ((createInfo.nextStage &
                 ~(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)) != 0) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08427", device, create_info_loc.dot(Field::stage),
                                 "is VK_SHADER_STAGE_VERTEX_BIT, but nextStage is %s.",
                                 string_VkShaderStageFlags(createInfo.nextStage).c_str());
            }
        } else if (createInfo.stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) {
            if ((createInfo.nextStage & ~(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)) != 0) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08430", device, create_info_loc.dot(Field::stage),
                                 "is VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, but nextStage is %s.",
                                 string_VkShaderStageFlags(createInfo.nextStage).c_str());
            }
        } else if (createInfo.stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
            if ((createInfo.nextStage & ~(VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)) != 0) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08431", device, create_info_loc.dot(Field::stage),
                                 "is VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, but nextStage is %s.",
                                 string_VkShaderStageFlags(createInfo.nextStage).c_str());
            }
        } else if (createInfo.stage == VK_SHADER_STAGE_GEOMETRY_BIT) {
            if ((createInfo.nextStage & ~VK_SHADER_STAGE_FRAGMENT_BIT) != 0) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08433", device, create_info_loc.dot(Field::stage),
                                 "is VK_SHADER_STAGE_GEOMETRY_BIT, but nextStage is %s.",
                                 string_VkShaderStageFlags(createInfo.nextStage).c_str());
            }
        } else if (createInfo.stage == VK_SHADER_STAGE_FRAGMENT_BIT || createInfo.stage == VK_SHADER_STAGE_COMPUTE_BIT) {
            if (createInfo.nextStage != 0) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08434", device, create_info_loc.dot(Field::stage),
                                 "is %s, but nextStage is %s.", string_VkShaderStageFlagBits(createInfo.stage),
                                 string_VkShaderStageFlags(createInfo.nextStage).c_str());
            }
        } else if (createInfo.stage == VK_SHADER_STAGE_TASK_BIT_EXT) {
            if ((createInfo.nextStage & ~VK_SHADER_STAGE_MESH_BIT_EXT) != 0) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08435", device, create_info_loc.dot(Field::stage),
                                 "is VK_SHADER_STAGE_TASK_BIT_EXT, but nextStage is %s.",
                                 string_VkShaderStageFlags(createInfo.nextStage).c_str());
            }
        } else if (createInfo.stage == VK_SHADER_STAGE_MESH_BIT_EXT) {
            if ((createInfo.nextStage & ~VK_SHADER_STAGE_FRAGMENT_BIT) != 0) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08436", device, create_info_loc.dot(Field::stage),
                                 "is VK_SHADER_STAGE_MESH_BIT_EXT, but nextStage is %s.",
                                 string_VkShaderStageFlags(createInfo.nextStage).c_str());
            }
        } else if (createInfo.stage == VK_SHADER_STAGE_ALL_GRAPHICS) {
            // string_VkShaderStageFlagBits can't print these, so list manually
            skip |= LogError("VUID-VkShaderCreateInfoEXT-stage-08418", device, create_info_loc.dot(Field::stage),
                             "is VK_SHADER_STAGE_ALL_GRAPHICS.");
        } else if (createInfo.stage == VK_SHADER_STAGE_ALL) {
            // string_VkShaderStageFlagBits can't print these, so list manually
            skip |= LogError("VUID-VkShaderCreateInfoEXT-stage-08418", device, create_info_loc.dot(Field::stage),
                             "is VK_SHADER_STAGE_ALL.");
        } else if (createInfo.stage == VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-stage-08425", device, create_info_loc.dot(Field::stage),
                             "is VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI.");
        } else if (createInfo.stage == VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-stage-08426", device, create_info_loc.dot(Field::stage),
                             "is VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI.");
        }

        if (((createInfo.flags & VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT) != 0) &&
            ((createInfo.stage & (VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_COMPUTE_BIT)) ==
             0)) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-flags-08992", device, create_info_loc.dot(Field::flags),
                             "includes VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT but the stage is %s.",
                             string_VkShaderStageFlagBits(createInfo.stage));
        }

        skip |= ValidatePushConstantRange(createInfo.pushConstantRangeCount, createInfo.pPushConstantRanges, create_info_loc);
    }

    return skip;
}

bool Device::manual_PreCallValidateGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t *pDataSize, void *pData,
                                                          const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (pData) {
        auto ptr = reinterpret_cast<std::uintptr_t>(pData);
        if (SafeModulo(ptr, 16 * sizeof(unsigned char)) != 0) {
            skip |= LogError("VUID-vkGetShaderBinaryDataEXT-None-08499", device, error_obj.location.dot(Field::pData),
                             "is not aligned to 16 bytes.");
        }
    }

    return skip;
}
}  // namespace stateless
