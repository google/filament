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

#include "source/fuzz/fuzzer_pass_copy_objects.h"

#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/transformation_copy_object.h"

namespace spvtools {
namespace fuzz {

FuzzerPassCopyObjects::FuzzerPassCopyObjects(
    opt::IRContext* ir_context, FactManager* fact_manager,
    FuzzerContext* fuzzer_context,
    protobufs::TransformationSequence* transformations)
    : FuzzerPass(ir_context, fact_manager, fuzzer_context, transformations) {}

FuzzerPassCopyObjects::~FuzzerPassCopyObjects() = default;

void FuzzerPassCopyObjects::Apply() {
  MaybeAddTransformationBeforeEachInstruction(
      [this](const opt::Function& function, opt::BasicBlock* block,
             opt::BasicBlock::iterator inst_it,
             const protobufs::InstructionDescriptor& instruction_descriptor)
          -> void {
        assert(inst_it->opcode() ==
                   instruction_descriptor.target_instruction_opcode() &&
               "The opcode of the instruction we might insert before must be "
               "the same as the opcode in the descriptor for the instruction");

        // Check whether it is legitimate to insert a copy before this
        // instruction.
        if (!fuzzerutil::CanInsertOpcodeBeforeInstruction(SpvOpCopyObject,
                                                          inst_it)) {
          return;
        }

        // Randomly decide whether to try inserting an object copy here.
        if (!GetFuzzerContext()->ChoosePercentage(
                GetFuzzerContext()->GetChanceOfCopyingObject())) {
          return;
        }

        std::vector<opt::Instruction*> relevant_instructions =
            FindAvailableInstructions(function, block, inst_it,
                                      fuzzerutil::CanMakeSynonymOf);

        // At this point, |relevant_instructions| contains all the instructions
        // we might think of copying.
        if (relevant_instructions.empty()) {
          return;
        }

        // Choose a copyable instruction at random, and create and apply an
        // object copying transformation based on it.
        uint32_t index = GetFuzzerContext()->RandomIndex(relevant_instructions);
        TransformationCopyObject transformation(
            relevant_instructions[index]->result_id(), instruction_descriptor,
            GetFuzzerContext()->GetFreshId());
        assert(transformation.IsApplicable(GetIRContext(), *GetFactManager()) &&
               "This transformation should be applicable by construction.");
        transformation.Apply(GetIRContext(), GetFactManager());
        *GetTransformations()->add_transformation() =
            transformation.ToMessage();
      });
}

}  // namespace fuzz
}  // namespace spvtools
