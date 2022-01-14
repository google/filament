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

// Validates correctness of derivative SPIR-V instructions.

#include "source/val/validate.h"

#include <string>

#include "source/diagnostic.h"
#include "source/opcode.h"
#include "source/val/instruction.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {

// Validates correctness of derivative instructions.
spv_result_t DerivativesPass(ValidationState_t& _, const Instruction* inst) {
  const SpvOp opcode = inst->opcode();
  const uint32_t result_type = inst->type_id();

  switch (opcode) {
    case SpvOpDPdx:
    case SpvOpDPdy:
    case SpvOpFwidth:
    case SpvOpDPdxFine:
    case SpvOpDPdyFine:
    case SpvOpFwidthFine:
    case SpvOpDPdxCoarse:
    case SpvOpDPdyCoarse:
    case SpvOpFwidthCoarse: {
      if (!_.IsFloatScalarOrVectorType(result_type)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected Result Type to be float scalar or vector type: "
               << spvOpcodeString(opcode);
      }
      if (!_.ContainsSizedIntOrFloatType(result_type, SpvOpTypeFloat, 32)) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Result type component width must be 32 bits";
      }

      const uint32_t p_type = _.GetOperandTypeId(inst, 2);
      if (p_type != result_type) {
        return _.diag(SPV_ERROR_INVALID_DATA, inst)
               << "Expected P type and Result Type to be the same: "
               << spvOpcodeString(opcode);
      }
      _.function(inst->function()->id())
          ->RegisterExecutionModelLimitation([opcode](SpvExecutionModel model,
                                                      std::string* message) {
            if (model != SpvExecutionModelFragment &&
                model != SpvExecutionModelGLCompute) {
              if (message) {
                *message =
                    std::string(
                        "Derivative instructions require Fragment or GLCompute "
                        "execution model: ") +
                    spvOpcodeString(opcode);
              }
              return false;
            }
            return true;
          });
      _.function(inst->function()->id())
          ->RegisterLimitation([opcode](const ValidationState_t& state,
                                        const Function* entry_point,
                                        std::string* message) {
            const auto* models = state.GetExecutionModels(entry_point->id());
            const auto* modes = state.GetExecutionModes(entry_point->id());
            if (models &&
                models->find(SpvExecutionModelGLCompute) != models->end() &&
                (!modes ||
                 (modes->find(SpvExecutionModeDerivativeGroupLinearNV) ==
                      modes->end() &&
                  modes->find(SpvExecutionModeDerivativeGroupQuadsNV) ==
                      modes->end()))) {
              if (message) {
                *message = std::string(
                               "Derivative instructions require "
                               "DerivativeGroupQuadsNV "
                               "or DerivativeGroupLinearNV execution mode for "
                               "GLCompute execution model: ") +
                           spvOpcodeString(opcode);
              }
              return false;
            }
            return true;
          });
      break;
    }

    default:
      break;
  }

  return SPV_SUCCESS;
}

}  // namespace val
}  // namespace spvtools
