// Copyright (c) 2018 Google LLC.
// Copyright (c) 2019 NVIDIA Corporation
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

#include "source/val/validate.h"

#include "source/opcode.h"
#include "source/spirv_target_env.h"
#include "source/val/instruction.h"
#include "source/val/validate_scopes.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {
namespace {

spv_result_t ValidateUndef(ValidationState_t& _, const Instruction* inst) {
  if (_.HasCapability(SpvCapabilityShader) &&
      _.ContainsLimitedUseIntOrFloatType(inst->type_id()) &&
      !_.IsPointerType(inst->type_id())) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Cannot create undefined values with 8- or 16-bit types";
  }

  if (spvIsWebGPUEnv(_.context()->target_env)) {
    return _.diag(SPV_ERROR_INVALID_BINARY, inst) << "OpUndef is disallowed";
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateShaderClock(ValidationState_t& _,
                                 const Instruction* inst) {
  const uint32_t execution_scope = inst->word(3);
  if (auto error = ValidateExecutionScope(_, inst, execution_scope)) {
    return error;
  }

  // Result Type must be a 64 - bit unsigned integer type or
  // a vector of two - components of 32 -
  // bit unsigned integer type
  const uint32_t result_type = inst->type_id();
  if (!(_.IsUnsignedIntScalarType(result_type) &&
        _.GetBitWidth(result_type) == 64) &&
      !(_.IsUnsignedIntVectorType(result_type) &&
        _.GetDimension(result_type) == 2 && _.GetBitWidth(result_type) == 32)) {
    return _.diag(SPV_ERROR_INVALID_DATA, inst) << "Expected Value to be a "
                                                   "vector of two components"
                                                   " of unsigned integer"
                                                   " or 64bit unsigned integer";
  }

  return SPV_SUCCESS;
}

}  // namespace

spv_result_t MiscPass(ValidationState_t& _, const Instruction* inst) {
  switch (inst->opcode()) {
    case SpvOpUndef:
      if (auto error = ValidateUndef(_, inst)) return error;
      break;
    default:
      break;
  }
  switch (inst->opcode()) {
    case SpvOpBeginInvocationInterlockEXT:
    case SpvOpEndInvocationInterlockEXT:
      _.function(inst->function()->id())
          ->RegisterExecutionModelLimitation(
              SpvExecutionModelFragment,
              "OpBeginInvocationInterlockEXT/OpEndInvocationInterlockEXT "
              "require Fragment execution model");

      _.function(inst->function()->id())
          ->RegisterLimitation([](const ValidationState_t& state,
                                  const Function* entry_point,
                                  std::string* message) {
            const auto* execution_modes =
                state.GetExecutionModes(entry_point->id());

            auto find_interlock = [](const SpvExecutionMode& mode) {
              switch (mode) {
                case SpvExecutionModePixelInterlockOrderedEXT:
                case SpvExecutionModePixelInterlockUnorderedEXT:
                case SpvExecutionModeSampleInterlockOrderedEXT:
                case SpvExecutionModeSampleInterlockUnorderedEXT:
                case SpvExecutionModeShadingRateInterlockOrderedEXT:
                case SpvExecutionModeShadingRateInterlockUnorderedEXT:
                  return true;
                default:
                  return false;
              }
            };

            bool found = false;
            if (execution_modes) {
              auto i = std::find_if(execution_modes->begin(),
                                    execution_modes->end(), find_interlock);
              found = (i != execution_modes->end());
            }

            if (!found) {
              *message =
                  "OpBeginInvocationInterlockEXT/OpEndInvocationInterlockEXT "
                  "require a fragment shader interlock execution mode.";
              return false;
            }
            return true;
          });
      break;
    case SpvOpDemoteToHelperInvocationEXT:
      _.function(inst->function()->id())
          ->RegisterExecutionModelLimitation(
              SpvExecutionModelFragment,
              "OpDemoteToHelperInvocationEXT requires Fragment execution "
              "model");
      break;
    case SpvOpIsHelperInvocationEXT: {
      const uint32_t result_type = inst->type_id();
      _.function(inst->function()->id())
          ->RegisterExecutionModelLimitation(
              SpvExecutionModelFragment,
              "OpIsHelperInvocationEXT requires Fragment execution model");
      if (!_.IsBoolScalarType(result_type))
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected bool scalar type as Result Type: "
               << spvOpcodeString(inst->opcode());
      break;
    }
    case SpvOpReadClockKHR:
      if (auto error = ValidateShaderClock(_, inst)) {
        return error;
      }
      break;
    default:
      break;
  }

  return SPV_SUCCESS;
}

}  // namespace val
}  // namespace spvtools
