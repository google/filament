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

// Validates correctness of conversion instructions.

#include <climits>

#include "source/opcode.h"
#include "source/spirv_constant.h"
#include "source/spirv_target_env.h"
#include "source/val/instruction.h"
#include "source/val/validate.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {

// Validates correctness of conversion instructions.
spv_result_t ConversionPass(ValidationState_t& _, const Instruction* inst) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();

  switch (opcode) {
    case spv::Op::OpConvertFToU: {
      if (!_.IsUnsignedIntScalarType(result_type) &&
          !_.IsUnsignedIntVectorType(result_type) &&
          !_.IsUnsignedIntCooperativeMatrixType(result_type) &&
          !_.IsUnsignedIntCooperativeVectorNVType(result_type))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected unsigned int scalar or vector type as Result Type: "
               << spvOpcodeString(opcode);

      const uint32_t input_type = _.GetOperandTypeId(inst, 2);
      if (!input_type || (!_.IsFloatScalarType(input_type) &&
                          !_.IsFloatVectorType(input_type) &&
                          !_.IsFloatCooperativeMatrixType(input_type) &&
                          !_.IsFloatCooperativeVectorNVType(input_type)))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected input to be float scalar or vector: "
               << spvOpcodeString(opcode);

      if (_.IsCooperativeVectorNVType(result_type) ||
          _.IsCooperativeVectorNVType(input_type)) {
        spv_result_t ret =
            _.CooperativeVectorDimensionsMatch(inst, result_type, input_type);
        if (ret != SPV_SUCCESS) return ret;
      } else if (_.IsCooperativeMatrixType(result_type) ||
                 _.IsCooperativeMatrixType(input_type)) {
        spv_result_t ret =
            _.CooperativeMatrixShapesMatch(inst, result_type, input_type, true);
        if (ret != SPV_SUCCESS) return ret;
      } else {
        if (_.GetDimension(result_type) != _.GetDimension(input_type))
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Expected input to have the same dimension as Result Type: "
                 << spvOpcodeString(opcode);
      }

      break;
    }

    case spv::Op::OpConvertFToS: {
      if (!_.IsIntScalarType(result_type) && !_.IsIntVectorType(result_type) &&
          !_.IsIntCooperativeMatrixType(result_type) &&
          !_.IsIntCooperativeVectorNVType(result_type))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected int scalar or vector type as Result Type: "
               << spvOpcodeString(opcode);

      const uint32_t input_type = _.GetOperandTypeId(inst, 2);
      if (!input_type || (!_.IsFloatScalarType(input_type) &&
                          !_.IsFloatVectorType(input_type) &&
                          !_.IsFloatCooperativeMatrixType(input_type) &&
                          !_.IsFloatCooperativeVectorNVType(input_type)))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected input to be float scalar or vector: "
               << spvOpcodeString(opcode);

      if (_.IsCooperativeVectorNVType(result_type) ||
          _.IsCooperativeVectorNVType(input_type)) {
        spv_result_t ret =
            _.CooperativeVectorDimensionsMatch(inst, result_type, input_type);
        if (ret != SPV_SUCCESS) return ret;
      } else if (_.IsCooperativeMatrixType(result_type) ||
                 _.IsCooperativeMatrixType(input_type)) {
        spv_result_t ret =
            _.CooperativeMatrixShapesMatch(inst, result_type, input_type, true);
        if (ret != SPV_SUCCESS) return ret;
      } else {
        if (_.GetDimension(result_type) != _.GetDimension(input_type))
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Expected input to have the same dimension as Result Type: "
                 << spvOpcodeString(opcode);
      }

      break;
    }

    case spv::Op::OpConvertSToF:
    case spv::Op::OpConvertUToF: {
      if (!_.IsFloatScalarType(result_type) &&
          !_.IsFloatVectorType(result_type) &&
          !_.IsFloatCooperativeMatrixType(result_type) &&
          !_.IsFloatCooperativeVectorNVType(result_type))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected float scalar or vector type as Result Type: "
               << spvOpcodeString(opcode);

      const uint32_t input_type = _.GetOperandTypeId(inst, 2);
      if (!input_type ||
          (!_.IsIntScalarType(input_type) && !_.IsIntVectorType(input_type) &&
           !_.IsIntCooperativeMatrixType(input_type) &&
           !_.IsIntCooperativeVectorNVType(input_type)))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected input to be int scalar or vector: "
               << spvOpcodeString(opcode);

      if (_.IsCooperativeVectorNVType(result_type) ||
          _.IsCooperativeVectorNVType(input_type)) {
        spv_result_t ret =
            _.CooperativeVectorDimensionsMatch(inst, result_type, input_type);
        if (ret != SPV_SUCCESS) return ret;
      } else if (_.IsCooperativeMatrixType(result_type) ||
                 _.IsCooperativeMatrixType(input_type)) {
        spv_result_t ret =
            _.CooperativeMatrixShapesMatch(inst, result_type, input_type, true);
        if (ret != SPV_SUCCESS) return ret;
      } else {
        if (_.GetDimension(result_type) != _.GetDimension(input_type))
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Expected input to have the same dimension as Result Type: "
                 << spvOpcodeString(opcode);
      }

      break;
    }

    case spv::Op::OpUConvert: {
      if (!_.IsUnsignedIntScalarType(result_type) &&
          !_.IsUnsignedIntVectorType(result_type) &&
          !_.IsUnsignedIntCooperativeMatrixType(result_type) &&
          !_.IsUnsignedIntCooperativeVectorNVType(result_type))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected unsigned int scalar or vector type as Result Type: "
               << spvOpcodeString(opcode);

      const uint32_t input_type = _.GetOperandTypeId(inst, 2);
      if (!input_type ||
          (!_.IsIntScalarType(input_type) && !_.IsIntVectorType(input_type) &&
           !_.IsIntCooperativeMatrixType(input_type) &&
           !_.IsIntCooperativeVectorNVType(input_type)))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected input to be int scalar or vector: "
               << spvOpcodeString(opcode);

      if (_.IsCooperativeVectorNVType(result_type) ||
          _.IsCooperativeVectorNVType(input_type)) {
        spv_result_t ret =
            _.CooperativeVectorDimensionsMatch(inst, result_type, input_type);
        if (ret != SPV_SUCCESS) return ret;
      } else if (_.IsCooperativeMatrixType(result_type) ||
                 _.IsCooperativeMatrixType(input_type)) {
        spv_result_t ret =
            _.CooperativeMatrixShapesMatch(inst, result_type, input_type, true);
        if (ret != SPV_SUCCESS) return ret;
      } else {
        if (_.GetDimension(result_type) != _.GetDimension(input_type))
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Expected input to have the same dimension as Result Type: "
                 << spvOpcodeString(opcode);
      }

      if (_.GetBitWidth(result_type) == _.GetBitWidth(input_type))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected input to have different bit width from Result "
                  "Type: "
               << spvOpcodeString(opcode);
      break;
    }

    case spv::Op::OpSConvert: {
      if (!_.IsIntScalarType(result_type) && !_.IsIntVectorType(result_type) &&
          !_.IsIntCooperativeMatrixType(result_type) &&
          !_.IsIntCooperativeVectorNVType(result_type))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected int scalar or vector type as Result Type: "
               << spvOpcodeString(opcode);

      const uint32_t input_type = _.GetOperandTypeId(inst, 2);
      if (!input_type ||
          (!_.IsIntScalarType(input_type) && !_.IsIntVectorType(input_type) &&
           !_.IsIntCooperativeMatrixType(input_type) &&
           !_.IsIntCooperativeVectorNVType(input_type)))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected input to be int scalar or vector: "
               << spvOpcodeString(opcode);

      if (_.IsCooperativeVectorNVType(result_type) ||
          _.IsCooperativeVectorNVType(input_type)) {
        spv_result_t ret =
            _.CooperativeVectorDimensionsMatch(inst, result_type, input_type);
        if (ret != SPV_SUCCESS) return ret;
      } else if (_.IsCooperativeMatrixType(result_type) ||
                 _.IsCooperativeMatrixType(input_type)) {
        spv_result_t ret =
            _.CooperativeMatrixShapesMatch(inst, result_type, input_type, true);
        if (ret != SPV_SUCCESS) return ret;
      } else {
        if (_.GetDimension(result_type) != _.GetDimension(input_type))
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Expected input to have the same dimension as Result Type: "
                 << spvOpcodeString(opcode);
      }

      if (_.GetBitWidth(result_type) == _.GetBitWidth(input_type))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected input to have different bit width from Result "
                  "Type: "
               << spvOpcodeString(opcode);
      break;
    }

    case spv::Op::OpFConvert: {
      if (!_.IsFloatScalarType(result_type) &&
          !_.IsFloatVectorType(result_type) &&
          !_.IsFloatCooperativeMatrixType(result_type) &&
          !_.IsFloatCooperativeVectorNVType(result_type))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected float scalar or vector type as Result Type: "
               << spvOpcodeString(opcode);

      const uint32_t input_type = _.GetOperandTypeId(inst, 2);
      if (!input_type || (!_.IsFloatScalarType(input_type) &&
                          !_.IsFloatVectorType(input_type) &&
                          !_.IsFloatCooperativeMatrixType(input_type) &&
                          !_.IsFloatCooperativeVectorNVType(input_type)))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected input to be float scalar or vector: "
               << spvOpcodeString(opcode);

      if (_.IsCooperativeVectorNVType(result_type) ||
          _.IsCooperativeVectorNVType(input_type)) {
        spv_result_t ret =
            _.CooperativeVectorDimensionsMatch(inst, result_type, input_type);
        if (ret != SPV_SUCCESS) return ret;
      } else if (_.IsCooperativeMatrixType(result_type) ||
                 _.IsCooperativeMatrixType(input_type)) {
        spv_result_t ret =
            _.CooperativeMatrixShapesMatch(inst, result_type, input_type, true);
        if (ret != SPV_SUCCESS) return ret;
      } else {
        if (_.GetDimension(result_type) != _.GetDimension(input_type))
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Expected input to have the same dimension as Result Type: "
                 << spvOpcodeString(opcode);
      }

      // Scalar type
      const uint32_t resScalarType = _.GetComponentType(result_type);
      const uint32_t inputScalartype = _.GetComponentType(input_type);
      if (resScalarType == inputScalartype) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected component type of Value to be different from "
                  "component type of Result Type: "
               << spvOpcodeString(opcode);
      }
      break;
    }

    case spv::Op::OpQuantizeToF16: {
      if ((!_.IsFloatScalarType(result_type) &&
           !_.IsFloatVectorType(result_type)) ||
          _.GetBitWidth(result_type) != 32)
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected 32-bit float scalar or vector type as Result Type: "
               << spvOpcodeString(opcode);

      const uint32_t input_type = _.GetOperandTypeId(inst, 2);
      if (input_type != result_type)
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected input type to be equal to Result Type: "
               << spvOpcodeString(opcode);
      break;
    }

    case spv::Op::OpConvertPtrToU: {
      if (!_.IsUnsignedIntScalarType(result_type))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected unsigned int scalar type as Result Type: "
               << spvOpcodeString(opcode);

      const uint32_t input_type = _.GetOperandTypeId(inst, 2);
      if (!_.IsPointerType(input_type))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected input to be a pointer: " << spvOpcodeString(opcode);

      if (_.addressing_model() == spv::AddressingModel::Logical)
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Logical addressing not supported: "
               << spvOpcodeString(opcode);

      if (_.addressing_model() ==
          spv::AddressingModel::PhysicalStorageBuffer64) {
        spv::StorageClass input_storage_class;
        uint32_t input_data_type = 0;
        _.GetPointerTypeInfo(input_type, &input_data_type,
                             &input_storage_class);
        if (input_storage_class != spv::StorageClass::PhysicalStorageBuffer)
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Pointer storage class must be PhysicalStorageBuffer: "
                 << spvOpcodeString(opcode);

        if (spvIsVulkanEnv(_.context()->target_env)) {
          if (_.GetBitWidth(result_type) != 64) {
            return _.diag(SPV_ERROR_INVALID_DATA, inst)
                   << _.VkErrorID(4710)
                   << "PhysicalStorageBuffer64 addressing mode requires the "
                      "result integer type to have a 64-bit width for Vulkan "
                      "environment.";
          }
        }
      }
      break;
    }

    case spv::Op::OpSatConvertSToU:
    case spv::Op::OpSatConvertUToS: {
      if (!_.IsIntScalarType(result_type) && !_.IsIntVectorType(result_type))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected int scalar or vector type as Result Type: "
               << spvOpcodeString(opcode);

      const uint32_t input_type = _.GetOperandTypeId(inst, 2);
      if (!input_type ||
          (!_.IsIntScalarType(input_type) && !_.IsIntVectorType(input_type)))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected int scalar or vector as input: "
               << spvOpcodeString(opcode);

      if (_.GetDimension(result_type) != _.GetDimension(input_type))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected input to have the same dimension as Result Type: "
               << spvOpcodeString(opcode);
      break;
    }

    case spv::Op::OpConvertUToPtr: {
      if (!_.IsPointerType(result_type))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected Result Type to be a pointer: "
               << spvOpcodeString(opcode);

      const uint32_t input_type = _.GetOperandTypeId(inst, 2);
      if (!input_type || !_.IsIntScalarType(input_type))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected int scalar as input: " << spvOpcodeString(opcode);

      if (_.addressing_model() == spv::AddressingModel::Logical)
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Logical addressing not supported: "
               << spvOpcodeString(opcode);

      if (_.addressing_model() ==
          spv::AddressingModel::PhysicalStorageBuffer64) {
        spv::StorageClass result_storage_class;
        uint32_t result_data_type = 0;
        _.GetPointerTypeInfo(result_type, &result_data_type,
                             &result_storage_class);
        if (result_storage_class != spv::StorageClass::PhysicalStorageBuffer)
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Pointer storage class must be PhysicalStorageBuffer: "
                 << spvOpcodeString(opcode);

        if (spvIsVulkanEnv(_.context()->target_env)) {
          if (_.GetBitWidth(input_type) != 64) {
            return _.diag(SPV_ERROR_INVALID_DATA, inst)
                   << _.VkErrorID(4710)
                   << "PhysicalStorageBuffer64 addressing mode requires the "
                      "input integer to have a 64-bit width for Vulkan "
                      "environment.";
          }
        }
      }
      break;
    }

    case spv::Op::OpPtrCastToGeneric: {
      spv::StorageClass result_storage_class;
      uint32_t result_data_type = 0;
      if (!_.GetPointerTypeInfo(result_type, &result_data_type,
                                &result_storage_class))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected Result Type to be a pointer: "
               << spvOpcodeString(opcode);

      if (result_storage_class != spv::StorageClass::Generic)
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected Result Type to have storage class Generic: "
               << spvOpcodeString(opcode);

      const uint32_t input_type = _.GetOperandTypeId(inst, 2);
      spv::StorageClass input_storage_class;
      uint32_t input_data_type = 0;
      if (!_.GetPointerTypeInfo(input_type, &input_data_type,
                                &input_storage_class))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected input to be a pointer: " << spvOpcodeString(opcode);

      if (input_storage_class != spv::StorageClass::Workgroup &&
          input_storage_class != spv::StorageClass::CrossWorkgroup &&
          input_storage_class != spv::StorageClass::Function)
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected input to have storage class Workgroup, "
               << "CrossWorkgroup or Function: " << spvOpcodeString(opcode);

      if (result_data_type != input_data_type)
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected input and Result Type to point to the same type: "
               << spvOpcodeString(opcode);
      break;
    }

    case spv::Op::OpGenericCastToPtr: {
      spv::StorageClass result_storage_class;
      uint32_t result_data_type = 0;
      if (!_.GetPointerTypeInfo(result_type, &result_data_type,
                                &result_storage_class))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected Result Type to be a pointer: "
               << spvOpcodeString(opcode);

      if (result_storage_class != spv::StorageClass::Workgroup &&
          result_storage_class != spv::StorageClass::CrossWorkgroup &&
          result_storage_class != spv::StorageClass::Function)
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected Result Type to have storage class Workgroup, "
               << "CrossWorkgroup or Function: " << spvOpcodeString(opcode);

      const uint32_t input_type = _.GetOperandTypeId(inst, 2);
      spv::StorageClass input_storage_class;
      uint32_t input_data_type = 0;
      if (!_.GetPointerTypeInfo(input_type, &input_data_type,
                                &input_storage_class))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected input to be a pointer: " << spvOpcodeString(opcode);

      if (input_storage_class != spv::StorageClass::Generic)
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected input to have storage class Generic: "
               << spvOpcodeString(opcode);

      if (result_data_type != input_data_type)
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected input and Result Type to point to the same type: "
               << spvOpcodeString(opcode);
      break;
    }

    case spv::Op::OpGenericCastToPtrExplicit: {
      spv::StorageClass result_storage_class;
      uint32_t result_data_type = 0;
      if (!_.GetPointerTypeInfo(result_type, &result_data_type,
                                &result_storage_class))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected Result Type to be a pointer: "
               << spvOpcodeString(opcode);

      const auto target_storage_class =
          inst->GetOperandAs<spv::StorageClass>(3);
      if (result_storage_class != target_storage_class)
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected Result Type to be of target storage class: "
               << spvOpcodeString(opcode);

      const uint32_t input_type = _.GetOperandTypeId(inst, 2);
      spv::StorageClass input_storage_class;
      uint32_t input_data_type = 0;
      if (!_.GetPointerTypeInfo(input_type, &input_data_type,
                                &input_storage_class))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected input to be a pointer: " << spvOpcodeString(opcode);

      if (input_storage_class != spv::StorageClass::Generic)
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected input to have storage class Generic: "
               << spvOpcodeString(opcode);

      if (result_data_type != input_data_type)
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected input and Result Type to point to the same type: "
               << spvOpcodeString(opcode);

      if (target_storage_class != spv::StorageClass::Workgroup &&
          target_storage_class != spv::StorageClass::CrossWorkgroup &&
          target_storage_class != spv::StorageClass::Function)
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected target storage class to be Workgroup, "
               << "CrossWorkgroup or Function: " << spvOpcodeString(opcode);
      break;
    }

    case spv::Op::OpBitcast: {
      const uint32_t input_type = _.GetOperandTypeId(inst, 2);
      if (!input_type)
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected input to have a type: " << spvOpcodeString(opcode);

      const bool result_is_pointer = _.IsPointerType(result_type);
      const bool result_is_int_scalar = _.IsIntScalarType(result_type);
      const bool input_is_pointer = _.IsPointerType(input_type);
      const bool input_is_int_scalar = _.IsIntScalarType(input_type);

      const bool result_is_coopmat = _.IsCooperativeMatrixType(result_type);
      const bool input_is_coopmat = _.IsCooperativeMatrixType(input_type);
      const bool result_is_coopvec = _.IsCooperativeVectorNVType(result_type);
      const bool input_is_coopvec = _.IsCooperativeVectorNVType(input_type);

      if (!result_is_pointer && !result_is_int_scalar && !result_is_coopmat &&
          !result_is_coopvec && !_.IsIntVectorType(result_type) &&
          !_.IsFloatScalarType(result_type) &&
          !_.IsFloatVectorType(result_type))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected Result Type to be a pointer or int or float vector "
               << "or scalar type: " << spvOpcodeString(opcode);

      if (!input_is_pointer && !input_is_int_scalar && !input_is_coopmat &&
          !input_is_coopvec && !_.IsIntVectorType(input_type) &&
          !_.IsFloatScalarType(input_type) && !_.IsFloatVectorType(input_type))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected input to be a pointer or int or float vector "
               << "or scalar: " << spvOpcodeString(opcode);

      // NV_cooperative_vector doesn't allow bitcasting between vec<->coopvec,
      // but long_vector does.
      if (result_is_coopvec != input_is_coopvec &&
          !_.HasCapability(spv::Capability::LongVectorEXT))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Cooperative vector can only be cast to another cooperative "
               << "vector: " << spvOpcodeString(opcode);

      if (result_is_coopmat != input_is_coopmat)
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Cooperative matrix can only be cast to another cooperative "
               << "matrix: " << spvOpcodeString(opcode);

      if (result_is_coopvec && input_is_coopvec &&
          !_.HasCapability(spv::Capability::LongVectorEXT)) {
        spv_result_t ret =
            _.CooperativeVectorDimensionsMatch(inst, result_type, input_type);
        if (ret != SPV_SUCCESS) return ret;
      }

      if (result_is_coopmat) {
        spv_result_t ret = _.CooperativeMatrixShapesMatch(inst, result_type,
                                                          input_type, false);
        if (ret != SPV_SUCCESS) return ret;
      }

      if (_.version() >= SPV_SPIRV_VERSION_WORD(1, 5) ||
          _.HasExtension(kSPV_KHR_physical_storage_buffer)) {
        const bool result_is_int_vector = _.IsIntVectorType(result_type);
        const bool result_has_int32 =
            _.ContainsSizedIntOrFloatType(result_type, spv::Op::OpTypeInt, 32);
        const bool input_is_int_vector = _.IsIntVectorType(input_type);
        const bool input_has_int32 =
            _.ContainsSizedIntOrFloatType(input_type, spv::Op::OpTypeInt, 32);
        if (result_is_pointer && !input_is_pointer && !input_is_int_scalar &&
            !(input_is_int_vector && input_has_int32))
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "In SPIR-V 1.5 or later (or with "
                    "SPV_KHR_physical_storage_buffer), expected input to be a "
                    "pointer, "
                    "int scalar or 32-bit int "
                    "vector if Result Type is pointer: "
                 << spvOpcodeString(opcode);

        if (input_is_pointer && !result_is_pointer && !result_is_int_scalar &&
            !(result_is_int_vector && result_has_int32))
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "In SPIR-V 1.5 or later (or with "
                    "SPV_KHR_physical_storage_buffer), pointer can only be "
                    "converted to "
                    "another pointer, int "
                    "scalar or 32-bit int vector: "
                 << spvOpcodeString(opcode);
      } else {
        if (result_is_pointer && !input_is_pointer && !input_is_int_scalar)
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "In SPIR-V 1.4 or earlier (and without "
                    "SPV_KHR_physical_storage_buffer), expected input to be a "
                    "pointer "
                    "or int scalar if Result "
                    "Type is pointer: "
                 << spvOpcodeString(opcode);

        if (input_is_pointer && !result_is_pointer && !result_is_int_scalar)
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "In SPIR-V 1.4 or earlier (and without "
                    "SPV_KHR_physical_storage_buffer), pointer can only be "
                    "converted "
                    "to another pointer or int "
                    "scalar: "
                 << spvOpcodeString(opcode);
      }

      if (!result_is_pointer && !input_is_pointer) {
        const uint32_t result_size =
            _.GetBitWidth(result_type) * _.GetDimension(result_type);
        const uint32_t input_size =
            _.GetBitWidth(input_type) * _.GetDimension(input_type);
        if (result_size != input_size)
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Expected input to have the same total bit width as "
                 << "Result Type: " << spvOpcodeString(opcode);
      }
      break;
    }

    case spv::Op::OpConvertUToAccelerationStructureKHR: {
      if (!_.IsAccelerationStructureType(result_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected Result Type to be a Acceleration Structure: "
               << spvOpcodeString(opcode);
      }

      const uint32_t input_type = _.GetOperandTypeId(inst, 2);
      if (!input_type || !_.IsUnsigned64BitHandle(input_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected 64-bit uint scalar or 2-component 32-bit uint "
                  "vector as input: "
               << spvOpcodeString(opcode);
      }

      break;
    }

    case spv::Op::OpCooperativeMatrixConvertNV:
    case spv::Op::OpCooperativeMatrixTransposeNV: {
      if (!_.IsCooperativeMatrixType(result_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected cooperative matrix Result Type: "
               << spvOpcodeString(opcode);
      }
      const uint32_t input_type = _.GetOperandTypeId(inst, 2);
      if (!_.IsCooperativeMatrixType(input_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected cooperative matrix type for Matrix input: "
               << spvOpcodeString(opcode);
      }

      bool swap_row_col = (opcode == spv::Op::OpCooperativeMatrixTransposeNV);
      if (auto error = _.CooperativeMatrixShapesMatch(
              inst, result_type, input_type, true, swap_row_col))
        return error;

      if (opcode == spv::Op::OpCooperativeMatrixConvertNV) {
        if (_.FindDef(result_type)->GetOperandAs<uint32_t>(1) !=
            _.FindDef(input_type)->GetOperandAs<uint32_t>(1)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Result Type and Matrix component types mismatch: "
                 << spvOpcodeString(opcode);
        }
      }

      if (opcode == spv::Op::OpCooperativeMatrixTransposeNV) {
        if (!_.IsCooperativeMatrixBType(result_type)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "Result Type must have UseB: " << spvOpcodeString(opcode);
        }
      }
      break;
    }

    case spv::Op::OpBitCastArrayQCOM: {
      const auto result_type_inst = _.FindDef(inst->type_id());
      const auto source = _.FindDef(inst->GetOperandAs<uint32_t>(2u));
      const auto source_type_inst = _.FindDef(source->type_id());

      // Are the input and the result arrays?
      if (result_type_inst->opcode() != spv::Op::OpTypeArray ||
          source_type_inst->opcode() != spv::Op::OpTypeArray) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Opcode " << spvOpcodeString(inst->opcode())
               << " requires OpTypeArray operands for the input and the "
                  "result.";
      }

      const auto source_elt_type = _.GetComponentType(source_type_inst->id());
      const auto result_elt_type = _.GetComponentType(result_type_inst->id());

      if (!_.IsIntNOrFP32OrFP16<32>(source_elt_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Opcode " << spvOpcodeString(inst->opcode())
               << " requires the source element type be one of 32-bit "
                  "OpTypeInt "
                  "(signed/unsigned), 32-bit OpTypeFloat and 16-bit "
                  "OpTypeFloat";
      }

      if (!_.IsIntNOrFP32OrFP16<32>(result_elt_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Opcode " << spvOpcodeString(inst->opcode())
               << " requires the result element type be one of 32-bit "
                  "OpTypeInt "
                  "(signed/unsigned), 32-bit OpTypeFloat and 16-bit "
                  "OpTypeFloat";
      }

      unsigned src_arr_len_id = source_type_inst->GetOperandAs<unsigned>(2u);
      unsigned res_arr_len_id = result_type_inst->GetOperandAs<unsigned>(2u);

      // Are the input and result element types compatible?
      unsigned src_arr_len = UINT_MAX, res_arr_len = UINT_MAX;
      bool src_arr_len_status =
          _.GetConstantValueAs<unsigned>(src_arr_len_id, src_arr_len);
      bool res_arr_len_status =
          _.GetConstantValueAs<unsigned>(res_arr_len_id, res_arr_len);

      bool is_src_arr_len_spec_const =
          spvOpcodeIsSpecConstant(_.FindDef(src_arr_len_id)->opcode());
      bool is_res_arr_len_spec_const =
          spvOpcodeIsSpecConstant(_.FindDef(res_arr_len_id)->opcode());

      unsigned source_bitlen = _.GetBitWidth(source_elt_type) * src_arr_len;
      unsigned result_bitlen = _.GetBitWidth(result_elt_type) * res_arr_len;
      if (!is_src_arr_len_spec_const && !is_res_arr_len_spec_const &&
          (!src_arr_len_status || !res_arr_len_status ||
           source_bitlen != result_bitlen)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Opcode " << spvOpcodeString(inst->opcode())
               << " requires source and result types be compatible for "
                  "conversion.";
      }
      break;
    }

    default:
      break;
  }

  if (_.HasCapability(spv::Capability::Shader)) {
    switch (inst->opcode()) {
      case spv::Op::OpConvertFToU:
      case spv::Op::OpConvertFToS:
      case spv::Op::OpConvertSToF:
      case spv::Op::OpConvertUToF:
      case spv::Op::OpBitcast:
        if (_.ContainsLimitedUseIntOrFloatType(inst->type_id()) ||
            _.ContainsLimitedUseIntOrFloatType(_.GetOperandTypeId(inst, 2u))) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << "8- or 16-bit types can only be used with width-only "
                    "conversions";
        }
        break;
      default:
        break;
    }
  }

  return SPV_SUCCESS;
}

}  // namespace val
}  // namespace spvtools
