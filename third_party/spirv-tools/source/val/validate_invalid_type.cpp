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

// Validates correctness of certain special type instructions.
spv_result_t InvalidTypePass(ValidationState_t& _, const Instruction* inst) {
  const spv::Op opcode = inst->opcode();

  switch (opcode) {
    // OpExtInst
    case spv::Op::OpExtInst:
    // Arithmetic Instructions
    case spv::Op::OpFAdd:
    case spv::Op::OpFSub:
    case spv::Op::OpFMul:
    case spv::Op::OpFDiv:
    case spv::Op::OpFRem:
    case spv::Op::OpFMod:
    case spv::Op::OpFNegate:
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
      if (_.IsBfloat16ScalarType(result_type) ||
          _.IsBfloat16VectorType(result_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << spvOpcodeString(opcode) << " doesn't support BFloat16 type.";
      }
      if (_.IsFP8ScalarOrVectorType(result_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << spvOpcodeString(opcode)
               << " doesn't support FP8 E4M3/E5M2 types.";
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
      break;
    }
    // Relational and Logical Instructions
    case spv::Op::OpIsNan:
    case spv::Op::OpIsInf:
    case spv::Op::OpIsFinite:
    case spv::Op::OpIsNormal:
    case spv::Op::OpSignBitSet: {
      const uint32_t operand_type = _.GetOperandTypeId(inst, 2);
      if (_.IsBfloat16ScalarType(operand_type) ||
          _.IsBfloat16VectorType(operand_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << spvOpcodeString(opcode) << " doesn't support BFloat16 type.";
      }
      if (_.IsFP8ScalarOrVectorType(operand_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << spvOpcodeString(opcode)
               << " doesn't support FP8 E4M3/E5M2 types.";
      }
      break;
    }

    case spv::Op::OpGroupNonUniformAllEqual: {
      const auto value_type = _.GetOperandTypeId(inst, 3);
      if (_.IsBfloat16ScalarType(value_type) ||
          _.IsBfloat16VectorType(value_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << spvOpcodeString(opcode) << " doesn't support BFloat16 type.";
      }
      if (_.IsFP8ScalarOrVectorType(value_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << spvOpcodeString(opcode)
               << " doesn't support FP8 E4M3/E5M2 types.";
      }

      break;
    }

    case spv::Op::OpMatrixTimesMatrix: {
      const uint32_t result_type = inst->type_id();
      uint32_t res_num_rows = 0;
      uint32_t res_num_cols = 0;
      uint32_t res_col_type = 0;
      uint32_t res_component_type = 0;
      if (_.GetMatrixTypeInfo(result_type, &res_num_rows, &res_num_cols,
                              &res_col_type, &res_component_type)) {
        if (_.IsBfloat16ScalarType(res_component_type)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << spvOpcodeString(opcode)
                 << " doesn't support BFloat16 type.";
        }
        if (_.IsFP8ScalarOrVectorType(res_component_type)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << spvOpcodeString(opcode)
                 << " doesn't support FP8 E4M3/E5M2 types.";
        }
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
