// Copyright (c) 2025 LunarG Inc.
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

#include <algorithm>
#include <map>
#include <set>
#include <vector>

#include "source/opt/pass.h"

namespace spvtools {
namespace opt {

// The canonicalize IDs pass is an optimization to improve compression of SPIR-V
// binary files via entropy reduction. It transforms SPIR-V to SPIR-V, remapping
// IDs. The resulting modules have an increased ID range (IDs are not as tightly
// packed around zero), but will compress better when multiple modules are
// compressed together, since the compressor's dictionary can find better cross
// module commonality. Remapping is accomplished via canonicalization. Thus,
// modules can be compressed one at a time with no loss of quality relative to
// operating on many modules at once.

// This pass should be run after most optimization passes except for
// --strip-debug because this pass will use OpName to canonicalize IDs. i.e. Run
// --strip-debug after this pass.

// This is a port of remap utility in glslang. There are great deal of magic
// numbers that are present throughout this code. The general goal is to replace
// the IDs with a hash value such that the distribution of IDs is deterministic
// and minimizes collisions. The magic numbers in the glslang version were
// chosen semi-arbitrarily and have been preserved in this port in order to
// maintain backward compatibility.

class CanonicalizeIdsPass : public Pass {
 public:
  CanonicalizeIdsPass() = default;
  virtual ~CanonicalizeIdsPass() = default;

  Pass::Status Process() override;

  const char* name() const override { return "canonicalize-ids"; }

 private:
  // Special values for IDs.
  static constexpr spv::Id unmapped_{spv::Id(-10000)};
  static constexpr spv::Id unused_{spv::Id(-10001)};

  // Scans the module for IDs and sets them to unmapped_.
  void ScanIds();

  // Functions to compute new IDs.
  void CanonicalizeTypeAndConst();
  spv::Id HashTypeAndConst(
      spv::Id const id) const;  // Helper for CanonicalizeTypeAndConst.
  void CanonicalizeNames();
  void CanonicalizeFunctions();
  spv::Id HashOpCode(Instruction const* const inst)
      const;  // Helper for CanonicalizeFunctions.
  void CanonicalizeRemainders();

  // Applies the new IDs.
  bool ApplyMap();

  // Methods to manage the bound field in header.
  spv::Id GetBound() const;  // All IDs must satisfy 0 < ID < bound.
  void UpdateBound();

  // Methods to map from old IDs to new IDs.
  spv::Id GetNewId(spv::Id const old_id) const { return new_id_[old_id]; }
  spv::Id SetNewId(spv::Id const old_id, spv::Id new_id);

  // Methods to manage claimed IDs.
  spv::Id ClaimNewId(spv::Id new_id);
  bool IsNewIdClaimed(spv::Id const new_id) const {
    return claimed_new_ids_.find(new_id) != claimed_new_ids_.end();
  }

  // Queries for old IDs.
  bool IsOldIdUnmapped(spv::Id const old_id) const {
    return GetNewId(old_id) == unmapped_;
  }
  bool IsOldIdUnused(spv::Id const old_id) const {
    return GetNewId(old_id) == unused_;
  }

  // Container to map old IDs to new IDs. e.g. new_id_[old_id] = new_id
  std::vector<spv::Id> new_id_;

  // IDs from the new ID space that have been claimed (faster than searching
  // through new_id_).
  std::set<spv::Id> claimed_new_ids_;

  // Helper functions for printing IDs (useful for debugging).
  std::string IdAsString(spv::Id const id) const;
  void PrintNewIds() const;

  // Containers to track IDs we want to canonicalize.
  std::vector<spv::Id> type_and_const_ids_;
  std::map<std::string, spv::Id> name_ids_;
  std::vector<spv::Id> function_ids_;
};

}  // namespace opt
}  // namespace spvtools
