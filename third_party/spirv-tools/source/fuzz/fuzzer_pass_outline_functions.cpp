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

#include "source/fuzz/fuzzer_pass_outline_functions.h"

#include <vector>

#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/transformation_outline_function.h"

namespace spvtools {
namespace fuzz {

FuzzerPassOutlineFunctions::FuzzerPassOutlineFunctions(
    opt::IRContext* ir_context, FactManager* fact_manager,
    FuzzerContext* fuzzer_context,
    protobufs::TransformationSequence* transformations)
    : FuzzerPass(ir_context, fact_manager, fuzzer_context, transformations) {}

FuzzerPassOutlineFunctions::~FuzzerPassOutlineFunctions() = default;

void FuzzerPassOutlineFunctions::Apply() {
  std::vector<opt::Function*> original_functions;
  for (auto& function : *GetIRContext()->module()) {
    original_functions.push_back(&function);
  }
  for (auto& function : original_functions) {
    if (!GetFuzzerContext()->ChoosePercentage(
            GetFuzzerContext()->GetChanceOfOutliningFunction())) {
      continue;
    }
    std::vector<opt::BasicBlock*> blocks;
    for (auto& block : *function) {
      blocks.push_back(&block);
    }
    auto entry_block = blocks[GetFuzzerContext()->RandomIndex(blocks)];
    auto dominator_analysis = GetIRContext()->GetDominatorAnalysis(function);
    auto postdominator_analysis =
        GetIRContext()->GetPostDominatorAnalysis(function);
    std::vector<opt::BasicBlock*> candidate_exit_blocks;
    for (auto postdominates_entry_block = entry_block;
         postdominates_entry_block != nullptr;
         postdominates_entry_block = postdominator_analysis->ImmediateDominator(
             postdominates_entry_block)) {
      if (dominator_analysis->Dominates(entry_block,
                                        postdominates_entry_block)) {
        candidate_exit_blocks.push_back(postdominates_entry_block);
      }
    }
    if (candidate_exit_blocks.empty()) {
      continue;
    }
    auto exit_block = candidate_exit_blocks[GetFuzzerContext()->RandomIndex(
        candidate_exit_blocks)];

    auto region_blocks = TransformationOutlineFunction::GetRegionBlocks(
        GetIRContext(), entry_block, exit_block);
    std::map<uint32_t, uint32_t> input_id_to_fresh_id;
    for (auto id : TransformationOutlineFunction::GetRegionInputIds(
             GetIRContext(), region_blocks, exit_block)) {
      input_id_to_fresh_id[id] = GetFuzzerContext()->GetFreshId();
    }
    std::map<uint32_t, uint32_t> output_id_to_fresh_id;
    for (auto id : TransformationOutlineFunction::GetRegionOutputIds(
             GetIRContext(), region_blocks, exit_block)) {
      output_id_to_fresh_id[id] = GetFuzzerContext()->GetFreshId();
    }
    TransformationOutlineFunction transformation(
        entry_block->id(), exit_block->id(),
        /*new_function_struct_return_type_id*/
        GetFuzzerContext()->GetFreshId(),
        /*new_function_type_id*/ GetFuzzerContext()->GetFreshId(),
        /*new_function_id*/ GetFuzzerContext()->GetFreshId(),
        /*new_function_region_entry_block*/
        GetFuzzerContext()->GetFreshId(),
        /*new_caller_result_id*/ GetFuzzerContext()->GetFreshId(),
        /*new_callee_result_id*/ GetFuzzerContext()->GetFreshId(),
        /*input_id_to_fresh_id*/ std::move(input_id_to_fresh_id),
        /*output_id_to_fresh_id*/ std::move(output_id_to_fresh_id));
    if (transformation.IsApplicable(GetIRContext(), *GetFactManager())) {
      transformation.Apply(GetIRContext(), GetFactManager());
      *GetTransformations()->add_transformation() = transformation.ToMessage();
    }
  }
}

}  // namespace fuzz
}  // namespace spvtools
