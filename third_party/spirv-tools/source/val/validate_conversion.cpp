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

namespace {

bool IsNumericScalarOrVectorType(ValidationState_t& _, uint32_t type_id) {
  return _.IsFloatScalarType(type_id) || _.IsFloatVectorType(type_id) ||
         _.IsIntScalarType(type_id) || _.IsIntVectorType(type_id);
}

bool IsIEEEOrAlternativeFloatTypeAllowedForOCPMicroscalingFConvert(
    ValidationState_t& _, uint32_t type_id) {
  const uint32_t component_type = _.GetComponentType(type_id);
  const Instruction* inst = _.FindDef(component_type);
  if (!inst || inst->opcode() != spv::Op::OpTypeFloat) return false;

  if (inst->words().size() <= 3) return true;

  const auto encoding = inst->GetOperandAs<spv::FPEncoding>(2);
  return encoding == spv::FPEncoding::Float8E4M3EXT ||
         encoding == spv::FPEncoding::Float8E5M2EXT ||
         encoding == spv::FPEncoding::BFloat16KHR;
}

spv_result_t ValidateVulkanOCPMicroscalingFloatIntConversion(
    ValidationState_t& _, const Instruction* inst, uint32_t float_type) {
  if (spvIsVulkanEnv(_.context()->target_env) &&
      _.ContainsOCPMicroscalingType(float_type)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << _.VkErrorID(12465) << spvOpcodeString(inst->opcode())
           << " must not consume or produce OCP microscaling types in the "
              "Vulkan environment.";
  }
  return SPV_SUCCESS;
}

bool HasCoopMatTranspose(ValidationState_t& _, uint32_t id) {
  return _.HasDecoration(id, spv::Decoration::CooperativeMatrixTransposeEXT);
}

}  // namespace

spv_result_t ValidateShaderBitWidth(ValidationState_t& _,
                                    const Instruction* inst) {
  if (_.HasCapability(spv::Capability::Shader)) {
    if (_.ContainsLimitedUseIntOrFloatType(inst->type_id()) ||
        _.ContainsLimitedUseIntOrFloatType(_.GetOperandTypeId(inst, 2u))) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "8- or 16-bit types can only be used with width-only "
                "conversions";
    }
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateConvertFToU(ValidationState_t& _, const Instruction* inst,
                                 uint32_t operand_index = 2) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
  if (!_.IsUnsignedIntScalarType(result_type) &&
      !_.IsUnsignedIntVectorType(result_type) &&
      !_.IsUnsignedIntCooperativeMatrixType(result_type) &&
      !_.IsUnsignedIntCooperativeVectorNVType(result_type))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected unsigned int scalar or vector type as Result Type: "
           << spvOpcodeString(opcode);

  const uint32_t input_type = _.GetOperandTypeId(inst, operand_index);
  if (!input_type ||
      (!_.IsFloatScalarType(input_type) && !_.IsFloatVectorType(input_type) &&
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
        _.CooperativeMatrixShapesMatch(inst, result_type, input_type, true,
                                       HasCoopMatTranspose(_, inst->id()));
    if (ret != SPV_SUCCESS) return ret;
  } else {
    if (_.GetDimension(result_type) != _.GetDimension(input_type))
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Expected input to have the same dimension as Result Type: "
             << spvOpcodeString(opcode);
  }

  if (auto error =
          ValidateVulkanOCPMicroscalingFloatIntConversion(_, inst, input_type))
    return error;

  if (auto error = ValidateShaderBitWidth(_, inst)) return error;

  return SPV_SUCCESS;
}

spv_result_t ValidateConvertFToS(ValidationState_t& _, const Instruction* inst,
                                 uint32_t operand_index = 2) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
  if (!_.IsIntScalarType(result_type) && !_.IsIntVectorType(result_type) &&
      !_.IsIntCooperativeMatrixType(result_type) &&
      !_.IsIntCooperativeVectorNVType(result_type))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected int scalar or vector type as Result Type: "
           << spvOpcodeString(opcode);

  const uint32_t input_type = _.GetOperandTypeId(inst, operand_index);
  if (!input_type ||
      (!_.IsFloatScalarType(input_type) && !_.IsFloatVectorType(input_type) &&
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
        _.CooperativeMatrixShapesMatch(inst, result_type, input_type, true,
                                       HasCoopMatTranspose(_, inst->id()));
    if (ret != SPV_SUCCESS) return ret;
  } else {
    if (_.GetDimension(result_type) != _.GetDimension(input_type))
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Expected input to have the same dimension as Result Type: "
             << spvOpcodeString(opcode);
  }

  if (auto error =
          ValidateVulkanOCPMicroscalingFloatIntConversion(_, inst, input_type))
    return error;

  if (auto error = ValidateShaderBitWidth(_, inst)) return error;

  return SPV_SUCCESS;
}

spv_result_t ValidateConvertIntToF(ValidationState_t& _,
                                   const Instruction* inst,
                                   uint32_t operand_index = 2) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
  if (!_.IsFloatScalarType(result_type) && !_.IsFloatVectorType(result_type) &&
      !_.IsFloatCooperativeMatrixType(result_type) &&
      !_.IsFloatCooperativeVectorNVType(result_type))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected float scalar or vector type as Result Type: "
           << spvOpcodeString(opcode);

  const uint32_t input_type = _.GetOperandTypeId(inst, operand_index);
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
        _.CooperativeMatrixShapesMatch(inst, result_type, input_type, true,
                                       HasCoopMatTranspose(_, inst->id()));
    if (ret != SPV_SUCCESS) return ret;
  } else {
    if (_.GetDimension(result_type) != _.GetDimension(input_type))
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Expected input to have the same dimension as Result Type: "
             << spvOpcodeString(opcode);
  }

  if (auto error =
          ValidateVulkanOCPMicroscalingFloatIntConversion(_, inst, result_type))
    return error;

  if (auto error = ValidateShaderBitWidth(_, inst)) return error;

  return SPV_SUCCESS;
}

