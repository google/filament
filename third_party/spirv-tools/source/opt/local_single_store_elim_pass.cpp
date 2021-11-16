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

#include "source/opt/local_single_store_elim_pass.h"

#include "source/cfa.h"
#include "source/latest_version_glsl_std_450_header.h"
#include "source/opt/iterator.h"

namespace spvtools {
namespace opt {

namespace {

const uint32_t kStoreValIdInIdx = 1;
const uint32_t kVariableInitIdInIdx = 1;

}  // anonymous namespace

bool LocalSingleStoreElimPass::LocalSingleStoreElim(Function* func) {
  bool modified = false;

  // Check all function scope variables in |func|.
  BasicBlock* entry_block = &*func->begin();
  for (Instruction& inst : *entry_block) {
    if (inst.opcode() != SpvOpVariable) {
      break;
    }

    modified |= ProcessVariable(&inst);
  }
  return modified;
}

bool LocalSingleStoreElimPass::AllExtensionsSupported() const {
  // If any extension not in allowlist, return false
  for (auto& ei : get_module()->extensions()) {
    const char* extName =
        reinterpret_cast<const char*>(&ei.GetInOperand(0).words[0]);
    if (extensions_allowlist_.find(extName) == extensions_allowlist_.end())
      return false;
  }
  // only allow NonSemantic.Shader.DebugInfo.100, we cannot safely optimise
  // around unknown extended
  // instruction sets even if they are non-semantic
  for (auto& inst : context()->module()->ext_inst_imports()) {
    assert(inst.opcode() == SpvOpExtInstImport &&
           "Expecting an import of an extension's instruction set.");
    const char* extension_name =
        reinterpret_cast<const char*>(&inst.GetInOperand(0).words[0]);
    if (0 == std::strncmp(extension_name, "NonSemantic.", 12) &&
        0 != std::strncmp(extension_name, "NonSemantic.Shader.DebugInfo.100",
                          32)) {
      return false;
    }
  }
  return true;
}

Pass::Status LocalSingleStoreElimPass::ProcessImpl() {
  // Assumes relaxed logical addressing only (see instruction.h)
  if (context()->get_feature_mgr()->HasCapability(SpvCapabilityAddresses))
    return Status::SuccessWithoutChange;

  // Do not process if any disallowed extensions are enabled
  if (!AllExtensionsSupported()) return Status::SuccessWithoutChange;
  // Process all entry point functions
  ProcessFunction pfn = [this](Function* fp) {
    return LocalSingleStoreElim(fp);
  };
  bool modified = context()->ProcessReachableCallTree(pfn);
  return modified ? Status::SuccessWithChange : Status::SuccessWithoutChange;
}

LocalSingleStoreElimPass::LocalSingleStoreElimPass() = default;

Pass::Status LocalSingleStoreElimPass::Process() {
  InitExtensionAllowList();
  return ProcessImpl();
}

void LocalSingleStoreElimPass::InitExtensionAllowList() {
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
      "SPV_KHR_variable_pointers",
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
      "SPV_NV_shader_subgroup_partitioned",
      "SPV_EXT_descriptor_indexing",
      "SPV_NV_fragment_shader_barycentric",
      "SPV_NV_compute_shader_derivatives",
      "SPV_NV_shader_image_footprint",
      "SPV_NV_shading_rate",
      "SPV_NV_mesh_shader",
      "SPV_NV_ray_tracing",
      "SPV_KHR_ray_query",
      "SPV_EXT_fragment_invocation_density",
      "SPV_EXT_physical_storage_buffer",
      "SPV_KHR_terminate_invocation",
      "SPV_KHR_subgroup_uniform_control_flow",
      "SPV_KHR_integer_dot_product",
      "SPV_EXT_shader_image_int64",
      "SPV_KHR_non_semantic_info",
  });
}
bool LocalSingleStoreElimPass::ProcessVariable(Instruction* var_inst) {
  std::vector<Instruction*> users;
  FindUses(var_inst, &users);

  Instruction* store_inst = FindSingleStoreAndCheckUses(var_inst, users);

  if (store_inst == nullptr) {
    return false;
  }

  bool all_rewritten;
  bool modified = RewriteLoads(store_inst, users, &all_rewritten);

  // If all uses are rewritten and the variable has a DebugDeclare and the
  // variable is not an aggregate, add a DebugValue after the store and remove
  // the DebugDeclare.
  uint32_t var_id = var_inst->result_id();
  if (all_rewritten &&
      context()->get_debug_info_mgr()->IsVariableDebugDeclared(var_id)) {
    const analysis::Type* var_type =
        context()->get_type_mgr()->GetType(var_inst->type_id());
    const analysis::Type* store_type = var_type->AsPointer()->pointee_type();
    if (!(store_type->AsStruct() || store_type->AsArray())) {
      modified |= RewriteDebugDeclares(store_inst, var_id);
    }
  }

  return modified;
}

