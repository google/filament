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

#include "source/fuzz/transformation_replace_id_with_synonym.h"

#include <algorithm>

#include "source/fuzz/data_descriptor.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/id_use_descriptor.h"
#include "source/opt/types.h"
#include "source/util/make_unique.h"

namespace spvtools {
namespace fuzz {

TransformationReplaceIdWithSynonym::TransformationReplaceIdWithSynonym(
    const spvtools::fuzz::protobufs::TransformationReplaceIdWithSynonym&
        message)
    : message_(message) {}

TransformationReplaceIdWithSynonym::TransformationReplaceIdWithSynonym(
    protobufs::IdUseDescriptor id_use_descriptor, uint32_t synonymous_id) {
  *message_.mutable_id_use_descriptor() = std::move(id_use_descriptor);
  message_.set_synonymous_id(synonymous_id);
}

bool TransformationReplaceIdWithSynonym::IsApplicable(
    spvtools::opt::IRContext* context,
    const spvtools::fuzz::FactManager& fact_manager) const {
  auto id_of_interest = message_.id_use_descriptor().id_of_interest();

  // Does the fact manager know about the synonym?
  auto data_descriptor_for_synonymous_id =
      MakeDataDescriptor(message_.synonymous_id(), {});
  if (!fact_manager.IsSynonymous(MakeDataDescriptor(id_of_interest, {}),
                                 data_descriptor_for_synonymous_id, context)) {
    return false;
  }

  // Does the id use descriptor in the transformation identify an instruction?
  auto use_instruction =
      FindInstructionContainingUse(message_.id_use_descriptor(), context);
  if (!use_instruction) {
    return false;
  }

  // Is the use suitable for being replaced in principle?
  if (!UseCanBeReplacedWithSynonym(
          context, use_instruction,
          message_.id_use_descriptor().in_operand_index())) {
    return false;
  }

  // The transformation is applicable if the synonymous id is available at the
  // use point.
  return IdsIsAvailableAtUse(context, use_instruction,
                             message_.id_use_descriptor().in_operand_index(),
                             message_.synonymous_id());
}

void TransformationReplaceIdWithSynonym::Apply(
    spvtools::opt::IRContext* context,
    spvtools::fuzz::FactManager* /*unused*/) const {
  auto instruction_to_change =
      FindInstructionContainingUse(message_.id_use_descriptor(), context);
  instruction_to_change->SetInOperand(
      message_.id_use_descriptor().in_operand_index(),
      {message_.synonymous_id()});
  context->InvalidateAnalysesExceptFor(opt::IRContext::Analysis::kAnalysisNone);
}

protobufs::Transformation TransformationReplaceIdWithSynonym::ToMessage()
    const {
  protobufs::Transformation result;
  *result.mutable_replace_id_with_synonym() = message_;
  return result;
}

bool TransformationReplaceIdWithSynonym::IdsIsAvailableAtUse(
    opt::IRContext* context, opt::Instruction* use_instruction,
    uint32_t use_input_operand_index, uint32_t id) {
  if (!context->get_instr_block(id)) {
    return true;
  }
  auto defining_instruction = context->get_def_use_mgr()->GetDef(id);
  if (defining_instruction == use_instruction) {
    return false;
  }
  auto dominator_analysis = context->GetDominatorAnalysis(
      context->get_instr_block(use_instruction)->GetParent());
  if (use_instruction->opcode() == SpvOpPhi) {
    // In the case where the use is an operand to OpPhi, it is actually the
    // *parent* block associated with the operand that must be dominated by
    // the synonym.
    auto parent_block =
        use_instruction->GetSingleWordInOperand(use_input_operand_index + 1);
    return dominator_analysis->Dominates(
        context->get_instr_block(defining_instruction)->id(), parent_block);
  }
  return dominator_analysis->Dominates(defining_instruction, use_instruction);
}

bool TransformationReplaceIdWithSynonym::UseCanBeReplacedWithSynonym(
    opt::IRContext* context, opt::Instruction* use_instruction,
    uint32_t use_in_operand_index) {
  if (use_instruction->opcode() == SpvOpAccessChain &&
      use_in_operand_index > 0) {
    // This is an access chain index.  If the (sub-)object being accessed by the
    // given index has struct type then we cannot replace the use with a
    // synonym, as the use needs to be an OpConstant.

    // Get the top-level composite type that is being accessed.
    auto object_being_accessed = context->get_def_use_mgr()->GetDef(
        use_instruction->GetSingleWordInOperand(0));
    auto pointer_type =
        context->get_type_mgr()->GetType(object_being_accessed->type_id());
    assert(pointer_type->AsPointer());
    auto composite_type_being_accessed =
        pointer_type->AsPointer()->pointee_type();

    // Now walk the access chain, tracking the type of each sub-object of the
    // composite that is traversed, until the index of interest is reached.
    for (uint32_t index_in_operand = 1; index_in_operand < use_in_operand_index;
         index_in_operand++) {
      // For vectors, matrices and arrays, getting the type of the sub-object is
      // trivial. For the struct case, the sub-object type is field-sensitive,
      // and depends on the constant index that is used.
      if (composite_type_being_accessed->AsVector()) {
        composite_type_being_accessed =
            composite_type_being_accessed->AsVector()->element_type();
      } else if (composite_type_being_accessed->AsMatrix()) {
        composite_type_being_accessed =
            composite_type_being_accessed->AsMatrix()->element_type();
      } else if (composite_type_being_accessed->AsArray()) {
        composite_type_being_accessed =
            composite_type_being_accessed->AsArray()->element_type();
      } else {
        assert(composite_type_being_accessed->AsStruct());
        auto constant_index_instruction = context->get_def_use_mgr()->GetDef(
            use_instruction->GetSingleWordInOperand(index_in_operand));
        assert(constant_index_instruction->opcode() == SpvOpConstant);
        uint32_t member_index =
            constant_index_instruction->GetSingleWordInOperand(0);
        composite_type_being_accessed =
            composite_type_being_accessed->AsStruct()
                ->element_types()[member_index];
      }
    }

    // We have found the composite type being accessed by the index we are
    // considering replacing. If it is a struct, then we cannot do the
    // replacement as struct indices must be constants.
    if (composite_type_being_accessed->AsStruct()) {
      return false;
    }
  }

  if (use_instruction->opcode() == SpvOpFunctionCall &&
      use_in_operand_index > 0) {
    // This is a function call argument.  It is not allowed to have pointer
    // type.

    // Get the definition of the function being called.
    auto function = context->get_def_use_mgr()->GetDef(
        use_instruction->GetSingleWordInOperand(0));
    // From the function definition, get the function type.
    auto function_type =
        context->get_def_use_mgr()->GetDef(function->GetSingleWordInOperand(1));
    // OpTypeFunction's 0-th input operand is the function return type, and the
    // function argument types follow. Because the arguments to OpFunctionCall
    // start from input operand 1, we can use |use_in_operand_index| to get the
    // type associated with this function argument.
    auto parameter_type = context->get_type_mgr()->GetType(
        function_type->GetSingleWordInOperand(use_in_operand_index));
    if (parameter_type->AsPointer()) {
      return false;
    }
  }
  return true;
}

}  // namespace fuzz
}  // namespace spvtools
