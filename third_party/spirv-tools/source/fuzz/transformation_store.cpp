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

#include "source/fuzz/transformation_store.h"

#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"

namespace spvtools {
namespace fuzz {

TransformationStore::TransformationStore(
    const spvtools::fuzz::protobufs::TransformationStore& message)
    : message_(message) {}

TransformationStore::TransformationStore(
    uint32_t pointer_id, uint32_t value_id,
    const protobufs::InstructionDescriptor& instruction_to_insert_before) {
  message_.set_pointer_id(pointer_id);
  message_.set_value_id(value_id);
  *message_.mutable_instruction_to_insert_before() =
      instruction_to_insert_before;
}

bool TransformationStore::IsApplicable(
    opt::IRContext* ir_context,
    const TransformationContext& transformation_context) const {
  // The pointer must exist and have a type.
  auto pointer = ir_context->get_def_use_mgr()->GetDef(message_.pointer_id());
  if (!pointer || !pointer->type_id()) {
    return false;
  }

  // The pointer type must indeed be a pointer.
  auto pointer_type = ir_context->get_def_use_mgr()->GetDef(pointer->type_id());
  assert(pointer_type && "Type id must be defined.");
  if (pointer_type->opcode() != SpvOpTypePointer) {
    return false;
  }

  // The pointer must not be read only.
  if (pointer->IsReadOnlyPointer()) {
    return false;
  }

  // We do not want to allow storing to null or undefined pointers.
  switch (pointer->opcode()) {
    case SpvOpConstantNull:
    case SpvOpUndef:
      return false;
    default:
      break;
  }

  // Determine which instruction we should be inserting before.
  auto insert_before =
      FindInstruction(message_.instruction_to_insert_before(), ir_context);
  // It must exist, ...
  if (!insert_before) {
    return false;
  }
  // ... and it must be legitimate to insert a store before it.
  if (!fuzzerutil::CanInsertOpcodeBeforeInstruction(SpvOpStore,
                                                    insert_before)) {
    return false;
  }

  // The block we are inserting into needs to be dead, or else the pointee type
  // of the pointer we are storing to needs to be irrelevant (otherwise the
  // store could impact on the observable behaviour of the module).
  if (!transformation_context.GetFactManager()->BlockIsDead(
          ir_context->get_instr_block(insert_before)->id()) &&
      !transformation_context.GetFactManager()->PointeeValueIsIrrelevant(
          message_.pointer_id())) {
    return false;
  }

  // The value being stored needs to exist and have a type.
  auto value = ir_context->get_def_use_mgr()->GetDef(message_.value_id());
  if (!value || !value->type_id()) {
    return false;
  }

  // The type of the value must match the pointee type.
  if (pointer_type->GetSingleWordInOperand(1) != value->type_id()) {
    return false;
  }

  // The pointer needs to be available at the insertion point.
  if (!fuzzerutil::IdIsAvailableBeforeInstruction(ir_context, insert_before,
                                                  message_.pointer_id())) {
    return false;
  }

  // The value needs to be available at the insertion point.
  return fuzzerutil::IdIsAvailableBeforeInstruction(ir_context, insert_before,
                                                    message_.value_id());
}

void TransformationStore::Apply(opt::IRContext* ir_context,
                                TransformationContext* /*unused*/) const {
  FindInstruction(message_.instruction_to_insert_before(), ir_context)
      ->InsertBefore(MakeUnique<opt::Instruction>(
          ir_context, SpvOpStore, 0, 0,
          opt::Instruction::OperandList(
              {{SPV_OPERAND_TYPE_ID, {message_.pointer_id()}},
               {SPV_OPERAND_TYPE_ID, {message_.value_id()}}})));
  ir_context->InvalidateAnalysesExceptFor(opt::IRContext::kAnalysisNone);
}

protobufs::Transformation TransformationStore::ToMessage() const {
  protobufs::Transformation result;
  *result.mutable_store() = message_;
  return result;
}

std::unordered_set<uint32_t> TransformationStore::GetFreshIds() const {
  return std::unordered_set<uint32_t>();
}

}  // namespace fuzz
}  // namespace spvtools
