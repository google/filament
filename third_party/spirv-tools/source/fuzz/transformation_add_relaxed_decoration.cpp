// Copyright (c) 2020 Google LLC
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

#include "source/fuzz/transformation_add_relaxed_decoration.h"

#include "source/fuzz/fuzzer_util.h"

namespace spvtools {
namespace fuzz {

TransformationAddRelaxedDecoration::TransformationAddRelaxedDecoration(
    const spvtools::fuzz::protobufs::TransformationAddRelaxedDecoration&
        message)
    : message_(message) {}

TransformationAddRelaxedDecoration::TransformationAddRelaxedDecoration(
    uint32_t result_id) {
  message_.set_result_id(result_id);
}

bool TransformationAddRelaxedDecoration::IsApplicable(
    opt::IRContext* ir_context,
    const TransformationContext& transformation_context) const {
  // |message_.result_id| must be the id of an instruction.
  auto instr = ir_context->get_def_use_mgr()->GetDef(message_.result_id());
  if (!instr) {
    return false;
  }
  opt::BasicBlock* cur_block = ir_context->get_instr_block(instr);
  // The instruction must have a block.
  if (cur_block == nullptr) {
    return false;
  }
  // |cur_block| must be a dead block.
  if (!(transformation_context.GetFactManager()->BlockIsDead(
          cur_block->id()))) {
    return false;
  }
  // The instruction must be numeric.
  return IsNumeric(instr->opcode());
}

void TransformationAddRelaxedDecoration::Apply(
    opt::IRContext* ir_context, TransformationContext* /*unused*/) const {
  // Add a RelaxedPrecision decoration targeting |message_.result_id|.
  ir_context->get_decoration_mgr()->AddDecoration(
      message_.result_id(), SpvDecorationRelaxedPrecision);
}

protobufs::Transformation TransformationAddRelaxedDecoration::ToMessage()
    const {
  protobufs::Transformation result;
  *result.mutable_add_relaxed_decoration() = message_;
  return result;
}

bool TransformationAddRelaxedDecoration::IsNumeric(uint32_t opcode) {
  switch (opcode) {
    case SpvOpConvertFToU:
    case SpvOpConvertFToS:
    case SpvOpConvertSToF:
    case SpvOpConvertUToF:
    case SpvOpUConvert:
    case SpvOpSConvert:
    case SpvOpFConvert:
    case SpvOpConvertPtrToU:
    case SpvOpSatConvertSToU:
    case SpvOpSatConvertUToS:
    case SpvOpVectorExtractDynamic:
    case SpvOpVectorInsertDynamic:
    case SpvOpVectorShuffle:
    case SpvOpTranspose:
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
    case SpvOpShiftRightLogical:
    case SpvOpShiftRightArithmetic:
    case SpvOpShiftLeftLogical:
    case SpvOpBitwiseOr:
    case SpvOpBitwiseXor:
    case SpvOpBitwiseAnd:
    case SpvOpNot:
    case SpvOpBitFieldInsert:
    case SpvOpBitFieldSExtract:
    case SpvOpBitFieldUExtract:
    case SpvOpBitReverse:
    case SpvOpBitCount:
    case SpvOpAtomicLoad:
    case SpvOpAtomicStore:
    case SpvOpAtomicExchange:
    case SpvOpAtomicCompareExchange:
    case SpvOpAtomicCompareExchangeWeak:
    case SpvOpAtomicIIncrement:
    case SpvOpAtomicIDecrement:
    case SpvOpAtomicIAdd:
    case SpvOpAtomicISub:
    case SpvOpAtomicSMin:
    case SpvOpAtomicUMin:
    case SpvOpAtomicSMax:
    case SpvOpAtomicUMax:
    case SpvOpAtomicAnd:
    case SpvOpAtomicOr:
    case SpvOpAtomicXor:
      return true;
    default:
      return false;
  }
}

std::unordered_set<uint32_t> TransformationAddRelaxedDecoration::GetFreshIds()
    const {
  return std::unordered_set<uint32_t>();
}

}  // namespace fuzz
}  // namespace spvtools