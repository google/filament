// Copyright (c) 2017 Google Inc.
// Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights
// reserved.
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

// Validates correctness of atomic SPIR-V instructions.

#include "source/val/validate.h"

#include "source/diagnostic.h"
#include "source/opcode.h"
#include "source/spirv_target_env.h"
#include "source/util/bitutils.h"
#include "source/val/instruction.h"
#include "source/val/validate_memory_semantics.h"
#include "source/val/validate_scopes.h"
#include "source/val/validation_state.h"

namespace {

bool IsStorageClassAllowedByUniversalRules(uint32_t storage_class) {
  switch (storage_class) {
    case SpvStorageClassUniform:
    case SpvStorageClassStorageBuffer:
    case SpvStorageClassWorkgroup:
    case SpvStorageClassCrossWorkgroup:
    case SpvStorageClassGeneric:
    case SpvStorageClassAtomicCounter:
    case SpvStorageClassImage:
    case SpvStorageClassFunction:
    case SpvStorageClassPhysicalStorageBufferEXT:
      return true;
      break;
    default:
      return false;
  }
}

bool HasReturnType(uint32_t opcode) {
  switch (opcode) {
    case SpvOpAtomicStore:
    case SpvOpAtomicFlagClear:
      return false;
      break;
    default:
      return true;
  }
}

bool HasOnlyFloatReturnType(uint32_t opcode) {
  switch (opcode) {
    case SpvOpAtomicFAddEXT:
    case SpvOpAtomicFMinEXT:
    case SpvOpAtomicFMaxEXT:
      return true;
      break;
    default:
      return false;
  }
}

bool HasOnlyIntReturnType(uint32_t opcode) {
  switch (opcode) {
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
      break;
    default:
      return false;
  }
}

bool HasIntOrFloatReturnType(uint32_t opcode) {
  switch (opcode) {
    case SpvOpAtomicLoad:
    case SpvOpAtomicExchange:
      return true;
      break;
    default:
      return false;
  }
}

bool HasOnlyBoolReturnType(uint32_t opcode) {
  switch (opcode) {
    case SpvOpAtomicFlagTestAndSet:
      return true;
      break;
    default:
      return false;
  }
}

}  // namespace

