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

#include "source/fuzz/transformation_add_constant_boolean.h"

#include "source/fuzz/fuzzer_util.h"
#include "source/opt/types.h"

namespace spvtools {
namespace fuzz {

TransformationAddConstantBoolean::TransformationAddConstantBoolean(
    const protobufs::TransformationAddConstantBoolean& message)
    : message_(message) {}

TransformationAddConstantBoolean::TransformationAddConstantBoolean(
    uint32_t fresh_id, bool is_true, bool is_irrelevant) {
  message_.set_fresh_id(fresh_id);
  message_.set_is_true(is_true);
  message_.set_is_irrelevant(is_irrelevant);
}

bool TransformationAddConstantBoolean::IsApplicable(
    opt::IRContext* ir_context, const TransformationContext& /*unused*/) const {
  return fuzzerutil::MaybeGetBoolType(ir_context) != 0 &&
         fuzzerutil::IsFreshId(ir_context, message_.fresh_id());
}

void TransformationAddConstantBoolean::Apply(
    opt::IRContext* ir_context,
    TransformationContext* transformation_context) const {
  // Add the boolean constant to the module, ensuring the module's id bound is
  // high enough.
  fuzzerutil::UpdateModuleIdBound(ir_context, message_.fresh_id());
  ir_context->module()->AddGlobalValue(
      message_.is_true() ? SpvOpConstantTrue : SpvOpConstantFalse,
      message_.fresh_id(), fuzzerutil::MaybeGetBoolType(ir_context));
  // We have added an instruction to the module, so need to be careful about the
  // validity of existing analyses.
  ir_context->InvalidateAnalysesExceptFor(
      opt::IRContext::Analysis::kAnalysisNone);

  if (message_.is_irrelevant()) {
    transformation_context->GetFactManager()->AddFactIdIsIrrelevant(
        message_.fresh_id());
  }
}

protobufs::Transformation TransformationAddConstantBoolean::ToMessage() const {
  protobufs::Transformation result;
  *result.mutable_add_constant_boolean() = message_;
  return result;
}

std::unordered_set<uint32_t> TransformationAddConstantBoolean::GetFreshIds()
    const {
  return {message_.fresh_id()};
}

}  // namespace fuzz
}  // namespace spvtools
