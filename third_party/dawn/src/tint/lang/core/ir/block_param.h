// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_IR_BLOCK_PARAM_H_
#define SRC_TINT_LANG_CORE_IR_BLOCK_PARAM_H_

#include "src/tint/lang/core/ir/value.h"
#include "src/tint/utils/rtti/castable.h"

// Forward declarations
namespace tint::core::ir {
class MultiInBlock;
}  // namespace tint::core::ir

namespace tint::core::ir {

/// A block parameter in the IR.
class BlockParam : public Castable<BlockParam, Value> {
  public:
    /// Constructor
    /// @param type the type of the parameter
    explicit BlockParam(const core::type::Type* type);
    ~BlockParam() override;

    /// @returns the type of the parameter
    const core::type::Type* Type() const override { return type_; }

    /// Sets the block that this parameter belongs to.
    /// @param block the block
    void SetBlock(MultiInBlock* block) { block_ = block; }

    /// @returns the block that this parameter belongs to, or nullptr
    MultiInBlock* Block() { return block_; }

    /// @returns the block that this parameter belongs to, or nullptr
    const MultiInBlock* Block() const { return block_; }

    /// @copydoc Instruction::Clone()
    BlockParam* Clone(CloneContext& ctx) override;

  private:
    /// the type of the parameter
    const core::type::Type* type_ = nullptr;
    /// the block that the parameter belongs to
    MultiInBlock* block_ = nullptr;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_BLOCK_PARAM_H_