namespace spvtools {
namespace val {

// Validates correctness of atomic instructions.
spv_result_t AtomicsPass(ValidationState_t& _, const Instruction* inst) {
  const SpvOp opcode = inst->opcode();
  switch (opcode) {
    case SpvOpAtomicLoad:
    case SpvOpAtomicStore:
    case SpvOpAtomicExchange:
    case SpvOpAtomicFAddEXT:
    case SpvOpAtomicCompareExchange:
    case SpvOpAtomicCompareExchangeWeak:
    case SpvOpAtomicIIncrement:
    case SpvOpAtomicIDecrement:
    case SpvOpAtomicIAdd:
    case SpvOpAtomicISub:
    case SpvOpAtomicSMin:
    case SpvOpAtomicUMin:
    case SpvOpAtomicFMinEXT:
    case SpvOpAtomicSMax:
    case SpvOpAtomicUMax:
    case SpvOpAtomicFMaxEXT:
    case SpvOpAtomicAnd:
    case SpvOpAtomicOr:
    case SpvOpAtomicXor:
    case SpvOpAtomicFlagTestAndSet:
    case SpvOpAtomicFlagClear: {
      const uint32_t result_type = inst->type_id();

      // All current atomics only are scalar result
      // Validate return type first so can just check if pointer type is same
      // (if applicable)
      if (HasReturnType(opcode)) {
        if (HasOnlyFloatReturnType(opcode) &&
            !_.IsFloatScalarType(result_type)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << spvOpcodeString(opcode)
                 << ": expected Result Type to be float scalar type";
        } else if (HasOnlyIntReturnType(opcode) &&
                   !_.IsIntScalarType(result_type)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << spvOpcodeString(opcode)
                 << ": expected Result Type to be integer scalar type";
        } else if (HasIntOrFloatReturnType(opcode) &&
                   !_.IsFloatScalarType(result_type) &&
                   !_.IsIntScalarType(result_type)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << spvOpcodeString(opcode)
                 << ": expected Result Type to be integer or float scalar type";
        } else if (HasOnlyBoolReturnType(opcode) &&
                   !_.IsBoolScalarType(result_type)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << spvOpcodeString(opcode)
                 << ": expected Result Type to be bool scalar type";
        }
      }

      uint32_t operand_index = HasReturnType(opcode) ? 2 : 0;
      const uint32_t pointer_type = _.GetOperandTypeId(inst, operand_index++);
      uint32_t data_type = 0;
      uint32_t storage_class = 0;
      if (!_.GetPointerTypeInfo(pointer_type, &data_type, &storage_class)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << spvOpcodeString(opcode)
               << ": expected Pointer to be of type OpTypePointer";
      }

      // Can't use result_type because OpAtomicStore doesn't have a result
      if (_.GetBitWidth(data_type) == 64 && _.IsIntScalarType(data_type) &&
          !_.HasCapability(SpvCapabilityInt64Atomics)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << spvOpcodeString(opcode)
               << ": 64-bit atomics require the Int64Atomics capability";
      }

      // Validate storage class against universal rules
      if (!IsStorageClassAllowedByUniversalRules(storage_class)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << spvOpcodeString(opcode)
               << ": storage class forbidden by universal validation rules.";
      }

      // Then Shader rules
      if (_.HasCapability(SpvCapabilityShader)) {
        // Vulkan environment rule
        if (spvIsVulkanEnv(_.context()->target_env)) {
          if ((storage_class != SpvStorageClassUniform) &&
              (storage_class != SpvStorageClassStorageBuffer) &&
              (storage_class != SpvStorageClassWorkgroup) &&
              (storage_class != SpvStorageClassImage) &&
              (storage_class != SpvStorageClassPhysicalStorageBuffer)) {
            return _.diag(SPV_ERROR_INVALID_DATA, inst)
                   << _.VkErrorID(4686) << spvOpcodeString(opcode)
                   << ": Vulkan spec only allows storage classes for atomic to "
                      "be: Uniform, Workgroup, Image, StorageBuffer, or "
                      "PhysicalStorageBuffer.";
          }
        } else if (storage_class == SpvStorageClassFunction) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << spvOpcodeString(opcode)
                 << ": Function storage class forbidden when the Shader "
                    "capability is declared.";
        }

        if (opcode == SpvOpAtomicFAddEXT) {
          // result type being float checked already
          if ((_.GetBitWidth(result_type) == 16) &&
              (!_.HasCapability(SpvCapabilityAtomicFloat16AddEXT))) {
            return _.diag(SPV_ERROR_INVALID_DATA, inst)
                   << spvOpcodeString(opcode)
                   << ": float add atomics require the AtomicFloat32AddEXT "
                      "capability";
          }
          if ((_.GetBitWidth(result_type) == 32) &&
              (!_.HasCapability(SpvCapabilityAtomicFloat32AddEXT))) {
            return _.diag(SPV_ERROR_INVALID_DATA, inst)
                   << spvOpcodeString(opcode)
                   << ": float add atomics require the AtomicFloat32AddEXT "
                      "capability";
          }
          if ((_.GetBitWidth(result_type) == 64) &&
              (!_.HasCapability(SpvCapabilityAtomicFloat64AddEXT))) {
            return _.diag(SPV_ERROR_INVALID_DATA, inst)
                   << spvOpcodeString(opcode)
                   << ": float add atomics require the AtomicFloat64AddEXT "
                      "capability";
          }
        } else if (opcode == SpvOpAtomicFMinEXT ||
                   opcode == SpvOpAtomicFMaxEXT) {
          if ((_.GetBitWidth(result_type) == 16) &&
              (!_.HasCapability(SpvCapabilityAtomicFloat16MinMaxEXT))) {
            return _.diag(SPV_ERROR_INVALID_DATA, inst)
                   << spvOpcodeString(opcode)
                   << ": float min/max atomics require the "
                      "AtomicFloat16MinMaxEXT capability";
          }
          if ((_.GetBitWidth(result_type) == 32) &&
              (!_.HasCapability(SpvCapabilityAtomicFloat32MinMaxEXT))) {
            return _.diag(SPV_ERROR_INVALID_DATA, inst)
                   << spvOpcodeString(opcode)
                   << ": float min/max atomics require the "
                      "AtomicFloat32MinMaxEXT capability";
          }
          if ((_.GetBitWidth(result_type) == 64) &&
              (!_.HasCapability(SpvCapabilityAtomicFloat64MinMaxEXT))) {
            return _.diag(SPV_ERROR_INVALID_DATA, inst)
                   << spvOpcodeString(opcode)
                   << ": float min/max atomics require the "
                      "AtomicFloat64MinMaxEXT capability";
          }
        }
      }

      // And finally OpenCL environment rules
      if (spvIsOpenCLEnv(_.context()->target_env)) {
        if ((storage_class != SpvStorageClassFunction) &&
            (storage_class != SpvStorageClassWorkgroup) &&
            (storage_class != SpvStorageClassCrossWorkgroup) &&
            (storage_class != SpvStorageClassGeneric)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << spvOpcodeString(opcode)
                 << ": storage class must be Function, Workgroup, "
                    "CrossWorkGroup or Generic in the OpenCL environment.";
        }

        if (_.context()->target_env == SPV_ENV_OPENCL_1_2) {
          if (storage_class == SpvStorageClassGeneric) {
            return _.diag(SPV_ERROR_INVALID_DATA, inst)
                   << "Storage class cannot be Generic in OpenCL 1.2 "
                      "environment";
          }
        }
      }

      // If result and pointer type are different, need to do special check here
      if (opcode == SpvOpAtomicFlagTestAndSet ||
          opcode == SpvOpAtomicFlagClear) {
        if (!_.IsIntScalarType(data_type) || _.GetBitWidth(data_type) != 32) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << spvOpcodeString(opcode)
                 << ": expected Pointer to point to a value of 32-bit integer "
                    "type";
        }
      } else if (opcode == SpvOpAtomicStore) {
        if (!_.IsFloatScalarType(data_type) && !_.IsIntScalarType(data_type)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << spvOpcodeString(opcode)
                 << ": expected Pointer to be a pointer to integer or float "
                 << "scalar type";
        }
      } else if (data_type != result_type) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << spvOpcodeString(opcode)
               << ": expected Pointer to point to a value of type Result "
                  "Type";
      }

