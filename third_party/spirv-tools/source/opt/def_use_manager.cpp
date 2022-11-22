// Copyright (c) 2016 Google Inc.
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

#include "source/opt/def_use_manager.h"
#include "source/util/make_unique.h"

namespace spvtools {
namespace opt {
namespace analysis {

// Don't compact before we have a reasonable number of ids allocated (~32kb).
static const size_t kCompactThresholdMinTotalIds = (8 * 1024);
// Compact when fewer than this fraction of the storage is used (should be 2^n
// for performance).
static const size_t kCompactThresholdFractionFreeIds = 8;

DefUseManager::DefUseManager() {
  use_pool_ = MakeUnique<UseListPool>();
  used_id_pool_ = MakeUnique<UsedIdListPool>();
}

void DefUseManager::AnalyzeInstDef(Instruction* inst) {
  const uint32_t def_id = inst->result_id();
  if (def_id != 0) {
    auto iter = id_to_def_.find(def_id);
    if (iter != id_to_def_.end()) {
      // Clear the original instruction that defining the same result id of the
      // new instruction.
      ClearInst(iter->second);
    }
    id_to_def_[def_id] = inst;
  } else {
    ClearInst(inst);
  }
}

void DefUseManager::AnalyzeInstUse(Instruction* inst) {
  // It might have existed before.
  EraseUseRecordsOfOperandIds(inst);

  // Create entry for the given instruction. Note that the instruction may
  // not have any in-operands. In such cases, we still need a entry for those
  // instructions so this manager knows it has seen the instruction later.
  UsedIdList& used_ids =
      inst_to_used_id_.insert({inst, UsedIdList(used_id_pool_.get())})
          .first->second;

  for (uint32_t i = 0; i < inst->NumOperands(); ++i) {
    switch (inst->GetOperand(i).type) {
      // For any id type but result id type
      case SPV_OPERAND_TYPE_ID:
      case SPV_OPERAND_TYPE_TYPE_ID:
      case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID:
      case SPV_OPERAND_TYPE_SCOPE_ID: {
        uint32_t use_id = inst->GetSingleWordOperand(i);
        Instruction* def = GetDef(use_id);
        assert(def && "Definition is not registered.");

        // Add to inst's use records
        used_ids.push_back(use_id);

        // Add to the users, taking care to avoid adding duplicates.  We know
        // the duplicate for this instruction will always be at the tail.
        UseList& list = inst_to_users_.insert({def, UseList(use_pool_.get())})
                            .first->second;
        if (list.empty() || list.back() != inst) {
          list.push_back(inst);
        }
      } break;
      default:
        break;
    }
  }
}

void DefUseManager::AnalyzeInstDefUse(Instruction* inst) {
  AnalyzeInstDef(inst);
  AnalyzeInstUse(inst);
  // Analyze lines last otherwise they will be cleared when inst is
  // cleared by preceding two calls
  for (auto& l_inst : inst->dbg_line_insts()) AnalyzeInstDefUse(&l_inst);
}

void DefUseManager::UpdateDefUse(Instruction* inst) {
  const uint32_t def_id = inst->result_id();
  if (def_id != 0) {
    auto iter = id_to_def_.find(def_id);
    if (iter == id_to_def_.end()) {
      AnalyzeInstDef(inst);
    }
  }
  AnalyzeInstUse(inst);
}

Instruction* DefUseManager::GetDef(uint32_t id) {
  auto iter = id_to_def_.find(id);
  if (iter == id_to_def_.end()) return nullptr;
  return iter->second;
}

const Instruction* DefUseManager::GetDef(uint32_t id) const {
  const auto iter = id_to_def_.find(id);
  if (iter == id_to_def_.end()) return nullptr;
  return iter->second;
}

bool DefUseManager::WhileEachUser(
    const Instruction* def, const std::function<bool(Instruction*)>& f) const {
  // Ensure that |def| has been registered.
  assert(def && (!def->HasResultId() || def == GetDef(def->result_id())) &&
         "Definition is not registered.");
  if (!def->HasResultId()) return true;

  auto iter = inst_to_users_.find(def);
  if (iter != inst_to_users_.end()) {
    for (Instruction* user : iter->second) {
      if (!f(user)) return false;
    }
  }
  return true;
}

bool DefUseManager::WhileEachUser(
    uint32_t id, const std::function<bool(Instruction*)>& f) const {
  return WhileEachUser(GetDef(id), f);
}

void DefUseManager::ForEachUser(
    const Instruction* def, const std::function<void(Instruction*)>& f) const {
  WhileEachUser(def, [&f](Instruction* user) {
    f(user);
    return true;
  });
}

void DefUseManager::ForEachUser(
    uint32_t id, const std::function<void(Instruction*)>& f) const {
  ForEachUser(GetDef(id), f);
}

bool DefUseManager::WhileEachUse(
    const Instruction* def,
    const std::function<bool(Instruction*, uint32_t)>& f) const {
  // Ensure that |def| has been registered.
  assert(def && (!def->HasResultId() || def == GetDef(def->result_id())) &&
         "Definition is not registered.");
  if (!def->HasResultId()) return true;

  auto iter = inst_to_users_.find(def);
  if (iter != inst_to_users_.end()) {
    for (Instruction* user : iter->second) {
      for (uint32_t idx = 0; idx != user->NumOperands(); ++idx) {
        const Operand& op = user->GetOperand(idx);
        if (op.type != SPV_OPERAND_TYPE_RESULT_ID && spvIsIdType(op.type)) {
          if (def->result_id() == op.words[0]) {
            if (!f(user, idx)) return false;
          }
        }
      }
    }
  }
  return true;
}

bool DefUseManager::WhileEachUse(
    uint32_t id, const std::function<bool(Instruction*, uint32_t)>& f) const {
  return WhileEachUse(GetDef(id), f);
}

void DefUseManager::ForEachUse(
    const Instruction* def,
    const std::function<void(Instruction*, uint32_t)>& f) const {
  WhileEachUse(def, [&f](Instruction* user, uint32_t index) {
    f(user, index);
    return true;
  });
}

void DefUseManager::ForEachUse(
    uint32_t id, const std::function<void(Instruction*, uint32_t)>& f) const {
  ForEachUse(GetDef(id), f);
}

uint32_t DefUseManager::NumUsers(const Instruction* def) const {
  uint32_t count = 0;
  ForEachUser(def, [&count](Instruction*) { ++count; });
  return count;
}

uint32_t DefUseManager::NumUsers(uint32_t id) const {
  return NumUsers(GetDef(id));
}

uint32_t DefUseManager::NumUses(const Instruction* def) const {
  uint32_t count = 0;
  ForEachUse(def, [&count](Instruction*, uint32_t) { ++count; });
  return count;
}

uint32_t DefUseManager::NumUses(uint32_t id) const {
  return NumUses(GetDef(id));
}

std::vector<Instruction*> DefUseManager::GetAnnotations(uint32_t id) const {
  std::vector<Instruction*> annos;
  const Instruction* def = GetDef(id);
  if (!def) return annos;

  ForEachUser(def, [&annos](Instruction* user) {
    if (IsAnnotationInst(user->opcode())) {
      annos.push_back(user);
    }
  });
  return annos;
}

void DefUseManager::AnalyzeDefUse(Module* module) {
  if (!module) return;
  // Analyze all the defs before any uses to catch forward references.
  module->ForEachInst(
      std::bind(&DefUseManager::AnalyzeInstDef, this, std::placeholders::_1),
      true);
  module->ForEachInst(
      std::bind(&DefUseManager::AnalyzeInstUse, this, std::placeholders::_1),
      true);
}

void DefUseManager::ClearInst(Instruction* inst) {
  if (inst_to_used_id_.find(inst) != inst_to_used_id_.end()) {
    EraseUseRecordsOfOperandIds(inst);
    uint32_t const result_id = inst->result_id();
    if (result_id != 0) {
      // For each using instruction, remove result_id from their used ids.
      auto iter = inst_to_users_.find(inst);
      if (iter != inst_to_users_.end()) {
        for (Instruction* use : iter->second) {
          inst_to_used_id_.at(use).remove_first(result_id);
        }
        inst_to_users_.erase(iter);
      }
      id_to_def_.erase(inst->result_id());
    }
  }
}

void DefUseManager::EraseUseRecordsOfOperandIds(const Instruction* inst) {
  // Go through all ids used by this instruction, remove this instruction's
  // uses of them.
  auto iter = inst_to_used_id_.find(inst);
  if (iter != inst_to_used_id_.end()) {
    const UsedIdList& used_ids = iter->second;
    for (uint32_t def_id : used_ids) {
      auto def_iter = inst_to_users_.find(GetDef(def_id));
      if (def_iter != inst_to_users_.end()) {
        def_iter->second.remove_first(const_cast<Instruction*>(inst));
      }
    }
    inst_to_used_id_.erase(inst);

    // If we're using only a fraction of the space in used_ids_, compact storage
    // to prevent memory usage from being unbounded.
    if (used_id_pool_->total_nodes() > kCompactThresholdMinTotalIds &&
        used_id_pool_->used_nodes() <
            used_id_pool_->total_nodes() / kCompactThresholdFractionFreeIds) {
      CompactStorage();
    }
  }
}

void DefUseManager::CompactStorage() {
  CompactUseRecords();
  CompactUsedIds();
}

void DefUseManager::CompactUseRecords() {
  std::unique_ptr<UseListPool> new_pool = MakeUnique<UseListPool>();
  for (auto& iter : inst_to_users_) {
    iter.second.move_nodes(new_pool.get());
  }
  use_pool_ = std::move(new_pool);
}

void DefUseManager::CompactUsedIds() {
  std::unique_ptr<UsedIdListPool> new_pool = MakeUnique<UsedIdListPool>();
  for (auto& iter : inst_to_used_id_) {
    iter.second.move_nodes(new_pool.get());
  }
  used_id_pool_ = std::move(new_pool);
}

bool CompareAndPrintDifferences(const DefUseManager& lhs,
                                const DefUseManager& rhs) {
  bool same = true;

  if (lhs.id_to_def_ != rhs.id_to_def_) {
    for (auto p : lhs.id_to_def_) {
      if (rhs.id_to_def_.find(p.first) == rhs.id_to_def_.end()) {
        printf("Diff in id_to_def: missing value in rhs\n");
      }
    }
    for (auto p : rhs.id_to_def_) {
      if (lhs.id_to_def_.find(p.first) == lhs.id_to_def_.end()) {
        printf("Diff in id_to_def: missing value in lhs\n");
      }
    }
    same = false;
  }

  for (const auto& l : lhs.inst_to_used_id_) {
    std::set<uint32_t> ul, ur;
    lhs.ForEachUse(l.first,
                   [&ul](Instruction*, uint32_t id) { ul.insert(id); });
    rhs.ForEachUse(l.first,
                   [&ur](Instruction*, uint32_t id) { ur.insert(id); });
    if (ul.size() != ur.size()) {
      printf(
          "Diff in inst_to_used_id_: different number of used ids (%zu != %zu)",
          ul.size(), ur.size());
      same = false;
    } else if (ul != ur) {
      printf("Diff in inst_to_used_id_: different used ids\n");
      same = false;
    }
  }
  for (const auto& r : rhs.inst_to_used_id_) {
    auto iter_l = lhs.inst_to_used_id_.find(r.first);
    if (r.second.empty() &&
        !(iter_l == lhs.inst_to_used_id_.end() || iter_l->second.empty())) {
      printf("Diff in inst_to_used_id_: unexpected instr in rhs\n");
      same = false;
    }
  }

  for (const auto& l : lhs.inst_to_users_) {
    std::set<Instruction*> ul, ur;
    lhs.ForEachUser(l.first, [&ul](Instruction* use) { ul.insert(use); });
    rhs.ForEachUser(l.first, [&ur](Instruction* use) { ur.insert(use); });
    if (ul.size() != ur.size()) {
      printf("Diff in inst_to_users_: different number of users (%zu != %zu)",
             ul.size(), ur.size());
      same = false;
    } else if (ul != ur) {
      printf("Diff in inst_to_users_: different users\n");
      same = false;
    }
  }
  for (const auto& r : rhs.inst_to_users_) {
    auto iter_l = lhs.inst_to_users_.find(r.first);
    if (r.second.empty() &&
        !(iter_l == lhs.inst_to_users_.end() || iter_l->second.empty())) {
      printf("Diff in inst_to_users_: unexpected instr in rhs\n");
      same = false;
    }
  }
  return same;
}

}  // namespace analysis
}  // namespace opt
}  // namespace spvtools
