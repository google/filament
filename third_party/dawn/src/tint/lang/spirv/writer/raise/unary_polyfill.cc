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

#include "src/tint/lang/spirv/writer/raise/unary_polyfill.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/result.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::spirv::writer::raise {

namespace {

struct State {
    core::ir::Module& ir;
    UnaryPolyfillConfig config;
    core::ir::Builder b{ir};
    core::type::Manager& ty{ir.Types()};

    void Process() {
        Vector<core::ir::Unary*, 4> unary_worklist;
        Vector<core::ir::CoreBuiltinCall*, 4> builtin_worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* unary = inst->As<core::ir::Unary>()) {
                if (config.polyfill_f32_negation && unary->Op() == core::UnaryOp::kNegation &&
                    unary->Result()->Type()->DeepestElement()->Is<core::type::F32>()) {
                    unary_worklist.Push(unary);
                }
            } else if (auto* builtin = inst->As<core::ir::CoreBuiltinCall>()) {
                if (config.polyfill_f32_abs && builtin->Func() == core::BuiltinFn::kAbs &&
                    builtin->Result()->Type()->DeepestElement()->Is<core::type::F32>()) {
                    builtin_worklist.Push(builtin);
                }
            }
        }

        for (auto* unary : unary_worklist) {
            PolyfillF32Negation(unary);
        }
        for (auto* builtin : builtin_worklist) {
            PolyfillF32Abs(builtin);
        }
    }

    void PolyfillF32Negation(core::ir::Unary* unary) {
        auto* val = unary->Val();
        auto* type = val->Type();

        // AMD mesa front end optimizer bug for unary negation and abs.
        // Fixed in 25.3 - See crbug.com/448294721
        // Note we use bitcast as a hammer to avoid the optimizer seeing through other possible
        // workarounds.
        b.InsertBefore(unary, [&] {
            auto* uint_ty = ty.MatchWidth(ty.u32(), type);
            auto* u32_val = b.Bitcast(uint_ty, val);
            auto* mask = b.MatchWidth(0x80000000_u, uint_ty);
            auto* xor_res = b.Xor(u32_val, mask);
            b.BitcastWithResult(unary->DetachResult(), xor_res->Result());
        });
        unary->Destroy();
    }

    void PolyfillF32Abs(core::ir::CoreBuiltinCall* builtin) {
        auto* val = builtin->Args()[0];
        auto* type = val->Type();

        // AMD mesa front end optimizer bug for unary negation and abs.
        // Fixed in 25.3 - See crbug.com/448294721
        // Note we use bitcast as a hammer to avoid the optimizer seeing through other possible
        // workarounds.
        b.InsertBefore(builtin, [&] {
            auto* uint_ty = ty.MatchWidth(ty.u32(), type);
            auto* u32_val = b.Bitcast(uint_ty, val);
            auto* mask = b.MatchWidth(0x7FFFFFFF_u, uint_ty);
            auto* and_res = b.And(u32_val, mask);
            b.BitcastWithResult(builtin->DetachResult(), and_res->Result());
        });
        builtin->Destroy();
    }
};

}  // namespace

Result<SuccessType> UnaryPolyfill(core::ir::Module& module, const UnaryPolyfillConfig& config) {
    AssertValid(module, kPolyfillUnaryCapabilities, "before spirv.UnaryPolyfill");

    State{module, config}.Process();

    return Success;
}

}  // namespace tint::spirv::writer::raise
