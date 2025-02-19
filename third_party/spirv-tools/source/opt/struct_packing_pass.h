// Copyright (c) 2024 Epic Games, Inc.
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

#ifndef SOURCE_OPT_STRUCT_PACKING_PASS_
#define SOURCE_OPT_STRUCT_PACKING_PASS_

#include <unordered_map>

#include "source/opt/ir_context.h"
#include "source/opt/module.h"
#include "source/opt/pass.h"

namespace spvtools {
namespace opt {

// This pass re-assigns all field offsets under the specified packing rules.
class StructPackingPass final : public Pass {
 public:
  enum class PackingRules {
    Undefined,
    Std140,
    Std140EnhancedLayout,
    Std430,
    Std430EnhancedLayout,
    HlslCbuffer,
    HlslCbufferPackOffset,
    Scalar,
    ScalarEnhancedLayout,
  };

  static PackingRules ParsePackingRuleFromString(const std::string& s);

  StructPackingPass(const char* structToPack, PackingRules rules);
  const char* name() const override { return "struct-packing"; }
  Status Process() override;

  IRContext::Analysis GetPreservedAnalyses() override {
    return IRContext::kAnalysisCombinators | IRContext::kAnalysisCFG |
           IRContext::kAnalysisDominatorAnalysis |
           IRContext::kAnalysisLoopAnalysis | IRContext::kAnalysisNameMap |
           IRContext::kAnalysisScalarEvolution |
           IRContext::kAnalysisStructuredCFG | IRContext::kAnalysisConstants |
           IRContext::kAnalysisDebugInfo | IRContext::kAnalysisLiveness;
  }

 private:
  void buildConstantsMap();
  uint32_t findStructIdByName(const char* structName) const;
  std::vector<const analysis::Type*> findStructMemberTypes(
      const Instruction& structDef) const;
  Status assignStructMemberOffsets(
      uint32_t structIdToPack,
      const std::vector<const analysis::Type*>& structMemberTypes);

  uint32_t getPackedAlignment(const analysis::Type& type) const;
  uint32_t getPackedSize(const analysis::Type& type) const;
  uint32_t getPackedArrayStride(const analysis::Array& arrayType) const;
  uint32_t getArrayLength(const analysis::Array& arrayType) const;
  uint32_t getConstantInt(spv::Id id) const;

 private:
  std::string structToPack_;
  PackingRules packingRules_ = PackingRules::Undefined;
  std::unordered_map<spv::Id, Instruction*> constantsMap_;
};

}  // namespace opt
}  // namespace spvtools

#endif  // SOURCE_OPT_STRUCT_PACKING_PASS_
