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

#include "source/fuzz/transformation_add_type_float.h"

#include "source/fuzz/fuzzer_util.h"

namespace spvtools {
namespace fuzz {

TransformationAddTypeFloat::TransformationAddTypeFloat(uint32_t fresh_id,
                                                       uint32_t width) {
  message_.set_fresh_id(fresh_id);
  message_.set_width(width);
}

TransformationAddTypeFloat::TransformationAddTypeFloat(
    const spvtools::fuzz::protobufs::TransformationAddTypeFloat& message)
    : message_(message) {}

bool TransformationAddTypeFloat::IsApplicable(
    opt::IRContext* ir_context, const TransformationContext& /*unused*/) const {
  // The id must be fresh.
  if (!fuzzerutil::IsFreshId(ir_context, message_.fresh_id())) {
    return false;
  }

  // Checks float type width capabilities.
  switch (message_.width()) {
    case 16:
      // The Float16 capability must be present.
      if (!ir_context->get_feature_mgr()->HasCapability(SpvCapabilityFloat16)) {
        return false;
      }
      break;
    case 32:
      // No capabilities needed.
      break;
    case 64:
      // The Float64 capability must be present.
      if (!ir_context->get_feature_mgr()->HasCapability(SpvCapabilityFloat64)) {
        return false;
      }
      break;
    default:
      assert(false && "Unexpected float type width");
      return false;
  }

  // Applicable if there is no float type with this width already declared in
  // the module.
  return fuzzerutil::MaybeGetFloatType(ir_context, message_.width()) == 0;
}

void TransformationAddTypeFloat::Apply(
    opt::IRContext* ir_context, TransformationContext* /*unused*/) const {
  fuzzerutil::AddFloatType(ir_context, message_.fresh_id(), message_.width());
  // We have added an instruction to the module, so need to be careful about the
  // validity of existing analyses.
  ir_context->InvalidateAnalysesExceptFor(
      opt::IRContext::Analysis::kAnalysisNone);
}

protobufs::Transformation TransformationAddTypeFloat::ToMessage() const {
  protobufs::Transformation result;
  *result.mutable_add_type_float() = message_;
  return result;
}

std::unordered_set<uint32_t> TransformationAddTypeFloat::GetFreshIds() const {
  return {message_.fresh_id()};
}

}  // namespace fuzz
}  // namespace spvtools
