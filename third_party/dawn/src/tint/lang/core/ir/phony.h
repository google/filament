// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_IR_PHONY_H_
#define SRC_TINT_LANG_CORE_IR_PHONY_H_

#include <string>

#include "src/tint/lang/core/ir/operand_instruction.h"

namespace tint::core::ir {

/// Phony assignment in the IR.
class Phony : public Castable<Phony, OperandInstruction<1, 0>> {
  public:
    /// The offset in Operands() for the value
    static constexpr size_t kValueOperandOffset = 0;

    /// The fixed number of operands accepted by phony instructions
    static constexpr size_t kNumOperands = 1;

    /// The fixed number of results returned by phony instructions
    static constexpr size_t kNumResults = 0;

    /// Constructor
    /// @param id the instruction id
    /// @param value the assigned value
    explicit Phony(Id id, Value* value);
    ~Phony() override;

    /// @copydoc Instruction::Clone()
    Phony* Clone(CloneContext& ctx) override;

    /// @returns the value
    const ir::Value* Value() const { return Operand(kValueOperandOffset); }

    /// @returns the friendly name for the instruction
    std::string FriendlyName() const override { return "phony"; }
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_PHONY_H_
