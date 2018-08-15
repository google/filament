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

#include "source/val/validate.h"

#include <algorithm>

#include "source/opcode.h"
#include "source/val/instruction.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {
namespace {

spv_result_t ValidateFunction(ValidationState_t& _, const Instruction* inst) {
  const auto function_type_id = inst->GetOperandAs<uint32_t>(3);
  const auto function_type = _.FindDef(function_type_id);
  if (!function_type || SpvOpTypeFunction != function_type->opcode()) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpFunction Function Type <id> '" << _.getIdName(function_type_id)
           << "' is not a function type.";
  }

  const auto return_id = function_type->GetOperandAs<uint32_t>(1);
  if (return_id != inst->type_id()) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpFunction Result Type <id> '" << _.getIdName(inst->type_id())
           << "' does not match the Function Type's return type <id> '"
           << _.getIdName(return_id) << "'.";
  }

  for (auto& pair : inst->uses()) {
    const auto* use = pair.first;
    const std::vector<SpvOp> acceptable = {
        SpvOpFunctionCall,
        SpvOpEntryPoint,
        SpvOpEnqueueKernel,
        SpvOpGetKernelNDrangeSubGroupCount,
        SpvOpGetKernelNDrangeMaxSubGroupSize,
        SpvOpGetKernelWorkGroupSize,
        SpvOpGetKernelPreferredWorkGroupSizeMultiple,
        SpvOpGetKernelLocalSizeForSubgroupCount,
        SpvOpGetKernelMaxNumSubgroups};
    if (std::find(acceptable.begin(), acceptable.end(), use->opcode()) ==
        acceptable.end()) {
      return _.diag(SPV_ERROR_INVALID_ID, use)
             << "Invalid use of function result id " << _.getIdName(inst->id())
             << ".";
    }
  }

  return SPV_SUCCESS;
}

spv_result_t ValidateFunctionParameter(ValidationState_t& _,
                                       const Instruction* inst) {
  // NOTE: Find OpFunction & ensure OpFunctionParameter is not out of place.
  size_t param_index = 0;
  size_t inst_num = inst->LineNum() - 1;
  if (inst_num == 0) {
    return _.diag(SPV_ERROR_INVALID_LAYOUT, inst)
           << "Function parameter cannot be the first instruction.";
  }

  auto func_inst = &_.ordered_instructions()[inst_num];
  while (--inst_num) {
    func_inst = &_.ordered_instructions()[inst_num];
    if (func_inst->opcode() == SpvOpFunction) {
      break;
    } else if (func_inst->opcode() == SpvOpFunctionParameter) {
      ++param_index;
    }
  }

  if (func_inst->opcode() != SpvOpFunction) {
    return _.diag(SPV_ERROR_INVALID_LAYOUT, inst)
           << "Function parameter must be preceded by a function.";
  }

  const auto function_type_id = func_inst->GetOperandAs<uint32_t>(3);
  const auto function_type = _.FindDef(function_type_id);
  if (!function_type) {
    return _.diag(SPV_ERROR_INVALID_ID, func_inst)
           << "Missing function type definition.";
  }
  if (param_index >= function_type->words().size() - 3) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Too many OpFunctionParameters for " << func_inst->id()
           << ": expected " << function_type->words().size() - 3
           << " based on the function's type";
  }

  const auto param_type =
      _.FindDef(function_type->GetOperandAs<uint32_t>(param_index + 2));
  if (!param_type || inst->type_id() != param_type->id()) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpFunctionParameter Result Type <id> '"
           << _.getIdName(inst->type_id())
           << "' does not match the OpTypeFunction parameter "
              "type of the same index.";
  }
  return SPV_SUCCESS;
}

spv_result_t ValidateFunctionCall(ValidationState_t& _,
                                  const Instruction* inst) {
  const auto function_id = inst->GetOperandAs<uint32_t>(2);
  const auto function = _.FindDef(function_id);
  if (!function || SpvOpFunction != function->opcode()) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpFunctionCall Function <id> '" << _.getIdName(function_id)
           << "' is not a function.";
  }

  auto return_type = _.FindDef(function->type_id());
  if (!return_type || return_type->id() != inst->type_id()) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpFunctionCall Result Type <id> '"
           << _.getIdName(inst->type_id())
           << "'s type does not match Function <id> '"
           << _.getIdName(return_type->id()) << "'s return type.";
  }

  const auto function_type_id = function->GetOperandAs<uint32_t>(3);
  const auto function_type = _.FindDef(function_type_id);
  if (!function_type || function_type->opcode() != SpvOpTypeFunction) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "Missing function type definition.";
  }

  const auto function_call_arg_count = inst->words().size() - 4;
  const auto function_param_count = function_type->words().size() - 3;
  if (function_param_count != function_call_arg_count) {
    return _.diag(SPV_ERROR_INVALID_ID, inst)
           << "OpFunctionCall Function <id>'s parameter count does not match "
              "the argument count.";
  }

  for (size_t argument_index = 3, param_index = 2;
       argument_index < inst->operands().size();
       argument_index++, param_index++) {
    const auto argument_id = inst->GetOperandAs<uint32_t>(argument_index);
    const auto argument = _.FindDef(argument_id);
    if (!argument) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "Missing argument " << argument_index - 3 << " definition.";
    }

    const auto argument_type = _.FindDef(argument->type_id());
    if (!argument_type) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "Missing argument " << argument_index - 3
             << " type definition.";
    }

    const auto parameter_type_id =
        function_type->GetOperandAs<uint32_t>(param_index);
    const auto parameter_type = _.FindDef(parameter_type_id);
    if (!parameter_type || argument_type->id() != parameter_type->id()) {
      return _.diag(SPV_ERROR_INVALID_ID, inst)
             << "OpFunctionCall Argument <id> '" << _.getIdName(argument_id)
             << "'s type does not match Function <id> '"
             << _.getIdName(parameter_type_id) << "'s parameter type.";
    }
  }
  return SPV_SUCCESS;
}

}  // namespace

spv_result_t FunctionPass(ValidationState_t& _, const Instruction* inst) {
  switch (inst->opcode()) {
    case SpvOpFunction:
      if (auto error = ValidateFunction(_, inst)) return error;
      break;
    case SpvOpFunctionParameter:
      if (auto error = ValidateFunctionParameter(_, inst)) return error;
      break;
    case SpvOpFunctionCall:
      if (auto error = ValidateFunctionCall(_, inst)) return error;
      break;
    default:
      break;
  }

  return SPV_SUCCESS;
}

}  // namespace val
}  // namespace spvtools