      auto memory_scope = inst->GetOperandAs<const uint32_t>(operand_index++);
      if (auto error = ValidateMemoryScope(_, inst, memory_scope)) {
        return error;
      }

      const auto equal_semantics_index = operand_index++;
      if (auto error = ValidateMemorySemantics(_, inst, equal_semantics_index,
                                               memory_scope))
        return error;

      if (opcode == SpvOpAtomicCompareExchange ||
          opcode == SpvOpAtomicCompareExchangeWeak) {
        const auto unequal_semantics_index = operand_index++;
        if (auto error = ValidateMemorySemantics(
                _, inst, unequal_semantics_index, memory_scope))
          return error;

        // Volatile bits must match for equal and unequal semantics. Previous
        // checks guarantee they are 32-bit constants, but we need to recheck
        // whether they are evaluatable constants.
        bool is_int32 = false;
        bool is_equal_const = false;
        bool is_unequal_const = false;
        uint32_t equal_value = 0;
        uint32_t unequal_value = 0;
        std::tie(is_int32, is_equal_const, equal_value) = _.EvalInt32IfConst(
            inst->GetOperandAs<uint32_t>(equal_semantics_index));
        std::tie(is_int32, is_unequal_const, unequal_value) =
            _.EvalInt32IfConst(
                inst->GetOperandAs<uint32_t>(unequal_semantics_index));
        if (is_equal_const && is_unequal_const &&
            ((equal_value & SpvMemorySemanticsVolatileMask) ^
             (unequal_value & SpvMemorySemanticsVolatileMask))) {
          return _.diag(SPV_ERROR_INVALID_ID, inst)
                 << "Volatile mask setting must match for Equal and Unequal "
                    "memory semantics";
        }
      }

      if (opcode == SpvOpAtomicStore) {
        const uint32_t value_type = _.GetOperandTypeId(inst, 3);
        if (value_type != data_type) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << spvOpcodeString(opcode)
                 << ": expected Value type and the type pointed to by "
                    "Pointer to be the same";
        }
      } else if (opcode != SpvOpAtomicLoad && opcode != SpvOpAtomicIIncrement &&
                 opcode != SpvOpAtomicIDecrement &&
                 opcode != SpvOpAtomicFlagTestAndSet &&
                 opcode != SpvOpAtomicFlagClear) {
        const uint32_t value_type = _.GetOperandTypeId(inst, operand_index++);
        if (value_type != result_type) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << spvOpcodeString(opcode)
                 << ": expected Value to be of type Result Type";
        }
      }

      if (opcode == SpvOpAtomicCompareExchange ||
          opcode == SpvOpAtomicCompareExchangeWeak) {
        const uint32_t comparator_type =
            _.GetOperandTypeId(inst, operand_index++);
        if (comparator_type != result_type) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << spvOpcodeString(opcode)
                 << ": expected Comparator to be of type Result Type";
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
