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

#include "source/fuzz/fuzzer_pass_add_dead_breaks.h"

#include "source/fuzz/transformation_add_dead_break.h"
#include "source/opt/ir_context.h"

namespace spvtools {
namespace fuzz {

FuzzerPassAddDeadBreaks::FuzzerPassAddDeadBreaks(
    opt::IRContext* ir_context, FactManager* fact_manager,
    FuzzerContext* fuzzer_context,
    protobufs::TransformationSequence* transformations)
    : FuzzerPass(ir_context, fact_manager, fuzzer_context, transformations) {}

FuzzerPassAddDeadBreaks::~FuzzerPassAddDeadBreaks() = default;

void FuzzerPassAddDeadBreaks::Apply() {
  // We first collect up lots of possibly-applicable transformations.
  std::vector<TransformationAddDeadBreak> candidate_transformations;
  // We consider each function separately.
  for (auto& function : *GetIRContext()->module()) {
    // For a given function, we find all the merge blocks in that function.
    std::vector<uint32_t> merge_block_ids;
    for (auto& block : function) {
      auto maybe_merge_id = block.MergeBlockIdIfAny();
      if (maybe_merge_id) {
        merge_block_ids.push_back(maybe_merge_id);
      }
    }
    // We rather aggressively consider the possibility of adding a break from
    // every block in the function to every merge block.  Many of these will be
    // inapplicable as they would be illegal.  That's OK - we later discard the
    // ones that turn out to be no good.
    for (auto& block : function) {
      for (auto merge_block_id : merge_block_ids) {
        // TODO(https://github.com/KhronosGroup/SPIRV-Tools/issues/2856): right
        //  now we completely ignore OpPhi instructions at merge blocks.  This
        //  will lead to interesting opportunities being missed.
        auto candidate_transformation = TransformationAddDeadBreak(
            block.id(), merge_block_id, GetFuzzerContext()->ChooseEven(), {});
        if (candidate_transformation.IsApplicable(GetIRContext(),
                                                  *GetFactManager())) {
          // Only consider a transformation as a candidate if it is applicable.
          candidate_transformations.push_back(
              std::move(candidate_transformation));
        }
      }
    }
  }

  // Go through the candidate transformations that were accumulated,
  // probabilistically deciding whether to consider each one further and
  // applying the still-applicable ones that are considered further.
  //
  // We iterate through the candidate transformations in a random order by
  // repeatedly removing a random candidate transformation from the sequence
  // until no candidate transformations remain.  This is done because
  // transformations can potentially disable one another, so that iterating
  // through them in order would lead to a higher probability of
  // transformations appearing early in the sequence being applied compared
  // with later transformations.
  while (!candidate_transformations.empty()) {
    // Choose a random index into the sequence of remaining candidate
    // transformations.
    auto index = GetFuzzerContext()->RandomIndex(candidate_transformations);
    // Remove the transformation at the chosen index from the sequence.
    auto transformation = std::move(candidate_transformations[index]);
    candidate_transformations.erase(candidate_transformations.begin() + index);
    // Probabilistically decide whether to try to apply it vs. ignore it, in the
    // case that it is applicable.
    if (transformation.IsApplicable(GetIRContext(), *GetFactManager()) &&
        GetFuzzerContext()->ChoosePercentage(
            GetFuzzerContext()->GetChanceOfAddingDeadBreak())) {
      transformation.Apply(GetIRContext(), GetFactManager());
      *GetTransformations()->add_transformation() = transformation.ToMessage();
    }
  }
}

}  // namespace fuzz
}  // namespace spvtools
