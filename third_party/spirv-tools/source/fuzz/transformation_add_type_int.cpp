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

#include "source/fuzz/transformation_add_type_int.h"

#include "source/fuzz/fuzzer_util.h"

namespace spvtools {
namespace fuzz {

TransformationAddTypeInt::TransformationAddTypeInt(
    const spvtools::fuzz::protobufs::TransformationAddTypeInt& message)
    : message_(message) {}

TransformationAddTypeInt::TransformationAddTypeInt(uint32_t fresh_id,
                                                   uint32_t width,
                                                   bool is_signed) {
  message_.set_fresh_id(fresh_id);
  message_.set_width(width);
  message_.set_is_signed(is_signed);
}

bool TransformationAddTypeInt::IsApplicable(
    opt::IRContext* ir_context, const TransformationContext& /*unused*/) const {
  // The id must be fresh.
  if (!fuzzerutil::IsFreshId(ir_context, message_.fresh_id())) {
    return false;
  }

  // Checks integer type width capabilities.
  switch (message_.width()) {
    case 8:
      // The Int8 capability must be present.
      if (!ir_context->get_feature_mgr()->HasCapability(SpvCapabilityInt8)) {
        return false;
      }
      break;
    case 16:
      // The Int16 capability must be present.
      if (!ir_context->get_feature_mgr()->HasCapability(SpvCapabilityInt16)) {
        return false;
      }
      break;
    case 32:
      // No capabilities needed.
      break;
    case 64:
      // The Int64 capability must be present.
      if (!ir_context->get_feature_mgr()->HasCapability(SpvCapabilityInt64)) {
        return false;
      }
      break;
    default:
      assert(false && "Unexpected integer type width");
      return false;
  }

  // Applicable if there is no int type with this width and signedness already
  // declared in the module.
  return fuzzerutil::MaybeGetIntegerType(ir_context, message_.width(),
                                         message_.is_signed()) == 0;
}

void TransformationAddTypeInt::Apply(opt::IRContext* ir_context,
                                     TransformationContext* /*unused*/) const {
  fuzzerutil::AddIntegerType(ir_context, message_.fresh_id(), message_.width(),
                             message_.is_signed());
  // We have added an instruction to the module, so need to be careful about the
  // validity of existing analyses.
  ir_context->InvalidateAnalysesExceptFor(
      opt::IRContext::Analysis::kAnalysisNone);
}

protobufs::Transformation TransformationAddTypeInt::ToMessage() const {
  protobufs::Transformation result;
  *result.mutable_add_type_int() = message_;
  return result;
}

std::unordered_set<uint32_t> TransformationAddTypeInt::GetFreshIds() const {
  return {message_.fresh_id()};
}

}  // namespace fuzz
}  // namespace spvtools
