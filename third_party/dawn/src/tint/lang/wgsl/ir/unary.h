// Copyright 2024 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_WGSL_IR_UNARY_H_
#define SRC_TINT_LANG_WGSL_IR_UNARY_H_

#include "src/tint/lang/core/ir/unary.h"

namespace tint::wgsl::ir {

/// A WGSL-dialect unary instruction in the IR.
class Unary final : public Castable<Unary, core::ir::Unary> {
  public:
    /// The offset in Operands() for the value
    static constexpr size_t kValueOperandOffset = 0;

    /// Constructor (no results, no operands)
    /// @param id the instruction id
    explicit Unary(Id id);

    /// Constructor
    /// @param id the instruction id
    /// @param result the result value
    /// @param op the unary operator
    /// @param val the input value for the instruction
    Unary(Id id, core::ir::InstructionResult* result, core::UnaryOp op, core::ir::Value* val);

    ~Unary() override;

    /// @copydoc core::ir::Instruction::Clone()
    Unary* Clone(core::ir::CloneContext& ctx) override;

    /// @returns the table data to validate this builtin
    const core::intrinsic::TableData& TableData() const override;
};

}  // namespace tint::wgsl::ir

#endif  // SRC_TINT_LANG_WGSL_IR_UNARY_H_
