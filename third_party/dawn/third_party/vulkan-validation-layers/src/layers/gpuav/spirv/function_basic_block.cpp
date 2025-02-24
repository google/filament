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

#include "function_basic_block.h"
#include "instruction.h"
#include "module.h"

namespace gpuav {
namespace spirv {

void BasicBlock::ToBinary(std::vector<uint32_t>& out) {
    for (const auto& inst : instructions_) {
        inst->ToBinary(out);
    }
}

void Function::ToBinary(std::vector<uint32_t>& out) {
    for (const auto& inst : pre_block_inst_) {
        inst->ToBinary(out);
    }
    for (const auto& block : blocks_) {
        block->ToBinary(out);
    }
    for (const auto& inst : post_block_inst_) {
        inst->ToBinary(out);
    }
}

BasicBlock::BasicBlock(std::unique_ptr<Instruction> label, Function& function) : function_(function) {
    // Used when loading initial SPIR-V
    instructions_.emplace_back(std::move(label));  // OpLabel
}

BasicBlock::BasicBlock(Module& module, Function& function) : function_(function) {
    uint32_t new_label_id = module.TakeNextId();
    CreateInstruction(spv::OpLabel, {new_label_id});
}

uint32_t BasicBlock::GetLabelId() { return (*(instructions_[0])).ResultId(); }

InstructionIt BasicBlock::GetFirstInjectableInstrution() {
    InstructionIt inst_it;
    for (inst_it = instructions_.begin(); inst_it != instructions_.end(); ++inst_it) {
        if ((*inst_it)->Opcode() != spv::OpLabel && (*inst_it)->Opcode() != spv::OpVariable) {
            break;
        }
    }
    return inst_it;
}

InstructionIt BasicBlock::GetLastInjectableInstrution() {
    for (auto inst_it = instructions_.rbegin(); inst_it != instructions_.rend(); ++inst_it) {
        switch ((*inst_it)->Opcode()) {
            case spv::OpBranch:
            case spv::OpBranchConditional:
            case spv::OpSwitch:
            case spv::OpReturn:
            case spv::OpReturnValue:
            case spv::OpKill:
            case spv::OpUnreachable:
            case spv::OpTerminateInvocation:
                break;
            default:
                // this works because we know we are not at rend() and there MUST be at least one termination instruction
                return (inst_it).base();
        }
    }
    return instructions_.end();
}

void BasicBlock::CreateInstruction(spv::Op opcode, const std::vector<uint32_t>& words, InstructionIt* inst_it) {
    const bool add_to_end = inst_it == nullptr;
    InstructionIt last_inst = instructions_.end();
    if (add_to_end) {
        inst_it = &last_inst;
    }

    // Add 1 as we need to reserve the first word for the opcode/length
    auto new_inst = std::make_unique<Instruction>((uint32_t)(words.size() + 1), opcode);
    new_inst->Fill(words);

    const uint32_t result_id = new_inst->ResultId();
    if (result_id != 0) {
        function_.inst_map_[result_id] = new_inst.get();
    }

    InstructionIt it = instructions_.insert(*inst_it, std::move(new_inst));
    // update after insertion because allows for easy adding of multiple instructions.
    // The caller already knows the added instructions info (since it passed it in).
    if (!add_to_end) {
        *inst_it = ++it;
    }
}

Function::Function(Module& module, std::unique_ptr<Instruction> function_inst) : module_(module), instrumentation_added_(false) {
    // Used when loading initial SPIR-V
    pre_block_inst_.emplace_back(std::move(function_inst));  // OpFunction
}

BasicBlockIt Function::InsertNewBlock(BasicBlockIt it) {
    auto new_block = std::make_unique<BasicBlock>(module_, (*it)->function_);
    it++;  // make sure it inserted after
    BasicBlockIt new_block_it = blocks_.insert(it, std::move(new_block));

    return new_block_it;
}

void Function::InitBlocks(uint32_t count) {
    blocks_.reserve(blocks_.size() + count);
    for (uint32_t i = 0; i < count; i++) {
        auto new_block = std::make_unique<BasicBlock>(module_, *this);
        blocks_.emplace_back(std::move(new_block));
    }
}

const Instruction* Function::FindInstruction(uint32_t id) const {
    auto it = inst_map_.find(id);
    return (it == inst_map_.end()) ? nullptr : it->second;
}

void Function::CreateInstruction(spv::Op opcode, const std::vector<uint32_t>& words, uint32_t id) {
    for (auto& block : blocks_) {
        for (auto inst_it = block->instructions_.begin(); inst_it != block->instructions_.end(); ++inst_it) {
            if ((*inst_it)->ResultId() == id) {
                inst_it++;  // insert after
                block->CreateInstruction(opcode, words, &inst_it);
                return;
            }
        }
    }
}

// Will not touch control flow logic
void Function::ReplaceAllUsesWith(uint32_t old_word, uint32_t new_word) {
    // Shouldn't have to replace anything outside the IDs in function blocks.
    //
    // This call only needed at a Function block level when not dealing with OpVariable as other instructions won't have to worry
    // about decorations/types/etc from pre-Function blocks (such as OpEntryPoint)
    assert(FindInstruction(old_word)->Opcode() != spv::OpVariable);

    // Because the caller might still be moving around blocks, need to just search all blocks currently
    for (auto& block : blocks_) {
        for (auto& inst : block->instructions_) {
            inst->ReplaceOperandId(old_word, new_word);
        }
    }
}

}  // namespace spirv
}  // namespace gpuav