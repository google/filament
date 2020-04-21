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

#include "source/fuzz/transformation_add_no_contraction_decoration.h"

#include "source/fuzz/fuzzer_util.h"

namespace spvtools {
namespace fuzz {

TransformationAddNoContractionDecoration::
    TransformationAddNoContractionDecoration(
        const spvtools::fuzz::protobufs::
            TransformationAddNoContractionDecoration& message)
    : message_(message) {}

TransformationAddNoContractionDecoration::
    TransformationAddNoContractionDecoration(uint32_t result_id) {
  message_.set_result_id(result_id);
}

bool TransformationAddNoContractionDecoration::IsApplicable(
    opt::IRContext* context,
    const spvtools::fuzz::FactManager& /*unused*/) const {
  // |message_.result_id| must be the id of an instruction.
  auto instr = context->get_def_use_mgr()->GetDef(message_.result_id());
  if (!instr) {
    return false;
  }
  // The instruction must be arithmetic.
  return IsArithmetic(instr->opcode());
}

void TransformationAddNoContractionDecoration::Apply(
    opt::IRContext* context, spvtools::fuzz::FactManager* /*unused*/) const {
  // Add a NoContraction decoration targeting |message_.result_id|.
  context->get_decoration_mgr()->AddDecoration(message_.result_id(),
                                               SpvDecorationNoContraction);
}

protobufs::Transformation TransformationAddNoContractionDecoration::ToMessage()
    const {
  protobufs::Transformation result;
  *result.mutable_add_no_contraction_decoration() = message_;
  return result;
}

bool TransformationAddNoContractionDecoration::IsArithmetic(uint32_t opcode) {
  switch (opcode) {
    case SpvOpSNegate:
    case SpvOpFNegate:
    case SpvOpIAdd:
    case SpvOpFAdd:
    case SpvOpISub:
    case SpvOpFSub:
    case SpvOpIMul:
    case SpvOpFMul:
    case SpvOpUDiv:
    case SpvOpSDiv:
    case SpvOpFDiv:
    case SpvOpUMod:
    case SpvOpSRem:
    case SpvOpSMod:
    case SpvOpFRem:
    case SpvOpFMod:
    case SpvOpVectorTimesScalar:
    case SpvOpMatrixTimesScalar:
    case SpvOpVectorTimesMatrix:
    case SpvOpMatrixTimesVector:
    case SpvOpMatrixTimesMatrix:
    case SpvOpOuterProduct:
    case SpvOpDot:
    case SpvOpIAddCarry:
    case SpvOpISubBorrow:
    case SpvOpUMulExtended:
    case SpvOpSMulExtended:
    case SpvOpAny:
    case SpvOpAll:
    case SpvOpIsNan:
    case SpvOpIsInf:
    case SpvOpIsFinite:
    case SpvOpIsNormal:
    case SpvOpSignBitSet:
    case SpvOpLessOrGreater:
    case SpvOpOrdered:
    case SpvOpUnordered:
    case SpvOpLogicalEqual:
    case SpvOpLogicalNotEqual:
    case SpvOpLogicalOr:
    case SpvOpLogicalAnd:
    case SpvOpLogicalNot:
      return true;
    default:
      return false;
  }
}

}  // namespace fuzz
}  // namespace spvtools
