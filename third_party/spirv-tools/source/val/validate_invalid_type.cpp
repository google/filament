// Copyright (c) 2025 Google Inc.
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

// Performs validation of invalid type instructions.

#include <vector>

#include "source/opcode.h"
#include "source/val/instruction.h"
#include "source/val/validate.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {

namespace {

bool IsBfloat16ScalarOrCompositeType(ValidationState_t& _, uint32_t type_id) {
  const Instruction* inst = _.FindDef(type_id);
  if (!inst) return false;

  if (_.IsBfloat16Type(type_id)) return true;

  switch (inst->opcode()) {
    case spv::Op::OpTypeMatrix:
    case spv::Op::OpTypeCooperativeMatrixNV:
      return _.IsBfloat16ScalarType(_.GetComponentType(type_id));
    default:
      return false;
  }
}

bool IsFP8ScalarOrCompositeType(ValidationState_t& _, uint32_t type_id) {
  const Instruction* inst = _.FindDef(type_id);
  if (!inst) return false;

  if (_.IsFP8Type(type_id)) return true;

  switch (inst->opcode()) {
    case spv::Op::OpTypeMatrix:
    case spv::Op::OpTypeCooperativeMatrixNV:
      return _.IsFP8ScalarType(_.GetComponentType(type_id));
    default:
      return false;
  }
}

bool IsOCPMicroscalingScalarOrCompositeType(ValidationState_t& _,
                                            uint32_t type_id) {
  return _.ContainsOCPMicroscalingType(type_id);
}

spv_result_t CheckInvalidScalarOrCompositeType(ValidationState_t& _,
                                               const Instruction* inst,
                                               uint32_t type_id) {
  const spv::Op opcode = inst->opcode();
  if (IsBfloat16ScalarOrCompositeType(_, type_id)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << spvOpcodeString(opcode) << " doesn't support BFloat16 type.";
  }
  if (IsFP8ScalarOrCompositeType(_, type_id)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << spvOpcodeString(opcode)
           << " doesn't support FP8 E4M3/E5M2 types.";
  }
  if (IsOCPMicroscalingScalarOrCompositeType(_, type_id)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << spvOpcodeString(opcode)
           << " doesn't support OCP microscaling types.";
  }

  return SPV_SUCCESS;
}

}  // namespace