spv_result_t ValidateUConvert(ValidationState_t& _, const Instruction* inst,
                              uint32_t operand_index = 2) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
  if (!_.IsUnsignedIntScalarType(result_type) &&
      !_.IsUnsignedIntVectorType(result_type) &&
      !_.IsUnsignedIntCooperativeMatrixType(result_type) &&
      !_.IsUnsignedIntCooperativeVectorNVType(result_type))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected unsigned int scalar or vector type as Result Type: "
           << spvOpcodeString(opcode);

  const uint32_t input_type = _.GetOperandTypeId(inst, operand_index);
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
        _.CooperativeMatrixShapesMatch(inst, result_type, input_type, true,
                                       HasCoopMatTranspose(_, inst->id()));
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
  return SPV_SUCCESS;
}

spv_result_t ValidateSConvert(ValidationState_t& _, const Instruction* inst,
                              uint32_t operand_index = 2) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
  if (!_.IsIntScalarType(result_type) && !_.IsIntVectorType(result_type) &&
      !_.IsIntCooperativeMatrixType(result_type) &&
      !_.IsIntCooperativeVectorNVType(result_type))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected int scalar or vector type as Result Type: "
           << spvOpcodeString(opcode);

  const uint32_t input_type = _.GetOperandTypeId(inst, operand_index);
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
        _.CooperativeMatrixShapesMatch(inst, result_type, input_type, true,
                                       HasCoopMatTranspose(_, inst->id()));
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
  return SPV_SUCCESS;
}

