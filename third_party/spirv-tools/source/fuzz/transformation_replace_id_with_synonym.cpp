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

namespace spvtools {
namespace fuzz {

TransformationReplaceIdWithSynonym::TransformationReplaceIdWithSynonym(
    const spvtools::fuzz::protobufs::TransformationReplaceIdWithSynonym&
        message)
    : message_(message) {}

TransformationReplaceIdWithSynonym::TransformationReplaceIdWithSynonym(
    protobufs::IdUseDescriptor id_use_descriptor,
    protobufs::DataDescriptor data_descriptor,
    uint32_t fresh_id_for_temporary) {
  assert(fresh_id_for_temporary == 0 && data_descriptor.index().size() == 0 &&
         "At present we do not support making an id that is synonymous with an "
         "index into a composite.");
  *message_.mutable_id_use_descriptor() = std::move(id_use_descriptor);
  *message_.mutable_data_descriptor() = std::move(data_descriptor);
  message_.set_fresh_id_for_temporary(fresh_id_for_temporary);
}

bool TransformationReplaceIdWithSynonym::IsApplicable(
    spvtools::opt::IRContext* context,
    const spvtools::fuzz::FactManager& fact_manager) const {
  auto id_of_interest = message_.id_use_descriptor().id_of_interest();

  // Does the fact manager know about the synonym?
  if (fact_manager.GetIdsForWhichSynonymsAreKnown().count(id_of_interest) ==
      0) {
    return false;
  }

  auto available_synonyms = fact_manager.GetSynonymsForId(id_of_interest);
  if (std::find_if(available_synonyms.begin(), available_synonyms.end(),
                   [this](protobufs::DataDescriptor dd) -> bool {
                     return DataDescriptorEquals()(&dd,
                                                   &message_.data_descriptor());
                   }) == available_synonyms.end()) {
    return false;
  }

  auto use_instruction =
      transformation::FindInstruction(message_.id_use_descriptor(), context);
  if (!use_instruction) {
    return false;
  }

  if (!ReplacingUseWithSynonymIsOk(
          context, use_instruction,
          message_.id_use_descriptor().in_operand_index(),
          message_.data_descriptor())) {
    return false;
  }

  assert(message_.fresh_id_for_temporary() == 0);
  assert(message_.data_descriptor().index().empty());

  return true;
}

void TransformationReplaceIdWithSynonym::Apply(
    spvtools::opt::IRContext* context,
    spvtools::fuzz::FactManager* /*unused*/) const {
  assert(message_.data_descriptor().index().empty());
  auto instruction_to_change =
      transformation::FindInstruction(message_.id_use_descriptor(), context);
  instruction_to_change->SetInOperand(
      message_.id_use_descriptor().in_operand_index(),
      {message_.data_descriptor().object()});
  context->InvalidateAnalysesExceptFor(opt::IRContext::Analysis::kAnalysisNone);
}

protobufs::Transformation TransformationReplaceIdWithSynonym::ToMessage()
    const {
  protobufs::Transformation result;
  *result.mutable_replace_id_with_synonym() = message_;
  return result;
}

bool TransformationReplaceIdWithSynonym::ReplacingUseWithSynonymIsOk(
    opt::IRContext* context, opt::Instruction* use_instruction,
    uint32_t use_in_operand_index, const protobufs::DataDescriptor& synonym) {
  auto defining_instruction =
      context->get_def_use_mgr()->GetDef(synonym.object());

  if (use_instruction == defining_instruction) {
    // If we have an instruction:
    //   %a = OpCopyObject %t %b
    // then we know %a and %b are synonymous, but we do *not* want to turn
    // this into:
    //   %a = OpCopyObject %t %a
    // We require this special case because an instruction dominates itself.
    return false;
  }

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

  // We now need to check that replacing the use with the synonym will respect
  // dominance rules - i.e. the synonym needs to dominate the use.
  auto dominator_analysis = context->GetDominatorAnalysis(
      context->get_instr_block(use_instruction)->GetParent());
  if (use_instruction->opcode() == SpvOpPhi) {
    // In the case where the use is an operand to OpPhi, it is actually the
    // *parent* block associated with the operand that must be dominated by the
    // synonym.
    auto parent_block =
        use_instruction->GetSingleWordInOperand(use_in_operand_index + 1);
    if (!dominator_analysis->Dominates(
            context->get_instr_block(defining_instruction)->id(),
            parent_block)) {
      return false;
    }
  } else if (!dominator_analysis->Dominates(defining_instruction,
                                            use_instruction)) {
    return false;
  }
  return true;
}

}  // namespace fuzz
}  // namespace spvtools
