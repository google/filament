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

#include "source/fuzz/transformation_add_type_function.h"

#include <vector>

#include "source/fuzz/fuzzer_util.h"

namespace spvtools {
namespace fuzz {

TransformationAddTypeFunction::TransformationAddTypeFunction(
    const spvtools::fuzz::protobufs::TransformationAddTypeFunction& message)
    : message_(message) {}

TransformationAddTypeFunction::TransformationAddTypeFunction(
    uint32_t fresh_id, uint32_t return_type_id,
    const std::vector<uint32_t>& argument_type_ids) {
  message_.set_fresh_id(fresh_id);
  message_.set_return_type_id(return_type_id);
  for (auto id : argument_type_ids) {
    message_.add_argument_type_id(id);
  }
}

bool TransformationAddTypeFunction::IsApplicable(
    opt::IRContext* context,
    const spvtools::fuzz::FactManager& /*unused*/) const {
  // The result id must be fresh.
  if (!fuzzerutil::IsFreshId(context, message_.fresh_id())) {
    return false;
  }
  // The return and argument types must be type ids but not not be function
  // type ids.
  if (!fuzzerutil::IsNonFunctionTypeId(context, message_.return_type_id())) {
    return false;
  }
  for (auto argument_type_id : message_.argument_type_id()) {
    if (!fuzzerutil::IsNonFunctionTypeId(context, argument_type_id)) {
      return false;
    }
  }
  // Check whether there is already an OpTypeFunction definition that uses
  // exactly the same return and argument type ids.  (Note that the type manager
  // does not allow us to check this, as it does not distinguish between
  // function types with different but isomorphic pointer argument types.)
  for (auto& inst : context->module()->types_values()) {
    if (inst.opcode() != SpvOpTypeFunction) {
      // Consider only OpTypeFunction instructions.
      continue;
    }
    if (inst.GetSingleWordInOperand(0) != message_.return_type_id()) {
      // Different return types - cannot be the same.
      continue;
    }
    if (inst.NumInOperands() !=
        1 + static_cast<uint32_t>(message_.argument_type_id().size())) {
      // Different numbers of arguments - cannot be the same.
      continue;
    }
    bool found_argument_mismatch = false;
    for (uint32_t index = 1; index < inst.NumInOperands(); index++) {
      if (message_.argument_type_id(index - 1) !=
          inst.GetSingleWordInOperand(index)) {
        // Argument mismatch - cannot be the same.
        found_argument_mismatch = true;
        break;
      }
    }
    if (found_argument_mismatch) {
      continue;
    }
    // Everything matches - the type is already declared.
    return false;
  }
  return true;
}

void TransformationAddTypeFunction::Apply(
    opt::IRContext* context, spvtools::fuzz::FactManager* /*unused*/) const {
  opt::Instruction::OperandList in_operands;
  in_operands.push_back({SPV_OPERAND_TYPE_ID, {message_.return_type_id()}});
  for (auto argument_type_id : message_.argument_type_id()) {
    in_operands.push_back({SPV_OPERAND_TYPE_ID, {argument_type_id}});
  }
  context->module()->AddType(MakeUnique<opt::Instruction>(
      context, SpvOpTypeFunction, 0, message_.fresh_id(), in_operands));
  fuzzerutil::UpdateModuleIdBound(context, message_.fresh_id());
  // We have added an instruction to the module, so need to be careful about the
  // validity of existing analyses.
  context->InvalidateAnalysesExceptFor(opt::IRContext::Analysis::kAnalysisNone);
}

protobufs::Transformation TransformationAddTypeFunction::ToMessage() const {
  protobufs::Transformation result;
  *result.mutable_add_type_function() = message_;
  return result;
}

}  // namespace fuzz
}  // namespace spvtools
