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
    opt::IRContext* context,
    const spvtools::fuzz::FactManager& /*unused*/) const {
  // The id must be fresh.
  if (!fuzzerutil::IsFreshId(context, message_.fresh_id())) {
    return false;
  }

  // Applicable if there is no float type with this width already declared in
  // the module.
  opt::analysis::Float float_type(message_.width());
  return context->get_type_mgr()->GetId(&float_type) == 0;
}

void TransformationAddTypeFloat::Apply(
    opt::IRContext* context, spvtools::fuzz::FactManager* /*unused*/) const {
  opt::Instruction::OperandList width = {
      {SPV_OPERAND_TYPE_LITERAL_INTEGER, {message_.width()}}};
  context->module()->AddType(MakeUnique<opt::Instruction>(
      context, SpvOpTypeFloat, 0, message_.fresh_id(), width));
  fuzzerutil::UpdateModuleIdBound(context, message_.fresh_id());
  // We have added an instruction to the module, so need to be careful about the
  // validity of existing analyses.
  context->InvalidateAnalysesExceptFor(opt::IRContext::Analysis::kAnalysisNone);
}

protobufs::Transformation TransformationAddTypeFloat::ToMessage() const {
  protobufs::Transformation result;
  *result.mutable_add_type_float() = message_;
  return result;
}

}  // namespace fuzz
}  // namespace spvtools
