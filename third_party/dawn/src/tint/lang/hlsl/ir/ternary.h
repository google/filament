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

#ifndef SRC_TINT_LANG_HLSL_IR_TERNARY_H_
#define SRC_TINT_LANG_HLSL_IR_TERNARY_H_

#include <string>

#include "src/tint/lang/core/ir/call.h"
#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/instruction_result.h"
#include "src/tint/lang/core/ir/value.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::hlsl::ir {

/// A ternary instruction in the IR.
class Ternary final : public Castable<Ternary, core::ir::Call> {
  public:
    /// Constructor
    ///
    /// Note, the args are in the order of (`false`, `true`, `compare`) to match select.
    /// Note, the ternary evaluates all branches, not just the selected branch.
    Ternary(Id id, core::ir::InstructionResult* result, VectorRef<core::ir::Value*> args);

    ~Ternary() override;

    /// @copydoc Instruction::Clone()
    Ternary* Clone(core::ir::CloneContext& ctx) override;

    /// @returns the false value
    core::ir::Value* False() const { return operands_[ArgsOperandOffset() + 0]; }

    /// @returns the true value
    core::ir::Value* True() const { return operands_[ArgsOperandOffset() + 1]; }

    /// @returns the compare value
    core::ir::Value* Cmp() const { return operands_[ArgsOperandOffset() + 2]; }

    /// @returns the friendly name for the instruction
    std::string FriendlyName() const override { return "hlsl.ternary"; }
};

}  // namespace tint::hlsl::ir

#endif  // SRC_TINT_LANG_HLSL_IR_TERNARY_H_
