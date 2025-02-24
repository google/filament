/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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

#include <cassert>
#include <cinttypes>
#include <spirv/unified1/spirv.hpp>
#include <sstream>
#include <string>
#include <vector>

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include "core_checks/cc_vuid_maps.h"
#include "core_validation.h"
#include "generated/spirv_grammar_helper.h"
#include "state_tracker/shader_instruction.h"
#include "state_tracker/shader_stage_state.h"
#include "utils/hash_util.h"
#include "chassis/chassis_modification_state.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/render_pass_state.h"
#include "spirv-tools/optimizer.hpp"
#include "utils/vk_layer_utils.h"

// Validate use of input attachments against subpass structure
bool CoreChecks::ValidateShaderInputAttachment(const spirv::Module &module_state, const vvl::Pipeline &pipeline,
                                               const spirv::ResourceInterfaceVariable &variable, const Location &loc) const {
    bool skip = false;
    assert(variable.is_input_attachment);

    const auto &rp_state = pipeline.RenderPassState();
    // Dynamic Rendering guards this with VUID 06061
    if (!rp_state || rp_state->UsesDynamicRendering()) {
        return skip;
    }

    for (uint32_t i = 0; i < variable.input_attachment_index_read.size(); i++) {
        // If the attachment is not read from, nothing to validate
        if (!variable.input_attachment_index_read[i]) {
            continue;
        }
        const auto rpci = rp_state->create_info.ptr();
        const uint32_t subpass = pipeline.Subpass();
        const auto subpass_description = rpci->pSubpasses[subpass];
        const auto input_attachments = subpass_description.pInputAttachments;
        // offsets by the InputAttachmentIndex decoration
        const uint32_t input_attachment_index = variable.decorations.input_attachment_index_start + i;

        // Same error, but provide more useful message 'how' VK_ATTACHMENT_UNUSED is derived
        if (!input_attachments) {
            const LogObjectList objlist(module_state.handle(), rp_state->Handle());
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06038", objlist, loc,
                             "SPIR-V consumes input attachment index %" PRIu32 " but pSubpasses[%" PRIu32
                             "].pInputAttachments is NULL.",
                             input_attachment_index, subpass);
        } else if (input_attachment_index >= subpass_description.inputAttachmentCount) {
            const LogObjectList objlist(module_state.handle(), rp_state->Handle());
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06038", objlist, loc,
                             "SPIR-V consumes input attachment index %" PRIu32 " but that is not less than the pSubpasses[%" PRIu32
                             "].inputAttachmentCount (%" PRIu32 ").",
                             input_attachment_index, subpass, subpass_description.inputAttachmentCount);
        } else if (input_attachments[input_attachment_index].attachment == VK_ATTACHMENT_UNUSED) {
            const LogObjectList objlist(module_state.handle(), rp_state->Handle());
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06038", objlist, loc,
                             "SPIR-V consumes input attachment index %" PRIu32 " but pSubpasses[%" PRIu32
                             "].pInputAttachments[%" PRIu32 "].attachment is VK_ATTACHMENT_UNUSED.",
                             input_attachment_index, subpass, input_attachment_index);
        }
    }

    return skip;
}

bool CoreChecks::ValidateConservativeRasterization(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                                   const spirv::StatelessData &stateless_data, const Location &loc) const {
    bool skip = false;

    // only new to validate if property is not enabled
    if (phys_dev_ext_props.conservative_rasterization_props.conservativeRasterizationPostDepthCoverage) {
        return skip;
    }

    if (stateless_data.has_builtin_fully_covered &&
        entrypoint.execution_mode.Has(spirv::ExecutionModeSet::post_depth_coverage_bit)) {
        skip |= LogError("VUID-FullyCoveredEXT-conservativeRasterizationPostDepthCoverage-04235", device, loc,
                         "SPIR-V (Fragment stage) has a\nOpExecutionMode EarlyFragmentTests\nOpDecorate BuiltIn "
                         "FullyCoveredEXT\nbut conservativeRasterizationPostDepthCoverage was not enabled.");
    }

    return skip;
}

bool CoreChecks::ValidatePushConstantUsage(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                           const vvl::Pipeline *pipeline, const ShaderStageState &stage_state,
                                           const Location &loc) const {
    bool skip = false;

    // TODO - Workaround for https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5911
    if (module_state.static_data_.has_specialization_constants) {
        return skip;
    }

    const VkShaderStageFlagBits stage = entrypoint.stage;
    const auto push_constant_variable = entrypoint.push_constant_variable;
    if (!push_constant_variable) {
        return skip;
    }

    PushConstantRangesId shader_object_push_constant_ranges_id;
    std::vector<VkPushConstantRange> const *push_constant_ranges;
    std::string stage_vuid;
    std::string range_vuid;
    if (pipeline) {
        push_constant_ranges = pipeline->PipelineLayoutState()->push_constant_ranges_layout.get();

        switch (pipeline->GetCreateInfoSType()) {
            case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO:
                stage_vuid = "VUID-VkGraphicsPipelineCreateInfo-layout-07987";
                range_vuid = "VUID-VkGraphicsPipelineCreateInfo-layout-10069";
                break;
            case VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO:
                stage_vuid = "VUID-VkComputePipelineCreateInfo-layout-07987";
                range_vuid = "VUID-VkComputePipelineCreateInfo-layout-10069";
                break;
            case VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR:
                stage_vuid = "VUID-VkRayTracingPipelineCreateInfoKHR-layout-07987";
                range_vuid = "VUID-VkRayTracingPipelineCreateInfoKHR-layout-10069";
                break;
            case VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV:
                stage_vuid = "VUID-VkRayTracingPipelineCreateInfoNV-layout-07987";
                range_vuid = "VUID-VkRayTracingPipelineCreateInfoNV-layout-10069";
                break;
            default:
                assert(false);
                break;
        }
    } else {
        shader_object_push_constant_ranges_id = GetCanonicalId(stage_state.shader_object_create_info->pushConstantRangeCount,
                                                          stage_state.shader_object_create_info->pPushConstantRanges);
        push_constant_ranges = shader_object_push_constant_ranges_id.get();
        stage_vuid = "VUID-VkShaderCreateInfoEXT-codeType-10064";
        range_vuid = "VUID-VkShaderCreateInfoEXT-codeType-10065";
    }

    bool found_stage = false;
    for (auto const &range : *push_constant_ranges) {
        if (range.stageFlags & stage) {
            found_stage = true;
            const uint32_t range_end = range.offset + range.size;
            const uint32_t push_constant_end = push_constant_variable->offset + push_constant_variable->size;
            // spec: "If a push constant block is declared in a shader"
            // Is checked regardless if element in Block is not statically used
            if ((push_constant_variable->offset < range.offset) | (push_constant_end > range_end)) {
                LogObjectList objlist(module_state.handle());
                if (pipeline) {
                    objlist.add(pipeline->PipelineLayoutState()->Handle());
                }
                skip |= LogError(range_vuid, objlist, loc,
                                 "SPIR-V (%s) has a push constant buffer Block with range [%" PRIu32 ", %" PRIu32
                                 "] which outside the VkPushConstantRange of [%" PRIu32 ", %" PRIu32 "].",
                                 string_VkShaderStageFlags(stage).c_str(), push_constant_variable->offset, push_constant_end,
                                 range.offset, range_end);
                break;
            }
        }
    }

    if (!found_stage) {
        LogObjectList objlist(module_state.handle());
        std::string msg = "";
        if (pipeline) {
            objlist.add(pipeline->PipelineLayoutState()->Handle());
            msg = FormatHandle(pipeline->PipelineLayoutState()->Handle());
        } else {
            msg = "VkShaderCreateInfoEXT::pPushConstantRanges";
        }
        skip |= LogError(stage_vuid, objlist, loc, "SPIR-V (%s) Push constant are used, but %s doesn't set %s.",
                         string_VkShaderStageFlags(stage).c_str(), msg.c_str(), string_VkShaderStageFlags(stage).c_str());
    }
    return skip;
}

static void TypeToDescriptorTypeSet(const spirv::Module &module_state, uint32_t type_id,
                                    vvl::unordered_set<uint32_t> &descriptor_type_set) {
    const spirv::Instruction *type = module_state.FindDef(type_id);
    bool is_storage_buffer = false;

    // Strip off any array or ptrs. Where we remove array levels, adjust the  descriptor count for each dimension.
    while (type->IsArray() || type->Opcode() == spv::OpTypePointer) {
        if (type->Opcode() == spv::OpTypeRuntimeArray) {
            type = module_state.FindDef(type->Word(2));
        } else if (type->Opcode() == spv::OpTypeArray) {
            type = module_state.FindDef(type->Word(2));
        } else {
            if (type->StorageClass() == spv::StorageClassStorageBuffer) {
                is_storage_buffer = true;
            }
            type = module_state.FindDef(type->Word(3));
        }
    }

    switch (type->Opcode()) {
        case spv::OpTypeStruct: {
            for (const spirv::Instruction *insn : module_state.static_data_.decoration_inst) {
                if (insn->Word(1) == type->Word(1)) {
                    if (insn->Word(2) == spv::DecorationBlock) {
                        if (is_storage_buffer) {
                            descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
                            descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
                        } else {
                            descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
                            descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
                            descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK);
                        }
                    } else if (insn->Word(2) == spv::DecorationBufferBlock) {
                        descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
                        descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
                    }
                    break;
                }
            }
            return;
        }

        case spv::OpTypeSampler:
            descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_SAMPLER);
            descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            return;

        case spv::OpTypeSampledImage: {
            // Slight relaxation for some GLSL historical madness: samplerBuffer doesn't really have a sampler, and a texel
            // buffer descriptor doesn't really provide one. Allow this slight mismatch.
            const spirv::Instruction *image_type = module_state.FindDef(type->Word(2));
            auto dim = image_type->Word(3);
            auto sampled = image_type->Word(7);
            if (dim == spv::DimBuffer && sampled == 1) {
                descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
            } else {
                descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            }
            return;
        }
        case spv::OpTypeImage: {
            // Many descriptor types backing image types-- depends on dimension and whether the image will be used with a sampler.
            // SPIRV for Vulkan requires that sampled be 1 or 2 -- leaving the decision to runtime is unacceptable.
            auto dim = type->Word(3);
            auto sampled = type->Word(7);

            if (dim == spv::DimSubpassData) {
                descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
            } else if (dim == spv::DimBuffer) {
                if (sampled == 1) {
                    descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
                } else {
                    descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
                }
            } else if (sampled == 1) {
                descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
                descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            } else {
                descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
            }
            return;
        }

        // The OpType are alias, but the Descriptor Types are different
        case spv::OpTypeAccelerationStructureKHR:
            if (module_state.HasCapability(spv::CapabilityRayTracingNV)) {
                descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV);
            } else {
                descriptor_type_set.insert(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR);
            }
            return;

        default:
            // We shouldn't really see any other junk types -- but if we do, they're a mismatch.
            return;  // Matches nothing
    }
}

static std::string string_DescriptorTypeSet(const vvl::unordered_set<uint32_t> &descriptor_type_set) {
    std::stringstream ss;
    for (auto it = descriptor_type_set.begin(); it != descriptor_type_set.end(); ++it) {
        if (ss.tellp()) ss << " or ";
        ss << string_VkDescriptorType(VkDescriptorType(*it));
    }
    return ss.str();
}

