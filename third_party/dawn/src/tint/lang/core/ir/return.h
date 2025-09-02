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

#ifndef SRC_TINT_LANG_CORE_IR_RETURN_H_
#define SRC_TINT_LANG_CORE_IR_RETURN_H_

#include <string>

#include "src/tint/lang/core/ir/terminator.h"
#include "src/tint/utils/rtti/castable.h"

// Forward declarations
namespace tint::core::ir {
class Function;
}  // namespace tint::core::ir

namespace tint::core::ir {

/// A return instruction.
class Return final : public Castable<Return, Terminator> {
  public:
    /// The offset in Operands() for the function being returned
    static constexpr size_t kFunctionOperandOffset = 0;

    /// The offset in Operands() for the return argument
    static constexpr size_t kArgsOperandOffset = 1;

    /// The minimum number of operands accepted by return instructions
    static constexpr size_t kMinOperands = 1;

    /// The maximum number of operands accepted by return instructions
    static constexpr size_t kMaxOperands = 2;

    /// Constructor (no operands)
    /// @param id the instruction id
    explicit Return(Id id);

    /// Constructor (no return value)
    /// @param id the instruction id
    /// @param func the function being returned
    Return(Id id, Function* func);

    /// Constructor
    /// @param id the instruction id
    /// @param func the function being returned
    /// @param arg the return value
    Return(Id id, Function* func, ir::Value* arg);

    ~Return() override;

    /// @copydoc Instruction::Clone()
    Return* Clone(CloneContext& ctx) override;

    /// @returns the function being returned
    Function* Func();

    /// @returns the function being returned
    const Function* Func() const;

    /// @returns true if the return has a value set
    bool HasValue() const { return operands_.Length() > kArgsOperandOffset; }

    /// @returns the return value, or nullptr
    ir::Value* Value() const { return HasValue() ? operands_[kArgsOperandOffset] : nullptr; }

    /// Sets the return value
    /// @param val the new return value
    void SetValue(ir::Value* val) { SetOperand(kArgsOperandOffset, val); }

    /// @returns the offset of the arguments in Operands()
    size_t ArgsOperandOffset() const override { return kArgsOperandOffset; }

    /// @returns the friendly name for the instruction
    std::string FriendlyName() const override { return "return"; }
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_RETURN_H_
