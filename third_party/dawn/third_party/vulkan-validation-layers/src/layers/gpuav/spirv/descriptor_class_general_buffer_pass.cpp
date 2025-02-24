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

#include "descriptor_class_general_buffer_pass.h"
#include "generated/spirv_grammar_helper.h"
#include "instruction.h"
#include "utils/vk_layer_utils.h"
#include "module.h"
#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/instrumentation_descriptor_class_general_buffer_comp.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"

namespace gpuav {
namespace spirv {

DescriptorClassGeneralBufferPass::DescriptorClassGeneralBufferPass(Module& module) : Pass(module) { module.use_bda_ = true; }

// By appending the LinkInfo, it will attempt at linking stage to add the function.
uint32_t DescriptorClassGeneralBufferPass::GetLinkFunctionId() {
    static LinkInfo link_info = {instrumentation_descriptor_class_general_buffer_comp,
                                 instrumentation_descriptor_class_general_buffer_comp_size, 0,
                                 "inst_descriptor_class_general_buffer"};

    if (link_function_id == 0) {
        link_function_id = module_.TakeNextId();
        link_info.function_id = link_function_id;
        module_.link_info_.push_back(link_info);
    }
    return link_function_id;
}

uint32_t DescriptorClassGeneralBufferPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it,
                                                              const InjectionData& injection_data) {
    assert(!access_chain_insts_.empty());
    const Constant& set_constant = module_.type_manager_.GetConstantUInt32(descriptor_set_);
    const Constant& binding_constant = module_.type_manager_.GetConstantUInt32(descriptor_binding_);
    const uint32_t descriptor_index_id = CastToUint32(descriptor_index_id_, block, inst_it);  // might be int32

    descriptor_offset_id_ = GetLastByte(*descriptor_type_, access_chain_insts_, block, inst_it);  // Get Last Byte Index

    BindingLayout binding_layout = module_.set_index_to_bindings_layout_lut_[descriptor_set_][descriptor_binding_];
    const Constant& binding_layout_offset = module_.type_manager_.GetConstantUInt32(binding_layout.start);

    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunctionId();
    const uint32_t bool_type = module_.type_manager_.GetTypeBool().Id();

    block.CreateInstruction(
        spv::OpFunctionCall,
        {bool_type, function_result, function_def, injection_data.inst_position_id, injection_data.stage_info_id, set_constant.Id(),
         binding_constant.Id(), descriptor_index_id, descriptor_offset_id_, binding_layout_offset.Id()},
        inst_it);

    return function_result;
}

void DescriptorClassGeneralBufferPass::Reset() {
    descriptor_type_ = nullptr;
    target_instruction_ = nullptr;
    descriptor_set_ = 0;
    descriptor_binding_ = 0;
    descriptor_index_id_ = 0;
    descriptor_offset_id_ = 0;
}

bool DescriptorClassGeneralBufferPass::RequiresInstrumentation(const Function& function, const Instruction& inst) {
    const uint32_t opcode = inst.Opcode();

    if (!IsValueIn(spv::Op(opcode), {spv::OpLoad, spv::OpStore, spv::OpAtomicStore, spv::OpAtomicLoad, spv::OpAtomicExchange})) {
        return false;
    }

    const Instruction* next_access_chain = function.FindInstruction(inst.Operand(0));
    if (!next_access_chain || next_access_chain->Opcode() != spv::OpAccessChain) {
        return false;
    }
    access_chain_insts_.clear();  // only clear right before we know we will need again

    const Variable* variable = nullptr;
    // We need to walk down possibly multiple chained OpAccessChains or OpCopyObject to get the variable
    while (next_access_chain && next_access_chain->Opcode() == spv::OpAccessChain) {
        access_chain_insts_.push_back(next_access_chain);
        const uint32_t access_chain_base_id = next_access_chain->Operand(0);
        variable = module_.type_manager_.FindVariableById(access_chain_base_id);
        if (variable) {
            break;  // found
        }
        next_access_chain = function.FindInstruction(access_chain_base_id);
    }
    if (!variable) {
        return false;
    }

    uint32_t storage_class = variable->StorageClass();
    if (storage_class != spv::StorageClassUniform && storage_class != spv::StorageClassStorageBuffer) {
        return false;
    }

    const Type* pointer_type = variable->PointerType(module_.type_manager_);
    if (pointer_type->spv_type_ == SpvType::kRuntimeArray) {
        return false;  // TODO - Currently we mark these as "bindless"
    }

    const bool is_descriptor_array = pointer_type->IsArray();

    // Check for deprecated storage block form
    if (storage_class == spv::StorageClassUniform) {
        const uint32_t block_type_id = is_descriptor_array ? pointer_type->inst_.Operand(0) : pointer_type->Id();
        assert(module_.type_manager_.FindTypeById(block_type_id)->spv_type_ == SpvType::kStruct && "unexpected block type");

        const bool block_found = GetDecoration(block_type_id, spv::DecorationBlock) != nullptr;

        // If block decoration not found, verify deprecated form of SSBO
        if (!block_found) {
            assert(GetDecoration(block_type_id, spv::DecorationBufferBlock) != nullptr && "block decoration not found");
            storage_class = spv::StorageClassStorageBuffer;
        }
    }

    // Grab front() as it will be the "final" type we access
    const Type* value_type = module_.type_manager_.FindValueTypeById(access_chain_insts_.front()->TypeId());
    if (!value_type) return false;

    if (is_descriptor_array) {
        // Because you can't have 2D array of descriptors, the first index of the last accessChain is the descriptor index
        descriptor_index_id_ = access_chain_insts_.back()->Operand(1);
    } else {
        // There is no array of this descriptor, so we essentially have an array of 1
        descriptor_index_id_ = module_.type_manager_.GetConstantZeroUint32().Id();
    }

    for (const auto& annotation : module_.annotations_) {
        if (annotation->Opcode() == spv::OpDecorate && annotation->Word(1) == variable->Id()) {
            if (annotation->Word(2) == spv::DecorationDescriptorSet) {
                descriptor_set_ = annotation->Word(3);
            } else if (annotation->Word(2) == spv::DecorationBinding) {
                descriptor_binding_ = annotation->Word(3);
            }
        }
    }

    if (descriptor_set_ >= glsl::kDebugInputBindlessMaxDescSets) {
        module_.InternalWarning(Name(), "Tried to use a descriptor slot over the current max limit");
        return false;
    }

    descriptor_type_ = variable->PointerType(module_.type_manager_);
    if (!descriptor_type_) return false;

    // Save information to be used to make the Function
    target_instruction_ = &inst;

    return true;
}

void DescriptorClassGeneralBufferPass::PrintDebugInfo() const {
    std::cout << "DescriptorClassGeneralBufferPass instrumentation count: " << instrumentations_count_ << '\n';
}

// Created own Instrument() because need to control finding the largest offset in a given block
bool DescriptorClassGeneralBufferPass::Instrument() {
    // Can safely loop function list as there is no injecting of new Functions until linking time
    for (const auto& function : module_.functions_) {
        if (function->instrumentation_added_) continue;
        for (auto block_it = function->blocks_.begin(); block_it != function->blocks_.end(); ++block_it) {
            if ((*block_it)->loop_header_) {
                continue;  // Currently can't properly handle injecting CFG logic into a loop header block
            }
            auto& block_instructions = (*block_it)->instructions_;
            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                // Every instruction is analyzed by the specific pass and lets us know if we need to inject a function or not
                if (!RequiresInstrumentation(*function, *(inst_it->get()))) continue;

                if (module_.max_instrumentations_count_ != 0 && instrumentations_count_ >= module_.max_instrumentations_count_) {
                    return true;  // hit limit
                }
                instrumentations_count_++;

                // Add any debug information to pass into the function call
                InjectionData injection_data;
                injection_data.stage_info_id = GetStageInfo(*function, block_it, inst_it);
                const uint32_t inst_position = target_instruction_->position_index_;
                auto inst_position_constant = module_.type_manager_.CreateConstantUInt32(inst_position);
                injection_data.inst_position_id = inst_position_constant.Id();

                // inst_it is updated to the instruction after the new function call, it will not add/remove any Blocks
                CreateFunctionCall(**block_it, &inst_it, injection_data);
                Reset();
            }
        }
    }

    return instrumentations_count_ != 0;
}

}  // namespace spirv
}  // namespace gpuav