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

#ifndef SRC_TINT_LANG_CORE_IR_SWIZZLE_H_
#define SRC_TINT_LANG_CORE_IR_SWIZZLE_H_

#include <string>
#include <utility>

#include "src/tint/lang/core/ir/operand_instruction.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::core::ir {

/// A swizzle instruction in the IR.
class Swizzle final : public Castable<Swizzle, OperandInstruction<1, 1>> {
  public:
    /// The offset in Operands() for the object being swizzled
    static constexpr size_t kObjectOperandOffset = 0;

    /// The fixed number of results returned by swizzle instructions
    static constexpr size_t kNumResults = 1;

    /// The fixed number of operands expected for swizzle instructions
    /// @note indices for swizzle are handled separately from the operands, so not included here
    static constexpr size_t kNumOperands = 1;

    /// Minimum number of indices expected for swizzle instructions
    static constexpr size_t kMinNumIndices = 1;

    /// Maximum number of indices expected for swizzle instructions
    static constexpr size_t kMaxNumIndices = 4;

    /// Maximum value of any indices for swizzle instructions
    static constexpr uint32_t kMaxIndexValue = 3;

    /// Constructor (no results, no operands)
    /// @param id the instruction id
    explicit Swizzle(Id id);

    /// Constructor
    /// @param id the instruction id
    /// @param result the result value
    /// @param object the object being swizzled
    /// @param indices the indices to swizzle
    Swizzle(Id id, InstructionResult* result, Value* object, VectorRef<uint32_t> indices);

    ~Swizzle() override;

    /// @copydoc Instruction::Clone()
    Swizzle* Clone(CloneContext& ctx) override;

    /// @returns the object used for the access
    Value* Object() { return Operand(kObjectOperandOffset); }

    /// @returns the object used for the access
    const Value* Object() const { return Operand(kObjectOperandOffset); }

    /// @returns the swizzle indices
    VectorRef<uint32_t> Indices() const { return indices_; }

    /// @param indices the new swizzle indices
    void SetIndices(VectorRef<uint32_t> indices) { indices_ = std::move(indices); }

    /// @returns the friendly name for the instruction
    std::string FriendlyName() const override { return "swizzle"; }

  private:
    Vector<uint32_t, 4> indices_;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_SWIZZLE_H_
