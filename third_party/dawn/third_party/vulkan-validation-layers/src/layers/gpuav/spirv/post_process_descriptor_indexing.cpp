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

#include "post_process_descriptor_indexing_pass.h"
#include "module.h"
#include <iostream>

#include "generated/instrumentation_post_process_descriptor_index_comp.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"

namespace gpuav {
namespace spirv {

PostProcessDescriptorIndexingPass::PostProcessDescriptorIndexingPass(Module& module) : Pass(module) { module.use_bda_ = true; }

// By appending the LinkInfo, it will attempt at linking stage to add the function.
uint32_t PostProcessDescriptorIndexingPass::GetLinkFunctionId() {
    static LinkInfo link_info = {instrumentation_post_process_descriptor_index_comp,
                                 instrumentation_post_process_descriptor_index_comp_size, 0, "inst_post_process_descriptor_index"};

    if (link_function_id == 0) {
        link_function_id = module_.TakeNextId();
        link_info.function_id = link_function_id;
        module_.link_info_.push_back(link_info);
    }
    return link_function_id;
}

void PostProcessDescriptorIndexingPass::CreateFunctionCall(BasicBlockIt block_it, InstructionIt* inst_it) {
    BasicBlock& block = **block_it;

    const Constant& set_constant = module_.type_manager_.GetConstantUInt32(descriptor_set_);
    const Constant& binding_constant = module_.type_manager_.GetConstantUInt32(descriptor_binding_);
    const uint32_t descriptor_index_id = CastToUint32(descriptor_index_id_, block, inst_it);  // might be int32

    BindingLayout binding_layout = module_.set_index_to_bindings_layout_lut_[descriptor_set_][descriptor_binding_];
    const Constant& binding_layout_offset = module_.type_manager_.GetConstantUInt32(binding_layout.start);
    const Constant& variable_id_constant = module_.type_manager_.GetConstantUInt32(variable_id_);

    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunctionId();
    const uint32_t void_type = module_.type_manager_.GetTypeVoid().Id();

    block.CreateInstruction(spv::OpFunctionCall,
                            {void_type, function_result, function_def, set_constant.Id(), binding_constant.Id(),
                             descriptor_index_id, binding_layout_offset.Id(), variable_id_constant.Id()},
                            inst_it);
}

bool PostProcessDescriptorIndexingPass::RequiresInstrumentation(const Function& function, const Instruction& inst) {
    const uint32_t opcode = inst.Opcode();

    const Instruction* var_inst = nullptr;
    if (opcode == spv::OpLoad || opcode == spv::OpStore) {
        const Variable* variable = nullptr;
        const Instruction* access_chain_inst = function.FindInstruction(inst.Operand(0));
        // We need to walk down possibly multiple chained OpAccessChains or OpCopyObject to get the variable
        while (access_chain_inst && access_chain_inst->Opcode() == spv::OpAccessChain) {
            const uint32_t access_chain_base_id = access_chain_inst->Operand(0);
            variable = module_.type_manager_.FindVariableById(access_chain_base_id);
            if (variable) {
                break;  // found
            }
            access_chain_inst = function.FindInstruction(access_chain_base_id);
        }
        if (!variable) {
            return false;
        }
        var_inst = &variable->inst_;

        const uint32_t storage_class = variable->StorageClass();
        if (storage_class != spv::StorageClassUniform && storage_class != spv::StorageClassStorageBuffer) {
            return false;
        }

        const Type* pointer_type = variable->PointerType(module_.type_manager_);
        if (pointer_type->IsArray()) {
            descriptor_index_id_ = access_chain_inst->Operand(1);
        } else {
            // There is no array of this descriptor, so we essentially have an array of 1
            descriptor_index_id_ = module_.type_manager_.GetConstantZeroUint32().Id();
        }

    } else {
        // Reference is not load or store, so if it isn't a image-based reference, move on
        const uint32_t image_word = OpcodeImageAccessPosition(opcode);
        if (image_word == 0) {
            return false;
        }
        if (opcode == spv::OpImageTexelPointer || opcode == spv::OpImage) {
            return false;  // need to test if we can support these
        }

        const Instruction* load_inst = function.FindInstruction(inst.Word(image_word));
        while (load_inst && (load_inst->Opcode() == spv::OpSampledImage || load_inst->Opcode() == spv::OpImage ||
                             load_inst->Opcode() == spv::OpCopyObject)) {
            load_inst = function.FindInstruction(load_inst->Operand(0));
        }
        if (!load_inst || load_inst->Opcode() != spv::OpLoad) {
            return false;  // TODO: Handle additional possibilities?
        }

        var_inst = function.FindInstruction(load_inst->Operand(0));
        if (!var_inst) {
            // can be a global variable
            const Variable* global_var = module_.type_manager_.FindVariableById(load_inst->Operand(0));
            var_inst = global_var ? &global_var->inst_ : nullptr;
        }
        if (!var_inst || (var_inst->Opcode() != spv::OpAccessChain && var_inst->Opcode() != spv::OpVariable)) {
            return false;
        }

        if (var_inst->Opcode() == spv::OpAccessChain) {
            descriptor_index_id_ = var_inst->Operand(1);

            if (var_inst->Length() > 5) {
                module_.InternalError(Name(), "OpAccessChain has more than 1 indexes");
                return false;
            }

            const Variable* variable = module_.type_manager_.FindVariableById(var_inst->Operand(0));
            if (!variable) {
                module_.InternalError(Name(), "OpAccessChain base is not a variable");
                return false;
            }
            var_inst = &variable->inst_;
        } else {
            descriptor_index_id_ = module_.type_manager_.GetConstantZeroUint32().Id();
        }
    }

    assert(var_inst);
    variable_id_ = var_inst->ResultId();
    for (const auto& annotation : module_.annotations_) {
        if (annotation->Opcode() == spv::OpDecorate && annotation->Word(1) == variable_id_) {
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

    target_instruction_ = &inst;

    return true;
}

void PostProcessDescriptorIndexingPass::Reset() {
    target_instruction_ = nullptr;
    descriptor_set_ = 0;
    descriptor_binding_ = 0;
    descriptor_index_id_ = 0;
    variable_id_ = 0;
}

bool PostProcessDescriptorIndexingPass::Instrument() {
    for (const auto& function : module_.functions_) {
        if (function->instrumentation_added_) continue;
        for (auto block_it = function->blocks_.begin(); block_it != function->blocks_.end(); ++block_it) {
            auto& block_instructions = (*block_it)->instructions_;
            for (auto inst_it = block_instructions.begin(); inst_it != block_instructions.end(); ++inst_it) {
                if (!RequiresInstrumentation(*function, *(inst_it->get()))) continue;

                if (module_.max_instrumentations_count_ != 0 && instrumentations_count_ >= module_.max_instrumentations_count_) {
                    return true;  // hit limit
                }
                instrumentations_count_++;

                CreateFunctionCall(block_it, &inst_it);
                Reset();
            }
        }
    }

    return (instrumentations_count_ != 0);
}

void PostProcessDescriptorIndexingPass::PrintDebugInfo() const {
    std::cout << "PostProcessDescriptorIndexingPass instrumentation count: " << instrumentations_count_ << '\n';
}

}  // namespace spirv
}  // namespace gpuav
