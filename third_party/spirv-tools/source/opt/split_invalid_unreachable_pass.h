// Copyright (c) 2019 Google LLC.
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

#ifndef SOURCE_OPT_SPLIT_INVALID_UNREACHABLE_PASS_H_
#define SOURCE_OPT_SPLIT_INVALID_UNREACHABLE_PASS_H_

#include "source/opt/ir_context.h"
#include "source/opt/module.h"
#include "source/opt/pass.h"

namespace spvtools {
namespace opt {

// Attempts to legalize for WebGPU by splitting up invalid unreachable blocks.
// Specifically, looking for cases of unreachable merge-blocks and
// continue-targets that are used more then once, which is illegal in WebGPU.
class SplitInvalidUnreachablePass : public Pass {
 public:
  const char* name() const override { return "split-invalid-unreachable"; }
  Status Process() override;

  IRContext::Analysis GetPreservedAnalyses() override {
    return IRContext::kAnalysisInstrToBlockMapping |
           IRContext::kAnalysisDecorations | IRContext::kAnalysisCombinators |
           IRContext::kAnalysisCFG | IRContext::kAnalysisDominatorAnalysis |
           IRContext::kAnalysisLoopAnalysis | IRContext::kAnalysisNameMap |
           IRContext::kAnalysisScalarEvolution |
           IRContext::kAnalysisRegisterPressure |
           IRContext::kAnalysisValueNumberTable |
           IRContext::kAnalysisStructuredCFG |
           IRContext::kAnalysisBuiltinVarId |
           IRContext::kAnalysisIdToFuncMapping | IRContext::kAnalysisTypes |
           IRContext::kAnalysisDefUse | IRContext::kAnalysisConstants;
  }
};

}  // namespace opt
}  // namespace spvtools

#endif  // SOURCE_OPT_SPLIT_INVALID_UNREACHABLE_PASS_H_
