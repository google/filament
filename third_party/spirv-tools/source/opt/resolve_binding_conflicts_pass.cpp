// Copyright (c) 2025 Google LLC
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

#include "source/opt/resolve_binding_conflicts_pass.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "source/opt/decoration_manager.h"
#include "source/opt/def_use_manager.h"
#include "source/opt/instruction.h"
#include "source/opt/ir_builder.h"
#include "source/opt/ir_context.h"
#include "spirv/unified1/spirv.h"

namespace spvtools {
namespace opt {

// A VarBindingInfo contains the binding information for a single resource
// variable.
//
// Exactly one such object is created per resource variable in the
// module. In particular, when a resource variable is statically used by
// more than one entry point, those entry points share the same VarBindingInfo
// object for that variable.
struct VarBindingInfo {
  const Instruction* const var;
  const uint32_t descriptor_set;
  Instruction* const binding_decoration;

  // Returns the binding number.
  uint32_t binding() const {
    return binding_decoration->GetSingleWordInOperand(2);
  }
  // Sets the binding number to 'b'.
  void updateBinding(uint32_t b) { binding_decoration->SetOperand(2, {b}); }
};

// The bindings in the same descriptor set that are used by an entry point.
using BindingList = std::vector<VarBindingInfo*>;
// A map from descriptor set number to the list of bindings in that descriptor
// set, as used by a particular entry point.
using DescriptorSets = std::unordered_map<uint32_t, BindingList>;

IRContext::Analysis ResolveBindingConflictsPass::GetPreservedAnalyses() {
  // All analyses are kept up to date.
  // At most this modifies the Binding numbers on variables.
  return IRContext::kAnalysisDefUse | IRContext::kAnalysisInstrToBlockMapping |
         IRContext::kAnalysisDecorations | IRContext::kAnalysisCombinators |
         IRContext::kAnalysisCFG | IRContext::kAnalysisDominatorAnalysis |
         IRContext::kAnalysisLoopAnalysis | IRContext::kAnalysisNameMap |
         IRContext::kAnalysisScalarEvolution |
         IRContext::kAnalysisRegisterPressure |
         IRContext::kAnalysisValueNumberTable |
         IRContext::kAnalysisStructuredCFG | IRContext::kAnalysisBuiltinVarId |
         IRContext::kAnalysisIdToFuncMapping | IRContext::kAnalysisConstants |
         IRContext::kAnalysisTypes | IRContext::kAnalysisDebugInfo |
         IRContext::kAnalysisLiveness;
}

// Orders variable binding info objects.
// * The binding number is most signficant;
// * Then a sampler-like object compares greater than non-sampler like object.
// * Otherwise compare based on variable ID.
// This provides a total order among bindings in a descriptor set for a valid
// Vulkan module.
bool Less(const VarBindingInfo* const lhs, const VarBindingInfo* const rhs) {
  if (lhs->binding() < rhs->binding()) return true;
  if (lhs->binding() > rhs->binding()) return false;

  // Examine types.
  // In valid Vulkan the only conflict can occur between
  // images and samplers.  We only care about a specific
  // comparison when one is a image-like thing and the other
  // is a sampler-like thing of the same shape.  So unwrap
  // types until we hit one of those two.

  auto* def_use_mgr = lhs->var->context()->get_def_use_mgr();

  // Returns the type found by iteratively following pointer pointee type,
  // or array element type.
  auto unwrap = [&def_use_mgr](Instruction* ty) {
    bool keep_going = true;
    do {
      switch (ty->opcode()) {
        case spv::Op::OpTypePointer:
          ty = def_use_mgr->GetDef(ty->GetSingleWordInOperand(1));
          break;
        case spv::Op::OpTypeArray:
        case spv::Op::OpTypeRuntimeArray:
          ty = def_use_mgr->GetDef(ty->GetSingleWordInOperand(0));
          break;
        default:
          keep_going = false;
          break;
      }
    } while (keep_going);
    return ty;
  };

  auto* lhs_ty = unwrap(def_use_mgr->GetDef(lhs->var->type_id()));
  auto* rhs_ty = unwrap(def_use_mgr->GetDef(rhs->var->type_id()));
  if (lhs_ty->opcode() == rhs_ty->opcode()) {
    // Pick based on variable ID.
    return lhs->var->result_id() < rhs->var->result_id();
  }
  // A sampler is always greater than an image.
  if (lhs_ty->opcode() == spv::Op::OpTypeSampler) {
    return false;
  }
  if (rhs_ty->opcode() == spv::Op::OpTypeSampler) {
    return true;
  }
  // Pick based on variable ID.
  return lhs->var->result_id() < rhs->var->result_id();
}

// Summarizes the caller-callee relationships between functions in a module.
class CallGraph {
 public:
  // Returns the list of all functions statically reachable from entry points,
  // where callees precede callers.
  const std::vector<uint32_t>& CalleesBeforeCallers() const {
    return visit_order_;
  }
  // Returns the list functions called from a given function.
  const std::unordered_set<uint32_t>& Callees(uint32_t caller) {
    return calls_[caller];
  }

