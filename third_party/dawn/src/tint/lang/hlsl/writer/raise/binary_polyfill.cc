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

#include "src/tint/lang/hlsl/writer/raise/binary_polyfill.h"

#include "src/tint/lang/core/fluent_types.h"  // IWYU pragma: export
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/hlsl/ir/builtin_call.h"

namespace tint::hlsl::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Process the module.
    void Process() {
        // Find the bitcasts that need replacing.
        Vector<core::ir::Binary*, 4> binary_worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* binary = inst->As<core::ir::Binary>()) {
                switch (binary->Op()) {
                    case core::BinaryOp::kModulo: {
                        if (binary->LHS()->Type()->IsFloatScalarOrVector()) {
                            binary_worklist.Push(binary);
                        }
                        break;
                    }
                    case core::BinaryOp::kMultiply: {
                        auto* lhs_ty = binary->LHS()->Type();
                        auto* rhs_ty = binary->RHS()->Type();

                        if ((lhs_ty->Is<core::type::Vector>() &&
                             rhs_ty->Is<core::type::Matrix>()) ||
                            (lhs_ty->Is<core::type::Matrix>() &&
                             rhs_ty->Is<core::type::Vector>()) ||
                            (lhs_ty->Is<core::type::Matrix>() &&
                             rhs_ty->Is<core::type::Matrix>())) {
                            binary_worklist.Push(binary);
                        }
                        break;
                    }

                    default:
                        break;
                }
                continue;
            }
        }

        // Replace the binary calls
        for (auto* binary : binary_worklist) {
            switch (binary->Op()) {
                case core::BinaryOp::kModulo:
                    PreciseFloatMod(binary);
                    break;
                case core::BinaryOp::kMultiply:
                    Mul(binary);
                    break;
                default:
                    TINT_UNIMPLEMENTED();
            }
        }
    }

    // Multiplying by a matrix requires the use of `mul` in order to get the
    // type of multiply we desire.
    //
    // Matrices are transposed, so swap LHS and RHS.
    void Mul(core::ir::Binary* binary) {
        auto* call = b.CallWithResult<hlsl::ir::BuiltinCall>(
            binary->DetachResult(), hlsl::BuiltinFn::kMul, binary->RHS(), binary->LHS());
        call->InsertBefore(binary);
        binary->Destroy();
    }

    // Replace with:
    //
    //   (lhs - (trunc(lhs / rhs)) * rhs)
    void PreciseFloatMod(core::ir::Binary* binary) {
        auto* type = binary->Result()->Type();
        b.InsertBefore(binary, [&] {
            auto* div = b.Divide(type, binary->LHS(), binary->RHS());

            // Force to a `let` to get better generated HLSL
            auto* d = b.Let(type);
            d->SetValue(div->Result());

            auto* trunc = b.Call(type, core::BuiltinFn::kTrunc, d);
            auto* mul = b.Multiply(type, trunc, binary->RHS());
            auto* sub = b.Subtract(type, binary->LHS(), mul);

            binary->Result()->ReplaceAllUsesWith(sub->Result());
        });
        binary->Destroy();
    }
};

}  // namespace

Result<SuccessType> BinaryPolyfill(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "hlsl.BinaryPolyfill",
                                          core::ir::Capabilities{
                                              core::ir::Capability::kAllowClipDistancesOnF32,
                                          });
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::hlsl::writer::raise
