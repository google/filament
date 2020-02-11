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
    uint32_t fresh_id, bool is_true) {
  message_.set_fresh_id(fresh_id);
  message_.set_is_true(is_true);
}

bool TransformationAddConstantBoolean::IsApplicable(
    opt::IRContext* context, const FactManager& /*unused*/) const {
  opt::analysis::Bool bool_type;
  if (!context->get_type_mgr()->GetId(&bool_type)) {
    // No OpTypeBool is present.
    return false;
  }
  return fuzzerutil::IsFreshId(context, message_.fresh_id());
}

void TransformationAddConstantBoolean::Apply(opt::IRContext* context,
                                             FactManager* /*unused*/) const {
  opt::analysis::Bool bool_type;
  // Add the boolean constant to the module, ensuring the module's id bound is
  // high enough.
  fuzzerutil::UpdateModuleIdBound(context, message_.fresh_id());
  context->module()->AddGlobalValue(
      message_.is_true() ? SpvOpConstantTrue : SpvOpConstantFalse,
      message_.fresh_id(), context->get_type_mgr()->GetId(&bool_type));
  // We have added an instruction to the module, so need to be careful about the
  // validity of existing analyses.
  context->InvalidateAnalysesExceptFor(opt::IRContext::Analysis::kAnalysisNone);
}

protobufs::Transformation TransformationAddConstantBoolean::ToMessage() const {
  protobufs::Transformation result;
  *result.mutable_add_constant_boolean() = message_;
  return result;
}

}  // namespace fuzz
}  // namespace spvtools
