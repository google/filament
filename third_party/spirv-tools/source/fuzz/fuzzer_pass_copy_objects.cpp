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
  // Consider every block in every function.
  for (auto& function : *GetIRContext()->module()) {
    for (auto& block : function) {
      // We now consider every instruction in the block, randomly deciding
      // whether to add an object copy before the instruction.

      // In order to insert an object copy instruction, we need to be able to
      // identify the instruction a copy should be inserted before.  We do this
      // by tracking a base instruction, which must generate a result id, and an
      // offset (to allow us to identify instructions that do not generate
      // result ids).

      // The initial base instruction is the block label.
      uint32_t base = block.id();
      uint32_t offset = 0;
      // Consider every instruction in the block.
      for (auto inst_it = block.begin(); inst_it != block.end(); ++inst_it) {
        if (inst_it->HasResultId()) {
          // In the case that the instruction has a result id, we use the
          // instruction as its own base, with zero offset.
          base = inst_it->result_id();
          offset = 0;
        } else {
          // The instruction does not have a result id, so we need to identify
          // it via the latest instruction that did have a result id (base), and
          // an incremented offset.
          offset++;
        }

        // Check whether it is legitimate to insert a copy before this
        // instruction.
        if (!TransformationCopyObject::CanInsertCopyBefore(inst_it)) {
          continue;
        }

        // Randomly decide whether to try inserting an object copy here.
        if (!GetFuzzerContext()->ChoosePercentage(
                GetFuzzerContext()->GetChanceOfCopyingObject())) {
          continue;
        }

        // Populate list of potential instructions that can be copied.
        // TODO(afd) The following is (relatively) simple, but may end up being
        //  prohibitively inefficient, as it walks the whole dominator tree for
        //  every copy that is added.
        std::vector<opt::Instruction*> copyable_instructions;

        // Consider all global declarations
        for (auto& global : GetIRContext()->module()->types_values()) {
          if (TransformationCopyObject::IsCopyable(GetIRContext(), &global)) {
            copyable_instructions.push_back(&global);
          }
        }

        // Consider all previous instructions in this block
        for (auto prev_inst_it = block.begin(); prev_inst_it != inst_it;
             ++prev_inst_it) {
          if (TransformationCopyObject::IsCopyable(GetIRContext(),
                                                   &*prev_inst_it)) {
            copyable_instructions.push_back(&*prev_inst_it);
          }
        }

        // Walk the dominator tree to consider all instructions from dominating
        // blocks
        auto dominator_analysis =
            GetIRContext()->GetDominatorAnalysis(&function);
        for (auto next_dominator =
                 dominator_analysis->ImmediateDominator(&block);
             next_dominator != nullptr;
             next_dominator =
                 dominator_analysis->ImmediateDominator(next_dominator)) {
          for (auto& dominating_inst : *next_dominator) {
            if (TransformationCopyObject::IsCopyable(GetIRContext(),
                                                     &dominating_inst)) {
              copyable_instructions.push_back(&dominating_inst);
            }
          }
        }

        // At this point, |copyable_instructions| contains all the instructions
        // we might think of copying.

        if (!copyable_instructions.empty()) {
          // Choose a copyable instruction at random, and create and apply an
          // object copying transformation based on it.
          uint32_t index =
              GetFuzzerContext()->RandomIndex(copyable_instructions);
          TransformationCopyObject transformation(
              copyable_instructions[index]->result_id(), base, offset,
              GetFuzzerContext()->GetFreshId());
          assert(
              transformation.IsApplicable(GetIRContext(), *GetFactManager()) &&
              "This transformation should be applicable by construction.");
          transformation.Apply(GetIRContext(), GetFactManager());
          *GetTransformations()->add_transformation() =
              transformation.ToMessage();

          if (!inst_it->HasResultId()) {
            // We have inserted a new instruction before the current
            // instruction, and we are tracking the current id-less instruction
            // via an offset (offset) from a previous instruction (base) that
            // has an id. We increment |offset| to reflect the newly-inserted
            // instruction.
            //
            // This is slightly preferable to the alternative of setting |base|
            // to be the result id of the new instruction, since on replay we
            // might end up eliminating this copy but keeping a subsequent copy.
            offset++;
          }
        }
      }
    }
  }
}

}  // namespace fuzz
}  // namespace spvtools