bool CoreChecks::ValidateShaderStageGroupNonUniform(const spirv::Module &module_state, const spirv::StatelessData &stateless_data,
                                                    VkShaderStageFlagBits stage, const Location &loc) const {
    bool skip = false;

    // Check anything using a group operation (which currently is only OpGroupNonUnifrom* operations)
    for (const spirv::Instruction *group_inst : stateless_data.group_inst) {
        const spirv::Instruction &insn = *group_inst;
        // Check the quad operations.
        if ((insn.Opcode() == spv::OpGroupNonUniformQuadBroadcast) || (insn.Opcode() == spv::OpGroupNonUniformQuadSwap)) {
            if ((stage != VK_SHADER_STAGE_FRAGMENT_BIT) && (stage != VK_SHADER_STAGE_COMPUTE_BIT)) {
                if (!phys_dev_props_core11.subgroupQuadOperationsInAllStages) {
                    skip |= LogError("VUID-RuntimeSpirv-None-06342", module_state.handle(), loc,
                                     "Can't use for stage %s because VkPhysicalDeviceSubgroupProperties::quadOperationsInAllStages "
                                     "is not supported on this VkDevice",
                                     string_VkShaderStageFlagBits(stage));
                }
            }
        }

        uint32_t scope_type = spv::ScopeMax;
        if (insn.Opcode() == spv::OpGroupNonUniformPartitionNV) {
            // OpGroupNonUniformPartitionNV always assumed subgroup as missing operand
            scope_type = spv::ScopeSubgroup;
        } else {
            // "All <id> used for Scope <id> must be of an OpConstant"
            const spirv::Instruction *scope_id = module_state.FindDef(insn.Word(3));
            scope_type = scope_id->Word(3);
        }

        if (scope_type == spv::ScopeSubgroup) {
            // "Group operations with subgroup scope" must have stage support
            const VkSubgroupFeatureFlags supported_stages = phys_dev_props_core11.subgroupSupportedStages;
            if ((supported_stages & stage) == 0) {
                skip |= LogError("VUID-RuntimeSpirv-None-06343", module_state.handle(), loc,
                                 "%s is not supported in subgroupSupportedStages (%s).", string_VkShaderStageFlagBits(stage),
                                 string_VkShaderStageFlags(supported_stages).c_str());
            }
        }

        if (!enabled_features.shaderSubgroupExtendedTypes) {
            const spirv::Instruction *type = module_state.FindDef(insn.Word(1));

            if (type->Opcode() == spv::OpTypeVector) {
                // Get the element type
                type = module_state.FindDef(type->Word(2));
            }

            if (type->Opcode() != spv::OpTypeBool) {
                // Both OpTypeInt and OpTypeFloat the width is in the 2nd word.
                const uint32_t width = type->Word(2);

                if ((type->Opcode() == spv::OpTypeFloat && width == 16) ||
                    (type->Opcode() == spv::OpTypeInt && (width == 8 || width == 16 || width == 64))) {
                    if (!enabled_features.shaderSubgroupExtendedTypes) {
                        skip |= LogError(
                            "VUID-RuntimeSpirv-None-06275", module_state.handle(), loc,
                            "VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures::shaderSubgroupExtendedTypes was not enabled");
                    }
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateMemoryScope(const spirv::Module &module_state, const spirv::Instruction &insn, const Location &loc) const {
    bool skip = false;

    const auto &entry = OpcodeMemoryScopePosition(insn.Opcode());
    if (entry > 0) {
        const uint32_t scope_id = insn.Word(entry);
        const spirv::Instruction *scope_def = module_state.GetConstantDef(scope_id);
        if (scope_def) {
            const spv::Scope scope_type = spv::Scope(scope_def->GetConstantValue());
            if (enabled_features.vulkanMemoryModel && !enabled_features.vulkanMemoryModelDeviceScope &&
                scope_type == spv::Scope::ScopeDevice) {
                skip |=
                    LogError("VUID-RuntimeSpirv-vulkanMemoryModel-06265", module_state.handle(), loc,
                             "SPIR-V uses Device memory scope, but the vulkanMemoryModelDeviceScope feature was not enabled.\n%s\n",
                             module_state.DescribeInstruction(insn).c_str());
            } else if (!enabled_features.vulkanMemoryModel && scope_type == spv::Scope::ScopeQueueFamily) {
                skip |= LogError("VUID-RuntimeSpirv-vulkanMemoryModel-06266", module_state.handle(), loc,
                                 "SPIR-V uses QueueFamily memory scope, but the vulkanMemoryModel feature was not enabled.\n%s\n",
                                 module_state.DescribeInstruction(insn).c_str());
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateSubgroupRotateClustered(const spirv::Module &module_state, const spirv::Instruction &insn,
                                                 const Location &loc) const {
    bool skip = false;
    if (!enabled_features.shaderSubgroupRotateClustered && insn.Opcode() == spv::OpGroupNonUniformRotateKHR && insn.Length() == 7) {
        skip |= LogError("VUID-RuntimeSpirv-shaderSubgroupRotateClustered-09566", module_state.handle(), loc,
                         "SPIR-V uses ClusterSize operand, but the shaderSubgroupRotateClustered feature was not enabled.\n%s\n",
                         module_state.DescribeInstruction(insn).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateShaderStorageImageFormatsVariables(const spirv::Module &module_state, const spirv::Instruction &insn,
                                                            const Location &loc) const {
    bool skip = false;
    // Go through all variables for images and check decorations
    // Note: Tried to move to ResourceInterfaceVariable but the issue is the variables don't need to be accessed in the entrypoint
    // to trigger the error.
    assert(insn.Opcode() == spv::OpVariable);
    // spirv-val validates this is an OpTypePointer
    const spirv::Instruction *pointer_def = module_state.FindDef(insn.Word(1));
    if (pointer_def->Word(2) != spv::StorageClassUniformConstant) {
        return skip;  // Vulkan Spec says storage image must be UniformConstant
    }
    const spirv::Instruction *type_def = module_state.FindDef(pointer_def->Word(3));

    // Unpack an optional level of arraying
    if (type_def && type_def->IsArray()) {
        type_def = module_state.FindDef(type_def->Word(2));
    }

    if (type_def && type_def->Opcode() == spv::OpTypeImage) {
        // Only check if the Image Dim operand is not SubpassData
        const uint32_t dim = type_def->Word(3);
        // Only check storage images
        const uint32_t sampled = type_def->Word(7);
        const uint32_t image_format = type_def->Word(8);
        if ((dim == spv::DimSubpassData) || (sampled != 2) || (image_format != spv::ImageFormatUnknown)) {
            return skip;
        }

        const uint32_t var_id = insn.Word(2);
        const auto decorations = module_state.GetDecorationSet(var_id);

        if (!enabled_features.shaderStorageImageReadWithoutFormat && !decorations.Has(spirv::DecorationSet::nonreadable_bit)) {
            skip |=
                LogError("VUID-RuntimeSpirv-apiVersion-07955", module_state.handle(), loc,
                         "SPIR-V variable\n%s\nhas an Image\n%s\nwith Unknown "
                         "format and is not decorated with NonReadable, but shaderStorageImageReadWithoutFormat is not supported.",
                         module_state.FindDef(var_id)->Describe().c_str(), type_def->Describe().c_str());
        }

        if (!enabled_features.shaderStorageImageWriteWithoutFormat && !decorations.Has(spirv::DecorationSet::nonwritable_bit)) {
            skip |= LogError(
                "VUID-RuntimeSpirv-apiVersion-07954", module_state.handle(), loc,
                "SPIR-V variable\n%s\nhas an Image\n%s\nwith "
                "Unknown format and is not decorated with NonWritable, but shaderStorageImageWriteWithoutFormat is not supported.",
                module_state.FindDef(var_id)->Describe().c_str(), type_def->Describe().c_str());
        }
    }

    return skip;
}

// Map SPIR-V type to VK_COMPONENT_TYPE enum
VkComponentTypeKHR GetComponentType(const spirv::Instruction *insn, bool is_signed_int) {
    switch (insn->Opcode()) {
        case spv::OpTypeInt:
            switch (insn->Word(2)) {
                case 8:
                    return is_signed_int ? VK_COMPONENT_TYPE_SINT8_KHR : VK_COMPONENT_TYPE_UINT8_KHR;
                case 16:
                    return is_signed_int ? VK_COMPONENT_TYPE_SINT16_KHR : VK_COMPONENT_TYPE_UINT16_KHR;
                case 32:
                    return is_signed_int ? VK_COMPONENT_TYPE_SINT32_KHR : VK_COMPONENT_TYPE_UINT32_KHR;
                case 64:
                    return is_signed_int ? VK_COMPONENT_TYPE_SINT64_KHR : VK_COMPONENT_TYPE_UINT64_KHR;
                default:
                    return VK_COMPONENT_TYPE_MAX_ENUM_KHR;
            }
        case spv::OpTypeFloat:
            switch (insn->Word(2)) {
                case 16:
                    return VK_COMPONENT_TYPE_FLOAT16_KHR;
                case 32:
                    return VK_COMPONENT_TYPE_FLOAT32_KHR;
                case 64:
                    return VK_COMPONENT_TYPE_FLOAT64_KHR;
                default:
                    return VK_COMPONENT_TYPE_MAX_ENUM_KHR;
            }
        default:
            return VK_COMPONENT_TYPE_MAX_ENUM_KHR;
    }
}

static bool IsSignedIntEnum(const VkComponentTypeKHR component_type) {
    switch (component_type) {
        case VK_COMPONENT_TYPE_SINT8_KHR:
        case VK_COMPONENT_TYPE_SINT16_KHR:
        case VK_COMPONENT_TYPE_SINT32_KHR:
        case VK_COMPONENT_TYPE_SINT64_KHR:
            return true;
        default:
            return false;
    }
}

// Validate SPV_KHR_cooperative_matrix (and SPV_NV_cooperative_matrix) behavior that can't be statically validated in SPIRV-Tools
// (e.g. due to specialization constant usage).
bool CoreChecks::ValidateCooperativeMatrix(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                           const ShaderStageState &stage_state, const uint32_t local_size_x,
                                           const uint32_t local_size_y, const uint32_t local_size_z, const Location &loc) const {
    bool skip = false;
    const auto workgroup_size = local_size_x * local_size_y * local_size_z;

    const auto &IsSignedIntType = [&module_state](const uint32_t type_id) {
        const spirv::Instruction *type = module_state.FindDef(type_id);
        if (type->Opcode() == spv::OpTypeCooperativeMatrixKHR || type->Opcode() == spv::OpTypeCooperativeMatrixNV) {
            type = module_state.FindDef(type->Word(2));
        }
        return type->Opcode() == spv::OpTypeInt && type->Word(3) != 0;
    };

    struct CoopMatType {
        VkScopeKHR scope;
        uint32_t rows;
        uint32_t cols;
        VkComponentTypeKHR component_type;
        uint32_t use;
        bool all_constant;

        CoopMatType(uint32_t id, const spirv::Module &module_state, const ShaderStageState &stage_state, bool is_signed_int) {
            const spirv::Instruction *insn = module_state.FindDef(id);
            const spirv::Instruction *component_type_insn = module_state.FindDef(insn->Word(2));
            const spirv::Instruction *scope_insn = module_state.FindDef(insn->Word(3));
            const spirv::Instruction *rows_insn = module_state.FindDef(insn->Word(4));
            const spirv::Instruction *cols_insn = module_state.FindDef(insn->Word(5));

            all_constant = true;
            uint32_t tmp_scope = 0;  // TODO - Remove GetIntConstantValue
            if (!stage_state.GetInt32ConstantValue(*scope_insn, &tmp_scope)) {
                all_constant = false;
            }
            scope = VkScopeKHR(tmp_scope);
            if (!stage_state.GetInt32ConstantValue(*rows_insn, &rows)) {
                all_constant = false;
            }
            if (!stage_state.GetInt32ConstantValue(*cols_insn, &cols)) {
                all_constant = false;
            }
            component_type = GetComponentType(component_type_insn, is_signed_int);

            if (insn->Opcode() == spv::OpTypeCooperativeMatrixKHR) {
                const spirv::Instruction *use_insn = module_state.FindDef(insn->Word(6));
                if (!stage_state.GetInt32ConstantValue(*use_insn, &use)) {
                    all_constant = false;
                }
            }
        }

        std::string Describe() {
            std::ostringstream ss;
            ss << "rows: " << rows << ", cols: " << cols << ", scope: " << string_VkScopeKHR(scope)
               << ", type: " << string_VkComponentTypeKHR(component_type) << ", use: " << use;
            return ss.str();
        }
    };

    if (module_state.HasCapability(spv::CapabilityCooperativeMatrixKHR)) {
        if (!(entrypoint.stage & phys_dev_ext_props.cooperative_matrix_props_khr.cooperativeMatrixSupportedStages)) {
            skip |=
                LogError("VUID-RuntimeSpirv-cooperativeMatrixSupportedStages-08985", module_state.handle(), loc,
                         "SPIR-V contains OpTypeCooperativeMatrixKHR used in shader stage %s but is not in "
                         "cooperativeMatrixSupportedStages (%s)",
                         string_VkShaderStageFlagBits(entrypoint.stage),
                         string_VkShaderStageFlags(phys_dev_ext_props.cooperative_matrix_props_khr.cooperativeMatrixSupportedStages)
                             .c_str());
        }
    } else if (module_state.HasCapability(spv::CapabilityCooperativeMatrixNV)) {
        if (!(entrypoint.stage & phys_dev_ext_props.cooperative_matrix_props.cooperativeMatrixSupportedStages)) {
            skip |= LogError(
                "VUID-RuntimeSpirv-OpTypeCooperativeMatrixNV-06322", module_state.handle(), loc,
                "SPIR-V contains OpTypeCooperativeMatrixNV used in shader stage %s but is not in cooperativeMatrixSupportedStages "
                "(%s)",
                string_VkShaderStageFlagBits(entrypoint.stage),
                string_VkShaderStageFlags(phys_dev_ext_props.cooperative_matrix_props.cooperativeMatrixSupportedStages).c_str());
        }
    } else {
        return skip;  // If the capability isn't enabled, don't bother with the rest of this function.
    }

    // Map SPIR-V result ID to the ID of its type.
    // TODO - Should have more robust way in ModuleState to find the type
    vvl::unordered_map<uint32_t, uint32_t> id_to_type_id;
    for (const spirv::Instruction &insn : module_state.GetInstructions()) {
        if (OpcodeHasType(insn.Opcode()) && OpcodeHasResult(insn.Opcode())) {
            id_to_type_id[insn.Word(2)] = insn.Word(1);
        }
    }

    auto print_properties = [this]() {
        std::ostringstream ss;
        for (uint32_t i = 0; i < cooperative_matrix_properties_khr.size(); ++i) {
            const auto &prop = cooperative_matrix_properties_khr[i];
            ss << "[" << i << "] MSize = " << prop.MSize << " | NSize = " << prop.NSize << " | KSize = " << prop.KSize
               << " | AType = " << string_VkComponentTypeKHR(prop.AType) << " | BType = " << string_VkComponentTypeKHR(prop.BType)
               << " | CType = " << string_VkComponentTypeKHR(prop.CType)
               << " | ResultType = " << string_VkComponentTypeKHR(prop.ResultType) << " | scope = " << string_VkScopeKHR(prop.scope)
               << '\n';
        }
        return ss.str();
    };

    auto print_flexible_properties = [this]() {
        std::ostringstream ss;
        for (uint32_t i = 0; i < cooperative_matrix_flexible_dimensions_properties.size(); ++i) {
            const auto &prop = cooperative_matrix_flexible_dimensions_properties[i];
            ss << "[" << i << "] MGranularity = " << prop.MGranularity << " | NGranularity = " << prop.NGranularity
               << " | KGranularity = " << prop.KGranularity << " | AType = " << string_VkComponentTypeKHR(prop.AType)
               << " | BType = " << string_VkComponentTypeKHR(prop.BType) << " | CType = " << string_VkComponentTypeKHR(prop.CType)
               << " | ResultType = " << string_VkComponentTypeKHR(prop.ResultType) << " | scope = " << string_VkScopeKHR(prop.scope)
               << " | workgroupInvocations = " << prop.workgroupInvocations << '\n';
        }
        return ss.str();
    };

    for (const spirv::Instruction *cooperative_matrix_inst : module_state.static_data_.cooperative_matrix_inst) {
        const spirv::Instruction &insn = *cooperative_matrix_inst;
        switch (insn.Opcode()) {
            case spv::OpTypeCooperativeMatrixKHR: {
                CoopMatType m(insn.Word(1), module_state, stage_state, IsSignedIntType(insn.Word(2)));

                if ((entrypoint.stage & VK_SHADER_STAGE_COMPUTE_BIT) != 0) {
                    if (SafeModulo(local_size_x, phys_dev_props_core11.subgroupSize) != 0) {
                        const auto vuid_string = m.scope == VK_SCOPE_SUBGROUP_KHR
                                                     ? "VUID-VkPipelineShaderStageCreateInfo-module-08987"
                                                     : "VUID-VkPipelineShaderStageCreateInfo-module-10169";
                        skip |= LogError(vuid_string, module_state.handle(), loc,
                                         "SPIR-V (compute stage) Local workgroup size in the X dimension (%" PRIu32
                                         ") is not a multiple of subgroupSize (%" PRIu32 ").",
                                         local_size_x, phys_dev_props_core11.subgroupSize);
                    }
                    if (m.scope == VK_SCOPE_WORKGROUP_KHR) {
                        if (workgroup_size >
                            phys_dev_ext_props.cooperative_matrix_props2_nv.cooperativeMatrixWorkgroupScopeMaxWorkgroupSize) {
                            skip |= LogError(
                                "VUID-VkPipelineShaderStageCreateInfo-module-10169", module_state.handle(), loc,
                                "SPIR-V (compute stage) Total local workgroup size (%" PRIu32
                                ") is larger than cooperativeMatrixWorkgroupScopeMaxWorkgroupSize (%" PRIu32 ").",
                                workgroup_size,
                                phys_dev_ext_props.cooperative_matrix_props2_nv.cooperativeMatrixWorkgroupScopeMaxWorkgroupSize);
                        }
                    }
                }

                if (!m.all_constant) {
                    break;
                }

                if (m.scope == VK_SCOPE_WORKGROUP_KHR && !enabled_features.cooperativeMatrixWorkgroupScope) {
                    skip |= LogError("VUID-RuntimeSpirv-cooperativeMatrixWorkgroupScope-10164", module_state.handle(), loc,
                                     "SPIR-V (compute stage) Cooperative matrix uses workgroup scope but "
                                     "cooperativeMatrixWorkgroupScope is not enabled.");
                }

                // Validate that the type parameters are all supported for one of the
                // operands of a cooperative matrix khr property.
                bool valid = false;
                for (uint32_t i = 0; i < cooperative_matrix_properties_khr.size(); ++i) {
                    const auto &property = cooperative_matrix_properties_khr[i];
                    if (property.AType == m.component_type && property.MSize == m.rows && property.KSize == m.cols &&
                        property.scope == m.scope && m.use == spv::CooperativeMatrixUseMatrixAKHR) {
                        valid = true;
                        break;
                    }
                    if (property.BType == m.component_type && property.KSize == m.rows && property.NSize == m.cols &&
                        property.scope == m.scope && m.use == spv::CooperativeMatrixUseMatrixBKHR) {
                        valid = true;
                        break;
                    }
                    if (property.CType == m.component_type && property.MSize == m.rows && property.NSize == m.cols &&
                        property.scope == m.scope && m.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR) {
                        valid = true;
                        break;
                    }
                    if (property.ResultType == m.component_type && property.MSize == m.rows && property.NSize == m.cols &&
                        property.scope == m.scope && m.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR) {
                        valid = true;
                        break;
                    }
                }
                if (enabled_features.cooperativeMatrixFlexibleDimensions) {
                    for (uint32_t i = 0; i < cooperative_matrix_flexible_dimensions_properties.size(); ++i) {
                        const auto &property = cooperative_matrix_flexible_dimensions_properties[i];

                        if (property.scope == VK_SCOPE_WORKGROUP_KHR && workgroup_size != property.workgroupInvocations) {
                            continue;
                        }

                        if (property.AType == m.component_type && SafeModulo(m.rows, property.MGranularity) == 0 &&
                            SafeModulo(m.cols, property.KGranularity) == 0 && property.scope == m.scope &&
                            m.use == spv::CooperativeMatrixUseMatrixAKHR) {
                            valid = true;
                            break;
                        }
                        if (property.BType == m.component_type && SafeModulo(m.rows, property.KGranularity) == 0 &&
                            SafeModulo(m.cols, property.NGranularity) == 0 && property.scope == m.scope &&
                            m.use == spv::CooperativeMatrixUseMatrixBKHR) {
                            valid = true;
                            break;
                        }
                        if (property.CType == m.component_type && SafeModulo(m.rows, property.MGranularity) == 0 &&
                            SafeModulo(m.cols, property.NGranularity) == 0 && property.scope == m.scope &&
                            m.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR) {
                            valid = true;
                            break;
                        }
                        if (property.ResultType == m.component_type && SafeModulo(m.rows, property.MGranularity) == 0 &&
                            SafeModulo(m.cols, property.NGranularity) == 0 && property.scope == m.scope &&
                            m.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR) {
                            valid = true;
                            break;
                        }
                    }
                }
                if (!valid) {
                    if (!enabled_features.cooperativeMatrixFlexibleDimensions) {
                        skip |= LogError("VUID-RuntimeSpirv-OpTypeCooperativeMatrixKHR-10163", module_state.handle(), loc,
                                         "SPIR-V (%s) has\n%s (%s)\nbut doesn't match any VkCooperativeMatrixPropertiesKHR\n%s.",
                                         string_VkShaderStageFlagBits(entrypoint.stage), insn.Describe().c_str(),
                                         m.Describe().c_str(), print_properties().c_str());
                    } else {
                        skip |= LogError("VUID-RuntimeSpirv-cooperativeMatrixFlexibleDimensions-10165", module_state.handle(), loc,
                                         "SPIR-V (%s) has\n%s (%s)\nbut doesn't match any VkCooperativeMatrixPropertiesKHR or "
                                         "VkCooperativeMatrixFlexibleDimensionsPropertiesNV\n%s\n%s.",
                                         string_VkShaderStageFlagBits(entrypoint.stage), insn.Describe().c_str(),
                                         m.Describe().c_str(), print_properties().c_str(), print_flexible_properties().c_str());
                    }
                }
                if (IsExtEnabled(extensions.vk_nv_cooperative_matrix2)) {
                    if (m.rows > phys_dev_ext_props.cooperative_matrix_props2_nv.cooperativeMatrixFlexibleDimensionsMaxDimension ||
                        m.cols > phys_dev_ext_props.cooperative_matrix_props2_nv.cooperativeMatrixFlexibleDimensionsMaxDimension) {
                        skip |= LogError(
                            "VUID-RuntimeSpirv-cooperativeMatrixFlexibleDimensionsMaxDimension-10167", module_state.handle(), loc,
                            "SPIR-V (%s) has\n%s (%s)\nbut number of rows or columns is greater than "
                            "cooperativeMatrixFlexibleDimensionsMaxDimension (%" PRIu32 ").",
                            string_VkShaderStageFlagBits(entrypoint.stage), insn.Describe().c_str(), m.Describe().c_str(),
                            phys_dev_ext_props.cooperative_matrix_props2_nv.cooperativeMatrixFlexibleDimensionsMaxDimension);
                    }
                }

                break;
            }
            case spv::OpCooperativeMatrixMulAddKHR: {
                const uint32_t flags = insn.Length() > 6 ? insn.Word(6) : 0u;
                CoopMatType r(id_to_type_id[insn.Word(2)], module_state, stage_state,
                              (flags & spv::CooperativeMatrixOperandsMatrixResultSignedComponentsKHRMask));
                CoopMatType a(id_to_type_id[insn.Word(3)], module_state, stage_state,
                              (flags & spv::CooperativeMatrixOperandsMatrixASignedComponentsKHRMask));
                CoopMatType b(id_to_type_id[insn.Word(4)], module_state, stage_state,
                              (flags & spv::CooperativeMatrixOperandsMatrixBSignedComponentsKHRMask));
                CoopMatType c(id_to_type_id[insn.Word(5)], module_state, stage_state,
                              (flags & spv::CooperativeMatrixOperandsMatrixCSignedComponentsKHRMask));
                if (a.all_constant && b.all_constant && c.all_constant && r.all_constant) {
                    // Validate that the type parameters are all supported for the same
                    // cooperative matrix property.
                    bool found_matching_prop = false;
                    for (uint32_t i = 0; i < cooperative_matrix_properties_khr.size(); ++i) {
                        const auto &property = cooperative_matrix_properties_khr[i];

                        bool valid = true;
                        valid &= property.AType == a.component_type && property.MSize == a.rows && property.KSize == a.cols &&
                                 property.scope == a.scope && a.use == spv::CooperativeMatrixUseMatrixAKHR;
                        valid &= property.BType == b.component_type && property.KSize == b.rows && property.NSize == b.cols &&
                                 property.scope == b.scope && b.use == spv::CooperativeMatrixUseMatrixBKHR;
                        valid &= property.CType == c.component_type && property.MSize == c.rows && property.NSize == c.cols &&
                                 property.scope == c.scope && c.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR;
                        valid &= property.ResultType == r.component_type && property.MSize == r.rows && property.NSize == r.cols &&
                                 property.scope == r.scope && r.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR;

                        valid &= !IsSignedIntEnum(property.AType) ||
                                 (flags & spv::CooperativeMatrixOperandsMatrixASignedComponentsKHRMask);
                        valid &= !IsSignedIntEnum(property.BType) ||
                                 (flags & spv::CooperativeMatrixOperandsMatrixBSignedComponentsKHRMask);
                        valid &= !IsSignedIntEnum(property.CType) ||
                                 (flags & spv::CooperativeMatrixOperandsMatrixCSignedComponentsKHRMask);
                        valid &= !IsSignedIntEnum(property.ResultType) ||
                                 (flags & spv::CooperativeMatrixOperandsMatrixResultSignedComponentsKHRMask);

                        valid &= property.saturatingAccumulation ==
                                 !!(flags & spv::CooperativeMatrixOperandsSaturatingAccumulationKHRMask);

                        if (valid) {
                            found_matching_prop = true;
                            break;
                        }
                    }
                    bool found_matching_flexible_prop = false;
                    if (enabled_features.cooperativeMatrixFlexibleDimensions) {
                        for (uint32_t i = 0; i < cooperative_matrix_flexible_dimensions_properties.size(); ++i) {
                            const auto &property = cooperative_matrix_flexible_dimensions_properties[i];

                            bool valid = true;
                            valid &= property.AType == a.component_type && SafeModulo(a.rows, property.MGranularity) == 0 &&
                                     SafeModulo(a.cols, property.KGranularity) == 0 && property.scope == a.scope &&
                                     a.use == spv::CooperativeMatrixUseMatrixAKHR;
                            valid &= property.BType == b.component_type && SafeModulo(b.rows, property.KGranularity) == 0 &&
                                     SafeModulo(b.cols, property.NGranularity) == 0 && property.scope == b.scope &&
                                     b.use == spv::CooperativeMatrixUseMatrixBKHR;
                            valid &= property.CType == c.component_type && SafeModulo(c.rows, property.MGranularity) == 0 &&
                                     SafeModulo(c.cols, property.NGranularity) == 0 && property.scope == c.scope &&
                                     c.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR;
                            valid &= property.ResultType == r.component_type && SafeModulo(r.rows, property.MGranularity) == 0 &&
                                     SafeModulo(r.cols, property.NGranularity) == 0 && property.scope == r.scope &&
                                     r.use == spv::CooperativeMatrixUseMatrixAccumulatorKHR;

                            valid &= !IsSignedIntEnum(property.AType) ||
                                     (flags & spv::CooperativeMatrixOperandsMatrixASignedComponentsKHRMask);
                            valid &= !IsSignedIntEnum(property.BType) ||
                                     (flags & spv::CooperativeMatrixOperandsMatrixBSignedComponentsKHRMask);
                            valid &= !IsSignedIntEnum(property.CType) ||
                                     (flags & spv::CooperativeMatrixOperandsMatrixCSignedComponentsKHRMask);
                            valid &= !IsSignedIntEnum(property.ResultType) ||
                                     (flags & spv::CooperativeMatrixOperandsMatrixResultSignedComponentsKHRMask);

                            valid &= property.saturatingAccumulation ==
                                     !!(flags & spv::CooperativeMatrixOperandsSaturatingAccumulationKHRMask);

                            valid &= property.scope != VK_SCOPE_WORKGROUP_KHR || workgroup_size == property.workgroupInvocations;

                            if (valid) {
                                found_matching_flexible_prop = true;
                                break;
                            }
                        }
                    }
                    if (!found_matching_prop && !found_matching_flexible_prop) {
                        if (!enabled_features.cooperativeMatrixFlexibleDimensions) {
                            skip |= LogError("VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-10060", module_state.handle(), loc,
                                             "SPIR-V (%s) instruction\n%s\ndoesn't match a supported matrix "
                                             "VkCooperativeMatrixPropertiesKHR\n%s\n%s\n%s\n%s\n%s\n",
                                             string_VkShaderStageFlagBits(entrypoint.stage), insn.Describe().c_str(),
                                             a.Describe().c_str(), b.Describe().c_str(), c.Describe().c_str(), r.Describe().c_str(),
                                             print_properties().c_str());
                        } else {
                            skip |=
                                LogError("VUID-RuntimeSpirv-cooperativeMatrixFlexibleDimensions-10166", module_state.handle(), loc,
                                         "SPIR-V (%s) instruction\n%s\ndoesn't match a supported matrix "
                                         "VkCooperativeMatrixPropertiesKHR or "
                                         "VkPhysicalDeviceCooperativeMatrix2PropertiesNV\n%s\n%s\n%s\n%s\n%s\n%s\n",
                                         string_VkShaderStageFlagBits(entrypoint.stage), insn.Describe().c_str(),
                                         a.Describe().c_str(), b.Describe().c_str(), c.Describe().c_str(), r.Describe().c_str(),
                                         print_properties().c_str(), print_flexible_properties().c_str());
                        }
                    }
                }
                break;
            }
            case spv::OpTypeCooperativeMatrixNV: {
                CoopMatType m(insn.Word(1), module_state, stage_state, IsSignedIntType(insn.Word(2)));

                if (!m.all_constant) {
                    break;
                }
                // Validate that the type parameters are all supported for one of the
                // operands of a cooperative matrix property.
                bool valid = false;
                for (uint32_t i = 0; i < cooperative_matrix_properties_nv.size(); ++i) {
                    const auto &property = cooperative_matrix_properties_nv[i];
                    if (property.AType == m.component_type && property.MSize == m.rows && property.KSize == m.cols &&
                        property.scope == m.scope) {
                        valid = true;
                        break;
                    }
                    if (property.BType == m.component_type && property.KSize == m.rows && property.NSize == m.cols &&
                        property.scope == m.scope) {
                        valid = true;
                        break;
                    }
                    if (property.CType == m.component_type && property.MSize == m.rows && property.NSize == m.cols &&
                        property.scope == m.scope) {
                        valid = true;
                        break;
                    }
                    if (property.DType == m.component_type && property.MSize == m.rows && property.NSize == m.cols &&
                        property.scope == m.scope) {
                        valid = true;
                        break;
                    }
                }
                if (!valid) {
                    skip |= LogError("VUID-RuntimeSpirv-OpTypeCooperativeMatrixNV-06316", module_state.handle(), loc,
                                     "SPIR-V (%s) has an OpTypeCooperativeMatrixNV (result id = %" PRIu32
                                     ") operand that don't match a supported matrix type (%s).",
                                     string_VkShaderStageFlagBits(entrypoint.stage), insn.Word(1), m.Describe().c_str());
                }
                break;
            }
            case spv::OpCooperativeMatrixMulAddNV: {
                CoopMatType d(id_to_type_id[insn.Word(2)], module_state, stage_state, IsSignedIntType(id_to_type_id[insn.Word(2)]));
                CoopMatType a(id_to_type_id[insn.Word(3)], module_state, stage_state, IsSignedIntType(id_to_type_id[insn.Word(3)]));
                CoopMatType b(id_to_type_id[insn.Word(4)], module_state, stage_state, IsSignedIntType(id_to_type_id[insn.Word(4)]));
                CoopMatType c(id_to_type_id[insn.Word(5)], module_state, stage_state, IsSignedIntType(id_to_type_id[insn.Word(5)]));

                if (a.all_constant && b.all_constant && c.all_constant && d.all_constant) {
                    // Validate that the type parameters are all supported for the same
                    // cooperative matrix property.
                    bool valid_a = false;
                    bool valid_b = false;
                    bool valid_c = false;
                    bool valid_d = false;
                    for (uint32_t i = 0; i < cooperative_matrix_properties_nv.size(); ++i) {
                        const auto &property = cooperative_matrix_properties_nv[i];
                        valid_a |= property.AType == a.component_type && property.MSize == a.rows && property.KSize == a.cols &&
                                   property.scope == a.scope;
                        valid_b |= property.BType == b.component_type && property.KSize == b.rows && property.NSize == b.cols &&
                                   property.scope == b.scope;
                        valid_c |= property.CType == c.component_type && property.MSize == c.rows && property.NSize == c.cols &&
                                   property.scope == c.scope;
                        valid_d |= property.DType == d.component_type && property.MSize == d.rows && property.NSize == d.cols &&
                                   property.scope == d.scope;
                        if (valid_a && valid_b && valid_c && valid_d) {
                            break;
                        }
                    }
                    if (!valid_a) {
                        skip |= LogError(
                            "VUID-RuntimeSpirv-OpTypeCooperativeMatrixMulAddNV-10059", module_state.handle(), loc,
                            "SPIR-V (%s) OpCooperativeMatrixMulAddNV (result id = %u) operands don't match a supported matrix "
                            "VkCooperativeMatrixPropertiesNV for A type (%s).",
                            string_VkShaderStageFlagBits(entrypoint.stage), insn.Word(2), a.Describe().c_str());
                    } else if (!valid_b) {
                        skip |= LogError(
                            "VUID-RuntimeSpirv-OpTypeCooperativeMatrixMulAddNV-10059", module_state.handle(), loc,
                            "SPIR-V (%s) OpCooperativeMatrixMulAddNV (result id = %u) operands don't match a supported matrix "
                            "VkCooperativeMatrixPropertiesNV for B type (%s).",
                            string_VkShaderStageFlagBits(entrypoint.stage), insn.Word(2), b.Describe().c_str());
                    } else if (!valid_c) {
                        skip |= LogError(
                            "VUID-RuntimeSpirv-OpTypeCooperativeMatrixMulAddNV-10059", module_state.handle(), loc,
                            "SPIR-V (%s) OpCooperativeMatrixMulAddNV (result id = %u) operands don't match a supported matrix "
                            "VkCooperativeMatrixPropertiesNV for C type (%s).",
                            string_VkShaderStageFlagBits(entrypoint.stage), insn.Word(2), c.Describe().c_str());
                    } else if (!valid_d) {
                        skip |= LogError(
                            "VUID-RuntimeSpirv-OpTypeCooperativeMatrixMulAddNV-10059", module_state.handle(), loc,
                            "SPIR-V (%s) OpCooperativeMatrixMulAddNV (result id = %u) operands don't match a supported matrix "
                            "VkCooperativeMatrixPropertiesNV for D type (%s).",
                            string_VkShaderStageFlagBits(entrypoint.stage), insn.Word(2), d.Describe().c_str());
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    return skip;
}

bool CoreChecks::ValidateCooperativeVector(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                           const ShaderStageState &stage_state, const Location &loc) const {
    bool skip = false;

    struct CoopVecType {
        VkComponentTypeKHR component_type;
        uint32_t component_count;
        bool all_constant;

        CoopVecType(uint32_t id, const spirv::Module &module_state, const ShaderStageState &stage_state) {
            const spirv::Instruction *insn = module_state.FindDef(id);
            const spirv::Instruction *component_type_insn = module_state.FindDef(insn->Word(2));
            const spirv::Instruction *component_count_insn = module_state.FindDef(insn->Word(3));

            all_constant = true;
            if (!stage_state.GetInt32ConstantValue(*component_count_insn, &component_count)) {
                all_constant = false;
            }
            component_type = GetComponentType(component_type_insn, false);
        }

        std::string Describe() {
            std::ostringstream ss;
            ss << "component count: " << component_count << ", type: " << string_VkComponentTypeKHR(component_type);
            return ss.str();
        }
    };

    if (module_state.HasCapability(spv::CapabilityCooperativeVectorNV) ||
        module_state.HasCapability(spv::CapabilityCooperativeVectorTrainingNV)) {
        if (!(entrypoint.stage & phys_dev_ext_props.cooperative_vector_props_nv.cooperativeVectorSupportedStages)) {
            skip |= LogError(
                "VUID-RuntimeSpirv-cooperativeVectorSupportedStages-10091", module_state.handle(), loc,
                "SPIR-V contains cooperative vector capability used in shader stage %s but is not in "
                "cooperativeVectorSupportedStages (%s)",
                string_VkShaderStageFlagBits(entrypoint.stage),
                string_VkShaderStageFlags(phys_dev_ext_props.cooperative_vector_props_nv.cooperativeVectorSupportedStages).c_str());
        }
    } else {
        return skip;
    }

    vvl::unordered_map<uint32_t, uint32_t> id_to_type_id;
    for (const spirv::Instruction &insn : module_state.GetInstructions()) {
        if (OpcodeHasType(insn.Opcode()) && OpcodeHasResult(insn.Opcode())) {
            id_to_type_id[insn.Word(2)] = insn.Word(1);
        }
    }
    for (const spirv::Instruction *cooperative_vector_inst : module_state.static_data_.cooperative_vector_inst) {
        const spirv::Instruction &insn = *cooperative_vector_inst;
        switch (insn.Opcode()) {
            case spv::OpTypeCooperativeVectorNV: {
                CoopVecType m(insn.Word(1), module_state, stage_state);

                if (!m.all_constant) {
                    break;
                }

                if (m.component_count > phys_dev_ext_props.cooperative_vector_props_nv.maxCooperativeVectorComponents) {
                    skip |= LogError("VUID-RuntimeSpirv-maxCooperativeVectorComponents-10094", module_state.handle(), loc,
                                     "SPIR-V (%s) component count (%d) is greater than maxCooperativeVectorComponents (%d)",
                                     string_VkShaderStageFlagBits(entrypoint.stage), m.component_count,
                                     phys_dev_ext_props.cooperative_vector_props_nv.maxCooperativeVectorComponents);
                }

                bool found = false;
                for (uint32_t i = 0; i < cooperative_vector_properties_nv.size(); ++i) {
                    const auto &property = cooperative_vector_properties_nv[i];
                    if (m.component_type == property.inputType || m.component_type == property.resultType) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    skip |= LogError("VUID-RuntimeSpirv-OpTypeCooperativeVector-10095", module_state.handle(), loc,
                                     "SPIR-V (%s) contains unsupported cooperative vector component type (%s)",
                                     string_VkShaderStageFlagBits(entrypoint.stage),
                                     string_VkComponentTypeKHR((VkComponentTypeKHR)m.component_type));
                }

                break;
            }
            case spv::OpCooperativeVectorLoadNV:
            case spv::OpCooperativeVectorStoreNV: {
                // Nothing we can validate outside of GPUAV
                break;
            }
            case spv::OpCooperativeVectorMatrixMulNV:
            case spv::OpCooperativeVectorMatrixMulAddNV: {
                CoopVecType result(id_to_type_id[insn.Word(2)], module_state, stage_state);
                CoopVecType input(id_to_type_id[insn.Word(3)], module_state, stage_state);

                uint32_t result_type = result.component_type;
                uint32_t input_type = input.component_type;

                uint32_t biasOffset = insn.Opcode() == spv::OpCooperativeVectorMatrixMulAddNV ? 3 : 0;

                bool all_constant = true;
                uint32_t input_interpretation{};
                uint32_t matrix_interpretation{};
                uint32_t bias_interpretation{};
                bool transpose{};
                if (!stage_state.GetInt32ConstantValue(*module_state.FindDef(insn.Word(4)), &input_interpretation)) {
                    all_constant = false;
                }
                if (!stage_state.GetInt32ConstantValue(*module_state.FindDef(insn.Word(7)), &matrix_interpretation)) {
                    all_constant = false;
                }
                if (insn.Opcode() == spv::OpCooperativeVectorMatrixMulAddNV) {
                    if (!stage_state.GetInt32ConstantValue(*module_state.FindDef(insn.Word(10)), &bias_interpretation)) {
                        all_constant = false;
                    }
                }
                if (!stage_state.GetBooleanConstantValue(*module_state.FindDef(insn.Word(11 + biasOffset)), &transpose)) {
                    all_constant = false;
                }

                if (!all_constant) {
                    break;
                }

                bool found = false;
                for (uint32_t i = 0; i < cooperative_vector_properties_nv.size(); ++i) {
                    const auto &property = cooperative_vector_properties_nv[i];
                    if (property.inputType == input_type && property.inputInterpretation == input_interpretation &&
                        property.matrixInterpretation == matrix_interpretation &&
                        (insn.Opcode() == spv::OpCooperativeVectorMatrixMulNV ||
                         property.biasInterpretation == bias_interpretation) &&
                        property.resultType == result_type && (!transpose || property.transpose)) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorMatrixMulNV-10089", module_state.handle(), loc,
                                     "SPIR-V (%s) contains unsupported cooperative vector matrix mul with "
                                     "result component type (%s), input component type (%s), input interpretation (%s), "
                                     "matrix interpretation (%s), bias interpretation (%s), transpose (%d)",
                                     string_VkShaderStageFlagBits(entrypoint.stage),
                                     string_VkComponentTypeKHR((VkComponentTypeKHR)result_type),
                                     string_VkComponentTypeKHR((VkComponentTypeKHR)input_type),
                                     string_VkComponentTypeKHR((VkComponentTypeKHR)input_interpretation),
                                     string_VkComponentTypeKHR((VkComponentTypeKHR)matrix_interpretation),
                                     (insn.Opcode() == spv::OpCooperativeVectorMatrixMulNV
                                          ? "None"
                                          : string_VkComponentTypeKHR((VkComponentTypeKHR)bias_interpretation)),
                                     transpose);
                }

                uint32_t memory_layout{};
                if (stage_state.GetInt32ConstantValue(*module_state.FindDef(insn.Word(10 + biasOffset)), &memory_layout)) {
                    if ((matrix_interpretation == VK_COMPONENT_TYPE_FLOAT_E4M3_NV ||
                         matrix_interpretation == VK_COMPONENT_TYPE_FLOAT_E5M2_NV) &&
                        !(memory_layout == VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_INFERENCING_OPTIMAL_NV ||
                          memory_layout == VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_TRAINING_OPTIMAL_NV)) {
                        skip |=
                            LogError("VUID-RuntimeSpirv-OpCooperativeVectorMatrixMulNV-10090", module_state.handle(), loc,
                                     "SPIR-V (%s) contains unsupported cooperative vector matrix mul with "
                                     "matrix_interpretation (%s) and memory layout (%s)",
                                     string_VkShaderStageFlagBits(entrypoint.stage),
                                     string_VkComponentTypeKHR((VkComponentTypeKHR)matrix_interpretation),
                                     string_VkCooperativeVectorMatrixLayoutNV((VkCooperativeVectorMatrixLayoutNV)memory_layout));
                    }
                }
                break;
            }
            case spv::OpCooperativeVectorReduceSumAccumulateNV: {
                CoopVecType v(id_to_type_id[insn.Word(3)], module_state, stage_state);

                switch (v.component_type) {
                    case VK_COMPONENT_TYPE_FLOAT16_KHR:
                        if (!phys_dev_ext_props.cooperative_vector_props_nv.cooperativeVectorTrainingFloat16Accumulation) {
                            skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorReduceSumAccumulateNV-10092",
                                             module_state.handle(), loc,
                                             "SPIR-V (%s) Component type is FLOAT16 but "
                                             "cooperativeVectorTrainingFloat16Accumulation not supported",
                                             string_VkShaderStageFlagBits(entrypoint.stage));
                        }
                        break;
                    case VK_COMPONENT_TYPE_FLOAT32_KHR:
                        if (!phys_dev_ext_props.cooperative_vector_props_nv.cooperativeVectorTrainingFloat32Accumulation) {
                            skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorReduceSumAccumulateNV-10092",
                                             module_state.handle(), loc,
                                             "SPIR-V (%s) Component type is FLOAT32 but "
                                             "cooperativeVectorTrainingFloat32Accumulation not supported",
                                             string_VkShaderStageFlagBits(entrypoint.stage));
                        }
                        break;
                    default:
                        skip |=
                            LogError("VUID-RuntimeSpirv-OpCooperativeVectorReduceSumAccumulateNV-10092", module_state.handle(), loc,
                                     "SPIR-V (%s) Unsupported component type (%s)", string_VkShaderStageFlagBits(entrypoint.stage),
                                     string_VkComponentTypeKHR((VkComponentTypeKHR)v.component_type));
                        break;
                }

                const spirv::Instruction *ptr_type = module_state.FindDef(id_to_type_id[insn.Word(1)]);
                if (ptr_type->StorageClass() != spv::StorageClassStorageBuffer &&
                    ptr_type->StorageClass() != spv::StorageClassPhysicalStorageBuffer) {
                    skip |=
                        LogError("VUID-RuntimeSpirv-OpCooperativeVectorReduceSumAccumulateNV-10092", module_state.handle(), loc,
                                 "SPIR-V (%s) Unsupported pointer storage class (%s)",
                                 string_VkShaderStageFlagBits(entrypoint.stage), string_SpvStorageClass(ptr_type->StorageClass()));
                }

                break;
            }

            case spv::OpCooperativeVectorOuterProductAccumulateNV: {
                uint32_t matrix_interpretation{};
                if (stage_state.GetInt32ConstantValue(*module_state.FindDef(insn.Word(6)), &matrix_interpretation)) {
                    switch (matrix_interpretation) {
                        case VK_COMPONENT_TYPE_FLOAT16_KHR:
                            if (!phys_dev_ext_props.cooperative_vector_props_nv.cooperativeVectorTrainingFloat16Accumulation) {
                                skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093",
                                                 module_state.handle(), loc,
                                                 "SPIR-V (%s) Matrix interpretation is FLOAT16 but "
                                                 "cooperativeVectorTrainingFloat16Accumulation not supported",
                                                 string_VkShaderStageFlagBits(entrypoint.stage));
                            }
                            break;
                        case VK_COMPONENT_TYPE_FLOAT32_KHR:
                            if (!phys_dev_ext_props.cooperative_vector_props_nv.cooperativeVectorTrainingFloat32Accumulation) {
                                skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093",
                                                 module_state.handle(), loc,
                                                 "SPIR-V (%s) Matrix interpretation is FLOAT32 but "
                                                 "cooperativeVectorTrainingFloat32Accumulation not supported",
                                                 string_VkShaderStageFlagBits(entrypoint.stage));
                            }
                            break;
                        default:
                            skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093",
                                             module_state.handle(), loc, "SPIR-V (%s) Unsupported Matrix interpretation (%s)",
                                             string_VkShaderStageFlagBits(entrypoint.stage),
                                             string_VkComponentTypeKHR((VkComponentTypeKHR)matrix_interpretation));
                            break;
                    }
                }

                CoopVecType a(id_to_type_id[insn.Word(3)], module_state, stage_state);
                CoopVecType b(id_to_type_id[insn.Word(4)], module_state, stage_state);

                if (a.component_type != VK_COMPONENT_TYPE_FLOAT16_KHR) {
                    skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093", module_state.handle(),
                                     loc, "SPIR-V (%s) Component type of A (%s) must be FLOAT16",
                                     string_VkShaderStageFlagBits(entrypoint.stage),
                                     string_VkComponentTypeKHR((VkComponentTypeKHR)a.component_type));
                }
                if (b.component_type != VK_COMPONENT_TYPE_FLOAT16_KHR) {
                    skip |= LogError("VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093", module_state.handle(),
                                     loc, "SPIR-V (%s) Component type of B (%s) must be FLOAT16",
                                     string_VkShaderStageFlagBits(entrypoint.stage),
                                     string_VkComponentTypeKHR((VkComponentTypeKHR)b.component_type));
                }

                uint32_t memory_layout{};
                if (stage_state.GetInt32ConstantValue(*module_state.FindDef(insn.Word(5)), &memory_layout)) {
                    if (memory_layout != VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_TRAINING_OPTIMAL_NV) {
                        skip |=
                            LogError("VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093", module_state.handle(),
                                     loc, "SPIR-V (%s) Memory layout (%s) must be TRAINING_OPTIMAL",
                                     string_VkShaderStageFlagBits(entrypoint.stage),
                                     string_VkCooperativeVectorMatrixLayoutNV((VkCooperativeVectorMatrixLayoutNV)memory_layout));
                    }
                }

                const spirv::Instruction *ptr_type = module_state.FindDef(id_to_type_id[insn.Word(1)]);
                if (ptr_type->StorageClass() != spv::StorageClassStorageBuffer &&
                    ptr_type->StorageClass() != spv::StorageClassPhysicalStorageBuffer) {
                    skip |=
                        LogError("VUID-RuntimeSpirv-OpCooperativeVectorOuterProductAccumulateNV-10093", module_state.handle(), loc,
                                 "SPIR-V (%s) Unsupported pointer storage class (%s)",
                                 string_VkShaderStageFlagBits(entrypoint.stage), string_SpvStorageClass(ptr_type->StorageClass()));
                }

                break;
            }

            default:
                assert(false);  // unexpected instruction
                break;
        }
    }
    return skip;
}

bool CoreChecks::ValidateShaderResolveQCOM(const spirv::Module &module_state, VkShaderStageFlagBits stage,
                                           const vvl::Pipeline &pipeline, const Location &loc) const {
    bool skip = false;

    // If the pipeline's subpass description contains flag VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM,
    // then the fragment shader must not enable the SPIRV SampleRateShading capability.
    if (stage == VK_SHADER_STAGE_FRAGMENT_BIT && module_state.HasCapability(spv::CapabilitySampleRateShading)) {
        const auto &rp_state = pipeline.RenderPassState();
        auto subpass_flags = (!rp_state) ? 0 : rp_state->create_info.pSubpasses[pipeline.Subpass()].flags;
        if ((subpass_flags & VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM) != 0) {
            const LogObjectList objlist(module_state.handle(), rp_state->Handle());
            skip |= LogError("VUID-RuntimeSpirv-SampleRateShading-06378", objlist, loc,
                             "SPIR-V (Fragment stage) enables SampleRateShading capability "
                             "and the subpass flags includes VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM.");
        }
    }

    return skip;
}

bool CoreChecks::ValidateAtomicsTypes(const spirv::Module &module_state, const spirv::StatelessData &stateless_data,
                                      const Location &loc) const {
    bool skip = false;

    // "If sparseImageInt64Atomics is enabled, shaderImageInt64Atomics must be enabled"
    const bool valid_image_64_int = enabled_features.shaderImageInt64Atomics == VK_TRUE;

    const bool valid_storage_buffer_float =
        ((enabled_features.shaderBufferFloat32Atomics == VK_TRUE) || (enabled_features.shaderBufferFloat32AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderBufferFloat64Atomics == VK_TRUE) || (enabled_features.shaderBufferFloat64AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderBufferFloat16Atomics == VK_TRUE) || (enabled_features.shaderBufferFloat16AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderBufferFloat16AtomicMinMax == VK_TRUE) ||
         (enabled_features.shaderBufferFloat32AtomicMinMax == VK_TRUE) ||
         (enabled_features.shaderBufferFloat64AtomicMinMax == VK_TRUE) || (enabled_features.shaderFloat16VectorAtomics == VK_TRUE));

    const bool valid_workgroup_float =
        ((enabled_features.shaderSharedFloat32Atomics == VK_TRUE) || (enabled_features.shaderSharedFloat32AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderSharedFloat64Atomics == VK_TRUE) || (enabled_features.shaderSharedFloat64AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderSharedFloat16Atomics == VK_TRUE) || (enabled_features.shaderSharedFloat16AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderSharedFloat16AtomicMinMax == VK_TRUE) ||
         (enabled_features.shaderSharedFloat32AtomicMinMax == VK_TRUE) ||
         (enabled_features.shaderSharedFloat64AtomicMinMax == VK_TRUE) || (enabled_features.shaderFloat16VectorAtomics == VK_TRUE));

    const bool valid_image_float =
        ((enabled_features.shaderImageFloat32Atomics == VK_TRUE) || (enabled_features.shaderImageFloat32AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderImageFloat32AtomicMinMax == VK_TRUE) || (enabled_features.shaderFloat16VectorAtomics == VK_TRUE));

    const bool valid_16_float =
        ((enabled_features.shaderBufferFloat16Atomics == VK_TRUE) || (enabled_features.shaderBufferFloat16AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderBufferFloat16AtomicMinMax == VK_TRUE) ||
         (enabled_features.shaderSharedFloat16Atomics == VK_TRUE) || (enabled_features.shaderSharedFloat16AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderSharedFloat16AtomicMinMax == VK_TRUE));

    const bool valid_32_float =
        ((enabled_features.shaderBufferFloat32Atomics == VK_TRUE) || (enabled_features.shaderBufferFloat32AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderSharedFloat32Atomics == VK_TRUE) || (enabled_features.shaderSharedFloat32AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderImageFloat32Atomics == VK_TRUE) || (enabled_features.shaderImageFloat32AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderBufferFloat32AtomicMinMax == VK_TRUE) ||
         (enabled_features.shaderSharedFloat32AtomicMinMax == VK_TRUE) ||
         (enabled_features.shaderImageFloat32AtomicMinMax == VK_TRUE));

    const bool valid_64_float =
        ((enabled_features.shaderBufferFloat64Atomics == VK_TRUE) || (enabled_features.shaderBufferFloat64AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderSharedFloat64Atomics == VK_TRUE) || (enabled_features.shaderSharedFloat64AtomicAdd == VK_TRUE) ||
         (enabled_features.shaderBufferFloat64AtomicMinMax == VK_TRUE) ||
         (enabled_features.shaderSharedFloat64AtomicMinMax == VK_TRUE));

    const bool valid_16_float_vector = (enabled_features.shaderFloat16VectorAtomics == VK_TRUE);
    // clang-format on

    for (const spirv::Instruction *atomic_def_ptr : stateless_data.atomic_inst) {
        const spirv::Instruction &atomic_def = *atomic_def_ptr;
        const spirv::AtomicInstructionInfo &atomic = module_state.GetAtomicInfo(atomic_def);
        const uint32_t opcode = atomic_def.Opcode();

        if (atomic.type == spv::OpTypeFloat && (atomic.vector_size == 2 || atomic.vector_size == 4)) {
            if (!valid_16_float_vector) {
                skip |=
                    LogError("VUID-RuntimeSpirv-shaderFloat16VectorAtomics-09581", module_state.handle(), loc,
                             "SPIR-V is using 16-bit float vector atomics operationswith %s storage class, but "
                             "shaderFloat16VectorAtomics was not enabled.\n%s\n",
                             string_SpvStorageClass(atomic.storage_class), module_state.DescribeInstruction(atomic_def).c_str());
            }
        } else if ((atomic.bit_width == 64) && (atomic.type == spv::OpTypeInt)) {
            // Validate 64-bit image atomics
            if (((atomic.storage_class == spv::StorageClassStorageBuffer) || (atomic.storage_class == spv::StorageClassUniform)) &&
                (enabled_features.shaderBufferInt64Atomics == VK_FALSE)) {
                skip |=
                    LogError("VUID-RuntimeSpirv-None-06278", module_state.handle(), loc,
                             "SPIR-V is using 64-bit int atomics operations with %s storage class, but "
                             "shaderBufferInt64Atomics was not enabled. \n%s\n",
                             string_SpvStorageClass(atomic.storage_class), module_state.DescribeInstruction(atomic_def).c_str());
            } else if ((atomic.storage_class == spv::StorageClassWorkgroup) &&
                       (enabled_features.shaderSharedInt64Atomics == VK_FALSE)) {
                skip |= LogError("VUID-RuntimeSpirv-None-06279", module_state.handle(), loc,
                                 "SPIR-V is using 64-bit int atomics operations with Workgroup storage class, but "
                                 "shaderSharedInt64Atomics was not enabled.\n%s\n",
                                 module_state.DescribeInstruction(atomic_def).c_str());
            } else if ((atomic.storage_class == spv::StorageClassImage) && (valid_image_64_int == false)) {
                skip |= LogError("VUID-RuntimeSpirv-None-06288", module_state.handle(), loc,
                                 "SPIR-V is using 64-bit int atomics operations with Image storage class, but "
                                 "shaderImageInt64Atomics was not enabled.\n%s\n",
                                 module_state.DescribeInstruction(atomic_def).c_str());
            }
        } else if (atomic.type == spv::OpTypeFloat) {
            // Validate Floats
            if (atomic.storage_class == spv::StorageClassStorageBuffer) {
                if (valid_storage_buffer_float == false) {
                    skip |= LogError("VUID-RuntimeSpirv-None-06284", module_state.handle(), loc,
                                     "SPIR-V is using float atomics operations with StorageBuffer storage class, but none of "
                                     "the required features were enabled.\n%s\n",
                                     module_state.DescribeInstruction(atomic_def).c_str());
                } else if (opcode == spv::OpAtomicFAddEXT) {
                    if ((atomic.bit_width == 16) && (enabled_features.shaderBufferFloat16AtomicAdd == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06337", module_state.handle(), loc,
                                         "SPIR-V is using 16-bit float atomics for add operations with "
                                         "StorageBuffer storage class, but shaderBufferFloat16AtomicAdd was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 32) && (enabled_features.shaderBufferFloat32AtomicAdd == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06338", module_state.handle(), loc,
                                         "SPIR-V is using 32-bit float atomics for add operations with "
                                         "StorageBuffer storage class, but shaderBufferFloat32AtomicAdd was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 64) && (enabled_features.shaderBufferFloat64AtomicAdd == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06339", module_state.handle(), loc,
                                         "SPIR-V is using 64-bit float atomics for add operations with "
                                         "StorageBuffer storage class, but shaderBufferFloat64AtomicAdd was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    }
                } else if (opcode == spv::OpAtomicFMinEXT || opcode == spv::OpAtomicFMaxEXT) {
                    if ((atomic.bit_width == 16) && (enabled_features.shaderBufferFloat16AtomicMinMax == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06337", module_state.handle(), loc,
                                         "SPIR-V is using 16-bit float atomics for min/max operations with "
                                         "StorageBuffer storage class, but shaderBufferFloat16AtomicMinMax was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 32) && (enabled_features.shaderBufferFloat32AtomicMinMax == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06338", module_state.handle(), loc,
                                         "SPIR-V is using 32-bit float atomics for min/max operations with "
                                         "StorageBuffer storage class, but shaderBufferFloat32AtomicMinMax was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 64) && (enabled_features.shaderBufferFloat64AtomicMinMax == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06339", module_state.handle(), loc,
                                         "SPIR-V is using 64-bit float atomics for min/max operations with "
                                         "StorageBuffer storage class, but shaderBufferFloat64AtomicMinMax was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    }
                } else {
                    // Assume is valid load/store/exchange (rest of supported atomic operations) or else spirv-val will catch
                    if ((atomic.bit_width == 16) && (enabled_features.shaderBufferFloat16Atomics == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06338", module_state.handle(), loc,
                                         "SPIR-V is using 16-bit float atomics for load/store/exhange operations with "
                                         "StorageBuffer storage class, but shaderBufferFloat16Atomics was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 32) && (enabled_features.shaderBufferFloat32Atomics == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06338", module_state.handle(), loc,
                                         "SPIR-V is using 32-bit float atomics for load/store/exhange operations with "
                                         "StorageBuffer storage class, but shaderBufferFloat32Atomics was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 64) && (enabled_features.shaderBufferFloat64Atomics == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06339", module_state.handle(), loc,
                                         "SPIR-V is using 64-bit float atomics for load/store/exhange operations with "
                                         "StorageBuffer storage class, but shaderBufferFloat64Atomics was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    }
                }
            } else if (atomic.storage_class == spv::StorageClassWorkgroup) {
                if (valid_workgroup_float == false) {
                    skip |= LogError("VUID-RuntimeSpirv-None-06285", module_state.handle(), loc,
                                     "SPIR-V is using float atomics operations with Workgroup storage class, but none of the "
                                     "required features were enabled.\n%s\n",
                                     module_state.DescribeInstruction(atomic_def).c_str());
                } else if (opcode == spv::OpAtomicFAddEXT) {
                    if ((atomic.bit_width == 16) && (enabled_features.shaderSharedFloat16AtomicAdd == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06337", module_state.handle(), loc,
                                         "SPIR-V is using 16-bit float atomics for add operations with Workgroup "
                                         "storage class, but shaderSharedFloat16AtomicAdd was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 32) && (enabled_features.shaderSharedFloat32AtomicAdd == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06338", module_state.handle(), loc,
                                         "SPIR-V is using 32-bit float atomics for add operations with Workgroup "
                                         "storage class, but shaderSharedFloat32AtomicAdd was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 64) && (enabled_features.shaderSharedFloat64AtomicAdd == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06339", module_state.handle(), loc,
                                         "SPIR-V is using 64-bit float atomics for add operations with Workgroup "
                                         "storage class, but shaderSharedFloat64AtomicAdd was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    }
                } else if (opcode == spv::OpAtomicFMinEXT || opcode == spv::OpAtomicFMaxEXT) {
                    if ((atomic.bit_width == 16) && (enabled_features.shaderSharedFloat16AtomicMinMax == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06337", module_state.handle(), loc,
                                         "SPIR-V is using 16-bit float atomics for min/max operations with "
                                         "Workgroup storage class, but shaderSharedFloat16AtomicMinMax was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 32) && (enabled_features.shaderSharedFloat32AtomicMinMax == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06338", module_state.handle(), loc,
                                         "SPIR-V is using 32-bit float atomics for min/max operations with "
                                         "Workgroup storage class, but shaderSharedFloat32AtomicMinMax was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 64) && (enabled_features.shaderSharedFloat64AtomicMinMax == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06339", module_state.handle(), loc,
                                         "SPIR-V is using 64-bit float atomics for min/max operations with "
                                         "Workgroup storage class, but shaderSharedFloat64AtomicMinMax was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    }
                } else {
                    // Assume is valid load/store/exchange (rest of supported atomic operations) or else spirv-val will catch
                    if ((atomic.bit_width == 16) && (enabled_features.shaderSharedFloat16Atomics == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06337", module_state.handle(), loc,
                                         "SPIR-V is using 16-bit float atomics for load/store/exhange operations with Workgroup "
                                         "storage class, but shaderSharedFloat16Atomics was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 32) && (enabled_features.shaderSharedFloat32Atomics == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06338", module_state.handle(), loc,
                                         "SPIR-V is using 32-bit float atomics for load/store/exhange operations with Workgroup "
                                         "storage class, but shaderSharedFloat32Atomics was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 64) && (enabled_features.shaderSharedFloat64Atomics == VK_FALSE)) {
                        skip |= LogError("VUID-RuntimeSpirv-None-06339", module_state.handle(), loc,
                                         "SPIR-V is using 64-bit float atomics for load/store/exhange operations with Workgroup "
                                         "storage class, but shaderSharedFloat64Atomics was not enabled.\n%s\n",
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    }
                }
            } else if ((atomic.storage_class == spv::StorageClassImage) && (valid_image_float == false)) {
                skip |= LogError("VUID-RuntimeSpirv-None-06286", module_state.handle(), loc,
                                 "SPIR-V is using float atomics operations with Image storage class, but none of the required "
                                 "features were enabled.\n%s\n",
                                 module_state.DescribeInstruction(atomic_def).c_str());
            } else if ((atomic.bit_width == 16) && (valid_16_float == false)) {
                skip |= LogError(
                    "VUID-RuntimeSpirv-None-06337", module_state.handle(), loc,
                    "SPIR-V is using 16-bit float atomics operations but none of the required features were enabled.\n%s\n",
                    module_state.DescribeInstruction(atomic_def).c_str());
            } else if ((atomic.bit_width == 32) && (valid_32_float == false)) {
                skip |= LogError(
                    "VUID-RuntimeSpirv-None-06338", module_state.handle(), loc,
                    "SPIR-V is using 32-bit float atomics operations but none of the required features were enabled.\n%s\n",
                    module_state.DescribeInstruction(atomic_def).c_str());
            } else if ((atomic.bit_width == 64) && (valid_64_float == false)) {
                skip |= LogError(
                    "VUID-RuntimeSpirv-None-06339", module_state.handle(), loc,
                    "SPIR-V is using 64-bit float atomics operations but snone of the required features were enabled.\n%s\n",
                    module_state.DescribeInstruction(atomic_def).c_str());
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateShaderFloatControl(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                            const spirv::StatelessData &stateless_data, const Location &loc) const {
    bool skip = false;

    // Need to wrap otherwise phys_dev_props_core12 can be junk
    if (!IsExtEnabled(extensions.vk_khr_shader_float_controls)) {
        return skip;
    }

    if (entrypoint.execution_mode.Has(spirv::ExecutionModeSet::signed_zero_inf_nan_preserve_width_16) &&
        !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat16) {
        skip |= LogError("VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat16-06293", module_state.handle(), loc,
                         "SPIR-V requires SignedZeroInfNanPreserve for bit width 16 but it is not enabled on the device.");
    } else if (entrypoint.execution_mode.Has(spirv::ExecutionModeSet::signed_zero_inf_nan_preserve_width_32) &&
               !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat32) {
        skip |= LogError("VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat32-06294", module_state.handle(), loc,
                         "SPIR-V requires SignedZeroInfNanPreserve for bit width 32 but it is not enabled on the device.");
    } else if (entrypoint.execution_mode.Has(spirv::ExecutionModeSet::signed_zero_inf_nan_preserve_width_64) &&
               !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat64) {
        skip |= LogError("VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat64-06295", module_state.handle(), loc,
                         "SPIR-V requires SignedZeroInfNanPreserve for bit width 64 but it is not enabled on the device.");
    }

    const bool has_denorm_preserve_width_16 = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::denorm_preserve_width_16);
    const bool has_denorm_preserve_width_32 = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::denorm_preserve_width_32);
    const bool has_denorm_preserve_width_64 = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::denorm_preserve_width_64);
    if (has_denorm_preserve_width_16 && !phys_dev_props_core12.shaderDenormPreserveFloat16) {
        skip |= LogError("VUID-RuntimeSpirv-shaderDenormPreserveFloat16-06296", module_state.handle(), loc,
                         "SPIR-V requires DenormPreserve for bit width 16 but it is not enabled on the device.");
    } else if (has_denorm_preserve_width_32 && !phys_dev_props_core12.shaderDenormPreserveFloat32) {
        skip |= LogError("VUID-RuntimeSpirv-shaderDenormPreserveFloat32-06297", module_state.handle(), loc,
                         "SPIR-V requires DenormPreserve for bit width 32 but it is not enabled on the device.");
    } else if (has_denorm_preserve_width_64 && !phys_dev_props_core12.shaderDenormPreserveFloat64) {
        skip |= LogError("VUID-RuntimeSpirv-shaderDenormPreserveFloat64-06298", module_state.handle(), loc,
                         "SPIR-V requires DenormPreserve for bit width 64 but it is not enabled on the device.");
    }

    const bool has_denorm_flush_to_zero_width_16 =
        entrypoint.execution_mode.Has(spirv::ExecutionModeSet::denorm_flush_to_zero_width_16);
    const bool has_denorm_flush_to_zero_width_32 =
        entrypoint.execution_mode.Has(spirv::ExecutionModeSet::denorm_flush_to_zero_width_32);
    const bool has_denorm_flush_to_zero_width_64 =
        entrypoint.execution_mode.Has(spirv::ExecutionModeSet::denorm_flush_to_zero_width_64);
    if (has_denorm_flush_to_zero_width_16 && !phys_dev_props_core12.shaderDenormFlushToZeroFloat16) {
        skip |= LogError("VUID-RuntimeSpirv-shaderDenormFlushToZeroFloat16-06299", module_state.handle(), loc,
                         "SPIR-V requires DenormFlushToZero for bit width 16 but it is not enabled on the device.");
    } else if (has_denorm_flush_to_zero_width_32 && !phys_dev_props_core12.shaderDenormFlushToZeroFloat32) {
        skip |= LogError("VUID-RuntimeSpirv-shaderDenormFlushToZeroFloat32-06300", module_state.handle(), loc,
                         "SPIR-V requires DenormFlushToZero for bit width 32 but it is not enabled on the device.");
    } else if (has_denorm_flush_to_zero_width_64 && !phys_dev_props_core12.shaderDenormFlushToZeroFloat64) {
        skip |= LogError("VUID-RuntimeSpirv-shaderDenormFlushToZeroFloat64-06301", module_state.handle(), loc,
                         "SPIR-V requires DenormFlushToZero for bit width 64 but it is not enabled on the device.");
    }

    const bool has_rounding_mode_rte_width_16 = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::rounding_mode_rte_width_16);
    const bool has_rounding_mode_rte_width_32 = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::rounding_mode_rte_width_32);
    const bool has_rounding_mode_rte_width_64 = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::rounding_mode_rte_width_64);
    if (has_rounding_mode_rte_width_16 && !phys_dev_props_core12.shaderRoundingModeRTEFloat16) {
        skip |= LogError("VUID-RuntimeSpirv-shaderRoundingModeRTEFloat16-06302", module_state.handle(), loc,
                         "SPIR-V requires RoundingModeRTE for bit width 16 but it is not enabled on the device.");
    } else if (has_rounding_mode_rte_width_32 && !phys_dev_props_core12.shaderRoundingModeRTEFloat32) {
        skip |= LogError("VUID-RuntimeSpirv-shaderRoundingModeRTEFloat32-06303", module_state.handle(), loc,
                         "SPIR-V requires RoundingModeRTE for bit width 32 but it is not enabled on the device.");
    } else if (has_rounding_mode_rte_width_64 && !phys_dev_props_core12.shaderRoundingModeRTEFloat64) {
        skip |= LogError("VUID-RuntimeSpirv-shaderRoundingModeRTEFloat64-06304", module_state.handle(), loc,
                         "SPIR-V requires RoundingModeRTE for bit width 64 but it is not enabled on the device.");
    }

    const bool has_rounding_mode_rtz_width_16 = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::rounding_mode_rtz_width_16);
    const bool has_rounding_mode_rtz_width_32 = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::rounding_mode_rtz_width_32);
    const bool has_rounding_mode_rtz_width_64 = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::rounding_mode_rtz_width_64);
    if (has_rounding_mode_rtz_width_16 && !phys_dev_props_core12.shaderRoundingModeRTZFloat16) {
        skip |= LogError("VUID-RuntimeSpirv-shaderRoundingModeRTZFloat16-06305", module_state.handle(), loc,
                         "SPIR-V requires RoundingModeRTZ for bit width 16 but it is not enabled on the device.");
    } else if (has_rounding_mode_rtz_width_32 && !phys_dev_props_core12.shaderRoundingModeRTZFloat32) {
        skip |= LogError("VUID-RuntimeSpirv-shaderRoundingModeRTZFloat32-06306", module_state.handle(), loc,
                         "SPIR-V requires RoundingModeRTZ for bit width 32 but it is not enabled on the device.");
    } else if (has_rounding_mode_rtz_width_64 && !phys_dev_props_core12.shaderRoundingModeRTZFloat64) {
        skip |= LogError("VUID-RuntimeSpirv-shaderRoundingModeRTZFloat64-06307", module_state.handle(), loc,
                         "SPIR-V requires RoundingModeRTZ for bit width 64 but it is not enabled on the device.");
    }

    auto get_float_width = [&module_state](uint32_t id) {
        const auto *insn = module_state.FindDef(id);
        if (!insn || insn->Opcode() != spv::OpTypeFloat) {
            return 0u;
        } else {
            return insn->Word(2);
        }
    };

    const uint32_t mask = spv::FPFastMathModeNotNaNMask | spv::FPFastMathModeNotInfMask | spv::FPFastMathModeNSZMask;
    if (entrypoint.execution_mode.Has(spirv::ExecutionModeSet::fp_fast_math_default)) {
        for (const spirv::Instruction *insn : stateless_data.execution_mode_id_inst) {
            const uint32_t mode = insn->Word(2);
            if (mode != spv::ExecutionModeFPFastMathDefault) {
                continue;
            }

            // spirv-val will catch if this is not a constant
            const auto *fast_math_mode = module_state.FindDef(insn->Word(4));
            const bool has_mask = (fast_math_mode->GetConstantValue() & mask) == mask;
            if (has_mask) {
                continue;  // nothing to validate
            }

            const uint32_t bit_width = get_float_width(insn->Word(3));

            if (bit_width == 16 && !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat16) {
                skip |= LogError("VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat16-09559", module_state.handle(), loc,
                                 "shaderSignedZeroInfNanPreserveFloat16 is false, but FPFastMathDefault is setting 16-bit floats "
                                 "with modes 0x%" PRIx32 ".",
                                 fast_math_mode->GetConstantValue());
            } else if (bit_width == 32 && !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat32) {
                skip |= LogError("VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat32-09561", module_state.handle(), loc,
                                 "shaderSignedZeroInfNanPreserveFloat32 is false, but FPFastMathDefault is setting 32-bit floats "
                                 "with modes 0x%" PRIx32 ".",
                                 fast_math_mode->GetConstantValue());
            } else if (bit_width == 64 && !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat64) {
                skip |= LogError("VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat64-09563", module_state.handle(), loc,
                                 "shaderSignedZeroInfNanPreserveFloat64 is false, but FPFastMathDefault is setting 64-bit floats "
                                 "with modes 0x%" PRIx32 ".",
                                 fast_math_mode->GetConstantValue());
            }
        }
    }

    for (const spirv::Instruction *insn : module_state.static_data_.decoration_inst) {
        uint32_t decoration = insn->Word(2);
        if (decoration != spv::DecorationFPFastMathMode) {
            continue;
        }
        uint32_t modes = insn->Word(3);
        const bool has_mask = (modes & mask) == mask;
        if (has_mask) {
            continue;  // nothing to validate
        }

        // See if target instruction uses any floats of various bit widths
        bool float_16 = false;
        bool float_32 = false;
        bool float_64 = false;
        uint32_t operand_index = 2;  // if using OpDecoration, this instruction must have a ResultId

        const auto *target_insn = module_state.FindDef(insn->Word(1));
        if (target_insn->TypeId() != 0) {
            operand_index++;
            const uint32_t bit_width = get_float_width(target_insn->TypeId());
            float_16 |= bit_width == 16;
            float_32 |= bit_width == 32;
            float_64 |= bit_width == 64;
        }

        for (; operand_index < target_insn->Length(); operand_index++) {
            const uint32_t bit_width = get_float_width(target_insn->Word(operand_index));
            float_16 |= bit_width == 16;
            float_32 |= bit_width == 32;
            float_64 |= bit_width == 64;
        }

        if (float_16 && !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat16) {
            skip |= LogError("VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat16-09560", module_state.handle(), loc,
                             "shaderSignedZeroInfNanPreserveFloat16 is false, FPFastMathMode has modes 0x%" PRIx32
                             " but the target instruction is using 16-bit floats.\n%s\n",
                             modes, module_state.DescribeInstruction(*target_insn).c_str());
        } else if (float_32 && !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat32) {
            skip |= LogError("VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat32-09562", module_state.handle(), loc,
                             "shaderSignedZeroInfNanPreserveFloat32 is false, FPFastMathMode has modes 0x%" PRIx32
                             " but the target instruction is using 32-bit floats.\n%s\n",
                             modes, module_state.DescribeInstruction(*target_insn).c_str());
        } else if (float_64 && !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat64) {
            skip |= LogError("VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat64-09564", module_state.handle(), loc,
                             "shaderSignedZeroInfNanPreserveFloat64 is false, FPFastMathMode has modes 0x%" PRIx32
                             " but the target instruction is using 64-bit floats.\n%s\n",
                             modes, module_state.DescribeInstruction(*target_insn).c_str());
        }
    }

    return skip;
}

bool CoreChecks::ValidatePhysicalStorageBuffers(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                                const Location &loc) const {
    bool skip = false;

    // https://gitlab.khronos.org/spirv/SPIR-V/-/issues/779
    // This will likely not get resolved for awhile as it will be a LOT of testing and currently it "just works" for simple cases,
    // but should warn users if trying to do this.
    // We need to check at vkCreateShaderModule time as we have found some drivers will crash here (from our VVL tests).
    if (entrypoint.has_physical_storage_buffer_interface) {
        skip |= LogWarning(
            "WARNING-PhysicalStorageBuffer-interface", module_state.handle(), loc,
            "(SPIR-V Interface) Is trying to use PhysicalStorageBuffer as an Input/Output User-Defined Variable in (%s). This "
            "has unresolved specification discussion and is undefined and caution should be taken. Advice is to use "
            "int64 or uvec2 instead to pass the pointer betweeen stages.",
            string_VkShaderStageFlagBits(entrypoint.stage));
    }

    return skip;
}

bool CoreChecks::ValidateExecutionModes(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                        const spirv::StatelessData &stateless_data, const Location &loc) const {
    bool skip = false;
    const VkShaderStageFlagBits stage = entrypoint.stage;

    if (entrypoint.execution_mode.Has(spirv::ExecutionModeSet::local_size_id_bit)) {
        // Special case to print error by extension and feature bit
        if (!enabled_features.maintenance4) {
            skip |= LogError("VUID-RuntimeSpirv-LocalSizeId-06434", module_state.handle(), loc,
                             "SPIR-V OpExecutionMode LocalSizeId is used but maintenance4 feature was not enabled.");
        }
        if (!IsExtEnabled(extensions.vk_khr_maintenance4)) {
            skip |= LogError("VUID-RuntimeSpirv-LocalSizeId-06434", module_state.handle(), loc,
                             "SPIR-V OpExecutionMode LocalSizeId is used but maintenance4 extension is not enabled and used "
                             "Vulkan api version is 1.2 or less.");
        }
    }

    if (entrypoint.execution_mode.Has(spirv::ExecutionModeSet::subgroup_uniform_control_flow_bit)) {
        if (!enabled_features.shaderSubgroupUniformControlFlow ||
            (phys_dev_ext_props.subgroup_props.supportedStages & stage) == 0 || stateless_data.has_invocation_repack_instruction) {
            std::stringstream msg;
            if (!enabled_features.shaderSubgroupUniformControlFlow) {
                msg << "shaderSubgroupUniformControlFlow feature must be enabled";
            } else if ((phys_dev_ext_props.subgroup_props.supportedStages & stage) == 0) {
                msg << "stage" << string_VkShaderStageFlagBits(stage)
                    << " must be in VkPhysicalDeviceSubgroupProperties::supportedStages("
                    << string_VkShaderStageFlags(phys_dev_ext_props.subgroup_props.supportedStages) << ")";
            } else {
                msg << "the shader must not use any invocation repack instructions";
            }
            skip |= LogError("VUID-RuntimeSpirv-SubgroupUniformControlFlowKHR-06379", module_state.handle(), loc,
                             "SPIR-V uses ExecutionModeSubgroupUniformControlFlowKHR, but %s.", msg.str().c_str());
        }
    }

    if ((stage == VK_SHADER_STAGE_MESH_BIT_EXT || stage == VK_SHADER_STAGE_TASK_BIT_EXT) &&
        !phys_dev_ext_props.compute_shader_derivatives_props.meshAndTaskShaderDerivatives) {
        if (entrypoint.execution_mode.Has(spirv::ExecutionModeSet::derivative_group_linear)) {
            skip |=
                LogError("VUID-RuntimeSpirv-meshAndTaskShaderDerivatives-10153", module_state.handle(), loc,
                         "SPIR-V uses DerivativeGroupLinearKHR in a %s shader, but meshAndTaskShaderDerivatives is not supported.",
                         string_VkShaderStageFlagBits(stage));
        } else if (entrypoint.execution_mode.Has(spirv::ExecutionModeSet::derivative_group_quads)) {
            skip |=
                LogError("VUID-RuntimeSpirv-meshAndTaskShaderDerivatives-10153", module_state.handle(), loc,
                         "SPIR-V uses DerivativeGroupQuadsKHR in a %s shader, but meshAndTaskShaderDerivatives is not supported.",
                         string_VkShaderStageFlagBits(stage));
        }
    }

    return skip;
}

bool CoreChecks::ValidateShaderExecutionModes(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                              VkShaderStageFlagBits stage, const vvl::Pipeline *pipeline,
                                              const Location &loc) const {
    bool skip = false;

    if (entrypoint.stage == VK_SHADER_STAGE_GEOMETRY_BIT) {
        const uint32_t vertices_out = entrypoint.execution_mode.output_vertices;
        const uint32_t invocations = entrypoint.execution_mode.invocations;
        if (vertices_out == 0 || vertices_out > phys_dev_props.limits.maxGeometryOutputVertices) {
            const char *vuid =
                pipeline ? "VUID-VkPipelineShaderStageCreateInfo-stage-00714" : "VUID-VkShaderCreateInfoEXT-pCode-08454";
            skip |= LogError(vuid, module_state.handle(), loc,
                             "SPIR-V (Geometry stage) entry point must have an OpExecutionMode instruction that "
                             "specifies a maximum output vertex count that is greater than 0 and less "
                             "than or equal to maxGeometryOutputVertices. "
                             "OutputVertices=%" PRIu32 ", maxGeometryOutputVertices=%" PRIu32 ".",
                             vertices_out, phys_dev_props.limits.maxGeometryOutputVertices);
        }

        if (invocations == 0 || invocations > phys_dev_props.limits.maxGeometryShaderInvocations) {
            const char *vuid =
                pipeline ? "VUID-VkPipelineShaderStageCreateInfo-stage-00715" : "VUID-VkShaderCreateInfoEXT-pCode-08455";
            skip |= LogError(vuid, module_state.handle(), loc,
                             "SPIR-V (Geometry stage) entry point must have an OpExecutionMode instruction that "
                             "specifies an invocation count that is greater than 0 and less "
                             "than or equal to maxGeometryShaderInvocations. "
                             "Invocations=%" PRIu32 ", maxGeometryShaderInvocations=%" PRIu32 ".",
                             invocations, phys_dev_props.limits.maxGeometryShaderInvocations);
        }
    } else if (entrypoint.stage == VK_SHADER_STAGE_FRAGMENT_BIT &&
               entrypoint.execution_mode.Has(spirv::ExecutionModeSet::early_fragment_test_bit)) {
        if (pipeline) {
            const auto *ds_state = pipeline->DepthStencilState();
            if ((ds_state &&
                 (ds_state->flags &
                  (VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_EXT |
                   VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_EXT)) != 0)) {
                skip |= LogError(
                    "VUID-VkGraphicsPipelineCreateInfo-flags-06591", module_state.handle(), loc,
                    "SPIR-V (Fragment stage) enables early fragment tests, but VkPipelineDepthStencilStateCreateInfo::flags == "
                    "%s.",
                    string_VkPipelineDepthStencilStateCreateFlags(ds_state->flags).c_str());
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidatePointSizeShaderState(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                              const vvl::Pipeline &pipeline, VkShaderStageFlagBits stage,
                                              const Location &loc) const {
    bool skip = false;
    // vkspec.html#primsrast-points describes which is the final stage that needs to check for points
    //
    // Vertex - Need to read input topology in pipeline
    // Geo/Tess - Need to know the feature bit is on
    // Mesh - are checked in spirv-val as they don't require any runtime information
    if (!IsValueIn(stage,
                   {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT})) {
        return skip;
    }

    const bool output_points = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::output_points_bit);
    const bool point_mode = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::point_mode_bit);
    const bool maintenance5 = enabled_features.maintenance5;

    if (stage == VK_SHADER_STAGE_GEOMETRY_BIT && output_points) {
        if (enabled_features.shaderTessellationAndGeometryPointSize && !entrypoint.written_builtin_point_size &&
            entrypoint.emit_vertex_geometry && !maintenance5) {
            skip |= LogError(
                "VUID-VkGraphicsPipelineCreateInfo-shaderTessellationAndGeometryPointSize-08776", module_state.handle(), loc,
                "SPIR-V (Geometry stage) PointSize is not written, but shaderTessellationAndGeometryPointSize was enabled.");
        } else if (!enabled_features.shaderTessellationAndGeometryPointSize && entrypoint.written_builtin_point_size) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-Geometry-07726", module_state.handle(), loc,
                             "SPIR-V (Geometry stage) PointSize is written to, but shaderTessellationAndGeometryPointSize was not "
                             "enabled (gl_PointSize must NOT be written and a default of 1.0 is assumed).");
        }
    } else if (stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT &&
               ((pipeline.create_info_shaders & VK_SHADER_STAGE_GEOMETRY_BIT) == 0) && point_mode) {
        if (enabled_features.shaderTessellationAndGeometryPointSize && !entrypoint.written_builtin_point_size && !maintenance5) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-TessellationEvaluation-07723", module_state.handle(), loc,
                             "SPIR-V (Tessellation Evaluation stage) PointSize is not written, but "
                             "shaderTessellationAndGeometryPointSize was enabled.");
        } else if (!enabled_features.shaderTessellationAndGeometryPointSize && entrypoint.written_builtin_point_size) {
            skip |=
                LogError("VUID-VkGraphicsPipelineCreateInfo-TessellationEvaluation-07724", module_state.handle(), loc,
                         "SPIR-V (Tessellation Evaluation stage) PointSize is written to, shaderTessellationAndGeometryPointSize "
                         "was not enabled (gl_PointSize must NOT be written and a default of 1.0 is assumed).");
        }
    } else if (stage == VK_SHADER_STAGE_VERTEX_BIT &&
               ((pipeline.create_info_shaders & (VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_GEOMETRY_BIT)) ==
                0) &&
               pipeline.topology_at_rasterizer == VK_PRIMITIVE_TOPOLOGY_POINT_LIST) {
        const bool ignore_topology = pipeline.IsDynamic(CB_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY) &&
                                     phys_dev_ext_props.extended_dynamic_state3_props.dynamicPrimitiveTopologyUnrestricted;
        if (!entrypoint.written_builtin_point_size && !ignore_topology && !maintenance5) {
            skip |= LogError(
                "VUID-VkGraphicsPipelineCreateInfo-topology-08773", module_state.handle(), loc,
                "SPIR-V (Vertex) PointSize is not written to, but Pipeline topology is set to VK_PRIMITIVE_TOPOLOGY_POINT_LIST.");
        }
    }

    return skip;
}

bool CoreChecks::ValidatePrimitiveRateShaderState(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                                  const vvl::Pipeline &pipeline, VkShaderStageFlagBits stage,
                                                  const Location &loc) const {
    bool skip = false;

    const auto viewport_state = pipeline.ViewportState();
    if (!phys_dev_ext_props.fragment_shading_rate_props.primitiveFragmentShadingRateWithMultipleViewports &&
        (pipeline.pipeline_type == VK_PIPELINE_BIND_POINT_GRAPHICS) && viewport_state) {
        if (!pipeline.IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT) && viewport_state->viewportCount > 1 &&
            entrypoint.written_builtin_primitive_shading_rate_khr) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04503",
                             module_state.handle(), loc,
                             "SPIR-V (%s) statically writes to PrimitiveShadingRateKHR built-in, but "
                             "multiple viewports "
                             "are used and the primitiveFragmentShadingRateWithMultipleViewports limit is not supported.",
                             string_VkShaderStageFlagBits(stage));
        }

        if (entrypoint.written_builtin_primitive_shading_rate_khr && entrypoint.written_builtin_viewport_index) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04504",
                             module_state.handle(), loc,
                             "SPIR-V (%s) statically writes to both PrimitiveShadingRateKHR and "
                             "ViewportIndex built-ins, "
                             "but the primitiveFragmentShadingRateWithMultipleViewports limit is not supported.",
                             string_VkShaderStageFlagBits(stage));
        }

        if (entrypoint.written_builtin_primitive_shading_rate_khr && entrypoint.written_builtin_viewport_mask_nv) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04505",
                             module_state.handle(), loc,
                             "SPIR-V (%s) statically writes to both PrimitiveShadingRateKHR and "
                             "ViewportMaskNV built-ins, "
                             "but the primitiveFragmentShadingRateWithMultipleViewports limit is not supported.",
                             string_VkShaderStageFlagBits(stage));
        }
    }
    return skip;
}

bool CoreChecks::ValidateTransformFeedbackDecorations(const spirv::Module &module_state, const Location &loc) const {
    bool skip = false;

    std::vector<const spirv::Instruction *> xfb_streams;
    std::vector<const spirv::Instruction *> xfb_buffers;
    std::vector<const spirv::Instruction *> xfb_offsets;

    for (const spirv::Instruction *op_decorate : module_state.static_data_.decoration_inst) {
        uint32_t decoration = op_decorate->Word(2);
        if (decoration == spv::DecorationXfbStride) {
            uint32_t stride = op_decorate->Word(3);
            if (stride > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataStride) {
                skip |= LogError("VUID-RuntimeSpirv-XfbStride-06313", module_state.handle(), loc,
                                 "SPIR-V uses transform feedback with xfb_stride (%" PRIu32
                                 ") greater than maxTransformFeedbackBufferDataStride (%" PRIu32 ").",
                                 stride, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataStride);
            }
        }
        if (decoration == spv::DecorationStream) {
            xfb_streams.push_back(op_decorate);
            uint32_t stream = op_decorate->Word(3);
            if (stream >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams) {
                skip |= LogError("VUID-RuntimeSpirv-Stream-06312", module_state.handle(), loc,
                                 "SPIR-V uses transform feedback with stream (%" PRIu32
                                 ") not less than maxTransformFeedbackStreams (%" PRIu32 ").",
                                 stream, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams);
            }
        }
        if (decoration == spv::DecorationXfbBuffer) {
            xfb_buffers.push_back(op_decorate);
        }
        if (decoration == spv::DecorationOffset) {
            xfb_offsets.push_back(op_decorate);
        }
    }

    // XfbBuffer, buffer data size
    std::vector<std::pair<uint32_t, uint32_t>> buffer_data_sizes;
    for (const spirv::Instruction *op_decorate : xfb_offsets) {
        for (const spirv::Instruction *xfb_buffer : xfb_buffers) {
            if (xfb_buffer->Word(1) == op_decorate->Word(1)) {
                const auto offset = op_decorate->Word(3);
                const spirv::Instruction *def = module_state.FindDef(xfb_buffer->Word(1));
                const auto size = module_state.GetTypeBytesSize(def);
                const uint32_t buffer_data_size = offset + size;
                if (buffer_data_size > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataSize) {
                    skip |= LogError(
                        "VUID-RuntimeSpirv-Offset-06308", module_state.handle(), loc,
                        "SPIR-V uses transform feedback with xfb_offset (%" PRIu32 ") + size of variable (%" PRIu32
                        ") greater than VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackBufferDataSize "
                        "(%" PRIu32 ").",
                        offset, size, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataSize);
                }

                bool found = false;
                for (auto &bds : buffer_data_sizes) {
                    if (bds.first == xfb_buffer->Word(1)) {
                        bds.second = std::max(bds.second, buffer_data_size);
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    buffer_data_sizes.emplace_back(xfb_buffer->Word(1), buffer_data_size);
                }

                break;
            }
        }
    }

    vvl::unordered_map<uint32_t, uint32_t> stream_data_size;
    for (const spirv::Instruction *xfb_stream : xfb_streams) {
        for (const auto &bds : buffer_data_sizes) {
            if (xfb_stream->Word(1) == bds.first) {
                uint32_t stream = xfb_stream->Word(3);
                const auto itr = stream_data_size.find(stream);
                if (itr != stream_data_size.end()) {
                    itr->second += bds.second;
                } else {
                    stream_data_size.insert({stream, bds.second});
                }
            }
        }
    }

    for (const auto &stream : stream_data_size) {
        if (stream.second > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreamDataSize) {
            skip |= LogError(
                "VUID-RuntimeSpirv-XfbBuffer-06309", module_state.handle(), loc,
                "SPIR-V uses transform feedback with stream (%" PRIu32 ") having the sum of buffer data sizes (%" PRIu32
                ") not less than VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackBufferDataSize "
                "(%" PRIu32 ").",
                stream.first, stream.second, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataSize);
        }
    }

    return skip;
}

bool CoreChecks::ValidateWorkgroupSharedMemory(const spirv::Module &module_state, VkShaderStageFlagBits stage,
                                               uint32_t total_workgroup_shared_memory, const Location &loc) const {
    bool skip = false;

    // If not found before with spec constants, find here
    if (total_workgroup_shared_memory == 0) {
        total_workgroup_shared_memory = module_state.CalculateWorkgroupSharedMemory();
    }

    switch (stage) {
        case VK_SHADER_STAGE_COMPUTE_BIT: {
            if (total_workgroup_shared_memory > phys_dev_props.limits.maxComputeSharedMemorySize) {
                skip |= LogError("VUID-RuntimeSpirv-Workgroup-06530", module_state.handle(), loc,
                                 "SPIR-V uses %" PRIu32
                                 " bytes of shared memory, which is more than maxComputeSharedMemorySize (%" PRIu32 ").",
                                 total_workgroup_shared_memory, phys_dev_props.limits.maxComputeSharedMemorySize);
            }

            if (enabled_features.cooperativeMatrixWorkgroupScope) {
                bool usesWorkgroupScope = false;
                for (auto &cooperative_matrix_inst : module_state.static_data_.cooperative_matrix_inst) {
                    if (cooperative_matrix_inst->Opcode() == spv::OpTypeCooperativeMatrixKHR) {
                        if (auto scope = module_state.GetConstantDef(cooperative_matrix_inst->Word(3))) {
                            if (scope->GetConstantValue() == VK_SCOPE_WORKGROUP_KHR) {
                                usesWorkgroupScope = true;
                                break;
                            }
                        }
                    }
                }
                if (usesWorkgroupScope) {
                    if (total_workgroup_shared_memory >
                        phys_dev_props.limits.maxComputeSharedMemorySize -
                            phys_dev_ext_props.cooperative_matrix_props2_nv.cooperativeMatrixWorkgroupScopeReservedSharedMemory) {
                        skip |= LogError(
                            "VUID-RuntimeSpirv-maxComputeSharedMemorySize-10168", module_state.handle(), loc,
                            "SPIR-V uses %" PRIu32
                            " bytes of shared memory, which is more than maxComputeSharedMemorySize (%" PRIu32
                            ") minus "
                            "cooperativeMatrixWorkgroupScopeReservedSharedMemory (%" PRIu32 ").",
                            total_workgroup_shared_memory, phys_dev_props.limits.maxComputeSharedMemorySize,
                            phys_dev_ext_props.cooperative_matrix_props2_nv.cooperativeMatrixWorkgroupScopeReservedSharedMemory);
                    }
                }
            }

            break;
        }
        case VK_SHADER_STAGE_MESH_BIT_EXT: {
            if (total_workgroup_shared_memory > phys_dev_ext_props.mesh_shader_props_ext.maxMeshSharedMemorySize) {
                skip |= LogError("VUID-RuntimeSpirv-maxMeshSharedMemorySize-08754", module_state.handle(), loc,
                                 "SPIR-V uses %" PRIu32
                                 " bytes of shared memory, which is more than maxMeshSharedMemorySize (%" PRIu32 ").",
                                 total_workgroup_shared_memory, phys_dev_ext_props.mesh_shader_props_ext.maxMeshSharedMemorySize);
            }
            break;
        }
        case VK_SHADER_STAGE_TASK_BIT_EXT: {
            if (total_workgroup_shared_memory > phys_dev_ext_props.mesh_shader_props_ext.maxTaskSharedMemorySize) {
                skip |= LogError("VUID-RuntimeSpirv-maxTaskSharedMemorySize-08759", module_state.handle(), loc,
                                 "SPIR-V uses %" PRIu32
                                 " bytes of shared memory, which is more than maxTaskSharedMemorySize (%" PRIu32 ").",
                                 total_workgroup_shared_memory, phys_dev_ext_props.mesh_shader_props_ext.maxTaskSharedMemorySize);
            }
            break;
        }
        default:
            assert(false);  // other stages should not have called this function
            break;
    }

    return skip;
}

// Temporary data of a OpVariable when validating it.
// If found useful in another location, can move out to the header
struct VariableInstInfo {
    bool has_8bit = false;
    bool has_16bit = false;
};

// easier to use recursion to traverse the OpTypeStruct
static void GetVariableInfo(const spirv::Module &module_state, const spirv::Instruction *insn, VariableInstInfo &info) {
    if (!insn) {
        return;
    } else if (insn->Opcode() == spv::OpTypePointer) {
        return;
    } else if (insn->Opcode() == spv::OpTypeFloat || insn->Opcode() == spv::OpTypeInt) {
        const uint32_t bit_width = insn->Word(2);
        info.has_8bit |= (bit_width == 8);
        info.has_16bit |= (bit_width == 16);
    } else if (insn->Opcode() == spv::OpTypeStruct) {
        for (uint32_t i = 2; i < insn->Length(); i++) {
            const spirv::Instruction *member_insn = module_state.FindDef(insn->Word(i));
            if (member_insn->StorageClass() == spv::StorageClassPhysicalStorageBuffer) {
                continue;  // a uint8 pointer is not a 8-bit element
            }
            const uint32_t base_insn_id = module_state.GetBaseType(member_insn);
            const spirv::Instruction *base_insn = module_state.FindDef(base_insn_id);
            GetVariableInfo(module_state, base_insn, info);
        }
    }
}

// This is to validate the VK_KHR_8bit_storage and VK_KHR_16bit_storage extensions
bool CoreChecks::Validate8And16BitStorage(const spirv::Module &module_state, const spirv::Instruction &var_insn,
                                          const Location &loc) const {
    bool skip = false;

    const uint32_t storage_class = var_insn.StorageClass();

    const spirv::Instruction *type_pointer = module_state.FindDef(var_insn.Word(1));
    const spirv::Instruction *type = module_state.FindDef(type_pointer->Word(3));
    // type will either be a float, int, or struct and if struct need to traverse it
    VariableInstInfo info;
    GetVariableInfo(module_state, type, info);

    if (info.has_8bit) {
        if (!enabled_features.storageBuffer8BitAccess &&
            (storage_class == spv::StorageClassStorageBuffer || storage_class == spv::StorageClassShaderRecordBufferKHR ||
             storage_class == spv::StorageClassPhysicalStorageBuffer)) {
            skip |= LogError("VUID-RuntimeSpirv-storageBuffer8BitAccess-06328", module_state.handle(), loc,
                             "SPIR-V contains an 8-bit "
                             "OpVariable with %s Storage Class, but storageBuffer8BitAccess was not enabled.\n%s\n",
                             string_SpvStorageClass(storage_class), var_insn.Describe().c_str());
        }
        if (!enabled_features.uniformAndStorageBuffer8BitAccess && storage_class == spv::StorageClassUniform) {
            skip |= LogError(
                "VUID-RuntimeSpirv-uniformAndStorageBuffer8BitAccess-06329", module_state.handle(), loc,
                "SPIR-V contains an "
                "8-bit OpVariable with Uniform Storage Class, but uniformAndStorageBuffer8BitAccess was not enabled.\n%s\n",
                var_insn.Describe().c_str());
        }
        if (!enabled_features.storagePushConstant8 && storage_class == spv::StorageClassPushConstant) {
            skip |= LogError("VUID-RuntimeSpirv-storagePushConstant8-06330", module_state.handle(), loc,
                             "SPIR-V contains an 8-bit "
                             "OpVariable with PushConstant Storage Class, but storagePushConstant8 was not enabled.\n%s\n",
                             var_insn.Describe().c_str());
        }
    }

    if (info.has_16bit) {
        if (!enabled_features.storageBuffer16BitAccess &&
            (storage_class == spv::StorageClassStorageBuffer || storage_class == spv::StorageClassShaderRecordBufferKHR ||
             storage_class == spv::StorageClassPhysicalStorageBuffer)) {
            skip |= LogError("VUID-RuntimeSpirv-storageBuffer16BitAccess-06331", module_state.handle(), loc,
                             "SPIR-V contains an 16-bit "
                             "OpVariable with %s Storage Class, but storageBuffer16BitAccess was not enabled.\n%s\n",
                             string_SpvStorageClass(storage_class), var_insn.Describe().c_str());
        }
        if (!enabled_features.uniformAndStorageBuffer16BitAccess && storage_class == spv::StorageClassUniform) {
            skip |= LogError(
                "VUID-RuntimeSpirv-uniformAndStorageBuffer16BitAccess-06332", module_state.handle(), loc,
                "SPIR-V contains an "
                "16-bit OpVariable with Uniform Storage Class, but uniformAndStorageBuffer16BitAccess was not enabled.\n%s\n",
                var_insn.Describe().c_str());
        }
        if (!enabled_features.storagePushConstant16 && storage_class == spv::StorageClassPushConstant) {
            skip |= LogError("VUID-RuntimeSpirv-storagePushConstant16-06333", module_state.handle(), loc,
                             "SPIR-V contains an 16-bit "
                             "OpVariable with PushConstant Storage Class, but storagePushConstant16 was not enabled.\n%s\n",
                             var_insn.Describe().c_str());
        }
        if (!enabled_features.storageInputOutput16 &&
            (storage_class == spv::StorageClassInput || storage_class == spv::StorageClassOutput)) {
            skip |= LogError("VUID-RuntimeSpirv-storageInputOutput16-06334", module_state.handle(), loc,
                             "SPIR-V contains an 16-bit "
                             "OpVariable with %s Storage Class, but storageInputOutput16 was not enabled.\n%s\n",
                             string_SpvStorageClass(storage_class), var_insn.Describe().c_str());
        }
    }

    return skip;
}

bool CoreChecks::ValidateVariables(const spirv::Module &module_state, const Location &loc) const {
    bool skip = false;

    for (const spirv::Instruction *insn : module_state.static_data_.variable_inst) {
        const uint32_t storage_class = insn->StorageClass();

        if (storage_class == spv::StorageClassWorkgroup) {
            // If Workgroup variable is initalized, make sure the feature is enabled
            if (insn->Length() > 4 && !enabled_features.shaderZeroInitializeWorkgroupMemory) {
                skip |= LogError("VUID-RuntimeSpirv-shaderZeroInitializeWorkgroupMemory-06372", module_state.handle(), loc,
                                 "SPIR-V contains an OpVariable with Workgroup Storage Class with an Initializer operand, but "
                                 "shaderZeroInitializeWorkgroupMemory was not enabled.\n%s\n.",
                                 insn->Describe().c_str());
            }
        }

        skip |= Validate8And16BitStorage(module_state, *insn, loc);

        // Checks based off shaderStorageImage(Read|Write)WithoutFormat are
        // disabled if VK_KHR_format_feature_flags2 is supported.
        //
        //   https://github.com/KhronosGroup/Vulkan-Docs/blob/6177645341afc/appendices/spirvenv.txt#L553
        //
        // The other checks need to take into account the format features and so
        // we apply that in the descriptor set matching validation code (see
        // descriptor_sets.cpp).
        if (!has_format_feature2) {
            skip |= ValidateShaderStorageImageFormatsVariables(module_state, *insn, loc);
        }
    }

    return skip;
}

bool CoreChecks::ValidateShaderInterfaceVariable(const spirv::Module &module_state,
                                                 const spirv::ResourceInterfaceVariable &variable,
                                                 vvl::unordered_set<uint32_t> &descriptor_type_set, const Location &loc) const {
    bool skip = false;

    if ((variable.is_storage_image || variable.is_storage_texel_buffer || variable.is_storage_buffer) &&
        !variable.decorations.Has(spirv::DecorationSet::nonwritable_bit)) {
        // If the variable is a struct, all members must contain NonWritable
        if (!variable.type_struct_info ||
            !variable.type_struct_info->decorations.AllMemberHave(spirv::DecorationSet::nonwritable_bit)) {
            switch (variable.stage) {
                case VK_SHADER_STAGE_FRAGMENT_BIT:
                    if (!enabled_features.fragmentStoresAndAtomics) {
                        skip |=
                            LogError("VUID-RuntimeSpirv-NonWritable-06340", module_state.handle(), loc,
                                     "SPIR-V (VK_SHADER_STAGE_FRAGMENT_BIT) uses descriptor %s (type %s) which is not "
                                     "marked with NonWritable, but fragmentStoresAndAtomics was not enabled.",
                                     variable.DescribeDescriptor().c_str(), string_DescriptorTypeSet(descriptor_type_set).c_str());
                    }
                    break;
                case VK_SHADER_STAGE_VERTEX_BIT:
                case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
                case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
                case VK_SHADER_STAGE_GEOMETRY_BIT:
                    if (!enabled_features.vertexPipelineStoresAndAtomics) {
                        skip |= LogError("VUID-RuntimeSpirv-NonWritable-06341", module_state.handle(), loc,
                                         "SPIR-V (%s) uses descriptor %s (type %s) which is not marked with NonWritable, but "
                                         "vertexPipelineStoresAndAtomics was not enabled.",
                                         string_VkShaderStageFlagBits(variable.stage), variable.DescribeDescriptor().c_str(),
                                         string_DescriptorTypeSet(descriptor_type_set).c_str());
                    }
                    break;
                default:
                    // No feature requirements for writes and atomics for other stages
                    break;
            }
        }
    }

    if (!variable.decorations.Has(spirv::DecorationSet::input_attachment_bit) && variable.info.image_dim == spv::DimSubpassData) {
        if (variable.array_length != 0) {
            skip |= LogError("VUID-RuntimeSpirv-OpTypeImage-09644", module_state.handle(), loc,
                             "the variable is an array of OpTypeImage with Dim::SubpassData, but it is missing the "
                             "InputAttachmentIndex decoration.\n%s\n",
                             variable.base_type.Describe().c_str());
        } else if (!enabled_features.dynamicRenderingLocalRead) {
            skip |= LogError("VUID-RuntimeSpirv-None-09558", module_state.handle(), loc,
                             "the variable is a OpTypeImage with Dim::SubpassData, but it is missing the "
                             "InputAttachmentIndex decoration (dynamicRenderingLocalRead was not enabled).\n%s\n",
                             variable.base_type.Describe().c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidateShaderInterfaceVariablePipeline(const spirv::Module &module_state, const vvl::Pipeline &pipeline,
                                                         const spirv::ResourceInterfaceVariable &variable,
                                                         vvl::unordered_set<uint32_t> &descriptor_type_set,
                                                         const Location &loc) const {
    bool skip = false;

    const LogObjectList objlist(module_state.handle(), pipeline.PipelineLayoutState()->Handle());

    const VkDescriptorSetLayoutBinding *binding = nullptr;
    // For given pipelineLayout verify that the set_layout_node at slot.first has the requested binding at slot.second and return
    // ptr to that binding
    {
        const vvl::PipelineLayout *pipeline_layout_state = pipeline.PipelineLayoutState().get();
        const uint32_t set = variable.decorations.set;
        if (pipeline_layout_state && set < pipeline_layout_state->set_layouts.size()) {
            binding =
                pipeline_layout_state->set_layouts[set]->GetDescriptorSetLayoutBindingPtrFromBinding(variable.decorations.binding);
        }
    }

    if (!binding) {
        skip |= LogError(GetPipelineInterfaceVariableVUID(pipeline, vvl::PipelineInterfaceVariableError::ShaderStage_07988),
                         objlist, loc, "SPIR-V (%s) uses descriptor %s (type %s) but was not declared in the pipeline layout.",
                         string_VkShaderStageFlagBits(variable.stage), variable.DescribeDescriptor().c_str(),
                         string_DescriptorTypeSet(descriptor_type_set).c_str());
    } else if (~binding->stageFlags & variable.stage) {
        skip |=
            LogError(GetPipelineInterfaceVariableVUID(pipeline, vvl::PipelineInterfaceVariableError::ShaderStage_07988), objlist,
                     loc, "SPIR-V (%s) uses descriptor %s (type %s) but the VkDescriptorSetLayoutBinding::stageFlags was %s.",
                     string_VkShaderStageFlagBits(variable.stage), variable.DescribeDescriptor().c_str(),
                     string_DescriptorTypeSet(descriptor_type_set).c_str(), string_VkShaderStageFlags(binding->stageFlags).c_str());
    } else if ((binding->descriptorType != VK_DESCRIPTOR_TYPE_MUTABLE_EXT) &&
               (descriptor_type_set.find(binding->descriptorType) == descriptor_type_set.end())) {
        skip |= LogError(GetPipelineInterfaceVariableVUID(pipeline, vvl::PipelineInterfaceVariableError::Mutable_07990), objlist,
                         loc, "SPIR-V (%s) uses descriptor %s of type %s but expected %s.",
                         string_VkShaderStageFlagBits(variable.stage), variable.DescribeDescriptor().c_str(),
                         string_VkDescriptorType(binding->descriptorType), string_DescriptorTypeSet(descriptor_type_set).c_str());
    } else if (binding->descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK && variable.array_length) {
        skip |=
            LogError(GetPipelineInterfaceVariableVUID(pipeline, vvl::PipelineInterfaceVariableError::Inline_10391), objlist, loc,
                     "SPIR-V (%s) uses descriptor %s as VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, but it is an array of descriptor.",
                     string_VkShaderStageFlagBits(variable.stage), variable.DescribeDescriptor().c_str());

    } else if (binding->descriptorCount < variable.array_length && variable.array_length != spirv::kRuntimeArray) {
        skip |= LogError(GetPipelineInterfaceVariableVUID(pipeline, vvl::PipelineInterfaceVariableError::DescriptorCount_07991),
                         objlist, loc,
                         "SPIR-V (%s) uses descriptor %s with a VkDescriptorSetLayoutBinding::descriptorCount of %" PRIu32
                         ", but requires at least %" PRIu32 " in the SPIR-V.",
                         string_VkShaderStageFlagBits(variable.stage), variable.DescribeDescriptor().c_str(),
                         binding->descriptorCount, variable.array_length);
    } else if (binding->descriptorCount == 0 && variable.array_length == spirv::kRuntimeArray) {
        skip |= LogError(GetPipelineInterfaceVariableVUID(pipeline, vvl::PipelineInterfaceVariableError::DescriptorCount_07991),
                         objlist, loc,
                         "SPIR-V (%s) uses a runtime descriptor array %s with a VkDescriptorSetLayoutBinding::descriptorCount of 0 "
                         "but requires at least 1 descriptor.",
                         string_VkShaderStageFlagBits(variable.stage), variable.DescribeDescriptor().c_str());
    }

    if (variable.decorations.Has(spirv::DecorationSet::input_attachment_bit)) {
        skip |= ValidateShaderInputAttachment(module_state, pipeline, variable, loc);
    }

    return skip;
}

bool CoreChecks::ValidateShaderInterfaceVariableShaderObject(const VkShaderCreateInfoEXT &create_info,
                                                             const spirv::ResourceInterfaceVariable &variable,
                                                             vvl::unordered_set<uint32_t> &descriptor_type_set,
                                                             const Location &loc) const {
    bool skip = false;
    const uint32_t set = variable.decorations.set;
    const VkDescriptorSetLayoutBinding *binding = nullptr;
    if (set < create_info.setLayoutCount) {
        auto descriptor_set_layout_state = Get<vvl::DescriptorSetLayout>(create_info.pSetLayouts[set]);
        if (descriptor_set_layout_state) {
            binding = descriptor_set_layout_state->GetDescriptorSetLayoutBindingPtrFromBinding(variable.decorations.binding);
        }
    }

    if (!binding) {
        skip |= LogError("VUID-VkShaderCreateInfoEXT-codeType-10383", device, loc,
                         "SPIR-V (%s) uses descriptor %s (type %s) but was not declared in pSetLayouts[%" PRIu32 "].",
                         string_VkShaderStageFlagBits(variable.stage), variable.DescribeDescriptor().c_str(),
                         string_DescriptorTypeSet(descriptor_type_set).c_str(), set);
    } else if (~binding->stageFlags & variable.stage) {
        skip |=
            LogError("VUID-VkShaderCreateInfoEXT-codeType-10383", device, loc,
                     "SPIR-V (%s) uses descriptor %s (type %s) but the VkDescriptorSetLayoutBinding::stageFlags was %s.",
                     string_VkShaderStageFlagBits(variable.stage), variable.DescribeDescriptor().c_str(),
                     string_DescriptorTypeSet(descriptor_type_set).c_str(), string_VkShaderStageFlags(binding->stageFlags).c_str());
    } else if ((binding->descriptorType != VK_DESCRIPTOR_TYPE_MUTABLE_EXT) &&
               (descriptor_type_set.find(binding->descriptorType) == descriptor_type_set.end())) {
        skip |= LogError("VUID-VkShaderCreateInfoEXT-codeType-10384", device, loc,
                         "SPIR-V (%s) uses descriptor %s of type %s but expected %s.", string_VkShaderStageFlagBits(variable.stage),
                         variable.DescribeDescriptor().c_str(), string_VkDescriptorType(binding->descriptorType),
                         string_DescriptorTypeSet(descriptor_type_set).c_str());
    } else if (binding->descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK && variable.array_length) {
        skip |=
            LogError("VUID-VkShaderCreateInfoEXT-codeType-10386", device, loc,
                     "SPIR-V (%s) uses descriptor %s as VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, but it is an array of descriptor.",
                     string_VkShaderStageFlagBits(variable.stage), variable.DescribeDescriptor().c_str());

    } else if (binding->descriptorCount < variable.array_length && variable.array_length != spirv::kRuntimeArray) {
        skip |= LogError("VUID-VkShaderCreateInfoEXT-codeType-10385", device, loc,
                         "SPIR-V (%s) uses descriptor %s with a VkDescriptorSetLayoutBinding::descriptorCount of %" PRIu32
                         ", but requires at least %" PRIu32 " in the SPIR-V.",
                         string_VkShaderStageFlagBits(variable.stage), variable.DescribeDescriptor().c_str(),
                         binding->descriptorCount, variable.array_length);
    }

    return skip;
}

bool CoreChecks::ValidateTransformFeedbackPipeline(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                                   const vvl::Pipeline &pipeline, const Location &loc) const {
    bool skip = false;

    const bool is_xfb_execution_mode = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::xfb_bit);
    if (is_xfb_execution_mode) {
        if ((pipeline.create_info_shaders & (VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT)) != 0) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-None-02322", module_state.handle(), loc,
                             "SPIR-V has OpExecutionMode of Xfb and using mesh shaders (%s).",
                             string_VkShaderStageFlags(pipeline.create_info_shaders).c_str());
        }

        if (pipeline.pre_raster_state) {
            if (entrypoint.stage != pipeline.pre_raster_state->last_stage) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pStages-02318", module_state.handle(), loc,
                                 "SPIR-V has OpExecutionMode of Xfb in %s, but %s is the last last pre-rasterization shader stage.",
                                 string_VkShaderStageFlagBits(entrypoint.stage),
                                 string_VkShaderStageFlagBits(pipeline.pre_raster_state->last_stage));
            }
            if ((pipeline.create_flags & VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT) != 0) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-11001", module_state.handle(), loc,
                                 "SPIR-V has OpExecutionMode of Xfb but this pipeline is being created with "
                                 "VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT.");
            }
        }
    }

    if (pipeline.pre_raster_state && (pipeline.create_info_shaders & VK_SHADER_STAGE_GEOMETRY_BIT) != 0 &&
        module_state.HasCapability(spv::CapabilityGeometryStreams) && !enabled_features.geometryStreams) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-geometryStreams-02321", module_state.handle(), loc,
                         "SPIR-V uses GeometryStreams capability, but "
                         "VkPhysicalDeviceTransformFeedbackFeaturesEXT::geometryStreams is not enabled.");
    }
    return skip;
}

bool CoreChecks::ValidateTransformFeedbackEmitStreams(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                                      const spirv::StatelessData &stateless_data, const Location &loc) const {
    bool skip = false;
    if (entrypoint.stage != VK_SHADER_STAGE_GEOMETRY_BIT) {
        return skip;  // GeometryStreams are only used in Geomtry Shaders
    }

    vvl::unordered_set<uint32_t> emitted_streams;
    for (const spirv::Instruction *insn : stateless_data.transform_feedback_stream_inst) {
        const uint32_t opcode = insn->Opcode();
        if (opcode == spv::OpEmitStreamVertex) {
            emitted_streams.emplace(module_state.GetConstantValueById(insn->Word(1)));
        }
        if (opcode == spv::OpEmitStreamVertex || opcode == spv::OpEndStreamPrimitive) {
            uint32_t stream = module_state.GetConstantValueById(insn->Word(1));
            if (stream >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams) {
                skip |= LogError("VUID-RuntimeSpirv-OpEmitStreamVertex-06310", module_state.handle(), loc,
                                 "SPIR-V uses transform feedback stream\n%s\nwith index %" PRIu32
                                 ", which is not less than maxTransformFeedbackStreams (%" PRIu32 ").",
                                 insn->Describe().c_str(), stream,
                                 phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams);
            }
        }
    }

    const bool output_points = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::output_points_bit);
    const uint32_t emitted_streams_size = static_cast<uint32_t>(emitted_streams.size());
    if (emitted_streams_size > 1 && !output_points &&
        phys_dev_ext_props.transform_feedback_props.transformFeedbackStreamsLinesTriangles == VK_FALSE) {
        skip |= LogError("VUID-RuntimeSpirv-transformFeedbackStreamsLinesTriangles-06311", module_state.handle(), loc,
                         "SPIR-V emits to %" PRIu32
                         " vertex streams and transformFeedbackStreamsLinesTriangles "
                         "is VK_FALSE, but execution mode is not OutputPoints.",
                         emitted_streams_size);
    }

    return skip;
}

// Checks for both TexelOffset and TexelGatherOffset limits
bool CoreChecks::ValidateTexelOffsetLimits(const spirv::Module &module_state, const spirv::Instruction &insn,
                                           const Location &loc) const {
    bool skip = false;

    const uint32_t opcode = insn.Opcode();
    const bool is_image_gather = ImageGatherOperation(opcode);
    if (!is_image_gather && !ImageSampleOperation(opcode) && !ImageFetchOperation(opcode)) {
        return false;
    }

    const uint32_t image_operand_position = OpcodeImageOperandsPosition(opcode);
    // Image operands can be optional
    if (image_operand_position == 0 || insn.Length() <= image_operand_position) {
        return false;
    }

    const uint32_t image_operand = insn.Word(image_operand_position);

    // Bits we are validating (sample/fetch only check ConstOffset)
    uint32_t offset_bits =
        is_image_gather ? (spv::ImageOperandsOffsetMask | spv::ImageOperandsConstOffsetMask | spv::ImageOperandsConstOffsetsMask)
                        : (spv::ImageOperandsConstOffsetMask);
    if ((image_operand & offset_bits) == 0) {
        return false;
    }

    // Operand values follow
    uint32_t index = image_operand_position + 1;
    // Each bit has it's own operand, starts with the smallest set bit and loop to the highest bit among
    // ImageOperandsOffsetMask, ImageOperandsConstOffsetMask and ImageOperandsConstOffsetsMask
    for (uint32_t i = 1; i < spv::ImageOperandsConstOffsetsMask; i <<= 1) {
        if ((image_operand & i) == 0) {
            continue;
        }

        // If the bit is set, consume operand
        if (insn.Length() > index && (i & offset_bits)) {
            uint32_t constant_id = insn.Word(index);
            const spirv::Instruction *constant = module_state.FindDef(constant_id);
            const bool is_dynamic_offset = constant == nullptr;
            if (!is_dynamic_offset && constant->Opcode() == spv::OpConstantComposite) {
                for (uint32_t j = 3; j < constant->Length(); ++j) {
                    uint32_t comp_id = constant->Word(j);
                    const spirv::Instruction *comp = module_state.FindDef(comp_id);
                    const spirv::Instruction *comp_type = module_state.FindDef(comp->Word(1));
                    // Get operand value
                    const uint32_t offset = comp->Word(3);
                    // spec requires minTexelGatherOffset/minTexelOffset to be -8 or less so never can compare if
                    // unsigned spec requires maxTexelGatherOffset/maxTexelOffset to be 7 or greater so never can
                    // compare if signed is less then zero
                    const int32_t signed_offset = static_cast<int32_t>(offset);
                    const bool use_signed = (comp_type->Opcode() == spv::OpTypeInt && comp_type->Word(3) != 0);

                    // There are 2 sets of VU being covered where the only main difference is the opcode
                    if (is_image_gather) {
                        // min/maxTexelGatherOffset
                        if (use_signed && (signed_offset < phys_dev_props.limits.minTexelGatherOffset)) {
                            skip |=
                                LogError("VUID-RuntimeSpirv-OpImage-06376", module_state.handle(), loc,
                                         "SPIR-V uses %s with offset (%" PRId32
                                         ") less than VkPhysicalDeviceLimits::minTexelGatherOffset (%" PRId32 ").\n%s\n",
                                         string_SpvOpcode(insn.Opcode()), signed_offset, phys_dev_props.limits.minTexelGatherOffset,
                                         module_state.DescribeInstruction(insn).c_str());
                        } else if ((offset > phys_dev_props.limits.maxTexelGatherOffset) &&
                                   (!use_signed || (use_signed && signed_offset > 0))) {
                            skip |= LogError("VUID-RuntimeSpirv-OpImage-06377", module_state.handle(), loc,
                                             "SPIR-V uses %s with offset (%" PRIu32
                                             ") greater than VkPhysicalDeviceLimits::maxTexelGatherOffset (%" PRIu32 ").\n%s\n",
                                             string_SpvOpcode(insn.Opcode()), offset, phys_dev_props.limits.maxTexelGatherOffset,
                                             module_state.DescribeInstruction(insn).c_str());
                        }
                    } else {
                        // min/maxTexelOffset
                        // TODO - maintenance8 added ability to use Offset, but will need validation on GPU side
                        if (use_signed && (signed_offset < phys_dev_props.limits.minTexelOffset)) {
                            skip |= LogError("VUID-RuntimeSpirv-OpImageSample-06435", module_state.handle(), loc,
                                             "SPIR-V uses %s with offset (%" PRId32
                                             ") less than VkPhysicalDeviceLimits::minTexelOffset (%" PRId32 ").\n%s\n",
                                             string_SpvOpcode(insn.Opcode()), signed_offset, phys_dev_props.limits.minTexelOffset,
                                             module_state.DescribeInstruction(insn).c_str());
                        } else if ((offset > phys_dev_props.limits.maxTexelOffset) &&
                                   (!use_signed || (use_signed && signed_offset > 0))) {
                            skip |= LogError("VUID-RuntimeSpirv-OpImageSample-06436", module_state.handle(), loc,
                                             "SPIR-V uses %s with offset (%" PRIu32
                                             ") greater than VkPhysicalDeviceLimits::maxTexelOffset (%" PRIu32 ").\n%s\n",
                                             string_SpvOpcode(insn.Opcode()), offset, phys_dev_props.limits.maxTexelOffset,
                                             module_state.DescribeInstruction(insn).c_str());
                        }
                    }
                }
            }
        }
        index += ImageOperandsParamCount(i);
    }

    return skip;
}

bool CoreChecks::ValidateShaderClock(const spirv::Module &module_state, const spirv::StatelessData &stateless_data,
                                     const Location &loc) const {
    bool skip = false;

    for (const spirv::Instruction *clock_inst : stateless_data.read_clock_inst) {
        const spirv::Instruction &insn = *clock_inst;
        const spirv::Instruction *scope_id = module_state.FindDef(insn.Word(3));
        auto scope_type = scope_id->Word(3);
        // if scope isn't Subgroup or Device, spirv-val will catch
        if ((scope_type == spv::ScopeSubgroup) && (enabled_features.shaderSubgroupClock == VK_FALSE)) {
            skip |= LogError("VUID-RuntimeSpirv-shaderSubgroupClock-06267", device, loc,
                             "SPIR-V uses OpReadClockKHR with a Subgroup scope but shaderSubgroupClock was not enabled.\n%s\n",
                             module_state.DescribeInstruction(insn).c_str());
        } else if ((scope_type == spv::ScopeDevice) && (enabled_features.shaderDeviceClock == VK_FALSE)) {
            skip |= LogError("VUID-RuntimeSpirv-shaderDeviceClock-06268", device, loc,
                             "SPIR-V uses OpReadClockKHR with a Device scope but shaderDeviceClock was not enabled.\n%s\n",
                             module_state.DescribeInstruction(insn).c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidateImageWrite(const spirv::Module &module_state, const Location &loc) const {
    bool skip = false;
    for (const auto &[insn, load_id] : module_state.static_data_.image_write_load_id_map) {
        // guaranteed by spirv-val to be an OpTypeImage
        const uint32_t image = module_state.GetTypeId(load_id);
        const spirv::Instruction *image_def = module_state.FindDef(image);
        const uint32_t image_format = image_def->Word(8);
        // If format is 'Unknown' then need to wait until a descriptor is bound to it
        if (image_format != spv::ImageFormatUnknown) {
            const VkFormat compatible_format = CompatibleSpirvImageFormat(image_format);
            if (compatible_format != VK_FORMAT_UNDEFINED) {
                const uint32_t format_component_count = vkuFormatComponentCount(compatible_format);
                const uint32_t texel_component_count = module_state.GetTexelComponentCount(*insn);
                if (texel_component_count < format_component_count) {
                    skip |= LogError("VUID-RuntimeSpirv-OpImageWrite-07112", module_state.handle(), loc,
                                     "SPIR-V OpImageWrite Texel operand only contains %" PRIu32
                                     " components, but the OpImage format mapping to %s has %" PRIu32 " components.\n%s\n%s\n",
                                     texel_component_count, string_VkFormat(compatible_format), format_component_count,
                                     module_state.DescribeInstruction(*insn).c_str(),
                                     module_state.DescribeInstruction(*image_def).c_str());
                }
            }
        }
    }
    return skip;
}

static const std::string GetShaderTileImageCapabilitiesString(const spirv::Module &module_state) {
    struct SpvCapabilityWithString {
        const spv::Capability cap;
        const std::string cap_string;
    };

    // Shader tile image capabilities
    static const std::array<SpvCapabilityWithString, 3> shader_tile_image_capabilities = {
        {{spv::CapabilityTileImageColorReadAccessEXT, "TileImageColorReadAccessEXT"},
         {spv::CapabilityTileImageDepthReadAccessEXT, "TileImageDepthReadAccessEXT"},
         {spv::CapabilityTileImageStencilReadAccessEXT, "TileImageStencilReadAccessEXT"}}};

    std::stringstream ss_capabilities;
    for (auto spv_capability : shader_tile_image_capabilities) {
        if (module_state.HasCapability(spv_capability.cap)) {
            if (ss_capabilities.tellp()) ss_capabilities << ", ";
            ss_capabilities << spv_capability.cap_string;
        }
    }

    return ss_capabilities.str();
}

bool CoreChecks::ValidateShaderTileImage(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                         const vvl::Pipeline *pipeline, const VkShaderStageFlagBits stage,
                                         const Location &loc) const {
    bool skip = false;

    if ((stage != VK_SHADER_STAGE_FRAGMENT_BIT) || !IsExtEnabled(extensions.vk_ext_shader_tile_image)) {
        return skip;
    }

    const bool using_tile_image_capability = module_state.HasCapability(spv::CapabilityTileImageColorReadAccessEXT) ||
                                             module_state.HasCapability(spv::CapabilityTileImageDepthReadAccessEXT) ||
                                             module_state.HasCapability(spv::CapabilityTileImageStencilReadAccessEXT);

    if (!using_tile_image_capability) {
        // None of the capabilities exist.
        return skip;
    }

    if (pipeline) {
        auto rp = pipeline->GraphicsCreateInfo().renderPass;
        if (rp != VK_NULL_HANDLE) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-08710", module_state.handle(), loc,
                             "SPIR-V (Fragment stage) is using capabilities (%s), but renderpass (%s) is not VK_NULL_HANDLE.",
                             GetShaderTileImageCapabilitiesString(module_state).c_str(), FormatHandle(rp).c_str());
        }

        const bool mode_early_fragment_test = entrypoint.execution_mode.Has(spirv::ExecutionModeSet::early_fragment_test_bit);
        if (module_state.static_data_.has_shader_tile_image_depth_read) {
            const auto *ds_state = pipeline->DepthStencilState();
            const bool write_enabled =
                !pipeline->IsDynamic(CB_DYNAMIC_STATE_DEPTH_WRITE_ENABLE) && (ds_state && ds_state->depthWriteEnable);
            if (mode_early_fragment_test && write_enabled) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pStages-08711", module_state.handle(), loc,
                                 "SPIR-V (Fragment stage) contains OpDepthAttachmentReadEXT, and depthWriteEnable is not false.");
            }
        }

        if (module_state.static_data_.has_shader_tile_image_stencil_read) {
            const auto *ds_state = pipeline->DepthStencilState();
            const bool is_write_mask_set = !pipeline->IsDynamic(CB_DYNAMIC_STATE_STENCIL_WRITE_MASK) &&
                                           (ds_state && (ds_state->front.writeMask != 0 || ds_state->back.writeMask != 0));
            if (mode_early_fragment_test && is_write_mask_set) {
                skip |= LogError(
                    "VUID-VkGraphicsPipelineCreateInfo-pStages-08712", module_state.handle(), loc,
                    "SPIR-V (Fragment stage) contains OpStencilAttachmentReadEXT, and stencil write mask is not equal to 0 for "
                    "both front(%" PRIu32 ") and back (%" PRIu32 ").",
                    ds_state->front.writeMask, ds_state->back.writeMask);
            }
        }

        bool using_tile_image_op = module_state.static_data_.has_shader_tile_image_depth_read ||
                                   module_state.static_data_.has_shader_tile_image_stencil_read ||
                                   module_state.static_data_.has_shader_tile_image_color_read;
        const auto *ms_state = pipeline->MultisampleState();
        if (using_tile_image_op && ms_state && ms_state->sampleShadingEnable && (ms_state->minSampleShading != 1.0)) {
            skip |= LogError("VUID-RuntimeSpirv-minSampleShading-08732", module_state.handle(), loc,
                             "minSampleShading (%f) is not equal to 1.0.", ms_state->minSampleShading);
        }
    }
    return skip;
}

// Validate the VkPipelineShaderStageCreateInfo from the various pipeline types or a Shader Object
bool CoreChecks::ValidateShaderStage(const ShaderStageState &stage_state, const vvl::Pipeline *pipeline,
                                     const Location &loc) const {
    bool skip = false;
    const VkShaderStageFlagBits stage = stage_state.GetStage();

    // First validate all things that don't require valid SPIR-V
    // this is found when using VK_EXT_shader_module_identifier
    skip |= ValidateShaderSubgroupSizeControl(stage, stage_state, loc);
    skip |= ValidateSpecializations(stage_state.GetSpecializationInfo(), loc.dot(Field::pSpecializationInfo));
    if (pipeline) {
        skip |= ValidateShaderStageMaxResources(stage, *pipeline, loc);
        if (const auto *pipeline_robustness_info =
                vku::FindStructInPNextChain<VkPipelineRobustnessCreateInfo>(stage_state.GetPNext())) {
            skip |= ValidatePipelineRobustnessCreateInfo(*pipeline, *pipeline_robustness_info, loc);
        }
    }

    if ((pipeline && pipeline->uses_shader_module_id) || !stage_state.spirv_state) {
        return skip;  // these edge cases should be validated already
    }

    const spirv::Module &module_state = *stage_state.spirv_state.get();
    if (!module_state.valid_spirv) return skip;  // checked elsewhere

    if (!stage_state.entrypoint) {
        const char *vuid = pipeline ? "VUID-VkPipelineShaderStageCreateInfo-pName-00707" : "VUID-VkShaderCreateInfoEXT-pName-08440";
        return LogError(vuid, device, loc.dot(Field::pName), "`%s` entrypoint not found for stage %s.", stage_state.GetPName(),
                        string_VkShaderStageFlagBits(stage));
    }
    const spirv::EntryPoint &entrypoint = *stage_state.entrypoint;

    // to prevent const_cast on pipeline object, just store here as not needed outside function anyway
    uint32_t local_size_x = 0;
    uint32_t local_size_y = 0;
    uint32_t local_size_z = 0;
    uint32_t total_workgroup_shared_memory = 0;

    // If specialization-constant instructions are present in the shader, the specializations should be applied.
    if (module_state.static_data_.has_specialization_constants) {
        // setup the call back if the optimizer fails
        spv_target_env spirv_environment = PickSpirvEnv(api_version, IsExtEnabled(extensions.vk_khr_spirv_1_4));
        spvtools::Optimizer optimizer(spirv_environment);
        spvtools::MessageConsumer consumer = [&skip, &module_state, &stage, loc, this](
                                                 spv_message_level_t level, const char *source, const spv_position_t &position,
                                                 const char *message) {
            skip |= LogError("VUID-VkPipelineShaderStageCreateInfo-module-parameter", device, loc,
                             "%s failed in spirv-opt because it does not contain valid spirv for stage %s. %s",
                             FormatHandle(module_state.handle()).c_str(), string_VkShaderStageFlagBits(stage), message);
        };
        optimizer.SetMessageConsumer(consumer);

        // The app might be using the default spec constant values, but if they pass values at runtime to the pipeline then need to
        // use those values to apply to the spec constants
        auto const &specialization_info = stage_state.GetSpecializationInfo();
        if (specialization_info != nullptr && specialization_info->mapEntryCount > 0 &&
            specialization_info->pMapEntries != nullptr) {
            // Gather the specialization-constant values.
            auto const &specialization_data = reinterpret_cast<uint8_t const *>(specialization_info->pData);
            std::unordered_map<uint32_t, std::vector<uint32_t>> id_value_map;  // note: this must be std:: to work with spvtools
            id_value_map.reserve(specialization_info->mapEntryCount);

            // spirv-val makes sure every OpSpecConstant has a OpDecoration.
            for (const auto &[result_id, spec_id] : module_state.static_data_.id_to_spec_id) {
                VkSpecializationMapEntry map_entry = {spirv::kInvalidValue, 0, 0};
                for (uint32_t i = 0; i < specialization_info->mapEntryCount; i++) {
                    if (specialization_info->pMapEntries[i].constantID == spec_id) {
                        map_entry = specialization_info->pMapEntries[i];
                        break;
                    }
                }

                // "If a constantID value is not a specialization constant ID used in the shader, that map entry does not affect the
                // behavior of the pipeline."
                if (map_entry.constantID == spirv::kInvalidValue) {
                    continue;
                }

                uint32_t spec_const_size = spirv::kInvalidValue;
                const spirv::Instruction *def_insn = module_state.FindDef(result_id);
                const spirv::Instruction *type_insn = module_state.FindDef(def_insn->Word(1));

                // Specialization constants can only be of type bool, scalar integer, or scalar floating point
                switch (type_insn->Opcode()) {
                    case spv::OpTypeBool:
                        // "If the specialization constant is of type boolean, size must be the byte size of VkBool32"
                        spec_const_size = sizeof(VkBool32);
                        break;
                    case spv::OpTypeInt:
                    case spv::OpTypeFloat:
                        spec_const_size = type_insn->Word(2) / 8;
                        break;
                    default:
                        // spirv-val should catch if SpecId is not used on a
                        // OpSpecConstantTrue/OpSpecConstantFalse/OpSpecConstant and OpSpecConstant is validated to be a
                        // OpTypeInt or OpTypeFloat
                        break;
                }

                if (map_entry.size != spec_const_size) {
                    std::stringstream name;
                    if (module_state.handle()) {
                        name << "shader module " << module_state.handle();
                    } else {
                        name << "shader object";
                    }
                    skip |= LogError("VUID-VkSpecializationMapEntry-constantID-00776", device, loc,
                                     "specialization constant (ID = %" PRIu32 ", entry = %" PRIu32
                                     ") has invalid size %zu in %s. Expected size is %" PRIu32 " from shader definition.",
                                     map_entry.constantID, spec_id, map_entry.size, FormatHandle(module_state.handle()).c_str(),
                                     spec_const_size);
                }

                if ((map_entry.offset + map_entry.size) <= specialization_info->dataSize) {
                    // Allocate enough room for ceil(map_entry.size / 4) to store entries
                    std::vector<uint32_t> entry_data((map_entry.size + 4 - 1) / 4, 0);
                    uint8_t *out_p = reinterpret_cast<uint8_t *>(entry_data.data());
                    const uint8_t *const start_in_p = specialization_data + map_entry.offset;
                    const uint8_t *const end_in_p = start_in_p + map_entry.size;

                    std::copy(start_in_p, end_in_p, out_p);
                    id_value_map.emplace(map_entry.constantID, std::move(entry_data));
                }
            }

            // This pass takes the runtime spec const values and applies it into the SPIR-V
            // will turn a spec constant like
            //     OpSpecConstant %uint 1
            // to a use the value passed in instead (for example if the value is 32) so now it looks like
            //     OpSpecConstant %uint 32
            optimizer.RegisterPass(spvtools::CreateSetSpecConstantDefaultValuePass(id_value_map));
        }

        // This pass will turn OpSpecConstant into a OpConstant (also OpSpecConstantTrue/OpSpecConstantFalse)
        optimizer.RegisterPass(spvtools::CreateFreezeSpecConstantValuePass());
        // Using the new frozen OpConstant all OpSpecConstantComposite can be resolved turning them into OpConstantComposite
        // This is need incase a shdaer looks like:
        //
        //     layout(constant_id = 0) const uint x = 64;
        //     shared uint arr[x > 64 ? 64 : x];
        //
        // this will generate branch/switch statements that we want to leverage spirv-opt to apply to make parsing easier
        optimizer.RegisterPass(spvtools::CreateFoldSpecConstantOpAndCompositePass());

        // Apply the specialization-constant values and revalidate the shader module is valid.
        std::vector<uint32_t> specialized_spirv;
        auto const optimized =
            optimizer.Run(module_state.words_.data(), module_state.words_.size(), &specialized_spirv, spirv_val_options, true);
        if (optimized) {
            spv_context ctx = spvContextCreate(spirv_environment);
            spv_const_binary_t binary{specialized_spirv.data(), specialized_spirv.size()};
            spv_diagnostic diag = nullptr;
            auto const spv_valid = spvValidateWithOptions(ctx, spirv_val_options, &binary, &diag);
            if (spv_valid != SPV_SUCCESS) {
                const char *vuid = pipeline ? "VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-06849"
                                            : "VUID-VkShaderCreateInfoEXT-pCode-08460";
                std::string name = pipeline ? FormatHandle(module_state.handle()) : "shader object";
                skip |= LogError(vuid, device, loc,
                                 "After specialization was applied, %s produces a spirv-val error (stage %s):\n%s", name.c_str(),
                                 string_VkShaderStageFlagBits(stage), diag && diag->error ? diag->error : "(no error text)");
            }

            // The new optimized SPIR-V will NOT match the original spirv::Module object parsing, so a new spirv::Module
            // object is needed. This an issue due to each pipeline being able to reuse the same shader module but with different
            // spec constant values.
            spirv::Module spec_mod(vvl::make_span<const uint32_t>(specialized_spirv.data(), specialized_spirv.size()));

            // According to https://github.com/KhronosGroup/Vulkan-Docs/issues/1671 anything labeled as "static use" (such as if an
            // input is used or not) don't have to be checked post spec constants freezing since the device compiler is not
            // guaranteed to run things such as dead-code elimination. The following checks are things that don't follow under
            // "static use" rules and need to be validated still.

            const auto spec_entrypoint = spec_mod.FindEntrypoint(entrypoint.name.c_str(), entrypoint.stage);
            assert(spec_entrypoint);  // spirv-opt won't change Entrypoint Name/stage

            spec_mod.FindLocalSize(*spec_entrypoint, local_size_x, local_size_y, local_size_z);

            total_workgroup_shared_memory = spec_mod.CalculateWorkgroupSharedMemory();

            spvDiagnosticDestroy(diag);
            spvContextDestroy(ctx);
        } else {
            // Should never get here, but better then asserting
            const char *vuid = pipeline ? "VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-06849"
                                        : "VUID-VkShaderCreateInfoEXT-pCode-08460";
            skip |= LogError(vuid, device, loc,
                             "%s shader (stage %s) attempted to apply specialization constants with spirv-opt but failed.",
                             FormatHandle(module_state.handle()).c_str(), string_VkShaderStageFlagBits(stage));
        }

        if (skip) {
            return skip;  // if spec constants have errors, can produce false positives later
        }
    } else {
        module_state.FindLocalSize(entrypoint, local_size_x, local_size_y, local_size_z);
    }

    skip |= ValidateShaderTileImage(module_state, entrypoint, pipeline, stage, loc);
    skip |= ValidateImageWrite(module_state, loc);
    skip |= ValidateShaderExecutionModes(module_state, entrypoint, stage, pipeline, loc);
    skip |= ValidateBuiltinLimits(module_state, entrypoint, pipeline, loc);
    skip |= ValidatePushConstantUsage(module_state, entrypoint, pipeline, stage_state, loc);
    if (enabled_features.cooperativeMatrix) {
        skip |= ValidateCooperativeMatrix(module_state, entrypoint, stage_state, local_size_x, local_size_y, local_size_z, loc);
    }
    if (enabled_features.cooperativeVector) {
        skip |= ValidateCooperativeVector(module_state, entrypoint, stage_state, loc);
    }

    if (pipeline) {
        if (enabled_features.transformFeedback) {
            skip |= ValidateTransformFeedbackPipeline(module_state, entrypoint, *pipeline, loc);
        }
        if (enabled_features.primitiveFragmentShadingRate) {
            skip |= ValidatePrimitiveRateShaderState(module_state, entrypoint, *pipeline, stage, loc);
        }
        if (IsExtEnabled(extensions.vk_qcom_render_pass_shader_resolve)) {
            skip |= ValidateShaderResolveQCOM(module_state, stage, *pipeline, loc);
        }
        skip |= ValidatePointSizeShaderState(module_state, entrypoint, *pipeline, stage, loc);
        skip |= ValidatePrimitiveTopology(module_state, entrypoint, *pipeline, loc);

        if (stage == VK_SHADER_STAGE_FRAGMENT_BIT && pipeline->GraphicsCreateInfo().renderPass == VK_NULL_HANDLE &&
            module_state.HasCapability(spv::CapabilityInputAttachment) && !enabled_features.dynamicRenderingLocalRead) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06061", device, loc,
                             "is being created with fragment shader state and renderPass = VK_NULL_HANDLE, but fragment "
                             "shader includes InputAttachment capability.");
        }
    }

    if (stage == VK_SHADER_STAGE_COMPUTE_BIT || stage == VK_SHADER_STAGE_TASK_BIT_EXT || stage == VK_SHADER_STAGE_MESH_BIT_EXT) {
        // If spec constants were used then the local size are already found if possible
        if (local_size_x == 0) {
            module_state.FindLocalSize(entrypoint, local_size_x, local_size_y, local_size_z);
        }

        bool fail = false;
        uint32_t limit = phys_dev_props.limits.maxComputeWorkGroupInvocations;
        uint64_t invocations = static_cast<uint64_t>(local_size_x) * static_cast<uint64_t>(local_size_y);
        // Prevent overflow.
        if (invocations > limit) {
            fail = true;
        }
        invocations *= local_size_z;
        if (invocations > limit) {
            fail = true;
        }

        if (fail && stage == VK_SHADER_STAGE_COMPUTE_BIT) {
            skip |= LogError("VUID-RuntimeSpirv-x-06432", module_state.handle(), loc,
                             "SPIR-V LocalSiz (%" PRIu32 ", %" PRIu32 ", %" PRIu32
                             ") exceeds device limit maxComputeWorkGroupInvocations (%" PRIu32 ").",
                             local_size_x, local_size_y, local_size_z, phys_dev_props.limits.maxComputeWorkGroupInvocations);
        }

        skip |= ValidateRequiredSubgroupSize(module_state, stage_state, invocations, local_size_x, local_size_y, local_size_z, loc);
        skip |= ValidateWorkgroupSharedMemory(module_state, stage, total_workgroup_shared_memory, loc);
    }

    for (const auto &variable : entrypoint.resource_interface_variables) {
        vvl::unordered_set<uint32_t> descriptor_type_set;
        TypeToDescriptorTypeSet(module_state, variable.type_id, descriptor_type_set);
        skip |= ValidateShaderInterfaceVariable(module_state, variable, descriptor_type_set, loc);
        if (pipeline) {
            skip |= ValidateShaderInterfaceVariablePipeline(module_state, *pipeline, variable, descriptor_type_set, loc);
        } else if (stage_state.shader_object_create_info) {
            skip |= ValidateShaderInterfaceVariableShaderObject(*stage_state.shader_object_create_info->ptr(), variable,
                                                                descriptor_type_set, loc);
        }
    }

    if (stage == VK_SHADER_STAGE_COMPUTE_BIT) {
        skip |= ValidateComputeWorkGroupSizes(module_state, entrypoint, stage_state, local_size_x, local_size_y, local_size_z, loc);
    } else if (stage == VK_SHADER_STAGE_TASK_BIT_EXT || stage == VK_SHADER_STAGE_MESH_BIT_EXT) {
        skip |= ValidateTaskMeshWorkGroupSizes(module_state, entrypoint, local_size_x, local_size_y, local_size_z, loc);
        if (stage == VK_SHADER_STAGE_TASK_BIT_EXT) {
            skip |= ValidateEmitMeshTasksSize(module_state, entrypoint, stage_state, loc);
        }
    }

    return skip;
}

uint32_t CoreChecks::CalcShaderStageCount(const vvl::Pipeline &pipeline, VkShaderStageFlagBits stageBit) const {
    uint32_t total = 0;
    for (const auto &stage_ci : pipeline.shader_stages_ci) {
        if (stage_ci.stage == stageBit) {
            total++;
        }
    }

    if (pipeline.ray_tracing_library_ci) {
        for (uint32_t i = 0; i < pipeline.ray_tracing_library_ci->libraryCount; ++i) {
            auto library_pipeline = Get<vvl::Pipeline>(pipeline.ray_tracing_library_ci->pLibraries[i]);
            if (!library_pipeline) continue;
            total += CalcShaderStageCount(*library_pipeline, stageBit);
        }
    }

    return total;
}

bool CoreChecks::GroupHasValidIndex(const vvl::Pipeline &pipeline, uint32_t group, uint32_t stage) const {
    if (group == VK_SHADER_UNUSED_NV) {
        return true;
    }

    const auto num_stages = static_cast<uint32_t>(pipeline.shader_stages_ci.size());
    if (group < num_stages) {
        return (pipeline.shader_stages_ci[group].stage & stage) != 0;
    }
    group -= num_stages;

    // Search libraries
    if (pipeline.ray_tracing_library_ci) {
        for (uint32_t i = 0; i < pipeline.ray_tracing_library_ci->libraryCount; ++i) {
            auto library_pipeline = Get<vvl::Pipeline>(pipeline.ray_tracing_library_ci->pLibraries[i]);
            if (!library_pipeline) continue;
            const uint32_t stage_count = static_cast<uint32_t>(library_pipeline->shader_stages_ci.size());
            if (group < stage_count) {
                return (library_pipeline->shader_stages_ci[group].stage & stage) != 0;
            }
            group -= stage_count;
        }
    }

    // group index too large
    return false;
}

// This is done in PreCallRecord to help with the interaction with GPU-AV
// See diagram on https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/6230
void CoreChecks::PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule,
                                                 const RecordObject &record_obj, chassis::CreateShaderModule &chassis_state) {
    // Normally would validate in PreCallValidate, but need a non-const function to update chassis_state
    // This is on the stack, we don't have to worry about threading hazards and this could be moved and used const_cast
    BaseClass::PreCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, record_obj,
                                                            chassis_state);
    chassis_state.skip |= ValidateSpirvStateless(*chassis_state.module_state, chassis_state.stateless_data, record_obj.location);
}

