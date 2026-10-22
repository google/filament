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

#include "src/tint/lang/msl/writer/raise/fix_u32_div_mod.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::msl::writer::raise {
namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The transform config.
    const FixU32DivModConfig& config;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// Process the module.
    void Process() {
        for (auto* inst : ir.Instructions()) {
            auto* binary = inst->As<core::ir::CoreBinary>();
            if (binary == nullptr) {
                continue;
            }

            auto op = binary->Op();
            auto* lhs_type = binary->LHS()->Type();
            if ((op == core::BinaryOp::kModulo || op == core::BinaryOp::kDivide) &&
                lhs_type->DeepestElement()->Is<core::type::U32>()) {
                InjectNonConstantZero(binary);
            }
        }
    }

    /// Inject a non-constant zero into the LHS of a unsigned div/mod instruction.
    /// @param binary the unsigned integer divide or modulo binary instruction
    void InjectNonConstantZero(core::ir::CoreBinary* binary) {
        b.InsertBefore(binary, [&] {
            auto* zero = b.Load(b.Access<ptr<immediate, u32>>(config.immediate_var,
                                                              u32(config.non_constant_zero_index)));
            binary->SetOperand(0U, b.Add(binary->LHS(), zero)->Result());
        });
    }
};

}  // namespace

Result<SuccessType> FixU32DivMod(core::ir::Module& ir, const FixU32DivModConfig& config) {
    AssertValid(ir, "before msl.FixU32DivMod");

    State{ir, config}.Process();

    return Success;
}

}  // namespace tint::msl::writer::raise
