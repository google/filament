// Copyright 2022 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_LANG_CORE_IR_MULTI_IN_BLOCK_H_
#define SRC_TINT_LANG_CORE_IR_MULTI_IN_BLOCK_H_

#include <utility>

#include "src/tint/lang/core/ir/block.h"
#include "src/tint/lang/core/ir/block_param.h"

namespace tint::core::ir {

/// A block that can be the target of multiple branches.
/// MultiInBlocks maintain a list of inbound branches and a number of BlockParam parameters, used to
/// pass values from the branch source to this target.
class MultiInBlock : public Castable<MultiInBlock, Block> {
  public:
    /// Constructor
    MultiInBlock();
    ~MultiInBlock() override;

    /// @copydoc Block::Clone()
    MultiInBlock* Clone(CloneContext& ctx) override;

    /// @copydoc Block::CloneInto()
    void CloneInto(CloneContext& ctx, Block* out) override;

    /// Sets the params to the block
    /// @param params the params for the block
    void SetParams(VectorRef<BlockParam*> params);

    /// Sets the params to the block
    /// @param params the params for the block
    void SetParams(std::initializer_list<BlockParam*> params);

    /// @returns the params to the block
    VectorRef<BlockParam*> Params() { return params_; }

    /// @returns the params to the block
    VectorRef<const BlockParam*> Params() const { return params_; }

    /// @returns branches made to this block by sibling blocks
    const VectorRef<ir::Terminator*> InboundSiblingBranches() { return inbound_sibling_branches_; }

    /// Adds the given branch to the list of branches made to this block by sibling blocks
    /// @param branch the branch to add
    void AddInboundSiblingBranch(ir::Terminator* branch);

    /// Removes the given branch to the list of branches made to this block by sibling blocks
    /// @param branch the branch to remove
    void RemoveInboundSiblingBranch(ir::Terminator* branch);

  private:
    Vector<BlockParam*, 2> params_;
    Vector<ir::Terminator*, 2> inbound_sibling_branches_;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_MULTI_IN_BLOCK_H_
