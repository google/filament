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

#ifndef SRC_TINT_LANG_CORE_IR_TERMINATOR_H_
#define SRC_TINT_LANG_CORE_IR_TERMINATOR_H_

#include "src/tint/lang/core/ir/operand_instruction.h"
#include "src/tint/lang/core/ir/value.h"
#include "src/tint/utils/rtti/castable.h"

// Forward declarations
namespace tint::core::ir {
class Block;
}  // namespace tint::core::ir

namespace tint::core::ir {

/// The base class of all instructions that terminate a block.
class Terminator : public Castable<Terminator, OperandInstruction<1, 0>> {
  public:
    /// Constructor
    /// @param id the instruction id
    explicit Terminator(Id id);

    ~Terminator() override;

    /// @returns the offset of the arguments in Operands()
    virtual size_t ArgsOperandOffset() const { return 0; }

    /// @returns the call arguments
    tint::Slice<Value* const> Args() { return operands_.Slice().Offset(ArgsOperandOffset()); }

    /// @returns the call arguments
    tint::Slice<const Value* const> Args() const {
        return operands_.Slice().Offset(ArgsOperandOffset());
    }
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_TERMINATOR_H_
