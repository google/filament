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

#include "source/fuzz/transformation_split_block.h"

#include <utility>

#include "source/fuzz/fuzzer_util.h"
#include "source/util/make_unique.h"

namespace spvtools {
namespace fuzz {

TransformationSplitBlock::TransformationSplitBlock(
    const spvtools::fuzz::protobufs::TransformationSplitBlock& message)
    : message_(message) {}

TransformationSplitBlock::TransformationSplitBlock(uint32_t base_instruction_id,
                                                   uint32_t offset,
                                                   uint32_t fresh_id) {
  message_.set_base_instruction_id(base_instruction_id);
  message_.set_offset(offset);
  message_.set_fresh_id(fresh_id);
}

bool TransformationSplitBlock::IsApplicable(
    opt::IRContext* context, const FactManager& /*unused*/) const {
  if (!fuzzerutil::IsFreshId(context, message_.fresh_id())) {
    // We require the id for the new block to be unused.
    return false;
  }
  auto base_instruction =
      context->get_def_use_mgr()->GetDef(message_.base_instruction_id());
  if (!base_instruction) {
    // The instruction describing the block we should split does not exist.
    return false;
  }
  auto block_containing_base_instruction =
      context->get_instr_block(base_instruction);
  if (!block_containing_base_instruction) {
    // The instruction describing the block we should split is not contained in
    // a block.
    return false;
  }

  if (block_containing_base_instruction->IsLoopHeader()) {
    // We cannot split a loop header block: back-edges would become invalid.
    return false;
  }

  auto split_before = fuzzerutil::GetIteratorForBaseInstructionAndOffset(
      block_containing_base_instruction, base_instruction, message_.offset());
  if (split_before == block_containing_base_instruction->end()) {
    // The offset was inappropriate.
    return false;
  }
  if (split_before->PreviousNode() &&
      split_before->PreviousNode()->opcode() == SpvOpSelectionMerge) {
    // We cannot split directly after a selection merge: this would separate
    // the merge from its associated branch or switch operation.
    return false;
  }
  if (split_before->opcode() == SpvOpVariable) {
    // We cannot split directly after a variable; variables in a function
    // must be contiguous in the entry block.
    return false;
  }
  // We cannot split before an OpPhi unless the OpPhi has exactly one
  // associated incoming edge.
  return !(split_before->opcode() == SpvOpPhi &&
           split_before->NumInOperands() != 2);
}

void TransformationSplitBlock::Apply(opt::IRContext* context,
                                     FactManager* /*unused*/) const {
  auto base_instruction =
      context->get_def_use_mgr()->GetDef(message_.base_instruction_id());
  assert(base_instruction && "Base instruction must exist");
  auto block_containing_base_instruction =
      context->get_instr_block(base_instruction);
  assert(block_containing_base_instruction &&
         "Base instruction must be in a block");
  auto split_before = fuzzerutil::GetIteratorForBaseInstructionAndOffset(
      block_containing_base_instruction, base_instruction, message_.offset());
  assert(split_before != block_containing_base_instruction->end() &&
         "If the transformation is applicable, we should have an "
         "instruction to split on.");
  // We need to make sure the module's id bound is large enough to add the
  // fresh id.
  fuzzerutil::UpdateModuleIdBound(context, message_.fresh_id());
  // Split the block.
  auto new_bb = block_containing_base_instruction->SplitBasicBlock(
      context, message_.fresh_id(), split_before);
  // The split does not automatically add a branch between the two parts of
  // the original block, so we add one.
  block_containing_base_instruction->AddInstruction(
      MakeUnique<opt::Instruction>(
          context, SpvOpBranch, 0, 0,
          std::initializer_list<opt::Operand>{
              opt::Operand(spv_operand_type_t::SPV_OPERAND_TYPE_ID,
                           {message_.fresh_id()})}));
  // If we split before OpPhi instructions, we need to update their
  // predecessor operand so that the block they used to be inside is now the
  // predecessor.
  new_bb->ForEachPhiInst(
      [block_containing_base_instruction](opt::Instruction* phi_inst) {
        // The following assertion is a sanity check.  It is guaranteed to hold
        // if IsApplicable holds.
        assert(phi_inst->NumInOperands() == 2 &&
               "We can only split a block before an OpPhi if block has exactly "
               "one predecessor.");
        phi_inst->SetInOperand(1, {block_containing_base_instruction->id()});
      });
  // Invalidate all analyses
  context->InvalidateAnalysesExceptFor(opt::IRContext::Analysis::kAnalysisNone);
}

protobufs::Transformation TransformationSplitBlock::ToMessage() const {
  protobufs::Transformation result;
  *result.mutable_split_block() = message_;
  return result;
}

}  // namespace fuzz
}  // namespace spvtools
