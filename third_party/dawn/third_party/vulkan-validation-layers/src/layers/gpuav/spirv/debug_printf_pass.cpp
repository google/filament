/* Copyright (c) 2024-2025 LunarG, Inc.
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

#include "debug_printf_pass.h"
#include "module.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include <spirv/unified1/NonSemanticDebugPrintf.h>
#include <cstring>
#include <iostream>

#include "generated/device_features.h"

namespace gpuav {
namespace spirv {

// All functions are a list of uint32_t
// The difference is just how many are passed in
uint32_t DebugPrintfPass::GetLinkFunctionId(uint32_t argument_count) {
    if (auto it = function_id_map_.find(argument_count); it != function_id_map_.end()) {
        return it->second;
    }

    const uint32_t link_function_id = module_.TakeNextId();
    function_id_map_[argument_count] = link_function_id;

    return link_function_id;
}

bool DebugPrintfPass::RequiresInstrumentation(const Instruction& inst) {
    if (inst.Opcode() == spv::OpExtInst && inst.Word(3) == ext_import_id_ && inst.Word(4) == NonSemanticDebugPrintfDebugPrintf) {
        target_instruction_ = &inst;
        return true;
    }
    return false;
}

// Takes the various arguments and casts them to a valid uint32_t to be passed as a parameter in the function
void DebugPrintfPass::CreateFunctionParams(uint32_t argument_id, const Type& argument_type, std::vector<uint32_t>& params,
                                           BasicBlock& block, InstructionIt* inst_it) {
    const uint32_t uint32_type_id = module_.type_manager_.GetTypeInt(32, false).Id();

    switch (argument_type.spv_type_) {
        case SpvType::kVector: {
            const uint32_t component_count = argument_type.inst_.Word(3);
            const uint32_t component_type_id = argument_type.inst_.Word(2);
            const Type* component_type = module_.type_manager_.FindTypeById(component_type_id);
            assert(component_type);
            for (uint32_t i = 0; i < component_count; i++) {
                const uint32_t extract_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpCompositeExtract, {component_type_id, extract_id, argument_id, i}, inst_it);
                CreateFunctionParams(extract_id, *component_type, params, block, inst_it);
            }
            break;
        }

        case SpvType::kInt: {
            const uint32_t width = argument_type.inst_.Word(2);

            // first thing is to get any signed to unsigned via bitcast
            const bool is_signed = argument_type.inst_.Word(3) != 0;
            uint32_t incoming_id = argument_id;
            if (is_signed) {
                const uint32_t bitcast_id = module_.TakeNextId();
                const uint32_t unsigned_type_id = module_.type_manager_.GetTypeInt(width, false).Id();
                block.CreateInstruction(spv::OpBitcast, {unsigned_type_id, bitcast_id, argument_id}, inst_it);
                incoming_id = bitcast_id;

                if (width == 8) {
                    if (expanded_parameter_count_ > 31) {
                        module_.InternalWarning("DEBUG-PRINTF-SIGNED-8-MASK",
                                                "More than 32 expanded parameters, can't properly detect 8-bit signed ints [Simple "
                                                "fix is to turn long printf() into 2 shorter printf() calls]");
                    } else {
                        signed_8_bitmask_ |= 1 << expanded_parameter_count_;
                    }
                } else if (width == 16) {
                    if (expanded_parameter_count_ > 31) {
                        module_.InternalWarning("DEBUG-PRINTF-SIGNED-16-MASK",
                                                "More than 32 expanded parameters, can't properly detect 16-bit signed ints "
                                                "[Simple fix is to turn long printf() into 2 shorter printf() calls]");
                    } else {
                        signed_16_bitmask_ |= 1 << expanded_parameter_count_;
                    }
                }
            }

            if (width == 8 || width == 16) {
                const uint32_t uconvert_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpUConvert, {uint32_type_id, uconvert_id, incoming_id}, inst_it);
                params.push_back(uconvert_id);
                expanded_parameter_count_++;
            } else if (width == 32) {
                params.push_back(incoming_id);
                expanded_parameter_count_++;
            } else if (width == 64) {
                const uint32_t uconvert_high_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpUConvert, {uint32_type_id, uconvert_high_id, incoming_id}, inst_it);
                params.push_back(uconvert_high_id);
                expanded_parameter_count_++;

                const uint32_t uint64_type_id = module_.type_manager_.GetTypeInt(64, false).Id();
                const uint32_t shift_right_id = module_.TakeNextId();
                const uint32_t constant_32_id = module_.type_manager_.GetConstantUInt32(32).Id();
                block.CreateInstruction(spv::OpShiftRightLogical, {uint64_type_id, shift_right_id, incoming_id, constant_32_id},
                                        inst_it);

                const uint32_t uconvert_low_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpUConvert, {uint32_type_id, uconvert_low_id, shift_right_id}, inst_it);
                params.push_back(uconvert_low_id);
            } else {
                module_.InternalError(Name(), "CreateFunctionParams has unsupported for int width");
            }
            break;
        }

        case SpvType::kFloat: {
            const uint32_t width = argument_type.inst_.Word(2);
            if (width == 16) {
                const uint32_t float32_type_id = module_.type_manager_.GetTypeFloat(32).Id();
                const uint32_t fconvert_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpFConvert, {float32_type_id, fconvert_id, argument_id}, inst_it);

                const uint32_t bitcast_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpBitcast, {uint32_type_id, bitcast_id, fconvert_id}, inst_it);
                params.push_back(bitcast_id);
                expanded_parameter_count_++;
            } else if (width == 32) {
                const uint32_t bitcast_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpBitcast, {uint32_type_id, bitcast_id, argument_id}, inst_it);
                params.push_back(bitcast_id);
                expanded_parameter_count_++;
            } else if (width == 64) {
                if (expanded_parameter_count_ > 31) {
                    // It is very unlikely to hit this
                    module_.InternalWarning("DEBUG-PRINTF-DOUBLE-MASK",
                                            "More than 32 expanded parameters, can't properly detect 64-bit float [Simple fix is "
                                            "to turn long printf() into 2 shorter printf() calls]");
                } else {
                    double_bitmask_ |= 1 << expanded_parameter_count_;
                }

                if (!module_.enabled_features_.shaderInt64) {
                    module_.InternalError(
                        "DEBUG-PRINTF-INT64-SUPPORT",
                        "shaderInt64 feature is not supported, but is required to cast a 64-bit float to a 64-bit int "
                        "when writing to the output buffer");
                }
                module_.AddCapability(spv::CapabilityInt64);

                const uint32_t uint64_type_id = module_.type_manager_.GetTypeInt(64, false).Id();
                const uint32_t bitcast_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpBitcast, {uint64_type_id, bitcast_id, argument_id}, inst_it);

                const uint32_t uconvert_high_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpUConvert, {uint32_type_id, uconvert_high_id, bitcast_id}, inst_it);
                params.push_back(uconvert_high_id);
                expanded_parameter_count_++;

                const uint32_t shift_right_id = module_.TakeNextId();
                const uint32_t constant_32_id = module_.type_manager_.GetConstantUInt32(32).Id();
                block.CreateInstruction(spv::OpShiftRightLogical, {uint64_type_id, shift_right_id, bitcast_id, constant_32_id},
                                        inst_it);

                const uint32_t uconvert_low_id = module_.TakeNextId();
                block.CreateInstruction(spv::OpUConvert, {uint32_type_id, uconvert_low_id, shift_right_id}, inst_it);
                params.push_back(uconvert_low_id);
            } else {
                module_.InternalError(Name(), "CreateFunctionParams has unsupported for float width");
            }
            break;
        }

        case SpvType::kBool: {
            // cast to uint32_t via an OpSelect
            const uint32_t zero_id = module_.type_manager_.GetConstantZeroUint32().Id();
            const uint32_t one_id = module_.type_manager_.GetConstantUInt32(1).Id();
            const uint32_t select_id = module_.TakeNextId();
            block.CreateInstruction(spv::OpSelect, {uint32_type_id, select_id, argument_id, one_id, zero_id}, inst_it);
            params.push_back(select_id);
            expanded_parameter_count_++;
            break;
        }

        default:
            module_.InternalError(Name(), "CreateFunctionParams has unsupported function param type");
            break;
    }
}

void DebugPrintfPass::CreateFunctionCall(BasicBlockIt block_it, InstructionIt* inst_it) {
    BasicBlock& block = **block_it;
    Function& block_func = block.function_;
    // need to call to get the underlying 4 IDs (simpler to pass in as 4 uint then a uvec4)
    GetStageInfo(block_func, block_it, *inst_it);

    const uint32_t inst_position = target_instruction_->position_index_;
    auto inst_position_constant = module_.type_manager_.CreateConstantUInt32(inst_position);

    const uint32_t string_id = target_instruction_->Word(5);
    auto string_id_constant = module_.type_manager_.CreateConstantUInt32(string_id);

    const uint32_t void_type = module_.type_manager_.GetTypeVoid().Id();
    const uint32_t function_result = module_.TakeNextId();

    // We know the first part, then build up the rest from the printf arguments
    // except a few slots, we place hold it with zero until we build up the params
    const size_t function_def_slot = 2;
    const size_t double_bitmask_slot = 5;
    const size_t signed_8_bitmask_slot = 6;
    const size_t signed_16_bitmask_slot = 7;
    std::vector<uint32_t> function_call_params = {void_type,
                                                  function_result,
                                                  0,  // function_def_slot
                                                  inst_position_constant.Id(),
                                                  string_id_constant.Id(),
                                                  0,  // double_bitmask_slot,
                                                  0,  // signed_8_bitmask_slot,
                                                  0,  // signed_16_bitmask_slot,
                                                  block_func.stage_info_x_id_,
                                                  block_func.stage_info_y_id_,
                                                  block_func.stage_info_z_id_,
                                                  block_func.stage_info_w_id_};

    // where we find the first arugment in OpExtInst instruction
    const uint32_t first_argument_offset = 6;
    const uint32_t argument_count = target_instruction_->Length() - first_argument_offset;
    for (uint32_t i = 0; i < argument_count; i++) {
        const uint32_t argument_id = target_instruction_->Word(first_argument_offset + i);
        const Instruction* argument_inst = nullptr;
        const Constant* constant = module_.type_manager_.FindConstantById(argument_id);
        if (constant) {
            argument_inst = &constant->inst_;
        } else {
            argument_inst = block.function_.FindInstruction(argument_id);
        }
        assert(argument_inst);  // argument is either constant or found within function block

        const Type* argument_type = module_.type_manager_.FindTypeById(argument_inst->TypeId());
        assert(argument_type);  // type needs to have been declared already

        CreateFunctionParams(argument_inst->ResultId(), *argument_type, function_call_params, block, inst_it);
    }

    // 3 params are the [result, function type, and function ID]
    const uint32_t ignored_params = 3;
    const uint32_t param_count = (uint32_t)function_call_params.size() - ignored_params;
    const uint32_t function_def = GetLinkFunctionId(param_count);

    // patch in params
    function_call_params[function_def_slot] = function_def;
    function_call_params[double_bitmask_slot] = module_.type_manager_.GetConstantUInt32(double_bitmask_).Id();
    function_call_params[signed_8_bitmask_slot] = module_.type_manager_.GetConstantUInt32(signed_8_bitmask_).Id();
    function_call_params[signed_16_bitmask_slot] = module_.type_manager_.GetConstantUInt32(signed_16_bitmask_).Id();

    block.CreateInstruction(spv::OpFunctionCall, function_call_params, inst_it);
}

void DebugPrintfPass::CreateDescriptorSet() {
    // Create descriptor set to match output buffer
    // The following is what the GLSL would look like
    //
    // layout(set = kSet, binding = kBinding, std430) buffer SSBO {
    //     uint written_count;
    //     uint data[];
    // } output_buffer;

    const Type& uint32_type = module_.type_manager_.GetTypeInt(32, false);
    const uint32_t runtime_array_type_id = module_.type_manager_.GetTypeRuntimeArray(uint32_type).Id();

    // if 2 OpTypeRuntimeArray are combined, we can't have ArrayStride twice
    bool has_array_stride = false;
    for (auto& inst : module_.annotations_) {
        if (inst->Opcode() == spv::OpDecorate && inst->Word(1) == runtime_array_type_id &&
            inst->Word(2) == spv::DecorationArrayStride) {
            has_array_stride = true;
            break;
        }
    }
    if (!has_array_stride) {
        module_.AddDecoration(runtime_array_type_id, spv::DecorationArrayStride, {4});
    }

    const uint32_t struct_type_id = module_.TakeNextId();
    auto new_struct_inst = std::make_unique<Instruction>(4, spv::OpTypeStruct);
    new_struct_inst->Fill({struct_type_id, uint32_type.Id(), runtime_array_type_id});
    const Type& struct_type = module_.type_manager_.AddType(std::move(new_struct_inst), SpvType::kStruct);
    module_.AddDecoration(struct_type_id, spv::DecorationBlock, {});
    module_.AddMemberDecoration(struct_type_id, gpuav::kDebugPrintfOutputBufferDWordsCount, spv::DecorationOffset, {0});
    module_.AddMemberDecoration(struct_type_id, gpuav::kDebugPrintfOutputBufferData, spv::DecorationOffset, {4});

    // create a storage buffer interface variable
    const Type& pointer_type = module_.type_manager_.GetTypePointer(spv::StorageClassStorageBuffer, struct_type);
    output_buffer_variable_id_ = module_.TakeNextId();
    auto new_inst = std::make_unique<Instruction>(4, spv::OpVariable);
    new_inst->Fill({pointer_type.Id(), output_buffer_variable_id_, spv::StorageClassStorageBuffer});
    module_.type_manager_.AddVariable(std::move(new_inst), pointer_type);
    module_.AddInterfaceVariables(output_buffer_variable_id_, spv::StorageClassStorageBuffer);

    module_.AddDecoration(output_buffer_variable_id_, spv::DecorationDescriptorSet, {module_.output_buffer_descriptor_set_});
    module_.AddDecoration(output_buffer_variable_id_, spv::DecorationBinding, {binding_slot_});
}

void DebugPrintfPass::CreateBufferWriteFunction(uint32_t argument_count, uint32_t function_id) {
    // Currently this is generated by the number of arguments
    // The following is what the GLSL would look like
    //
    // void inst_debug_printf_5(uint a, uint b, uint c) {
    //     uint offset = atomicAdd(output_buffer.written_count, 5);
    //     if ((offset + 5) <= uint(output_buffer.data.length())) {
    //         output_buffer.data[offset + 0] = 5; // bytes of buffer
    //         output_buffer.data[offset + 1] = stage_id; // known and not passed in
    //         output_buffer.data[offset + 2] = a;
    //         output_buffer.data[offset + 3] = b;
    //         output_buffer.data[offset + 4] = c;
    //     }
    // }

    // Need 1 byte to write the "how many bytes will there be"
    // Need 1 byte for the shader stage (which we don't pass in as we know already)
    const uint32_t byte_written = argument_count + 2;

    // Debug name is matching number of bytes written into the buffer
    std::string function_name = "inst_debug_printf_" + std::to_string(byte_written);
    module_.AddDebugName(function_name.c_str(), function_id);

    // Need to create the function type
    const uint32_t function_type_id = module_.TakeNextId();
    const uint32_t void_type_id = module_.type_manager_.GetTypeVoid().Id();
    const uint32_t uint32_type_id = module_.type_manager_.GetTypeInt(32, false).Id();
    {
        std::vector<uint32_t> words = {function_type_id, void_type_id};
        for (size_t i = 0; i < argument_count; i++) {
            words.push_back(uint32_type_id);
        }
        auto new_inst = std::make_unique<Instruction>((uint32_t)words.size() + 1, spv::OpTypeFunction);
        new_inst->Fill(words);
        module_.type_manager_.AddType(std::move(new_inst), SpvType::kFunction);
    }

    auto& new_function = module_.functions_.emplace_back(std::make_unique<Function>(module_));
    std::vector<uint32_t> function_param_ids;
    {
        auto new_inst = std::make_unique<Instruction>(5, spv::OpFunction);
        new_inst->Fill({void_type_id, function_id, spv::FunctionControlMaskNone, function_type_id});
        new_function->pre_block_inst_.emplace_back(std::move(new_inst));

        for (size_t i = 0; i < argument_count; i++) {
            const uint32_t new_id = module_.TakeNextId();
            auto param_inst = std::make_unique<Instruction>(3, spv::OpFunctionParameter);
            param_inst->Fill({uint32_type_id, new_id});
            new_function->pre_block_inst_.emplace_back(std::move(param_inst));
            function_param_ids.push_back(new_id);
        }
    }

    new_function->InitBlocks(3);
    auto& check_block = new_function->blocks_[0];
    auto& store_block = new_function->blocks_[1];
    auto& merge_block = new_function->blocks_[2];

    const Type& uint32_type = module_.type_manager_.GetTypeInt(32, false);
    const uint32_t pointer_type_id = module_.type_manager_.GetTypePointer(spv::StorageClassStorageBuffer, uint32_type).Id();
    const uint32_t zero_id = module_.type_manager_.GetConstantZeroUint32().Id();
    const uint32_t one_id = module_.type_manager_.GetConstantUInt32(1).Id();
    const uint32_t byte_written_id = module_.type_manager_.GetConstantUInt32(byte_written).Id();
    uint32_t atomic_add_id = 0;

    // Atomically get a write index in the output buffer, and check if this index is with buffer's bounds
    {
        const uint32_t access_chain_id = module_.TakeNextId();
        check_block->CreateInstruction(spv::OpAccessChain, {pointer_type_id, access_chain_id, output_buffer_variable_id_, zero_id});

        atomic_add_id = module_.TakeNextId();
        const uint32_t scope_invok_id = module_.type_manager_.GetConstantUInt32(spv::ScopeInvocation).Id();
        const uint32_t mask_none_id = module_.type_manager_.GetConstantUInt32(spv::MemoryAccessMaskNone).Id();
        check_block->CreateInstruction(
            spv::OpAtomicIAdd, {uint32_type_id, atomic_add_id, access_chain_id, scope_invok_id, mask_none_id, byte_written_id});

        const uint32_t int_add_id = module_.TakeNextId();
        check_block->CreateInstruction(spv::OpIAdd, {uint32_type_id, int_add_id, atomic_add_id, byte_written_id});

        const uint32_t array_length_id = module_.TakeNextId();
        check_block->CreateInstruction(spv::OpArrayLength, {uint32_type_id, array_length_id, output_buffer_variable_id_, 1});

        const uint32_t less_than_equal_id = module_.TakeNextId();
        const uint32_t bool_type_id = module_.type_manager_.GetTypeBool().Id();
        check_block->CreateInstruction(spv::OpULessThanEqual, {bool_type_id, less_than_equal_id, int_add_id, array_length_id});

        const uint32_t merge_block_label_id = merge_block->GetLabelId();
        check_block->CreateInstruction(spv::OpSelectionMerge, {merge_block_label_id, spv::SelectionControlMaskNone});

        const uint32_t store_block_label_id = store_block->GetLabelId();
        check_block->CreateInstruction(spv::OpBranchConditional, {less_than_equal_id, store_block_label_id, merge_block_label_id});
    }

    // Store how many 32-bit words
    {
        const uint32_t int_add_id = module_.TakeNextId();
        store_block->CreateInstruction(spv::OpIAdd, {uint32_type_id, int_add_id, atomic_add_id, zero_id});

        const uint32_t access_chain_id = module_.TakeNextId();
        store_block->CreateInstruction(spv::OpAccessChain,
                                       {pointer_type_id, access_chain_id, output_buffer_variable_id_, one_id, int_add_id});

        store_block->CreateInstruction(spv::OpStore, {access_chain_id, byte_written_id});
    }

    // Store Shader Stage ID
    {
        const uint32_t int_add_id = module_.TakeNextId();
        store_block->CreateInstruction(spv::OpIAdd, {uint32_type_id, int_add_id, atomic_add_id, one_id});

        const uint32_t access_chain_id = module_.TakeNextId();
        store_block->CreateInstruction(spv::OpAccessChain,
                                       {pointer_type_id, access_chain_id, output_buffer_variable_id_, one_id, int_add_id});

        const uint32_t shader_id = module_.type_manager_.GetConstantUInt32(module_.shader_id_).Id();
        store_block->CreateInstruction(spv::OpStore, {access_chain_id, shader_id});
    }

    // Write a 32-bit word to the output buffer for each argument
    const uint32_t argument_id_offset = 2;
    for (uint32_t i = 0; i < argument_count; i++) {
        const uint32_t int_add_id = module_.TakeNextId();
        const uint32_t offset_id = module_.type_manager_.GetConstantUInt32(i + argument_id_offset).Id();
        store_block->CreateInstruction(spv::OpIAdd, {uint32_type_id, int_add_id, atomic_add_id, offset_id});

        const uint32_t access_chain_id = module_.TakeNextId();
        store_block->CreateInstruction(spv::OpAccessChain,
                                       {pointer_type_id, access_chain_id, output_buffer_variable_id_, one_id, int_add_id});

        store_block->CreateInstruction(spv::OpStore, {access_chain_id, function_param_ids[i]});
    }

    // merge block of the above if() check
    {
        store_block->CreateInstruction(spv::OpBranch, {merge_block->GetLabelId()});
        merge_block->CreateInstruction(spv::OpReturn, {});
    }

    {
        auto new_inst = std::make_unique<Instruction>(1, spv::OpFunctionEnd);
        new_function->post_block_inst_.emplace_back(std::move(new_inst));
    }
}

// Used between injections of a function
void DebugPrintfPass::Reset() {
    target_instruction_ = nullptr;
    double_bitmask_ = 0;
    signed_8_bitmask_ = 0;
    signed_16_bitmask_ = 0;
    expanded_parameter_count_ = 0;
}

bool DebugPrintfPass::Instrument() {
    for (const auto& inst : module_.ext_inst_imports_) {
        const char* import_string = inst->GetAsString(2);
        if (strcmp(import_string, "NonSemantic.DebugPrintf") == 0) {
            ext_import_id_ = inst->ResultId();
            break;
        }
    }

    if (ext_import_id_ == 0) {
        return false;  // no printf strings found, early return
    }

    for (const auto& function : module_.functions_) {
        for (auto block_it = function->blocks_.begin(); block_it != function->blocks_.end(); ++block_it) {
            auto& block_instructions = (*block_it)->instructions_;
            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                if (!RequiresInstrumentation(*(inst_it->get()))) continue;
                if (!Validate(*(function.get()))) continue;  // if not valid, don't attempt to instrument it
                instrumentations_count_++;

                // Save the OpString here so we can use it later
                if (function->instrumentation_added_) {
                    for (const auto& debug_inst : module_.debug_source_) {
                        const uint32_t string_id = (*inst_it)->Word(5);
                        if (debug_inst->Opcode() == spv::OpString && debug_inst->ResultId() == string_id) {
                            intenral_only_debug_printf_.emplace_back(
                                InternalOnlyDebugPrintf{module_.shader_id_, string_id, debug_inst->GetAsString(2)});
                        }
                    }
                }

                CreateFunctionCall(block_it, &inst_it);

                // remove the OpExtInst incase they don't support VK_KHR_non_semantic_info
                if (!module_.support_non_semantic_info_) {
                    inst_it = block_instructions.erase(inst_it);
                    inst_it--;
                }

                Reset();
            }
        }
    }
    if (instrumentations_count_ == 0) {
        return false;
    }

    CreateDescriptorSet();

    // Here we "link" the functions, but since it is all generated, no need to go through the LinkInfo flow
    for (const auto& [number_of_args, function_id] : function_id_map_) {
        CreateBufferWriteFunction(number_of_args, function_id);
    }

    // remove the everything else possible incase they don't support VK_KHR_non_semantic_info
    if (!module_.support_non_semantic_info_) {
        bool other_non_semantic = false;
        for (auto inst_it = module_.ext_inst_imports_.begin(); inst_it != module_.ext_inst_imports_.end(); ++inst_it) {
            const char* import_string = (inst_it->get())->GetAsString(2);
            if (strcmp(import_string, "NonSemantic.DebugPrintf") == 0) {
                module_.ext_inst_imports_.erase(inst_it);
                break;
            } else if (strncmp(import_string, "NonSemantic.", 12) == 0) {
                other_non_semantic = true;
            }
        }
        if (!other_non_semantic) {
            for (auto inst_it = module_.extensions_.begin(); inst_it != module_.extensions_.end(); ++inst_it) {
                const char* import_string = (inst_it->get())->GetAsString(1);
                if (strcmp(import_string, "SPV_KHR_non_semantic_info") == 0) {
                    module_.extensions_.erase(inst_it);
                    break;
                }
            }
        }
    }

    return true;
}

void DebugPrintfPass::PrintDebugInfo() const {
    std::cout << "DebugPrintfPass instrumentation count: " << instrumentations_count_ << '\n';
}

// Strictly speaking - the format given in GLSL_EXT_debug_printf is a client side implementation of SPIR-V
// NonSemantic.DebugPrintf There is nothing stopping someone from creating a debug printf implementation
// that goes `printf("Use this &q to print int", myInt)` but this requires both having
// a different HLL and Tool consuming it.
// Currently RenderDoc and the Validation Layers both follow the same syntax, but that also could possibly change.
// Therefore, we validate these here based on the VVL implementation only
bool DebugPrintfPass::Validate(const Function& current_function) {
    static const char* tag = "DEBUG-PRINTF-FORMATTING";

    struct ParamInfo {
        bool is_float = false;  // else int (don't attempt to validate unsigned vs signed here)
        bool is_64_bit = false;
        uint32_t vector_size = 0;  // zero == scalar
        char modifier[32];
    };

    // where we find the first arugment in OpExtInst instruction
    const uint32_t first_argument_offset = 6;

    if (target_instruction_->Length() < first_argument_offset) {
        module_.InternalError(tag, "OpExtInst in a invalid SPIR-V format and should have been caught in spirv-val");
        return false;
    }

    uint32_t string_id = target_instruction_->Word(5);
    const char* op_string = nullptr;
    for (const auto& inst : module_.debug_source_) {
        if (inst->Opcode() == spv::OpString && inst->ResultId() == string_id) {
            op_string = inst->GetAsString(2);
            break;
        }
    }
    if (!op_string) {
        module_.InternalError(tag, "OpExtInst points to an empty/invalid OpString, this should have been caught in spirv-val");
        return false;
    }

    const size_t op_string_len = strlen(op_string);
    if (op_string_len == 0) {
        module_.InternalError(tag, "OpString is empty (string was found, but is empty)");
        return false;
    }

    // If we are going to print the OpString to the user for an error/warning, we need to process it first
    auto print_op_string = [&op_string]() {
        // If there is a '\n' we want to print it like we see in the shader, so need to escape the backslash
        std::string result;
        for (const char* p = op_string; *p != '\0'; ++p) {
            if (*p == '\n') {
                result += "\\n";
            } else if (*p == '\t') {
                result += "\\t";
            } else {
                result += *p;
            }
        }
        return result;
    };

    bool valid = true;
    std::vector<ParamInfo> param_infos;

    // No reason to start checking at the last character, since always need % and something following it
    for (size_t i = 0; i < op_string_len - 1; i++) {
        if (op_string[i] != '%') continue;
        const size_t starting_i = i;
        i++;
        char modifier = op_string[i];
        if (modifier == '%') continue;  // skip "%%"

        if (modifier == ' ') {
            std::string err_msg =
                "OpString \"" + print_op_string() + "\" contains a isolated % which is missing the modifier (to escape use %%)";
            module_.InternalError(tag, err_msg.c_str());
            valid = false;
            break;
        }

        bool found_specifier = false;
        ParamInfo param_info;
        while (i < op_string_len && modifier != ' ' && valid && !found_specifier) {
            switch (modifier) {
                case 'i':
                case 'd':
                case 'o':
                case 'X':
                case 'x':
                    found_specifier = true;
                    break;
                case 'u':
                    if (i + 1 < op_string_len && op_string[i + 1] == 'l') {
                        param_info.is_64_bit = true;
                    }
                    found_specifier = true;
                    break;
                case 'a':
                case 'A':
                case 'e':
                case 'E':
                case 'f':
                case 'F':
                case 'g':
                case 'G':
                    found_specifier = true;
                    param_info.is_float = true;
                    break;
                case 'l':
                    param_info.is_64_bit = true;
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                case '*':
                case '.':
                    break;  // expected for precision
                case 'v': {
                    if (i + 1 >= op_string_len) {
                        std::string err_msg = "OpString \"" + print_op_string() +
                                              "\" contains a %v at the end, but vectors require a width and type after it";
                        module_.InternalError(tag, err_msg.c_str());
                        valid = false;
                    } else {
                        i++;
                        const char vec_size = op_string[i];
                        if (vec_size == '2') {
                            param_info.vector_size = 2;
                        } else if (vec_size == '3') {
                            param_info.vector_size = 3;
                        } else if (vec_size == '4') {
                            param_info.vector_size = 4;
                        } else {
                            std::string err_msg = "OpString \"" + print_op_string() + "\" contains a %v" + vec_size +
                                                  " needs to be valid vector width (v2, v3, or v4)";
                            module_.InternalError(tag, err_msg.c_str());
                            valid = false;
                        }
                    }
                    break;
                }
                default:
                    std::string err_msg = "OpString \"" + print_op_string() + "\" contains a \"" + modifier +
                                          "\" modifier which is an unknown modifier.";
                    module_.InternalError(tag, err_msg.c_str());
                    valid = false;
                    break;  // unknown
            };

            i++;
            modifier = op_string[i];
        }

        if (valid) {
            // Get for other error messages
            strncpy(param_info.modifier, &op_string[starting_i], i - starting_i);
            param_info.modifier[i - starting_i] = '\0';

            if (!found_specifier) {
                std::string err_msg = "OpString \"" + print_op_string() + "\" contains \"" + std::string(param_info.modifier) +
                                      "\" which is missing a valid specifier (d, i, o, u, x, X, a, A, e, E, f, F, g, or G).";
                module_.InternalError(tag, err_msg.c_str());
                valid = false;
            }
        }

        if (!valid) break;
        param_infos.push_back(param_info);
    }
    if (!valid) return false;

    const uint32_t argument_count = target_instruction_->Length() - first_argument_offset;
    if (argument_count > param_infos.size()) {
        std::string err_msg = "OpString \"" + print_op_string() + "\" contains only " + std::to_string(param_infos.size()) +
                              " modifiers, but " + std::to_string(argument_count) +
                              " arguments were passed in and some will be ignored";
        module_.InternalWarning(tag, err_msg.c_str());

    } else if (argument_count < param_infos.size()) {
        std::string err_msg = "OpString \"" + print_op_string() + "\" contains " + std::to_string(param_infos.size()) +
                              " modifiers, but only " + std::to_string(argument_count) +
                              " arguments were passed in and garbage data might start to occur";
        module_.InternalError(tag, err_msg.c_str());
        return false;
    }

    const uint32_t count = std::min(argument_count, (uint32_t)param_infos.size());
    for (uint32_t i = 0; i < count; i++) {
        const ParamInfo& param = param_infos[i];
        const uint32_t argument_id = target_instruction_->Word(first_argument_offset + i);

        const Type* argument_type = nullptr;
        if (const Constant* constant = module_.type_manager_.FindConstantById(argument_id)) {
            argument_type = &constant->type_;
        } else {
            const Instruction* inst = current_function.FindInstruction(argument_id);
            if (!inst) {
                module_.InternalWarning(tag, "Unable to find OpExtInst ID inside function block");
                return true;  // possibily our error, so leave a warning
            }
            argument_type = module_.type_manager_.FindTypeById(inst->TypeId());
        }
        if (!argument_type) {
            module_.InternalWarning(tag, "Unable find OpExtInst ID type");
            return true;  // possibily our error, so leave a warning
        }

        // first strip/validate vectors
        if (param.vector_size != 0) {
            if (argument_type->spv_type_ != SpvType::kVector) {
                std::string err_msg = "OpString \"" + print_op_string() + "\" contains a vector modifier \"" + param.modifier +
                                      "\", but the argument (SPIR-V Id " + std::to_string(argument_id) + ") is not a vector";
                module_.InternalError(tag, err_msg.c_str());
                return false;
            }
            const uint32_t vector_size = argument_type->inst_.Word(3);
            if (vector_size != param.vector_size) {
                std::string err_msg = "OpString \"" + print_op_string() + "\" contains a " + std::to_string(param.vector_size) +
                                      "-wide vector modifier \"" + param.modifier + "\", but the argument (SPIR-V Id " +
                                      std::to_string(argument_id) + ") is a " + std::to_string(vector_size) +
                                      "-wide vector (values might be truncated or padded)";
                module_.InternalWarning(tag, err_msg.c_str());
            }

            // Get the underlying type (float or int)
            argument_type = module_.type_manager_.FindTypeById(argument_type->inst_.Word(2));
            assert(argument_type);
        } else {
            if (argument_type->spv_type_ == SpvType::kVector) {
                std::string err_msg = "OpString \"" + print_op_string() + "\" contains a non-vector modifier \"" + param.modifier +
                                      "\", but the argument (SPIR-V Id " + std::to_string(argument_id) + ") is a vector";
                module_.InternalError(tag, err_msg.c_str());
                return false;
            }
        }

        // this is after stripping the vector
        if (argument_type->spv_type_ != SpvType::kFloat && argument_type->spv_type_ != SpvType::kInt &&
            argument_type->spv_type_ != SpvType::kBool) {
            std::string err_msg = "OpString \"" + print_op_string() + "\" contains a modifier \"" + param.modifier +
                                  "\", but the argument (SPIR-V Id " + std::to_string(argument_id) +
                                  ") is not a float, int, or bool";
            module_.InternalError(tag, err_msg.c_str());
            return false;
        }

        const bool type_is_64 = argument_type->spv_type_ != SpvType::kBool && argument_type->inst_.Word(2) == 64;
        if (!param.is_64_bit && type_is_64) {
            std::string err_msg = "OpString \"" + print_op_string() + "\" contains a non-64-bit modifier \"" + param.modifier +
                                  "\", but the argument (SPIR-V Id " + std::to_string(argument_id) + ") a 64-bit";
            module_.InternalWarning(tag, err_msg.c_str());
        } else if (param.is_64_bit && !type_is_64) {
            std::string err_msg = "OpString \"" + print_op_string() + "\" contains a 64-bit modifier \"" + param.modifier +
                                  "\", but the argument (SPIR-V Id " + std::to_string(argument_id) + ") is not 64-bit";
            module_.InternalWarning(tag, err_msg.c_str());
        } else if (!param.is_float && argument_type->spv_type_ == SpvType::kFloat) {
            std::string err_msg = "OpString \"" + print_op_string() + "\" contains a non-float modifier \"" + param.modifier +
                                  "\", but the argument (SPIR-V Id " + std::to_string(argument_id) + ") is a float";
            module_.InternalWarning(tag, err_msg.c_str());
        } else if (param.is_float && argument_type->spv_type_ != SpvType::kFloat) {
            std::string err_msg = "OpString \"" + print_op_string() + "\" contains a float modifier \"" + param.modifier +
                                  "\", but the argument (SPIR-V Id " + std::to_string(argument_id) + ") is not a float";
            module_.InternalWarning(tag, err_msg.c_str());
        }
    }

    return true;
}

}  // namespace spirv
}  // namespace gpuav
