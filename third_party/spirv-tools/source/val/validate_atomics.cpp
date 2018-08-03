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

// Validates correctness of atomic SPIR-V instructions.

#include "source/val/validate.h"

#include "source/diagnostic.h"
#include "source/opcode.h"
#include "source/spirv_target_env.h"
#include "source/util/bitutils.h"
#include "source/val/instruction.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {

// Validates Memory Scope operand.
spv_result_t ValidateMemoryScope(ValidationState_t& _, const Instruction* inst,
                                 uint32_t id) {
  const SpvOp opcode = inst->opcode();
  bool is_int32 = false, is_const_int32 = false;
  uint32_t value = 0;
  std::tie(is_int32, is_const_int32, value) = _.EvalInt32IfConst(id);

  if (!is_int32) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << spvOpcodeString(opcode) << ": expected Scope to be 32-bit int";
  }

  if (!is_const_int32) {
    return SPV_SUCCESS;
  }

#if 0
  // TODO(atgoo@github.com): this check fails Vulkan CTS, reenable once fixed.
  if (spvIsVulkanEnv(_.context()->target_env)) {
    if (value != SpvScopeDevice && value != SpvScopeWorkgroup &&
        value != SpvScopeInvocation) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << spvOpcodeString(opcode)
             << ": in Vulkan environment memory scope is limited to Device, "
                "Workgroup and Invocation";
    }
  }
#endif

  // TODO(atgoo@github.com) Add checks for OpenCL and OpenGL environments.

  return SPV_SUCCESS;
}

// Validates a Memory Semantics operand.
spv_result_t ValidateMemorySemantics(ValidationState_t& _,
                                     const Instruction* inst,
                                     uint32_t operand_index) {
  const SpvOp opcode = inst->opcode();
  bool is_int32 = false, is_const_int32 = false;
  uint32_t flags = 0;
  auto memory_semantics_id = inst->GetOperandAs<const uint32_t>(operand_index);
  std::tie(is_int32, is_const_int32, flags) =
      _.EvalInt32IfConst(memory_semantics_id);

  if (!is_int32) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << spvOpcodeString(opcode)
           << ": expected Memory Semantics to be 32-bit int";
  }

  if (!is_const_int32) {
    return SPV_SUCCESS;
  }

  if (spvtools::utils::CountSetBits(
          flags &
          (SpvMemorySemanticsAcquireMask | SpvMemorySemanticsReleaseMask |
           SpvMemorySemanticsAcquireReleaseMask |
           SpvMemorySemanticsSequentiallyConsistentMask)) > 1) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << spvOpcodeString(opcode)
           << ": no more than one of the following Memory Semantics bits can "
              "be set at the same time: Acquire, Release, AcquireRelease or "
              "SequentiallyConsistent";
  }

  if (flags & SpvMemorySemanticsUniformMemoryMask &&
      !_.HasCapability(SpvCapabilityShader)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << spvOpcodeString(opcode)
           << ": Memory Semantics UniformMemory requires capability Shader";
  }

  if (flags & SpvMemorySemanticsAtomicCounterMemoryMask &&
      !_.HasCapability(SpvCapabilityAtomicStorage)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << spvOpcodeString(opcode)
           << ": Memory Semantics UniformMemory requires capability "
              "AtomicStorage";
  }

  if (opcode == SpvOpAtomicFlagClear &&
      (flags & SpvMemorySemanticsAcquireMask ||
       flags & SpvMemorySemanticsAcquireReleaseMask)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << "Memory Semantics Acquire and AcquireRelease cannot be used with "
           << spvOpcodeString(opcode);
  }

  if (opcode == SpvOpAtomicCompareExchange && operand_index == 5 &&
      (flags & SpvMemorySemanticsReleaseMask ||
       flags & SpvMemorySemanticsAcquireReleaseMask)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst)
           << spvOpcodeString(opcode)
           << ": Memory Semantics Release and AcquireRelease cannot be used "
              "for operand Unequal";
  }

  if (spvIsVulkanEnv(_.context()->target_env)) {
    if (opcode == SpvOpAtomicLoad &&
        (flags & SpvMemorySemanticsReleaseMask ||
         flags & SpvMemorySemanticsAcquireReleaseMask ||
         flags & SpvMemorySemanticsSequentiallyConsistentMask)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Vulkan spec disallows OpAtomicLoad with Memory Semantics "
                "Release, AcquireRelease and SequentiallyConsistent";
    }

    if (opcode == SpvOpAtomicStore &&
        (flags & SpvMemorySemanticsAcquireMask ||
         flags & SpvMemorySemanticsAcquireReleaseMask ||
         flags & SpvMemorySemanticsSequentiallyConsistentMask)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Vulkan spec disallows OpAtomicStore with Memory Semantics "
                "Acquire, AcquireRelease and SequentiallyConsistent";
    }
  }

  // TODO(atgoo@github.com) Add checks for OpenCL and OpenGL environments.

  return SPV_SUCCESS;
}

