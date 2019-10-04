// Copyright (c) 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "source/fuzz/id_use_descriptor.h"

namespace spvtools {
namespace fuzz {

opt::Instruction* transformation::FindInstruction(
    const protobufs::IdUseDescriptor& descriptor,
    spvtools::opt::IRContext* context) {
  for (auto& function : *context->module()) {
    for (auto& block : function) {
      bool found_base = block.id() == descriptor.base_instruction_result_id();
      uint32_t num_ignored = 0;
      for (auto& instruction : block) {
        if (instruction.HasResultId() &&
            instruction.result_id() ==
                descriptor.base_instruction_result_id()) {
          assert(!found_base &&
                 "It should not be possible to find the base instruction "
                 "multiple times.");
          found_base = true;
          assert(num_ignored == 0 &&
                 "The skipped instruction count should only be incremented "
                 "after the instruction base has been found.");
        }
        if (found_base &&
            instruction.opcode() == descriptor.target_instruction_opcode()) {
          if (num_ignored == descriptor.num_opcodes_to_ignore()) {
            if (descriptor.in_operand_index() >= instruction.NumInOperands()) {
              return nullptr;
            }
            auto in_operand =
                instruction.GetInOperand(descriptor.in_operand_index());
            if (in_operand.type != SPV_OPERAND_TYPE_ID) {
              return nullptr;
            }
            if (in_operand.words[0] != descriptor.id_of_interest()) {
              return nullptr;
            }
            return &instruction;
          }
          num_ignored++;
        }
      }
      if (found_base) {
        // We found the base instruction, but did not find the target
        // instruction in the same block.
        return nullptr;
      }
    }
  }
  return nullptr;
}

protobufs::IdUseDescriptor transformation::MakeIdUseDescriptor(
    uint32_t id_of_interest, SpvOp target_instruction_opcode,
    uint32_t in_operand_index, uint32_t base_instruction_result_id,
    uint32_t num_opcodes_to_ignore) {
  protobufs::IdUseDescriptor result;
  result.set_id_of_interest(id_of_interest);
  result.set_target_instruction_opcode(target_instruction_opcode);
  result.set_in_operand_index(in_operand_index);
  result.set_base_instruction_result_id(base_instruction_result_id);
  result.set_num_opcodes_to_ignore(num_opcodes_to_ignore);
  return result;
}

protobufs::IdUseDescriptor transformation::MakeIdUseDescriptorFromUse(
    opt::IRContext* context, opt::Instruction* inst,
    uint32_t in_operand_index) {
  auto in_operand = inst->GetInOperand(in_operand_index);
  assert(in_operand.type == SPV_OPERAND_TYPE_ID);
  auto id_of_interest = in_operand.words[0];

  auto block = context->get_instr_block(inst);
  uint32_t base_instruction_result_id = block->id();
  uint32_t num_opcodes_to_ignore = 0;
  for (auto& inst_in_block : *block) {
    if (inst_in_block.HasResultId()) {
      base_instruction_result_id = inst_in_block.result_id();
      num_opcodes_to_ignore = 0;
    }
    if (&inst_in_block == inst) {
      return MakeIdUseDescriptor(id_of_interest, inst->opcode(),
                                 in_operand_index, base_instruction_result_id,
                                 num_opcodes_to_ignore);
    }
    if (inst_in_block.opcode() == inst->opcode()) {
      num_opcodes_to_ignore++;
    }
  }
  assert(false && "No matching instruction was found.");
  return protobufs::IdUseDescriptor();
}

}  // namespace fuzz
}  // namespace spvtools
