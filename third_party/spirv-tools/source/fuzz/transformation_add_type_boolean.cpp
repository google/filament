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

#include "source/fuzz/transformation_add_type_boolean.h"

#include "source/fuzz/fuzzer_util.h"

namespace spvtools {
namespace fuzz {

TransformationAddTypeBoolean::TransformationAddTypeBoolean(
    const spvtools::fuzz::protobufs::TransformationAddTypeBoolean& message)
    : message_(message) {}

TransformationAddTypeBoolean::TransformationAddTypeBoolean(uint32_t fresh_id) {
  message_.set_fresh_id(fresh_id);
}

bool TransformationAddTypeBoolean::IsApplicable(
    opt::IRContext* ir_context, const TransformationContext& /*unused*/) const {
  // The id must be fresh.
  if (!fuzzerutil::IsFreshId(ir_context, message_.fresh_id())) {
    return false;
  }

  // Applicable if there is no bool type already declared in the module.
  opt::analysis::Bool bool_type;
  return ir_context->get_type_mgr()->GetId(&bool_type) == 0;
}

void TransformationAddTypeBoolean::Apply(
    opt::IRContext* ir_context, TransformationContext* /*unused*/) const {
  opt::Instruction::OperandList empty_operands;
  ir_context->module()->AddType(MakeUnique<opt::Instruction>(
      ir_context, SpvOpTypeBool, 0, message_.fresh_id(), empty_operands));
  fuzzerutil::UpdateModuleIdBound(ir_context, message_.fresh_id());
  // We have added an instruction to the module, so need to be careful about the
  // validity of existing analyses.
  ir_context->InvalidateAnalysesExceptFor(
      opt::IRContext::Analysis::kAnalysisNone);
}

protobufs::Transformation TransformationAddTypeBoolean::ToMessage() const {
  protobufs::Transformation result;
  *result.mutable_add_type_boolean() = message_;
  return result;
}

std::unordered_set<uint32_t> TransformationAddTypeBoolean::GetFreshIds() const {
  return {message_.fresh_id()};
}

}  // namespace fuzz
}  // namespace spvtools
