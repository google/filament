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

#ifndef SRC_TINT_LANG_CORE_IR_ACCESS_H_
#define SRC_TINT_LANG_CORE_IR_ACCESS_H_

#include <string>

#include "src/tint/lang/core/ir/operand_instruction.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::core::ir {

/// An access instruction in the IR.
class Access final : public Castable<Access, OperandInstruction<3, 1>> {
  public:
    /// The offset in Operands() for the object being accessed
    static constexpr size_t kObjectOperandOffset = 0;

    /// The base offset in Operands() for the access indices
    static constexpr size_t kIndicesOperandOffset = 1;

    /// The fixed number of results returned by this instruction
    static constexpr size_t kNumResults = 1;

    /// The minimum number of operands used by this instruction
    static constexpr size_t kMinNumOperands = 2;

    /// Constructor (no results, no operands)
    /// @param id the instruction id
    explicit Access(Instruction::Id id);

    /// Constructor
    /// @param id the instruction id
    /// @param result the result value
    /// @param object the accessor object
    /// @param indices the indices to access
    Access(Instruction::Id id, InstructionResult* result, Value* object, VectorRef<Value*> indices);

    ~Access() override;

    /// @copydoc Instruction::Clone()
    Access* Clone(CloneContext& ctx) override;

    /// @returns the object used for the access
    Value* Object() { return Operand(kObjectOperandOffset); }

    /// @returns the object used for the access
    const Value* Object() const { return Operand(kObjectOperandOffset); }

    /// Adds the given index to the end of the access chain
    /// @param idx the index to add
    void AddIndex(Value* idx) { AddOperand(operands_.Length(), idx); }

    /// @returns the accessor indices
    tint::Slice<Value* const> Indices() { return operands_.Slice().Offset(kIndicesOperandOffset); }

    /// @returns the accessor indices
    tint::Slice<const Value* const> Indices() const {
        return operands_.Slice().Offset(kIndicesOperandOffset);
    }

    /// Removes the last index from the access indices
    /// @returns the last index value
    Value* PopLastIndex() { return PopOperand(); }

    /// @returns the friendly name for the instruction
    std::string FriendlyName() const override { return "access"; }
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_ACCESS_H_
