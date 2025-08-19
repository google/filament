// Copyright (c) 2023-2025 Arm Ltd.
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

// Validates correctness of tensor instructions.

#include "source/opcode.h"
#include "source/val/validate.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {
namespace {

bool IsRankedTensor(ValidationState_t& _, uint32_t id) {
  auto inst = _.FindDef(id);
  if (!inst || inst->opcode() != spv::Op::OpTypeTensorARM ||
      inst->words().size() <= 3) {
    return false;
  }
  return true;
}

uint64_t GetTensorTypeRank(ValidationState_t& _, uint32_t id) {
  auto inst = _.FindDef(id);
  if (!inst || inst->opcode() != spv::Op::OpTypeTensorARM ||
      inst->words().size() <= 3) {
    return 0;
  }
  uint64_t rank = 0;
  if (!_.EvalConstantValUint64(inst->word(3), &rank)) {
    return 0;
  }
  return rank;
}

bool IsScalarTypeOrOrArrayOfScalarType(ValidationState_t& _, uint32_t id) {
  auto inst = _.FindDef(id);
  if (!inst) {
    return false;
  }
  return _.IsScalarType(id) || (inst->opcode() == spv::Op::OpTypeArray &&
                                _.IsScalarType(inst->word(2)));
}

spv_result_t ValidateTensorRead(ValidationState_t& _, const Instruction* inst) {
  // Result Type must be a scalar type or array of scalar type.
  if (!IsScalarTypeOrOrArrayOfScalarType(_, inst->type_id())) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Result Type to be a scalar type or array of "
              "scalar type.";
  }

