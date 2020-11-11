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

#include "source/fuzz/fuzzer_pass_add_stores.h"

#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/transformation_store.h"

namespace spvtools {
namespace fuzz {

FuzzerPassAddStores::FuzzerPassAddStores(
    opt::IRContext* ir_context, TransformationContext* transformation_context,
    FuzzerContext* fuzzer_context,
    protobufs::TransformationSequence* transformations)
    : FuzzerPass(ir_context, transformation_context, fuzzer_context,
                 transformations) {}

FuzzerPassAddStores::~FuzzerPassAddStores() = default;

void FuzzerPassAddStores::Apply() {
  ForEachInstructionWithInstructionDescriptor(
      [this](opt::Function* function, opt::BasicBlock* block,
             opt::BasicBlock::iterator inst_it,
             const protobufs::InstructionDescriptor& instruction_descriptor)
          -> void {
        assert(inst_it->opcode() ==
                   instruction_descriptor.target_instruction_opcode() &&
               "The opcode of the instruction we might insert before must be "
               "the same as the opcode in the descriptor for the instruction");

        // Check whether it is legitimate to insert a store before this
        // instruction.
        if (!fuzzerutil::CanInsertOpcodeBeforeInstruction(SpvOpStore,
                                                          inst_it)) {
          return;
        }

        // Randomly decide whether to try inserting a store here.
        if (!GetFuzzerContext()->ChoosePercentage(
                GetFuzzerContext()->GetChanceOfAddingStore())) {
          return;
        }

        // Look for pointers we might consider storing to.
        std::vector<opt::Instruction*> relevant_pointers =
            FindAvailableInstructions(
                function, block, inst_it,
                [this, block](opt::IRContext* context,
                              opt::Instruction* instruction) -> bool {
                  if (!instruction->result_id() || !instruction->type_id()) {
                    return false;
                  }
                  auto type_inst = context->get_def_use_mgr()->GetDef(
                      instruction->type_id());
                  if (type_inst->opcode() != SpvOpTypePointer) {
                    // Not a pointer.
                    return false;
                  }
                  if (instruction->IsReadOnlyPointer()) {
                    // Read only: cannot store to it.
                    return false;
                  }
                  switch (instruction->opcode()) {
                    case SpvOpConstantNull:
                    case SpvOpUndef:
                      // Do not allow storing to a null or undefined pointer;
                      // this might be OK if the block is dead, but for now we
                      // conservatively avoid it.
                      return false;
                    default:
                      break;
                  }
                  return GetTransformationContext()
                             ->GetFactManager()
                             ->BlockIsDead(block->id()) ||
                         GetTransformationContext()
                             ->GetFactManager()
                             ->PointeeValueIsIrrelevant(
                                 instruction->result_id());
                });

        // At this point, |relevant_pointers| contains all the pointers we might
        // think of storing to.
        if (relevant_pointers.empty()) {
          return;
        }

        auto pointer = relevant_pointers[GetFuzzerContext()->RandomIndex(
            relevant_pointers)];

        std::vector<opt::Instruction*> relevant_values =
            FindAvailableInstructions(
                function, block, inst_it,
                [pointer](opt::IRContext* context,
                          opt::Instruction* instruction) -> bool {
                  if (!instruction->result_id() || !instruction->type_id()) {
                    return false;
                  }
                  return instruction->type_id() ==
                         context->get_def_use_mgr()
                             ->GetDef(pointer->type_id())
                             ->GetSingleWordInOperand(1);
                });

        if (relevant_values.empty()) {
          return;
        }

        // Choose a value at random, and create and apply a storing
        // transformation based on it and the pointer.
        ApplyTransformation(TransformationStore(
            pointer->result_id(),
            relevant_values[GetFuzzerContext()->RandomIndex(relevant_values)]
                ->result_id(),
            instruction_descriptor));
      });
}

}  // namespace fuzz
}  // namespace spvtools