void CoreChecks::PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT *pCreateInfos,
                                               const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                               const RecordObject &record_obj, chassis::ShaderObject &chassis_state) {
    BaseClass::PreCallRecordCreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders, record_obj,
                                                          chassis_state);
    for (uint32_t i = 0; i < createInfoCount; ++i) {
        if (chassis_state.module_states[i]) {
            chassis_state.skip |= ValidateSpirvStateless(*chassis_state.module_states[i], chassis_state.stateless_data[i],
                                                         record_obj.location.dot(Field::pCreateInfos, i));
        }
    }
}

bool CoreChecks::RunSpirvValidation(spv_const_binary_t &binary, const Location &loc, ValidationCache *cache) const {
    bool skip = false;

    if (global_settings.debug_disable_spirv_val) {
        return skip;
    }

    uint32_t hash = 0;
    if (cache) {
        hash = hash_util::ShaderHash((void *)binary.code, binary.wordCount * sizeof(uint32_t));
        if (cache->Contains(hash)) {
            return skip;
        }
    }

    // Use SPIRV-Tools validator to try and catch any issues with the module itself. If specialization constants are present,
    // the default values will be used during validation.
    spv_target_env spirv_environment = PickSpirvEnv(api_version, IsExtEnabled(extensions.vk_khr_spirv_1_4));
    spv_context ctx = spvContextCreate(spirv_environment);
    spv_diagnostic diag = nullptr;
    const spv_result_t spv_valid = spvValidateWithOptions(ctx, spirv_val_options, &binary, &diag);
    if (spv_valid != SPV_SUCCESS) {
        // VkShaderModuleCreateInfo can come from many functions
        const char *vuid = loc.function == Func::vkCreateShadersEXT ? "VUID-VkShaderCreateInfoEXT-pCode-08737"
                                                                    : "VUID-VkShaderModuleCreateInfo-pCode-08737";

        if (spv_valid == SPV_WARNING) {
            skip |= LogWarning(vuid, device, loc.dot(Field::pCode), "(spirv-val produced a warning):\n%s",
                               diag && diag->error ? diag->error : "(no error text)");
        } else {
            skip |= LogError(vuid, device, loc.dot(Field::pCode), "(spirv-val produced an error):\n%s",
                             diag && diag->error ? diag->error : "(no error text)");
        }
    } else if (cache) {
        // No point to cache anything that is not valid, or it will get supressed on the next run
        cache->Insert(hash);
    }

    spvDiagnosticDestroy(diag);
    spvContextDestroy(ctx);

    return skip;
}

