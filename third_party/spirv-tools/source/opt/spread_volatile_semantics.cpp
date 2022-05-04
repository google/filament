// Copyright (c) 2022 Google LLC
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

#include "source/opt/spread_volatile_semantics.h"

#include "source/opt/decoration_manager.h"
#include "source/opt/ir_builder.h"
#include "source/spirv_constant.h"

namespace spvtools {
namespace opt {
namespace {

const uint32_t kOpDecorateInOperandBuiltinDecoration = 2u;
const uint32_t kOpLoadInOperandMemoryOperands = 1u;
const uint32_t kOpEntryPointInOperandEntryPoint = 1u;
const uint32_t kOpEntryPointInOperandInterface = 3u;

bool HasBuiltinDecoration(analysis::DecorationManager* decoration_manager,
                          uint32_t var_id, uint32_t built_in) {
  return decoration_manager->FindDecoration(
      var_id, SpvDecorationBuiltIn, [built_in](const Instruction& inst) {
        return built_in == inst.GetSingleWordInOperand(
                               kOpDecorateInOperandBuiltinDecoration);
      });
}

bool IsBuiltInForRayTracingVolatileSemantics(uint32_t built_in) {
  switch (built_in) {
    case SpvBuiltInSMIDNV:
    case SpvBuiltInWarpIDNV:
    case SpvBuiltInSubgroupSize:
    case SpvBuiltInSubgroupLocalInvocationId:
    case SpvBuiltInSubgroupEqMask:
    case SpvBuiltInSubgroupGeMask:
    case SpvBuiltInSubgroupGtMask:
    case SpvBuiltInSubgroupLeMask:
    case SpvBuiltInSubgroupLtMask:
      return true;
    default:
      return false;
  }
}

bool HasBuiltinForRayTracingVolatileSemantics(
    analysis::DecorationManager* decoration_manager, uint32_t var_id) {
  return decoration_manager->FindDecoration(
      var_id, SpvDecorationBuiltIn, [](const Instruction& inst) {
        uint32_t built_in =
            inst.GetSingleWordInOperand(kOpDecorateInOperandBuiltinDecoration);
        return IsBuiltInForRayTracingVolatileSemantics(built_in);
      });
}

bool HasVolatileDecoration(analysis::DecorationManager* decoration_manager,
                           uint32_t var_id) {
  return decoration_manager->HasDecoration(var_id, SpvDecorationVolatile);
}

bool HasOnlyEntryPointsAsFunctions(IRContext* context, Module* module) {
  std::unordered_set<uint32_t> entry_function_ids;
  for (Instruction& entry_point : module->entry_points()) {
    entry_function_ids.insert(
        entry_point.GetSingleWordInOperand(kOpEntryPointInOperandEntryPoint));
  }
  for (auto& function : *module) {
    if (entry_function_ids.find(function.result_id()) ==
        entry_function_ids.end()) {
      std::string message(
          "Functions of SPIR-V for spread-volatile-semantics pass input must "
          "be inlined except entry points");
      message += "\n  " + function.DefInst().PrettyPrint(
                              SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
      context->consumer()(SPV_MSG_ERROR, "", {0, 0, 0}, message.c_str());
      return false;
    }
  }
  return true;
}

}  // namespace

Pass::Status SpreadVolatileSemantics::Process() {
  if (HasNoExecutionModel()) {
    return Status::SuccessWithoutChange;
  }

  if (!HasOnlyEntryPointsAsFunctions(context(), get_module())) {
    return Status::Failure;
  }

  const bool is_vk_memory_model_enabled =
      context()->get_feature_mgr()->HasCapability(
          SpvCapabilityVulkanMemoryModel);
  CollectTargetsForVolatileSemantics(is_vk_memory_model_enabled);

  // If VulkanMemoryModel capability is not enabled, we have to set Volatile
  // decoration for interface variables instead of setting Volatile for load
  // instructions. If an interface (or pointers to it) is used by two load
  // instructions in two entry points and one must be volatile while another
  // is not, we have to report an error for the conflict.
  if (!is_vk_memory_model_enabled &&
      HasInterfaceInConflictOfVolatileSemantics()) {
    return Status::Failure;
  }

  return SpreadVolatileSemanticsToVariables(is_vk_memory_model_enabled);
}

Pass::Status SpreadVolatileSemantics::SpreadVolatileSemanticsToVariables(
    const bool is_vk_memory_model_enabled) {
  Status status = Status::SuccessWithoutChange;
  for (Instruction& var : context()->types_values()) {
    auto entry_function_ids =
        EntryFunctionsToSpreadVolatileSemanticsForVar(var.result_id());
    if (entry_function_ids.empty()) {
      continue;
    }

    if (is_vk_memory_model_enabled) {
      SetVolatileForLoadsInEntries(&var, entry_function_ids);
    } else {
      DecorateVarWithVolatile(&var);
    }
    status = Status::SuccessWithChange;
  }
  return status;
}

bool SpreadVolatileSemantics::IsTargetUsedByNonVolatileLoadInEntryPoint(
    uint32_t var_id, Instruction* entry_point) {
  uint32_t entry_function_id =
      entry_point->GetSingleWordInOperand(kOpEntryPointInOperandEntryPoint);
  return !VisitLoadsOfPointersToVariableInEntries(
      var_id,
      [](Instruction* load) {
        // If it has a load without volatile memory operand, finish traversal
        // and return false.
        if (load->NumInOperands() <= kOpLoadInOperandMemoryOperands) {
          return false;
        }
        uint32_t memory_operands =
            load->GetSingleWordInOperand(kOpLoadInOperandMemoryOperands);
        return (memory_operands & SpvMemoryAccessVolatileMask) != 0;
      },
      {entry_function_id});
}

bool SpreadVolatileSemantics::HasInterfaceInConflictOfVolatileSemantics() {
  for (Instruction& entry_point : get_module()->entry_points()) {
    SpvExecutionModel execution_model =
        static_cast<SpvExecutionModel>(entry_point.GetSingleWordInOperand(0));
    for (uint32_t operand_index = kOpEntryPointInOperandInterface;
         operand_index < entry_point.NumInOperands(); ++operand_index) {
      uint32_t var_id = entry_point.GetSingleWordInOperand(operand_index);
      if (!EntryFunctionsToSpreadVolatileSemanticsForVar(var_id).empty() &&
          !IsTargetForVolatileSemantics(var_id, execution_model) &&
          IsTargetUsedByNonVolatileLoadInEntryPoint(var_id, &entry_point)) {
        Instruction* inst = context()->get_def_use_mgr()->GetDef(var_id);
        context()->EmitErrorMessage(
            "Variable is a target for Volatile semantics for an entry point, "
            "but it is not for another entry point",
            inst);
        return true;
      }
    }
  }
  return false;
}

void SpreadVolatileSemantics::MarkVolatileSemanticsForVariable(
    uint32_t var_id, Instruction* entry_point) {
  uint32_t entry_function_id =
      entry_point->GetSingleWordInOperand(kOpEntryPointInOperandEntryPoint);
  auto itr = var_ids_to_entry_fn_for_volatile_semantics_.find(var_id);
  if (itr == var_ids_to_entry_fn_for_volatile_semantics_.end()) {
    var_ids_to_entry_fn_for_volatile_semantics_[var_id] = {entry_function_id};
    return;
  }
  itr->second.insert(entry_function_id);
}

void SpreadVolatileSemantics::CollectTargetsForVolatileSemantics(
    const bool is_vk_memory_model_enabled) {
  for (Instruction& entry_point : get_module()->entry_points()) {
    SpvExecutionModel execution_model =
        static_cast<SpvExecutionModel>(entry_point.GetSingleWordInOperand(0));
    for (uint32_t operand_index = kOpEntryPointInOperandInterface;
         operand_index < entry_point.NumInOperands(); ++operand_index) {
      uint32_t var_id = entry_point.GetSingleWordInOperand(operand_index);
      if (!IsTargetForVolatileSemantics(var_id, execution_model)) {
        continue;
      }
      if (is_vk_memory_model_enabled ||
          IsTargetUsedByNonVolatileLoadInEntryPoint(var_id, &entry_point)) {
        MarkVolatileSemanticsForVariable(var_id, &entry_point);
      }
    }
  }
}

void SpreadVolatileSemantics::DecorateVarWithVolatile(Instruction* var) {
  analysis::DecorationManager* decoration_manager =
      context()->get_decoration_mgr();
  uint32_t var_id = var->result_id();
  if (HasVolatileDecoration(decoration_manager, var_id)) {
    return;
  }
  get_decoration_mgr()->AddDecoration(
      SpvOpDecorate, {{spv_operand_type_t::SPV_OPERAND_TYPE_ID, {var_id}},
                      {spv_operand_type_t::SPV_OPERAND_TYPE_LITERAL_INTEGER,
                       {SpvDecorationVolatile}}});
}

bool SpreadVolatileSemantics::VisitLoadsOfPointersToVariableInEntries(
    uint32_t var_id, const std::function<bool(Instruction*)>& handle_load,
    const std::unordered_set<uint32_t>& entry_function_ids) {
  std::vector<uint32_t> worklist({var_id});
  auto* def_use_mgr = context()->get_def_use_mgr();
  while (!worklist.empty()) {
    uint32_t ptr_id = worklist.back();
    worklist.pop_back();
    bool finish_traversal = !def_use_mgr->WhileEachUser(
        ptr_id, [this, &worklist, &ptr_id, handle_load,
                 &entry_function_ids](Instruction* user) {
          BasicBlock* block = context()->get_instr_block(user);
          if (block == nullptr ||
              entry_function_ids.find(block->GetParent()->result_id()) ==
                  entry_function_ids.end()) {
            return true;
          }

          if (user->opcode() == SpvOpAccessChain ||
              user->opcode() == SpvOpInBoundsAccessChain ||
              user->opcode() == SpvOpPtrAccessChain ||
              user->opcode() == SpvOpInBoundsPtrAccessChain ||
              user->opcode() == SpvOpCopyObject) {
            if (ptr_id == user->GetSingleWordInOperand(0))
              worklist.push_back(user->result_id());
            return true;
          }

          if (user->opcode() != SpvOpLoad) {
            return true;
          }

          return handle_load(user);
        });
    if (finish_traversal) return false;
  }
  return true;
}

void SpreadVolatileSemantics::SetVolatileForLoadsInEntries(
    Instruction* var, const std::unordered_set<uint32_t>& entry_function_ids) {
  // Set Volatile memory operand for all load instructions if they do not have
  // it.
  VisitLoadsOfPointersToVariableInEntries(
      var->result_id(),
      [](Instruction* load) {
        if (load->NumInOperands() <= kOpLoadInOperandMemoryOperands) {
          load->AddOperand(
              {SPV_OPERAND_TYPE_MEMORY_ACCESS, {SpvMemoryAccessVolatileMask}});
          return true;
        }
        uint32_t memory_operands =
            load->GetSingleWordInOperand(kOpLoadInOperandMemoryOperands);
        memory_operands |= SpvMemoryAccessVolatileMask;
        load->SetInOperand(kOpLoadInOperandMemoryOperands, {memory_operands});
        return true;
      },
      entry_function_ids);
}

bool SpreadVolatileSemantics::IsTargetForVolatileSemantics(
    uint32_t var_id, SpvExecutionModel execution_model) {
  analysis::DecorationManager* decoration_manager =
      context()->get_decoration_mgr();
  if (execution_model == SpvExecutionModelFragment) {
    return get_module()->version() >= SPV_SPIRV_VERSION_WORD(1, 6) &&
           HasBuiltinDecoration(decoration_manager, var_id,
                                SpvBuiltInHelperInvocation);
  }

  if (execution_model == SpvExecutionModelIntersectionKHR ||
      execution_model == SpvExecutionModelIntersectionNV) {
    if (HasBuiltinDecoration(decoration_manager, var_id,
                             SpvBuiltInRayTmaxKHR)) {
      return true;
    }
  }

  switch (execution_model) {
    case SpvExecutionModelRayGenerationKHR:
    case SpvExecutionModelClosestHitKHR:
    case SpvExecutionModelMissKHR:
    case SpvExecutionModelCallableKHR:
    case SpvExecutionModelIntersectionKHR:
      return HasBuiltinForRayTracingVolatileSemantics(decoration_manager,
                                                      var_id);
    default:
      return false;
  }
}

}  // namespace opt
}  // namespace spvtools