  // Tensor must be a Ranked Tensor.
  auto op_tensor = inst->word(3);
  auto inst_tensor = _.FindDef(op_tensor);
  if (!inst_tensor || !IsRankedTensor(_, inst_tensor->type_id())) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Tensor to be an OpTypeTensorARM whose Rank is "
              "specified";
  }

  // The scalar type must be the same as the Element Type of Tensor.
  if (_.GetComponentType(inst_tensor->type_id()) !=
      _.GetComponentType(inst->type_id())) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Result Type to be the same as the Element Type of "
              "Tensor.";
  }

  // Coordinates is an array whose Element Type must be an integer type and
  // whose Length must be equal to the Rank of Tensor.
  auto op_coord = inst->word(4);
  auto inst_coord = _.FindDef(op_coord);
  auto tensor_rank = GetTensorTypeRank(_, inst_tensor->type_id());
  if (tensor_rank == 0 ||
      !_.IsIntArrayType(inst_coord->type_id(), tensor_rank)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Coordinates to be an array whose Element Type is an "
              "integer type and whose Length is equal to the Rank of Tensor.";
  }

  // Validate Tensor Operands
  if (inst->words().size() > 5) {
    auto toperands = static_cast<spv::TensorOperandsMask>(inst->word(5));
    if ((toperands & spv::TensorOperandsMask::OutOfBoundsValueARM) !=
        spv::TensorOperandsMask::MaskNone) {
      if (inst->words().size() < 7) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << "A value must be provided after the OutOfBoundsValueARM "
                  "Tensor Operand.";
      }
      auto op_oobval = inst->word(6);
      auto inst_oobval = _.FindDef(op_oobval);
      if (_.GetComponentType(inst_tensor->type_id()) !=
          _.GetComponentType(inst_oobval->type_id())) {
        return _.diag(SPV_ERROR_INVALID_ID, inst)
               << "Expected the type of the OutOfBoundsValueARM value to be "
                  "the same "
                  "as the Element Type of Tensor.";
      }
    }
    if ((toperands & spv::TensorOperandsMask::MakeElementAvailableARM) !=
        spv::TensorOperandsMask::MaskNone) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "MakeElementAvailableARM cannot be used with OpTensorReadARM.";
    }
    if (((toperands & spv::TensorOperandsMask::MakeElementVisibleARM) !=
         spv::TensorOperandsMask::MaskNone) &&
        ((toperands & spv::TensorOperandsMask::NonPrivateElementARM) ==
         spv::TensorOperandsMask::MaskNone)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "MakeElementAvailableARM requires NonPrivateElementARM.";
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateTensorWrite(ValidationState_t& _,
                                 const Instruction* inst) {
  // Tensor must be a Ranked Tensor.
  auto op_tensor = inst->word(1);
  auto inst_tensor = _.FindDef(op_tensor);
  if (!IsRankedTensor(_, inst_tensor->type_id())) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Tensor to be an OpTypeTensorARM whose Rank is "
              "specified";
  }

  // Coordinates is an array whose Element Type must be an integer type and
  // whose Length must be equal to the Rank of Tensor.
  auto op_coord = inst->word(2);
  auto inst_coord = _.FindDef(op_coord);
  auto tensor_rank = GetTensorTypeRank(_, inst_tensor->type_id());
  if (tensor_rank == 0 ||
      !_.IsIntArrayType(inst_coord->type_id(), tensor_rank)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Coordinates to be an array whose Element Type is an "
              "integer type and whose Length is equal to the Rank of Tensor.";
  }

  // Object must be an object of scalar type or array of scalar type.
  // The scalar type must be the same as the Element Type of Tensor.
  auto op_object = inst->word(3);
  auto inst_object = _.FindDef(op_object);
  if (!IsScalarTypeOrOrArrayOfScalarType(_, inst_object->type_id()) ||
      (_.GetComponentType(inst_object->type_id()) !=
       _.GetComponentType(inst_tensor->type_id()))) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Object to be a scalar type or array of scalar "
              "type that is the same as the Element Type of Tensor.";
  }

  // Validate Tensor Operands
  if (inst->words().size() > 5) {
    auto toperands = static_cast<spv::TensorOperandsMask>(inst->word(4));
    if ((toperands & spv::TensorOperandsMask::OutOfBoundsValueARM) !=
        spv::TensorOperandsMask::MaskNone) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "OutOfBoundsValue Tensor Operand not allowed with "
                "OpTensorWriteARM.";
    }
    if ((toperands & spv::TensorOperandsMask::MakeElementVisibleARM) !=
        spv::TensorOperandsMask::MaskNone) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "MakeElementVisibleARM not allowed with OpTensorWriteARM.";
    }
    if (((toperands & spv::TensorOperandsMask::MakeElementAvailableARM) !=
         spv::TensorOperandsMask::MaskNone) &&
        ((toperands & spv::TensorOperandsMask::NonPrivateElementARM) ==
         spv::TensorOperandsMask::MaskNone)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "MakeElementAvailableARM requires NonPrivateElementARM.";
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateTensorQuerySize(ValidationState_t& _,
                                     const Instruction* inst) {
  // Check result type
  if (!_.IsIntScalarType(inst->type_id())) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Result Type to be an integer type scalar";
  }

  // Check Tensor operand
  auto op_tensor = inst->word(3);
  auto inst_tensor = _.FindDef(op_tensor);
  if (!inst_tensor || !IsRankedTensor(_, inst_tensor->type_id())) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Tensor to be an OpTypeTensorARM whose Rank is "
              "specified";
  }

  // Check Dimension operand
  auto op_dim = inst->word(4);
  auto inst_dim = _.FindDef(op_dim);
  if (!spvOpcodeIsConstant(inst_dim->opcode()) ||
      !_.IsIntScalarType(inst_dim->type_id())) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Dimension must come from a constant instruction of scalar "
              "integer type.";
  }

  auto inst_tensor_type = _.FindDef(inst_tensor->type_id());
  auto op_tensor_rank = inst_tensor_type->word(3);
  uint64_t tensor_rank = 0;
  uint64_t dim;
  if (_.EvalConstantValUint64(op_tensor_rank, &tensor_rank) &&
      _.EvalConstantValUint64(op_dim, &dim) && (dim >= tensor_rank)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Dimension (" << dim << ") must be less than the Rank of Tensor ("
           << tensor_rank << ").";
  }

  return SPV_SUCCESS;
}

}  // namespace

// Validates correctness of tensor instructions.
spv_result_t TensorPass(ValidationState_t& _, const Instruction* inst) {
  (void)_;
  const spv::Op opcode = inst->opcode();
  switch (opcode) {
    case spv::Op::OpTensorReadARM:
      return ValidateTensorRead(_, inst);
    case spv::Op::OpTensorWriteARM:
      return ValidateTensorWrite(_, inst);
    case spv::Op::OpTensorQuerySizeARM:
      return ValidateTensorQuerySize(_, inst);
    default:
      break;
  }
  return SPV_SUCCESS;
}

}  // namespace val
}  // namespace spvtools
