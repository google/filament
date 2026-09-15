// Copyright (c) 2026 LunarG Inc.
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

#include <cstdint>

#include "source/val/instruction.h"
#include "source/val/validate.h"
#include "source/val/validate_scopes.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {
namespace {

spv_result_t ValidateGroupAnyAll(ValidationState_t& _,
                                 const Instruction* inst) {
  if (!_.IsBoolScalarType(inst->type_id())) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Result must be a boolean scalar type";
  }

  if (!_.IsBoolScalarType(_.GetOperandTypeId(inst, 3))) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Predicate must be a boolean scalar type";
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateGroupBroadcast(ValidationState_t& _,
                                    const Instruction* inst) {
  const uint32_t type_id = inst->type_id();
  if (!_.IsFloatScalarOrVectorType(type_id) &&
      !_.IsIntScalarOrVectorType(type_id) &&
      !_.IsBoolScalarOrVectorType(type_id)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Result must be a scalar or vector of integer, floating-point, "
              "or boolean type";
  }

  const uint32_t value_type_id = _.GetOperandTypeId(inst, 3);
  if (value_type_id != type_id) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "The type of Value must match the Result type";
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateGroupFloat(ValidationState_t& _, const Instruction* inst) {
  const uint32_t type_id = inst->type_id();
  if (!_.IsFloatScalarOrVectorType(type_id)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Result must be a scalar or vector of float type";
  }

  const uint32_t x_type_id = _.GetOperandTypeId(inst, 4);
  if (x_type_id != type_id) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "The type of X must match the Result type";
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateGroupInt(ValidationState_t& _, const Instruction* inst) {
  const uint32_t type_id = inst->type_id();
  if (!_.IsIntScalarOrVectorType(type_id)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Result must be a scalar or vector of integer type";
  }

  const uint32_t x_type_id = _.GetOperandTypeId(inst, 4);
  if (x_type_id != type_id) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "The type of X must match the Result type";
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateGroupAsyncCopy(ValidationState_t& _,
                                    const Instruction* inst) {
  if (_.FindDef(inst->type_id())->opcode() != spv::Op::OpTypeEvent) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "The result type must be OpTypeEvent.";
  }

  const uint32_t destination = _.GetOperandTypeId(inst, 3);
  const Instruction* destination_pointer = _.FindDef(destination);
  if (destination_pointer->opcode() != spv::Op::OpTypePointer) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Destination to be a pointer.";
  }
  const auto destination_sc =
      destination_pointer->GetOperandAs<spv::StorageClass>(1);
  if (destination_sc != spv::StorageClass::Workgroup &&
      destination_sc != spv::StorageClass::CrossWorkgroup) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Destination to be a pointer with storage class "
              "Workgroup or CrossWorkgroup.";
  }
  const uint32_t destination_type =
      destination_pointer->GetOperandAs<uint32_t>(2);
  if (!_.IsIntScalarOrVectorType(destination_type) &&
      !_.IsFloatScalarOrVectorType(destination_type)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Destination to be a pointer to scalar or vector of "
              "floating-point type or integer type.";
  }

  const uint32_t source = _.GetOperandTypeId(inst, 4);
  const Instruction* source_pointer = _.FindDef(source);
  const auto source_sc = source_pointer->GetOperandAs<spv::StorageClass>(1);
  const uint32_t source_type = source_pointer->GetOperandAs<uint32_t>(2);
  if (destination_type != source_type) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Destination and Source to be the same type.";
  }

  if (destination_sc == spv::StorageClass::Workgroup &&
      source_sc != spv::StorageClass::CrossWorkgroup) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "If Destination storage class is Workgroup, then the Source "
              "storage class must be CrossWorkgroup.";
  } else if (destination_sc == spv::StorageClass::CrossWorkgroup &&
             source_sc != spv::StorageClass::Workgroup) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "If Destination storage class is CrossWorkgroup, then the Source "
              "storage class must be Workgroup.";
  }

  const bool is_physical_64 =
      _.addressing_model() == spv::AddressingModel::Physical64;
  const uint32_t bit_width = is_physical_64 ? 64 : 32;

  const uint32_t num_elements_type =
      _.GetTypeId(inst->GetOperandAs<uint32_t>(5));
  if (!_.IsIntScalarType(num_elements_type, bit_width)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "NumElements must be a " << bit_width
           << "-bit int scalar when Addressing Model is "
           << (is_physical_64 ? "Physical64" : "Physical32");
  }

  const uint32_t stride_type = _.GetTypeId(inst->GetOperandAs<uint32_t>(6));
  if (!_.IsIntScalarType(stride_type, bit_width)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Stride must be a " << bit_width
           << "-bit int scalar when Addressing Model is "
           << (is_physical_64 ? "Physical64" : "Physical32");
  }

  const uint32_t event = _.GetOperandTypeId(inst, 7);
  const Instruction* event_type = _.FindDef(event);
  if (event_type->opcode() != spv::Op::OpTypeEvent) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Event to be type OpTypeEvent.";
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateGroupWaitEvents(ValidationState_t& _,
                                     const Instruction* inst) {
  const uint32_t num_events_id = _.GetOperandTypeId(inst, 1);
  if (!_.IsIntScalarType(num_events_id, 32)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Num Events to be a 32-bit int scalar.";
  }

  const uint32_t events_id = _.GetOperandTypeId(inst, 2);
  const Instruction* var_pointer = _.FindDef(events_id);
  if (var_pointer->opcode() != spv::Op::OpTypePointer) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Events List to be a pointer.";
  }
  const Instruction* event_list_type =
      _.FindDef(var_pointer->GetOperandAs<uint32_t>(2));
  if (event_list_type->opcode() != spv::Op::OpTypeEvent) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Events List to be a pointer to OpTypeEvent.";
  }

  return SPV_SUCCESS;
}

}  // namespace

spv_result_t GroupPass(ValidationState_t& _, const Instruction* inst) {
  const spv::Op opcode = inst->opcode();

  switch (opcode) {
    case spv::Op::OpGroupAny:
    case spv::Op::OpGroupAll:
      return ValidateGroupAnyAll(_, inst);
    case spv::Op::OpGroupBroadcast:
      return ValidateGroupBroadcast(_, inst);
    case spv::Op::OpGroupFAdd:
    case spv::Op::OpGroupFMax:
    case spv::Op::OpGroupFMin:
      return ValidateGroupFloat(_, inst);
    case spv::Op::OpGroupIAdd:
    case spv::Op::OpGroupUMin:
    case spv::Op::OpGroupSMin:
    case spv::Op::OpGroupUMax:
    case spv::Op::OpGroupSMax:
      return ValidateGroupInt(_, inst);
    case spv::Op::OpGroupAsyncCopy:
      return ValidateGroupAsyncCopy(_, inst);
    case spv::Op::OpGroupWaitEvents:
      return ValidateGroupWaitEvents(_, inst);
    default:
      break;
  }

  return SPV_SUCCESS;
}

}  // namespace val
}  // namespace spvtools