spv_result_t ValidateFConvert(ValidationState_t& _, const Instruction* inst,
                              uint32_t operand_index = 2) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
  if (!_.IsFloatScalarType(result_type) && !_.IsFloatVectorType(result_type) &&
      !_.IsFloatCooperativeMatrixType(result_type) &&
      !_.IsFloatCooperativeVectorNVType(result_type))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected float scalar or vector type as Result Type: "
           << spvOpcodeString(opcode);

  const uint32_t input_type = _.GetOperandTypeId(inst, operand_index);
  if (!input_type ||
      (!_.IsFloatScalarType(input_type) && !_.IsFloatVectorType(input_type) &&
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
        _.CooperativeMatrixShapesMatch(inst, result_type, input_type, true,
                                       HasCoopMatTranspose(_, inst->id()));
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

  if (spvIsVulkanEnv(_.context()->target_env)) {
    if (_.ContainsOCPMicroscalingType(result_type)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << _.VkErrorID(12466) << spvOpcodeString(opcode)
             << " must not produce OCP microscaling types in the Vulkan "
                "environment.";
    }
    if (_.ContainsOCPMicroscalingType(input_type) &&
        !IsIEEEOrAlternativeFloatTypeAllowedForOCPMicroscalingFConvert(
            _, result_type)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << _.VkErrorID(12467) << spvOpcodeString(opcode)
             << " consuming an OCP microscaling type in the Vulkan "
                "environment must produce IEEE 754, Float8E4M3EXT, "
                "Float8E5M2EXT, or BFloat16KHR.";
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateQuantizeToF16(ValidationState_t& _,
                                   const Instruction* inst,
                                   uint32_t operand_index = 2) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
  if ((!_.IsFloatScalarType(result_type) &&
       !_.IsFloatVectorType(result_type)) ||
      _.GetBitWidth(result_type) != 32)
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected 32-bit float scalar or vector type as Result Type: "
           << spvOpcodeString(opcode);

  const uint32_t input_type = _.GetOperandTypeId(inst, operand_index);
  if (input_type != result_type)
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected input type to be equal to Result Type: "
           << spvOpcodeString(opcode);
  return SPV_SUCCESS;
}

spv_result_t ValidateConvertPtrToU(ValidationState_t& _,
                                   const Instruction* inst,
                                   uint32_t operand_index = 2) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
  const bool has_masked_gather_scatter =
      _.HasCapability(spv::Capability::MaskedGatherScatterINTEL);

  bool valid_result_type = _.IsUnsignedIntScalarType(result_type);
  if (!valid_result_type && has_masked_gather_scatter) {
    valid_result_type = _.IsUnsignedIntVectorType(result_type);
  }

  if (!valid_result_type) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected unsigned int scalar type as Result Type"
           << (has_masked_gather_scatter ? " (or vector of unsigned int with "
                                           "MaskedGatherScatterINTEL)"
                                         : "")
           << ": " << spvOpcodeString(opcode);
  }

  const uint32_t input_type = _.GetOperandTypeId(inst, operand_index);

  bool valid_input_type = _.IsPointerType(input_type);
  if (!valid_input_type && has_masked_gather_scatter && input_type) {
    if (_.IsVectorType(input_type)) {
      const uint32_t component_type = _.GetComponentType(input_type);
      valid_input_type = _.IsPointerType(component_type);
    }
  }

  if (!valid_input_type) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected input to be a pointer"
           << (has_masked_gather_scatter
                   ? " (or vector of pointers with MaskedGatherScatterINTEL)"
                   : "")
           << ": " << spvOpcodeString(opcode);
  }

  if (has_masked_gather_scatter && _.IsVectorType(result_type)) {
    if (!_.IsVectorType(input_type)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Expected input to be a vector when Result Type is a vector: "
             << spvOpcodeString(opcode);
    }
    if (_.GetDimension(result_type) != _.GetDimension(input_type)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Expected input to have the same dimension as Result Type: "
             << spvOpcodeString(opcode);
    }
  }

  if (_.addressing_model() == spv::AddressingModel::Logical)
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Logical addressing not supported: " << spvOpcodeString(opcode);

  if (_.addressing_model() == spv::AddressingModel::PhysicalStorageBuffer64) {
    uint32_t ptr_type = input_type;
    if (_.IsVectorType(input_type)) {
      ptr_type = _.GetComponentType(input_type);
    }
    spv::StorageClass input_storage_class;
    uint32_t input_data_type = 0;
    _.GetPointerTypeInfo(ptr_type, &input_data_type, &input_storage_class);
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
  return SPV_SUCCESS;
}

spv_result_t ValidateSatConvertInt(ValidationState_t& _,
                                   const Instruction* inst) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
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
  return SPV_SUCCESS;
}

