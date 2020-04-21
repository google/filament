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

#include "source/fuzz/transformation_merge_blocks.h"

#include "source/fuzz/fuzzer_util.h"
#include "source/opt/block_merge_util.h"

namespace spvtools {
namespace fuzz {

TransformationMergeBlocks::TransformationMergeBlocks(
    const spvtools::fuzz::protobufs::TransformationMergeBlocks& message)
    : message_(message) {}

TransformationMergeBlocks::TransformationMergeBlocks(uint32_t block_id) {
  message_.set_block_id(block_id);
}

bool TransformationMergeBlocks::IsApplicable(
    opt::IRContext* context,
    const spvtools::fuzz::FactManager& /*unused*/) const {
  auto second_block = fuzzerutil::MaybeFindBlock(context, message_.block_id());
  // The given block must exist.
  if (!second_block) {
    return false;
  }
  // The block must have just one predecessor.
  auto predecessors = context->cfg()->preds(second_block->id());
  if (predecessors.size() != 1) {
    return false;
  }
  auto first_block = context->cfg()->block(predecessors.at(0));

  return opt::blockmergeutil::CanMergeWithSuccessor(context, first_block);
}

void TransformationMergeBlocks::Apply(
    opt::IRContext* context, spvtools::fuzz::FactManager* /*unused*/) const {
  auto second_block = fuzzerutil::MaybeFindBlock(context, message_.block_id());
  auto first_block =
      context->cfg()->block(context->cfg()->preds(second_block->id()).at(0));

  auto function = first_block->GetParent();
  // We need an iterator pointing to the predecessor, hence the loop.
  for (auto bi = function->begin(); bi != function->end(); ++bi) {
    if (bi->id() == first_block->id()) {
      assert(opt::blockmergeutil::CanMergeWithSuccessor(context, &*bi) &&
             "Because 'Apply' should only be invoked if 'IsApplicable' holds, "
             "it must be possible to merge |bi| with its successor.");
      opt::blockmergeutil::MergeWithSuccessor(context, function, bi);
      // Invalidate all analyses, since we have changed the module
      // significantly.
      context->InvalidateAnalysesExceptFor(opt::IRContext::kAnalysisNone);
      return;
    }
  }
  assert(false &&
         "Control should not reach here - we should always find the desired "
         "block");
}

protobufs::Transformation TransformationMergeBlocks::ToMessage() const {
  protobufs::Transformation result;
  *result.mutable_merge_blocks() = message_;
  return result;
}

}  // namespace fuzz
}  // namespace spvtools
