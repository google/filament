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

#include "source/fuzz/fuzzer_pass_add_dead_blocks.h"

#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/transformation_add_dead_block.h"

namespace spvtools {
namespace fuzz {

FuzzerPassAddDeadBlocks::FuzzerPassAddDeadBlocks(
    opt::IRContext* ir_context, FactManager* fact_manager,
    FuzzerContext* fuzzer_context,
    protobufs::TransformationSequence* transformations)
    : FuzzerPass(ir_context, fact_manager, fuzzer_context, transformations) {}

FuzzerPassAddDeadBlocks::~FuzzerPassAddDeadBlocks() = default;

void FuzzerPassAddDeadBlocks::Apply() {
  // We iterate over all blocks in the module collecting up those at which we
  // might add a branch to a new dead block.  We then loop over all such
  // candidates and actually apply transformations.  This separation is to
  // avoid modifying the module as we traverse it.
  std::vector<TransformationAddDeadBlock> candidate_transformations;
  for (auto& function : *GetIRContext()->module()) {
    for (auto& block : function) {
      if (!GetFuzzerContext()->ChoosePercentage(
              GetFuzzerContext()->GetChanceOfAddingDeadBlock())) {
        continue;
      }
      // We speculatively create a transformation, and then apply it (below) if
      // it turns out to be applicable.  This avoids duplicating the logic for
      // applicability checking.
      //
      // It means that fresh ids for transformations that turn out not to be
      // applicable end up being unused.
      candidate_transformations.emplace_back(TransformationAddDeadBlock(
          GetFuzzerContext()->GetFreshId(), block.id(),
          GetFuzzerContext()->ChooseEven()));
    }
  }
  // Apply all those transformations that are in fact applicable.
  for (auto& transformation : candidate_transformations) {
    if (transformation.IsApplicable(GetIRContext(), *GetFactManager())) {
      transformation.Apply(GetIRContext(), GetFactManager());
      *GetTransformations()->add_transformation() = transformation.ToMessage();
    }
  }
}

}  // namespace fuzz
}  // namespace spvtools