spv_result_t ValidateConvertUToPtr(ValidationState_t& _,
                                   const Instruction* inst,
                                   uint32_t operand_index = 2) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
  const bool has_masked_gather_scatter =
      _.HasCapability(spv::Capability::MaskedGatherScatterINTEL);

  bool valid_result_type = _.IsPointerType(result_type);
  if (!valid_result_type && has_masked_gather_scatter) {
    if (_.IsVectorType(result_type)) {
      const uint32_t component_type = _.GetComponentType(result_type);
      valid_result_type = _.IsPointerType(component_type);
    }
  }

  if (!valid_result_type) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Result Type to be a pointer"
           << (has_masked_gather_scatter
                   ? " (or vector of pointers with MaskedGatherScatterINTEL)"
                   : "")
           << ": " << spvOpcodeString(opcode);
  }

  const uint32_t input_type = _.GetOperandTypeId(inst, operand_index);

  bool valid_input_type = input_type && _.IsIntScalarType(input_type);
  if (!valid_input_type && has_masked_gather_scatter && input_type) {
    valid_input_type = _.IsIntVectorType(input_type);
  }

  if (!valid_input_type) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected int scalar as input"
           << (has_masked_gather_scatter
                   ? " (or vector of int with MaskedGatherScatterINTEL)"
                   : "")
           << ": " << spvOpcodeString(opcode);
  }

  if (has_masked_gather_scatter && _.IsVectorType(result_type)) {
    if (!_.IsVectorType(input_type)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Expected input to be a vector when Result Type is a vector: "
             << spvOpcodeString(opcode);
    }
    if (_.GetDimension(result_type) != _.GetDimension(input_type)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Expected input to have the same dimension as Result Type: "
             << spvOpcodeString(opcode);
    }
  }

  if (_.addressing_model() == spv::AddressingModel::Logical)
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Logical addressing not supported: " << spvOpcodeString(opcode);

  if (_.addressing_model() == spv::AddressingModel::PhysicalStorageBuffer64) {
    uint32_t ptr_type = result_type;
    if (_.IsVectorType(result_type)) {
      ptr_type = _.GetComponentType(result_type);
    }
    spv::StorageClass result_storage_class;
    uint32_t result_data_type = 0;
    _.GetPointerTypeInfo(ptr_type, &result_data_type, &result_storage_class);
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
  return SPV_SUCCESS;
}

spv_result_t ValidatePtrCastToGeneric(ValidationState_t& _,
                                      const Instruction* inst,
                                      uint32_t operand_index = 2) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
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

  const uint32_t input_type = _.GetOperandTypeId(inst, operand_index);
  spv::StorageClass input_storage_class;
  uint32_t input_data_type = 0;
  if (!_.GetPointerTypeInfo(input_type, &input_data_type, &input_storage_class))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected input to be a pointer: " << spvOpcodeString(opcode);

  if (input_storage_class != spv::StorageClass::Workgroup &&
      input_storage_class != spv::StorageClass::CrossWorkgroup &&
      input_storage_class != spv::StorageClass::Function &&
      input_storage_class != spv::StorageClass::CodeSectionINTEL)
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected input to have storage class Workgroup, "
           << "CrossWorkgroup, Function or CodeSectionINTEL: "
           << spvOpcodeString(opcode);

  if (result_data_type != input_data_type)
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected input and Result Type to point to the same type: "
           << spvOpcodeString(opcode);
  return SPV_SUCCESS;
}

spv_result_t ValidateGenericCastToPtr(ValidationState_t& _,
                                      const Instruction* inst,
                                      uint32_t operand_index = 2) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
  spv::StorageClass result_storage_class;
  uint32_t result_data_type = 0;
  if (!_.GetPointerTypeInfo(result_type, &result_data_type,
                            &result_storage_class))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Result Type to be a pointer: "
           << spvOpcodeString(opcode);

  if (result_storage_class != spv::StorageClass::Workgroup &&
      result_storage_class != spv::StorageClass::CrossWorkgroup &&
      result_storage_class != spv::StorageClass::Function &&
      result_storage_class != spv::StorageClass::CodeSectionINTEL)
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Result Type to have storage class Workgroup, "
           << "CrossWorkgroup, Function or CodeSectionINTEL: "
           << spvOpcodeString(opcode);

  const uint32_t input_type = _.GetOperandTypeId(inst, operand_index);
  spv::StorageClass input_storage_class;
  uint32_t input_data_type = 0;
  if (!_.GetPointerTypeInfo(input_type, &input_data_type, &input_storage_class))
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
  return SPV_SUCCESS;
}

spv_result_t ValidateGenericCastToPtrExplicit(ValidationState_t& _,
                                              const Instruction* inst) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
  spv::StorageClass result_storage_class;
  uint32_t result_data_type = 0;
  if (!_.GetPointerTypeInfo(result_type, &result_data_type,
                            &result_storage_class))
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Result Type to be a pointer: "
           << spvOpcodeString(opcode);

  const auto target_storage_class = inst->GetOperandAs<spv::StorageClass>(3);
  if (result_storage_class != target_storage_class)
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Result Type to be of target storage class: "
           << spvOpcodeString(opcode);

  const uint32_t input_type = _.GetOperandTypeId(inst, 2);
  spv::StorageClass input_storage_class;
  uint32_t input_data_type = 0;
  if (!_.GetPointerTypeInfo(input_type, &input_data_type, &input_storage_class))
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
      target_storage_class != spv::StorageClass::Function &&
      target_storage_class != spv::StorageClass::CodeSectionINTEL)
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected target storage class to be Workgroup, "
           << "CrossWorkgroup, Function or CodeSectionINTEL: "
           << spvOpcodeString(opcode);
  return SPV_SUCCESS;
}

