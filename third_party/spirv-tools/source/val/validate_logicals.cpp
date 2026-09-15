// Copyright (c) 2017 Google Inc.
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

// Validates correctness of logical SPIR-V instructions.

#include "source/opcode.h"
#include "source/val/instruction.h"
#include "source/val/validate.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {

spv_result_t ValidateAnyAll(ValidationState_t& _, const Instruction* inst) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
  if (!_.IsBoolScalarType(result_type))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected bool scalar type as Result Type: "
           << spvOpcodeString(opcode);

  const uint32_t vector_type = _.GetOperandTypeId(inst, 2);
  if (!vector_type || !_.IsBoolVectorType(vector_type))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected operand to be vector bool: " << spvOpcodeString(opcode);
  return SPV_SUCCESS;
}

spv_result_t ValidateSingleStatus(ValidationState_t& _,
                                  const Instruction* inst) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
  if (!_.IsBoolScalarType(result_type) && !_.IsBoolVectorType(result_type))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected bool scalar or vector type as Result Type: "
           << spvOpcodeString(opcode);

  const uint32_t operand_type = _.GetOperandTypeId(inst, 2);
  if (!operand_type || (!_.IsFloatScalarType(operand_type) &&
                        !_.IsFloatVectorType(operand_type)))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected operand to be scalar or vector float: "
           << spvOpcodeString(opcode);

  if (_.GetDimension(result_type) != _.GetDimension(operand_type))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected vector sizes of Result Type and the operand to be "
              "equal: "
           << spvOpcodeString(opcode);

  return SPV_SUCCESS;
}

spv_result_t ValidateFloatCompare(ValidationState_t& _,
                                  const Instruction* inst) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
  if (!_.IsBoolScalarType(result_type) && !_.IsBoolVectorType(result_type))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected bool scalar or vector type as Result Type: "
           << spvOpcodeString(opcode);

  const uint32_t left_operand_type = _.GetOperandTypeId(inst, 2);
  if (!left_operand_type || (!_.IsFloatScalarType(left_operand_type) &&
                             !_.IsFloatVectorType(left_operand_type)))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected operands to be scalar or vector float: "
           << spvOpcodeString(opcode);

  if (_.GetDimension(result_type) != _.GetDimension(left_operand_type))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected vector sizes of Result Type and the operands to be "
              "equal: "
           << spvOpcodeString(opcode);

  if (left_operand_type != _.GetOperandTypeId(inst, 3))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected left and right operands to have the same type: "
           << spvOpcodeString(opcode);
  return SPV_SUCCESS;
}

spv_result_t ValidateLogicalCompare(ValidationState_t& _,
                                    const Instruction* inst,
                                    uint32_t operand_index = 2) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
  if (!_.IsBoolScalarType(result_type) && !_.IsBoolVectorType(result_type))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected bool scalar or vector type as Result Type: "
           << spvOpcodeString(opcode);

  const uint32_t operand_1 = _.GetOperandTypeId(inst, operand_index);
  const uint32_t operand_2 = _.GetOperandTypeId(inst, operand_index + 1);
  if (result_type != operand_1 || result_type != operand_2)
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected both operands to be of Result Type: "
           << spvOpcodeString(opcode);
  return SPV_SUCCESS;
}

spv_result_t ValidateLogicalNot(ValidationState_t& _, const Instruction* inst,
                                uint32_t operand_index = 2) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
  if (!_.IsBoolScalarType(result_type) && !_.IsBoolVectorType(result_type))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected bool scalar or vector type as Result Type: "
           << spvOpcodeString(opcode);

  if (result_type != _.GetOperandTypeId(inst, operand_index))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected operand to be of Result Type: "
           << spvOpcodeString(opcode);
  return SPV_SUCCESS;
}

spv_result_t ValidateSelect(ValidationState_t& _, const Instruction* inst,
                            uint32_t operand_index = 2) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
  uint32_t dimension = 1;
  const Instruction* type_inst = _.FindDef(result_type);
  assert(type_inst);

  const auto composites = _.features().select_between_composites;
  auto fail = [&_, composites, inst, opcode]() -> spv_result_t {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected scalar or " << (composites ? "composite" : "vector")
           << " type as Result Type: " << spvOpcodeString(opcode);
  };

  const spv::Op type_opcode = type_inst->opcode();
  switch (type_opcode) {
    case spv::Op::OpTypeUntypedPointerKHR:
    case spv::Op::OpTypePointer: {
      if (_.addressing_model() == spv::AddressingModel::Logical &&
          !_.HasCapability(spv::Capability::VariablePointersStorageBuffer))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Using pointers with OpSelect requires capability "
               << "VariablePointers or VariablePointersStorageBuffer";
      break;
    }

    case spv::Op::OpTypeSampledImage:
    case spv::Op::OpTypeImage:
    case spv::Op::OpTypeSampler: {
      if (!_.HasCapability(spv::Capability::BindlessTextureNV))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Using image/sampler with OpSelect requires capability "
               << "BindlessTextureNV";
      break;
    }

    case spv::Op::OpTypeVector: {
      dimension = type_inst->word(3);
      break;
    }
    case spv::Op::OpTypeVectorIdEXT: {
      dimension = _.GetDimension(result_type);
      break;
    }

    case spv::Op::OpTypeBool:
    case spv::Op::OpTypeInt:
    case spv::Op::OpTypeFloat: {
      break;
    }

    // Not RuntimeArray because of other rules.
    case spv::Op::OpTypeArray:
    case spv::Op::OpTypeMatrix:
    case spv::Op::OpTypeStruct: {
      if (!composites) return fail();
      break;
    }

    default:
      return fail();
  }

  const uint32_t condition_type = _.GetOperandTypeId(inst, operand_index);
  const uint32_t left_type = _.GetOperandTypeId(inst, operand_index + 1);
  const uint32_t right_type = _.GetOperandTypeId(inst, operand_index + 2);

  if (!condition_type || (!_.IsBoolScalarType(condition_type) &&
                          !_.IsBoolVectorType(condition_type)))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected bool scalar or vector type as condition: "
           << spvOpcodeString(opcode);

  if (_.GetDimension(condition_type) != dimension) {
    // If the condition is a vector type, then the result must also be a
    // vector with matching dimensions. In SPIR-V 1.4, a scalar condition
    // can be used to select between vector types. |composites| is a
    // proxy for SPIR-V 1.4 functionality.
    if (!composites || _.IsBoolVectorType(condition_type)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Expected vector sizes of Result Type and the condition "
                "to be equal: "
             << spvOpcodeString(opcode);
    }
  }

  if (result_type != left_type || result_type != right_type)
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected both objects to be of Result Type: "
           << spvOpcodeString(opcode);
  return SPV_SUCCESS;
}

