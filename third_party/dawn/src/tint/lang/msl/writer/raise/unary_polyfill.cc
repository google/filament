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

#include "src/tint/lang/msl/writer/raise/unary_polyfill.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::msl::writer::raise {
namespace {

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
        // Find the unary operators that need replacing.
        Vector<core::ir::Unary*, 4> signed_int_negate_worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* unary = inst->As<core::ir::Unary>()) {
                auto op = unary->Op();
                auto* type = unary->Val()->Type();
                if (op == core::UnaryOp::kNegation && type->IsSignedIntegerScalarOrVector()) {
                    signed_int_negate_worklist.Push(unary);
                }
            }
        }

        // Replace the instructions that we found.
        for (auto* signed_int_negate : signed_int_negate_worklist) {
            SignedIntegerNegation(signed_int_negate);
        }
    }

    /// Replace a signed integer negation to avoid undefined behavior.
    /// @param unary the unary instruction
    void SignedIntegerNegation(core::ir::Unary* unary) {
        // Replace `-x` with `as_type<int>((~as_type<uint>(x)) + 1)`.
        auto* signed_type = unary->Result()->Type();
        auto* unsigned_type = ty.MatchWidth(ty.u32(), signed_type);
        b.InsertBefore(unary, [&] {
            auto* unsigned_value = b.Bitcast(unsigned_type, unary->Val());
            auto* complement = b.Complement(unsigned_type, unsigned_value);
            auto* plus_one = b.Add(unsigned_type, complement, b.MatchWidth(u32(1), unsigned_type));
            auto* result = b.Bitcast(signed_type, plus_one);
            unary->Result()->ReplaceAllUsesWith(result->Result());
        });
        unary->Destroy();
    }
};

}  // namespace

Result<SuccessType> UnaryPolyfill(core::ir::Module& ir) {
    auto result =
        ValidateAndDumpIfNeeded(ir, "msl.UnaryPolyfill",
                                core::ir::Capabilities{
                                    core::ir::Capability::kAllowPointersAndHandlesInStructures,
                                    core::ir::Capability::kAllowPrivateVarsInFunctions,
                                    core::ir::Capability::kAllowAnyLetType,
                                });
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::msl::writer::raise
