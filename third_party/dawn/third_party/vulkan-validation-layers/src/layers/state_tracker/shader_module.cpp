/* Copyright (c) 2021-2025 The Khronos Group Inc.
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

#include "state_tracker/shader_module.h"

#include <sstream>
#include <string>
#include <queue>

#include "utils/hash_util.h"
#include "generated/spirv_grammar_helper.h"

#include <spirv/unified1/spirv.hpp>
#include <spirv/1.2/GLSL.std.450.h>
#include <spirv/unified1/NonSemanticShaderDebugInfo100.h>
#include "error_message/spirv_logging.h"
#include "utils/vk_layer_utils.h"

namespace spirv {

void DecorationBase::Add(uint32_t decoration, uint32_t value) {
    switch (decoration) {
        case spv::DecorationLocation:
            location = value;
            break;
        case spv::DecorationPatch:
            flags |= patch_bit;
            break;
        case spv::DecorationBlock:
            flags |= block_bit;
            break;
        case spv::DecorationBufferBlock:
            flags |= buffer_block_bit;
            break;
        case spv::DecorationComponent:
            component = value;
            break;
        case spv::DecorationIndex:
            index = value;
            break;
        case spv::DecorationNonWritable:
            flags |= nonwritable_bit;
            break;
        case spv::DecorationBuiltIn:
            assert(builtin == kInvalidValue);  // being over written - not valid
            builtin = value;
            break;
        case spv::DecorationNonReadable:
            flags |= nonreadable_bit;
            break;
        case spv::DecorationPerVertexKHR:  // VK_KHR_fragment_shader_barycentric
            flags |= per_vertex_bit;
            break;
        case spv::DecorationPassthroughNV:  // VK_NV_geometry_shader_passthrough
            flags |= passthrough_bit;
            break;
        case spv::DecorationAliased:
            flags |= aliased_bit;
            break;
        case spv::DecorationPerTaskNV:  // VK_NV_mesh_shader
            flags |= per_task_nv;
            break;
        case spv::DecorationPerPrimitiveEXT:  // VK_EXT_mesh_shader
            flags |= per_primitive_ext;
            break;
        case spv::DecorationOffset:
            offset |= value;
            break;
        default:
            break;
    }
}

// Some decorations are only avaiable for variables, so can't be in OpMemberDecorate
void DecorationSet::Add(uint32_t decoration, uint32_t value) {
    switch (decoration) {
        case spv::DecorationDescriptorSet:
            set = value;
            break;
        case spv::DecorationBinding:
            binding = value;
            break;
        case spv::DecorationInputAttachmentIndex:
            flags |= input_attachment_bit;
            input_attachment_index_start = value;
            break;
        default:
            DecorationBase::Add(decoration, value);
    }
}

bool DecorationSet::HasAnyBuiltIn() const {
    if (kInvalidValue != builtin) {
        return true;
    } else if (!member_decorations.empty()) {
        for (const auto& member : member_decorations) {
            if (kInvalidValue != member.second.builtin) {
                return true;
            }
        }
    }
    return false;
}

bool DecorationSet::HasInMember(FlagBit flag_bit) const {
    for (const auto& decoration : member_decorations) {
        if (decoration.second.Has(flag_bit)) {
            return true;
        }
    }
    return false;
}

bool DecorationSet::AllMemberHave(FlagBit flag_bit) const {
    for (const auto& decoration : member_decorations) {
        if (!decoration.second.Has(flag_bit)) {
            return false;
        }
    }
    return true;
}

void ExecutionModeSet::Add(const Instruction& insn) {
    const uint32_t execution_mode = insn.Word(2);
    const uint32_t value = insn.Length() > 3u ? insn.Word(3) : 0u;
    switch (execution_mode) {
        case spv::ExecutionModeOutputPoints:  // for geometry shaders
            flags |= output_points_bit;
            primitive_topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            break;
        case spv::ExecutionModePointMode:  // for tessellation shaders
            flags |= point_mode_bit;
            break;
        case spv::ExecutionModePostDepthCoverage:  // VK_EXT_post_depth_coverage
            flags |= post_depth_coverage_bit;
            break;
        case spv::ExecutionModeIsolines:  // Tessellation
            flags |= iso_lines_bit;
            tessellation_subdivision = spv::ExecutionModeIsolines;
            primitive_topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            break;
        case spv::ExecutionModeOutputLineStrip:
        case spv::ExecutionModeOutputLinesEXT:  // alias ExecutionModeOutputLinesNV
            primitive_topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            break;
        case spv::ExecutionModeTriangles:
            // ExecutionModeTriangles is input if shader is geometry and output if shader is tessellation evaluation
            // Because we don't know which shader stage is used here we set both, but only set input for geometry shader if it
            // hasn't been set yet
            if (input_primitive_topology == VK_PRIMITIVE_TOPOLOGY_MAX_ENUM) {
                input_primitive_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            }
            primitive_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            tessellation_subdivision = spv::ExecutionModeTriangles;
            break;
        case spv::ExecutionModeQuads:
            primitive_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            tessellation_subdivision = spv::ExecutionModeQuads;
            break;
        case spv::ExecutionModeOutputTriangleStrip:
        case spv::ExecutionModeOutputTrianglesEXT:  // alias ExecutionModeOutputTrianglesNV
            primitive_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            break;
        case spv::ExecutionModeInputPoints:
            input_primitive_topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            break;
        case spv::ExecutionModeInputLines:
        case spv::ExecutionModeInputLinesAdjacency:
            input_primitive_topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            break;
        case spv::ExecutionModeInputTrianglesAdjacency:
            input_primitive_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
        case spv::ExecutionModeLocalSizeId:
            flags |= local_size_id_bit;
            // Store ID here, will use flag to know to pull then out
            local_size_x = insn.Word(3);
            local_size_y = insn.Word(4);
            local_size_z = insn.Word(5);
            break;
        case spv::ExecutionModeLocalSize:
            flags |= local_size_bit;
            local_size_x = insn.Word(3);
            local_size_y = insn.Word(4);
            local_size_z = insn.Word(5);
            break;
        case spv::ExecutionModeOutputVertices:
            output_vertices = value;
            break;
        case spv::ExecutionModeOutputPrimitivesEXT:  // alias ExecutionModeOutputPrimitivesNV
            output_primitives = value;
            break;
        case spv::ExecutionModeXfb:  // TransformFeedback
            flags |= xfb_bit;
            break;
        case spv::ExecutionModeInvocations:
            invocations = value;
            break;
        case spv::ExecutionModeSignedZeroInfNanPreserve:  // VK_KHR_shader_float_controls
            if (value == 16) {
                flags |= signed_zero_inf_nan_preserve_width_16;
            } else if (value == 32) {
                flags |= signed_zero_inf_nan_preserve_width_32;
            } else if (value == 64) {
                flags |= signed_zero_inf_nan_preserve_width_64;
            }
            break;
        case spv::ExecutionModeDenormPreserve:  // VK_KHR_shader_float_controls
            if (value == 16) {
                flags |= denorm_preserve_width_16;
            } else if (value == 32) {
                flags |= denorm_preserve_width_32;
            } else if (value == 64) {
                flags |= denorm_preserve_width_64;
            }
            break;
        case spv::ExecutionModeDenormFlushToZero:  // VK_KHR_shader_float_controls
            if (value == 16) {
                flags |= denorm_flush_to_zero_width_16;
            } else if (value == 32) {
                flags |= denorm_flush_to_zero_width_32;
            } else if (value == 64) {
                flags |= denorm_flush_to_zero_width_64;
            }
            break;
        case spv::ExecutionModeRoundingModeRTE:  // VK_KHR_shader_float_controls
            if (value == 16) {
                flags |= rounding_mode_rte_width_16;
            } else if (value == 32) {
                flags |= rounding_mode_rte_width_32;
            } else if (value == 64) {
                flags |= rounding_mode_rte_width_64;
            }
            break;
        case spv::ExecutionModeRoundingModeRTZ:  // VK_KHR_shader_float_controls
            if (value == 16) {
                flags |= rounding_mode_rtz_width_16;
            } else if (value == 32) {
                flags |= rounding_mode_rtz_width_32;
            } else if (value == 64) {
                flags |= rounding_mode_rtz_width_64;
            }
            break;
        case spv::ExecutionModeFPFastMathDefault:  // VK_KHR_shader_float_controls2
            // This is to indicate the mode was used
            // Will look up the ID later as need the entire module parsed first
            flags |= fp_fast_math_default;
            break;
        case spv::ExecutionModeEarlyFragmentTests:
            flags |= early_fragment_test_bit;
            break;
        case spv::ExecutionModeSubgroupUniformControlFlowKHR:  // VK_KHR_shader_subgroup_uniform_control_flow
            flags |= subgroup_uniform_control_flow_bit;
            break;
        case spv::ExecutionModeSpacingEqual:
            tessellation_spacing = spv::ExecutionModeSpacingEqual;
            break;
        case spv::ExecutionModeSpacingFractionalEven:
            tessellation_spacing = spv::ExecutionModeSpacingFractionalEven;
            break;
        case spv::ExecutionModeSpacingFractionalOdd:
            tessellation_spacing = spv::ExecutionModeSpacingFractionalOdd;
            break;
        case spv::ExecutionModeVertexOrderCw:
            tessellation_orientation = spv::ExecutionModeVertexOrderCw;
            break;
        case spv::ExecutionModeVertexOrderCcw:
            tessellation_orientation = spv::ExecutionModeVertexOrderCcw;
            break;
        case spv::ExecutionModeDepthReplacing:
            flags |= depth_replacing_bit;
            break;
        case spv::ExecutionModeStencilRefReplacingEXT:
            flags |= stencil_ref_replacing_bit;
            break;
        case spv::ExecutionModeDerivativeGroupLinearKHR:
            flags |= derivative_group_linear;
            break;
        case spv::ExecutionModeDerivativeGroupQuadsKHR:
            flags |= derivative_group_quads;
            break;
        default:
            break;
    }
}

static uint32_t ExecutionModelToShaderStageFlagBits(uint32_t mode) {
    switch (mode) {
        case spv::ExecutionModelVertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case spv::ExecutionModelTessellationControl:
            return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case spv::ExecutionModelTessellationEvaluation:
            return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case spv::ExecutionModelGeometry:
            return VK_SHADER_STAGE_GEOMETRY_BIT;
        case spv::ExecutionModelFragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case spv::ExecutionModelGLCompute:
            return VK_SHADER_STAGE_COMPUTE_BIT;
        case spv::ExecutionModelRayGenerationKHR:
            return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        case spv::ExecutionModelAnyHitKHR:
            return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        case spv::ExecutionModelClosestHitKHR:
            return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        case spv::ExecutionModelMissKHR:
            return VK_SHADER_STAGE_MISS_BIT_KHR;
        case spv::ExecutionModelIntersectionKHR:
            return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        case spv::ExecutionModelCallableKHR:
            return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
        case spv::ExecutionModelTaskNV:
            return VK_SHADER_STAGE_TASK_BIT_NV;
        case spv::ExecutionModelMeshNV:
            return VK_SHADER_STAGE_MESH_BIT_NV;
        case spv::ExecutionModelTaskEXT:
            return VK_SHADER_STAGE_TASK_BIT_EXT;
        case spv::ExecutionModelMeshEXT:
            return VK_SHADER_STAGE_MESH_BIT_EXT;
        default:
            return 0;
    }
}

// TODO: The set of interesting opcodes here was determined by eyeballing the SPIRV spec. It might be worth
// converting parts of this to be generated from the machine-readable spec instead.
static void FindPointersAndObjects(const Instruction& insn, vvl::unordered_set<uint32_t>& result) {
    switch (insn.Opcode()) {
        case spv::OpLoad:
            result.insert(insn.Word(3));  // ptr
            break;
        case spv::OpStore:
            result.insert(insn.Word(1));  // ptr
            break;
        case spv::OpAccessChain:
        case spv::OpInBoundsAccessChain:
            result.insert(insn.Word(3));  // base ptr
            break;
        case spv::OpArrayLength:
            // This is not an access of memory, but counts as static usage of the variable
            result.insert(insn.Word(3));
            break;
        case spv::OpSampledImage:
        case spv::OpImageSampleImplicitLod:
        case spv::OpImageSampleExplicitLod:
        case spv::OpImageSampleDrefImplicitLod:
        case spv::OpImageSampleDrefExplicitLod:
        case spv::OpImageSampleProjImplicitLod:
        case spv::OpImageSampleProjExplicitLod:
        case spv::OpImageSampleProjDrefImplicitLod:
        case spv::OpImageSampleProjDrefExplicitLod:
        case spv::OpImageFetch:
        case spv::OpImageGather:
        case spv::OpImageDrefGather:
        case spv::OpImageRead:
        case spv::OpImage:
        case spv::OpImageQueryFormat:
        case spv::OpImageQueryOrder:
        case spv::OpImageQuerySizeLod:
        case spv::OpImageQuerySize:
        case spv::OpImageQueryLod:
        case spv::OpImageQueryLevels:
        case spv::OpImageQuerySamples:
        case spv::OpImageSparseSampleImplicitLod:
        case spv::OpImageSparseSampleExplicitLod:
        case spv::OpImageSparseSampleDrefImplicitLod:
        case spv::OpImageSparseSampleDrefExplicitLod:
        case spv::OpImageSparseSampleProjImplicitLod:
        case spv::OpImageSparseSampleProjExplicitLod:
        case spv::OpImageSparseSampleProjDrefImplicitLod:
        case spv::OpImageSparseSampleProjDrefExplicitLod:
        case spv::OpImageSparseFetch:
        case spv::OpImageSparseGather:
        case spv::OpImageSparseDrefGather:
        case spv::OpImageTexelPointer:
        case spv::OpFragmentFetchAMD:
        case spv::OpFragmentMaskFetchAMD:
            // Note: we only explore parts of the image which might actually contain ids we care about for the above analyses.
            //  - NOT the shader input/output interfaces.
            result.insert(insn.Word(3));  // Image or sampled image
            break;
        case spv::OpImageWrite:
            result.insert(insn.Word(1));  // Image -- different operand order to above
            break;
        case spv::OpFunctionCall:
            for (uint32_t i = 3; i < insn.Length(); i++) {
                result.insert(insn.Word(i));  // fn itself, and all args
            }
            break;

        case spv::OpExtInst:
            for (uint32_t i = 5; i < insn.Length(); i++) {
                result.insert(insn.Word(i));  // Operands to ext inst
            }
            break;

        default: {
            if (AtomicOperation(insn.Opcode())) {
                if (insn.Opcode() == spv::OpAtomicStore) {
                    result.insert(insn.Word(1));  // ptr
                } else {
                    result.insert(insn.Word(3));  // ptr
                }
            }
            break;
        }
    }
}

// Built-in can be both on the OpVariable or a inside a OpTypeStruct for Block built-in.
bool EntryPoint::IsBuiltInWritten(spv::BuiltIn built_in, const Module& module_state, const StageInterfaceVariable& variable,
                                  const AccessChainVariableMap& access_chain_map) {
    if (!variable.IsWrittenTo()) {
        return false;
    }
    if (built_in == variable.decorations.builtin) {
        return true;  // The built-in is on the Variable
    } else if (!variable.type_struct_info || variable.type_struct_info->decorations.member_decorations.empty()) {
        return false;
    }

    for (const auto& member : variable.type_struct_info->decorations.member_decorations) {
        if (built_in != member.second.builtin) continue;

        // We have confirmed the Block variable was written to, now need to confirm an access to.
        // Because Built-in can't both be the input and output at the same time, we can confirm all accesses are either all
        // loads or all stores.
        const auto it = access_chain_map.find(variable.id);
        if (it == access_chain_map.end()) {
            return false;
        }
        const uint32_t member_index = member.first;
        for (const auto access_chain_insn : it->second) {
            if (access_chain_insn->Length() < 5) continue;

            // We know for sure any built-in inside a block are only 1-element deep so can just check the "Indexes 0" operand
            // Also no built-in we are dealing with are inside array-of-structs
            const Instruction* value_def = module_state.GetConstantDef(access_chain_insn->Word(4));
            if (value_def) {
                const uint32_t value = value_def->GetConstantValue();
                if (value == member_index) {
                    return true;
                }
            }
        }
        break;
    }
    return false;
}

bool EntryPoint::HasBuiltIn(spv::BuiltIn built_in) const {
    for (const auto* variable : built_in_variables) {
        if (variable->decorations.builtin == built_in) {
            return true;
        }
    }
    return false;
}

vvl::unordered_set<uint32_t> EntryPoint::GetAccessibleIds(const Module& module_state, EntryPoint& entrypoint) {
    vvl::unordered_set<uint32_t> result_ids;

    // For some analyses, we need to know about all ids referenced by the static call tree of a particular entrypoint.
    // This is important for identifying the set of shader resources actually used by an entrypoint.
    vvl::unordered_set<uint32_t> worklist;
    worklist.insert(entrypoint.id);

    while (!worklist.empty()) {
        auto worklist_id_iter = worklist.begin();
        auto worklist_id = *worklist_id_iter;
        worklist.erase(worklist_id_iter);

        const Instruction* next_insn = module_state.FindDef(worklist_id);
        if (!next_insn) {
            // ID is something we didn't collect in SpirvStaticData. that's OK -- we'll stumble across all kinds of things here
            // that we may not care about.
            continue;
        }

        // Try to add to the output set
        if (!result_ids.insert(worklist_id).second) {
            continue;  // If we already saw this id, we don't want to walk it again.
        }

        if (next_insn->Opcode() == spv::OpFunction) {
            // Scan whole body of the function
            while (++next_insn, next_insn->Opcode() != spv::OpFunctionEnd) {
                const auto& insn = *next_insn;
                // Build up list of accessible ID
                FindPointersAndObjects(insn, worklist);

                // Gather any instructions info that is only for the EntryPoint and not whole module
                switch (insn.Opcode()) {
                    case spv::OpEmitVertex:
                    case spv::OpEmitStreamVertex:
                        entrypoint.emit_vertex_geometry = true;
                        break;
                    default:
                        break;
                }
            }
        }
    }

    return result_ids;
}

std::vector<StageInterfaceVariable> EntryPoint::GetStageInterfaceVariables(const Module& module_state, const EntryPoint& entrypoint,
                                                                           const VariableAccessMap& variable_access_map,
                                                                           const DebugNameMap& debug_name_map) {
    std::vector<StageInterfaceVariable> variables;

    // spirv-val validates that any Input/Output used in the entrypoint is listed in as interface IDs
    uint32_t word = 3;  // operand Name operand starts
    // Find the end of the entrypoint's name string. additional zero bytes follow the actual null terminator, to fill out
    // the rest of the word - so we only need to look at the last byte in the word to determine which word contains the
    // terminator.
    while (entrypoint.entrypoint_insn.Word(word) & 0xff000000u) {
        ++word;
    }
    ++word;

    vvl::unordered_set<uint32_t> unique_interface_id;
    for (; word < entrypoint.entrypoint_insn.Length(); word++) {
        const uint32_t interface_id = entrypoint.entrypoint_insn.Word(word);
        if (unique_interface_id.insert(interface_id).second == false) {
            continue;  // Before SPIR-V 1.4 duplicates of these IDs are allowed
        };
        // guaranteed by spirv-val to be a OpVariable
        const Instruction& insn = *module_state.FindDef(interface_id);

        if (insn.Word(3) != spv::StorageClassInput && insn.Word(3) != spv::StorageClassOutput) {
            continue;  // Only checking for input/output here
        }
        variables.emplace_back(module_state, insn, entrypoint.stage, variable_access_map, debug_name_map);
    }
    return variables;
}

std::vector<ResourceInterfaceVariable> EntryPoint::GetResourceInterfaceVariables(const Module& module_state, EntryPoint& entrypoint,
                                                                                 const ImageAccessMap& image_access_map,
                                                                                 const AccessChainVariableMap& access_chain_map,
                                                                                 const VariableAccessMap& variable_access_map,
                                                                                 const DebugNameMap& debug_name_map) {
    std::vector<ResourceInterfaceVariable> variables;

    // Now that the accessible_ids list is known, fill in any information that can be statically known per EntryPoint
    for (const auto& accessible_id : entrypoint.accessible_ids) {
        const Instruction& insn = *module_state.FindDef(accessible_id);
        if (insn.Opcode() != spv::OpVariable) {
            continue;
        }
        const uint32_t storage_class = insn.StorageClass();
        // These are the only storage classes that interface with a descriptor
        // see vkspec.html#interfaces-resources-descset
        if (storage_class == spv::StorageClassUniform || storage_class == spv::StorageClassUniformConstant ||
            storage_class == spv::StorageClassStorageBuffer) {
            variables.emplace_back(module_state, entrypoint, insn, image_access_map, access_chain_map, variable_access_map,
                                   debug_name_map);
        } else if (storage_class == spv::StorageClassPushConstant) {
            entrypoint.push_constant_variable =
                std::make_shared<PushConstantVariable>(module_state, insn, entrypoint.stage, variable_access_map, debug_name_map);
        }
    }
    return variables;
}

static inline bool IsImageOperandsBiasOffset(uint32_t type) {
    return (type & (spv::ImageOperandsBiasMask | spv::ImageOperandsConstOffsetMask | spv::ImageOperandsOffsetMask |
                    spv::ImageOperandsConstOffsetsMask)) != 0;
}

ImageAccess::ImageAccess(const Module& module_state, const Instruction& image_insn, const FuncParameterMap& func_parameter_map)
    : image_insn(image_insn) {
    const uint32_t image_opcode = image_insn.Opcode();

    // Get properties from each access instruction
    switch (image_opcode) {
        case spv::OpImageDrefGather:
        case spv::OpImageSparseDrefGather:
            is_dref = true;
            break;

        case spv::OpImageSampleDrefImplicitLod:
        case spv::OpImageSampleDrefExplicitLod:
        case spv::OpImageSampleProjDrefImplicitLod:
        case spv::OpImageSampleProjDrefExplicitLod:
        case spv::OpImageSparseSampleDrefImplicitLod:
        case spv::OpImageSparseSampleDrefExplicitLod:
        case spv::OpImageSparseSampleProjDrefImplicitLod:
        case spv::OpImageSparseSampleProjDrefExplicitLod: {
            is_dref = true;
            is_sampler_implicitLod_dref_proj = true;
            is_sampler_sampled = true;
            break;
        }

        case spv::OpImageSampleImplicitLod:
        case spv::OpImageSampleProjImplicitLod:
        case spv::OpImageSampleProjExplicitLod:
        case spv::OpImageSparseSampleImplicitLod:
        case spv::OpImageSparseSampleProjImplicitLod:
        case spv::OpImageSparseSampleProjExplicitLod: {
            is_sampler_implicitLod_dref_proj = true;
            is_sampler_sampled = true;
            break;
        }

        case spv::OpImageSampleExplicitLod:
        case spv::OpImageSparseSampleExplicitLod: {
            is_sampler_sampled = true;
            break;
        }

        case spv::OpImageWrite:
            texel_component_count = module_state.GetTexelComponentCount(image_insn);
            break;

        case spv::OpImageRead:
        case spv::OpImageSparseRead:
        case spv::OpImageTexelPointer:
        case spv::OpImageFetch:
        case spv::OpImageSparseFetch:
        case spv::OpImageGather:
        case spv::OpImageSparseGather:
        case spv::OpImageQueryLod:
        case spv::OpFragmentFetchAMD:
        case spv::OpFragmentMaskFetchAMD:
            break;

        case spv::OpImageSparseTexelsResident:
            assert(false);  // This is not a proper OpImage* instruction, has no OpImage operand
            break;

        default:
            assert(false);  // This is an OpImage* we are not catching
            break;
    }

    // There is only one way to write to images, everything else is considered a read access
    access_mask |= (image_opcode == spv::OpImageWrite) ? AccessBit::image_write : AccessBit::image_read;

    is_not_sampler_sampled = !is_sampler_sampled;

    // Find any optional Image Operands
    const uint32_t image_operand_position = OpcodeImageOperandsPosition(image_opcode);
    if (image_insn.Length() > image_operand_position) {
        const uint32_t image_operand_word = image_insn.Word(image_operand_position);

        if (is_sampler_sampled) {
            if (IsImageOperandsBiasOffset(image_operand_word)) {
                is_sampler_bias_offset = true;
            }
            if ((image_operand_word & (spv::ImageOperandsConstOffsetMask | spv::ImageOperandsOffsetMask)) != 0) {
                is_sampler_offset = true;
            }
        }

        if ((image_operand_word & spv::ImageOperandsSignExtendMask) != 0) {
            is_sign_extended = true;
        } else if ((image_operand_word & spv::ImageOperandsZeroExtendMask) != 0) {
            is_zero_extended = true;
        }
    }

    // Do sampler searching as seperate walk to not have the "visited" loop protection be falsly triggered
    std::vector<const Instruction*> sampler_insn_to_search;

    auto walk_to_variables = [this, &module_state, &func_parameter_map, &sampler_insn_to_search](const Instruction* insn,
                                                                                                 bool sampler) {
        // Protect from loops
        vvl::unordered_set<uint32_t> visited;

        // stack of function call sites to search through
        std::queue<const Instruction*> insn_to_search;
        insn_to_search.push(insn);
        bool new_func = false;

        // Keep walking down until get to variables
        while (!insn_to_search.empty()) {
            // for debugging, easier if only search one function at a time
            if (new_func) {
                // If any function can't resolve to a variable, by design,
                // it will kill searching other functions and those before it are now invalidated.
                new_func = false;
                insn = insn_to_search.front();
                // spirv-val makes sure functions-to-functions are not recursive
                visited.clear();
            }

            const uint32_t current_id = insn->ResultId();
            const auto visited_iter = visited.find(current_id);
            if (visited_iter != visited.end()) {
                valid_access = false;  // Caught in a loop
                return;
            }
            visited.insert(current_id);

            switch (insn->Opcode()) {
                case spv::OpSampledImage:
                    // If there is a OpSampledImage we will need to split off and walk down to get the sampler variable
                    sampler_insn_to_search.push_back(module_state.FindDef(insn->Word(4)));
                    insn = module_state.FindDef(insn->Word(3));
                    break;
                case spv::OpImage:
                    // OpImageFetch grabs OpImage before OpLoad
                    insn = module_state.FindDef(insn->Word(3));
                    break;
                case spv::OpLoad:
                    // Follow the pointer being loaded
                    insn = module_state.FindDef(insn->Word(3));
                    break;
                case spv::OpCopyObject:
                    // Follow the object being copied.
                    insn = module_state.FindDef(insn->Word(3));
                    break;
                case spv::OpAccessChain:
                case spv::OpInBoundsAccessChain:
                case spv::OpPtrAccessChain:
                case spv::OpInBoundsPtrAccessChain: {
                    // If Image is an array (but not descriptor indexing), then need to get the index.
                    const Instruction* const_def = module_state.GetConstantDef(insn->Word(4));
                    if (const_def) {
                        image_access_chain_index = const_def->GetConstantValue();
                    }
                    insn = module_state.FindDef(insn->Word(3));
                    break;
                }
                case spv::OpFunctionParameter: {
                    // might be dead-end, but end searching in this Function block
                    insn_to_search.pop();
                    new_func = true;

                    auto it = func_parameter_map.find(insn->ResultId());
                    if (it != func_parameter_map.end()) {
                        for (uint32_t arg : it->second) {
                            insn_to_search.push(module_state.FindDef(arg));
                        }
                    }
                    break;
                }
                case spv::OpVariable: {
                    if (sampler) {
                        variable_sampler_insn.push_back(insn);
                    } else {
                        variable_image_insn.push_back(insn);
                    }
                    insn_to_search.pop();
                    new_func = true;  // keep searching if more functions
                    break;
                }
                default:
                    // Hit invalid (or unsupported) opcode
                    valid_access = false;
                    return;
            }
        }
    };

    const uint32_t image_operand = OpcodeImageAccessPosition(image_opcode);
    assert(image_operand != 0);
    const Instruction* insn = module_state.FindDef(image_insn.Word(image_operand));
    walk_to_variables(insn, false);
    for (const auto* sampler_insn : sampler_insn_to_search) {
        walk_to_variables(sampler_insn, true);
    }
}

EntryPoint::EntryPoint(const Module& module_state, const Instruction& entrypoint_insn, const ImageAccessMap& image_access_map,
                       const AccessChainVariableMap& access_chain_map, const VariableAccessMap& variable_access_map,
                       const DebugNameMap& debug_name_map)
    : entrypoint_insn(entrypoint_insn),
      execution_model(spv::ExecutionModel(entrypoint_insn.Word(1))),
      stage(static_cast<VkShaderStageFlagBits>(ExecutionModelToShaderStageFlagBits(execution_model))),
      id(entrypoint_insn.Word(2)),
      name(entrypoint_insn.GetAsString(3)),
      execution_mode(module_state.GetExecutionModeSet(id)),
      emit_vertex_geometry(false),
      accessible_ids(GetAccessibleIds(module_state, *this)),
      resource_interface_variables(GetResourceInterfaceVariables(module_state, *this, image_access_map, access_chain_map,
                                                                 variable_access_map, debug_name_map)),
      stage_interface_variables(GetStageInterfaceVariables(module_state, *this, variable_access_map, debug_name_map)) {
    // After all variables are made, can get references from them
    // Also can set per-Entrypoint values now
    for (const auto& variable : stage_interface_variables) {
        if (variable.is_per_task_nv) {
            continue;  // SPV_NV_mesh_shader has a PerTaskNV which is not a builtin or interface
        }
        has_passthrough |= variable.decorations.Has(DecorationSet::passthrough_bit);

        if (variable.is_builtin) {
            built_in_variables.push_back(&variable);

            if (variable.storage_class == spv::StorageClassInput) {
                builtin_input_components += variable.total_builtin_components;
            } else if (variable.storage_class == spv::StorageClassOutput) {
                builtin_output_components += variable.total_builtin_components;
            }

            if (IsBuiltInWritten(spv::BuiltInPrimitiveShadingRateKHR, module_state, variable, access_chain_map)) {
                written_builtin_primitive_shading_rate_khr = true;
            }
            if (IsBuiltInWritten(spv::BuiltInViewportIndex, module_state, variable, access_chain_map)) {
                written_builtin_viewport_index = true;
            }
            if (IsBuiltInWritten(spv::BuiltInPointSize, module_state, variable, access_chain_map)) {
                written_builtin_point_size = true;
            }
            if (IsBuiltInWritten(spv::BuiltInLayer, module_state, variable, access_chain_map)) {
                written_builtin_layer = true;
            }
            if (IsBuiltInWritten(spv::BuiltInViewportMaskNV, module_state, variable, access_chain_map)) {
                written_builtin_viewport_mask_nv = true;
            }
        } else {
            user_defined_interface_variables.push_back(&variable);

            if (variable.base_type.StorageClass() == spv::StorageClassPhysicalStorageBuffer) {
                has_physical_storage_buffer_interface = true;
            }

            // After creating, make lookup table
            if (variable.interface_slots.empty()) {
                continue;  // will skip for things like PhysicalStorageBuffer
            }
            for (const auto& slot : variable.interface_slots) {
                if (variable.storage_class == spv::StorageClassInput) {
                    input_interface_slots[slot] = &variable;
                    if (!max_input_slot || slot.slot > max_input_slot->slot) {
                        max_input_slot = &slot;
                        max_input_slot_variable = &variable;
                    }
                } else if (variable.storage_class == spv::StorageClassOutput) {
                    output_interface_slots[slot] = &variable;
                    if (!max_output_slot || slot.slot > max_output_slot->slot) {
                        max_output_slot = &slot;
                        max_output_slot_variable = &variable;
                    }
                    // Dual source blending can use a non-index of zero here
                    if (slot.Location() == 0 && slot.Component() == 3 && variable.decorations.index == 0) {
                        has_alpha_to_coverage_variable = true;
                    }
                }
            }
        }
    }
}

std::optional<VkPrimitiveTopology> Module::GetTopology(const EntryPoint& entrypoint) const {
    std::optional<VkPrimitiveTopology> result;

    // In tessellation shaders, PointMode is separate and trumps the tessellation topology.
    if (entrypoint.execution_mode.Has(ExecutionModeSet::point_mode_bit)) {
        result.emplace(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
    } else if (entrypoint.execution_mode.primitive_topology != VK_PRIMITIVE_TOPOLOGY_MAX_ENUM) {
        result.emplace(entrypoint.execution_mode.primitive_topology);
    }

    return result;
}

Module::StaticData::StaticData(const Module& module_state, StatelessData* stateless_data) {
    if (!module_state.valid_spirv) return;

    // Parse the words first so we have instruction class objects to use
    {
        std::vector<uint32_t>::const_iterator it = module_state.words_.cbegin();
        it += 5;  // skip first 5 word of header
        instructions.reserve(module_state.words_.size() * 4);
        while (it != module_state.words_.cend()) {
            auto new_insn = instructions.emplace_back(it);
            const uint32_t opcode = new_insn.Opcode();

            // Check for opcodes that would require reparsing of the words
            if (opcode == spv::OpGroupDecorate || opcode == spv::OpDecorationGroup || opcode == spv::OpGroupMemberDecorate) {
                if (stateless_data) {
                    assert(stateless_data->has_group_decoration == false);  // if assert, spirv-opt didn't flatten it
                    stateless_data->has_group_decoration = true;
                    return;  // no need to continue parsing
                }
            }

            it += new_insn.Length();
        }
        instructions.shrink_to_fit();
    }

    // These have their own object class, but need entire module parsed first
    std::vector<const Instruction*> entry_point_instructions;
    std::vector<const Instruction*> type_struct_instructions;
    std::vector<const Instruction*> image_instructions;
    std::vector<const Instruction*> func_call_instructions;
    // both OpDecorate and OpMemberDecorate builtin instructions
    std::vector<const Instruction*> builtin_decoration_instructions;

    DebugNameMap debug_name_map;

    std::vector<uint32_t> store_pointer_ids;
    std::vector<uint32_t> load_pointer_ids;
    std::vector<uint32_t> atomic_store_pointer_ids;
    std::vector<uint32_t> atomic_load_pointer_ids;

    AccessChainVariableMap access_chain_map;

    uint32_t last_func_id = 0;
    // < Function ID, OpFunctionParameter Ids >
    vvl::unordered_map<uint32_t, std::vector<uint32_t>> func_parameter_list;

    // Loop through once and build up the static data
    // Also process the entry points
    for (const Instruction& insn : instructions) {
        // Build definition list
        const uint32_t result_id = insn.ResultId();
        if (result_id != 0) {
            definitions[result_id] = &insn;
        }

        const uint32_t opcode = insn.Opcode();
        switch (opcode) {
            // Specialization constants
            case spv::OpSpecConstantTrue:
            case spv::OpSpecConstantFalse:
            case spv::OpSpecConstant:
            case spv::OpSpecConstantComposite:
            case spv::OpSpecConstantOp:
                has_specialization_constants = true;
                break;

            // Decorations
            case spv::OpDecorate: {
                const uint32_t target_id = insn.Word(1);
                decorations[target_id].Add(insn.Word(2), insn.Length() > 3u ? insn.Word(3) : 0u);
                decoration_inst.push_back(&insn);
                if (insn.Word(2) == spv::DecorationBuiltIn) {
                    builtin_decoration_instructions.push_back(&insn);
                } else if (insn.Word(2) == spv::DecorationSpecId) {
                    id_to_spec_id[target_id] = insn.Word(3);
                }
            } break;
            case spv::OpMemberDecorate: {
                const uint32_t target_id = insn.Word(1);
                const uint32_t member_index = insn.Word(2);
                decorations[target_id].member_decorations[member_index].Add(insn.Word(3), insn.Length() > 4u ? insn.Word(4) : 0u);
                member_decoration_inst.push_back(&insn);
                if (insn.Word(3) == spv::DecorationBuiltIn) {
                    builtin_decoration_instructions.push_back(&insn);
                }
            } break;

            case spv::OpCapability:
                capability_list.push_back(static_cast<spv::Capability>(insn.Word(1)));
                // Cache frequently checked capabilities
                if (capability_list.back() == spv::CapabilityRuntimeDescriptorArray) {
                    has_capability_runtime_descriptor_array = true;
                }
                break;

            case spv::OpVariable:
                variable_inst.push_back(&insn);
                break;

            case spv::OpEmitStreamVertex:
            case spv::OpEndStreamPrimitive:
                if (stateless_data) {
                    stateless_data->transform_feedback_stream_inst.push_back(&insn);
                }
                break;

            // Execution Mode
            case spv::OpExecutionMode:
            case spv::OpExecutionModeId: {
                execution_modes[insn.Word(1)].Add(insn);

                // Some OpExecutionModeId will have IDs after that need the entire module parsed first,
                if (stateless_data && opcode == spv::OpExecutionModeId) {
                    stateless_data->execution_mode_id_inst.push_back(&insn);
                }
            } break;
            // Listed from vkspec.html#ray-tracing-repack
            case spv::OpTraceRayKHR:
            case spv::OpTraceRayMotionNV:
            case spv::OpReportIntersectionKHR:
            case spv::OpExecuteCallableKHR:
                if (stateless_data) {
                    stateless_data->has_invocation_repack_instruction = true;
                }
                break;

            // Entry points
            case spv::OpEntryPoint: {
                entry_point_instructions.push_back(&insn);
                break;
            }

            // Shader Tile image instructions
            case spv::OpDepthAttachmentReadEXT:
                has_shader_tile_image_depth_read = true;
                break;
            case spv::OpStencilAttachmentReadEXT:
                has_shader_tile_image_stencil_read = true;
                break;
            case spv::OpColorAttachmentReadEXT:
                has_shader_tile_image_color_read = true;
                break;

            // Access operations
            case spv::OpImageSampleImplicitLod:
            case spv::OpImageSampleProjImplicitLod:
            case spv::OpImageSampleProjExplicitLod:
            case spv::OpImageSparseSampleImplicitLod:
            case spv::OpImageSparseSampleProjImplicitLod:
            case spv::OpImageSparseSampleProjExplicitLod:
            case spv::OpImageDrefGather:
            case spv::OpImageSparseDrefGather:
            case spv::OpImageSampleDrefImplicitLod:
            case spv::OpImageSampleDrefExplicitLod:
            case spv::OpImageSampleProjDrefImplicitLod:
            case spv::OpImageSampleProjDrefExplicitLod:
            case spv::OpImageSparseSampleDrefImplicitLod:
            case spv::OpImageSparseSampleDrefExplicitLod:
            case spv::OpImageSparseSampleProjDrefImplicitLod:
            case spv::OpImageSparseSampleProjDrefExplicitLod:
            case spv::OpImageSampleExplicitLod:
            case spv::OpImageSparseSampleExplicitLod:
            case spv::OpImageRead:
            case spv::OpImageSparseRead:
            case spv::OpImageFetch:
            case spv::OpImageGather:
            case spv::OpImageQueryLod:
            case spv::OpImageSparseFetch:
            case spv::OpImageSparseGather:
            case spv::OpFragmentFetchAMD:
            case spv::OpFragmentMaskFetchAMD: {
                image_instructions.push_back(&insn);
                break;
            }
            case spv::OpImageQuerySizeLod:
            case spv::OpImageQuerySize:
            case spv::OpImageQueryLevels:
            case spv::OpImageQuerySamples:
                // from spec "return properties of the image descriptor that would be accessed. The image itself is not accessed."
                break;
            case spv::OpStore: {
                store_pointer_ids.emplace_back(insn.Word(1));  // object id or AccessChain id
                break;
            }
            case spv::OpImageWrite: {
                image_instructions.push_back(&insn);
                image_write_load_id_map.emplace(&insn, insn.Word(1));
                break;
            }
            case spv::OpLoad: {
                load_pointer_ids.emplace_back(insn.Word(3));  // object id or AccessChain id
                break;
            }
            case spv::OpAccessChain:
            case spv::OpInBoundsAccessChain: {
                const uint32_t base_id = insn.Word(3);
                access_chain_map[base_id].push_back(&insn);
                break;
            }
            case spv::OpImageTexelPointer: {
                // All Image atomics go through here.
                // Currrently only interested if used/accessed
                image_instructions.push_back(&insn);
                break;
            }
            case spv::OpTypeStruct: {
                type_struct_instructions.push_back(&insn);
                break;
            }
            case spv::OpReadClockKHR: {
                if (stateless_data) {
                    stateless_data->read_clock_inst.push_back(&insn);
                }
                break;
            }
            case spv::OpTypeCooperativeMatrixNV:
            case spv::OpCooperativeMatrixMulAddNV:
            case spv::OpTypeCooperativeMatrixKHR:
            case spv::OpCooperativeMatrixMulAddKHR: {
                cooperative_matrix_inst.push_back(&insn);
                break;
            }

            case spv::OpTypeCooperativeVectorNV:
            case spv::OpCooperativeVectorLoadNV:
            case spv::OpCooperativeVectorStoreNV:
            case spv::OpCooperativeVectorMatrixMulNV:
            case spv::OpCooperativeVectorMatrixMulAddNV:
            case spv::OpCooperativeVectorReduceSumAccumulateNV:
            case spv::OpCooperativeVectorOuterProductAccumulateNV: {
                cooperative_vector_inst.push_back(&insn);
                break;
            }

            case spv::OpExtInst: {
                if (insn.Word(4) == GLSLstd450InterpolateAtSample) {
                    uses_interpolate_at_sample = true;
                }
                break;
            }

            case spv::OpName: {
                debug_name_map[insn.Word(1)] = &insn;
                break;
            }

            case spv::OpLine:
            case spv::OpSource: {
                using_legacy_debug_info = true;
                break;
            }
            case spv::OpExtInstImport: {
                if (strcmp(insn.GetAsString(2), "NonSemantic.Shader.DebugInfo.100") == 0) {
                    shader_debug_info_set_id = insn.ResultId();
                }
                break;
            }

            // Build up Function mappings
            case spv::OpFunction:
                last_func_id = insn.ResultId();
                func_parameter_list[last_func_id];  // create empty vector list
                break;
            case spv::OpFunctionParameter:
                func_parameter_list[last_func_id].push_back(insn.ResultId());
                break;
            case spv::OpFunctionCall:
                func_call_instructions.push_back(&insn);
                break;

            default:
                if (AtomicOperation(opcode)) {
                    if (stateless_data) {
                        stateless_data->atomic_inst.push_back(&insn);
                    }
                    if (opcode == spv::OpAtomicStore) {
                        atomic_store_pointer_ids.emplace_back(insn.Word(1));
                    } else {
                        atomic_load_pointer_ids.emplace_back(insn.Word(3));
                    }
                }
                if (GroupOperation(opcode)) {
                    if (stateless_data) {
                        stateless_data->group_inst.push_back(&insn);
                    }
                }
                // We don't care about any other defs for now.
                break;
        }
    }

    FuncParameterMap func_parameter_map;
    const uint32_t first_arg_word = 4;
    for (const auto& func_def : func_parameter_list) {
        const uint32_t func_id = func_def.first;
        for (const Instruction* func_call : func_call_instructions) {
            if (func_call->Word(3) != func_id) {
                continue;
            }
            // guaranteed number of args/params is same
            const uint32_t arg_count = (func_call->Length() - first_arg_word);
            for (uint32_t i = 0; i < arg_count; i++) {
                const uint32_t arg = func_call->Word(first_arg_word + i);
                const uint32_t param = func_def.second[i];
                func_parameter_map[param].push_back(arg);
            }
        }
    }

    // parsing, take every load/store find the variable it touches
    // (image access are done later)
    VariableAccessMap variable_access_map;
    auto mark_variable_access = [&module_state, &variable_access_map](const std::vector<uint32_t>& ids, uint32_t access) {
        for (const auto& object_id : ids) {
            uint32_t variable_id = object_id;
            const Instruction* insn = module_state.FindDef(object_id);
            while (insn) {
                switch (insn->Opcode()) {
                    case spv::OpImageTexelPointer:  // used for atomics
                    case spv::OpAccessChain:
                    case spv::OpInBoundsAccessChain:
                    case spv::OpCopyObject:
                        variable_id = insn->Word(3);
                        insn = module_state.FindDef(variable_id);
                        break;
                    case spv::OpVariable:
                        variable_access_map[variable_id] |= access;
                        insn = nullptr;
                        break;
                    default:
                        insn = nullptr;
                        break;
                }
            }
        }
    };

    mark_variable_access(store_pointer_ids, AccessBit::write);
    mark_variable_access(load_pointer_ids, AccessBit::read);
    mark_variable_access(atomic_store_pointer_ids, AccessBit::atomic_write);
    mark_variable_access(atomic_load_pointer_ids, AccessBit::atomic_read);

    for (const Instruction* decoration_inst : builtin_decoration_instructions) {
        const uint32_t built_in = decoration_inst->GetBuiltIn();
        if (built_in == spv::BuiltInLayer) {
            has_builtin_layer = true;
        } else if (built_in == spv::BuiltInFullyCoveredEXT) {
            if (stateless_data) {
                stateless_data->has_builtin_fully_covered = true;
            }
        } else if (built_in == spv::BuiltInWorkgroupSize) {
            has_builtin_workgroup_size = true;
            builtin_workgroup_size_id = decoration_inst->Word(1);
        } else if (built_in == spv::BuiltInDrawIndex) {
            has_builtin_draw_index = true;
        }
    }

    // Need to get struct first and EntryPoint's variables depend on it
    for (const auto& insn : type_struct_instructions) {
        auto new_struct = type_structs.emplace_back(std::make_shared<TypeStructInfo>(module_state, *insn));
        type_struct_map[new_struct->id] = new_struct;
    }

    // Need to get ImageAccesses as EntryPoint's variables depend on it
    std::vector<std::shared_ptr<ImageAccess>> image_accesses;
    ImageAccessMap image_access_map;

    for (const auto& insn : image_instructions) {
        auto new_access = image_accesses.emplace_back(std::make_shared<ImageAccess>(module_state, *insn, func_parameter_map));
        if (!new_access->variable_image_insn.empty() && new_access->valid_access) {
            for (const Instruction* image_insn : new_access->variable_image_insn) {
                image_access_map[image_insn->ResultId()].push_back(new_access);
            }
        }
    }

    // Need to build the definitions table for FindDef before looking for which instructions each entry point uses
    for (const auto& insn : entry_point_instructions) {
        entry_points.emplace_back(std::make_shared<EntryPoint>(module_state, *insn, image_access_map, access_chain_map,
                                                               variable_access_map, debug_name_map));
    }
}

std::string Module::GetDecorations(uint32_t id) const {
    std::ostringstream ss;
    for (const spirv::Instruction& insn : GetInstructions()) {
        if (insn.Opcode() == spv::OpFunction) {
            break;  // decorations are found before first function block
        } else if (insn.Opcode() == spv::OpDecorate && insn.Word(1) == id) {
            ss << " " << string_SpvDecoration(insn.Word(2));
        }
    }
    return ss.str();
}

std::string Module::GetName(uint32_t id) const {
    for (const spirv::Instruction& insn : GetInstructions()) {
        if (insn.Opcode() == spv::OpFunction) {
            break;  // names are found before first function block
        } else if (insn.Opcode() == spv::OpName && insn.Word(1) == id) {
            return insn.GetAsString(2);
        }
    }
    return "";
}

std::string Module::GetMemberName(uint32_t id, uint32_t member_index) const {
    for (const spirv::Instruction& insn : GetInstructions()) {
        if (insn.Opcode() == spv::OpFunction) {
            break;  // names are found before first function block
        } else if (insn.Opcode() == spv::OpMemberName && insn.Word(1) == id && insn.Word(2) == member_index) {
            return insn.GetAsString(3);
        }
    }
    return "";
}

// Used to pretty-print the OpType* for an error message
void Module::DescribeTypeInner(std::ostringstream& ss, uint32_t type, uint32_t indent) const {
    const Instruction* insn = FindDef(type);
    auto indent_by = [&ss](uint32_t i) {
        for (uint32_t x = 0; x < i; x++) {
            ss << '\t';
        }
    };

    switch (insn->Opcode()) {
        case spv::OpTypeBool:
            ss << "bool";
            break;
        case spv::OpTypeInt:
            ss << (insn->Word(3) ? 's' : 'u') << "int" << insn->Word(2);
            break;
        case spv::OpTypeFloat:
            ss << "float" << insn->Word(2);
            break;
        case spv::OpTypeVector:
            ss << "vec" << insn->Word(3) << " of ";
            DescribeTypeInner(ss, insn->Word(2), indent);
            break;
        case spv::OpTypeMatrix:
            ss << "mat" << insn->Word(3) << " of ";
            DescribeTypeInner(ss, insn->Word(2), indent);
            break;
        case spv::OpTypeArray:
            ss << "array[" << GetConstantValueById(insn->Word(3)) << "] of ";
            DescribeTypeInner(ss, insn->Word(2), indent);
            break;
        case spv::OpTypeRuntimeArray:
            ss << "runtime array[] of ";
            DescribeTypeInner(ss, insn->Word(2), indent);
            break;
        case spv::OpTypePointer:
            ss << "pointer to " << string_SpvStorageClass(insn->Word(2)) << " -> ";
            DescribeTypeInner(ss, insn->Word(3), indent);
            break;
        case spv::OpTypeStruct: {
            ss << "struct of {\n";
            indent++;
            for (uint32_t i = 2; i < insn->Length(); i++) {
                indent_by(indent);
                ss << "- ";
                DescribeTypeInner(ss, insn->Word(i), indent);

                auto name = GetMemberName(type, i - 2);
                if (!name.empty()) {
                    ss << " \"" << name << "\"";
                }

                ss << '\n';
            }
            indent--;
            indent_by(indent);
            ss << '}';

            auto name = GetName(type);
            if (!name.empty()) {
                ss << " \"" << name << "\"";
            }
            break;
        }
        case spv::OpTypeSampler:
            ss << "sampler";
            break;
        case spv::OpTypeSampledImage:
            ss << "sampler+";
            DescribeTypeInner(ss, insn->Word(2), indent);
            break;
        case spv::OpTypeImage:
            ss << "image(dim=" << insn->Word(3) << ", sampled=" << insn->Word(7) << ")";
            break;
        case spv::OpTypeAccelerationStructureNV:
            ss << "accelerationStruture";
            break;
        default:
            ss << "unknown type";
            break;
    }
}

std::string Module::DescribeType(uint32_t type) const {
    std::ostringstream ss;
    DescribeTypeInner(ss, type, 0);
    return ss.str();
}

std::string Module::DescribeVariable(uint32_t id) const {
    std::ostringstream ss;
    auto name = GetName(id);
    if (!name.empty()) {
        ss << "Variable \"" << name << "\"";
        auto decorations = GetDecorations(id);
        if (!decorations.empty()) {
            ss << " (Decorations:" << decorations << ')';
        }
        ss << '\n';
    }
    return ss.str();
}

std::string Module::DescribeInstruction(const Instruction& error_insn) const {
    if (static_data_.shader_debug_info_set_id == 0 && !static_data_.using_legacy_debug_info) {
        return error_insn.Describe();
    }

    const Instruction* last_line_inst = nullptr;
    for (const auto& insn : static_data_.instructions) {
        const uint32_t opcode = insn.Opcode();
        if (opcode == spv::OpExtInst && insn.Word(3) == static_data_.shader_debug_info_set_id &&
            insn.Word(4) == NonSemanticShaderDebugInfo100DebugLine) {
            last_line_inst = &insn;
        } else if (opcode == spv::OpLine) {
            last_line_inst = &insn;
        } else if (opcode == spv::OpFunctionEnd) {
            last_line_inst = nullptr;  // debug lines can't cross functions boundaries
        }

        if (insn == error_insn) {
            break;
        }
    }
    if (!last_line_inst) {
        return error_insn.Describe();  // can't find a suitable line above instruciton
    }

    std::ostringstream ss;
    ss << error_insn.Describe();
    ss << "\nFrom shader debug information ";
    GetShaderSourceInfo(ss, words_, *last_line_inst);
    return ss.str();
}

std::shared_ptr<const EntryPoint> Module::FindEntrypoint(char const* name, VkShaderStageFlagBits stageBits) const {
    if (!name) return nullptr;
    for (const auto& entry_point : static_data_.entry_points) {
        if (entry_point->name.compare(name) == 0 && entry_point->stage == stageBits) {
            return entry_point;
        }
    }
    return nullptr;
}

// Because the following is legal, need the entry point
//    OpEntryPoint GLCompute %main "name_a"
//    OpEntryPoint GLCompute %main "name_b"
// Assumes shader module contains no spec constants used to set the local size values
bool Module::FindLocalSize(const EntryPoint& entrypoint, uint32_t& local_size_x, uint32_t& local_size_y,
                           uint32_t& local_size_z) const {
    // "If an object is decorated with the WorkgroupSize decoration, this takes precedence over any LocalSize or LocalSizeId
    // execution mode."
    if (static_data_.has_builtin_workgroup_size) {
        const Instruction* composite_def = FindDef(static_data_.builtin_workgroup_size_id);
        if (composite_def->Opcode() == spv::OpConstantComposite) {
            // VUID-WorkgroupSize-WorkgroupSize-04427 makes sure this is a OpTypeVector of int32
            local_size_x = GetConstantValueById(composite_def->Word(3));
            local_size_y = GetConstantValueById(composite_def->Word(4));
            local_size_z = GetConstantValueById(composite_def->Word(5));
            return true;
        }
    }

    if (entrypoint.execution_mode.Has(ExecutionModeSet::local_size_bit)) {
        local_size_x = entrypoint.execution_mode.local_size_x;
        local_size_y = entrypoint.execution_mode.local_size_y;
        local_size_z = entrypoint.execution_mode.local_size_z;
        return true;
    } else if (entrypoint.execution_mode.Has(ExecutionModeSet::local_size_id_bit)) {
        // Uses ExecutionModeLocalSizeId so need to resolve ID value
        local_size_x = GetConstantValueById(entrypoint.execution_mode.local_size_x);
        local_size_y = GetConstantValueById(entrypoint.execution_mode.local_size_y);
        local_size_z = GetConstantValueById(entrypoint.execution_mode.local_size_z);
        return true;
    }

    return false;  // not found
}

uint32_t Module::CalculateWorkgroupSharedMemory() const {
    uint32_t total_size = 0;
    // when using WorkgroupMemoryExplicitLayoutKHR
    // either all or none the structs are decorated with Block,
    // if using block, all must decorated with Aliased.
    // In this case we want to find the MAX not ADD the block sizes
    bool find_max_block = false;

    for (const Instruction* insn : static_data_.variable_inst) {
        // StorageClass Workgroup is shared memory
        if (insn->StorageClass() == spv::StorageClassWorkgroup) {
            if (GetDecorationSet(insn->Word(2)).Has(DecorationSet::aliased_bit)) {
                find_max_block = true;
            }

            const uint32_t result_type_id = insn->Word(1);
            const Instruction* result_type = FindDef(result_type_id);
            const Instruction* type = FindDef(result_type->Word(3));

            // structs might have an offset padding
            const uint32_t variable_shared_size = (type->Opcode() == spv::OpTypeStruct)
                                                      ? GetTypeStructInfo(type->Word(1))->GetSize(*this).size
                                                      : GetTypeBytesSize(type);

            if (find_max_block) {
                total_size = std::max(total_size, variable_shared_size);
            } else {
                total_size += variable_shared_size;
            }
        }
    }
    return total_size;
}

// If the instruction at |id| is a OpConstant or copy of a constant, returns the instruction
// Cases such as runtime arrays, will not find a constant and return NULL
const Instruction* Module::GetConstantDef(uint32_t id) const {
    const Instruction* value = FindDef(id);

    // If id is a copy, see where it was copied from
    if (value && ((value->Opcode() == spv::OpCopyObject) || (value->Opcode() == spv::OpCopyLogical))) {
        id = value->Word(3);
        value = FindDef(id);
    }

    if (value && (value->Opcode() == spv::OpConstant)) {
        return value;
    }
    return nullptr;
}

// Returns the constant value described by the instruction at |id|
// Caller ensures there can't be a runtime array or specialization constants
uint32_t Module::GetConstantValueById(uint32_t id) const {
    const Instruction* value = GetConstantDef(id);

    // If this hit, most likley a runtime array (probably from VK_EXT_descriptor_indexing)
    // or unhandled specialization constants
    // Caller needs to call GetConstantDef() and check if null
    if (!value) {
        // TODO - still not fixed
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6293
        return 1;
    }

    return value->GetConstantValue();
}

// Returns the number of Location slots used for a given ID reference to a OpType*
uint32_t Module::GetLocationsConsumedByType(uint32_t type) const {
    const Instruction* insn = FindDef(type);

    switch (insn->Opcode()) {
        case spv::OpTypePointer:
            // See through the ptr -- this is only ever at the toplevel for graphics shaders we're never actually passing
            // pointers around.
            return GetLocationsConsumedByType(insn->Word(3));
        case spv::OpTypeArray: {
            // Spec: "If an array of size n and each element takes m locations,
            // it will be assigned m × n consecutive locations starting with the location specified"
            const uint32_t locations = GetLocationsConsumedByType(insn->Word(2));
            const uint32_t array_size = GetConstantValueById(insn->Word(3));
            return locations * array_size;
        }
        case spv::OpTypeMatrix: {
            // Spec: "if n × m matrix, the number of locations assigned for each matrix will be the same as for an n-element array
            // of m-component vectors"
            const uint32_t column_type = insn->Word(2);
            const uint32_t column_count = insn->Word(3);
            return column_count * GetLocationsConsumedByType(column_type);
        }
        case spv::OpTypeVector: {
            const Instruction* scalar_type = FindDef(insn->Word(2));
            const uint32_t width = scalar_type->GetByteWidth();
            const uint32_t vector_length = insn->Word(3);
            const uint32_t components = width * vector_length;
            // Locations are 128-bit wide (4 components)
            // 3- and 4-component vectors of 64 bit types require two.
            return (components / 5) + 1;
        }
        case spv::OpTypeStruct: {
            uint32_t sum = 0;
            // first 2 words of struct are not the elements to check
            for (uint32_t i = 2; i < insn->Length(); i++) {
                sum += GetLocationsConsumedByType(insn->Word(i));
            }
            return sum;
        }
        default:
            // Scalars (Int, Float, Bool, etc) are are just 1.
            return 1;
    }
}

// Returns the number of Components slots used for a given ID reference to a OpType*
uint32_t Module::GetComponentsConsumedByType(uint32_t type) const {
    const Instruction* insn = FindDef(type);

    switch (insn->Opcode()) {
        case spv::OpTypePointer:
            // See through the ptr -- this is only ever at the toplevel for graphics shaders we're never actually passing
            // pointers around.
            return GetComponentsConsumedByType(insn->Word(3));
        case spv::OpTypeArray:
            // Skip array as each array element is a whole new Location and we care only about the base type
            // ex. vec3[5] will only return 3
            return GetComponentsConsumedByType(insn->Word(2));
        case spv::OpTypeMatrix: {
            const uint32_t column_type = insn->Word(2);
            const uint32_t column_count = insn->Word(3);
            return column_count * GetComponentsConsumedByType(column_type);
        }
        case spv::OpTypeVector: {
            const Instruction* scalar_type = FindDef(insn->Word(2));
            const uint32_t width = scalar_type->GetByteWidth();
            const uint32_t vector_length = insn->Word(3);
            return width * vector_length;  // One component is 32-bit
        }
        case spv::OpTypeStruct: {
            uint32_t sum = 0;
            // first 2 words of struct are not the elements to check
            for (uint32_t i = 2; i < insn->Length(); i++) {
                sum += GetComponentsConsumedByType(insn->Word(i));
            }
            return sum;
        }
        default:
            // Int, Float, Bool, etc
            return insn->GetByteWidth();
    }
}

// characterizes a SPIR-V type appearing in an interface to a FF stage, for comparison to a VkFormat's characterization above.
// also used for input attachments, as we statically know their format.
NumericType Module::GetNumericType(uint32_t type) const {
    const Instruction* insn = FindDef(type);

    switch (insn->Opcode()) {
        case spv::OpTypeInt:
            return insn->Word(3) ? NumericTypeSint : NumericTypeUint;
        case spv::OpTypeFloat:
            return NumericTypeFloat;
        case spv::OpTypeVector:
        case spv::OpTypeMatrix:
        case spv::OpTypeArray:
        case spv::OpTypeRuntimeArray:
        case spv::OpTypeImage:
            return GetNumericType(insn->Word(2));
        case spv::OpTypePointer:
            return GetNumericType(insn->Word(3));
        default:
            return NumericTypeUnknown;
    }
}

bool Module::HasRuntimeArray(uint32_t type_id) const {
    const Instruction* type = FindDef(type_id);
    if (!type) {
        return false;
    }
    while (type->IsArray() || type->Opcode() == spv::OpTypePointer || type->Opcode() == spv::OpTypeSampledImage) {
        if (type->Opcode() == spv::OpTypeRuntimeArray) {
            return true;
        }
        const uint32_t next_word = (type->Opcode() == spv::OpTypePointer) ? 3 : 2;
        type = FindDef(type->Word(next_word));
    }
    return false;
}

std::string InterfaceSlot::Describe() const {
    std::stringstream msg;
    msg << "Location = " << Location() << " | Component = " << Component() << " | Type = " << string_SpvOpcode(type) << " "
        << bit_width << " bits";
    return msg.str();
}

uint32_t GetFormatType(VkFormat format) {
    if (vkuFormatIsSINT(format)) return NumericTypeSint;
    if (vkuFormatIsUINT(format)) return NumericTypeUint;
    // Formats such as VK_FORMAT_D16_UNORM_S8_UINT are both
    if (vkuFormatIsDepthAndStencil(format)) return NumericTypeFloat | NumericTypeUint;
    if (format == VK_FORMAT_UNDEFINED) return NumericTypeUnknown;
    // everything else -- UNORM/SNORM/FLOAT/USCALED/SSCALED is all float in the shader.
    return NumericTypeFloat;
}

char const* string_NumericType(uint32_t type) {
    if (type == NumericTypeSint) return "SINT";
    if (type == NumericTypeUint) return "UINT";
    if (type == NumericTypeFloat) return "FLOAT";
    return "(none)";
}

const char* VariableBase::FindDebugName(const VariableBase& variable, const DebugNameMap& debug_name_map) {
    const char* name = "";
    // We prefer to always get the variable name if it has it
    auto name_it = debug_name_map.find(variable.id);
    if (name_it != debug_name_map.end()) {
        name = name_it->second->GetAsString(2);
    }
    // if the shader looks like
    //     layout(binding=0) uniform StructName { vec4 x };
    // The variable name will be an empty string, for this, grab the struct name instead
    if (!name[0] && variable.type_struct_info) {
        name_it = debug_name_map.find(variable.type_struct_info->id);
        if (name_it != debug_name_map.end()) {
            name = name_it->second->GetAsString(2);
        }
    }
    return name;
}

VariableBase::VariableBase(const Module& module_state, const Instruction& insn, VkShaderStageFlagBits stage,
                           const VariableAccessMap& variable_access_map, const DebugNameMap& debug_name_map)
    : id(insn.Word(2)),
      type_id(insn.Word(1)),
      storage_class(static_cast<spv::StorageClass>(insn.Word(3))),
      decorations(module_state.GetDecorationSet(id)),
      type_struct_info(module_state.GetTypeStructInfo(&insn)),
      access_mask(AccessBit::empty),
      stage(stage),
      debug_name(FindDebugName(*this, debug_name_map)) {
    assert(insn.Opcode() == spv::OpVariable);

    // Finding the access of an image is more complex we will set that using the ImageAccessMap later
    // (Also there are no images for push constant or stage interface variables)
    auto access_it = variable_access_map.find(id);
    if (access_it != variable_access_map.end()) {
        access_mask = access_it->second;
    }
}

std::string VariableBase::DescribeDescriptor() const {
    std::ostringstream ss;
    ss << "[Set " << decorations.set << ", Binding " << decorations.binding;
    if (!debug_name.empty()) {
        ss << ", variable \"" << debug_name << "\"";
    }
    ss << "]";
    return ss.str();
}

bool StageInterfaceVariable::IsPerTaskNV(const StageInterfaceVariable& variable) {
    // will always be in a struct member
    if (variable.type_struct_info &&
        (variable.stage == VK_SHADER_STAGE_MESH_BIT_EXT || variable.stage == VK_SHADER_STAGE_TASK_BIT_EXT)) {
        return variable.type_struct_info->decorations.HasInMember(DecorationSet::per_task_nv);
    }
    return false;
}

// Some cases there is an array that is there to be "per-vertex" (or related)
// We want to remove this as it is not part of the Location caluclation or true type of variable
bool StageInterfaceVariable::IsArrayInterface(const StageInterfaceVariable& variable) {
    switch (variable.stage) {
        case VK_SHADER_STAGE_GEOMETRY_BIT:
            return variable.storage_class == spv::StorageClassInput;
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            return !variable.is_patch;
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            return !variable.is_patch && (variable.storage_class == spv::StorageClassInput);
        case VK_SHADER_STAGE_FRAGMENT_BIT:
            return variable.is_per_vertex && (variable.storage_class == spv::StorageClassInput);
        case VK_SHADER_STAGE_MESH_BIT_EXT:
            return !variable.is_per_task_nv && (variable.storage_class == spv::StorageClassOutput);
        default:
            break;
    }
    return false;
}

const Instruction& StageInterfaceVariable::FindBaseType(StageInterfaceVariable& variable, const Module& module_state) {
    // base type is always just grabbing the type of the OpTypePointer tied to the variables
    // This is allowed only here because interface variables are never Phyiscal pointers
    const Instruction* base_type = module_state.FindDef(module_state.FindDef(variable.type_id)->Word(3));

    // Strip away the first array, if any, if special interface array
    // Most times won't be anything to strip
    if (variable.is_array_interface && base_type->IsArray()) {
        const uint32_t type_id = base_type->Word(2);
        base_type = module_state.FindDef(type_id);
    }

    while (base_type->Opcode() == spv::OpTypeArray) {
        variable.array_size *= module_state.GetConstantValueById(base_type->Word(3));
        base_type = module_state.FindDef(base_type->Word(2));
    }

    return *base_type;
}

bool StageInterfaceVariable::IsBuiltin(const StageInterfaceVariable& variable, const Module& module_state) {
    const auto decoration_set = module_state.GetDecorationSet(variable.id);
    // If OpTypeStruct, will grab it's own decoration set
    return decoration_set.HasAnyBuiltIn() || (variable.type_struct_info && variable.type_struct_info->decorations.HasAnyBuiltIn());
}

// This logic is based off assumption that the Location are implicit and not member decorations
// when we have structs-of-structs, only the top struct can have explicit locations given
static uint32_t GetStructInterfaceSlots(const Module& module_state, std::shared_ptr<const TypeStructInfo> type_struct_info,
                                        std::vector<InterfaceSlot>& slots, uint32_t starting_location) {
    uint32_t locations_added = 0;
    for (uint32_t i = 0; i < type_struct_info->length; i++) {
        const auto& member = type_struct_info->members[i];

        // Keep walking down nested structs
        if (member.type_struct_info) {
            const uint32_t array_size = module_state.GetFlattenArraySize(*member.insn);
            for (uint32_t j = 0; j < array_size; j++) {
                locations_added +=
                    GetStructInterfaceSlots(module_state, member.type_struct_info, slots, starting_location + locations_added);
            }
            continue;
        }

        const uint32_t member_id = member.id;
        const uint32_t components = module_state.GetComponentsConsumedByType(member_id);
        const uint32_t locations = module_state.GetLocationsConsumedByType(member_id);

        // Info needed to test type matching later
        const Instruction* numerical_type = module_state.GetBaseTypeInstruction(member_id);
        const uint32_t numerical_type_opcode = numerical_type->Opcode();
        const uint32_t numerical_type_width = numerical_type->GetBitWidth();

        for (uint32_t j = 0; j < locations; j++) {
            for (uint32_t k = 0; k < components; k++) {
                slots.emplace_back(starting_location + locations_added, k, numerical_type_opcode, numerical_type_width);
            }
            locations_added++;
        }
    }
    return locations_added;
}

std::vector<InterfaceSlot> StageInterfaceVariable::GetInterfaceSlots(StageInterfaceVariable& variable, const Module& module_state) {
    std::vector<InterfaceSlot> slots;
    if (variable.is_builtin || variable.is_per_task_nv) {
        // SPV_NV_mesh_shader has a PerTaskNV which is not a builtin or interface
        return slots;
    }

    if (variable.base_type.StorageClass() == spv::StorageClassPhysicalStorageBuffer) {
        // PhysicalStorageBuffer interfaces not supported (https://gitlab.khronos.org/spirv/SPIR-V/-/issues/779)
        return slots;
    }

    if (variable.type_struct_info) {
        // Structs has two options being labeled
        // 1. The block is given a Location, need to walk though and add up starting for that value
        // 2. The block is NOT given a Location, each member has dedicated decoration
        const bool block_decorated_with_location = variable.decorations.location != kInvalidValue;
        if (block_decorated_with_location) {
            // In case of option 1, need to keep track as we go
            uint32_t base_location = variable.decorations.location;
            for (const auto& members : variable.type_struct_info->members) {
                const uint32_t member_id = members.id;
                const uint32_t components = module_state.GetComponentsConsumedByType(member_id);

                // Info needed to test type matching later
                const Instruction* numerical_type = module_state.GetBaseTypeInstruction(member_id);
                ASSERT_AND_CONTINUE(numerical_type);

                const uint32_t numerical_type_opcode = numerical_type->Opcode();
                // TODO - Handle nested structs
                if (numerical_type_opcode == spv::OpTypeStruct) {
                    variable.nested_struct = true;
                    break;
                }
                const uint32_t numerical_type_width = numerical_type->GetBitWidth();

                for (uint32_t j = 0; j < components; j++) {
                    slots.emplace_back(base_location, j, numerical_type_opcode, numerical_type_width);
                }
                base_location++;  // If using, each members starts a new Location
            }
        } else {
            // Option 2
            for (uint32_t i = 0; i < variable.type_struct_info->length; i++) {
                const auto& member = variable.type_struct_info->members[i];
                const uint32_t member_id = member.id;
                // Location/Components cant be decorated in nested structs, so no need to keep checking further
                // The spec says all or non of the member variables must have Location
                const auto member_decoration = variable.type_struct_info->decorations.member_decorations.at(i);
                uint32_t location = member_decoration.location;
                const uint32_t starting_component = member_decoration.component;

                if (member.type_struct_info) {
                    const uint32_t array_size = module_state.GetFlattenArraySize(*member.insn);
                    for (uint32_t j = 0; j < array_size; j++) {
                        location += GetStructInterfaceSlots(module_state, member.type_struct_info, slots, location);
                    }
                } else {
                    const uint32_t components = module_state.GetComponentsConsumedByType(member_id);

                    // Info needed to test type matching later
                    const Instruction* numerical_type = module_state.GetBaseTypeInstruction(member_id);
                    const uint32_t numerical_type_opcode = numerical_type->Opcode();
                    const uint32_t numerical_type_width = numerical_type->GetBitWidth();

                    for (uint32_t j = 0; j < components; j++) {
                        slots.emplace_back(location, starting_component + j, numerical_type_opcode, numerical_type_width);
                    }
                }
            }
        }
    } else {
        uint32_t locations = 0;
        // Will have array peeled off already
        const uint32_t type_id = variable.base_type.ResultId();

        locations = module_state.GetLocationsConsumedByType(type_id);
        const uint32_t components = module_state.GetComponentsConsumedByType(type_id);

        // Info needed to test type matching later
        const Instruction* numerical_type = module_state.GetBaseTypeInstruction(type_id);
        const uint32_t numerical_type_opcode = numerical_type->Opcode();
        const uint32_t numerical_type_width = numerical_type->GetBitWidth();

        const uint32_t starting_location = variable.decorations.location;
        const uint32_t starting_component = variable.decorations.component;
        for (uint32_t array_index = 0; array_index < variable.array_size; array_index++) {
            // offet into array if there is one
            const uint32_t location = starting_location + (locations * array_index);
            for (uint32_t component = 0; component < components; component++) {
                slots.emplace_back(location, component + starting_component, numerical_type_opcode, numerical_type_width);
            }
        }
    }
    return slots;
}

std::vector<uint32_t> StageInterfaceVariable::GetBuiltinBlock(const StageInterfaceVariable& variable, const Module& module_state) {
    // Built-in Location slot will always be [zero, size]
    std::vector<uint32_t> slots;
    // Only check block built-ins - many builtin are non-block and not used between shaders
    if (!variable.is_builtin || !variable.type_struct_info) {
        return slots;
    }

    const auto& decoration_set = variable.type_struct_info->decorations;
    if (decoration_set.Has(DecorationSet::block_bit)) {
        for (uint32_t i = 0; i < variable.type_struct_info->length; i++) {
            slots.push_back(decoration_set.member_decorations.at(i).builtin);
        }
    }
    return slots;
}

uint32_t StageInterfaceVariable::GetBuiltinComponents(const StageInterfaceVariable& variable, const Module& module_state) {
    uint32_t count = 0;
    if (!variable.is_builtin) {
        return count;
    }
    if (variable.type_struct_info) {
        for (const auto& members : variable.type_struct_info->members) {
            count += module_state.GetComponentsConsumedByType(members.id);
        }
    } else {
        const uint32_t base_type_id = variable.base_type.ResultId();
        count += module_state.GetComponentsConsumedByType(base_type_id);
    }
    return count;
}

StageInterfaceVariable::StageInterfaceVariable(const Module& module_state, const Instruction& insn, VkShaderStageFlagBits stage,
                                               const VariableAccessMap& variable_access_map, const DebugNameMap& debug_name_map)
    : VariableBase(module_state, insn, stage, variable_access_map, debug_name_map),
      is_patch(decorations.Has(DecorationSet::patch_bit)),
      is_per_vertex(decorations.Has(DecorationSet::per_vertex_bit)),
      is_per_task_nv(IsPerTaskNV(*this)),
      is_array_interface(IsArrayInterface(*this)),
      base_type(FindBaseType(*this, module_state)),
      is_builtin(IsBuiltin(*this, module_state)),
      nested_struct(false),
      interface_slots(GetInterfaceSlots(*this, module_state)),
      builtin_block(GetBuiltinBlock(*this, module_state)),
      total_builtin_components(GetBuiltinComponents(*this, module_state)) {}

const Instruction& ResourceInterfaceVariable::FindBaseType(ResourceInterfaceVariable& variable, const Module& module_state) {
    // Takes a OpVariable and looks at the the descriptor type it uses. This will find things such as if the variable is writable,
    // image atomic operation, matching images to samplers, etc
    const Instruction* type = module_state.FindDef(variable.type_id);

    // Strip off any array or ptrs. Where we remove array levels, adjust the  descriptor count for each dimension.
    while (type->IsArray() || type->Opcode() == spv::OpTypePointer || type->Opcode() == spv::OpTypeSampledImage) {
        if (type->IsArray() || type->Opcode() == spv::OpTypeSampledImage) {
            // currently just tracks 1D arrays
            if (type->Opcode() == spv::OpTypeArray && variable.array_length == 0) {
                variable.array_length = module_state.GetConstantValueById(type->Word(3));
            } else if (type->Opcode() == spv::OpTypeRuntimeArray) {
                variable.array_length = spirv::kRuntimeArray;
            }

            if (type->Opcode() == spv::OpTypeSampledImage) {
                variable.is_sampled_image = true;
            }

            type = module_state.FindDef(type->Word(2));  // Element type
        } else {
            type = module_state.FindDef(type->Word(3));  // Pointer type
        }
    }
    return *type;
}

uint32_t ResourceInterfaceVariable::FindImageSampledTypeWidth(const Module& module_state, const Instruction& base_type) {
    return (base_type.Opcode() == spv::OpTypeImage) ? module_state.GetTypeBitsSize(&base_type) : 0;
}

NumericType ResourceInterfaceVariable::FindImageFormatType(const Module& module_state, const Instruction& base_type) {
    return (base_type.Opcode() == spv::OpTypeImage) ? module_state.GetNumericType(base_type.Word(2)) : NumericTypeUnknown;
}

bool ResourceInterfaceVariable::IsStorageBuffer(const ResourceInterfaceVariable& variable) {
    // before VK_KHR_storage_buffer_storage_class Storage Buffer were a Uniform storage class
    const bool physical_storage_buffer = variable.storage_class == spv::StorageClassPhysicalStorageBuffer;
    const bool storage_buffer = variable.storage_class == spv::StorageClassStorageBuffer;
    const bool uniform = variable.storage_class == spv::StorageClassUniform;
    // Block decorations are always on the struct of the variable
    const bool buffer_block =
        variable.type_struct_info && variable.type_struct_info->decorations.Has(DecorationSet::buffer_block_bit);
    const bool block = variable.type_struct_info && variable.type_struct_info->decorations.Has(DecorationSet::block_bit);
    return ((uniform && buffer_block) || ((storage_buffer || physical_storage_buffer) && block));
}

ResourceInterfaceVariable::ResourceInterfaceVariable(const Module& module_state, const EntryPoint& entrypoint,
                                                     const Instruction& insn, const ImageAccessMap& image_access_map,
                                                     const AccessChainVariableMap& access_chain_map,
                                                     const VariableAccessMap& variable_access_map,
                                                     const DebugNameMap& debug_name_map)
    : VariableBase(module_state, insn, entrypoint.stage, variable_access_map, debug_name_map),
      array_length(0),
      is_sampled_image(false),
      base_type(FindBaseType(*this, module_state)),
      is_runtime_descriptor_array(module_state.HasRuntimeArray(type_id)),
      image_sampled_type_width(FindImageSampledTypeWidth(module_state, base_type)),
      is_storage_buffer(IsStorageBuffer(*this)) {
    // to make sure no padding in-between the struct produce noise and force same data to become a different hash
    info = {};  // will be cleared with c++11 initialization
    info.image_format_type = FindImageFormatType(module_state, base_type);
    info.image_dim = base_type.FindImageDim();
    info.is_image_array = base_type.IsImageArray();
    info.is_multisampled = base_type.IsImageMultisampled();

    // Handle anything specific to the base type
    if (base_type.Opcode() == spv::OpTypeImage) {
        // Things marked regardless of the image being accessed or not
        const bool is_sampled_without_sampler = base_type.Word(7) == 2;  // Word(7) == Sampled
        if (is_sampled_without_sampler) {
            if (info.image_dim == spv::DimSubpassData) {
                is_input_attachment = true;
                if (array_length != spirv::kRuntimeArray) {
                    input_attachment_index_read.resize(array_length);
                }
            } else if (info.image_dim == spv::DimBuffer) {
                is_storage_texel_buffer = true;
            } else {
                is_storage_image = true;
            }
        }

        const auto image_access_it = image_access_map.find(id);
        if (image_access_it != image_access_map.end()) {
            for (const auto& image_access_ptr : image_access_it->second) {
                const auto& image_access = *image_access_ptr;

                info.is_dref |= image_access.is_dref;
                info.is_sampler_implicitLod_dref_proj |= image_access.is_sampler_implicitLod_dref_proj;
                info.is_sampler_sampled |= image_access.is_sampler_sampled;
                info.is_not_sampler_sampled |= image_access.is_not_sampler_sampled;
                info.is_sampler_bias_offset |= image_access.is_sampler_bias_offset;
                info.is_sampler_offset |= image_access.is_sampler_offset;
                info.is_sign_extended |= image_access.is_sign_extended;
                info.is_zero_extended |= image_access.is_zero_extended;
                access_mask |= image_access.access_mask;

                const bool is_image_without_format =
                    ((is_sampled_without_sampler) && (base_type.Word(8) == spv::ImageFormatUnknown));
                if (image_access.access_mask & AccessBit::image_write) {
                    if (is_image_without_format) {
                        info.is_write_without_format |= true;
                        if (image_access.texel_component_count != kInvalidValue) {
                            write_without_formats_component_count_list.push_back(image_access.texel_component_count);
                        }
                    }
                }

                if (image_access.access_mask & AccessBit::image_read) {
                    if (is_image_without_format) {
                        info.is_read_without_format |= true;
                    }

                    // If accessed in an array, track which indexes were read, if not runtime array
                    if (is_input_attachment && !module_state.HasRuntimeArray(type_id)) {
                        if (image_access.image_access_chain_index != kInvalidValue) {
                            input_attachment_index_read[image_access.image_access_chain_index] = true;
                        } else {
                            // if InputAttachment is accessed from load, just a single, non-array, index
                            input_attachment_index_read.resize(1);
                            input_attachment_index_read[0] = true;
                        }
                    }
                }

                // if not CombinedImageSampler, need to find all Samplers that were accessed with the image
                if (!image_access.variable_sampler_insn.empty() && !is_sampled_image) {
                    // if no AccessChain, it is same conceptually as being zero
                    const uint32_t image_index =
                        image_access.image_access_chain_index != kInvalidValue ? image_access.image_access_chain_index : 0;
                    const uint32_t sampler_index =
                        image_access.sampler_access_chain_index != kInvalidValue ? image_access.sampler_access_chain_index : 0;

                    if (image_index >= samplers_used_by_image.size()) {
                        samplers_used_by_image.resize(image_index + 1);
                    }

                    for (const Instruction* sampler_insn : image_access.variable_sampler_insn) {
                        const auto& decoration_set = module_state.GetDecorationSet(sampler_insn->ResultId());
                        samplers_used_by_image[image_index].emplace(
                            SamplerUsedByImage{DescriptorSlot{decoration_set.set, decoration_set.binding}, sampler_index});
                    }
                }
            }
        }
    }

    info.access_mask = access_mask;
    descriptor_hash = hash_util::DescriptorVariableHash(&info, sizeof(info));
}

PushConstantVariable::PushConstantVariable(const Module& module_state, const Instruction& insn, VkShaderStageFlagBits stage,
                                           const VariableAccessMap& variable_access_map, const DebugNameMap& debug_name_map)
    : VariableBase(module_state, insn, stage, variable_access_map, debug_name_map), offset(vvl::kU32Max), size(0) {
    assert(type_struct_info != nullptr);  // Push Constants need to be structs

    auto struct_size = type_struct_info->GetSize(module_state);
    offset = struct_size.offset;
    size = struct_size.size;
}

TypeStructInfo::TypeStructInfo(const Module& module_state, const Instruction& struct_insn)
    : id(struct_insn.Word(1)), length(struct_insn.Length() - 2), decorations(module_state.GetDecorationSet(id)) {
    members.resize(length);
    for (uint32_t i = 0; i < length; i++) {
        Member& member = members[i];
        member.id = struct_insn.Word(2 + i);
        member.insn = module_state.FindDef(member.id);
        member.type_struct_info = module_state.GetTypeStructInfo(member.insn);

        const auto it = decorations.member_decorations.find(i);
        if (it != decorations.member_decorations.end()) {
            member.decorations = &it->second;
        }
    }
}

TypeStructSize TypeStructInfo::GetSize(const Module& module_state) const {
    uint32_t offset = vvl::kU32Max;
    uint32_t size = 0;

    // Non-Blocks don't have offset so can get packed size
    if (!decorations.Has(DecorationSet::block_bit)) {
        offset = 0;
        size = module_state.GetTypeBytesSize(module_state.FindDef(id));
        return {offset, size};
    }

    // Currently to know the range we only need to know
    // - The lowest offset element is in root struct
    // - how large the highest offset element is in root struct
    //
    // Note structs don't have to be ordered, the following is legal
    //    OpMemberDecorate %x 1 Offset 0
    //    OpMemberDecorate %x 0 Offset 4
    //
    // Info at https://gitlab.khronos.org/spirv/SPIR-V/-/issues/763
    uint32_t highest_element_index = 0;
    uint32_t highest_element_offset = 0;

    for (uint32_t i = 0; i < members.size(); i++) {
        const auto& member = members[i];
        // all struct elements are required to have offset decorations in Block
        const uint32_t member_offset = member.decorations->offset;
        offset = std::min(offset, member_offset);
        if (member_offset > highest_element_offset) {
            highest_element_index = i;
            highest_element_offset = member_offset;
        }
    }

    const auto& highest_member = members[highest_element_index];
    uint32_t highest_element_size = 0;
    if (highest_member.insn->Opcode() == spv::OpTypeArray &&
        module_state.FindDef(highest_member.insn->Word(3))->Opcode() == spv::OpSpecConstant) {
        // TODO - This is a work-around because currently we only apply SpecConstant for workgroup size
        // The shader validation needs to be fixed so we handle all cases when spec constant are applied, while still being catious
        // of the fact that information is not known until pipeline creation (not at shader module creation time)
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5911
        highest_element_size = module_state.FindDef(highest_member.insn->Word(3))->Word(3);
    } else {
        highest_element_size = module_state.GetTypeBytesSize(highest_member.insn);
    }
    size = (highest_element_size + highest_element_offset) - offset;

    return {offset, size};
}

uint32_t Module::GetNumComponentsInBaseType(const Instruction* insn) const {
    const uint32_t opcode = insn->Opcode();
    uint32_t component_count = 0;
    if (opcode == spv::OpTypeFloat || opcode == spv::OpTypeInt) {
        component_count = 1;
    } else if (opcode == spv::OpTypeVector) {
        component_count = insn->Word(3);
    } else if (opcode == spv::OpTypeMatrix) {
        const Instruction* column_type = FindDef(insn->Word(2));
        // Because we are calculating components for a single location we do not care about column count
        component_count = GetNumComponentsInBaseType(column_type);  // vector length
    } else if (opcode == spv::OpTypeArray) {
        const Instruction* element_type = FindDef(insn->Word(2));
        component_count = GetNumComponentsInBaseType(element_type);  // element length
    } else if (opcode == spv::OpTypeStruct) {
        for (uint32_t i = 2; i < insn->Length(); ++i) {
            component_count += GetNumComponentsInBaseType(FindDef(insn->Word(i)));
        }
    } else if (opcode == spv::OpTypePointer) {
        const Instruction* type = FindDef(insn->Word(3));
        component_count = GetNumComponentsInBaseType(type);
    }
    return component_count;
}

// Returns the total size in 'bits' of any OpType*
uint32_t Module::GetTypeBitsSize(const Instruction* insn) const {
    const uint32_t opcode = insn->Opcode();
    uint32_t bit_size = 0;
    if (opcode == spv::OpTypeVector) {
        const Instruction* component_type = FindDef(insn->Word(2));
        uint32_t scalar_width = GetTypeBitsSize(component_type);
        uint32_t component_count = insn->Word(3);
        bit_size = scalar_width * component_count;
    } else if (opcode == spv::OpTypeMatrix) {
        const Instruction* column_type = FindDef(insn->Word(2));
        uint32_t vector_width = GetTypeBitsSize(column_type);
        uint32_t column_count = insn->Word(3);
        bit_size = vector_width * column_count;
    } else if (opcode == spv::OpTypeArray) {
        const Instruction* element_type = FindDef(insn->Word(2));
        uint32_t element_width = GetTypeBitsSize(element_type);
        const Instruction* length_type = FindDef(insn->Word(3));
        uint32_t length = length_type->GetConstantValue();
        bit_size = element_width * length;
    } else if (opcode == spv::OpTypeStruct) {
        // Will not consider any possible Offset, gets size of a packed struct
        for (uint32_t i = 2; i < insn->Length(); ++i) {
            bit_size += GetTypeBitsSize(FindDef(insn->Word(i)));
        }
    } else if (opcode == spv::OpTypePointer) {
        if (insn->StorageClass() == spv::StorageClassPhysicalStorageBuffer) {
            // All PhysicalStorageBuffer are just 64-bit pointers
            // We don't need to go chasing it to find the size, as it is not calculated for any VUs
            bit_size = 8;
        } else {
            const Instruction* type = FindDef(insn->Word(3));
            bit_size = GetTypeBitsSize(type);
        }
    } else if (opcode == spv::OpVariable) {
        const Instruction* type = FindDef(insn->Word(1));
        bit_size = GetTypeBitsSize(type);
    } else if (opcode == spv::OpTypeImage) {
        const Instruction* type = FindDef(insn->Word(2));
        bit_size = GetTypeBitsSize(type);
    } else if (opcode == spv::OpTypeVoid) {
        // Sampled Type of OpTypeImage can be a void
        bit_size = 0;
    } else {
        bit_size = insn->GetBitWidth();
    }

    return bit_size;
}

// Returns the total size in 'bytes' of any OpType*
uint32_t Module::GetTypeBytesSize(const Instruction* insn) const { return GetTypeBitsSize(insn) / 8; }

// Returns the base type (float, int or unsigned int) or struct (can have multiple different base types inside)
// Will return 0 if it can not be determined
uint32_t Module::GetBaseType(const Instruction* insn) const {
    const uint32_t opcode = insn->Opcode();
    if (opcode == spv::OpTypeFloat || opcode == spv::OpTypeInt || opcode == spv::OpTypeBool || opcode == spv::OpTypeStruct) {
        // point to itself as its the base type (or a struct that needs to be traversed still)
        return insn->Word(1);
    } else if (opcode == spv::OpTypeVector) {
        const Instruction* component_type = FindDef(insn->Word(2));
        return GetBaseType(component_type);
    } else if (opcode == spv::OpTypeMatrix) {
        const Instruction* column_type = FindDef(insn->Word(2));
        return GetBaseType(column_type);
    } else if (opcode == spv::OpTypeArray || opcode == spv::OpTypeRuntimeArray) {
        const Instruction* element_type = FindDef(insn->Word(2));
        return GetBaseType(element_type);
    } else if (opcode == spv::OpTypePointer) {
        const auto& storage_class = insn->StorageClass();
        const Instruction* type = FindDef(insn->Word(3));
        if (storage_class == spv::StorageClassPhysicalStorageBuffer && type->Opcode() == spv::OpTypeStruct) {
            // A physical storage buffer to a struct has a chance to point to itself and can't resolve a baseType
            // GLSL example:
            // layout(buffer_reference) buffer T1 {
            //     T1 b[2];
            // };
            return 0;
        }
        return GetBaseType(type);
    }
    // If we assert here, we are missing a valid base type that must be handled. Without this assert, a return value of 0 will
    // produce a hard bug to track
    assert(false);
    return 0;
}

const Instruction* Module::GetBaseTypeInstruction(uint32_t type) const {
    const Instruction* insn = FindDef(type);
    const uint32_t base_insn_id = GetBaseType(insn);
    // Will return end() if an invalid/unknown base_insn_id is returned
    return FindDef(base_insn_id);
}

// Returns type_id if id has type or zero otherwise
uint32_t Module::GetTypeId(uint32_t id) const {
    const Instruction* type = FindDef(id);
    return type ? type->TypeId() : 0;
}

// Return zero if nothing is found
uint32_t Module::GetTexelComponentCount(const Instruction& insn) const {
    uint32_t texel_component_count = 0;
    switch (insn.Opcode()) {
        case spv::OpImageWrite: {
            const Instruction* texel_def = FindDef(insn.Word(3));
            const Instruction* texel_type = FindDef(texel_def->Word(1));
            texel_component_count = (texel_type->Opcode() == spv::OpTypeVector) ? texel_type->Word(3) : 1;
            break;
        }
        default:
            break;
    }
    return texel_component_count;
}

// Takes an array like [3][2][4] and returns 24
// If not an array, returns 1
uint32_t Module::GetFlattenArraySize(const Instruction& insn) const {
    uint32_t array_size = 1;
    if (insn.Opcode() == spv::OpTypeArray) {
        array_size = GetConstantValueById(insn.Word(3));
        const Instruction* element_insn = FindDef(insn.Word(2));
        if (element_insn->Opcode() == spv::OpTypeArray) {
            array_size *= GetFlattenArraySize(*element_insn);
        }
    }
    return array_size;
}

AtomicInstructionInfo Module::GetAtomicInfo(const Instruction& insn) const {
    AtomicInstructionInfo info;

    // All atomics have a pointer referenced
    const uint32_t pointer_index = insn.Opcode() == spv::OpAtomicStore ? 1 : 3;
    const Instruction* access = FindDef(insn.Word(pointer_index));

    // spirv-val will catch if not OpTypePointer
    const Instruction* pointer = FindDef(access->Word(1));
    info.storage_class = pointer->Word(2);

    const Instruction* data_type = FindDef(pointer->Word(3));

    if (data_type->Opcode() == spv::OpTypeVector) {
        info.vector_size = data_type->Word(3);
        data_type = FindDef(data_type->Word(2));
    }

    info.type = data_type->Opcode();

    info.bit_width = data_type->GetBitWidth();

    return info;
}

}  // namespace spirv
