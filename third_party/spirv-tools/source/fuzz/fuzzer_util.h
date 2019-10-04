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

#ifndef SOURCE_FUZZ_FUZZER_UTIL_H_
#define SOURCE_FUZZ_FUZZER_UTIL_H_

#include <vector>

#include "source/fuzz/protobufs/spirvfuzz_protobufs.h"
#include "source/opt/basic_block.h"
#include "source/opt/instruction.h"
#include "source/opt/ir_context.h"

namespace spvtools {
namespace fuzz {

// Provides global utility methods for use by the fuzzer
namespace fuzzerutil {

// Returns true if and only if the module does not define the given id.
bool IsFreshId(opt::IRContext* context, uint32_t id);

// Updates the module's id bound if needed so that it is large enough to
// account for the given id.
void UpdateModuleIdBound(opt::IRContext* context, uint32_t id);

// Return the block with id |maybe_block_id| if it exists, and nullptr
// otherwise.
opt::BasicBlock* MaybeFindBlock(opt::IRContext* context,
                                uint32_t maybe_block_id);

// When adding an edge from |bb_from| to |bb_to| (which are assumed to be blocks
// in the same function), it is important to supply |bb_to| with ids that can be
// used to augment OpPhi instructions in the case that there is not already such
// an edge.  This function returns true if and only if the ids provided in
// |phi_ids| suffice for this purpose,
bool PhiIdsOkForNewEdge(
    opt::IRContext* context, opt::BasicBlock* bb_from, opt::BasicBlock* bb_to,
    const google::protobuf::RepeatedField<google::protobuf::uint32>& phi_ids);

// Requires that PhiIdsOkForNewEdge(context, bb_from, bb_to, phi_ids) holds,
// and that bb_from ends with "OpBranch %some_block".  Turns OpBranch into
// "OpBranchConditional |condition_value| ...", such that control will branch
// to %some_block, with |bb_to| being the unreachable alternative.  Updates
// OpPhi instructions in |bb_to| using |phi_ids| so that the new edge is valid.
void AddUnreachableEdgeAndUpdateOpPhis(
    opt::IRContext* context, opt::BasicBlock* bb_from, opt::BasicBlock* bb_to,
    bool condition_value,
    const google::protobuf::RepeatedField<google::protobuf::uint32>& phi_ids);

// Returns true if and only if |maybe_loop_header_id| is a loop header and
// |block_id| is in the continue construct of the associated loop.
bool BlockIsInLoopContinueConstruct(opt::IRContext* context, uint32_t block_id,
                                    uint32_t maybe_loop_header_id);

// Requires that |base_inst| is either the label instruction of |block| or an
// instruction inside |block|.
//
// If the block contains a (non-label, non-terminator) instruction |offset|
// instructions after |base_inst|, an iterator to this instruction is returned.
//
// Otherwise |block|->end() is returned.
opt::BasicBlock::iterator GetIteratorForBaseInstructionAndOffset(
    opt::BasicBlock* block, const opt::Instruction* base_inst, uint32_t offset);

// The function determines whether adding an edge from |bb_from| to |bb_to| -
// is legitimate with respect to the SPIR-V rule that a definition must
// dominate all of its uses.  This is because adding such an edge can change
// dominance in the control flow graph, potentially making the module invalid.
bool NewEdgeRespectsUseDefDominance(opt::IRContext* context,
                                    opt::BasicBlock* bb_from,
                                    opt::BasicBlock* bb_to);

// Returns true if and only if there is a path to |bb| from the entry block of
// the function that contains |bb|.
bool BlockIsReachableInItsFunction(opt::IRContext* context,
                                   opt::BasicBlock* bb);

}  // namespace fuzzerutil

}  // namespace fuzz
}  // namespace spvtools

#endif  // SOURCE_FUZZ_FUZZER_UTIL_H_
