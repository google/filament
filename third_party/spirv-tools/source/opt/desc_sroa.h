// Copyright (c) 2019 Google LLC
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

#ifndef SOURCE_OPT_DESC_SROA_H_
#define SOURCE_OPT_DESC_SROA_H_

#include <cstdio>
#include <memory>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "source/opt/function.h"
#include "source/opt/pass.h"
#include "source/opt/type_manager.h"

namespace spvtools {
namespace opt {

// Documented in optimizer.hpp
class DescriptorScalarReplacement : public Pass {
 public:
  DescriptorScalarReplacement() {}

  const char* name() const override { return "descriptor-scalar-replacement"; }

  Status Process() override;

  IRContext::Analysis GetPreservedAnalyses() override {
    return IRContext::kAnalysisDefUse |
           IRContext::kAnalysisInstrToBlockMapping |
           IRContext::kAnalysisCombinators | IRContext::kAnalysisCFG |
           IRContext::kAnalysisConstants | IRContext::kAnalysisTypes;
  }

 private:
  // Returns true if |var| is an OpVariable instruction that represents a
  // descriptor array.  These are the variables that we want to replace.
  bool IsCandidate(Instruction* var);

  // Replaces all references to |var| by new variables, one for each element of
  // the array |var|.  The binding for the new variables corresponding to
  // element i will be the binding of |var| plus i.  Returns true if successful.
  bool ReplaceCandidate(Instruction* var);

  // Replaces the base address |var| in the OpAccessChain or
  // OpInBoundsAccessChain instruction |use| by the variable that the access
  // chain accesses.  The first index in |use| must be an |OpConstant|.  Returns
  // |true| if successful.
  bool ReplaceAccessChain(Instruction* var, Instruction* use);

  // Returns the id of the variable that will be used to replace the |idx|th
  // element of |var|.  The variable is created if it has not already been
  // created.
  uint32_t GetReplacementVariable(Instruction* var, uint32_t idx);

  // Returns the id of a new variable that can be used to replace the |idx|th
  // element of |var|.
  uint32_t CreateReplacementVariable(Instruction* var, uint32_t idx);

  // A map from an OpVariable instruction to the set of variables that will be
  // used to replace it. The entry |replacement_variables_[var][i]| is the id of
  // a variable that will be used in the place of the the ith element of the
  // array |var|. If the entry is |0|, then the variable has not been
  // created yet.
  std::map<Instruction*, std::vector<uint32_t>> replacement_variables_;
};

}  // namespace opt
}  // namespace spvtools

#endif  // SOURCE_OPT_DESC_SROA_H_
