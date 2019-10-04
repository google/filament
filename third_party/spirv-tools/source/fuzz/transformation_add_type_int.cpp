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
    opt::IRContext* context,
    const spvtools::fuzz::FactManager& /*unused*/) const {
  // The id must be fresh.
  if (!fuzzerutil::IsFreshId(context, message_.fresh_id())) {
    return false;
  }

  // Applicable if there is no int type with this width and signedness already
  // declared in the module.
  opt::analysis::Integer int_type(message_.width(), message_.is_signed());
  return context->get_type_mgr()->GetId(&int_type) == 0;
}

void TransformationAddTypeInt::Apply(
    opt::IRContext* context, spvtools::fuzz::FactManager* /*unused*/) const {
  opt::Instruction::OperandList in_operands = {
      {SPV_OPERAND_TYPE_LITERAL_INTEGER, {message_.width()}},
      {SPV_OPERAND_TYPE_LITERAL_INTEGER, {message_.is_signed() ? 1u : 0u}}};
  context->module()->AddType(MakeUnique<opt::Instruction>(
      context, SpvOpTypeInt, 0, message_.fresh_id(), in_operands));
  fuzzerutil::UpdateModuleIdBound(context, message_.fresh_id());
  // We have added an instruction to the module, so need to be careful about the
  // validity of existing analyses.
  context->InvalidateAnalysesExceptFor(opt::IRContext::Analysis::kAnalysisNone);
}

protobufs::Transformation TransformationAddTypeInt::ToMessage() const {
  protobufs::Transformation result;
  *result.mutable_add_type_int() = message_;
  return result;
}

}  // namespace fuzz
}  // namespace spvtools