bool LocalSingleStoreElimPass::RewriteDebugDeclares(Instruction* store_inst,
                                                    uint32_t var_id) {
  std::unordered_set<Instruction*> invisible_decls;
  uint32_t value_id = store_inst->GetSingleWordInOperand(1);
  bool modified =
      context()->get_debug_info_mgr()->AddDebugValueIfVarDeclIsVisible(
          store_inst, var_id, value_id, store_inst, &invisible_decls);

  // For cases like the argument passing for an inlined function, the value
  // assignment is out of DebugDeclare's scope, but we have to preserve the
  // value assignment information using DebugValue. Generally, we need
  // ssa-rewrite analysis to decide a proper value assignment but at this point
  // we confirm that |var_id| has a single store. We can safely add DebugValue.
  if (!invisible_decls.empty()) {
    BasicBlock* store_block = context()->get_instr_block(store_inst);
    DominatorAnalysis* dominator_analysis =
        context()->GetDominatorAnalysis(store_block->GetParent());
    for (auto* decl : invisible_decls) {
      if (dominator_analysis->Dominates(store_inst, decl)) {
        context()->get_debug_info_mgr()->AddDebugValueForDecl(decl, value_id,
                                                              decl, store_inst);
        modified = true;
      }
    }
  }
  modified |= context()->get_debug_info_mgr()->KillDebugDeclares(var_id);
  return modified;
}

Instruction* LocalSingleStoreElimPass::FindSingleStoreAndCheckUses(
    Instruction* var_inst, const std::vector<Instruction*>& users) const {
  // Make sure there is exactly 1 store.
  Instruction* store_inst = nullptr;

  // If |var_inst| has an initializer, then that will count as a store.
  if (var_inst->NumInOperands() > 1) {
    store_inst = var_inst;
  }

  for (Instruction* user : users) {
    switch (user->opcode()) {
      case SpvOpStore:
        // Since we are in the relaxed addressing mode, the use has to be the
        // base address of the store, and not the value being store.  Otherwise,
        // we would have a pointer to a pointer to function scope memory, which
        // is not allowed.
        if (store_inst == nullptr) {
          store_inst = user;
        } else {
          // More than 1 store.
          return nullptr;
        }
        break;
      case SpvOpAccessChain:
      case SpvOpInBoundsAccessChain:
        if (FeedsAStore(user)) {
          // Has a partial store.  Cannot propagate that.
          return nullptr;
        }
        break;
      case SpvOpLoad:
      case SpvOpImageTexelPointer:
      case SpvOpName:
      case SpvOpCopyObject:
        break;
      case SpvOpExtInst: {
        auto dbg_op = user->GetCommonDebugOpcode();
        if (dbg_op == CommonDebugInfoDebugDeclare ||
            dbg_op == CommonDebugInfoDebugValue) {
          break;
        }
        return nullptr;
      }
      default:
        if (!user->IsDecoration()) {
          // Don't know if this instruction modifies the variable.
          // Conservatively assume it is a store.
          return nullptr;
        }
        break;
    }
  }
  return store_inst;
}

void LocalSingleStoreElimPass::FindUses(
    const Instruction* var_inst, std::vector<Instruction*>* users) const {
  analysis::DefUseManager* def_use_mgr = context()->get_def_use_mgr();
  def_use_mgr->ForEachUser(var_inst, [users, this](Instruction* user) {
    users->push_back(user);
    if (user->opcode() == SpvOpCopyObject) {
      FindUses(user, users);
    }
  });
}

bool LocalSingleStoreElimPass::FeedsAStore(Instruction* inst) const {
  analysis::DefUseManager* def_use_mgr = context()->get_def_use_mgr();
  return !def_use_mgr->WhileEachUser(inst, [this](Instruction* user) {
    switch (user->opcode()) {
      case SpvOpStore:
        return false;
      case SpvOpAccessChain:
      case SpvOpInBoundsAccessChain:
      case SpvOpCopyObject:
        return !FeedsAStore(user);
      case SpvOpLoad:
      case SpvOpImageTexelPointer:
      case SpvOpName:
        return true;
      default:
        // Don't know if this instruction modifies the variable.
        // Conservatively assume it is a store.
        return user->IsDecoration();
    }
  });
}

bool LocalSingleStoreElimPass::RewriteLoads(
    Instruction* store_inst, const std::vector<Instruction*>& uses,
    bool* all_rewritten) {
  BasicBlock* store_block = context()->get_instr_block(store_inst);
  DominatorAnalysis* dominator_analysis =
      context()->GetDominatorAnalysis(store_block->GetParent());

  uint32_t stored_id;
  if (store_inst->opcode() == SpvOpStore)
    stored_id = store_inst->GetSingleWordInOperand(kStoreValIdInIdx);
  else
    stored_id = store_inst->GetSingleWordInOperand(kVariableInitIdInIdx);

  *all_rewritten = true;
  bool modified = false;
  for (Instruction* use : uses) {
    if (use->opcode() == SpvOpStore) continue;
    auto dbg_op = use->GetCommonDebugOpcode();
    if (dbg_op == CommonDebugInfoDebugDeclare ||
        dbg_op == CommonDebugInfoDebugValue)
      continue;
    if (use->opcode() == SpvOpLoad &&
        dominator_analysis->Dominates(store_inst, use)) {
      modified = true;
      context()->KillNamesAndDecorates(use->result_id());
      context()->ReplaceAllUsesWith(use->result_id(), stored_id);
      context()->KillInst(use);
    } else {
      *all_rewritten = false;
    }
  }

  return modified;
}

}  // namespace opt
}  // namespace spvtools
