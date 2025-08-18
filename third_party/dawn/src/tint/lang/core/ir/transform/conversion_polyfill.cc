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

#include "src/tint/lang/core/ir/transform/conversion_polyfill.h"

#include <cstdint>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The polyfill config.
    const ConversionPolyfillConfig& config;

    /// The IR module.
    Module& ir;

    /// The IR builder.
    Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The symbol table.
    SymbolTable& sym{ir.symbols};

    /// Map from integer type to its f32toi helper function.
    Hashmap<const type::Type*, Function*, 4> f32toi_helpers{};

    /// Map from integer type to its f16toi helper function.
    Hashmap<const type::Type*, Function*, 4> f16toi_helpers{};

    /// Process the module.
    void Process() {
        // Find the conversion instructions that need to be polyfilled.
        Vector<ir::Convert*, 64> ftoi_worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* convert = inst->As<ir::Convert>()) {
                auto* src_ty = convert->Args()[0]->Type();
                auto* res_ty = convert->Result()->Type();
                if (config.ftoi &&                      //
                    src_ty->IsFloatScalarOrVector() &&  //
                    res_ty->IsIntegerScalarOrVector()) {
                    ftoi_worklist.Push(convert);
                }
            }
        }

        // Polyfill the conversion instructions that we found.
        for (auto* convert : ftoi_worklist) {
            ftoi(convert);
            convert->Destroy();
        }
    }

    /// Replace a conversion instruction with a call to helper function that manually clamps the
    /// result to within the limit of the destination type.
    /// @param convert the conversion instruction
    void ftoi(ir::Convert* convert) {
        auto* res_ty = convert->Result()->Type();
        auto* src_ty = convert->Args()[0]->Type();
        auto* src_el_ty = src_ty->DeepestElement();

        auto& helpers = src_el_ty->Is<type::F32>() ? f32toi_helpers : f16toi_helpers;
        auto* helper = helpers.GetOrAdd(res_ty, [&] {
            // Generate a name for the helper function.
            StringStream name;
            name << "tint_";
            if (auto* src_vec = src_ty->As<type::Vector>()) {
                name << "v" << src_vec->Width() << src_vec->Type()->FriendlyName();
            } else {
                name << src_ty->FriendlyName();
            }
            name << "_to_";
            if (auto* res_vec = res_ty->As<type::Vector>()) {
                name << "v" << res_vec->Width() << res_vec->Type()->FriendlyName();
            } else {
                name << res_ty->FriendlyName();
            }

            // Generate constants for the limits.
            struct {
                ir::Constant* low_limit_f = nullptr;
                ir::Constant* high_limit_f = nullptr;
            } limits;


            // Largest integers representable in the source floating point format.
            if (src_el_ty->Is<type::F32>()) {
                // These values are chosen specifically to enable f32 clamping.
                // See https://github.com/gpuweb/gpuweb/issues/5043
                if (res_ty->IsSignedIntegerScalarOrVector()) {
                    limits.low_limit_f = b.MatchWidth(f32(INT32_MIN), res_ty);
                    limits.high_limit_f =
                        b.MatchWidth(f32(tint::core::kMaxI32WhichIsAlsoF32), res_ty);
                } else {
                    limits.low_limit_f = b.MatchWidth(f32(0), res_ty);
                    limits.high_limit_f =
                        b.MatchWidth(f32(tint::core::kMaxU32WhichIsAlsoF32), res_ty);
                }
            } else if (src_el_ty->Is<type::F16>()) {
                constexpr float MAX_F16 = 65504;
                if (res_ty->IsSignedIntegerScalarOrVector()) {
                    limits.low_limit_f = b.MatchWidth(f16(-MAX_F16), res_ty);
                    limits.high_limit_f = b.MatchWidth(f16(MAX_F16), res_ty);
                } else {
                    limits.low_limit_f = b.MatchWidth(f16(0), res_ty);
                    limits.high_limit_f = b.MatchWidth(f16(MAX_F16), res_ty);
                }
            } else {
                TINT_UNIMPLEMENTED() << "unhandled floating-point type";
            }

            // Create the helper function.
            auto* func = b.Function(name.str(), res_ty);
            auto* value = b.FunctionParam("value", src_ty);
            func->SetParams({value});
            b.Append(func->Block(), [&] {
                auto* clamped = b.Call(src_ty, core::BuiltinFn::kClamp, value, limits.low_limit_f,
                                       limits.high_limit_f);
                auto* converted = b.Convert(res_ty, clamped);

                b.Return(func, converted->Result());
            });
            return func;
        });

        // Call the helper function, splatting the arguments to match the target vector width.
        auto* call = b.CallWithResult(convert->DetachResult(), helper, convert->Args()[0]);
        call->InsertBefore(convert);
    }
};

}  // namespace

Result<SuccessType> ConversionPolyfill(Module& ir, const ConversionPolyfillConfig& config) {
    auto result =
        ValidateAndDumpIfNeeded(ir, "core.ConversionPolyfill", kConversionPolyfillCapabilities);
    if (result != Success) {
        return result;
    }

    State{config, ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