spv_result_t ValidateBitcast(ValidationState_t& _, const Instruction* inst,
                             uint32_t operand_index = 2) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
  const uint32_t input_type = _.GetOperandTypeId(inst, operand_index);
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
      !_.IsFloatScalarType(result_type) && !_.IsFloatVectorType(result_type))
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
                                                      input_type, false, false);
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

  if (auto error = ValidateShaderBitWidth(_, inst)) return error;

  return SPV_SUCCESS;
}

spv_result_t ValidateConvertUToAccelerationStructure(ValidationState_t& _,
                                                     const Instruction* inst) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
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
  return SPV_SUCCESS;
}

spv_result_t ValidateBitcastExtract(ValidationState_t& _,
                                    const Instruction* inst) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
  const uint32_t base_type = _.GetOperandTypeId(inst, 2);
  const uint32_t offset_type = _.GetOperandTypeId(inst, 3);

  if (!IsNumericScalarOrVectorType(_, result_type)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << _.VkErrorID(12468)
           << "Expected Result Type to be a numerical scalar or vector type: "
           << spvOpcodeString(opcode);
  }

  if (spvIsVulkanEnv(_.context()->target_env) &&
      !_.ContainsOCPMicroscalingNonByteType(result_type)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << _.VkErrorID(12468)
           << "Expected Result Type to be a Float4EXT or Float6EXT type in "
              "the Vulkan environment: "
           << spvOpcodeString(opcode);
  }

  if (!base_type || !IsNumericScalarOrVectorType(_, base_type)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << _.VkErrorID(12469)
           << "Expected Base to be a numerical scalar or vector type: "
           << spvOpcodeString(opcode);
  }

  if (spvIsVulkanEnv(_.context()->target_env) &&
      !_.IsIntScalarType(base_type) && !_.IsIntVectorType(base_type)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << _.VkErrorID(12469)
           << "Expected Base to be an integer scalar or vector type in the "
              "Vulkan environment: "
           << spvOpcodeString(opcode);
  }

  if (_.GetDimension(result_type) != _.GetDimension(base_type)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Base to have the same dimension as Result Type: "
           << spvOpcodeString(opcode);
  }

  if (_.GetBitWidth(base_type) <= _.GetBitWidth(result_type)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Base component bit width to be greater than Result "
              "Type component bit width: "
           << spvOpcodeString(opcode);
  }

  if (!offset_type || !_.IsIntScalarType(offset_type)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected Offset to be a scalar integer type: "
           << spvOpcodeString(opcode);
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateCooperativeMatrix(ValidationState_t& _,
                                       const Instruction* inst) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
  if (!_.IsCooperativeMatrixKHRType(result_type)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected OpTypeCooperativeMatrixKHR Result Type: "
           << spvOpcodeString(opcode);
  }
  const uint32_t input_type = _.GetOperandTypeId(inst, 2);
  if (!_.IsCooperativeMatrixKHRType(input_type)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Expected OpTypeCooperativeMatrixKHR type for Matrix input: "
           << spvOpcodeString(opcode);
  }

  const bool has_transpose_decoration = HasCoopMatTranspose(_, inst->id());
  const bool swap_row_col = opcode == spv::Op::OpCooperativeMatrixTransposeNV ||
                            has_transpose_decoration;
  if (auto error = _.CooperativeMatrixShapesMatch(inst, result_type, input_type,
                                                  true, swap_row_col))
    return error;

  if (opcode == spv::Op::OpCooperativeMatrixConvertUseEXT) {
    auto result_comp_type_id =
        _.FindDef(result_type)->GetOperandAs<uint32_t>(1);
    auto input_comp_type_id = _.FindDef(input_type)->GetOperandAs<uint32_t>(1);
    auto result_comp_type = _.FindDef(result_comp_type_id);
    auto input_comp_type = _.FindDef(input_comp_type_id);

    const bool same_component_type = result_comp_type_id == input_comp_type_id;
    const bool signedness_only_difference =
        result_comp_type->opcode() == spv::Op::OpTypeInt &&
        input_comp_type->opcode() == spv::Op::OpTypeInt &&
        result_comp_type->word(2) == input_comp_type->word(2) &&
        result_comp_type->word(3) != input_comp_type->word(3);
    if (!same_component_type && !signedness_only_difference) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Result Type and Matrix component types mismatch: "
             << spvOpcodeString(opcode);
    }

    const auto result_use_id =
        _.FindDef(result_type)->GetOperandAs<uint32_t>(5);
    const auto input_use_id = _.FindDef(input_type)->GetOperandAs<uint32_t>(5);
    const auto result_use_eval = _.EvalInt32IfConst(result_use_id);
    const auto input_use_eval = _.EvalInt32IfConst(input_use_id);
    const bool result_is_const_int32 = std::get<1>(result_use_eval);
    const bool input_is_const_int32 = std::get<1>(input_use_eval);
    const uint32_t result_use = std::get<2>(result_use_eval);
    const uint32_t input_use = std::get<2>(input_use_eval);

    const auto is_accumulator = [](uint32_t use) {
      return use == uint32_t(spv::CooperativeMatrixUse::MatrixAccumulatorKHR);
    };
    const auto is_a_or_b = [](uint32_t use) {
      return use == uint32_t(spv::CooperativeMatrixUse::MatrixAKHR) ||
             use == uint32_t(spv::CooperativeMatrixUse::MatrixBKHR);
    };

    bool invalid_use_change = result_use_id == input_use_id;
    if (_.HasCapability(spv::Capability::CooperativeMatrixConversionsEXT)) {
      if (result_is_const_int32 && input_is_const_int32) {
        invalid_use_change |=
            !((is_accumulator(input_use) && is_a_or_b(result_use)) ||
              (is_a_or_b(input_use) && is_accumulator(result_use)));
      } else {
        invalid_use_change |=
            (result_is_const_int32 && !is_accumulator(result_use) &&
             !is_a_or_b(result_use)) ||
            (input_is_const_int32 && !is_accumulator(input_use) &&
             !is_a_or_b(input_use));
      }
    } else {
      invalid_use_change |=
          (result_is_const_int32 && !is_a_or_b(result_use)) ||
          (input_is_const_int32 && !is_accumulator(input_use));
    }

    if (invalid_use_change) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Matrix and Result Type must convert between "
                "MatrixAccumulatorKHR and MatrixAKHR or MatrixBKHR: "
             << spvOpcodeString(opcode);
    }
  }

  if (opcode == spv::Op::OpCooperativeMatrixTransposeNV) {
    const auto result_use_id =
        _.FindDef(result_type)->GetOperandAs<uint32_t>(5);
    const auto input_use_id = _.FindDef(input_type)->GetOperandAs<uint32_t>(5);
    const auto result_use_eval = _.EvalInt32IfConst(result_use_id);
    const auto input_use_eval = _.EvalInt32IfConst(input_use_id);
    const bool result_is_const_int32 = std::get<1>(result_use_eval);
    const bool input_is_const_int32 = std::get<1>(input_use_eval);
    const uint32_t result_use = std::get<2>(result_use_eval);
    const uint32_t input_use = std::get<2>(input_use_eval);
    if (result_use_id == input_use_id ||
        (result_is_const_int32 &&
         result_use != uint32_t(spv::CooperativeMatrixUse::MatrixBKHR)) ||
        (input_is_const_int32 &&
         input_use !=
             uint32_t(spv::CooperativeMatrixUse::MatrixAccumulatorKHR))) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Result Type must have UseB and Matrix must have "
                "UseAccumulator: "
             << spvOpcodeString(opcode);
    }
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateBitCastArray(ValidationState_t& _,
                                  const Instruction* inst) {
  const spv::Op opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();
  const auto result_type_inst = _.FindDef(result_type);
  const auto source = _.FindDef(inst->GetOperandAs<uint32_t>(2u));
  const auto source_type_inst = _.FindDef(source->type_id());

  // Are the input and the result arrays?
  if (result_type_inst->opcode() != spv::Op::OpTypeArray ||
      source_type_inst->opcode() != spv::Op::OpTypeArray) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Opcode " << spvOpcodeString(opcode)
           << " requires OpTypeArray operands for the input and the "
              "result.";
  }

  const auto source_elt_type = _.GetComponentType(source_type_inst->id());
  const auto result_elt_type = _.GetComponentType(result_type_inst->id());

  if (!_.IsIntNOrFP32OrFP16<32>(source_elt_type)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Opcode " << spvOpcodeString(opcode)
           << " requires the source element type be one of 32-bit "
              "OpTypeInt "
              "(signed/unsigned), 32-bit OpTypeFloat and 16-bit "
              "OpTypeFloat";
  }

  if (!_.IsIntNOrFP32OrFP16<32>(result_elt_type)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Opcode " << spvOpcodeString(opcode)
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
           << "Opcode " << spvOpcodeString(opcode)
           << " requires source and result types be compatible for "
              "conversion.";
  }
  return SPV_SUCCESS;
}

