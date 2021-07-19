// Copyright (c) 2020 Google LLC
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

#include "source/fuzz/fuzzer_pass_add_loads.h"

#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/transformation_load.h"

namespace spvtools {
namespace fuzz {

FuzzerPassAddLoads::FuzzerPassAddLoads(
    opt::IRContext* ir_context, TransformationContext* transformation_context,
    FuzzerContext* fuzzer_context,
    protobufs::TransformationSequence* transformations)
    : FuzzerPass(ir_context, transformation_context, fuzzer_context,
                 transformations) {}

void FuzzerPassAddLoads::Apply() {
  ForEachInstructionWithInstructionDescriptor(
      [this](opt::Function* function, opt::BasicBlock* block,
             opt::BasicBlock::iterator inst_it,
             const protobufs::InstructionDescriptor& instruction_descriptor)
          -> void {
        assert(inst_it->opcode() ==
                   instruction_descriptor.target_instruction_opcode() &&
               "The opcode of the instruction we might insert before must be "
               "the same as the opcode in the descriptor for the instruction");

        // Check whether it is legitimate to insert a load before this
        // instruction.
        if (!fuzzerutil::CanInsertOpcodeBeforeInstruction(SpvOpLoad, inst_it)) {
          return;
        }

        // Randomly decide whether to try inserting a load here.
        if (!GetFuzzerContext()->ChoosePercentage(
                GetFuzzerContext()->GetChanceOfAddingLoad())) {
          return;
        }

        std::vector<opt::Instruction*> relevant_instructions =
            FindAvailableInstructions(
                function, block, inst_it,
                [](opt::IRContext* context,
                   opt::Instruction* instruction) -> bool {
                  if (!instruction->result_id() || !instruction->type_id()) {
                    return false;
                  }
                  switch (instruction->opcode()) {
                    case SpvOpConstantNull:
                    case SpvOpUndef:
                      // Do not allow loading from a null or undefined pointer;
                      // this might be OK if the block is dead, but for now we
                      // conservatively avoid it.
                      return false;
                    default:
                      break;
                  }
                  return context->get_def_use_mgr()
                             ->GetDef(instruction->type_id())
                             ->opcode() == SpvOpTypePointer;
                });

        // At this point, |relevant_instructions| contains all the pointers
        // we might think of loading from.
        if (relevant_instructions.empty()) {
          return;
        }

        // Choose a pointer at random, and create and apply a loading
        // transformation based on it.
        ApplyTransformation(TransformationLoad(
            GetFuzzerContext()->GetFreshId(),
            relevant_instructions[GetFuzzerContext()->RandomIndex(
                                      relevant_instructions)]
                ->result_id(),
            instruction_descriptor));
      });
}

}  // namespace fuzz
}  // namespace spvtools