// Validates correctness of certain special type instructions.
spv_result_t InvalidTypePass(ValidationState_t& _, const Instruction* inst) {
  const spv::Op opcode = inst->opcode();

  switch (opcode) {
    // Arithmetic Instructions
    case spv::Op::OpFAdd:
    case spv::Op::OpFSub:
    case spv::Op::OpFMul:
    case spv::Op::OpFDiv:
    case spv::Op::OpFRem:
    case spv::Op::OpFMod:
    case spv::Op::OpFNegate:
    case spv::Op::OpFmaKHR:
    case spv::Op::OpVectorTimesScalar:
    case spv::Op::OpMatrixTimesScalar:
    case spv::Op::OpVectorTimesMatrix:
    case spv::Op::OpMatrixTimesVector:
    case spv::Op::OpMatrixTimesMatrix:
    case spv::Op::OpOuterProduct:
    // Derivative Instructions
    case spv::Op::OpDPdx:
    case spv::Op::OpDPdy:
    case spv::Op::OpFwidth:
    case spv::Op::OpDPdxFine:
    case spv::Op::OpDPdyFine:
    case spv::Op::OpFwidthFine:
    case spv::Op::OpDPdxCoarse:
    case spv::Op::OpDPdyCoarse:
    case spv::Op::OpFwidthCoarse:
    // Atomic Instructions
    case spv::Op::OpAtomicFAddEXT:
    case spv::Op::OpAtomicFMinEXT:
    case spv::Op::OpAtomicFMaxEXT:
    case spv::Op::OpAtomicLoad:
    case spv::Op::OpAtomicExchange:
    // Group and Subgroup Instructions
    case spv::Op::OpGroupNonUniformRotateKHR:
    case spv::Op::OpGroupNonUniformBroadcast:
    case spv::Op::OpGroupNonUniformShuffle:
    case spv::Op::OpGroupNonUniformShuffleXor:
    case spv::Op::OpGroupNonUniformShuffleUp:
    case spv::Op::OpGroupNonUniformShuffleDown:
    case spv::Op::OpGroupNonUniformQuadBroadcast:
    case spv::Op::OpGroupNonUniformQuadSwap:
    case spv::Op::OpGroupNonUniformBroadcastFirst:
    case spv::Op::OpGroupNonUniformFAdd:
    case spv::Op::OpGroupNonUniformFMul:
    case spv::Op::OpGroupNonUniformFMin: {
      const uint32_t result_type = inst->type_id();
      if (spv_result_t result =
              CheckInvalidScalarOrCompositeType(_, inst, result_type)) {
        return result;
      }

      break;
    }

    case spv::Op::OpCooperativeMatrixMulAddNV: {
      if (spv_result_t result =
              CheckInvalidScalarOrCompositeType(_, inst, inst->type_id())) {
        return result;
      }
      for (uint32_t operand_index = 2; operand_index <= 4; ++operand_index) {
        if (spv_result_t result = CheckInvalidScalarOrCompositeType(
                _, inst, _.GetOperandTypeId(inst, operand_index))) {
          return result;
        }
      }
      break;
    }

    case spv::Op::OpDot: {
      const uint32_t result_type = inst->type_id();
      if (_.IsFP8Type(result_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << spvOpcodeString(opcode)
               << " doesn't support FP8 E4M3/E5M2 types.";
      }
      if (_.ContainsOCPMicroscalingType(result_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << spvOpcodeString(opcode)
               << " doesn't support OCP microscaling types.";
      }
      break;
    }

    case spv::Op::OpAtomicStore: {
      uint32_t data_type =
          _.FindDef(inst->GetOperandAs<uint32_t>(3))->type_id();
      if (_.IsBfloat16VectorType(data_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << spvOpcodeString(opcode) << " doesn't support BFloat16 type.";
      }
      if (_.IsFP8VectorType(data_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << spvOpcodeString(opcode)
               << " doesn't support FP8 E4M3/E5M2 types.";
      }
      if (_.ContainsOCPMicroscalingType(data_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << spvOpcodeString(opcode)
               << " doesn't support OCP microscaling types.";
      }
      break;
    }
    // Relational and Logical Instructions
    case spv::Op::OpIsNan:
    case spv::Op::OpIsInf:
    case spv::Op::OpIsFinite:
    case spv::Op::OpIsNormal:
    case spv::Op::OpFOrdEqual:
    case spv::Op::OpFUnordEqual:
    case spv::Op::OpFOrdNotEqual:
    case spv::Op::OpFUnordNotEqual:
    case spv::Op::OpFOrdLessThan:
    case spv::Op::OpFUnordLessThan:
    case spv::Op::OpFOrdGreaterThan:
    case spv::Op::OpFUnordGreaterThan:
    case spv::Op::OpFOrdLessThanEqual:
    case spv::Op::OpFUnordLessThanEqual:
    case spv::Op::OpFOrdGreaterThanEqual:
    case spv::Op::OpFUnordGreaterThanEqual:
    case spv::Op::OpLessOrGreater:
    case spv::Op::OpOrdered:
    case spv::Op::OpUnordered:
    case spv::Op::OpSignBitSet: {
      const uint32_t operand_type = _.GetOperandTypeId(inst, 2);
      if (_.IsBfloat16Type(operand_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << spvOpcodeString(opcode) << " doesn't support BFloat16 type.";
      }
      if (_.IsFP8Type(operand_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << spvOpcodeString(opcode)
               << " doesn't support FP8 E4M3/E5M2 types.";
      }
      if (_.ContainsOCPMicroscalingType(operand_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << spvOpcodeString(opcode)
               << " doesn't support OCP microscaling types.";
      }
      break;
    }

    case spv::Op::OpGroupNonUniformAllEqual: {
      const auto value_type = _.GetOperandTypeId(inst, 3);
      if (_.IsBfloat16Type(value_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << spvOpcodeString(opcode) << " doesn't support BFloat16 type.";
      }
      if (_.IsFP8Type(value_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << spvOpcodeString(opcode)
               << " doesn't support FP8 E4M3/E5M2 types.";
      }
      if (_.ContainsOCPMicroscalingType(value_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << spvOpcodeString(opcode)
               << " doesn't support OCP microscaling types.";
      }

      break;
    }
    default:
      break;
  }

  return SPV_SUCCESS;
}

}  // namespace val
}  // namespace spvtools
