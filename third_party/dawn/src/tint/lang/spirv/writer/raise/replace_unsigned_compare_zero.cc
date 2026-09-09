// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/writer/raise/replace_unsigned_compare_zero.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/core_binary.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::spirv::writer::raise {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// Process the module.
    void Process() {
        for (auto* inst : ir.Instructions()) {
            auto* binary = inst->As<core::ir::CoreBinary>();
            if (!binary || binary->Op() != core::BinaryOp::kEqual) {
                continue;
            }

            if (!binary->LHS()->Type()->DeepestElement()->Is<core::type::U32>()) {
                continue;
            }

            if (IsConstantZero(binary->RHS())) {
                binary->SetOp(core::BinaryOp::kLessThan);
                binary->SetOperand(core::ir::Binary::kRhsOperandOffset,
                                   b.MatchWidth(1_u, binary->RHS()->Type()));
            } else if (IsConstantZero(binary->LHS())) {
                binary->SetOp(core::BinaryOp::kGreaterThan);
                binary->SetOperand(core::ir::Binary::kLhsOperandOffset,
                                   b.MatchWidth(1_u, binary->LHS()->Type()));
            }
        }
    }

    /// @returns true if @p val is a constant zero value
    bool IsConstantZero(const core::ir::Value* val) {
        if (auto* c = val->As<core::ir::Constant>()) {
            return c->Value()->AllZero();
        }
        return false;
    }
};

}  // namespace

Result<SuccessType> ReplaceUnsignedCompareZero(core::ir::Module& ir) {
    AssertValid(ir, "before spirv.ReplaceUnsignedCompareZero");

    State{ir}.Process();

    return Success;
}

}  // namespace tint::spirv::writer::raise
