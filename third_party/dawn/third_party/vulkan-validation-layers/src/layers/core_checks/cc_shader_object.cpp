/* Copyright (c) 2023-2025 Nintendo
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

#include <vulkan/vulkan_core.h>
#include "core_validation.h"
#include "state_tracker/shader_object_state.h"
#include "state_tracker/shader_module.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/device_state.h"
#include "generated/spirv_grammar_helper.h"
#include "drawdispatch/drawdispatch_vuids.h"

VkShaderStageFlags FindNextStage(uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos, VkShaderStageFlagBits stage) {
    constexpr uint32_t graphicsStagesCount = 5;
    constexpr uint32_t meshStagesCount = 3;
    const VkShaderStageFlagBits graphicsStages[graphicsStagesCount] = {
        VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
        VK_SHADER_STAGE_GEOMETRY_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderStageFlagBits meshStages[meshStagesCount] = {VK_SHADER_STAGE_TASK_BIT_EXT, VK_SHADER_STAGE_MESH_BIT_EXT,
                                                               VK_SHADER_STAGE_FRAGMENT_BIT};

    uint32_t graphicsIndex = graphicsStagesCount;
    uint32_t meshIndex = meshStagesCount;
    for (uint32_t i = 0; i < graphicsStagesCount; ++i) {
        if (graphicsStages[i] == stage) {
            graphicsIndex = i;
            break;
        }
        if (i < meshStagesCount && meshStages[i] == stage) {
            meshIndex = i;
            break;
        }
    }

    if (graphicsIndex < graphicsStagesCount) {
        while (++graphicsIndex < graphicsStagesCount) {
            for (uint32_t i = 0; i < createInfoCount; ++i) {
                if (pCreateInfos[i].stage == graphicsStages[graphicsIndex]) {
                    return graphicsStages[graphicsIndex];
                }
            }
        }
    } else {
        while (++meshIndex < meshStagesCount) {
            for (uint32_t i = 0; i < createInfoCount; ++i) {
                if (pCreateInfos[i].stage == meshStages[meshIndex]) {
                    return meshStages[meshIndex];
                }
            }
        }
    }

    return 0;
}

bool CoreChecks::ValidateCreateShadersLinking(uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                              const Location& loc) const {
    bool skip = false;

    const uint32_t invalid = createInfoCount;
    uint32_t linked_stage = invalid;
    uint32_t non_linked_graphics_stage = invalid;
    uint32_t non_linked_task_mesh_stage = invalid;
    uint32_t linked_task_mesh_stage = invalid;
    uint32_t linked_vert_stage = invalid;
    uint32_t linked_task_stage = invalid;
    uint32_t linked_mesh_no_task_stage = invalid;
    uint32_t linked_spirv_index = invalid;
    uint32_t linked_binary_index = invalid;
    for (uint32_t i = 0; i < createInfoCount; ++i) {
        const Location create_info_loc = loc.dot(Field::pCreateInfos, i);
        const VkShaderCreateInfoEXT& create_info = pCreateInfos[i];
        if (create_info.stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT ||
            create_info.stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
            if (enabled_features.tessellationShader == VK_FALSE) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-stage-08419", device, create_info_loc.dot(Field::stage),
                                 "is %s, but the tessellationShader feature was not enabled.",
                                 string_VkShaderStageFlagBits(create_info.stage));
            }
        } else if (create_info.stage == VK_SHADER_STAGE_GEOMETRY_BIT) {
            if (enabled_features.geometryShader == VK_FALSE) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-stage-08420", device, create_info_loc.dot(Field::stage),
                                 "is VK_SHADER_STAGE_GEOMETRY_BIT, but the geometryShader feature was not enabled.");
            }
        } else if (create_info.stage == VK_SHADER_STAGE_TASK_BIT_EXT) {
            if (enabled_features.taskShader == VK_FALSE) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-stage-08421", device, create_info_loc.dot(Field::stage),
                                 "is VK_SHADER_STAGE_TASK_BIT_EXT, but the taskShader feature was not enabled.");
            }
        } else if (create_info.stage == VK_SHADER_STAGE_MESH_BIT_EXT) {
            if (enabled_features.meshShader == VK_FALSE) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-stage-08422", device, create_info_loc.dot(Field::stage),
                                 "is VK_SHADER_STAGE_MESH_BIT_EXT, but the meshShader feature was not enabled.");
            }
        }

        if ((create_info.flags & VK_SHADER_CREATE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_EXT) != 0 &&
            enabled_features.attachmentFragmentShadingRate == VK_FALSE) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-flags-08487", device, create_info_loc.dot(Field::flags),
                             "is %s, but the attachmentFragmentShadingRate feature was not enabled.",
                             string_VkShaderCreateFlagsEXT(create_info.flags).c_str());
        }
        if ((create_info.flags & VK_SHADER_CREATE_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT) != 0 &&
            enabled_features.fragmentDensityMap == VK_FALSE) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-flags-08489", device, create_info_loc.dot(Field::flags),
                             "is %s, but the fragmentDensityMap feature was not enabled.",
                             string_VkShaderCreateFlagsEXT(create_info.flags).c_str());
        }

        if ((create_info.flags & VK_SHADER_CREATE_LINK_STAGE_BIT_EXT) != 0) {
            const auto nextStage = FindNextStage(createInfoCount, pCreateInfos, create_info.stage);
            if (nextStage != 0 && create_info.nextStage != nextStage) {
                skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08409", device, create_info_loc.dot(Field::flags),
                                 "is %s, but nextStage (%s) does not equal the "
                                 "logically next stage (%s) which also has the VK_SHADER_CREATE_LINK_STAGE_BIT_EXT bit.",
                                 string_VkShaderCreateFlagsEXT(create_info.flags).c_str(),
                                 string_VkShaderStageFlags(create_info.nextStage).c_str(),
                                 string_VkShaderStageFlags(nextStage).c_str());
            }
            for (uint32_t j = i; j < createInfoCount; ++j) {
                if (i != j && create_info.stage == pCreateInfos[j].stage) {
                    skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08410", device, create_info_loc,
                                     "and pCreateInfos[%" PRIu32
                                     "] both contain VK_SHADER_CREATE_LINK_STAGE_BIT_EXT and have the stage %s.",
                                     j, string_VkShaderStageFlagBits(create_info.stage));
                }
            }

            linked_stage = i;
            if ((create_info.stage & VK_SHADER_STAGE_VERTEX_BIT) != 0) {
                linked_vert_stage = i;
            } else if ((create_info.stage & VK_SHADER_STAGE_TASK_BIT_EXT) != 0) {
                linked_task_mesh_stage = i;
                linked_task_stage = i;
            } else if ((create_info.stage & VK_SHADER_STAGE_MESH_BIT_EXT) != 0) {
                linked_task_mesh_stage = i;
                if ((create_info.flags & VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT) != 0) {
                    linked_mesh_no_task_stage = i;
                }
            }
            if (create_info.codeType == VK_SHADER_CODE_TYPE_SPIRV_EXT) {
                linked_spirv_index = i;
            } else if (create_info.codeType == VK_SHADER_CODE_TYPE_BINARY_EXT) {
                linked_binary_index = i;
            }
        } else if ((create_info.stage & (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                                         VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_GEOMETRY_BIT |
                                         VK_SHADER_STAGE_FRAGMENT_BIT)) != 0) {
            non_linked_graphics_stage = i;
        } else if ((create_info.stage & (VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)) != 0) {
            non_linked_task_mesh_stage = i;
        }

        if (enabled_features.tessellationShader == VK_FALSE &&
            (create_info.nextStage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT ||
             create_info.nextStage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08428", device, create_info_loc.dot(Field::nextStage),
                             "is %s, but tessellationShader feature was not enabled.",
                             string_VkShaderStageFlags(create_info.nextStage).c_str());
        }
        if (enabled_features.geometryShader == VK_FALSE && create_info.nextStage == VK_SHADER_STAGE_GEOMETRY_BIT) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08429", device, create_info_loc.dot(Field::nextStage),
                             "is VK_SHADER_STAGE_GEOMETRY_BIT, but tessellationShader feature was not enabled.");
        }
        if (create_info.stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT &&
            (create_info.nextStage & ~VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) > 0) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08430", device, create_info_loc.dot(Field::stage),
                             "is VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, but nextStage is %s.",
                             string_VkShaderStageFlags(create_info.nextStage).c_str());
        }
        if (create_info.stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT &&
            (create_info.nextStage & ~(VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)) > 0) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08431", device, create_info_loc.dot(Field::stage),
                             "is VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, but nextStage is %s.",
                             string_VkShaderStageFlags(create_info.nextStage).c_str());
        }
        if (create_info.stage == VK_SHADER_STAGE_GEOMETRY_BIT && (create_info.nextStage & ~VK_SHADER_STAGE_FRAGMENT_BIT) > 0) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08433", device, create_info_loc.dot(Field::stage),
                             "is VK_SHADER_STAGE_GEOMETRY_BIT, but nextStage is %s.",
                             string_VkShaderStageFlags(create_info.nextStage).c_str());
        }
        if ((create_info.stage == VK_SHADER_STAGE_FRAGMENT_BIT || create_info.stage == VK_SHADER_STAGE_COMPUTE_BIT) &&
            create_info.nextStage > 0) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08434", device, create_info_loc.dot(Field::stage),
                             "is %s, but nextStage is %s.", string_VkShaderStageFlagBits(create_info.stage),
                             string_VkShaderStageFlags(create_info.nextStage).c_str());
        }
        if (create_info.stage == VK_SHADER_STAGE_TASK_BIT_EXT && (create_info.nextStage & ~VK_SHADER_STAGE_MESH_BIT_EXT) > 0) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08435", device, create_info_loc.dot(Field::stage),
                             "is VK_SHADER_STAGE_TASK_BIT_EXT, but nextStage is %s.",
                             string_VkShaderStageFlags(create_info.nextStage).c_str());
        }
        if (create_info.stage == VK_SHADER_STAGE_MESH_BIT_EXT && (create_info.nextStage & ~VK_SHADER_STAGE_FRAGMENT_BIT) > 0) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08436", device, create_info_loc.dot(Field::stage),
                             "is VK_SHADER_STAGE_MESH_BIT_EXT, but nextStage is %s.",
                             string_VkShaderStageFlags(create_info.nextStage).c_str());
        }

        if ((create_info.flags & VK_SHADER_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT) != 0 &&
            enabled_features.subgroupSizeControl == VK_FALSE) {
            skip |= LogError(
                "VUID-VkShaderCreateInfoEXT-flags-09404", device, create_info_loc.dot(Field::flags),
                "contains VK_SHADER_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT, but subgroupSizeControl feature is not enabled.");
        }
        if ((create_info.flags & VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT) != 0 &&
            enabled_features.computeFullSubgroups == VK_FALSE) {
            skip |= LogError(
                "VUID-VkShaderCreateInfoEXT-flags-09405", device, create_info_loc.dot(Field::flags),
                "contains VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT, but computeFullSubgroups feature is not enabled.");
        }
        if ((create_info.flags & VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT) != 0 &&
            enabled_features.deviceGeneratedCommands == VK_FALSE) {
            skip |= LogError(
                " VUID-VkShaderCreateInfoEXT-flags-11005", device, create_info_loc.dot(Field::flags),
                "contains VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT, but deviceGeneratedCommands feature is not enabled.");
        }
    }

    if (linked_stage != invalid && non_linked_graphics_stage != invalid) {
        skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08402", device,
                         loc.dot(Field::pCreateInfos, linked_stage).dot(Field::flags),
                         "contains VK_SHADER_CREATE_LINK_STAGE_BIT_EXT, but pCreateInfos[%" PRIu32
                         "].stage is %s and does not have VK_SHADER_CREATE_LINK_STAGE_BIT_EXT.",
                         non_linked_graphics_stage, string_VkShaderStageFlagBits(pCreateInfos[non_linked_graphics_stage].stage));
    }
    if (linked_stage != invalid && non_linked_task_mesh_stage != invalid) {
        skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08403", device,
                         loc.dot(Field::pCreateInfos, linked_stage).dot(Field::flags),
                         "contains VK_SHADER_CREATE_LINK_STAGE_BIT_EXT, but pCreateInfos[%" PRIu32
                         "] stage is %s and does not have VK_SHADER_CREATE_LINK_STAGE_BIT_EXT.",
                         non_linked_task_mesh_stage, string_VkShaderStageFlagBits(pCreateInfos[non_linked_task_mesh_stage].stage));
    }
    if (linked_vert_stage != invalid && linked_task_mesh_stage != invalid) {
        skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08404", device,
                         loc.dot(Field::pCreateInfos, linked_vert_stage).dot(Field::stage),
                         "is %s and pCreateInfos[%" PRIu32 "].stage is %s, but both contain VK_SHADER_CREATE_LINK_STAGE_BIT_EXT.",
                         string_VkShaderStageFlagBits(pCreateInfos[linked_vert_stage].stage), linked_task_mesh_stage,
                         string_VkShaderStageFlagBits(pCreateInfos[linked_task_mesh_stage].stage));
    }
    if (linked_task_stage != invalid && linked_mesh_no_task_stage != invalid) {
        skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08405", device, loc.dot(Field::pCreateInfos, linked_task_stage),
                         "is a linked task shader, but pCreateInfos[%" PRIu32
                         "] is a linked mesh shader with VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT flag.",
                         linked_mesh_no_task_stage);
    }
    if (linked_spirv_index != invalid && linked_binary_index != invalid) {
        skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08411", device, loc.dot(Field::pCreateInfos, linked_spirv_index),
                         "is a linked shader with codeType VK_SHADER_CODE_TYPE_SPIRV_EXT, but pCreateInfos[%" PRIu32
                         "] is a linked shader with codeType VK_SHADER_CODE_TYPE_BINARY_EXT.",
                         linked_binary_index);
    }

    return skip;
}

bool CoreChecks::ValidateCreateShadersMesh(const VkShaderCreateInfoEXT& create_info, const spirv::Module& spirv,
                                           const Location& create_info_loc) const {
    bool skip = false;
    if (create_info.flags & VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT) return skip;
    if (spirv.static_data_.has_builtin_draw_index) {
        skip |= LogError(
            "VUID-vkCreateShadersEXT-pCreateInfos-09632", device, create_info_loc,
            "the mesh Shader Object being created uses DrawIndex (gl_DrawID) which will be an undefined value when reading.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                 const VkShaderCreateInfoEXT* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                                 VkShaderEXT* pShaders, const ErrorObject& error_obj) const {
    bool skip = false;

    // the spec clarifies that VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT works on VK_EXT_shader_object as well
    if (disabled[shader_validation]) {
        return skip; // VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT
    }

    if (enabled_features.shaderObject == VK_FALSE) {
        skip |=
            LogError("VUID-vkCreateShadersEXT-None-08400", device, error_obj.location, "the shaderObject feature was not enabled.");
    }

    skip |= ValidateCreateShadersLinking(createInfoCount, pCreateInfos, error_obj.location);

    uint32_t tesc_linked_subdivision = 0u;
    uint32_t tese_linked_subdivision = 0u;
    uint32_t tesc_linked_orientation = 0u;
    uint32_t tese_linked_orientation = 0u;
    bool tesc_linked_point_mode = false;
    bool tese_linked_point_mode = false;
    uint32_t tesc_linked_spacing = 0u;
    uint32_t tese_linked_spacing = 0u;
    bool has_compute = false;

    // Currently we don't provide a way for apps to supply their own cache for shader object
    // https://gitlab.khronos.org/vulkan/vulkan/-/issues/3570
    ValidationCache* cache = CastFromHandle<ValidationCache*>(core_validation_cache);

    for (uint32_t i = 0; i < createInfoCount; ++i) {
        const VkShaderCreateInfoEXT& create_info = pCreateInfos[i];
        if (create_info.codeType != VK_SHADER_CODE_TYPE_SPIRV_EXT) {
            continue;
        }
        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);

        spv_const_binary_t binary{static_cast<const uint32_t*>(create_info.pCode), create_info.codeSize / sizeof(uint32_t)};
        skip |= RunSpirvValidation(binary, create_info_loc, cache);

        const auto spirv = std::make_shared<spirv::Module>(create_info.codeSize, static_cast<const uint32_t*>(create_info.pCode));
        vku::safe_VkShaderCreateInfoEXT safe_create_info = vku::safe_VkShaderCreateInfoEXT(&pCreateInfos[i]);
        const ShaderStageState stage_state(nullptr, &safe_create_info, nullptr, spirv);
        skip |= ValidateShaderStage(stage_state, nullptr, create_info_loc);

        if (create_info.stage == VK_SHADER_STAGE_MESH_BIT_EXT) {
            skip |= ValidateCreateShadersMesh(create_info, *spirv, create_info_loc);
        }

        has_compute = create_info.stage == VK_SHADER_STAGE_COMPUTE_BIT;

        // Validate tessellation stages
        if (stage_state.entrypoint && (create_info.stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT ||
                                       create_info.stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)) {
            if (create_info.stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
                if (stage_state.entrypoint->execution_mode.tessellation_subdivision == 0) {
                    skip |= LogError("VUID-VkShaderCreateInfoEXT-codeType-08872", device, create_info_loc.dot(Field::stage),
                                     "is VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, but subdivision is not specified.");
                }
                if (stage_state.entrypoint->execution_mode.tessellation_orientation == 0) {
                    skip |= LogError("VUID-VkShaderCreateInfoEXT-codeType-08873", device, create_info_loc.dot(Field::stage),
                                     "is VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, but orientation is not specified.");
                }
                if (stage_state.entrypoint->execution_mode.tessellation_spacing == 0) {
                    skip |= LogError("VUID-VkShaderCreateInfoEXT-codeType-08874", device, create_info_loc.dot(Field::stage),
                                     "is VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, but spacing is not specified.");
                }
            }

            if (stage_state.entrypoint->execution_mode.output_vertices != vvl::kU32Max &&
                (stage_state.entrypoint->execution_mode.output_vertices == 0u ||
                 stage_state.entrypoint->execution_mode.output_vertices > phys_dev_props.limits.maxTessellationPatchSize)) {
                skip |= LogError(
                    "VUID-VkShaderCreateInfoEXT-pCode-08453", device, create_info_loc.dot(Field::pCode),
                    "is using patch size %" PRIu32 ", which is not between 1 and maxTessellationPatchSize (%" PRIu32 ").",
                    stage_state.entrypoint->execution_mode.output_vertices, phys_dev_props.limits.maxTessellationPatchSize);
            }

            if ((create_info.flags & VK_SHADER_CREATE_LINK_STAGE_BIT_EXT) != 0u) {
                if (create_info.stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) {
                    tesc_linked_subdivision = stage_state.entrypoint->execution_mode.tessellation_subdivision;
                    tesc_linked_orientation = stage_state.entrypoint->execution_mode.tessellation_orientation;
                    tesc_linked_point_mode = stage_state.entrypoint->execution_mode.flags & spirv::ExecutionModeSet::point_mode_bit;
                    tesc_linked_spacing = stage_state.entrypoint->execution_mode.tessellation_spacing;
                } else if (create_info.stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
                    tese_linked_subdivision = stage_state.entrypoint->execution_mode.tessellation_subdivision;
                    tese_linked_orientation = stage_state.entrypoint->execution_mode.tessellation_orientation;
                    tese_linked_point_mode = stage_state.entrypoint->execution_mode.flags & spirv::ExecutionModeSet::point_mode_bit;
                    tese_linked_spacing = stage_state.entrypoint->execution_mode.tessellation_spacing;
                }
            }
        }
    }

    if (tesc_linked_subdivision != 0 && tese_linked_subdivision != 0 && tesc_linked_subdivision != tese_linked_subdivision) {
        skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08867", device, error_obj.location,
                         "The subdivision specified in tessellation control shader (%s) does not match the subdivision in "
                         "tessellation evaluation shader (%s).",
                         string_SpvExecutionMode(tesc_linked_subdivision), string_SpvExecutionMode(tese_linked_subdivision));
    }
    if (tesc_linked_orientation != 0 && tese_linked_orientation != 0 && tesc_linked_orientation != tese_linked_orientation) {
        skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08868", device, error_obj.location,
                         "The orientation specified in tessellation control shader (%s) does not match the orientation in "
                         "tessellation evaluation shader (%s).",
                         string_SpvExecutionMode(tesc_linked_orientation), string_SpvExecutionMode(tese_linked_orientation));
    }
    if (tesc_linked_point_mode && !tese_linked_point_mode) {
        skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08869", device, error_obj.location,
                         "The tessellation control shader specifies execution mode point mode, but the tessellation evaluation "
                         "shader does not.");
    }
    if (tesc_linked_spacing != 0 && tese_linked_spacing != 0 && tesc_linked_spacing != tese_linked_spacing) {
        skip |= LogError("VUID-vkCreateShadersEXT-pCreateInfos-08870", device, error_obj.location,
                         "The spacing specified in tessellation control shader (%s) does not match the spacing in "
                         "tessellation evaluation shader (%s).",
                         string_SpvExecutionMode(tesc_linked_spacing), string_SpvExecutionMode(tese_linked_spacing));
    }

    const VkQueueFlags queue_flag = has_compute ? VK_QUEUE_COMPUTE_BIT : VK_QUEUE_GRAPHICS_BIT;
    if ((physical_device_state->supported_queues & queue_flag) == 0) {
        // Used instead of calling ValidateDeviceQueueSupport()
        const char* vuid = has_compute ? "VUID-vkCreateShadersEXT-stage-09670" : "VUID-vkCreateShadersEXT-stage-09671";
        skip |=
            LogError(vuid, device, error_obj.location, "device only supports (%s) but require %s.",
                     string_VkQueueFlags(physical_device_state->supported_queues).c_str(), string_VkQueueFlags(queue_flag).c_str());
    }

    return skip;
}

bool CoreChecks::PreCallValidateDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator,
                                                 const ErrorObject& error_obj) const {
    bool skip = false;

    if (enabled_features.shaderObject == VK_FALSE) {
        skip |=
            LogError("VUID-vkDestroyShaderEXT-None-08481", device, error_obj.location, "the shaderObject feature was not enabled.");
    }

    if (const auto shader_state = Get<vvl::ShaderObject>(shader)) {
        skip |= ValidateObjectNotInUse(shader_state.get(), error_obj.location.dot(Field::shader),
                                       "VUID-vkDestroyShaderEXT-shader-08482");
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount,
                                                  const VkShaderStageFlagBits* pStages, const VkShaderEXT* pShaders,
                                                  const ErrorObject& error_obj) const {
    bool skip = false;

    const auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);

    if (enabled_features.shaderObject == VK_FALSE) {
        skip |= LogError("VUID-vkCmdBindShadersEXT-None-08462", commandBuffer, error_obj.location,
                         "the shaderObject feature was not enabled.");
    }

    uint32_t vertexStageIndex = stageCount;
    uint32_t taskStageIndex = stageCount;
    uint32_t meshStageIndex = stageCount;
    for (uint32_t i = 0; i < stageCount; ++i) {
        const Location stage_loc = error_obj.location.dot(Field::pStages, i);
        const VkShaderStageFlagBits& stage = pStages[i];
        VkShaderEXT shader = pShaders ? pShaders[i] : VK_NULL_HANDLE;

        for (uint32_t j = i; j < stageCount; ++j) {
            if (i != j && stage == pStages[j]) {
                skip |= LogError("VUID-vkCmdBindShadersEXT-pStages-08463", commandBuffer, stage_loc,
                                 "and pStages[%" PRIu32 "] are both %s.", j, string_VkShaderStageFlagBits(stage));
            }
        }

        if (stage == VK_SHADER_STAGE_VERTEX_BIT && shader != VK_NULL_HANDLE) {
            vertexStageIndex = i;
        } else if (stage == VK_SHADER_STAGE_TASK_BIT_EXT && shader != VK_NULL_HANDLE) {
            taskStageIndex = i;
        } else if (stage == VK_SHADER_STAGE_MESH_BIT_EXT && shader != VK_NULL_HANDLE) {
            meshStageIndex = i;
        } else if (stage == VK_SHADER_STAGE_COMPUTE_BIT) {
            if ((cb_state->command_pool->queue_flags & VK_QUEUE_COMPUTE_BIT) == 0) {
                const LogObjectList objlist(commandBuffer, cb_state->command_pool->Handle());
                skip |=
                    LogError("VUID-vkCmdBindShadersEXT-pShaders-08476", objlist, stage_loc,
                             "is VK_SHADER_STAGE_COMPUTE_BIT, but the command pool the command buffer (%s) was allocated from "
                             "does not support compute operations (%s).",
                             FormatHandle(commandBuffer).c_str(), string_VkQueueFlags(cb_state->command_pool->queue_flags).c_str());
            }
        }
        if ((stage & (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                      VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)) >
            0) {
            if ((cb_state->command_pool->queue_flags & VK_QUEUE_GRAPHICS_BIT) == 0) {
                const LogObjectList objlist(commandBuffer, cb_state->command_pool->Handle());
                skip |= LogError("VUID-vkCmdBindShadersEXT-pShaders-08477", objlist, stage_loc,
                                 "is %s, but the command pool the command buffer %s was allocated from "
                                 "does not support graphics operations (%s).",
                                 string_VkShaderStageFlagBits(stage), FormatHandle(commandBuffer).c_str(),
                                 string_VkQueueFlags(cb_state->command_pool->queue_flags).c_str());
            }
        }
        if ((stage & (VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT)) > 0) {
            if ((cb_state->command_pool->queue_flags & VK_QUEUE_GRAPHICS_BIT) == 0) {
                const LogObjectList objlist(commandBuffer, cb_state->command_pool->Handle());
                skip |= LogError("VUID-vkCmdBindShadersEXT-pShaders-08478", objlist, stage_loc,
                                 "is %s, but the command pool the command buffer %s was allocated from "
                                 "does not support graphics operations (%s).",
                                 string_VkShaderStageFlagBits(stage), FormatHandle(commandBuffer).c_str(),
                                 string_VkQueueFlags(cb_state->command_pool->queue_flags).c_str());
            }
        }
        if (stage == VK_SHADER_STAGE_ALL_GRAPHICS || stage == VK_SHADER_STAGE_ALL) {
            skip |= LogError("VUID-vkCmdBindShadersEXT-pStages-08464", commandBuffer, stage_loc, "is %s.",
                             string_VkShaderStageFlagBits(stage));
        }
        if (stage & kShaderStageAllRayTracing) {
            skip |= LogError("VUID-vkCmdBindShadersEXT-pStages-08465", commandBuffer, stage_loc, "is %s.",
                             string_VkShaderStageFlagBits(stage));
        }
        if (stage == VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI) {
            skip |= LogError("VUID-vkCmdBindShadersEXT-pStages-08467", commandBuffer, stage_loc, "is %s.",
                             string_VkShaderStageFlagBits(stage));
        }
        if (stage == VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI) {
            skip |= LogError("VUID-vkCmdBindShadersEXT-pStages-08468", commandBuffer, stage_loc, "is %s.",
                             string_VkShaderStageFlagBits(stage));
        }
        if (shader != VK_NULL_HANDLE) {
            const auto shader_state = Get<vvl::ShaderObject>(shader);
            if (shader_state && shader_state->create_info.stage != stage) {
                const LogObjectList objlist(commandBuffer, shader);
                skip |=
                    LogError("VUID-vkCmdBindShadersEXT-pShaders-08469", objlist, stage_loc,
                             "is %s, but pShaders[%" PRIu32 "] was created with shader stage %s.",
                             string_VkShaderStageFlagBits(stage), i, string_VkShaderStageFlagBits(shader_state->create_info.stage));
            }
        }
    }

    if (vertexStageIndex != stageCount && taskStageIndex != stageCount) {
        skip |= LogError("VUID-vkCmdBindShadersEXT-pShaders-08470", commandBuffer, error_obj.location,
                         "pStages[%" PRIu32 "] is VK_SHADER_STAGE_VERTEX_BIT and pStages[%" PRIu32
                         "] is VK_SHADER_STAGE_TASK_BIT_EXT, but neither of pShaders[%" PRIu32 "] and pShaders[%" PRIu32
                         "] are VK_NULL_HANDLE.",
                         vertexStageIndex, taskStageIndex, vertexStageIndex, taskStageIndex);
    }
    if (vertexStageIndex != stageCount && meshStageIndex != stageCount) {
        skip |= LogError("VUID-vkCmdBindShadersEXT-pShaders-08471", commandBuffer, error_obj.location,
                         "pStages[%" PRIu32 "] is VK_SHADER_STAGE_VERTEX_BIT and pStages[%" PRIu32
                         "] is VK_SHADER_STAGE_MESH_BIT_EXT, but neither of pShaders[%" PRIu32 "] and pShaders[%" PRIu32
                         "] are VK_NULL_HANDLE.",
                         vertexStageIndex, meshStageIndex, vertexStageIndex, meshStageIndex);
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;

    if (enabled_features.shaderObject == VK_FALSE) {
        skip |= LogError("VUID-vkGetShaderBinaryDataEXT-None-08461", shader, error_obj.location,
                         "the shaderObject feature was not enabled.");
    }

    return skip;
}

bool CoreChecks::ValidateShaderObjectBoundShader(const LastBound& last_bound_state, const VkPipelineBindPoint bind_point,
                                                 const vvl::DrawDispatchVuid& vuid) const {
    bool skip = false;
    const vvl::CommandBuffer& cb_state = last_bound_state.cb_state;

    if (!last_bound_state.ValidShaderObjectCombination(bind_point, enabled_features)) {
        skip |= LogError(vuid.pipeline_or_shaders_bound_08607, cb_state.Handle(), vuid.loc(),
                         "A valid %s pipeline must be bound with vkCmdBindPipeline or shader objects with "
                         "vkCmdBindShadersEXT before calling this command.",
                         string_VkPipelineBindPoint(bind_point));
    }

    if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
        if (!last_bound_state.IsValidShaderOrNullBound(ShaderObjectStage::VERTEX)) {
            const bool tried_mesh = last_bound_state.IsValidShaderOrNullBound(ShaderObjectStage::MESH);
            skip |= LogError(
                vuid.vertex_shader_08684, cb_state.Handle(), vuid.loc(),
                "There is no graphics pipeline bound and vkCmdBindShadersEXT() was not called with stage "
                "VK_SHADER_STAGE_VERTEX_BIT and either VK_NULL_HANDLE or a valid VK_SHADER_STAGE_VERTEX_BIT shader%s.",
                tried_mesh ? " (Even if you are using a mesh shader, a VK_NULL_HANDLE must be bound to the vertex stage)" : "");
        }
        if (enabled_features.tessellationShader &&
            !last_bound_state.IsValidShaderOrNullBound(ShaderObjectStage::TESSELLATION_CONTROL)) {
            skip |= LogError(vuid.tessellation_control_shader_08685, cb_state.Handle(), vuid.loc(),
                             "There is no graphics pipeline bound and vkCmdBindShadersEXT() was not called with stage "
                             "VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT and either VK_NULL_HANDLE or a valid "
                             "VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT shader. (If the tessellationShader is enabled, the stage "
                             "needs to be provided)");
        }
        if (enabled_features.tessellationShader &&
            !last_bound_state.IsValidShaderOrNullBound(ShaderObjectStage::TESSELLATION_EVALUATION)) {
            skip |= LogError(vuid.tessellation_evaluation_shader_08686, cb_state.Handle(), vuid.loc(),
                             "There is no graphics pipeline bound and vkCmdBindShadersEXT() was not called with stage "
                             "VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT and either VK_NULL_HANDLE or a valid "
                             "VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT shader. (If the tessellationShader is enabled, the stage "
                             "needs to be provided)");
        }
        if (enabled_features.geometryShader && !last_bound_state.IsValidShaderOrNullBound(ShaderObjectStage::GEOMETRY)) {
            skip |= LogError(vuid.geometry_shader_08687, cb_state.Handle(), vuid.loc(),
                             "There is no graphics pipeline bound and vkCmdBindShadersEXT() was not called with stage "
                             "VK_SHADER_STAGE_GEOMETRY_BIT and either VK_NULL_HANDLE or a valid VK_SHADER_STAGE_GEOMETRY_BIT "
                             "shader. (If the geometryShader is enabled, the stage needs to be provided)");
        }
        if (!last_bound_state.IsValidShaderOrNullBound(ShaderObjectStage::FRAGMENT)) {
            skip |=
                LogError(vuid.fragment_shader_08688, cb_state.Handle(), vuid.loc(),
                         "There is no graphics pipeline bound and vkCmdBindShadersEXT() was not called with stage "
                         "VK_SHADER_STAGE_FRAGMENT_BIT and either VK_NULL_HANDLE or a valid VK_SHADER_STAGE_FRAGMENT_BIT shader.");
        }
        if (enabled_features.taskShader && !last_bound_state.IsValidShaderOrNullBound(ShaderObjectStage::TASK)) {
            skip |= LogError(vuid.task_shader_08689, cb_state.Handle(), vuid.loc(),
                             "There is no graphics pipeline bound and vkCmdBindShadersEXT() was not called with stage "
                             "VK_SHADER_STAGE_TASK_BIT and either VK_NULL_HANDLE or a valid VK_SHADER_STAGE_TASK_BIT shader. (If "
                             "the taskShader is enabled, the stage needs to be provided)");
        }
        if (enabled_features.meshShader && !last_bound_state.IsValidShaderOrNullBound(ShaderObjectStage::MESH)) {
            skip |= LogError(vuid.mesh_shader_08690, cb_state.Handle(), vuid.loc(),
                             "There is no graphics pipeline bound and vkCmdBindShadersEXT() was not called with stage "
                             "VK_SHADER_STAGE_MESH_BIT and either VK_NULL_HANDLE or a valid VK_SHADER_STAGE_MESH_BIT shader. (If "
                             "the meshShader is enabled, the stage needs to be provided)");
        }
    }

    return skip;
}

bool CoreChecks::ValidateDrawShaderObject(const LastBound& last_bound_state, const vvl::DrawDispatchVuid& vuid) const {
    bool skip = false;
    const vvl::CommandBuffer& cb_state = last_bound_state.cb_state;

    if (cb_state.active_render_pass && !cb_state.active_render_pass->UsesDynamicRendering()) {
        skip |= LogError(vuid.render_pass_began_08876, cb_state.Handle(), vuid.loc(),
                         "Shader objects must be used with dynamic rendering, but VkRenderPass %s is active.",
                         FormatHandle(cb_state.active_render_pass->Handle()).c_str());
    }

    skip |= ValidateDrawShaderObjectLinking(last_bound_state, vuid);
    skip |= ValidateDrawShaderObjectPushConstantAndLayout(last_bound_state, vuid);
    skip |= ValidateDrawShaderObjectMesh(last_bound_state, vuid);

    return skip;
}

bool CoreChecks::ValidateDrawShaderObjectLinking(const LastBound& last_bound_state, const vvl::DrawDispatchVuid& vuid) const {
    bool skip = false;
    const vvl::CommandBuffer& cb_state = last_bound_state.cb_state;
    const Location loc = vuid.loc();

    for (uint32_t i = 0; i < kShaderObjectStageCount; ++i) {
        if (i == static_cast<uint32_t>(ShaderObjectStage::COMPUTE) || !last_bound_state.shader_object_states[i]) {
            continue;
        }

        for (const auto& linked_shader : last_bound_state.shader_object_states[i]->linked_shaders) {
            bool found = false;
            for (uint32_t j = 0; j < kShaderObjectStageCount; ++j) {
                if (linked_shader == last_bound_state.GetShader(static_cast<ShaderObjectStage>(j))) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                const auto missing_Shader = Get<vvl::ShaderObject>(linked_shader);
                skip |= LogError(vuid.linked_shaders_08698, cb_state.Handle(), loc,
                                 "Shader %s (%s) was created with VK_SHADER_CREATE_LINK_STAGE_BIT_EXT, but the linked %s "
                                 "shader (%s) is not bound.",
                                 debug_report->FormatHandle(last_bound_state.GetShader(static_cast<ShaderObjectStage>(i))).c_str(),
                                 string_VkShaderStageFlagBits(last_bound_state.shader_object_states[i]->create_info.stage),
                                 debug_report->FormatHandle(linked_shader).c_str(),
                                 string_VkShaderStageFlagBits(missing_Shader->create_info.stage));
                break;
            }
        }
    }

    // In order of how stages are linked together
    const std::array graphics_stages = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                        VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                        VK_SHADER_STAGE_FRAGMENT_BIT};
    VkShaderStageFlagBits prev_stage = VK_SHADER_STAGE_ALL;
    VkShaderStageFlagBits next_stage = VK_SHADER_STAGE_ALL;
    const vvl::ShaderObject* producer = nullptr;
    const vvl::ShaderObject* consumer = nullptr;

    for (const auto stage : graphics_stages) {
        if (skip) break;
        consumer = last_bound_state.GetShaderState(VkShaderStageToShaderObjectStage(stage));
        if (!consumer) continue;
        if (next_stage != VK_SHADER_STAGE_ALL && consumer->create_info.stage != next_stage) {
            skip |= LogError(vuid.linked_shaders_08699, cb_state.Handle(), loc,
                             "Shaders %s and %s were created with VK_SHADER_CREATE_LINK_STAGE_BIT_EXT without intermediate "
                             "stage %s linked, but %s shader is bound.",
                             string_VkShaderStageFlagBits(prev_stage), string_VkShaderStageFlagBits(next_stage),
                             string_VkShaderStageFlagBits(stage), string_VkShaderStageFlagBits(stage));
            break;
        }

        next_stage = VK_SHADER_STAGE_ALL;
        if (!consumer->linked_shaders.empty()) {
            prev_stage = stage;
            for (const auto& linked_shader : consumer->linked_shaders) {
                const auto& linked_state = Get<vvl::ShaderObject>(linked_shader);
                if (linked_state && linked_state->create_info.stage == consumer->create_info.nextStage) {
                    next_stage = static_cast<VkShaderStageFlagBits>(consumer->create_info.nextStage);
                    break;
                }
            }
        }

        if (producer && consumer->spirv && producer->spirv && consumer->entrypoint && producer->entrypoint) {
            skip |= ValidateInterfaceBetweenStages(*producer->spirv, *producer->entrypoint, *consumer->spirv, *consumer->entrypoint,
                                                   loc);
        }
        producer = consumer;
    }

    return skip;
}

bool CoreChecks::ValidateDrawShaderObjectPushConstantAndLayout(const LastBound& last_bound_state,
                                                               const vvl::DrawDispatchVuid& vuid) const {
    bool skip = false;
    const vvl::CommandBuffer& cb_state = last_bound_state.cb_state;

    const vvl::ShaderObject* first = nullptr;
    for (const auto shader_state : last_bound_state.shader_object_states) {
        if (!shader_state || !shader_state->IsGraphicsShaderState()) {
            continue;
        }
        if (!first) {
            first = shader_state;
            continue;
        }
        bool push_constant_different =
            first->create_info.pushConstantRangeCount != shader_state->create_info.pushConstantRangeCount;
        if (!push_constant_different) {
            bool found = false;  // find duplicate push constant ranges
            for (uint32_t i = 0; i < shader_state->create_info.pushConstantRangeCount; ++i) {
                for (uint32_t j = 0; j < first->create_info.pushConstantRangeCount; ++j) {
                    if (shader_state->create_info.pPushConstantRanges[i] == first->create_info.pPushConstantRanges[j]) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    push_constant_different = true;
                    break;
                }
            }
        }
        if (push_constant_different) {
            skip |= LogError(vuid.shaders_push_constants_08878, cb_state.Handle(), vuid.loc(),
                             "Shaders %s and %s have different push constant ranges.",
                             string_VkShaderStageFlagBits(first->create_info.stage),
                             string_VkShaderStageFlagBits(shader_state->create_info.stage));
        }

        bool descriptor_layouts_different = first->create_info.setLayoutCount != shader_state->create_info.setLayoutCount;
        if (!descriptor_layouts_different) {
            bool found = false;  // find duplicate set layouts
            for (uint32_t i = 0; i < shader_state->create_info.setLayoutCount; ++i) {
                for (uint32_t j = 0; j < first->create_info.setLayoutCount; ++j) {
                    if (shader_state->create_info.pSetLayouts[i] == first->create_info.pSetLayouts[j]) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    descriptor_layouts_different = true;
                    break;
                }
            }
        }
        if (descriptor_layouts_different) {
            skip |= LogError(vuid.shaders_descriptor_layouts_08879, cb_state.Handle(), vuid.loc(),
                             "Shaders %s and %s have different descriptor set layouts.",
                             string_VkShaderStageFlagBits(first->create_info.stage),
                             string_VkShaderStageFlagBits(shader_state->create_info.stage));
        }
    }

    return skip;
}

bool CoreChecks::ValidateDrawShaderObjectMesh(const LastBound& last_bound_state, const vvl::DrawDispatchVuid& vuid) const {
    bool skip = false;
    const vvl::CommandBuffer& cb_state = last_bound_state.cb_state;

    const VkShaderEXT vertex_shader_handle = last_bound_state.GetShader(ShaderObjectStage::VERTEX);
    const VkShaderEXT task_shader_handle = last_bound_state.GetShader(ShaderObjectStage::TASK);
    const VkShaderEXT mesh_shader_handle = last_bound_state.GetShader(ShaderObjectStage::MESH);
    const bool has_vertex_shader = vertex_shader_handle != VK_NULL_HANDLE;
    const bool has_task_shader = task_shader_handle != VK_NULL_HANDLE;
    const bool has_mesh_shader = mesh_shader_handle != VK_NULL_HANDLE;

    const bool is_mesh_command =
        IsValueIn(vuid.function,
                  {Func::vkCmdDrawMeshTasksNV, Func::vkCmdDrawMeshTasksIndirectNV, Func::vkCmdDrawMeshTasksIndirectCountNV,
                   Func::vkCmdDrawMeshTasksEXT, Func::vkCmdDrawMeshTasksIndirectEXT, Func::vkCmdDrawMeshTasksIndirectCountEXT});

    if (has_task_shader || has_mesh_shader) {
        auto print_mesh_task = [this, has_task_shader, has_mesh_shader, mesh_shader_handle, task_shader_handle]() {
            std::stringstream msg;
            if (has_task_shader && has_mesh_shader) {
                msg << "Task shader (" << FormatHandle(task_shader_handle).c_str() << ") and mesh shader ("
                    << FormatHandle(mesh_shader_handle).c_str() << ") are";
            } else if (has_task_shader) {
                msg << "Task shader (" << FormatHandle(task_shader_handle).c_str() << ") is";
            } else {
                msg << "Mesh shader (" << FormatHandle(mesh_shader_handle).c_str() << ") is";
            }
            return msg.str();
        };

        if (!is_mesh_command) {
            skip |= LogError(vuid.draw_shaders_no_task_mesh_08885, cb_state.Handle(), vuid.loc(),
                             "%s bound, but this is not a vkCmdDrawMeshTasks* call.", print_mesh_task().c_str());
        }
        if (has_vertex_shader) {
            skip |= LogError(vuid.vert_task_mesh_shader_08696, cb_state.Handle(), vuid.loc(),
                             "Vertex shader (%s) is bound, but %s bound as well.", FormatHandle(mesh_shader_handle).c_str(),
                             print_mesh_task().c_str());
        }
    }

    if (enabled_features.taskShader || enabled_features.meshShader) {
        if (has_vertex_shader && has_mesh_shader) {
            skip |= LogError(vuid.vert_mesh_shader_08693, cb_state.Handle(), vuid.loc(),
                             "Both vertex shader (%s) and mesh shader (%s) are bound, but only %s should be bound (the other needs "
                             "to be set to VK_NULL_HANDLE).",
                             FormatHandle(vertex_shader_handle).c_str(), FormatHandle(mesh_shader_handle).c_str(),
                             is_mesh_command ? "mesh" : "vertex");
        } else if (!has_vertex_shader && !has_mesh_shader) {
            // TODO - should have a dedicated VU or rework 08693
            skip |= LogError(vuid.vert_mesh_shader_08693, cb_state.Handle(), vuid.loc(),
                             "Neither vertex shader nor mesh shader are bound (for %s you need %s)", String(vuid.function),
                             is_mesh_command ? "mesh" : "vertex");
        }
    }

    if (enabled_features.taskShader && enabled_features.meshShader && is_mesh_command && has_mesh_shader) {
        if (const auto shader_object = last_bound_state.GetShaderState(ShaderObjectStage::MESH)) {
            const bool no_task_shader_flag = (shader_object->create_info.flags & VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT) != 0;

            if (!no_task_shader_flag && !has_task_shader) {
                skip |= LogError(
                    vuid.task_mesh_shader_08694, cb_state.Handle(), vuid.loc(),
                    "Mesh shader (%s) was created without VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT, but no task shader is bound.",
                    FormatHandle(mesh_shader_handle).c_str());
            } else if (no_task_shader_flag && has_task_shader) {
                skip |= LogError(
                    vuid.task_mesh_shader_08695, cb_state.Handle(), vuid.loc(),
                    "Mesh shader (%s) was created with VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT, but a task shader (%s) is bound.",
                    FormatHandle(mesh_shader_handle).c_str(), FormatHandle(task_shader_handle).c_str());
            }
        }
    }

    if (is_mesh_command) {
        const VkShaderEXT tesc_shader_handle = last_bound_state.GetShader(ShaderObjectStage::TESSELLATION_CONTROL);
        const VkShaderEXT tese_shader_handle = last_bound_state.GetShader(ShaderObjectStage::TESSELLATION_EVALUATION);
        const VkShaderEXT geom_shader_handle = last_bound_state.GetShader(ShaderObjectStage::GEOMETRY);
        const bool has_tesc_shader = tesc_shader_handle != VK_NULL_HANDLE;
        const bool has_tese_shader = tese_shader_handle != VK_NULL_HANDLE;
        const bool has_geom_shader = geom_shader_handle != VK_NULL_HANDLE;
        if (has_vertex_shader || has_tesc_shader || has_tese_shader || has_geom_shader) {
            std::stringstream msg;
            if (has_vertex_shader) msg << "Vertex shader: " << FormatHandle(vertex_shader_handle) << '\n';
            if (has_tese_shader) msg << "Tessellation Eval shader: " << FormatHandle(tese_shader_handle) << '\n';
            if (has_tesc_shader) msg << "Tessellation Control shader: " << FormatHandle(tesc_shader_handle) << '\n';
            if (has_geom_shader) msg << "Geometry shader: " << FormatHandle(geom_shader_handle) << '\n';
            // VU being added to https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/7125
            skip |=
                LogError("UNASSIGNED-vkCmdDrawMeshTasks-pre-raster-stages", cb_state.Handle(), vuid.loc(),
                         "Calling a mesh draw call, but the following stages are bound, but need to be bound as VK_NULL_HANDLE\n%s",
                         msg.str().c_str());
        }
    }
    return skip;
}
