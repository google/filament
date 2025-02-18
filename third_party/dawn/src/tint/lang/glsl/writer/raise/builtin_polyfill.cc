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

#include "src/tint/lang/glsl/writer/raise/builtin_polyfill.h"

#include <string>
#include <tuple>
#include <utility>

#include "src/tint/lang/core/fluent_types.h"  // IWYU pragma: export
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/glsl/builtin_fn.h"
#include "src/tint/lang/glsl/ir/builtin_call.h"
#include "src/tint/lang/glsl/ir/member_builtin_call.h"

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

    /// Dot polyfills for non `f32`.
    Hashmap<const core::type::Type*, core::ir::Function*, 4> dot_funcs_{};
    /// Quantize polyfills
    Hashmap<const core::type::Type*, core::ir::Function*, 4> quantize_to_f16_funcs_{};

    /// Process the module.
    void Process() {
        Vector<core::ir::CoreBuiltinCall*, 4> call_worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* call = inst->As<core::ir::CoreBuiltinCall>()) {
                switch (call->Func()) {
                    case core::BuiltinFn::kAbs:
                    case core::BuiltinFn::kAll:
                    case core::BuiltinFn::kAny:
                    case core::BuiltinFn::kArrayLength:
                    case core::BuiltinFn::kAtomicCompareExchangeWeak:
                    case core::BuiltinFn::kAtomicSub:
                    case core::BuiltinFn::kAtomicLoad:
                    case core::BuiltinFn::kCountOneBits:
                    case core::BuiltinFn::kDot:
                    case core::BuiltinFn::kExtractBits:
                    case core::BuiltinFn::kFma:
                    case core::BuiltinFn::kFrexp:
                    case core::BuiltinFn::kInsertBits:
                    case core::BuiltinFn::kModf:
                    case core::BuiltinFn::kQuantizeToF16:
                    case core::BuiltinFn::kSelect:
                    case core::BuiltinFn::kStorageBarrier:
                    case core::BuiltinFn::kTextureBarrier:
                    case core::BuiltinFn::kWorkgroupBarrier:
                        call_worklist.Push(call);
                        break;
                    default:
                        break;
                }
                continue;
            }
        }

        // Replace the builtin calls that we found
        for (auto* call : call_worklist) {
            switch (call->Func()) {
                case core::BuiltinFn::kAbs:
                    Abs(call);
                    break;
                case core::BuiltinFn::kAll:
                    All(call);
                    break;
                case core::BuiltinFn::kAny:
                    Any(call);
                    break;
                case core::BuiltinFn::kArrayLength:
                    ArrayLength(call);
                    break;
                case core::BuiltinFn::kAtomicCompareExchangeWeak:
                    AtomicCompareExchangeWeak(call);
                    break;
                case core::BuiltinFn::kAtomicSub:
                    AtomicSub(call);
                    break;
                case core::BuiltinFn::kAtomicLoad:
                    AtomicLoad(call);
                    break;
                case core::BuiltinFn::kCountOneBits:
                    CountOneBits(call);
                    break;
                case core::BuiltinFn::kDot:
                    Dot(call);
                    break;
                case core::BuiltinFn::kExtractBits:
                    ExtractBits(call);
                    break;
                case core::BuiltinFn::kFma:
                    FMA(call);
                    break;
                case core::BuiltinFn::kFrexp:
                    Frexp(call);
                    break;
                case core::BuiltinFn::kInsertBits:
                    InsertBits(call);
                    break;
                case core::BuiltinFn::kModf:
                    Modf(call);
                    break;
                case core::BuiltinFn::kQuantizeToF16:
                    QuantizeToF16(call);
                    break;
                case core::BuiltinFn::kSelect:
                    Select(call);
                    break;
                case core::BuiltinFn::kStorageBarrier:
                case core::BuiltinFn::kTextureBarrier:
                case core::BuiltinFn::kWorkgroupBarrier:
                    Barrier(call);
                    break;
                default:
                    TINT_UNREACHABLE();
            }
        }
    }

    void Abs(core::ir::BuiltinCall* call) {
        auto args = call->Args();

        if (args[0]->Type()->DeepestElement()->IsUnsignedIntegerScalarOrVector()) {
            // GLSL does not support `abs` on unsigned arguments, replace it with the arg.
            call->Result(0)->ReplaceAllUsesWith(args[0]);
        } else {
            b.InsertBefore(call, [&] {
                b.CallWithResult<glsl::ir::BuiltinCall>(call->DetachResult(), glsl::BuiltinFn::kAbs,
                                                        args[0]);
            });
        }
        call->Destroy();
    }

    void Any(core::ir::BuiltinCall* call) {
        auto args = call->Args();

        if (args[0]->Type()->Is<core::type::Scalar>()) {
            // GLSL has no scalar `any`, replace it with the arg.
            call->Result(0)->ReplaceAllUsesWith(args[0]);
        } else {
            b.InsertBefore(call, [&] {
                b.CallWithResult<glsl::ir::BuiltinCall>(call->DetachResult(), glsl::BuiltinFn::kAny,
                                                        args[0]);
            });
        }
        call->Destroy();
    }

    void All(core::ir::BuiltinCall* call) {
        auto args = call->Args();

        if (args[0]->Type()->Is<core::type::Scalar>()) {
            // GLSL has no scalar `all`, replace it with the arg.
            call->Result(0)->ReplaceAllUsesWith(args[0]);
        } else {
            b.InsertBefore(call, [&] {
                b.CallWithResult<glsl::ir::BuiltinCall>(call->DetachResult(), glsl::BuiltinFn::kAll,
                                                        args[0]);
            });
        }
        call->Destroy();
    }

    void ArrayLength(core::ir::Call* call) {
        b.InsertBefore(call, [&] {
            auto* len = b.MemberCall<glsl::ir::MemberBuiltinCall>(ty.i32(), BuiltinFn::kLength,
                                                                  call->Args()[0]);
            b.ConvertWithResult(call->DetachResult(), len->Result(0));
        });
        call->Destroy();
    }

    core::ir::Function* CreateDotPolyfill(const core::type::Vector* type) {
        auto* ret_ty = type->DeepestElement();

        return dot_funcs_.GetOrAdd(type, [&]() -> core::ir::Function* {
            auto* f = b.Function("tint_int_dot", ret_ty);
            auto* x = b.FunctionParam("x", type);
            auto* y = b.FunctionParam("y", type);
            f->SetParams({x, y});

            b.Append(f->Block(), [&] {
                core::ir::Value* ret = nullptr;

                for (uint32_t i = 0; i < type->Width(); ++i) {
                    auto* lhs = b.Swizzle(ret_ty, x, {i});
                    auto* rhs = b.Swizzle(ret_ty, y, {i});
                    auto* v = b.Multiply(ret_ty, lhs, rhs);

                    if (ret != nullptr) {
                        ret = b.Add(ret_ty, ret, v)->Result(0);
                    } else {
                        ret = v->Result(0);
                    }
                }

                b.Return(f, ret);
            });
            return f;
        });
    }

    // GLSL does not have a builtin for `dot` with integer vector types. Generate the helper
    // function if it hasn't been created already
    void Dot(core::ir::BuiltinCall* call) {
        auto args = call->Args();

        auto* vec_ty = call->Args()[0]->Type()->As<core::type::Vector>();
        TINT_ASSERT(vec_ty);

        b.InsertBefore(call, [&] {
            if (!vec_ty->DeepestElement()->IsIntegerScalar()) {
                b.CallWithResult<glsl::ir::BuiltinCall>(call->DetachResult(), glsl::BuiltinFn::kDot,
                                                        args[0], args[1]);
            } else {
                auto* func = CreateDotPolyfill(vec_ty);
                b.CallWithResult(call->DetachResult(), func, args[0], args[1]);
            }
        });

        call->Destroy();
    }

    void Frexp(core::ir::BuiltinCall* call) {
        b.InsertBefore(call, [&] {
            // GLSL's frexp returns `fract` and outputs `whole` as an output parameter.
            // Polyfill it by declaring the result struct and then setting the values:
            //   __frexp_result result = {};
            //   result.fract = frexp(arg, result.exp);
            auto* result_type = call->Result(0)->Type();
            auto* float_type = result_type->Element(0);
            auto* i32_type = result_type->Element(1);
            auto* result = b.Var(ty.ptr(function, result_type));
            auto* exp = b.Access(ty.ptr(function, i32_type), result, u32(1));
            auto args = Vector<core::ir::Value*, 2>{call->Args()[0], exp->Result(0)};
            auto* res =
                b.Call<glsl::ir::BuiltinCall>(float_type, glsl::BuiltinFn::kFrexp, std::move(args));
            b.Store(b.Access(ty.ptr(function, float_type), result, u32(0)), res);
            b.LoadWithResult(call->DetachResult(), result);
        });
        call->Destroy();
    }

    void Modf(core::ir::BuiltinCall* call) {
        b.InsertBefore(call, [&] {
            // GLSL's modf returns `fract` and outputs `whole` as an output parameter.
            // Polyfill it by declaring the result struct and then setting the values:
            //   __modf_result result = {};
            //   result.fract = modf(arg, result.whole);
            auto* result_type = call->Result(0)->Type();
            auto* element_type = result_type->Element(0);
            auto* result = b.Var(ty.ptr(function, result_type));
            auto* whole = b.Access(ty.ptr(function, element_type), result, u32(1));
            auto args = Vector<core::ir::Value*, 2>{call->Args()[0], whole->Result(0)};
            auto* res = b.Call<glsl::ir::BuiltinCall>(element_type, glsl::BuiltinFn::kModf,
                                                      std::move(args));
            b.Store(b.Access(ty.ptr(function, element_type), result, u32(0)), res);
            b.LoadWithResult(call->DetachResult(), result);
        });
        call->Destroy();
    }

    void ExtractBits(core::ir::Call* call) {
        b.InsertBefore(call, [&] {
            auto args = call->Args();
            auto* offset = b.Convert(ty.i32(), args[1]);
            auto* bits = b.Convert(ty.i32(), args[2]);

            b.CallWithResult<glsl::ir::BuiltinCall>(
                call->DetachResult(), glsl::BuiltinFn::kBitfieldExtract, args[0], offset, bits);
        });
        call->Destroy();
    }

    void InsertBits(core::ir::Call* call) {
        b.InsertBefore(call, [&] {
            auto args = call->Args();
            auto* offset = b.Convert(ty.i32(), args[2]);
            auto* bits = b.Convert(ty.i32(), args[3]);

            b.CallWithResult<glsl::ir::BuiltinCall>(call->DetachResult(),
                                                    glsl::BuiltinFn::kBitfieldInsert, args[0],
                                                    args[1], offset, bits);
        });
        call->Destroy();
    }

    // There is no `fma` method in GLSL ES 3.10 so we emulate it. `fma` does exist in desktop after
    // 4.00 but we use the emulated version to be consistent. We could use the real one on desktop
    // if we decide too in the future.
    void FMA(core::ir::Call* call) {
        auto args = call->Args();

        b.InsertBefore(call, [&] {
            auto* res_ty = call->Result(0)->Type();
            auto* mul = b.Multiply(res_ty, args[0], args[1]);
            b.AddWithResult(call->DetachResult(), mul, args[2]);
        });
        call->Destroy();
    }

    // GLSL `bitCount` always returns an `i32` so may need to be converted. Convert to a `bitCount`
    void CountOneBits(core::ir::Call* call) {
        auto* call_ty = call->Result(0)->Type();
        auto* bitcount_ty = ty.MatchWidth(ty.i32(), call_ty);

        b.InsertBefore(call, [&] {
            core::ir::Value* c = b.Call<glsl::ir::BuiltinCall>(
                                      bitcount_ty, glsl::BuiltinFn::kBitCount, call->Args()[0])
                                     ->Result(0);
            c = b.InsertConvertIfNeeded(call_ty, c);
            call->Result(0)->ReplaceAllUsesWith(c);
        });
        call->Destroy();
    }

    void AtomicCompareExchangeWeak(core::ir::BuiltinCall* call) {
        auto args = call->Args();
        auto* type = args[1]->Type();

        auto* dest = args[0];
        auto* compare_value = args[1];
        auto* value = args[2];

        auto* result_type = call->Result(0)->Type();

        b.InsertBefore(call, [&] {
            auto* bitcast_cmp_value = b.Bitcast(type, compare_value);
            auto* bitcast_value = b.Bitcast(type, value);

            auto* swap = b.Call<glsl::ir::BuiltinCall>(
                type, glsl::BuiltinFn::kAtomicCompSwap,
                Vector<core::ir::Value*, 3>{dest, bitcast_cmp_value->Result(0),
                                            bitcast_value->Result(0)});

            auto* exchanged = b.Equal(ty.bool_(), swap, compare_value);

            auto* result = b.Construct(result_type, swap, exchanged)->Result(0);
            call->Result(0)->ReplaceAllUsesWith(result);
        });
        call->Destroy();
    }

    void AtomicSub(core::ir::BuiltinCall* call) {
        b.InsertBefore(call, [&] {
            auto args = call->Args();

            if (args[1]->Type()->Is<core::type::I32>()) {
                b.CallWithResult(call->DetachResult(), core::BuiltinFn::kAtomicAdd, args[0],
                                 b.Negation(args[1]->Type(), args[1]));
            } else {
                // Negating a u32 isn't possible in the IR, so pass a fake GLSL function and
                // handle in the printer.
                b.CallWithResult<glsl::ir::BuiltinCall>(
                    call->DetachResult(), glsl::BuiltinFn::kAtomicSub,
                    Vector<core::ir::Value*, 2>{args[0], args[1]});
            }
        });
        call->Destroy();
    }

    void AtomicLoad(core::ir::CoreBuiltinCall* call) {
        // GLSL does not have an atomicLoad, so we emulate it with atomicOr using 0 as the OR
        // value
        b.InsertBefore(call, [&] {
            auto args = call->Args();
            b.CallWithResult(
                call->DetachResult(), core::BuiltinFn::kAtomicOr, args[0],
                b.Zero(args[0]->Type()->UnwrapPtr()->As<core::type::Atomic>()->Type()));
        });
        call->Destroy();
    }

    void Barrier(core::ir::CoreBuiltinCall* call) {
        b.InsertBefore(call, [&] {
            switch (call->Func()) {
                case core::BuiltinFn::kStorageBarrier:
                    b.Call<glsl::ir::BuiltinCall>(ty.void_(),
                                                  glsl::BuiltinFn::kMemoryBarrierBuffer);
                    break;
                case core::BuiltinFn::kTextureBarrier:
                    b.Call<glsl::ir::BuiltinCall>(ty.void_(), glsl::BuiltinFn::kMemoryBarrierImage);
                    break;
                default:
                    break;
            }
            b.Call<glsl::ir::BuiltinCall>(ty.void_(), glsl::BuiltinFn::kBarrier);
        });

        call->Destroy();
    }

    void Select(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();

        // Implement as `mix` in GLSL. The one caveat is that `mix` requires the number of
        // parameters to match, so if we have a `vec2` for the results and a single `bool` value,
        // we need to splat the `bool`.
        auto bool_ty = args[2]->Type();
        auto val_ty = args[0]->Type();

        b.InsertBefore(call, [&] {
            core::ir::Value* cond = args[2];
            if (val_ty->Is<core::type::Vector>() && !bool_ty->Is<core::type::Vector>()) {
                cond = b.Construct(ty.MatchWidth(ty.bool_(), val_ty), cond)->Result(0);
            }

            b.CallWithResult<glsl::ir::BuiltinCall>(call->DetachResult(), glsl::BuiltinFn::kMix,
                                                    args[0], args[1], cond);
        });
        call->Destroy();
    }

    core::ir::Function* CreateQuantizeToF16Polyfill(const core::type::Type* type) {
        return quantize_to_f16_funcs_.GetOrAdd(type, [&]() -> core::ir::Function* {
            auto* f = b.Function("tint_quantize_to_f16", type);
            auto* val = b.FunctionParam("val", type);
            f->SetParams({val});

            b.Append(f->Block(), [&] {
                core::ir::Value* ret = nullptr;

                auto* inner_ty = type->DeepestElement();
                auto* v2 = ty.vec2(inner_ty);

                auto pack_unpack = [&](core::ir::Value* item) {
                    auto* r = b.Call(ty.u32(), core::BuiltinFn::kPack2X16Float, item)->Result(0);
                    return b.Call(v2, core::BuiltinFn::kUnpack2X16Float, r)->Result(0);
                };

                if (auto* vec = type->As<core::type::Vector>()) {
                    switch (vec->Width()) {
                        case 2: {
                            ret = pack_unpack(val);
                            break;
                        }
                        case 3: {
                            core::ir::Value* lhs = b.Swizzle(v2, val, {0, 1})->Result(0);
                            lhs = pack_unpack(lhs);

                            core::ir::Value* rhs = b.Swizzle(v2, val, {2, 2})->Result(0);
                            rhs = pack_unpack(rhs);
                            rhs = b.Swizzle(inner_ty, rhs, {0})->Result(0);

                            ret = b.Construct(type, lhs, rhs)->Result(0);
                            break;
                        }
                        default: {
                            core::ir::Value* lhs = b.Swizzle(v2, val, {0, 1})->Result(0);
                            lhs = pack_unpack(lhs);

                            core::ir::Value* rhs = b.Swizzle(v2, val, {2, 3})->Result(0);
                            rhs = pack_unpack(rhs);

                            ret = b.Construct(type, lhs, rhs)->Result(0);
                            break;
                        }
                    }
                } else {
                    ret = b.Construct(v2, val)->Result(0);
                    ret = pack_unpack(ret);
                    ret = b.Swizzle(type, ret, {0})->Result(0);
                }
                b.Return(f, ret);
            });
            return f;
        });
    }

    // Emulate by casting to f16 and back again.
    void QuantizeToF16(core::ir::BuiltinCall* call) {
        auto args = call->Args();

        b.InsertBefore(call, [&] {
            auto* func = CreateQuantizeToF16Polyfill(args[0]->Type());
            b.CallWithResult(call->DetachResult(), func, args[0]);
        });
        call->Destroy();
    }
};

}  // namespace

Result<SuccessType> BuiltinPolyfill(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "glsl.BuiltinPolyfill");
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::glsl::writer::raise
