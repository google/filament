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

#ifndef SRC_TINT_LANG_CORE_IR_IF_H_
#define SRC_TINT_LANG_CORE_IR_IF_H_

#include <string>

#include "src/tint/lang/core/ir/control_instruction.h"
#include "src/tint/utils/containers/const_propagating_ptr.h"

// Forward declarations
namespace tint::core::ir {
class MultiInBlock;
}  // namespace tint::core::ir

namespace tint::core::ir {

/// If instruction.
///
/// ```
///                   in
///                    ┃
///         ┏━━━━━━━━━━┻━━━━━━━━━━┓
///         ▼                     ▼
///    ┌────────────┐      ┌────────────┐
///    │  True      │      │  False     │
///    | (optional) |      | (optional) |
///    └────────────┘      └────────────┘
///  ExitIf ┃                     ┃ ExitIf
///         ┗━━━━━━━━━━┳━━━━━━━━━━┛
///                    ▼
///                   out
/// ```
class If : public Castable<If, ControlInstruction> {
  public:
    /// The index of the condition operand
    static constexpr size_t kConditionOperandOffset = 0;

    /// Constructor (no results, no operands, no blocks)
    /// @param id the instruction id
    explicit If(Id id);

    /// Constructor
    /// @param cond the if condition
    /// @param t the true block
    /// @param f the false block
    If(Id id, Value* cond, ir::Block* t, ir::Block* f);

    ~If() override;

    /// @copydoc Instruction::Clone()
    If* Clone(CloneContext& ctx) override;

    /// @copydoc ControlInstruction::ForeachBlock
    void ForeachBlock(const std::function<void(ir::Block*)>& cb) override;

    /// @copydoc ControlInstruction::ForeachBlock
    void ForeachBlock(const std::function<void(const ir::Block*)>& cb) const override;

    /// @returns the if condition
    Value* Condition() { return Operand(kConditionOperandOffset); }

    /// @returns the if condition
    const Value* Condition() const { return Operand(kConditionOperandOffset); }

    /// @returns the true block
    ir::Block* True() { return true_; }

    /// @returns the true block
    const ir::Block* True() const { return true_; }

    /// @param block the new true block
    void SetTrue(ir::Block* block);

    /// @returns the false block
    ir::Block* False() { return false_; }

    /// @returns the false block
    const ir::Block* False() const { return false_; }

    /// @param block the new false block
    void SetFalse(ir::Block* block);

    /// @returns the friendly name for the instruction
    std::string FriendlyName() const override { return "if"; }

  private:
    ConstPropagatingPtr<ir::Block> true_;
    ConstPropagatingPtr<ir::Block> false_;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_IF_H_
