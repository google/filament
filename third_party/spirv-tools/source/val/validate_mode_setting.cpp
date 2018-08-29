// Copyright (c) 2018 Google LLC.
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
//
#include "source/val/validate.h"

#include <algorithm>

#include "source/opcode.h"
#include "source/val/instruction.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {
namespace {

spv_result_t ValidateEntryPoint(ValidationState_t& _, const Instruction* inst) {
  const auto entry_point_id = inst->GetOperandAs<uint32_t>(1);
  auto entry_point = _.FindDef(entry_point_id);
  if (!entry_point || SpvOpFunction != entry_point->opcode()) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpEntryPoint Entry Point <id> '" << _.getIdName(entry_point_id)
           << "' is not a function.";
  }
  // don't check kernel function signatures
  const SpvExecutionModel execution_model =
      inst->GetOperandAs<SpvExecutionModel>(0);
  if (execution_model != SpvExecutionModelKernel) {
    // TODO: Check the entry point signature is void main(void), may be subject
    // to change
    const auto entry_point_type_id = entry_point->GetOperandAs<uint32_t>(3);
    const auto entry_point_type = _.FindDef(entry_point_type_id);
    if (!entry_point_type || 3 != entry_point_type->words().size()) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpEntryPoint Entry Point <id> '" << _.getIdName(entry_point_id)
             << "'s function parameter count is not zero.";
    }
  }

  auto return_type = _.FindDef(entry_point->type_id());
  if (!return_type || SpvOpTypeVoid != return_type->opcode()) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpEntryPoint Entry Point <id> '" << _.getIdName(entry_point_id)
           << "'s function return type is not void.";
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateExecutionMode(ValidationState_t& _,
                                   const Instruction* inst) {
  const auto entry_point_id = inst->GetOperandAs<uint32_t>(0);
  const auto found = std::find(_.entry_points().cbegin(),
                               _.entry_points().cend(), entry_point_id);
  if (found == _.entry_points().cend()) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpExecutionMode Entry Point <id> '"
           << _.getIdName(entry_point_id)
           << "' is not the Entry Point "
              "operand of an OpEntryPoint.";
  }
  return SPV_SUCCESS;
}

}  // namespace

spv_result_t ModeSettingPass(ValidationState_t& _, const Instruction* inst) {
  switch (inst->opcode()) {
    case SpvOpEntryPoint:
      if (auto error = ValidateEntryPoint(_, inst)) return error;
      break;
    case SpvOpExecutionMode:
      if (auto error = ValidateExecutionMode(_, inst)) return error;
      break;
    default:
      break;
  }
  return SPV_SUCCESS;
}

}  // namespace val
}  // namespace spvtools