// Validates correctness of atomic instructions.
spv_result_t AtomicsPass(ValidationState_t& _, const Instruction* inst) {
  const SpvOp opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();

  switch (opcode) {
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
    case SpvOpAtomicFlagTestAndSet:
    case SpvOpAtomicFlagClear: {
      if (_.HasCapability(SpvCapabilityKernel) &&
          (opcode == SpvOpAtomicLoad || opcode == SpvOpAtomicExchange ||
           opcode == SpvOpAtomicCompareExchange)) {
        if (!_.IsFloatScalarType(result_type) &&
            !_.IsIntScalarType(result_type)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << spvOpcodeString(opcode)
                 << ": expected Result Type to be int or float scalar type";
        }
      } else if (opcode == SpvOpAtomicFlagTestAndSet) {
        if (!_.IsBoolScalarType(result_type)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << spvOpcodeString(opcode)
                 << ": expected Result Type to be bool scalar type";
        }
      } else if (opcode == SpvOpAtomicFlagClear || opcode == SpvOpAtomicStore) {
        assert(result_type == 0);
      } else {
        if (!_.IsIntScalarType(result_type)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << spvOpcodeString(opcode)
                 << ": expected Result Type to be int scalar type";
        }
        if (spvIsVulkanEnv(_.context()->target_env) &&
            _.GetBitWidth(result_type) != 32) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << spvOpcodeString(opcode)
                 << ": according to the Vulkan spec atomic Result Type needs "
                    "to be a 32-bit int scalar type";
        }
      }

      uint32_t operand_index =
          opcode == SpvOpAtomicFlagClear || opcode == SpvOpAtomicStore ? 0 : 2;
      const uint32_t pointer_type = _.GetOperandTypeId(inst, operand_index++);

      uint32_t data_type = 0;
      uint32_t storage_class = 0;
      if (!_.GetPointerTypeInfo(pointer_type, &data_type, &storage_class)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << spvOpcodeString(opcode)
               << ": expected Pointer to be of type OpTypePointer";
      }

      switch (storage_class) {
        case SpvStorageClassUniform:
        case SpvStorageClassWorkgroup:
        case SpvStorageClassCrossWorkgroup:
        case SpvStorageClassGeneric:
        case SpvStorageClassAtomicCounter:
        case SpvStorageClassImage:
        case SpvStorageClassStorageBuffer:
          break;
        default:
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << spvOpcodeString(opcode)
                 << ": expected Pointer Storage Class to be Uniform, "
                    "Workgroup, CrossWorkgroup, Generic, AtomicCounter, Image "
                    "or StorageBuffer";
      }

      if (opcode == SpvOpAtomicFlagTestAndSet ||
          opcode == SpvOpAtomicFlagClear) {
        if (!_.IsIntScalarType(data_type) || _.GetBitWidth(data_type) != 32) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << spvOpcodeString(opcode)
                 << ": expected Pointer to point to a value of 32-bit int type";
        }
      } else if (opcode == SpvOpAtomicStore) {
        if (!_.IsFloatScalarType(data_type) && !_.IsIntScalarType(data_type)) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << spvOpcodeString(opcode)
                 << ": expected Pointer to be a pointer to int or float "
                 << "scalar type";
        }
      } else {
        if (data_type != result_type) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << spvOpcodeString(opcode)
                 << ": expected Pointer to point to a value of type Result "
                    "Type";
        }
      }

      auto memory_scope = inst->GetOperandAs<const uint32_t>(operand_index++);
      if (auto error = ValidateMemoryScope(_, inst, memory_scope)) {
        return error;
      }

      if (auto error = ValidateMemorySemantics(_, inst, operand_index++))
        return error;

      if (opcode == SpvOpAtomicCompareExchange ||
          opcode == SpvOpAtomicCompareExchangeWeak) {
        if (auto error = ValidateMemorySemantics(_, inst, operand_index++))
          return error;
      }

      if (opcode == SpvOpAtomicStore) {
        const uint32_t value_type = _.GetOperandTypeId(inst, 3);
        if (value_type != data_type) {
          return _.diag(SPV_ERROR_INVALID_DATA, inst)
                 << spvOpcodeString(opcode)
                 << ": expected Value type and the type pointed to by Pointer "
                    "to"
                 << " be the same";
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
