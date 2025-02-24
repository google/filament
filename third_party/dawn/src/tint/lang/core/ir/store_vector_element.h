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

#ifndef SRC_TINT_LANG_CORE_IR_STORE_VECTOR_ELEMENT_H_
#define SRC_TINT_LANG_CORE_IR_STORE_VECTOR_ELEMENT_H_

#include <string>

#include "src/tint/lang/core/ir/operand_instruction.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::core::ir {

/// A store instruction for a single vector element in the IR.
class StoreVectorElement final : public Castable<StoreVectorElement, OperandInstruction<3, 0>> {
  public:
    /// The offset in Operands() for the `to` value
    static constexpr size_t kToOperandOffset = 0;

    /// The offset in Operands() for the `index` value
    static constexpr size_t kIndexOperandOffset = 1;

    /// The offset in Operands() for the `value` value
    static constexpr size_t kValueOperandOffset = 2;

    /// The fixed number of results returned by this instruction
    static constexpr size_t kNumResults = 0;

    /// The fixed number of operands used by this instruction
    static constexpr size_t kNumOperands = 3;

    /// Constructor (no operands)
    /// @param id the instruction id
    explicit StoreVectorElement(Id id);

    /// Constructor
    /// @param id the instruction id
    /// @param to the vector pointer
    /// @param index the new vector element index
    /// @param value the new vector element value
    StoreVectorElement(Id id, ir::Value* to, ir::Value* index, ir::Value* value);

    ~StoreVectorElement() override;

    /// @copydoc Instruction::Clone()
    StoreVectorElement* Clone(CloneContext& ctx) override;

    /// @returns the vector pointer value
    ir::Value* To() { return Operand(kToOperandOffset); }

    /// @returns the vector pointer value
    const ir::Value* To() const { return Operand(kToOperandOffset); }

    /// @returns the new vector element index
    ir::Value* Index() { return Operand(kIndexOperandOffset); }

    /// @returns the new vector element index
    const ir::Value* Index() const { return Operand(kIndexOperandOffset); }

    /// @returns the new vector element value
    ir::Value* Value() { return Operand(kValueOperandOffset); }

    /// @returns the new vector element value
    const ir::Value* Value() const { return Operand(kValueOperandOffset); }

    /// @returns the friendly name for the instruction
    std::string FriendlyName() const override { return "store_vector_element"; }

    /// @returns the side effects for this instruction
    Accesses GetSideEffects() const override { return Accesses{Access::kStore}; }
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_STORE_VECTOR_ELEMENT_H_
