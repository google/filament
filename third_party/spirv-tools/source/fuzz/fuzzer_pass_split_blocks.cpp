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

#include "source/fuzz/fuzzer_pass_split_blocks.h"

#include <utility>
#include <vector>

#include "source/fuzz/transformation_split_block.h"

namespace spvtools {
namespace fuzz {

FuzzerPassSplitBlocks::FuzzerPassSplitBlocks(
    opt::IRContext* ir_context, FactManager* fact_manager,
    FuzzerContext* fuzzer_context,
    protobufs::TransformationSequence* transformations)
    : FuzzerPass(ir_context, fact_manager, fuzzer_context, transformations) {}

FuzzerPassSplitBlocks::~FuzzerPassSplitBlocks() = default;

void FuzzerPassSplitBlocks::Apply() {
  // Gather up pointers to all the blocks in the module.  We are then able to
  // iterate over these pointers and split the blocks to which they point;
  // we cannot safely split blocks while we iterate through the module.
  std::vector<opt::BasicBlock*> blocks;
  for (auto& function : *GetIRContext()->module()) {
    for (auto& block : function) {
      blocks.push_back(&block);
    }
  }

  // Now go through all the block pointers that were gathered.
  for (auto& block : blocks) {
    // Probabilistically decide whether to try to split this block.
    if (!GetFuzzerContext()->ChoosePercentage(
            GetFuzzerContext()->GetChanceOfSplittingBlock())) {
      // We are not going to try to split this block.
      continue;
    }
    // We are going to try to split this block.  We now need to choose where
    // to split it.  We do this by finding a base instruction that has a
    // result id, and an offset from that base instruction.  We would like
    // offsets to be as small as possible and ideally 0 - we only need offsets
    // because not all instructions can be identified by a result id (e.g.
    // OpStore instructions cannot).
    std::vector<std::pair<uint32_t, uint32_t>> base_offset_pairs;
    // The initial base instruction is the block label.
    uint32_t base = block->id();
    uint32_t offset = 0;
    // Consider every instruction in the block.  The label is excluded: it is
    // only necessary to consider it as a base in case the first instruction
    // in the block does not have a result id.
    for (auto& inst : *block) {
      if (inst.HasResultId()) {
        // In the case that the instruction has a result id, we use the
        // instruction as its own base, with zero offset.
        base = inst.result_id();
        offset = 0;
      } else {
        // The instruction does not have a result id, so we need to identify
        // it via the latest instruction that did have a result id (base), and
        // an incremented offset.
        offset++;
      }
      base_offset_pairs.emplace_back(base, offset);
    }
    // Having identified all the places we might be able to split the block,
    // we choose one of them.
    auto base_offset =
        base_offset_pairs[GetFuzzerContext()->RandomIndex(base_offset_pairs)];
    auto transformation =
        TransformationSplitBlock(base_offset.first, base_offset.second,
                                 GetFuzzerContext()->GetFreshId());
    // If the position we have chosen turns out to be a valid place to split
    // the block, we apply the split. Otherwise the block just doesn't get
    // split.
    if (transformation.IsApplicable(GetIRContext(), *GetFactManager())) {
      transformation.Apply(GetIRContext(), GetFactManager());
      *GetTransformations()->add_transformation() = transformation.ToMessage();
    }
  }
}

}  // namespace fuzz
}  // namespace spvtools
