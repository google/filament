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

#include "source/fuzz/transformation_add_type_array.h"

#include "source/fuzz/fuzzer_util.h"

namespace spvtools {
namespace fuzz {

TransformationAddTypeArray::TransformationAddTypeArray(
    const spvtools::fuzz::protobufs::TransformationAddTypeArray& message)
    : message_(message) {}

TransformationAddTypeArray::TransformationAddTypeArray(uint32_t fresh_id,
                                                       uint32_t element_type_id,
                                                       uint32_t size_id) {
  message_.set_fresh_id(fresh_id);
  message_.set_element_type_id(element_type_id);
  message_.set_size_id(size_id);
}

bool TransformationAddTypeArray::IsApplicable(
    opt::IRContext* context,
    const spvtools::fuzz::FactManager& /*unused*/) const {
  // A fresh id is required.
  if (!fuzzerutil::IsFreshId(context, message_.fresh_id())) {
    return false;
  }
  auto element_type =
      context->get_type_mgr()->GetType(message_.element_type_id());
  if (!element_type || element_type->AsFunction()) {
    // The element type id either does not refer to a type, or refers to a
    // function type; both are illegal.
    return false;
  }
  auto constant =
      context->get_constant_mgr()->GetConstantsFromIds({message_.size_id()});
  if (constant.empty()) {
    // The size id does not refer to a constant.
    return false;
  }
  assert(constant.size() == 1 &&
         "Only one constant id was provided, so only one constant should have "
         "been returned");

  auto int_constant = constant[0]->AsIntConstant();
  if (!int_constant) {
    // The size constant is not an integer.
    return false;
  }
  // We require that the size constant be a 32-bit value that is positive when
  // interpreted as being signed.
  return int_constant->words().size() == 1 && int_constant->GetS32() >= 1;
}

void TransformationAddTypeArray::Apply(
    opt::IRContext* context, spvtools::fuzz::FactManager* /*unused*/) const {
  opt::Instruction::OperandList in_operands;
  in_operands.push_back({SPV_OPERAND_TYPE_ID, {message_.element_type_id()}});
  in_operands.push_back({SPV_OPERAND_TYPE_ID, {message_.size_id()}});
  context->module()->AddType(MakeUnique<opt::Instruction>(
      context, SpvOpTypeArray, 0, message_.fresh_id(), in_operands));
  fuzzerutil::UpdateModuleIdBound(context, message_.fresh_id());
  // We have added an instruction to the module, so need to be careful about the
  // validity of existing analyses.
  context->InvalidateAnalysesExceptFor(opt::IRContext::Analysis::kAnalysisNone);
}

protobufs::Transformation TransformationAddTypeArray::ToMessage() const {
  protobufs::Transformation result;
  *result.mutable_add_type_array() = message_;
  return result;
}

}  // namespace fuzz
}  // namespace spvtools
