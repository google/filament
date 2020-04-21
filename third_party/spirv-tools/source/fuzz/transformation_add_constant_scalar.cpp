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

#include "source/fuzz/transformation_add_constant_scalar.h"

#include "source/fuzz/fuzzer_util.h"

namespace spvtools {
namespace fuzz {

TransformationAddConstantScalar::TransformationAddConstantScalar(
    const spvtools::fuzz::protobufs::TransformationAddConstantScalar& message)
    : message_(message) {}

TransformationAddConstantScalar::TransformationAddConstantScalar(
    uint32_t fresh_id, uint32_t type_id, std::vector<uint32_t> words) {
  message_.set_fresh_id(fresh_id);
  message_.set_type_id(type_id);
  for (auto word : words) {
    message_.add_word(word);
  }
}

bool TransformationAddConstantScalar::IsApplicable(
    opt::IRContext* context,
    const spvtools::fuzz::FactManager& /*unused*/) const {
  // The id needs to be fresh.
  if (!fuzzerutil::IsFreshId(context, message_.fresh_id())) {
    return false;
  }
  // The type id for the scalar must exist and be a type.
  auto type = context->get_type_mgr()->GetType(message_.type_id());
  if (!type) {
    return false;
  }
  uint32_t width;
  if (type->AsFloat()) {
    width = type->AsFloat()->width();
  } else if (type->AsInteger()) {
    width = type->AsInteger()->width();
  } else {
    return false;
  }
  // The number of words is the integer floor of the width.
  auto words = (width + 32 - 1) / 32;

  // The number of words provided by the transformation needs to match the
  // width of the type.
  return static_cast<uint32_t>(message_.word().size()) == words;
}

void TransformationAddConstantScalar::Apply(
    opt::IRContext* context, spvtools::fuzz::FactManager* /*unused*/) const {
  opt::Instruction::OperandList operand_list;
  for (auto word : message_.word()) {
    operand_list.push_back({SPV_OPERAND_TYPE_LITERAL_INTEGER, {word}});
  }
  context->module()->AddGlobalValue(
      MakeUnique<opt::Instruction>(context, SpvOpConstant, message_.type_id(),
                                   message_.fresh_id(), operand_list));

  fuzzerutil::UpdateModuleIdBound(context, message_.fresh_id());

  // We have added an instruction to the module, so need to be careful about the
  // validity of existing analyses.
  context->InvalidateAnalysesExceptFor(opt::IRContext::Analysis::kAnalysisNone);
}

protobufs::Transformation TransformationAddConstantScalar::ToMessage() const {
  protobufs::Transformation result;
  *result.mutable_add_constant_scalar() = message_;
  return result;
}

}  // namespace fuzz
}  // namespace spvtools