  CallGraph(IRContext& context) {
    // Populate calls_.
    std::queue<uint32_t> callee_queue;
    for (const auto& fn : *context.module()) {
      auto& callees = calls_[fn.result_id()];
      context.AddCalls(&fn, &callee_queue);
      while (!callee_queue.empty()) {
        callees.insert(callee_queue.front());
        callee_queue.pop();
      }
    }

    // Perform depth-first search, starting from each entry point.
    // Populates visit_order_.
    for (const auto& ep : context.module()->entry_points()) {
      Visit(ep.GetSingleWordInOperand(1));
    }
  }

 private:
  // Visits a function, recursively visiting its callees. Adds this ID
  // to the visit_order after all callees have been visited.
  void Visit(uint32_t func_id) {
    if (visited_.count(func_id)) {
      return;
    }
    visited_.insert(func_id);
    for (auto callee_id : calls_[func_id]) {
      Visit(callee_id);
    }
    visit_order_.push_back(func_id);
  }

  // Maps the ID of a function to the IDs of functions it calls.
  std::unordered_map<uint32_t, std::unordered_set<uint32_t>> calls_;

  // IDs of visited functions;
  std::unordered_set<uint32_t> visited_;
  // IDs of functions, where callees precede callers.
  std::vector<uint32_t> visit_order_;
};

// Returns vector binding info for all resource variables in the module.
auto GetVarBindings(IRContext& context) {
  std::vector<VarBindingInfo> vars;
  auto* deco_mgr = context.get_decoration_mgr();
  for (auto& inst : context.module()->types_values()) {
    if (inst.opcode() == spv::Op::OpVariable) {
      Instruction* descriptor_set_deco = nullptr;
      Instruction* binding_deco = nullptr;
      for (auto* deco : deco_mgr->GetDecorationsFor(inst.result_id(), false)) {
        switch (static_cast<spv::Decoration>(deco->GetSingleWordInOperand(1))) {
          case spv::Decoration::DescriptorSet:
            assert(!descriptor_set_deco);
            descriptor_set_deco = deco;
            break;
          case spv::Decoration::Binding:
            assert(!binding_deco);
            binding_deco = deco;
            break;
          default:
            break;
        }
      }
      if (descriptor_set_deco && binding_deco) {
        vars.push_back({&inst, descriptor_set_deco->GetSingleWordInOperand(2),
                        binding_deco});
      }
    }
  }
  return vars;
}

// Merges the bindings from source into sink. Maintains order and uniqueness
// within a list of bindings.
void Merge(DescriptorSets& sink, const DescriptorSets& source) {
  for (auto index_and_bindings : source) {
    const uint32_t index = index_and_bindings.first;
    const BindingList& src1 = index_and_bindings.second;
    const BindingList& src2 = sink[index];
    BindingList merged;
    merged.resize(src1.size() + src2.size());
    auto merged_end = std::merge(src1.begin(), src1.end(), src2.begin(),
                                 src2.end(), merged.begin(), Less);
    auto unique_end = std::unique(merged.begin(), merged_end);
    merged.resize(unique_end - merged.begin());
    sink[index] = std::move(merged);
  }
}

// Resolves conflicts within this binding list, so the binding number on an
// item is at least one more than the binding number on the previous item.
// When this does not yet hold, increase the binding number on the second
// item in the pair. Returns true if any changes were applied.
bool ResolveConflicts(BindingList& bl) {
  bool changed = false;
  for (size_t i = 1; i < bl.size(); i++) {
    const auto prev_num = bl[i - 1]->binding();
    if (prev_num >= bl[i]->binding()) {
      bl[i]->updateBinding(prev_num + 1);
      changed = true;
    }
  }
  return changed;
}

Pass::Status ResolveBindingConflictsPass::Process() {
  // Assumes the descriptor set and binding decorations are not provided
  // via decoration groups.  Decoration groups were deprecated in SPIR-V 1.3
  // Revision 6.  I have not seen any compiler generate them. --dneto

  auto vars = GetVarBindings(*context());

  // Maps a function ID to the variables used directly or indirectly by the
  // function, organized into descriptor sets. Each descriptor set
  // consists of a BindingList of distinct variables.
  std::unordered_map<uint32_t, DescriptorSets> used_vars;

  // Determine variables directly used by functions.
  auto* def_use_mgr = context()->get_def_use_mgr();
  for (auto& var : vars) {
    std::unordered_set<uint32_t> visited_functions_for_var;
    def_use_mgr->ForEachUser(var.var, [&](Instruction* user) {
      if (auto* block = context()->get_instr_block(user)) {
        auto* fn = block->GetParent();
        assert(fn);
        const auto fn_id = fn->result_id();
        if (visited_functions_for_var.insert(fn_id).second) {
          used_vars[fn_id][var.descriptor_set].push_back(&var);
        }
      }
    });
  }

  // Sort within a descriptor set by binding number.
  for (auto& sets_for_fn : used_vars) {
    for (auto& ds : sets_for_fn.second) {
      BindingList& bindings = ds.second;
      std::stable_sort(bindings.begin(), bindings.end(), Less);
    }
  }

  // Propagate from callees to callers.
  CallGraph call_graph(*context());
  for (const uint32_t caller : call_graph.CalleesBeforeCallers()) {
    DescriptorSets& caller_ds = used_vars[caller];
    for (const uint32_t callee : call_graph.Callees(caller)) {
      Merge(caller_ds, used_vars[callee]);
    }
  }

  // At this point, the descriptor sets associated with each entry point
  // capture exactly the set of resource variables statically used
  // by the static call tree of that entry point.

  // Resolve conflicts.
  // VarBindingInfo objects may be shared between the bindings lists.
  // Updating a binding in one list can require updating another list later.
  // So repeat updates until settling.

  // The union of BindingLists across all entry points.
  std::vector<BindingList*> ep_bindings;

  for (auto& ep : context()->module()->entry_points()) {
    for (auto& ds : used_vars[ep.GetSingleWordInOperand(1)]) {
      BindingList& bindings = ds.second;
      ep_bindings.push_back(&bindings);
    }
  }
  bool modified = false;
  bool found_conflict;
  do {
    found_conflict = false;
    for (BindingList* bl : ep_bindings) {
      found_conflict |= ResolveConflicts(*bl);
    }
    modified |= found_conflict;
  } while (found_conflict);

  return modified ? Pass::Status::SuccessWithChange
                  : Pass::Status::SuccessWithoutChange;
}

}  // namespace opt
}  // namespace spvtools
