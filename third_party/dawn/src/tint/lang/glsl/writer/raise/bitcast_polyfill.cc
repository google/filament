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

#include "src/tint/lang/glsl/writer/raise/bitcast_polyfill.h"

#include <tuple>

#include "src/tint/lang/core/fluent_types.h"  // IWYU pragma: export
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/glsl/builtin_fn.h"
#include "src/tint/lang/glsl/ir/builtin_call.h"

namespace tint::glsl::writer::raise {
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

    // Polyfill functions for bitcast expression, BitcastType indicates the source type and the
    // destination type.
    using BitcastType =
        tint::UnorderedKeyWrapper<std::tuple<const core::type::Type*, const core::type::Type*>>;
    Hashmap<BitcastType, core::ir::Function*, 4> bitcast_funcs_{};

    /// Process the module.
    void Process() {
        Vector<core::ir::Bitcast*, 4> bitcast_worklist{};
        for (auto* inst : ir.Instructions()) {
            if (auto* bitcast = inst->As<core::ir::Bitcast>()) {
                bitcast_worklist.Push(bitcast);
                continue;
            }
        }

        for (auto* bitcast : bitcast_worklist) {
            auto* src_type = bitcast->Val()->Type();
            auto* dst_type = bitcast->Result()->Type();
            auto* dst_deepest = dst_type->DeepestElement();

            if (src_type == dst_type) {
                ReplaceBitcastWithValue(bitcast);
            } else if (src_type->DeepestElement()->Is<core::type::F16>()) {
                ReplaceBitcastWithFromF16Polyfill(bitcast);
            } else if (dst_deepest->Is<core::type::F16>()) {
                ReplaceBitcastWithToF16Polyfill(bitcast);
            } else if (src_type->DeepestElement()->Is<core::type::F32>()) {
                ReplaceBitcastFromF32(bitcast);
            } else if (dst_type->DeepestElement()->Is<core::type::F32>()) {
                ReplaceBitcastToF32(bitcast);
            } else {
                ReplaceBitcast(bitcast);
            }
        }
    }

    void ReplaceBitcastWithValue(core::ir::Bitcast* bitcast) {
        bitcast->Result()->ReplaceAllUsesWith(bitcast->Val());
        bitcast->Destroy();
    }

    core::ir::Value* CreateBitcastFromF32(const core::type::Type* type,
                                          const core::type::Type* result_type,
                                          core::ir::Value* val) {
        BuiltinFn fn = BuiltinFn::kNone;
        tint::Switch(
            type,                                                               //
            [&](const core::type::I32*) { fn = BuiltinFn::kFloatBitsToInt; },   //
            [&](const core::type::U32*) { fn = BuiltinFn::kFloatBitsToUint; },  //
            TINT_ICE_ON_NO_MATCH);

        return b.Call<glsl::ir::BuiltinCall>(result_type, fn, val)->Result();
    }

    void ReplaceBitcastFromF32(core::ir::Bitcast* bitcast) {
        auto* dst_type = bitcast->Result()->Type();
        auto* dst_deepest = dst_type->DeepestElement();

        b.InsertBefore(bitcast, [&] {
            auto* bc = CreateBitcastFromF32(dst_deepest, bitcast->Result()->Type(), bitcast->Val());
            bitcast->Result()->ReplaceAllUsesWith(bc);
        });
        bitcast->Destroy();
    }

    core::ir::Value* CreateBitcastToF32(const core::type::Type* src_type,
                                        const core::type::Type* dst_type,
                                        core::ir::Value* val) {
        BuiltinFn fn = BuiltinFn::kNone;
        tint::Switch(
            src_type,                                                           //
            [&](const core::type::I32*) { fn = BuiltinFn::kIntBitsToFloat; },   //
            [&](const core::type::U32*) { fn = BuiltinFn::kUintBitsToFloat; },  //
            TINT_ICE_ON_NO_MATCH);

        return b.Call<glsl::ir::BuiltinCall>(dst_type, fn, val)->Result();
    }

    void ReplaceBitcastToF32(core::ir::Bitcast* bitcast) {
        auto* src_type = bitcast->Val()->Type();
        auto* src_deepest = src_type->DeepestElement();

        b.InsertBefore(bitcast, [&] {
            auto* bc = CreateBitcastToF32(src_deepest, bitcast->Result()->Type(), bitcast->Val());
            bitcast->Result()->ReplaceAllUsesWith(bc);
        });
        bitcast->Destroy();
    }

    void ReplaceBitcast(core::ir::Bitcast* bitcast) {
        b.InsertBefore(bitcast,
                       [&] { b.ConvertWithResult(bitcast->DetachResult(), bitcast->Val()); });
        bitcast->Destroy();
    }

