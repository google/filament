// Copyright (c) 2024 NVIDIA Corporation
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

// Validate instructions that manipulate tensor layout and view objects

#include "source/opcode.h"
#include "source/spirv_target_env.h"
#include "source/val/instruction.h"
#include "source/val/validate.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {
namespace {

spv_result_t ValidateTensorLayoutResultTypeNV(ValidationState_t& _,
                                              const Instruction* inst) {
  const auto result_type_index = 0;
  const auto result_type_id = inst->GetOperandAs<uint32_t>(result_type_index);
  const auto result_type = _.FindDef(result_type_id);

  if (!result_type || spv::Op::OpTypeTensorLayoutNV != result_type->opcode()) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << spvOpcodeString(inst->opcode()) << " Result Type <id> "
           << _.getIdName(result_type_id) << " is not a tensor layout type.";
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateTensorViewResultTypeNV(ValidationState_t& _,
                                            const Instruction* inst) {
  const auto result_type_index = 0;
  const auto result_type_id = inst->GetOperandAs<uint32_t>(result_type_index);
  const auto result_type = _.FindDef(result_type_id);

  if (!result_type || spv::Op::OpTypeTensorViewNV != result_type->opcode()) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << spvOpcodeString(inst->opcode()) << " Result Type <id> "
           << _.getIdName(result_type_id) << " is not a tensor view type.";
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateCreateTensorLayoutNV(ValidationState_t& _,
                                          const Instruction* inst) {
  if (auto error = ValidateTensorLayoutResultTypeNV(_, inst)) return error;

  return SPV_SUCCESS;
}

spv_result_t ValidateCreateTensorViewNV(ValidationState_t& _,
                                        const Instruction* inst) {
  if (auto error = ValidateTensorViewResultTypeNV(_, inst)) return error;

  return SPV_SUCCESS;
}

enum ExpectedNumValues {
  DIM,
  DIMx2,
  ONE,
  FOUR,
};

spv_result_t ValidateTensorTypeWithDimValuesNV(ValidationState_t& _,
                                               const Instruction* inst,
                                               ExpectedNumValues expected,
                                               bool is_view) {
  std::string type_str;
  if (is_view) {
    if (auto error = ValidateTensorViewResultTypeNV(_, inst)) return error;
    type_str = "TensorView";
  } else {
    if (auto error = ValidateTensorLayoutResultTypeNV(_, inst)) return error;
    type_str = "TensorLayout";
  }

  const auto result_type_id = inst->GetOperandAs<uint32_t>(0);
  const auto tensor_id = inst->GetOperandAs<uint32_t>(2);
  const auto tensor = _.FindDef(tensor_id);
  if (!tensor || result_type_id != tensor->type_id()) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << spvOpcodeString(inst->opcode()) << " Result Type <id> "
           << _.getIdName(result_type_id) << " does not match " << type_str
           << " type.";
  }

  const auto num_values = inst->operands().size() - 3;

  const auto result_type = _.FindDef(result_type_id);
  const auto dim_index = 1;
  const auto dim_id = result_type->GetOperandAs<uint32_t>(dim_index);
  uint64_t dim_value;
  if (_.EvalConstantValUint64(dim_id, &dim_value)) {
    uint64_t expected_num_values = 0;
    switch (expected) {
      case DIM:
        expected_num_values = dim_value;
        break;
      case DIMx2:
        expected_num_values = dim_value * 2;
        break;
      case ONE:
        expected_num_values = 1;
        break;
      case FOUR:
        expected_num_values = 4;
        break;
    }

    if (num_values != expected_num_values) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << spvOpcodeString(inst->opcode())
             << " unexpected number of operands.";
    }
  }

  for (uint32_t i = 0; i < num_values; ++i) {
    const auto val_id = inst->GetOperandAs<uint32_t>(i + 3);
    const auto val = _.FindDef(val_id);
    if (!val || !_.IsIntScalarType(val->type_id(), 32)) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << spvOpcodeString(inst->opcode()) << " operand <id> "
             << _.getIdName(val_id) << " is not a 32-bit integer.";
    }
  }

  return SPV_SUCCESS;
}

}  // namespace

spv_result_t TensorLayoutPass(ValidationState_t& _, const Instruction* inst) {
  switch (inst->opcode()) {
    case spv::Op::OpCreateTensorLayoutNV:
      if (auto error = ValidateCreateTensorLayoutNV(_, inst)) return error;
      break;
    case spv::Op::OpCreateTensorViewNV:
      if (auto error = ValidateCreateTensorViewNV(_, inst)) return error;
      break;
    case spv::Op::OpTensorLayoutSetBlockSizeNV:
    case spv::Op::OpTensorLayoutSetDimensionNV:
    case spv::Op::OpTensorLayoutSetStrideNV:
      if (auto error = ValidateTensorTypeWithDimValuesNV(_, inst, DIM, false))
        return error;
      break;
    case spv::Op::OpTensorLayoutSliceNV:
      if (auto error = ValidateTensorTypeWithDimValuesNV(_, inst, DIMx2, false))
        return error;
      break;
    case spv::Op::OpTensorLayoutSetClampValueNV:
      if (auto error = ValidateTensorTypeWithDimValuesNV(_, inst, ONE, false))
        return error;
      break;
    case spv::Op::OpTensorViewSetDimensionNV:
    case spv::Op::OpTensorViewSetStrideNV:
      if (auto error = ValidateTensorTypeWithDimValuesNV(_, inst, DIM, true))
        return error;
      break;
    case spv::Op::OpTensorViewSetClipNV:
      if (auto error = ValidateTensorTypeWithDimValuesNV(_, inst, FOUR, true))
        return error;
      break;
    default:
      break;
  }

  return SPV_SUCCESS;
}

}  // namespace val
}  // namespace spvtools
