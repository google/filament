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

#include "inject_conditional_function_pass.h"
#include "module.h"

namespace gpuav {
namespace spirv {

InjectConditionalFunctionPass::InjectConditionalFunctionPass(Module& module) : Pass(module) { module.use_bda_ = true; }

BasicBlockIt InjectConditionalFunctionPass::InjectFunction(Function* function, BasicBlockIt block_it, InstructionIt inst_it,
                                                           const InjectionData& injection_data) {
    // We turn the block into 4 separate blocks
    block_it = function->InsertNewBlock(block_it);
    block_it = function->InsertNewBlock(block_it);
    block_it = function->InsertNewBlock(block_it);
    BasicBlock& original_block = **(std::prev(block_it, 3));
    // Where we call targeted instruction if it is valid
    BasicBlock& valid_block = **(std::prev(block_it, 2));
    // will be an empty block, used for the Phi node, even if no result, create for simplicity
    BasicBlock& invalid_block = **(std::prev(block_it, 1));
    // All the remaining block instructions after targeted instruction
    BasicBlock& merge_block = **block_it;

    const uint32_t original_label = original_block.GetLabelId();
    const uint32_t valid_block_label = valid_block.GetLabelId();
    const uint32_t invalid_block_label = invalid_block.GetLabelId();
    const uint32_t merge_block_label = merge_block.GetLabelId();

    // need to preserve the control-flow of how things, like a OpPhi, are accessed from a predecessor block
    function->ReplaceAllUsesWith(original_label, merge_block_label);

    // Move the targeted instruction to a valid block
    const Instruction& target_inst = *valid_block.instructions_.emplace_back(std::move(*inst_it));
    inst_it = original_block.instructions_.erase(inst_it);
    valid_block.CreateInstruction(spv::OpBranch, {merge_block_label});

    // If thre is a result, we need to create an additional BasicBlock to hold the |else| case, then after we create a Phi node to
    // hold the result
    const uint32_t target_inst_id = target_inst.ResultId();
    if (target_inst_id != 0) {
        const uint32_t phi_id = module_.TakeNextId();
        const Type& phi_type = *module_.type_manager_.FindTypeById(target_inst.TypeId());
        uint32_t null_id = 0;
        // Can't create ConstantNull of pointer type, so convert uint64 zero to pointer
        if (phi_type.spv_type_ == SpvType::kPointer) {
            const Type& uint64_type = module_.type_manager_.GetTypeInt(64, false);
            const Constant& null_constant = module_.type_manager_.GetConstantNull(uint64_type);
            null_id = module_.TakeNextId();
            // We need to put any intermittent instructions here so Phi is first in the merge block
            invalid_block.CreateInstruction(spv::OpConvertUToPtr, {phi_type.Id(), null_id, null_constant.Id()});
            module_.AddCapability(spv::CapabilityInt64);
        } else {
            if ((phi_type.spv_type_ == SpvType::kInt || phi_type.spv_type_ == SpvType::kFloat) && phi_type.inst_.Word(2) < 32) {
                // You can't make a constant of a 8-int, 16-int, 16-float without having the capability
                // The only way this situation occurs if they use something like
                //     OpCapability StorageBuffer8BitAccess
                // but there is not explicit Int8
                // It should be more than safe to inject it for them
                spv::Capability capability = (phi_type.spv_type_ == SpvType::kFloat) ? spv::CapabilityFloat16
                                             : (phi_type.inst_.Word(2) == 16)        ? spv::CapabilityInt16
                                                                                     : spv::CapabilityInt8;
                module_.AddCapability(capability);
            }

            null_id = module_.type_manager_.GetConstantNull(phi_type).Id();
        }

        // replace before creating instruction, otherwise will over-write itself
        function->ReplaceAllUsesWith(target_inst_id, phi_id);
        merge_block.CreateInstruction(spv::OpPhi,
                                      {phi_type.Id(), phi_id, target_inst_id, valid_block_label, null_id, invalid_block_label});
    }

    // When skipping some instructions, we need something valid to replace it
    if (target_inst.Opcode() == spv::OpRayQueryInitializeKHR) {
        // Currently assume the RayQuery and AS object were valid already
        const uint32_t uint32_0_id = module_.type_manager_.GetConstantZeroUint32().Id();
        const uint32_t float32_0_id = module_.type_manager_.GetConstantZeroFloat32().Id();
        const uint32_t vec3_0_id = module_.type_manager_.GetConstantZeroVec3().Id();
        invalid_block.CreateInstruction(spv::OpRayQueryInitializeKHR,
                                        {target_inst.Operand(0), target_inst.Operand(1), uint32_0_id, uint32_0_id, vec3_0_id,
                                         float32_0_id, vec3_0_id, float32_0_id});
    }

    invalid_block.CreateInstruction(spv::OpBranch, {merge_block_label});

    // move all remaining instructions to the newly created merge block
    merge_block.instructions_.insert(merge_block.instructions_.end(), std::make_move_iterator(inst_it),
                                     std::make_move_iterator(original_block.instructions_.end()));
    original_block.instructions_.erase(inst_it, original_block.instructions_.end());

    // Go back to original Block and add function call and branch from the bool result
    const uint32_t function_result = CreateFunctionCall(original_block, nullptr, injection_data);

    original_block.CreateInstruction(spv::OpSelectionMerge, {merge_block_label, spv::SelectionControlMaskNone});
    original_block.CreateInstruction(spv::OpBranchConditional, {function_result, valid_block_label, invalid_block_label});

    Reset();

    return block_it;
}

bool InjectConditionalFunctionPass::Instrument() {
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
                if (!RequiresInstrumentation(*function, *(inst_it->get()))) {
                    // TODO - This should be cleaned up then having it injected here
                    // we can have a situation where the incoming SPIR-V looks like
                    // %a = OpSampledImage %type %image %sampler
                    // ... other stuff we inject a
                    // function around
                    // %b = OpImageSampleExplicitLod %type2 %a %3893 Lod %3918
                    // and we get an error "All OpSampledImage instructions must be in the same block in which their Result <id> are
                    // consumed" to get around this we inject a OpCopyObject right after the OpSampledImage
                    if ((*inst_it)->Opcode() == spv::OpSampledImage) {
                        const uint32_t result_id = (*inst_it)->ResultId();
                        const uint32_t type_id = (*inst_it)->TypeId();
                        const uint32_t copy_id = module_.TakeNextId();
                        function->ReplaceAllUsesWith(result_id, copy_id);
                        inst_it++;
                        (*block_it)->CreateInstruction(spv::OpCopyObject, {type_id, copy_id, result_id}, &inst_it);
                        inst_it--;
                    }
                    continue;
                }

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

                block_it = InjectFunction(function.get(), block_it, inst_it, injection_data);
                // will start searching again from newly split merge block
                block_it--;
                break;
            }
        }
    }

    return instrumentations_count_ != 0;
}

}  // namespace spirv
}  // namespace gpuav