// Validates correctness of conversion instructions.
spv_result_t ConversionPass(ValidationState_t& _, const Instruction* inst) {
  switch (inst->opcode()) {
    case spv::Op::OpConvertFToU:
      return ValidateConvertFToU(_, inst);
    case spv::Op::OpConvertFToS:
      return ValidateConvertFToS(_, inst);
    case spv::Op::OpConvertSToF:
    case spv::Op::OpConvertUToF:
      return ValidateConvertIntToF(_, inst);
    case spv::Op::OpUConvert:
      return ValidateUConvert(_, inst);
    case spv::Op::OpSConvert:
      return ValidateSConvert(_, inst);
    case spv::Op::OpFConvert:
      return ValidateFConvert(_, inst);
    case spv::Op::OpQuantizeToF16:
      return ValidateQuantizeToF16(_, inst);
    case spv::Op::OpConvertPtrToU:
      return ValidateConvertPtrToU(_, inst);
    case spv::Op::OpSatConvertSToU:
    case spv::Op::OpSatConvertUToS:
      return ValidateSatConvertInt(_, inst);
    case spv::Op::OpConvertUToPtr:
      return ValidateConvertUToPtr(_, inst);
    case spv::Op::OpPtrCastToGeneric:
      return ValidatePtrCastToGeneric(_, inst);
    case spv::Op::OpGenericCastToPtr:
      return ValidateGenericCastToPtr(_, inst);
    case spv::Op::OpGenericCastToPtrExplicit:
      return ValidateGenericCastToPtrExplicit(_, inst);
    case spv::Op::OpBitcast:
      return ValidateBitcast(_, inst);
    case spv::Op::OpBitcastExtractEXT:
      return ValidateBitcastExtract(_, inst);
    case spv::Op::OpConvertUToAccelerationStructureKHR:
      return ValidateConvertUToAccelerationStructure(_, inst);
    case spv::Op::OpCooperativeMatrixConvertUseEXT:
    case spv::Op::OpCooperativeMatrixTransposeNV:
      return ValidateCooperativeMatrix(_, inst);
    case spv::Op::OpBitCastArrayQCOM:
      return ValidateBitCastArray(_, inst);

    case spv::Op::OpSpecConstantOp: {
      switch (inst->GetOperandAs<spv::Op>(2u)) {
        case spv::Op::OpUConvert:
          return ValidateUConvert(_, inst, 3);
        case spv::Op::OpSConvert:
          return ValidateSConvert(_, inst, 3);
        case spv::Op::OpFConvert:
          return ValidateFConvert(_, inst, 3);
        case spv::Op::OpConvertSToF:
        case spv::Op::OpConvertUToF:
          return ValidateConvertIntToF(_, inst, 3);
        case spv::Op::OpConvertFToS:
          return ValidateConvertFToS(_, inst, 3);
        case spv::Op::OpConvertFToU:
          return ValidateConvertFToU(_, inst, 3);
        case spv::Op::OpQuantizeToF16:
          return ValidateQuantizeToF16(_, inst, 3);
        case spv::Op::OpConvertPtrToU:
          return ValidateConvertPtrToU(_, inst, 3);
        case spv::Op::OpConvertUToPtr:
          return ValidateConvertUToPtr(_, inst, 3);
        case spv::Op::OpGenericCastToPtr:
          return ValidateGenericCastToPtr(_, inst, 3);
        case spv::Op::OpPtrCastToGeneric:
          return ValidatePtrCastToGeneric(_, inst, 3);
        case spv::Op::OpBitcast:
          return ValidateBitcast(_, inst, 3);
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
