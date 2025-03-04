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

#ifndef SRC_TINT_LANG_CORE_IR_BREAK_IF_H_
#define SRC_TINT_LANG_CORE_IR_BREAK_IF_H_

#include <string>

#include "src/tint/lang/core/ir/exit.h"
#include "src/tint/lang/core/ir/value.h"
#include "src/tint/utils/containers/const_propagating_ptr.h"
#include "src/tint/utils/rtti/castable.h"

// Forward declarations
namespace tint::core::ir {
class Loop;
}  // namespace tint::core::ir

namespace tint::core::ir {

/// A break-if terminator instruction.
class BreakIf final : public Castable<BreakIf, Exit> {
  public:
    /// The offset in Operands() for the condition
    static constexpr size_t kConditionOperandOffset = 0;

    /// The base offset in Operands() for the arguments
    static constexpr size_t kArgsOperandOffset = 1;

    /// Constructor (no operands, no loop)
    /// @param id the instruction id
    explicit BreakIf(Id id);

    /// Constructor
    /// @param id the instruction id
    /// @param condition the break condition
    /// @param loop the loop containing the break-if
    /// @param next_iter_values the arguments passed to the loop body MultiInBlock, if the break
    /// condition evaluates to `false`.
    /// @param exit_values the values returned by the loop, if the break condition evaluates to
    /// `true`.
    BreakIf(Id id,
            Value* condition,
            ir::Loop* loop,
            VectorRef<Value*> next_iter_values = tint::Empty,
            VectorRef<Value*> exit_values = tint::Empty);

    ~BreakIf() override;

    /// @copydoc Instruction::Clone()
    BreakIf* Clone(CloneContext& ctx) override;

    /// @returns the offset of the arguments in Operands()
    size_t ArgsOperandOffset() const override { return kArgsOperandOffset; }

    /// @returns the break condition
    Value* Condition() { return Operand(kConditionOperandOffset); }

    /// @returns the break condition
    const Value* Condition() const { return Operand(kConditionOperandOffset); }

    /// @returns the loop containing the break-if
    ir::Loop* Loop() { return loop_; }

    /// @returns the loop containing the break-if
    const ir::Loop* Loop() const { return loop_; }

    /// @param loop the new loop containing the continue
    void SetLoop(ir::Loop* loop);

    /// @returns the friendly name for the instruction
    std::string FriendlyName() const override { return "break_if"; }

    /// @returns the arguments passed to the loop body MultiInBlock, if the break condition
    /// evaluates to `false`.
    Slice<Value* const> NextIterValues() {
        return operands_.Slice().Offset(kArgsOperandOffset).Truncate(num_next_iter_values_);
    }

    /// @returns the arguments passed to the loop body MultiInBlock, if the break condition
    /// evaluates to `false`.
    Slice<const Value* const> NextIterValues() const {
        return operands_.Slice().Offset(kArgsOperandOffset).Truncate(num_next_iter_values_);
    }

    /// @returns the values returned by the loop, if the break condition evaluates to `true`.
    Slice<Value* const> ExitValues() {
        return operands_.Slice().Offset(kArgsOperandOffset + num_next_iter_values_);
    }

    /// @returns the values returned by the loop, if the break condition evaluates to `true`.
    Slice<const Value* const> ExitValues() const {
        return operands_.Slice().Offset(kArgsOperandOffset + num_next_iter_values_);
    }

    /// Sets the number of operands used as the next iterator values.
    /// The first @p num operands after kArgsOperandOffset are used as next iterator values,
    /// subsequent operators are used as exit values.
    void SetNumNextIterValues(size_t num) {
        TINT_ASSERT(operands_.Length() >= num + kArgsOperandOffset);
        num_next_iter_values_ = num;
    }

  private:
    ConstPropagatingPtr<ir::Loop> loop_;
    size_t num_next_iter_values_ = 0;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_BREAK_IF_H_
