// Copyright (c) 2017 The Khronos Group Inc.
// Copyright (c) 2017 Valve Corporation
// Copyright (c) 2017 LunarG Inc.
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

#include "source/opt/local_access_chain_convert_pass.h"

#include "ir_builder.h"
#include "ir_context.h"
#include "iterator.h"

namespace spvtools {
namespace opt {

namespace {

const uint32_t kStoreValIdInIdx = 1;
const uint32_t kAccessChainPtrIdInIdx = 0;
const uint32_t kConstantValueInIdx = 0;
const uint32_t kTypeIntWidthInIdx = 0;

}  // anonymous namespace

void LocalAccessChainConvertPass::BuildAndAppendInst(
    SpvOp opcode, uint32_t typeId, uint32_t resultId,
    const std::vector<Operand>& in_opnds,
    std::vector<std::unique_ptr<Instruction>>* newInsts) {
  std::unique_ptr<Instruction> newInst(
      new Instruction(context(), opcode, typeId, resultId, in_opnds));
  get_def_use_mgr()->AnalyzeInstDefUse(&*newInst);
  newInsts->emplace_back(std::move(newInst));
}

uint32_t LocalAccessChainConvertPass::BuildAndAppendVarLoad(
    const Instruction* ptrInst, uint32_t* varId, uint32_t* varPteTypeId,
    std::vector<std::unique_ptr<Instruction>>* newInsts) {
  const uint32_t ldResultId = TakeNextId();
  if (ldResultId == 0) {
    return 0;
  }

  *varId = ptrInst->GetSingleWordInOperand(kAccessChainPtrIdInIdx);
  const Instruction* varInst = get_def_use_mgr()->GetDef(*varId);
  assert(varInst->opcode() == SpvOpVariable);
  *varPteTypeId = GetPointeeTypeId(varInst);
  BuildAndAppendInst(SpvOpLoad, *varPteTypeId, ldResultId,
                     {{spv_operand_type_t::SPV_OPERAND_TYPE_ID, {*varId}}},
                     newInsts);
  return ldResultId;
}

void LocalAccessChainConvertPass::AppendConstantOperands(
    const Instruction* ptrInst, std::vector<Operand>* in_opnds) {
  uint32_t iidIdx = 0;
  ptrInst->ForEachInId([&iidIdx, &in_opnds, this](const uint32_t* iid) {
    if (iidIdx > 0) {
      const Instruction* cInst = get_def_use_mgr()->GetDef(*iid);
      uint32_t val = cInst->GetSingleWordInOperand(kConstantValueInIdx);
      in_opnds->push_back(
          {spv_operand_type_t::SPV_OPERAND_TYPE_LITERAL_INTEGER, {val}});
    }
    ++iidIdx;
  });
}

bool LocalAccessChainConvertPass::ReplaceAccessChainLoad(
    const Instruction* address_inst, Instruction* original_load) {
  // Build and append load of variable in ptrInst
  if (address_inst->NumInOperands() == 1) {
    // An access chain with no indices is essentially a copy.  All that is
    // needed is to propagate the address.
    context()->ReplaceAllUsesWith(
        address_inst->result_id(),
        address_inst->GetSingleWordInOperand(kAccessChainPtrIdInIdx));
    return true;
  }

  std::vector<std::unique_ptr<Instruction>> new_inst;
  uint32_t varId;
  uint32_t varPteTypeId;
  const uint32_t ldResultId =
      BuildAndAppendVarLoad(address_inst, &varId, &varPteTypeId, &new_inst);
  if (ldResultId == 0) {
    return false;
  }

  new_inst[0]->UpdateDebugInfoFrom(original_load);
  context()->get_decoration_mgr()->CloneDecorations(
      original_load->result_id(), ldResultId, {SpvDecorationRelaxedPrecision});
  original_load->InsertBefore(std::move(new_inst));
  context()->get_debug_info_mgr()->AnalyzeDebugInst(
      original_load->PreviousNode());

  // Rewrite |original_load| into an extract.
  Instruction::OperandList new_operands;

  // copy the result id and the type id to the new operand list.
  new_operands.emplace_back(original_load->GetOperand(0));
  new_operands.emplace_back(original_load->GetOperand(1));

  new_operands.emplace_back(
      Operand({spv_operand_type_t::SPV_OPERAND_TYPE_ID, {ldResultId}}));
  AppendConstantOperands(address_inst, &new_operands);
  original_load->SetOpcode(SpvOpCompositeExtract);
  original_load->ReplaceOperands(new_operands);
  context()->UpdateDefUse(original_load);
  return true;
}

bool LocalAccessChainConvertPass::GenAccessChainStoreReplacement(
    const Instruction* ptrInst, uint32_t valId,
    std::vector<std::unique_ptr<Instruction>>* newInsts) {
  if (ptrInst->NumInOperands() == 1) {
    // An access chain with no indices is essentially a copy.  However, we still
    // have to create a new store because the old ones will be deleted.
    BuildAndAppendInst(
        SpvOpStore, 0, 0,
        {{spv_operand_type_t::SPV_OPERAND_TYPE_ID,
          {ptrInst->GetSingleWordInOperand(kAccessChainPtrIdInIdx)}},
         {spv_operand_type_t::SPV_OPERAND_TYPE_ID, {valId}}},
        newInsts);
    return true;
  }

  // Build and append load of variable in ptrInst
  uint32_t varId;
  uint32_t varPteTypeId;
  const uint32_t ldResultId =
      BuildAndAppendVarLoad(ptrInst, &varId, &varPteTypeId, newInsts);
  if (ldResultId == 0) {
    return false;
  }

  context()->get_decoration_mgr()->CloneDecorations(
      varId, ldResultId, {SpvDecorationRelaxedPrecision});

  // Build and append Insert
  const uint32_t insResultId = TakeNextId();
  if (insResultId == 0) {
    return false;
  }
  std::vector<Operand> ins_in_opnds = {
      {spv_operand_type_t::SPV_OPERAND_TYPE_ID, {valId}},
      {spv_operand_type_t::SPV_OPERAND_TYPE_ID, {ldResultId}}};
  AppendConstantOperands(ptrInst, &ins_in_opnds);
  BuildAndAppendInst(SpvOpCompositeInsert, varPteTypeId, insResultId,
                     ins_in_opnds, newInsts);

  context()->get_decoration_mgr()->CloneDecorations(
      varId, insResultId, {SpvDecorationRelaxedPrecision});

  // Build and append Store
  BuildAndAppendInst(SpvOpStore, 0, 0,
                     {{spv_operand_type_t::SPV_OPERAND_TYPE_ID, {varId}},
                      {spv_operand_type_t::SPV_OPERAND_TYPE_ID, {insResultId}}},
                     newInsts);
  return true;
}

bool LocalAccessChainConvertPass::IsConstantIndexAccessChain(
    const Instruction* acp) const {
  uint32_t inIdx = 0;
  return acp->WhileEachInId([&inIdx, this](const uint32_t* tid) {
    if (inIdx > 0) {
      Instruction* opInst = get_def_use_mgr()->GetDef(*tid);
      if (opInst->opcode() != SpvOpConstant) return false;
    }
    ++inIdx;
    return true;
  });
}

bool LocalAccessChainConvertPass::HasOnlySupportedRefs(uint32_t ptrId) {
  if (supported_ref_ptrs_.find(ptrId) != supported_ref_ptrs_.end()) return true;
  if (get_def_use_mgr()->WhileEachUser(ptrId, [this](Instruction* user) {
        if (user->GetOpenCL100DebugOpcode() == OpenCLDebugInfo100DebugValue ||
            user->GetOpenCL100DebugOpcode() == OpenCLDebugInfo100DebugDeclare) {
          return true;
        }
        SpvOp op = user->opcode();
        if (IsNonPtrAccessChain(op) || op == SpvOpCopyObject) {
          if (!HasOnlySupportedRefs(user->result_id())) {
            return false;
          }
        } else if (op != SpvOpStore && op != SpvOpLoad && op != SpvOpName &&
                   !IsNonTypeDecorate(op)) {
          return false;
        }
        return true;
      })) {
    supported_ref_ptrs_.insert(ptrId);
    return true;
  }
  return false;
}

void LocalAccessChainConvertPass::FindTargetVars(Function* func) {
  for (auto bi = func->begin(); bi != func->end(); ++bi) {
    for (auto ii = bi->begin(); ii != bi->end(); ++ii) {
      switch (ii->opcode()) {
        case SpvOpStore:
        case SpvOpLoad: {
          uint32_t varId;
          Instruction* ptrInst = GetPtr(&*ii, &varId);
          if (!IsTargetVar(varId)) break;
          const SpvOp op = ptrInst->opcode();
          // Rule out variables with non-supported refs eg function calls
          if (!HasOnlySupportedRefs(varId)) {
            seen_non_target_vars_.insert(varId);
            seen_target_vars_.erase(varId);
            break;
          }
          // Rule out variables with nested access chains
          // TODO(): Convert nested access chains
          if (IsNonPtrAccessChain(op) && ptrInst->GetSingleWordInOperand(
                                             kAccessChainPtrIdInIdx) != varId) {
            seen_non_target_vars_.insert(varId);
            seen_target_vars_.erase(varId);
            break;
          }
          // Rule out variables accessed with non-constant indices
          if (!IsConstantIndexAccessChain(ptrInst)) {
            seen_non_target_vars_.insert(varId);
            seen_target_vars_.erase(varId);
            break;
          }
        } break;
        default:
          break;
      }
    }
  }
}

Pass::Status LocalAccessChainConvertPass::ConvertLocalAccessChains(
    Function* func) {
  FindTargetVars(func);
  // Replace access chains of all targeted variables with equivalent
  // extract and insert sequences
  bool modified = false;
  for (auto bi = func->begin(); bi != func->end(); ++bi) {
    std::vector<Instruction*> dead_instructions;
    for (auto ii = bi->begin(); ii != bi->end(); ++ii) {
      switch (ii->opcode()) {
        case SpvOpLoad: {
          uint32_t varId;
          Instruction* ptrInst = GetPtr(&*ii, &varId);
          if (!IsNonPtrAccessChain(ptrInst->opcode())) break;
          if (!IsTargetVar(varId)) break;
          if (!ReplaceAccessChainLoad(ptrInst, &*ii)) {
            return Status::Failure;
          }
          modified = true;
        } break;
        case SpvOpStore: {
          uint32_t varId;
          Instruction* store = &*ii;
          Instruction* ptrInst = GetPtr(store, &varId);
          if (!IsNonPtrAccessChain(ptrInst->opcode())) break;
          if (!IsTargetVar(varId)) break;
          std::vector<std::unique_ptr<Instruction>> newInsts;
          uint32_t valId = store->GetSingleWordInOperand(kStoreValIdInIdx);
          if (!GenAccessChainStoreReplacement(ptrInst, valId, &newInsts)) {
            return Status::Failure;
          }
          size_t num_of_instructions_to_skip = newInsts.size() - 1;
          dead_instructions.push_back(store);
          ++ii;
          ii = ii.InsertBefore(std::move(newInsts));
          for (size_t i = 0; i < num_of_instructions_to_skip; ++i) {
            ii->UpdateDebugInfoFrom(store);
            context()->get_debug_info_mgr()->AnalyzeDebugInst(&*ii);
            ++ii;
          }
          ii->UpdateDebugInfoFrom(store);
          context()->get_debug_info_mgr()->AnalyzeDebugInst(&*ii);
          modified = true;
        } break;
        default:
          break;
      }
    }

    while (!dead_instructions.empty()) {
      Instruction* inst = dead_instructions.back();
      dead_instructions.pop_back();
      DCEInst(inst, [&dead_instructions](Instruction* other_inst) {
        auto i = std::find(dead_instructions.begin(), dead_instructions.end(),
                           other_inst);
        if (i != dead_instructions.end()) {
          dead_instructions.erase(i);
        }
      });
    }
  }
  return (modified ? Status::SuccessWithChange : Status::SuccessWithoutChange);
}

void LocalAccessChainConvertPass::Initialize() {
  // Initialize Target Variable Caches
  seen_target_vars_.clear();
  seen_non_target_vars_.clear();

  // Initialize collections
  supported_ref_ptrs_.clear();

  // Initialize extension allowlist
  InitExtensions();
}

bool LocalAccessChainConvertPass::AllExtensionsSupported() const {
  // This capability can now exist without the extension, so we have to check
  // for the capability.  This pass is only looking at function scope symbols,
  // so we do not care if there are variable pointers on storage buffers.
  if (context()->get_feature_mgr()->HasCapability(
          SpvCapabilityVariablePointers))
    return false;
  // If any extension not in allowlist, return false
  for (auto& ei : get_module()->extensions()) {
    const char* extName =
        reinterpret_cast<const char*>(&ei.GetInOperand(0).words[0]);
    if (extensions_allowlist_.find(extName) == extensions_allowlist_.end())
      return false;
  }
  return true;
}

Pass::Status LocalAccessChainConvertPass::ProcessImpl() {
  // If non-32-bit integer type in module, terminate processing
  // TODO(): Handle non-32-bit integer constants in access chains
  for (const Instruction& inst : get_module()->types_values())
    if (inst.opcode() == SpvOpTypeInt &&
        inst.GetSingleWordInOperand(kTypeIntWidthInIdx) != 32)
      return Status::SuccessWithoutChange;
  // Do not process if module contains OpGroupDecorate. Additional
  // support required in KillNamesAndDecorates().
  // TODO(greg-lunarg): Add support for OpGroupDecorate
  for (auto& ai : get_module()->annotations())
    if (ai.opcode() == SpvOpGroupDecorate) return Status::SuccessWithoutChange;
  // Do not process if any disallowed extensions are enabled
  if (!AllExtensionsSupported()) return Status::SuccessWithoutChange;

  // Process all functions in the module.
  Status status = Status::SuccessWithoutChange;
  for (Function& func : *get_module()) {
    status = CombineStatus(status, ConvertLocalAccessChains(&func));
    if (status == Status::Failure) {
      break;
    }
  }
  return status;
}

LocalAccessChainConvertPass::LocalAccessChainConvertPass() {}

Pass::Status LocalAccessChainConvertPass::Process() {
  Initialize();
  return ProcessImpl();
}

void LocalAccessChainConvertPass::InitExtensions() {
  extensions_allowlist_.clear();
  extensions_allowlist_.insert({
      "SPV_AMD_shader_explicit_vertex_parameter",
      "SPV_AMD_shader_trinary_minmax",
      "SPV_AMD_gcn_shader",
      "SPV_KHR_shader_ballot",
      "SPV_AMD_shader_ballot",
      "SPV_AMD_gpu_shader_half_float",
      "SPV_KHR_shader_draw_parameters",
      "SPV_KHR_subgroup_vote",
      "SPV_KHR_8bit_storage",
      "SPV_KHR_16bit_storage",
      "SPV_KHR_device_group",
      "SPV_KHR_multiview",
      "SPV_NVX_multiview_per_view_attributes",
      "SPV_NV_viewport_array2",
      "SPV_NV_stereo_view_rendering",
      "SPV_NV_sample_mask_override_coverage",
      "SPV_NV_geometry_shader_passthrough",
      "SPV_AMD_texture_gather_bias_lod",
      "SPV_KHR_storage_buffer_storage_class",
      // SPV_KHR_variable_pointers
      //   Currently do not support extended pointer expressions
      "SPV_AMD_gpu_shader_int16",
      "SPV_KHR_post_depth_coverage",
      "SPV_KHR_shader_atomic_counter_ops",
      "SPV_EXT_shader_stencil_export",
      "SPV_EXT_shader_viewport_index_layer",
      "SPV_AMD_shader_image_load_store_lod",
      "SPV_AMD_shader_fragment_mask",
      "SPV_EXT_fragment_fully_covered",
      "SPV_AMD_gpu_shader_half_float_fetch",
      "SPV_GOOGLE_decorate_string",
      "SPV_GOOGLE_hlsl_functionality1",
      "SPV_GOOGLE_user_type",
      "SPV_NV_shader_subgroup_partitioned",
      "SPV_EXT_demote_to_helper_invocation",
      "SPV_EXT_descriptor_indexing",
      "SPV_NV_fragment_shader_barycentric",
      "SPV_NV_compute_shader_derivatives",
      "SPV_NV_shader_image_footprint",
      "SPV_NV_shading_rate",
      "SPV_NV_mesh_shader",
      "SPV_NV_ray_tracing",
      "SPV_KHR_ray_tracing",
      "SPV_KHR_ray_query",
      "SPV_EXT_fragment_invocation_density",
      "SPV_KHR_terminate_invocation",
  });
}

}  // namespace opt
}  // namespace spvtools
