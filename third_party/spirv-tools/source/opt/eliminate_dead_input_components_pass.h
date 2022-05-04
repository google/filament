// Copyright (c) 2022 The Khronos Group Inc.
// Copyright (c) 2022 LunarG Inc.
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

#ifndef SOURCE_OPT_ELIMINATE_DEAD_INPUT_COMPONENTS_H_
#define SOURCE_OPT_ELIMINATE_DEAD_INPUT_COMPONENTS_H_

#include <unordered_map>

#include "source/opt/ir_context.h"
#include "source/opt/module.h"
#include "source/opt/pass.h"

namespace spvtools {
namespace opt {

// See optimizer.hpp for documentation.
class EliminateDeadInputComponentsPass : public Pass {
 public:
  explicit EliminateDeadInputComponentsPass() {}

  const char* name() const override { return "reduce-load-size"; }
  Status Process() override;

  // Return the mask of preserved Analyses.
  IRContext::Analysis GetPreservedAnalyses() override {
    return IRContext::kAnalysisDefUse |
           IRContext::kAnalysisInstrToBlockMapping |
           IRContext::kAnalysisCombinators | IRContext::kAnalysisCFG |
           IRContext::kAnalysisDominatorAnalysis |
           IRContext::kAnalysisLoopAnalysis | IRContext::kAnalysisNameMap |
           IRContext::kAnalysisConstants | IRContext::kAnalysisTypes;
  }

 private:
  // Find the max constant used to index the variable declared by |var|
  // through OpAccessChain or OpInBoundsAccessChain. If any non-constant
  // indices or non-Op*AccessChain use of |var|, return |original_max|.
  unsigned FindMaxIndex(Instruction& var, unsigned original_max);

  // Change the length of the array |inst| to |length|
  void ChangeArrayLength(Instruction& inst, unsigned length);
};

}  // namespace opt
}  // namespace spvtools

#endif  // SOURCE_OPT_ELIMINATE_DEAD_INPUT_COMPONENTS_H_