bool CoreChecks::ValidateShaderModuleCreateInfo(const VkShaderModuleCreateInfo &create_info,
                                                const Location &create_info_loc) const {
    bool skip = false;

    if (disabled[shader_validation]) {
        return skip; // VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT
    }

    if (!create_info.pCode) {
        return skip;  // will be caught elsewhere
    } else if (create_info.pCode[0] != spv::MagicNumber) {
        if (!IsExtEnabled(extensions.vk_nv_glsl_shader)) {
            skip |= LogError("VUID-VkShaderModuleCreateInfo-pCode-07912", device, create_info_loc.dot(Field::pCode),
                             "doesn't point to a SPIR-V module (The first dword is not the SPIR-V MagicNumber 0x07230203).");
        }
    } else if (SafeModulo(create_info.codeSize, 4) != 0) {
        skip |= LogError("VUID-VkShaderModuleCreateInfo-codeSize-08735", device, create_info_loc.dot(Field::codeSize),
                         "(%zu) must be a multiple of 4.", create_info.codeSize);
    } else {
        // if pCode is garbage, don't pass along to spirv-val

        const auto validation_cache_ci = vku::FindStructInPNextChain<VkShaderModuleValidationCacheCreateInfoEXT>(create_info.pNext);
        ValidationCache *cache =
            validation_cache_ci ? CastFromHandle<ValidationCache *>(validation_cache_ci->validationCache) : nullptr;
        // If app isn't using a shader validation cache, use the default one from CoreChecks
        if (!cache) {
            cache = CastFromHandle<ValidationCache *>(core_validation_cache);
        }

        spv_const_binary_t binary{create_info.pCode, create_info.codeSize / sizeof(uint32_t)};
        skip |= RunSpirvValidation(binary, create_info_loc, cache);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule,
                                                   const ErrorObject &error_obj) const {
    return ValidateShaderModuleCreateInfo(*pCreateInfo, error_obj.location.dot(Field::pCreateInfo));
}

bool CoreChecks::PreCallValidateGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                             VkShaderModuleIdentifierEXT *pIdentifier,
                                                             const ErrorObject &error_obj) const {
    bool skip = false;
    if (!(enabled_features.shaderModuleIdentifier)) {
        skip |= LogError("VUID-vkGetShaderModuleIdentifierEXT-shaderModuleIdentifier-06884", shaderModule, error_obj.location,
                         "the shaderModuleIdentifier feature was not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                                       VkShaderModuleIdentifierEXT *pIdentifier,
                                                                       const ErrorObject &error_obj) const {
    bool skip = false;
    if (!(enabled_features.shaderModuleIdentifier)) {
        skip |= LogError("VUID-vkGetShaderModuleCreateInfoIdentifierEXT-shaderModuleIdentifier-06885", device, error_obj.location,
                         "the shaderModuleIdentifier feature was not enabled.");
    }
    return skip;
}

bool CoreChecks::ValidateRequiredSubgroupSize(const spirv::Module &module_state, const ShaderStageState &stage_state,
                                              uint64_t invocations, uint32_t local_size_x, uint32_t local_size_y,
                                              uint32_t local_size_z, const Location &loc) const {
    bool skip = false;

    const auto *required_subgroup_size_ci =
        vku::FindStructInPNextChain<VkPipelineShaderStageRequiredSubgroupSizeCreateInfo>(stage_state.GetPNext());
    if (!required_subgroup_size_ci) return skip;

    const Location pNext_loc = loc.pNext(Struct::VkPipelineShaderStageRequiredSubgroupSizeCreateInfo);

    const uint32_t required_subgroup_size = required_subgroup_size_ci->requiredSubgroupSize;
    if (!enabled_features.subgroupSizeControl) {
        skip |= LogError("VUID-VkPipelineShaderStageCreateInfo-pNext-02755", module_state.handle(), pNext_loc,
                         "the subgroupSizeControl feature was not enabled");
    }
    if ((phys_dev_props_core13.requiredSubgroupSizeStages & stage_state.GetStage()) == 0) {
        skip |=
            LogError("VUID-VkPipelineShaderStageCreateInfo-pNext-02755", module_state.handle(), loc,
                     "SPIR-V (%s) is not in requiredSubgroupSizeStages (%s).", string_VkShaderStageFlagBits(stage_state.GetStage()),
                     string_VkShaderStageFlags(phys_dev_props_core13.requiredSubgroupSizeStages).c_str());
    }
    if ((invocations > required_subgroup_size * phys_dev_props_core13.maxComputeWorkgroupSubgroups)) {
        skip |= LogError("VUID-VkPipelineShaderStageCreateInfo-pNext-02756", module_state.handle(), loc,
                         "SPIR-V Local workgroup size (%" PRIu32 ", %" PRIu32 ", %" PRIu32
                         ") is greater than requiredSubgroupSize (%" PRIu32 ") * maxComputeWorkgroupSubgroups (%" PRIu32 ").",
                         local_size_x, local_size_y, local_size_z, required_subgroup_size,
                         phys_dev_props_core13.maxComputeWorkgroupSubgroups);
    }
    if (stage_state.pipeline_create_info &&
        (stage_state.pipeline_create_info->flags & VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT) > 0) {
        if (SafeModulo(local_size_x, required_subgroup_size) != 0) {
            skip |= LogError("VUID-VkPipelineShaderStageCreateInfo-pNext-02757", module_state.handle(), loc,
                             "SPIR-V Local workgroup size x (%" PRIu32
                             ") is not a multiple of "
                             "requiredSubgroupSize (%" PRIu32 ").",
                             local_size_x, required_subgroup_size);
        }
    }
    if (!IsPowerOfTwo(required_subgroup_size)) {
        skip |= LogError("VUID-VkPipelineShaderStageRequiredSubgroupSizeCreateInfo-requiredSubgroupSize-02760",
                         module_state.handle(), pNext_loc.dot(Field::requiredSubgroupSizeStages),
                         "(%" PRIu32 ") is not a power of 2.", required_subgroup_size);
    }
    if (required_subgroup_size < phys_dev_props_core13.minSubgroupSize) {
        skip |=
            LogError("VUID-VkPipelineShaderStageRequiredSubgroupSizeCreateInfo-requiredSubgroupSize-02761", module_state.handle(),
                     pNext_loc.dot(Field::requiredSubgroupSizeStages), "(%" PRIu32 ") is less than minSubgroupSize (%" PRIu32 ").",
                     required_subgroup_size, phys_dev_props_core13.minSubgroupSize);
    }
    if (required_subgroup_size > phys_dev_props_core13.maxSubgroupSize) {
        skip |= LogError("VUID-VkPipelineShaderStageRequiredSubgroupSizeCreateInfo-requiredSubgroupSize-02762",
                         module_state.handle(), pNext_loc.dot(Field::requiredSubgroupSizeStages),
                         "(%" PRIu32 ") is greater than maxSubgroupSize (%" PRIu32 ").", required_subgroup_size,
                         phys_dev_props_core13.maxSubgroupSize);
    }

    return skip;
}

bool CoreChecks::ValidateComputeWorkGroupSizes(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                               const ShaderStageState &stage_state, uint32_t local_size_x, uint32_t local_size_y,
                                               uint32_t local_size_z, const Location &loc) const {
    bool skip = false;

    if (local_size_x == 0) {
        return skip;
    }

    if (local_size_x > phys_dev_props.limits.maxComputeWorkGroupSize[0]) {
        skip |= LogError("VUID-RuntimeSpirv-x-06429", module_state.handle(), loc,
                         "SPIR-V LocalSize X (%" PRIu32 ") exceeds device limit maxComputeWorkGroupSize[0] (%" PRIu32 ").",
                         local_size_x, phys_dev_props.limits.maxComputeWorkGroupSize[0]);
    }
    if (local_size_y > phys_dev_props.limits.maxComputeWorkGroupSize[1]) {
        skip |= LogError("VUID-RuntimeSpirv-y-06430", module_state.handle(), loc,
                         "SPIR-V LocalSize Y (%" PRIu32 ") exceeds device limit maxComputeWorkGroupSize[1] (%" PRIu32 ").",
                         local_size_y, phys_dev_props.limits.maxComputeWorkGroupSize[1]);
    }
    if (local_size_z > phys_dev_props.limits.maxComputeWorkGroupSize[2]) {
        skip |= LogError("VUID-RuntimeSpirv-z-06431", module_state.handle(), loc,
                         "SPIR-V LocalSize Z (%" PRIu32 ") exceeds device limit maxComputeWorkGroupSize[2] (%" PRIu32 ").",
                         local_size_z, phys_dev_props.limits.maxComputeWorkGroupSize[2]);
    }

    if (stage_state.pipeline_create_info) {
        const auto subgroup_flags = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT |
                                    VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT;
        if ((stage_state.pipeline_create_info->flags & subgroup_flags) == subgroup_flags) {
            if (SafeModulo(local_size_x, phys_dev_props_core13.maxSubgroupSize) != 0) {
                skip |= LogError(
                    "VUID-VkPipelineShaderStageCreateInfo-flags-02758", module_state.handle(), loc.dot(Field::flags),
                    "(%s), but local workgroup size X dimension (%" PRIu32
                    ") is not a multiple of VkPhysicalDeviceSubgroupSizeControlPropertiesEXT::maxSubgroupSize (%" PRIu32 ").",
                    string_VkPipelineShaderStageCreateFlags(stage_state.pipeline_create_info->flags).c_str(), local_size_x,
                    phys_dev_props_core13.maxSubgroupSize);
            }
        } else if ((stage_state.pipeline_create_info->flags & VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT) &&
                   (stage_state.pipeline_create_info->flags & VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT) ==
                       0) {
            if (!vku::FindStructInPNextChain<VkPipelineShaderStageRequiredSubgroupSizeCreateInfo>(stage_state.GetPNext())) {
                if (SafeModulo(local_size_x, phys_dev_props_core11.subgroupSize) != 0) {
                    skip |=
                        LogError("VUID-VkPipelineShaderStageCreateInfo-flags-02759", module_state.handle(), loc.dot(Field::flags),
                                 "(%s), but local workgroup size X dimension (%" PRIu32
                                 ") is not a multiple of VkPhysicalDeviceVulkan11Properties::subgroupSize (%" PRIu32 ").",
                                 string_VkPipelineShaderStageCreateFlags(stage_state.pipeline_create_info->flags).c_str(),
                                 local_size_x, phys_dev_props_core11.subgroupSize);
                }
            }
        }
    } else {
        const bool varying = stage_state.shader_object_create_info->flags & VK_SHADER_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT;
        const bool full = stage_state.shader_object_create_info->flags & VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT;
        const auto *required_subgroup_size =
            vku::FindStructInPNextChain<VkShaderRequiredSubgroupSizeCreateInfoEXT>(stage_state.GetPNext());
        if (varying && full) {
            if (SafeModulo(local_size_x, phys_dev_props_core13.maxSubgroupSize) != 0) {
                skip |= LogError(
                    "VUID-VkShaderCreateInfoEXT-flags-08416", module_state.handle(), loc.dot(Field::flags),
                    "(%s) but local workgroup size X dimension (%" PRIu32
                    ") is not a multiple of VkPhysicalDeviceSubgroupSizeControlPropertiesEXT::maxSubgroupSize (%" PRIu32 ").",
                    string_VkPipelineShaderStageCreateFlags(stage_state.shader_object_create_info->flags).c_str(), local_size_x,
                    phys_dev_props_core13.maxSubgroupSize);
            }
        } else if (full && !varying) {
            if (!required_subgroup_size && SafeModulo(local_size_x, phys_dev_props_core11.subgroupSize) != 0) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-flags-08417", module_state.handle(), loc.dot(Field::flags),
                                 "(%s), but local workgroup size X dimension (%" PRIu32
                                 ") is not a multiple of VkPhysicalDeviceVulkan11Properties::subgroupSize (%" PRIu32 ").",
                                 string_VkPipelineShaderStageCreateFlags(stage_state.shader_object_create_info->flags).c_str(),
                                 local_size_x, phys_dev_props_core11.subgroupSize);
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateTaskMeshWorkGroupSizes(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                                uint32_t local_size_x, uint32_t local_size_y, uint32_t local_size_z,
                                                const Location &loc) const {
    bool skip = false;

    if (local_size_x == 0) {
        return skip;
    }

    uint32_t max_local_size_x = 0;
    uint32_t max_local_size_y = 0;
    uint32_t max_local_size_z = 0;
    uint32_t max_workgroup_size = 0;
    const char *x_vuid;
    const char *y_vuid;
    const char *z_vuid;
    const char *workgroup_size_vuid;

    switch (entrypoint.execution_model) {
        case spv::ExecutionModelTaskEXT: {
            x_vuid = "VUID-RuntimeSpirv-TaskEXT-07291";
            y_vuid = "VUID-RuntimeSpirv-TaskEXT-07292";
            z_vuid = "VUID-RuntimeSpirv-TaskEXT-07293";
            workgroup_size_vuid = "VUID-RuntimeSpirv-TaskEXT-07294";
            max_local_size_x = phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupSize[0];
            max_local_size_y = phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupSize[1];
            max_local_size_z = phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupSize[2];
            max_workgroup_size = phys_dev_ext_props.mesh_shader_props_ext.maxTaskWorkGroupInvocations;
            break;
        }

        case spv::ExecutionModelMeshEXT: {
            x_vuid = "VUID-RuntimeSpirv-MeshEXT-07295";
            y_vuid = "VUID-RuntimeSpirv-MeshEXT-07296";
            z_vuid = "VUID-RuntimeSpirv-MeshEXT-07297";
            workgroup_size_vuid = "VUID-RuntimeSpirv-MeshEXT-07298";
            max_local_size_x = phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupSize[0];
            max_local_size_y = phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupSize[1];
            max_local_size_z = phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupSize[2];
            max_workgroup_size = phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupInvocations;
            break;
        }

        // skip for spv::ExecutionModelTaskNV and spv::ExecutionModelMeshNV case
        default: {
            // must match one of the above case
            return skip;
        }
    }

    if (local_size_x > max_local_size_x) {
        skip |= LogError(x_vuid, module_state.handle(), loc,
                         "SPIR-V (%s) local workgroup size X dimension (%" PRIu32
                         ") must be less than or equal to the max workgroup size (%" PRIu32 ").",
                         string_SpvExecutionModel(entrypoint.execution_model), local_size_x, max_local_size_x);
    }

    if (local_size_y > max_local_size_y) {
        skip |= LogError(y_vuid, module_state.handle(), loc,
                         "SPIR-V (%s) local workgroup size Y dimension (%" PRIu32
                         ") must be less than or equal to the max workgroup size (%" PRIu32 ").",
                         string_SpvExecutionModel(entrypoint.execution_model), local_size_y, max_local_size_y);
    }

    if (local_size_z > max_local_size_z) {
        skip |= LogError(z_vuid, module_state.handle(), loc,
                         "SPIR-V (%s) local workgroup size Z dimension (%" PRIu32
                         ") must be less than or equal to the max workgroup size (%" PRIu32 ").",
                         string_SpvExecutionModel(entrypoint.execution_model), local_size_z, max_local_size_z);
    }

    uint64_t invocations = static_cast<uint64_t>(local_size_x) * static_cast<uint64_t>(local_size_y);
    // Prevent overflow.
    bool fail = false;
    if (invocations > vvl::kU32Max || invocations > max_workgroup_size) {
        fail = true;
    }
    if (!fail) {
        invocations *= local_size_z;
        if (invocations > vvl::kU32Max || invocations > max_workgroup_size) {
            fail = true;
        }
    }
    if (fail) {
        skip |= LogError(workgroup_size_vuid, module_state.handle(), loc,
                         "SPIR-V (%s) total invocation size (%" PRIu32 " x %" PRIu32 " x %" PRIu32 " = %" PRIu32
                         ") must be less than or equal to max workgroup invocations (%" PRIu32 ").",
                         string_SpvExecutionModel(entrypoint.execution_model), local_size_x, local_size_y, local_size_z,
                         local_size_x * local_size_y * local_size_z, max_workgroup_size);
    }
    return skip;
}

bool CoreChecks::ValidateEmitMeshTasksSize(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                           const ShaderStageState &stage_state, const Location &loc) const {
    bool skip = false;

    for (const spirv::Instruction &insn : module_state.static_data_.instructions) {
        if (insn.Opcode() == spv::OpEmitMeshTasksEXT) {
            uint32_t x, y, z;
            bool found_x = stage_state.GetInt32ConstantValue(*module_state.FindDef(insn.Word(1)), &x);
            bool found_y = stage_state.GetInt32ConstantValue(*module_state.FindDef(insn.Word(2)), &y);
            bool found_z = stage_state.GetInt32ConstantValue(*module_state.FindDef(insn.Word(3)), &z);
            if (found_x && x > phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[0]) {
                skip |= LogError("VUID-RuntimeSpirv-TaskEXT-07299", module_state.handle(), loc,
                                 "SPIR-V (%s) is emitting %" PRIu32
                                 " mesh work groups in X dimension, which is greater than max mesh "
                                 "workgroup count (%" PRIu32 ").",
                                 string_SpvExecutionModel(entrypoint.execution_model), x,
                                 phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[0]);
            }
            if (found_y && y > phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[1]) {
                skip |= LogError("VUID-RuntimeSpirv-TaskEXT-07300", module_state.handle(), loc,
                                 "SPIR-V (%s) is emitting %" PRIu32
                                 " mesh work groups in Y dimension, which is greater than max mesh "
                                 "workgroup count (%" PRIu32 ").",
                                 string_SpvExecutionModel(entrypoint.execution_model), y,
                                 phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[1]);
            }
            if (found_z && z > phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[2]) {
                skip |= LogError("VUID-RuntimeSpirv-TaskEXT-07301", module_state.handle(), loc,
                                 "SPIR-V (%s) is emitting %" PRIu32
                                 " mesh work groups in Z dimension, which is greater than max mesh "
                                 "workgroup count (%" PRIu32 ").",
                                 string_SpvExecutionModel(entrypoint.execution_model), z,
                                 phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupCount[2]);
            }
            if (found_x && found_y && found_z) {
                uint64_t invocations = static_cast<uint64_t>(x) * static_cast<uint64_t>(y);
                // Prevent overflow.
                bool fail = false;
                if (invocations > phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupTotalCount) {
                    fail = true;
                }
                if (!fail) {
                    invocations *= z;
                    if (invocations > vvl::kU32Max ||
                        invocations > phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupTotalCount) {
                        fail = true;
                    }
                }
                if (fail) {
                    skip |=
                        LogError("VUID-RuntimeSpirv-TaskEXT-07302", module_state.handle(), loc,
                                 "SPIR-V (%s) is emitting %" PRIu32 " x %" PRIu32 " x %" PRIu32 " mesh work groups (total %" PRIu32
                                 "), which is greater than max mesh "
                                 "workgroup total count (%" PRIu32 ").",
                                 string_SpvExecutionModel(entrypoint.execution_model), x, y, z, x * y * z,
                                 phys_dev_ext_props.mesh_shader_props_ext.maxMeshWorkGroupTotalCount);
                }
            }
        }
    }

    return skip;
}

// stateless spirv == doesn't require pipeline state and/or shader object info
// Originally the goal was to move more validation to vkCreateShaderModule time in case the driver decided to parse an invalid
// SPIR-V here, while that is likely not the case anymore, a bigger reason for checking here is to save on memory. There is a lot of
// state saved in the Module that is only checked once later and could be reduced if not saved.
bool CoreChecks::ValidateSpirvStateless(const spirv::Module &module_state, const spirv::StatelessData &stateless_data,
                                        const Location &loc) const {
    bool skip = false;
    if (!module_state.valid_spirv) return skip;

    skip |= ValidateShaderClock(module_state, stateless_data, loc);
    skip |= ValidateAtomicsTypes(module_state, stateless_data, loc);
    skip |= ValidateVariables(module_state, loc);

    if (enabled_features.transformFeedback) {
        skip |= ValidateTransformFeedbackDecorations(module_state, loc);
    }

    // The following tries to limit the number of passes through the shader module.
    // It save a good amount of memory and complex state tracking to just check these in a 2nd pass
    for (const spirv::Instruction &insn : module_state.GetInstructions()) {
        skip |= ValidateShaderCapabilitiesAndExtensions(insn, loc);
        skip |= ValidateTexelOffsetLimits(module_state, insn, loc);
        skip |= ValidateMemoryScope(module_state, insn, loc);
        skip |= ValidateSubgroupRotateClustered(module_state, insn, loc);
    }

    for (const auto &entry_point : module_state.static_data_.entry_points) {
        skip |= ValidateShaderStageGroupNonUniform(module_state, stateless_data, entry_point->stage, loc);
        skip |= ValidateShaderStageInputOutputLimits(module_state, *entry_point, stateless_data, loc);
        skip |= ValidateShaderFloatControl(module_state, *entry_point, stateless_data, loc);
        skip |= ValidateExecutionModes(module_state, *entry_point, stateless_data, loc);
        skip |= ValidatePhysicalStorageBuffers(module_state, *entry_point, loc);
        skip |= ValidateConservativeRasterization(module_state, *entry_point, stateless_data, loc);
        if (enabled_features.transformFeedback) {
            skip |= ValidateTransformFeedbackEmitStreams(module_state, *entry_point, stateless_data, loc);
        }
    }
    return skip;
}
