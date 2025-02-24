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

#include "descriptor_indexing_oob_pass.h"
#include "link.h"
#include "module.h"
#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/instrumentation_descriptor_indexing_oob_bindless_comp.h"
#include "generated/instrumentation_descriptor_indexing_oob_non_bindless_comp.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"

namespace gpuav {
namespace spirv {

// By appending the LinkInfo, it will attempt at linking stage to add the function.
uint32_t DescriptorIndexingOOBPass::GetLinkFunctionId() {
    // This pass has 2 variations of GLSL we can pull in. Non-Bindless is simpler and we want to use when possible
    static LinkInfo link_info_bindless = {instrumentation_descriptor_indexing_oob_bindless_comp,
                                          instrumentation_descriptor_indexing_oob_bindless_comp_size, 0,
                                          "inst_descriptor_indexing_oob_bindless"};
    static LinkInfo link_info_non_bindless = {instrumentation_descriptor_indexing_oob_non_bindless_comp,
                                              instrumentation_descriptor_indexing_oob_non_bindless_comp_size, 0,
                                              "inst_descriptor_indexing_oob_non_bindless"};

    if (link_function_id == 0) {
        link_function_id = module_.TakeNextId();
        LinkInfo& link_info = module_.has_bindless_descriptors_ ? link_info_bindless : link_info_non_bindless;
        link_info.function_id = link_function_id;
        module_.link_info_.push_back(link_info);
    }
    return link_function_id;
}

uint32_t DescriptorIndexingOOBPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it,
                                                       const InjectionData& injection_data) {
    const Constant& set_constant = module_.type_manager_.GetConstantUInt32(descriptor_set_);
    const Constant& binding_constant = module_.type_manager_.GetConstantUInt32(descriptor_binding_);
    const uint32_t descriptor_index_id = CastToUint32(descriptor_index_id_, block, inst_it);  // might be int32

    if (image_inst_) {
        const uint32_t opcode = target_instruction_->Opcode();
        if (opcode != spv::OpImageRead && opcode != spv::OpImageFetch && opcode != spv::OpImageWrite) {
            // if not a direct read/write/fetch, will be a OpSampledImage
            // "All OpSampledImage instructions must be in the same block in which their Result <id> are consumed"
            // the simple way around this is to add a OpCopyObject to be consumed by the target instruction
            uint32_t image_id = target_instruction_->Operand(0);
            const Instruction* sampled_image_inst = block.function_.FindInstruction(image_id);
            // TODO - Add tests to understand what else can be here other then OpSampledImage
            if (sampled_image_inst->Opcode() == spv::OpSampledImage) {
                const uint32_t type_id = sampled_image_inst->TypeId();
                const uint32_t copy_id = module_.TakeNextId();
                const_cast<Instruction*>(target_instruction_)->ReplaceOperandId(image_id, copy_id);

                // incase the OpSampledImage is shared, copy the previous OpCopyObject
                auto copied = copy_object_map_.find(image_id);
                if (copied != copy_object_map_.end()) {
                    image_id = copied->second;
                    block.CreateInstruction(spv::OpCopyObject, {type_id, copy_id, image_id}, inst_it);
                } else {
                    copy_object_map_.emplace(image_id, copy_id);
                    // slower, but need to guarantee it is placed after a OpSampledImage
                    block.function_.CreateInstruction(spv::OpCopyObject, {type_id, copy_id, image_id}, image_id);
                }
            }
        }
    }

    BindingLayout binding_layout = module_.set_index_to_bindings_layout_lut_[descriptor_set_][descriptor_binding_];
    const Constant& binding_layout_size = module_.type_manager_.GetConstantUInt32(binding_layout.count);
    const Constant& binding_layout_offset = module_.type_manager_.GetConstantUInt32(binding_layout.start);

    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunctionId();
    const uint32_t bool_type = module_.type_manager_.GetTypeBool().Id();

    block.CreateInstruction(
        spv::OpFunctionCall,
        {bool_type, function_result, function_def, injection_data.inst_position_id, injection_data.stage_info_id, set_constant.Id(),
         binding_constant.Id(), descriptor_index_id, binding_layout_size.Id(), binding_layout_offset.Id()},
        inst_it);

    return function_result;
}

void DescriptorIndexingOOBPass::Reset() {
    var_inst_ = nullptr;
    image_inst_ = nullptr;
    target_instruction_ = nullptr;
    descriptor_set_ = 0;
    descriptor_binding_ = 0;
    descriptor_index_id_ = 0;
}

bool DescriptorIndexingOOBPass::RequiresInstrumentation(const Function& function, const Instruction& inst) {
    const uint32_t opcode = inst.Opcode();

    bool array_found = false;
    if (opcode == spv::OpAtomicLoad || opcode == spv::OpAtomicStore || opcode == spv::OpAtomicExchange) {
        // Image Atomics
        const Instruction* image_texel_ptr_inst = function.FindInstruction(inst.Operand(0));
        if (!image_texel_ptr_inst || image_texel_ptr_inst->Opcode() != spv::OpImageTexelPointer) {
            return false;
        }

        const Variable* variable = nullptr;
        const Instruction* access_chain_inst = function.FindInstruction(image_texel_ptr_inst->Operand(0));
        if (access_chain_inst) {
            variable = module_.type_manager_.FindVariableById(access_chain_inst->Operand(0));
        } else {
            // if no array, will point right to a variable
            variable = module_.type_manager_.FindVariableById(image_texel_ptr_inst->Operand(0));
        }

        if (!variable) {
            return false;
        }
        var_inst_ = &variable->inst_;

        const Type* pointer_type = variable->PointerType(module_.type_manager_);
        if (!pointer_type) {
            module_.InternalError(Name(), "Pointer type not found");
            return false;
        }

        const bool non_empty_access_chain = access_chain_inst && access_chain_inst->Length() >= 5;
        if (pointer_type->IsArray() && non_empty_access_chain) {
            array_found = true;
            descriptor_index_id_ = access_chain_inst->Operand(1);
        } else {
            // There is no array of this descriptor, so we essentially have an array of 1
            descriptor_index_id_ = module_.type_manager_.GetConstantZeroUint32().Id();
        }
    } else if (opcode == spv::OpLoad || opcode == spv::OpStore || AtomicOperation(opcode)) {
        // Buffer and Buffer Atomics and Storage Images

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

        var_inst_ = &variable->inst_;

        const uint32_t storage_class = variable->StorageClass();
        if (storage_class == spv::StorageClassUniformConstant) {
            // TODO - Need to add Storage Image support
            return false;
        }
        if (storage_class != spv::StorageClassUniform && storage_class != spv::StorageClassStorageBuffer) {
            return false;  // Prevents things like Push Constants
        }

        const Type* pointer_type = variable->PointerType(module_.type_manager_);
        if (!pointer_type) {
            module_.InternalError(Name(), "Pointer type not found");
            return false;
        }

        if (pointer_type->IsArray()) {
            array_found = true;
            descriptor_index_id_ = access_chain_inst->Operand(1);
        } else {
            // There is no array of this descriptor, so we essentially have an array of 1
            descriptor_index_id_ = module_.type_manager_.GetConstantZeroUint32().Id();
        }
    } else {
        // sampled image (non-atomic)

        // Reference is not load or store, so if it isn't a image-based reference, move on
        const uint32_t image_word = OpcodeImageAccessPosition(opcode);
        if (image_word == 0) {
            return false;
        }

        // Things that have an OpImage (in OpcodeImageAccessPosition) but we don't want to handle
        if (opcode == spv::OpImageRead || opcode == spv::OpImageWrite) {
            return false;  // Storage Images are handled at OpLoad
        } else if (opcode == spv::OpImageTexelPointer) {
            return false;  // atomics are handled separately
        } else if (opcode == spv::OpImage) {
            return false;  // Don't deal with the access directly
        }

        image_inst_ = function.FindInstruction(inst.Word(image_word));
        const Instruction* load_inst = image_inst_;
        while (load_inst && (load_inst->Opcode() == spv::OpSampledImage || load_inst->Opcode() == spv::OpImage ||
                             load_inst->Opcode() == spv::OpCopyObject)) {
            load_inst = function.FindInstruction(load_inst->Operand(0));
        }
        if (!load_inst || load_inst->Opcode() != spv::OpLoad) {
            return false;  // TODO: Handle additional possibilities?
        }

        var_inst_ = function.FindInstruction(load_inst->Operand(0));
        if (!var_inst_) {
            // can be a global variable
            const Variable* global_var = module_.type_manager_.FindVariableById(load_inst->Operand(0));
            var_inst_ = global_var ? &global_var->inst_ : nullptr;
        }
        if (!var_inst_ || (var_inst_->Opcode() != spv::OpAccessChain && var_inst_->Opcode() != spv::OpVariable)) {
            return false;
        }

        if (var_inst_->Opcode() == spv::OpAccessChain) {
            array_found = true;
            descriptor_index_id_ = var_inst_->Operand(1);

            if (var_inst_->Length() > 5) {
                module_.InternalError(Name(), "OpAccessChain has more than 1 indexes");
                return false;
            }

            const Variable* variable = module_.type_manager_.FindVariableById(var_inst_->Operand(0));
            if (!variable) {
                module_.InternalError(Name(), "OpAccessChain base is not a variable");
                return false;
            }
            var_inst_ = &variable->inst_;
        } else {
            descriptor_index_id_ = module_.type_manager_.GetConstantZeroUint32().Id();
        }
    }

    // guaranteed to be valid already, save compiler time optimizing the check out
    if (!array_found && !module_.has_bindless_descriptors_) {
        return false;
    }

    assert(var_inst_);
    uint32_t variable_id = var_inst_->ResultId();
    for (const auto& annotation : module_.annotations_) {
        if (annotation->Opcode() == spv::OpDecorate && annotation->Word(1) == variable_id) {
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

    // Save information to be used to make the Function
    target_instruction_ = &inst;

    return true;
}

void DescriptorIndexingOOBPass::PrintDebugInfo() const {
    std::cout << "DescriptorIndexingOOBPass instrumentation count: " << instrumentations_count_ << " ("
              << (module_.has_bindless_descriptors_ ? "Bindless version" : "Non Bindless version") << ")\n";
}

}  // namespace spirv
}  // namespace gpuav