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

#ifndef SOURCE_OPT_DEF_USE_MANAGER_H_
#define SOURCE_OPT_DEF_USE_MANAGER_H_

#include <set>
#include <unordered_map>
#include <vector>

#include "source/opt/instruction.h"
#include "source/opt/module.h"
#include "source/util/pooled_linked_list.h"
#include "spirv-tools/libspirv.hpp"

namespace spvtools {
namespace opt {
namespace analysis {

// Class for representing a use of id. Note that:
// * Result type id is a use.
// * Ids referenced in OpSectionMerge & OpLoopMerge are considered as use.
// * Ids referenced in OpPhi's in operands are considered as use.
struct Use {
  Instruction* inst;       // Instruction using the id.
  uint32_t operand_index;  // logical operand index of the id use. This can be
                           // the index of result type id.
};

inline bool operator==(const Use& lhs, const Use& rhs) {
  return lhs.inst == rhs.inst && lhs.operand_index == rhs.operand_index;
}

inline bool operator!=(const Use& lhs, const Use& rhs) { return !(lhs == rhs); }

inline bool operator<(const Use& lhs, const Use& rhs) {
  if (lhs.inst < rhs.inst) return true;
  if (lhs.inst > rhs.inst) return false;
  return lhs.operand_index < rhs.operand_index;
}

// A class for analyzing and managing defs and uses in an Module.
class DefUseManager {
 public:
  using IdToDefMap = std::unordered_map<uint32_t, Instruction*>;

  // Constructs a def-use manager from the given |module|. All internal messages
  // will be communicated to the outside via the given message |consumer|. This
  // instance only keeps a reference to the |consumer|, so the |consumer| should
  // outlive this instance.
  DefUseManager(Module* module) : DefUseManager() { AnalyzeDefUse(module); }

  DefUseManager(const DefUseManager&) = delete;
  DefUseManager(DefUseManager&&) = delete;
  DefUseManager& operator=(const DefUseManager&) = delete;
  DefUseManager& operator=(DefUseManager&&) = delete;

  // Analyzes the defs in the given |inst|.
  void AnalyzeInstDef(Instruction* inst);

  // Analyzes the uses in the given |inst|.
  //
  // All operands of |inst| must be analyzed as defs.
  void AnalyzeInstUse(Instruction* inst);

  // Analyzes the defs and uses in the given |inst|.
  void AnalyzeInstDefUse(Instruction* inst);

  // Returns the def instruction for the given |id|. If there is no instruction
  // defining |id|, returns nullptr.
  Instruction* GetDef(uint32_t id);
  const Instruction* GetDef(uint32_t id) const;

  // Runs the given function |f| on each unique user instruction of |def| (or
  // |id|).
  //
  // If one instruction uses |def| in multiple operands, that instruction will
  // only be visited once.
  //
  // |def| (or |id|) must be registered as a definition.
  void ForEachUser(const Instruction* def,
                   const std::function<void(Instruction*)>& f) const;
  void ForEachUser(uint32_t id,
                   const std::function<void(Instruction*)>& f) const;

  // Runs the given function |f| on each unique user instruction of |def| (or
  // |id|). If |f| returns false, iteration is terminated and this function
  // returns false.
  //
  // If one instruction uses |def| in multiple operands, that instruction will
  // be only be visited once.
  //
  // |def| (or |id|) must be registered as a definition.
  bool WhileEachUser(const Instruction* def,
                     const std::function<bool(Instruction*)>& f) const;
  bool WhileEachUser(uint32_t id,
                     const std::function<bool(Instruction*)>& f) const;

  // Runs the given function |f| on each unique use of |def| (or
  // |id|).
  //
  // If one instruction uses |def| in multiple operands, each operand will be
  // visited separately.
  //
  // |def| (or |id|) must be registered as a definition.
  void ForEachUse(
      const Instruction* def,
      const std::function<void(Instruction*, uint32_t operand_index)>& f) const;
  void ForEachUse(
      uint32_t id,
      const std::function<void(Instruction*, uint32_t operand_index)>& f) const;

  // Runs the given function |f| on each unique use of |def| (or
  // |id|). If |f| returns false, iteration is terminated and this function
  // returns false.
  //
  // If one instruction uses |def| in multiple operands, each operand will be
  // visited separately.
  //
  // |def| (or |id|) must be registered as a definition.
  bool WhileEachUse(
      const Instruction* def,
      const std::function<bool(Instruction*, uint32_t operand_index)>& f) const;
  bool WhileEachUse(
      uint32_t id,
      const std::function<bool(Instruction*, uint32_t operand_index)>& f) const;

  // Returns the number of users of |def| (or |id|).
  uint32_t NumUsers(const Instruction* def) const;
  uint32_t NumUsers(uint32_t id) const;

  // Returns the number of uses of |def| (or |id|).
  uint32_t NumUses(const Instruction* def) const;
  uint32_t NumUses(uint32_t id) const;

  // Returns the annotation instrunctions which are a direct use of the given
  // |id|. This means when the decorations are applied through decoration
  // group(s), this function will just return the OpGroupDecorate
  // instruction(s) which refer to the given id as an operand. The OpDecorate
  // instructions which decorate the decoration group will not be returned.
  std::vector<Instruction*> GetAnnotations(uint32_t id) const;

  // Returns the map from ids to their def instructions.
  const IdToDefMap& id_to_defs() const { return id_to_def_; }

  // Clear the internal def-use record of the given instruction |inst|. This
  // method will update the use information of the operand ids of |inst|. The
  // record: |inst| uses an |id|, will be removed from the use records of |id|.
  // If |inst| defines an result id, the use record of this result id will also
  // be removed. Does nothing if |inst| was not analyzed before.
  void ClearInst(Instruction* inst);

  // Erases the records that a given instruction uses its operand ids.
  void EraseUseRecordsOfOperandIds(const Instruction* inst);

  friend bool CompareAndPrintDifferences(const DefUseManager&,
                                         const DefUseManager&);

  // If |inst| has not already been analysed, then analyses its definition and
  // uses.
  void UpdateDefUse(Instruction* inst);

  // Compacts any internal storage to save memory.
  void CompactStorage();

 private:
  using UseList = spvtools::utils::PooledLinkedList<Instruction*>;
  using UseListPool = spvtools::utils::PooledLinkedListNodes<Instruction*>;
  // Stores linked lists of Instructions using a def.
  using InstToUsersMap = std::unordered_map<const Instruction*, UseList>;

  using UsedIdList = spvtools::utils::PooledLinkedList<uint32_t>;
  using UsedIdListPool = spvtools::utils::PooledLinkedListNodes<uint32_t>;
  // Stores mapping from instruction to their UsedIdRange.
  using InstToUsedIdMap = std::unordered_map<const Instruction*, UsedIdList>;

  DefUseManager();

  // Analyzes the defs and uses in the given |module| and populates data
  // structures in this class. Does nothing if |module| is nullptr.
  void AnalyzeDefUse(Module* module);

  // Removes unused entries in used_records_ and used_ids_.
  void CompactUseRecords();
  void CompactUsedIds();

  IdToDefMap id_to_def_;          // Mapping from ids to their definitions
  InstToUsersMap inst_to_users_;  // Map from def to uses.
  std::unique_ptr<UseListPool> use_pool_;

  std::unique_ptr<UsedIdListPool> used_id_pool_;
  InstToUsedIdMap inst_to_used_id_;  // Map from instruction to used ids.
};

}  // namespace analysis
}  // namespace opt
}  // namespace spvtools

#endif  // SOURCE_OPT_DEF_USE_MANAGER_H_
