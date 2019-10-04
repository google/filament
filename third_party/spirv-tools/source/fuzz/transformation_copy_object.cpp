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

#include "source/fuzz/transformation_copy_object.h"

#include "source/fuzz/fuzzer_util.h"
#include "source/opt/instruction.h"
#include "source/util/make_unique.h"

namespace spvtools {
namespace fuzz {

TransformationCopyObject::TransformationCopyObject(
    const protobufs::TransformationCopyObject& message)
    : message_(message) {}

TransformationCopyObject::TransformationCopyObject(uint32_t object,
                                                   uint32_t base_instruction_id,
                                                   uint32_t offset,
                                                   uint32_t fresh_id) {
  message_.set_object(object);
  message_.set_base_instruction_id(base_instruction_id);
  message_.set_offset(offset);
  message_.set_fresh_id(fresh_id);
}

bool TransformationCopyObject::IsApplicable(
    opt::IRContext* context, const FactManager& /*fact_manager*/) const {
  if (!fuzzerutil::IsFreshId(context, message_.fresh_id())) {
    // We require the id for the object copy to be unused.
    return false;
  }
  // The id of the object to be copied must exist
  auto object_inst = context->get_def_use_mgr()->GetDef(message_.object());
  if (!object_inst) {
    return false;
  }
  if (!IsCopyable(context, object_inst)) {
    return false;
  }

  auto base_instruction =
      context->get_def_use_mgr()->GetDef(message_.base_instruction_id());
  if (!base_instruction) {
    // The given id to insert after is not defined.
    return false;
  }

  auto destination_block = context->get_instr_block(base_instruction);
  if (!destination_block) {
    // The given id to insert after is not in a block.
    return false;
  }

  auto insert_before = fuzzerutil::GetIteratorForBaseInstructionAndOffset(
      destination_block, base_instruction, message_.offset());

  if (insert_before == destination_block->end()) {
    // The offset was inappropriate.
    return false;
  }

  if (!CanInsertCopyBefore(insert_before)) {
    return false;
  }

  // |message_object| must be available at the point where we want to add the
  // copy. It is available if it is at global scope (in which case it has no
  // block), or if it dominates the point of insertion but is different from the
  // point of insertion.
  //
  // The reason why the object needs to be different from the insertion point is
  // that the copy will be added *before* this point, and we do not want to
  // insert it before the object's defining instruction.
  return !context->get_instr_block(object_inst) ||
         (object_inst != &*insert_before &&
          context->GetDominatorAnalysis(destination_block->GetParent())
              ->Dominates(object_inst, &*insert_before));
}

void TransformationCopyObject::Apply(opt::IRContext* context,
                                     FactManager* fact_manager) const {
  // - A new instruction,
  //     %|message_.fresh_id| = OpCopyObject %ty %|message_.object|
  //   is added directly before the instruction at |message_.insert_after_id| +
  //   |message_|.offset, where %ty is the type of |message_.object|.
  // - The fact that |message_.fresh_id| and |message_.object| are synonyms
  //   is added to the fact manager.
  // The id of the object to be copied must exist
  auto object_inst = context->get_def_use_mgr()->GetDef(message_.object());
  assert(object_inst && "The object to be copied must exist.");
  auto base_instruction =
      context->get_def_use_mgr()->GetDef(message_.base_instruction_id());
  assert(base_instruction && "The base instruction must exist.");
  auto destination_block = context->get_instr_block(base_instruction);
  assert(destination_block && "The base instruction must be in a block.");
  auto insert_before = fuzzerutil::GetIteratorForBaseInstructionAndOffset(
      destination_block, base_instruction, message_.offset());
  assert(insert_before != destination_block->end() &&
         "There must be an instruction before which the copy can be inserted.");

  opt::Instruction::OperandList operands = {
      {SPV_OPERAND_TYPE_ID, {message_.object()}}};
  insert_before->InsertBefore(MakeUnique<opt::Instruction>(
      context, SpvOp::SpvOpCopyObject, object_inst->type_id(),
      message_.fresh_id(), operands));

  fuzzerutil::UpdateModuleIdBound(context, message_.fresh_id());
  context->InvalidateAnalysesExceptFor(opt::IRContext::Analysis::kAnalysisNone);

  protobufs::Fact fact;
  fact.mutable_id_synonym_fact()->set_id(message_.object());
  fact.mutable_id_synonym_fact()->mutable_data_descriptor()->set_object(
      message_.fresh_id());
  fact_manager->AddFact(fact, context);
}

protobufs::Transformation TransformationCopyObject::ToMessage() const {
  protobufs::Transformation result;
  *result.mutable_copy_object() = message_;
  return result;
}

bool TransformationCopyObject::IsCopyable(opt::IRContext* ir_context,
                                          opt::Instruction* inst) {
  if (!inst->HasResultId()) {
    // We can only apply OpCopyObject to instructions that generate ids.
    return false;
  }
  if (!inst->type_id()) {
    // We can only apply OpCopyObject to instructions that have types.
    return false;
  }
  // We do not copy objects that have decorations: if the copy is not
  // decorated analogously, using the original object vs. its copy may not be
  // equivalent.
  // TODO(afd): it would be possible to make the copy but not add an id
  // synonym.
  return ir_context->get_decoration_mgr()
      ->GetDecorationsFor(inst->result_id(), true)
      .empty();
}

bool TransformationCopyObject::CanInsertCopyBefore(
    const opt::BasicBlock::iterator& instruction_in_block) {
  if (instruction_in_block->PreviousNode() &&
      (instruction_in_block->PreviousNode()->opcode() == SpvOpLoopMerge ||
       instruction_in_block->PreviousNode()->opcode() == SpvOpSelectionMerge)) {
    // We cannot insert a copy directly after a merge instruction.
    return false;
  }
  if (instruction_in_block->opcode() == SpvOpVariable) {
    // We cannot insert a copy directly before a variable; variables in a
    // function must be contiguous in the entry block.
    return false;
  }
  // We cannot insert a copy directly before OpPhi, because OpPhi instructions
  // need to be contiguous at the start of a block.
  return instruction_in_block->opcode() != SpvOpPhi;
}

}  // namespace fuzz
}  // namespace spvtools