spv_result_t ValidateIntCompare(ValidationState_t& _, const Instruction* inst,
                                uint32_t operand_index = 2) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
  if (!_.IsBoolScalarType(result_type) && !_.IsBoolVectorType(result_type))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected bool scalar or vector type as Result Type: "
           << spvOpcodeString(opcode);

  const uint32_t left_type = _.GetOperandTypeId(inst, operand_index);
  const uint32_t right_type = _.GetOperandTypeId(inst, operand_index + 1);

  if (!left_type ||
      (!_.IsIntScalarType(left_type) && !_.IsIntVectorType(left_type)))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected operands to be scalar or vector int: "
           << spvOpcodeString(opcode);

  if (_.GetDimension(result_type) != _.GetDimension(left_type))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected vector sizes of Result Type and the operands to be"
           << " equal: " << spvOpcodeString(opcode);

  if (!right_type ||
      (!_.IsIntScalarType(right_type) && !_.IsIntVectorType(right_type)))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected operands to be scalar or vector int: "
           << spvOpcodeString(opcode);

  if (_.GetDimension(result_type) != _.GetDimension(right_type))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected vector sizes of Result Type and the operands to be"
           << " equal: " << spvOpcodeString(opcode);

  if (_.GetBitWidth(left_type) != _.GetBitWidth(right_type))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected both operands to have the same component bit "
              "width: "
           << spvOpcodeString(opcode);
  return SPV_SUCCESS;
}

// Validates correctness of logical instructions.
spv_result_t LogicalsPass(ValidationState_t& _, const Instruction* inst) {
  switch (inst->opcode()) {
    case spv::Op::OpAny:
    case spv::Op::OpAll:
      return ValidateAnyAll(_, inst);
    case spv::Op::OpIsNan:
    case spv::Op::OpIsInf:
    case spv::Op::OpIsFinite:
    case spv::Op::OpIsNormal:
    case spv::Op::OpSignBitSet:
      return ValidateSingleStatus(_, inst);
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
      return ValidateFloatCompare(_, inst);
    case spv::Op::OpLogicalEqual:
    case spv::Op::OpLogicalNotEqual:
    case spv::Op::OpLogicalOr:
    case spv::Op::OpLogicalAnd:
      return ValidateLogicalCompare(_, inst);
    case spv::Op::OpLogicalNot:
      return ValidateLogicalNot(_, inst);
    case spv::Op::OpSelect:
      return ValidateSelect(_, inst);
    case spv::Op::OpIEqual:
    case spv::Op::OpINotEqual:
    case spv::Op::OpUGreaterThan:
    case spv::Op::OpUGreaterThanEqual:
    case spv::Op::OpULessThan:
    case spv::Op::OpULessThanEqual:
    case spv::Op::OpSGreaterThan:
    case spv::Op::OpSGreaterThanEqual:
    case spv::Op::OpSLessThan:
    case spv::Op::OpSLessThanEqual:
      return ValidateIntCompare(_, inst);

    case spv::Op::OpSpecConstantOp: {
      switch (inst->GetOperandAs<spv::Op>(2u)) {
        case spv::Op::OpLogicalEqual:
        case spv::Op::OpLogicalNotEqual:
        case spv::Op::OpLogicalOr:
        case spv::Op::OpLogicalAnd:
          return ValidateLogicalCompare(_, inst, 3);
        case spv::Op::OpLogicalNot:
          return ValidateLogicalNot(_, inst, 3);
        case spv::Op::OpSelect:
          return ValidateSelect(_, inst, 3);
        case spv::Op::OpIEqual:
        case spv::Op::OpINotEqual:
        case spv::Op::OpUGreaterThan:
        case spv::Op::OpUGreaterThanEqual:
        case spv::Op::OpULessThan:
        case spv::Op::OpULessThanEqual:
        case spv::Op::OpSGreaterThan:
        case spv::Op::OpSGreaterThanEqual:
        case spv::Op::OpSLessThan:
        case spv::Op::OpSLessThanEqual:
          return ValidateIntCompare(_, inst, 3);
        default:
          break;
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
