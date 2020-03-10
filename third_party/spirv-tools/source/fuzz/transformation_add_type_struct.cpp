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

#include "source/fuzz/transformation_add_type_struct.h"

#include "source/fuzz/fuzzer_util.h"

namespace spvtools {
namespace fuzz {

TransformationAddTypeStruct::TransformationAddTypeStruct(
    const spvtools::fuzz::protobufs::TransformationAddTypeStruct& message)
    : message_(message) {}

TransformationAddTypeStruct::TransformationAddTypeStruct(
    uint32_t fresh_id, const std::vector<uint32_t>& member_type_ids) {
  message_.set_fresh_id(fresh_id);
  for (auto member_type_id : member_type_ids) {
    message_.add_member_type_id(member_type_id);
  }
}

bool TransformationAddTypeStruct::IsApplicable(
    opt::IRContext* context,
    const spvtools::fuzz::FactManager& /*unused*/) const {
  // A fresh id is required.
  if (!fuzzerutil::IsFreshId(context, message_.fresh_id())) {
    return false;
  }
  for (auto member_type : message_.member_type_id()) {
    auto type = context->get_type_mgr()->GetType(member_type);
    if (!type || type->AsFunction()) {
      // The member type id either does not refer to a type, or refers to a
      // function type; both are illegal.
      return false;
    }
  }
  return true;
}

void TransformationAddTypeStruct::Apply(
    opt::IRContext* context, spvtools::fuzz::FactManager* /*unused*/) const {
  opt::Instruction::OperandList in_operands;
  for (auto member_type : message_.member_type_id()) {
    in_operands.push_back({SPV_OPERAND_TYPE_ID, {member_type}});
  }
  context->module()->AddType(MakeUnique<opt::Instruction>(
      context, SpvOpTypeStruct, 0, message_.fresh_id(), in_operands));
  fuzzerutil::UpdateModuleIdBound(context, message_.fresh_id());
  // We have added an instruction to the module, so need to be careful about the
  // validity of existing analyses.
  context->InvalidateAnalysesExceptFor(opt::IRContext::Analysis::kAnalysisNone);
}

protobufs::Transformation TransformationAddTypeStruct::ToMessage() const {
  protobufs::Transformation result;
  *result.mutable_add_type_struct() = message_;
  return result;
}

}  // namespace fuzz
}  // namespace spvtools
