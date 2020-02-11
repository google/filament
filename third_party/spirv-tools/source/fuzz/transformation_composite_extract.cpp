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

#include "source/fuzz/transformation_composite_extract.h"

#include <vector>

#include "source/fuzz/data_descriptor.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"

namespace spvtools {
namespace fuzz {

TransformationCompositeExtract::TransformationCompositeExtract(
    const spvtools::fuzz::protobufs::TransformationCompositeExtract& message)
    : message_(message) {}

TransformationCompositeExtract::TransformationCompositeExtract(
    const protobufs::InstructionDescriptor& instruction_to_insert_before,
    uint32_t fresh_id, uint32_t composite_id, std::vector<uint32_t>&& index) {
  *message_.mutable_instruction_to_insert_before() =
      instruction_to_insert_before;
  message_.set_fresh_id(fresh_id);
  message_.set_composite_id(composite_id);
  for (auto an_index : index) {
    message_.add_index(an_index);
  }
}

bool TransformationCompositeExtract::IsApplicable(
    opt::IRContext* context,
    const spvtools::fuzz::FactManager& /*unused*/) const {
  if (!fuzzerutil::IsFreshId(context, message_.fresh_id())) {
    return false;
  }
  auto instruction_to_insert_before =
      FindInstruction(message_.instruction_to_insert_before(), context);
  if (!instruction_to_insert_before) {
    return false;
  }
  auto composite_instruction =
      context->get_def_use_mgr()->GetDef(message_.composite_id());
  if (!composite_instruction) {
    return false;
  }
  if (auto block = context->get_instr_block(composite_instruction)) {
    if (composite_instruction == instruction_to_insert_before ||
        !context->GetDominatorAnalysis(block->GetParent())
             ->Dominates(composite_instruction, instruction_to_insert_before)) {
      return false;
    }
  }
  assert(composite_instruction->type_id() &&
         "An instruction in a block cannot have a result id but no type id.");

  auto composite_type =
      context->get_type_mgr()->GetType(composite_instruction->type_id());
  if (!composite_type) {
    return false;
  }

  if (!fuzzerutil::CanInsertOpcodeBeforeInstruction(
          SpvOpCompositeExtract, instruction_to_insert_before)) {
    return false;
  }

  return fuzzerutil::WalkCompositeTypeIndices(
             context, composite_instruction->type_id(), message_.index()) != 0;
}

void TransformationCompositeExtract::Apply(
    opt::IRContext* context, spvtools::fuzz::FactManager* fact_manager) const {
  opt::Instruction::OperandList extract_operands;
  extract_operands.push_back({SPV_OPERAND_TYPE_ID, {message_.composite_id()}});
  for (auto an_index : message_.index()) {
    extract_operands.push_back({SPV_OPERAND_TYPE_LITERAL_INTEGER, {an_index}});
  }
  auto composite_instruction =
      context->get_def_use_mgr()->GetDef(message_.composite_id());
  auto extracted_type = fuzzerutil::WalkCompositeTypeIndices(
      context, composite_instruction->type_id(), message_.index());

  FindInstruction(message_.instruction_to_insert_before(), context)
      ->InsertBefore(MakeUnique<opt::Instruction>(
          context, SpvOpCompositeExtract, extracted_type, message_.fresh_id(),
          extract_operands));

  fuzzerutil::UpdateModuleIdBound(context, message_.fresh_id());

  context->InvalidateAnalysesExceptFor(opt::IRContext::Analysis::kAnalysisNone);

  // Add the fact that the id storing the extracted element is synonymous with
  // the index into the structure.
  std::vector<uint32_t> indices;
  for (auto an_index : message_.index()) {
    indices.push_back(an_index);
  }
  protobufs::DataDescriptor data_descriptor_for_extracted_element =
      MakeDataDescriptor(message_.composite_id(), std::move(indices));
  protobufs::DataDescriptor data_descriptor_for_result_id =
      MakeDataDescriptor(message_.fresh_id(), {});
  fact_manager->AddFactDataSynonym(data_descriptor_for_extracted_element,
                                   data_descriptor_for_result_id, context);
}

protobufs::Transformation TransformationCompositeExtract::ToMessage() const {
  protobufs::Transformation result;
  *result.mutable_composite_extract() = message_;
  return result;
}

}  // namespace fuzz
}  // namespace spvtools
