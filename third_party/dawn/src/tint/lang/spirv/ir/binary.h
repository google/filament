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

#ifndef SRC_TINT_LANG_SPIRV_IR_BINARY_H_
#define SRC_TINT_LANG_SPIRV_IR_BINARY_H_

#include "src/tint/lang/core/intrinsic/table_data.h"
#include "src/tint/lang/core/ir/binary.h"
#include "src/tint/lang/spirv/intrinsic/dialect.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::spirv::ir {

/// A SPIRV-dialect binary instruction in the IR.
class Binary final : public Castable<Binary, core::ir::Binary> {
  public:
    /// Constructor
    /// @param id the instruction id
    /// @param result the result value
    /// @param op the binary operator
    /// @param lhs the lhs of the instruction
    /// @param rhs the rhs of the instruction
    Binary(Id id,
           core::ir::InstructionResult* result,
           core::BinaryOp op,
           core::ir::Value* lhs,
           core::ir::Value* rhs);

    ~Binary() override;

    /// @copydoc core::ir::Instruction::Clone()
    Binary* Clone(core::ir::CloneContext& ctx) override;

    /// @returns the table data to validate this builtin
    const core::intrinsic::TableData& TableData() const override {
        return spirv::intrinsic::Dialect::kData;
    }
};

}  // namespace tint::spirv::ir

#endif  // SRC_TINT_LANG_SPIRV_IR_BINARY_H_