    core::ir::Function* CreateBitcastFromF16(const core::type::Type* src_type,
                                             const core::type::Type* dst_type) {
        return bitcast_funcs_.GetOrAdd(
            BitcastType{{src_type, dst_type}}, [&]() -> core::ir::Function* {
                TINT_ASSERT(src_type->Is<core::type::Vector>());

                // Generate a helper function that performs the following (in GLSL):
                //
                // ivec2 tint_bitcast_from_f16(f16vec4 src) {
                //   uvec2 r = uvec2(packFloat2x16(src.xy), packFloat2x16(src.zw));
                //   return ivec2(r);
                // }

                auto fn_name = b.ir.symbols.New("tint_bitcast_from_f16").Name();

                auto* f = b.Function(fn_name, dst_type);
                auto* src = b.FunctionParam("src", src_type);
                f->SetParams({src});

                b.Append(f->Block(), [&] {
                    auto* src_vec = src_type->As<core::type::Vector>();

                    core::ir::Value* packed = nullptr;
                    if (src_vec->Width() == 2) {
                        packed = b.Call<glsl::ir::BuiltinCall>(ty.u32(),
                                                               glsl::BuiltinFn::kPackFloat2X16, src)
                                     ->Result();
                    } else if (src_vec->Width() == 4) {
                        auto* left =
                            b.Call<glsl::ir::BuiltinCall>(ty.u32(), glsl::BuiltinFn::kPackFloat2X16,
                                                          b.Swizzle(ty.vec2<f16>(), src, {0, 1}));
                        auto* right =
                            b.Call<glsl::ir::BuiltinCall>(ty.u32(), glsl::BuiltinFn::kPackFloat2X16,
                                                          b.Swizzle(ty.vec2<f16>(), src, {2, 3}));
                        packed = b.Construct(ty.vec2<u32>(), left, right)->Result();
                    } else {
                        TINT_UNREACHABLE();
                    }

                    if (dst_type->DeepestElement()->Is<core::type::F32>()) {
                        packed =
                            CreateBitcastToF32(packed->Type()->DeepestElement(), dst_type, packed);
                    } else {
                        packed = b.InsertConvertIfNeeded(dst_type, packed);
                    }

                    b.Return(f, packed);
                });
                return f;
            });
    }

    void ReplaceBitcastWithFromF16Polyfill(core::ir::Bitcast* bitcast) {
        auto* src_type = bitcast->Val()->Type();
        auto* dst_type = bitcast->Result()->Type();

        auto* f = CreateBitcastFromF16(src_type, dst_type);
        b.InsertBefore(bitcast,
                       [&] { b.CallWithResult(bitcast->DetachResult(), f, bitcast->Args()[0]); });
        bitcast->Destroy();
    }

    core::ir::Function* CreateBitcastToF16(const core::type::Type* src_type,
                                           const core::type::Type* dst_type) {
        return bitcast_funcs_.GetOrAdd(
            BitcastType{{src_type, dst_type}}, [&]() -> core::ir::Function* {
                TINT_ASSERT(dst_type->Is<core::type::Vector>());

                // Generate a helper function that performs the following (in GLSL):
                //
                // f16vec4 tint_bitcast_to_f16(ivec2 src) {
                //   uvec2 r = uvec2(src);
                //   f16vec2 v_xy = unpackFloat2x16(r.x);
                //   f16vec2 v_zw = unpackFloat2x16(r.y);
                //   return f16vec4(v_xy.x, v_xy.y, v_zw.x, v_zw.y);
                // }

                auto fn_name = b.ir.symbols.New("tint_bitcast_to_f16").Name();

                auto* f = b.Function(fn_name, dst_type);
                auto* src = b.FunctionParam("src", src_type);
                f->SetParams({src});
                b.Append(f->Block(), [&] {
                    core::ir::Value* conv = src;
                    if (conv->Type()->DeepestElement()->Is<core::type::F32>()) {
                        conv = CreateBitcastFromF32(ty.u32(), ty.MatchWidth(ty.u32(), conv->Type()),
                                                    conv);
                    } else {
                        auto* target_ty = ty.MatchWidth(ty.u32(), conv->Type());
                        conv = b.InsertConvertIfNeeded(target_ty, conv);
                    }

                    core::ir::Value* val = nullptr;
                    if (conv->Type()->Is<core::type::Vector>()) {
                        auto* left = b.Call<glsl::ir::BuiltinCall>(
                            ty.vec2<f16>(), glsl::BuiltinFn::kUnpackFloat2X16,
                            b.Swizzle(ty.u32(), conv, {0}));
                        auto* right = b.Call<glsl::ir::BuiltinCall>(
                            ty.vec2<f16>(), glsl::BuiltinFn::kUnpackFloat2X16,
                            b.Swizzle(ty.u32(), conv, {1}));

                        val = b.Construct(dst_type, left, right)->Result();
                    } else {
                        val = b.Call<glsl::ir::BuiltinCall>(ty.vec2<f16>(),
                                                            glsl::BuiltinFn::kUnpackFloat2X16, conv)
                                  ->Result();
                    }
                    b.Return(f, val);
                });
                return f;
            });
    }

    void ReplaceBitcastWithToF16Polyfill(core::ir::Bitcast* bitcast) {
        auto* src_type = bitcast->Val()->Type();
        auto* dst_type = bitcast->Result()->Type();

        auto* f = CreateBitcastToF16(src_type, dst_type);
        b.InsertBefore(bitcast,
                       [&] { b.CallWithResult(bitcast->DetachResult(), f, bitcast->Args()[0]); });
        bitcast->Destroy();
    }
};

}  // namespace

Result<SuccessType> BitcastPolyfill(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(
        ir, "glsl.BitcastPolyfill",
        core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings,
                               core::ir::Capability::kAllowDuplicateBindings});
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::glsl::writer::raise
