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
    opt::IRContext* ir_context, const TransformationContext& /*unused*/) const {
  // A fresh id is required.
  if (!fuzzerutil::IsFreshId(ir_context, message_.fresh_id())) {
    return false;
  }
  for (auto member_type : message_.member_type_id()) {
    auto type = ir_context->get_type_mgr()->GetType(member_type);
    if (!type || type->AsFunction()) {
      // The member type id either does not refer to a type, or refers to a
      // function type; both are illegal.
      return false;
    }

    // From the spec for the BuiltIn decoration:
    // - When applied to a structure-type member, that structure type cannot
    //   be contained as a member of another structure type.
    if (type->AsStruct() &&
        fuzzerutil::MembersHaveBuiltInDecoration(ir_context, member_type)) {
      return false;
    }
  }
  return true;
}

void TransformationAddTypeStruct::Apply(
    opt::IRContext* ir_context, TransformationContext* /*unused*/) const {
  fuzzerutil::AddStructType(
      ir_context, message_.fresh_id(),
      std::vector<uint32_t>(message_.member_type_id().begin(),
                            message_.member_type_id().end()));
  // We have added an instruction to the module, so need to be careful about the
  // validity of existing analyses.
  ir_context->InvalidateAnalysesExceptFor(
      opt::IRContext::Analysis::kAnalysisNone);
}

protobufs::Transformation TransformationAddTypeStruct::ToMessage() const {
  protobufs::Transformation result;
  *result.mutable_add_type_struct() = message_;
  return result;
}

std::unordered_set<uint32_t> TransformationAddTypeStruct::GetFreshIds() const {
  return {message_.fresh_id()};
}

}  // namespace fuzz
}  // namespace spvtools
