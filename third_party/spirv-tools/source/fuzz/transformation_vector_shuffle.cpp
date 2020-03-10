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

#include "source/fuzz/transformation_vector_shuffle.h"

#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"

namespace spvtools {
namespace fuzz {

TransformationVectorShuffle::TransformationVectorShuffle(
    const spvtools::fuzz::protobufs::TransformationVectorShuffle& message)
    : message_(message) {}

TransformationVectorShuffle::TransformationVectorShuffle(
    const protobufs::InstructionDescriptor& instruction_to_insert_before,
    uint32_t fresh_id, uint32_t vector1, uint32_t vector2,
    const std::vector<uint32_t>& component) {
  *message_.mutable_instruction_to_insert_before() =
      instruction_to_insert_before;
  message_.set_fresh_id(fresh_id);
  message_.set_vector1(vector1);
  message_.set_vector2(vector2);
  for (auto a_component : component) {
    message_.add_component(a_component);
  }
}

bool TransformationVectorShuffle::IsApplicable(
    opt::IRContext* context,
    const spvtools::fuzz::FactManager& /*unused*/) const {
  // The fresh id must not already be in use.
  if (!fuzzerutil::IsFreshId(context, message_.fresh_id())) {
    return false;
  }
  // The instruction before which the shuffle will be inserted must exist.
  auto instruction_to_insert_before =
      FindInstruction(message_.instruction_to_insert_before(), context);
  if (!instruction_to_insert_before) {
    return false;
  }
  // The first vector must be an instruction with a type id
  auto vector1_instruction =
      context->get_def_use_mgr()->GetDef(message_.vector1());
  if (!vector1_instruction || !vector1_instruction->type_id()) {
    return false;
  }
  // The second vector must be an instruction with a type id
  auto vector2_instruction =
      context->get_def_use_mgr()->GetDef(message_.vector2());
  if (!vector2_instruction || !vector2_instruction->type_id()) {
    return false;
  }
  auto vector1_type =
      context->get_type_mgr()->GetType(vector1_instruction->type_id());
  // The first vector instruction's type must actually be a vector type.
  if (!vector1_type->AsVector()) {
    return false;
  }
  auto vector2_type =
      context->get_type_mgr()->GetType(vector2_instruction->type_id());
  // The second vector instruction's type must actually be a vector type.
  if (!vector2_type->AsVector()) {
    return false;
  }
  // The element types of the vectors must be the same.
  if (vector1_type->AsVector()->element_type() !=
      vector2_type->AsVector()->element_type()) {
    return false;
  }
  uint32_t combined_size = vector1_type->AsVector()->element_count() +
                           vector2_type->AsVector()->element_count();
  for (auto a_compoment : message_.component()) {
    // 0xFFFFFFFF is used to represent an undefined component.  Unless
    // undefined, a component must be less than the combined size of the
    // vectors.
    if (a_compoment != 0xFFFFFFFF && a_compoment >= combined_size) {
      return false;
    }
  }
  // The module must already declare an appropriate type in which to store the
  // result of the shuffle.
  if (!GetResultTypeId(context, *vector1_type->AsVector()->element_type())) {
    return false;
  }
  // Each of the vectors used in the shuffle must be available at the insertion
  // point.
  for (auto used_instruction : {vector1_instruction, vector2_instruction}) {
    if (auto block = context->get_instr_block(used_instruction)) {
      if (!context->GetDominatorAnalysis(block->GetParent())
               ->Dominates(used_instruction, instruction_to_insert_before)) {
        return false;
      }
    }
  }

  // It must be legitimate to insert an OpVectorShuffle before the identified
  // instruction.
  return fuzzerutil::CanInsertOpcodeBeforeInstruction(
      SpvOpVectorShuffle, instruction_to_insert_before);
}

void TransformationVectorShuffle::Apply(
    opt::IRContext* context, spvtools::fuzz::FactManager* fact_manager) const {
  // Make input operands for a shuffle instruction - these comprise the two
  // vectors being shuffled, followed by the integer literal components.
  opt::Instruction::OperandList shuffle_operands = {
      {SPV_OPERAND_TYPE_ID, {message_.vector1()}},
      {SPV_OPERAND_TYPE_ID, {message_.vector2()}}};
  for (auto a_component : message_.component()) {
    shuffle_operands.push_back(
        {SPV_OPERAND_TYPE_LITERAL_INTEGER, {a_component}});
  }

  uint32_t result_type_id = GetResultTypeId(
      context, *GetVectorType(context, message_.vector1())->element_type());

  // Add a shuffle instruction right before the instruction identified by
  // |message_.instruction_to_insert_before|.
  FindInstruction(message_.instruction_to_insert_before(), context)
      ->InsertBefore(MakeUnique<opt::Instruction>(
          context, SpvOpVectorShuffle, result_type_id, message_.fresh_id(),
          shuffle_operands));
  fuzzerutil::UpdateModuleIdBound(context, message_.fresh_id());
  context->InvalidateAnalysesExceptFor(opt::IRContext::Analysis::kAnalysisNone);

  // Add synonym facts relating the defined elements of the shuffle result to
  // the vector components that they come from.
  for (uint32_t component_index = 0;
       component_index < static_cast<uint32_t>(message_.component_size());
       component_index++) {
    uint32_t component = message_.component(component_index);
    if (component == 0xFFFFFFFF) {
      // This component is undefined, so move on - but first note that the
      // overall shuffle result cannot be synonymous with any vector.
      continue;
    }

    // This describes the element of the result vector associated with
    // |component_index|.
    protobufs::DataDescriptor descriptor_for_result_component =
        MakeDataDescriptor(message_.fresh_id(), {component_index});

    protobufs::DataDescriptor descriptor_for_source_component;

    // Get a data descriptor for the component of the input vector to which
    // |component| refers.
    if (component <
        GetVectorType(context, message_.vector1())->element_count()) {
      descriptor_for_source_component =
          MakeDataDescriptor(message_.vector1(), {component});
    } else {
      auto index_into_vector_2 =
          component -
          GetVectorType(context, message_.vector1())->element_count();
      assert(index_into_vector_2 <
                 GetVectorType(context, message_.vector2())->element_count() &&
             "Vector shuffle index is out of bounds.");
      descriptor_for_source_component =
          MakeDataDescriptor(message_.vector2(), {index_into_vector_2});
    }

    // Add a fact relating this input vector component with the associated
    // result component.
    fact_manager->AddFactDataSynonym(descriptor_for_result_component,
                                     descriptor_for_source_component, context);
  }
}

protobufs::Transformation TransformationVectorShuffle::ToMessage() const {
  protobufs::Transformation result;
  *result.mutable_vector_shuffle() = message_;
  return result;
}

uint32_t TransformationVectorShuffle::GetResultTypeId(
    opt::IRContext* context, const opt::analysis::Type& element_type) const {
  opt::analysis::Vector result_type(
      &element_type, static_cast<uint32_t>(message_.component_size()));
  return context->get_type_mgr()->GetId(&result_type);
}

opt::analysis::Vector* TransformationVectorShuffle::GetVectorType(
    opt::IRContext* context, uint32_t id_of_vector) {
  return context->get_type_mgr()
      ->GetType(context->get_def_use_mgr()->GetDef(id_of_vector)->type_id())
      ->AsVector();
}

}  // namespace fuzz
}  // namespace spvtools
