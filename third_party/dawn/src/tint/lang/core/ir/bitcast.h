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

#ifndef SRC_TINT_LANG_CORE_IR_BITCAST_H_
#define SRC_TINT_LANG_CORE_IR_BITCAST_H_

#include <string>

#include "src/tint/lang/core/ir/call.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::core::ir {

/// A bitcast instruction in the IR.
class Bitcast final : public Castable<Bitcast, Call> {
  public:
    /// The offset in Operands() for the value
    static constexpr size_t kValueOperandOffset = 0;

    /// The fixed number of results returned by this instruction
    static constexpr size_t kNumResults = 1;

    /// The fixed number of operands expected for this instruction
    static constexpr size_t kNumOperands = 1;

    /// Constructor (no results, no operands)
    /// @param id the instruction id
    explicit Bitcast(Id id);

    /// Constructor
    /// @param id the instruction id
    /// @param result the result value
    /// @param val the value being bitcast
    Bitcast(Id id, InstructionResult* result, Value* val);

    ~Bitcast() override;

    /// @copydoc Instruction::Clone()
    Bitcast* Clone(CloneContext& ctx) override;

    /// @returns the operand value
    Value* Val() { return Operand(kValueOperandOffset); }

    /// @returns the operand value
    const Value* Val() const { return Operand(kValueOperandOffset); }

    /// @returns the friendly name for the instruction
    std::string FriendlyName() const override { return "bitcast"; }

    /// @returns an empty access as the bitcast neither loads nor stores.
    Accesses GetSideEffects() const override { return Accesses{}; }
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_BITCAST_H_
