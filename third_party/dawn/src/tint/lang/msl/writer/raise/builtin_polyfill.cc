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

#include "src/tint/lang/msl/writer/raise/builtin_polyfill.h"

#include <atomic>
#include <cstdint>
#include <utility>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/scalar.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/msl/barrier_type.h"
#include "src/tint/lang/msl/builtin_fn.h"
#include "src/tint/lang/msl/ir/binary.h"
#include "src/tint/lang/msl/ir/builtin_call.h"
#include "src/tint/lang/msl/ir/component.h"
#include "src/tint/lang/msl/ir/member_builtin_call.h"
#include "src/tint/lang/msl/ir/memory_order.h"
#include "src/tint/lang/msl/type/bias.h"
#include "src/tint/lang/msl/type/gradient.h"
#include "src/tint/lang/msl/type/level.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/ice/ice.h"

namespace tint::msl::writer::raise {
namespace {

using namespace tint::core::fluent_types;  // NOLINT

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// A map from an atomic pointer type to an atomicCompareExchangeWeak polyfill.
    Hashmap<const core::type::Type*, core::ir::Function*, 2> atomic_compare_exchange_polyfills{};

    /// A map from an integer vector type to a dot polyfill.
    Hashmap<const core::type::Vector*, core::ir::Function*, 4> integer_dot_polyfills{};

    /// Process the module.
    void Process() {
        // Find the builtins that need replacing.
        Vector<core::ir::CoreBuiltinCall*, 4> builtin_worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* builtin = inst->As<core::ir::CoreBuiltinCall>()) {
                switch (builtin->Func()) {
                    case core::BuiltinFn::kAtomicAdd:
                    case core::BuiltinFn::kAtomicAnd:
                    case core::BuiltinFn::kAtomicCompareExchangeWeak:
                    case core::BuiltinFn::kAtomicExchange:
                    case core::BuiltinFn::kAtomicLoad:
                    case core::BuiltinFn::kAtomicMax:
                    case core::BuiltinFn::kAtomicMin:
                    case core::BuiltinFn::kAtomicOr:
                    case core::BuiltinFn::kAtomicStore:
                    case core::BuiltinFn::kAtomicSub:
                    case core::BuiltinFn::kAtomicXor:
                    case core::BuiltinFn::kDistance:
                    case core::BuiltinFn::kDot:
                    case core::BuiltinFn::kFrexp:
                    case core::BuiltinFn::kLength:
                    case core::BuiltinFn::kModf:
                    case core::BuiltinFn::kPack2X16Float:
                    case core::BuiltinFn::kQuadSwapDiagonal:
                    case core::BuiltinFn::kQuadSwapX:
                    case core::BuiltinFn::kQuadSwapY:
                    case core::BuiltinFn::kQuantizeToF16:
                    case core::BuiltinFn::kSign:
                    case core::BuiltinFn::kSubgroupMatrixLoad:
                    case core::BuiltinFn::kSubgroupMatrixStore:
                    case core::BuiltinFn::kSubgroupMatrixMultiply:
                    case core::BuiltinFn::kSubgroupMatrixMultiplyAccumulate:
                    case core::BuiltinFn::kTextureDimensions:
                    case core::BuiltinFn::kTextureGather:
                    case core::BuiltinFn::kTextureGatherCompare:
                    case core::BuiltinFn::kTextureLoad:
                    case core::BuiltinFn::kTextureNumLayers:
                    case core::BuiltinFn::kTextureNumLevels:
                    case core::BuiltinFn::kTextureNumSamples:
                    case core::BuiltinFn::kTextureSample:
                    case core::BuiltinFn::kTextureSampleBias:
                    case core::BuiltinFn::kTextureSampleCompare:
                    case core::BuiltinFn::kTextureSampleCompareLevel:
                    case core::BuiltinFn::kTextureSampleGrad:
                    case core::BuiltinFn::kTextureSampleLevel:
                    case core::BuiltinFn::kTextureStore:
                    case core::BuiltinFn::kStorageBarrier:
                    case core::BuiltinFn::kWorkgroupBarrier:
                    case core::BuiltinFn::kTextureBarrier:
                    case core::BuiltinFn::kUnpack2X16Float:
                        builtin_worklist.Push(builtin);
                        break;
                    default:
                        break;
                }
            }
        }

        // Replace the builtins that we found.
        for (auto* builtin : builtin_worklist) {
            switch (builtin->Func()) {
                // Atomics.
                case core::BuiltinFn::kAtomicAdd:
                    AtomicCall(builtin, msl::BuiltinFn::kAtomicFetchAddExplicit);
                    break;
                case core::BuiltinFn::kAtomicAnd:
                    AtomicCall(builtin, msl::BuiltinFn::kAtomicFetchAndExplicit);
                    break;
                case core::BuiltinFn::kAtomicCompareExchangeWeak:
                    AtomicCompareExchangeWeak(builtin);
                    break;
                case core::BuiltinFn::kAtomicExchange:
                    AtomicCall(builtin, msl::BuiltinFn::kAtomicExchangeExplicit);
                    break;
                case core::BuiltinFn::kAtomicLoad:
                    AtomicCall(builtin, msl::BuiltinFn::kAtomicLoadExplicit);
                    break;
                case core::BuiltinFn::kAtomicMax:
                    AtomicCall(builtin, msl::BuiltinFn::kAtomicFetchMaxExplicit);
                    break;
                case core::BuiltinFn::kAtomicMin:
                    AtomicCall(builtin, msl::BuiltinFn::kAtomicFetchMinExplicit);
                    break;
                case core::BuiltinFn::kAtomicOr:
                    AtomicCall(builtin, msl::BuiltinFn::kAtomicFetchOrExplicit);
                    break;
                case core::BuiltinFn::kAtomicStore:
                    AtomicCall(builtin, msl::BuiltinFn::kAtomicStoreExplicit);
                    break;
                case core::BuiltinFn::kAtomicSub:
                    AtomicCall(builtin, msl::BuiltinFn::kAtomicFetchSubExplicit);
                    break;
                case core::BuiltinFn::kAtomicXor:
                    AtomicCall(builtin, msl::BuiltinFn::kAtomicFetchXorExplicit);
                    break;

                // Arithmetic builtins.
                case core::BuiltinFn::kDistance:
                    Distance(builtin);
                    break;
                case core::BuiltinFn::kDot:
                    Dot(builtin);
                    break;
                case core::BuiltinFn::kFrexp:
                    Frexp(builtin);
                    break;
                case core::BuiltinFn::kLength:
                    Length(builtin);
                    break;
                case core::BuiltinFn::kModf:
                    Modf(builtin);
                    break;
                case core::BuiltinFn::kQuantizeToF16:
                    QuantizeToF16(builtin);
                    break;
                case core::BuiltinFn::kSign:
                    Sign(builtin);
                    break;

                // Texture builtins.
                case core::BuiltinFn::kTextureDimensions:
                    TextureDimensions(builtin);
                    break;
                case core::BuiltinFn::kTextureGather:
                    TextureGather(builtin);
                    break;
                case core::BuiltinFn::kTextureGatherCompare:
                    TextureGatherCompare(builtin);
                    break;
                case core::BuiltinFn::kTextureLoad:
                    TextureLoad(builtin);
                    break;
                case core::BuiltinFn::kTextureNumLayers:
                    TextureNumLayers(builtin);
                    break;
                case core::BuiltinFn::kTextureNumLevels:
                    TextureNumLevels(builtin);
                    break;
                case core::BuiltinFn::kTextureNumSamples:
                    TextureNumSamples(builtin);
                    break;
                case core::BuiltinFn::kTextureSample:
                    TextureSample(builtin);
                    break;
                case core::BuiltinFn::kTextureSampleBias:
                    TextureSampleBias(builtin);
                    break;
                case core::BuiltinFn::kTextureSampleCompare:
                    TextureSampleCompare(builtin);
                    break;
                case core::BuiltinFn::kTextureSampleCompareLevel:
                    TextureSampleCompareLevel(builtin);
                    break;
                case core::BuiltinFn::kTextureSampleGrad:
                    TextureSampleGrad(builtin);
                    break;
                case core::BuiltinFn::kTextureSampleLevel:
                    TextureSampleLevel(builtin);
                    break;
                case core::BuiltinFn::kTextureStore:
                    TextureStore(builtin);
                    break;

                // Barriers.
                case core::BuiltinFn::kStorageBarrier:
                    ThreadgroupBarrier(builtin, BarrierType::kDevice);
                    break;
                case core::BuiltinFn::kWorkgroupBarrier:
                    ThreadgroupBarrier(builtin, BarrierType::kThreadGroup);
                    break;
                case core::BuiltinFn::kTextureBarrier:
                    ThreadgroupBarrier(builtin, BarrierType::kTexture);
                    break;

                // QuadSwap builtins.
                case core::BuiltinFn::kQuadSwapDiagonal:
                    QuadSwap(builtin, 0b11);
                    break;
                case core::BuiltinFn::kQuadSwapX:
                    QuadSwap(builtin, 0b01);
                    break;
                case core::BuiltinFn::kQuadSwapY:
                    QuadSwap(builtin, 0b10);
                    break;

                // Pack/unpack builtins.
                case core::BuiltinFn::kPack2X16Float:
                    Pack2x16Float(builtin);
                    break;
                case core::BuiltinFn::kUnpack2X16Float:
                    Unpack2x16Float(builtin);
                    break;

                // Subgroup matrix builtins.
                case core::BuiltinFn::kSubgroupMatrixLoad:
                    SubgroupMatrixLoad(builtin);
                    break;
                case core::BuiltinFn::kSubgroupMatrixStore:
                    SubgroupMatrixStore(builtin);
                    break;
                case core::BuiltinFn::kSubgroupMatrixMultiply:
                    SubgroupMatrixMultiply(builtin);
                    break;
                case core::BuiltinFn::kSubgroupMatrixMultiplyAccumulate:
                    SubgroupMatrixMultiplyAccumulate(builtin);
                    break;

                default:
                    break;
            }
        }
    }

    /// Replace an atomic builtin call with an equivalent MSL intrinsic.
    /// @param builtin the builtin call instruction
    void AtomicCall(core::ir::CoreBuiltinCall* builtin, msl::BuiltinFn intrinsic) {
        auto args = Vector<core::ir::Value*, 4>{builtin->Args()};
        args.Push(
            ir.CreateValue<msl::ir::MemoryOrder>(b.ConstantValue(u32(std::memory_order_relaxed))));
        auto* call = b.CallWithResult<msl::ir::BuiltinCall>(builtin->DetachResult(), intrinsic,
                                                            std::move(args));
        call->InsertBefore(builtin);
        builtin->Destroy();
    }

    /// Replace an atomicCompareExchangeWeak builtin call with an equivalent MSL polyfill.
    /// @param builtin the builtin call instruction
    void AtomicCompareExchangeWeak(core::ir::CoreBuiltinCall* builtin) {
        // Get or generate a polyfill function.
        auto* atomic_ptr = builtin->Args()[0]->Type();
        auto* polyfill = atomic_compare_exchange_polyfills.GetOrAdd(atomic_ptr, [&] {
            // The polyfill function performs the equivalent to the following:
            //     int old_value = cmp;
            //     bool exchanged = atomic_compare_exchange_weak_explicit(
            //                         atomic_ptr, old_value, val,
            //                         memory_order_relaxed, memory_order_relaxed);
            //     return __atomic_compare_exchange_result_i32(old_value, exchanged);
            auto* ptr = b.FunctionParam("atomic_ptr", atomic_ptr);
            auto* cmp = b.FunctionParam("cmp", builtin->Args()[1]->Type());
            auto* val = b.FunctionParam("val", builtin->Args()[2]->Type());
            auto* func = b.Function(builtin->Result(0)->Type());
            func->SetParams({ptr, cmp, val});
            b.Append(func->Block(), [&] {
                auto* old_value = b.Var<function>("old_value", cmp)->Result(0);
                auto* order = ir.CreateValue<msl::ir::MemoryOrder>(
                    b.ConstantValue(u32(std::memory_order_relaxed)));
                auto* call = b.Call<msl::ir::BuiltinCall>(
                    ty.bool_(), BuiltinFn::kAtomicCompareExchangeWeakExplicit,
                    Vector{ptr, old_value, val, order, order});
                auto* result =
                    b.Construct(builtin->Result(0)->Type(), Vector{
                                                                b.Load(old_value)->Result(0),
                                                                call->Result(0),
                                                            });
                b.Return(func, result);
            });
            return func;
        });

        // Call the polyfill function.
        auto args = Vector<core::ir::Value*, 4>{builtin->Args()};
        auto* call = b.CallWithResult(builtin->DetachResult(), polyfill, std::move(args));
        call->InsertBefore(builtin);
        builtin->Destroy();
    }

    /// Polyfill a distance call if necessary.
    /// @param builtin the builtin call instruction
    void Distance(core::ir::CoreBuiltinCall* builtin) {
        b.InsertBefore(builtin, [&] {
            auto* arg0 = builtin->Args()[0];
            auto* arg1 = builtin->Args()[1];
            if (arg0->Type()->Is<core::type::Scalar>()) {
                // Calls to `distance` with a scalar argument are replaced with `abs(a - b)`.
                auto* sub = b.Subtract(builtin->Result(0)->Type(), arg0, arg1);
                b.CallWithResult(builtin->DetachResult(), core::BuiltinFn::kAbs, sub);
            } else {
                b.CallWithResult<msl::ir::BuiltinCall>(builtin->DetachResult(),
                                                       msl::BuiltinFn::kDistance, arg0, arg1);
            }
        });
        builtin->Destroy();
    }

    /// Polyfill a dot call if necessary.
    /// @param builtin the builtin call instruction
    void Dot(core::ir::CoreBuiltinCall* builtin) {
        b.InsertBefore(builtin, [&] {
            auto* arg0 = builtin->Args()[0];
            auto* arg1 = builtin->Args()[1];
            auto* vec = arg0->Type()->As<core::type::Vector>();
            if (vec->Type()->IsIntegerScalar()) {
                // Calls to `dot` with a integer arguments are replaced with helper functions, as
                // MSL's `dot` builtin only supports floating point arguments.
                auto* polyfill = integer_dot_polyfills.GetOrAdd(vec, [&] {
                    // Generate a helper function that performs the following:
                    //     fn tint_integer_dot(lhs: vec4i, rhs: vec4i) {
                    //         let mul = lhs * rhs;
                    //         return mul[0] + mul[1] + mul[2] + mul[3];
                    //     }
                    auto* el_ty = vec->Type();
                    auto* lhs = b.FunctionParam("lhs", vec);
                    auto* rhs = b.FunctionParam("rhs", vec);
                    auto* func = b.Function("tint_dot", el_ty);
                    func->SetParams({lhs, rhs});
                    b.Append(func->Block(), [&] {
                        auto* mul = b.Multiply(vec, lhs, rhs);
                        auto* sum = b.Access(el_ty, mul, u32(0))->Result(0);
                        for (uint32_t i = 1; i < vec->Width(); i++) {
                            sum = b.Add(el_ty, sum, b.Access(el_ty, mul, u32(i)))->Result(0);
                        }
                        b.Return(func, sum);
                    });
                    return func;
                });
                b.CallWithResult(builtin->DetachResult(), polyfill, arg0, arg1);
            } else {
                b.CallWithResult<msl::ir::BuiltinCall>(builtin->DetachResult(),
                                                       msl::BuiltinFn::kDot, arg0, arg1);
            }
        });
        builtin->Destroy();
    }

    /// Polyfill a frexp call.
    /// @param builtin the builtin call instruction
    void Frexp(core::ir::CoreBuiltinCall* builtin) {
        b.InsertBefore(builtin, [&] {
            // MSL's frexp returns `fract` and outputs `exp` as an output parameter.
            // Polyfill it by declaring the result struct and then setting the values:
            //   __frexp_result result = {};
            //   result.fract = frexp(arg, result.exp);
            //
            // Note: We need to use a `load` instruction to pass `result.exp`, as the intrinsic
            // definition expects a value type (as we do not have reference types in the IR). The
            // printer will just fold away the load, which achieves the pass-by-reference semantics
            // that we want.
            //
            auto* result_type = builtin->Result(0)->Type();
            auto* float_type = result_type->Element(0);
            auto* i32_type = result_type->Element(1);
            auto* result = b.Var(ty.ptr(function, result_type));
            auto* exp = b.Access(ty.ptr(function, i32_type), result, u32(1));
            auto args = Vector<core::ir::Value*, 2>{builtin->Args()[0], b.Load(exp)->Result(0)};
            auto* call =
                b.Call<msl::ir::BuiltinCall>(float_type, msl::BuiltinFn::kFrexp, std::move(args));
            b.Store(b.Access(ty.ptr(function, float_type), result, u32(0)), call);
            builtin->Result(0)->ReplaceAllUsesWith(b.Load(result)->Result(0));
        });
        builtin->Destroy();
    }

    /// Polyfill a length call if necessary.
    /// @param builtin the builtin call instruction
    void Length(core::ir::CoreBuiltinCall* builtin) {
        auto* arg = builtin->Args()[0];
        if (arg->Type()->Is<core::type::Scalar>()) {
            // Calls to `length` with a scalar argument are replaced with `abs`.
            auto* call = b.CallWithResult(builtin->DetachResult(), core::BuiltinFn::kAbs, arg);
            call->InsertBefore(builtin);
        } else {
            auto* call = b.CallWithResult<msl::ir::BuiltinCall>(builtin->DetachResult(),
                                                                msl::BuiltinFn::kLength, arg);
            call->InsertBefore(builtin);
        }
        builtin->Destroy();
    }

    /// Polyfill a modf call.
    /// @param builtin the builtin call instruction
    void Modf(core::ir::CoreBuiltinCall* builtin) {
        b.InsertBefore(builtin, [&] {
            // MSL's modf returns `fract` and outputs `whole` as an output parameter.
            // Polyfill it by declaring the result struct and then setting the values:
            //   __modf_result result = {};
            //   result.fract = modf(arg, result.whole);
            //
            // Note: We need to use a `load` instruction to pass `result.whole`, as the intrinsic
            // definition expects a value type (as we do not have reference types in the IR). The
            // printer will just fold away the load, which achieves the pass-by-reference semantics
            // that we want.
            //
            auto* result_type = builtin->Result(0)->Type();
            auto* element_type = result_type->Element(0);
            auto* result = b.Var(ty.ptr(function, result_type));
            auto* whole = b.Access(ty.ptr(function, element_type), result, u32(1));
            auto args = Vector<core::ir::Value*, 2>{builtin->Args()[0], b.Load(whole)->Result(0)};
            auto* call =
                b.Call<msl::ir::BuiltinCall>(element_type, msl::BuiltinFn::kModf, std::move(args));
            b.Store(b.Access(ty.ptr(function, element_type), result, u32(0)), call);
            builtin->Result(0)->ReplaceAllUsesWith(b.Load(result)->Result(0));
        });
        builtin->Destroy();
    }

    /// Polyfill an Pack2x16Float call.
    /// @param builtin the builtin call instruction
    void Pack2x16Float(core::ir::CoreBuiltinCall* builtin) {
        // Replace the call with `as_type<uint>(half2(value))`.
        b.InsertBefore(builtin, [&] {
            auto* convert = b.Convert<vec2<f16>>(builtin->Args()[0]);
            auto* bitcast = b.Bitcast(ty.u32(), convert);
            bitcast->SetResults(Vector{builtin->DetachResult()});
        });
        builtin->Destroy();
    }

    /// Polyfill a quantizeToF16 call.
    /// @param builtin the builtin call instruction
    void QuantizeToF16(core::ir::CoreBuiltinCall* builtin) {
        auto* arg = builtin->Args()[0];

        // Convert the argument to f16 and then back again.
        b.InsertBefore(builtin, [&] {
            b.ConvertWithResult(builtin->DetachResult(),
                                b.Convert(ty.MatchWidth(ty.f16(), arg->Type()), arg));
        });
        builtin->Destroy();
    }

    /// Polyfill a sign call if necessary.
    /// @param builtin the builtin call instruction
    void Sign(core::ir::CoreBuiltinCall* builtin) {
        auto* arg = builtin->Args()[0];
        auto* type = arg->Type();
        b.InsertBefore(builtin, [&] {
            // Calls to `sign` with an integer argument are replaced with select operations:
            //   result = select(select(-1, 1, arg > 0), 0, arg == 0);
            if (type->IsIntegerScalarOrVector()) {
                core::ir::Value* pos_one = b.MatchWidth(i32(1), type);
                core::ir::Value* neg_one = b.MatchWidth(i32(-1), type);
                const core::type::Type* bool_type = ty.MatchWidth(ty.bool_(), type);
                auto* zero = b.Zero(type);
                auto* sign = b.Call(type, core::BuiltinFn::kSelect, neg_one, pos_one,
                                    b.GreaterThan(bool_type, arg, zero));
                b.CallWithResult(builtin->DetachResult(), core::BuiltinFn::kSelect, sign, zero,
                                 b.Equal(bool_type, arg, zero));
            } else {
                b.CallWithResult<msl::ir::BuiltinCall>(builtin->DetachResult(),
                                                       msl::BuiltinFn::kSign, arg);
            }
        });
        builtin->Destroy();
    }

    /// Replace a textureDimensions call with the equivalent MSL intrinsics.
    /// @param builtin the builtin call instruction
    void TextureDimensions(core::ir::CoreBuiltinCall* builtin) {
        auto* tex = builtin->Args()[0];
        auto* type = tex->Type()->As<core::type::Texture>();
        bool needs_lod_arg = type->Dim() != core::type::TextureDimension::k1d &&
                             !type->Is<core::type::MultisampledTexture>() &&
                             !type->Is<core::type::DepthMultisampledTexture>();

        b.InsertBefore(builtin, [&] {
            // If we need a LOD argument, use the one provided or default to 0.
            core::ir::Value* lod = nullptr;
            if (needs_lod_arg) {
                if (builtin->Args().Length() == 1) {
                    lod = b.Value(u32(0));
                } else {
                    lod = builtin->Args()[1];
                    if (lod->Type()->IsSignedIntegerScalar()) {
                        lod = b.Convert<u32>(lod)->Result(0);
                    }
                }
            }

            // Call MSL member functions to get the dimensions of the image.
            Vector<core::ir::InstructionResult*, 4> values;
            auto get_dim = [&](msl::BuiltinFn fn) {
                auto* call = b.MemberCall<msl::ir::MemberBuiltinCall>(ty.u32(), fn, tex);
                if (lod) {
                    call->AppendArg(lod);
                }
                values.Push(call->Result(0));
            };
            get_dim(msl::BuiltinFn::kGetWidth);
            if (type->Dim() != core::type::TextureDimension::k1d) {
                get_dim(msl::BuiltinFn::kGetHeight);
                if (type->Dim() == core::type::TextureDimension::k3d) {
                    get_dim(msl::BuiltinFn::kGetDepth);
                }
            }

            // Reconstruct the original result type from the individual dimensions.
            b.ConstructWithResult(builtin->DetachResult(), std::move(values));
        });
        builtin->Destroy();
    }

    /// Replace a textureGather call with the equivalent MSL intrinsic.
    /// @param builtin the builtin call instruction
    void TextureGather(core::ir::CoreBuiltinCall* builtin) {
        // If there is a component argument it will always be first followed by the texture object.
        // Otherwise, the texture object will be first.
        core::ir::Value* tex = nullptr;
        Vector<core::ir::Value*, 4> args;
        auto* component = builtin->Args()[0]->As<core::ir::Constant>();
        if (component) {
            tex = builtin->Args()[1];
            args = builtin->Args().Offset(2);
        } else {
            tex = builtin->Args()[0];
            args = builtin->Args().Offset(1);
        }
        auto* tex_type = tex->Type()->As<core::type::Texture>();

        // Add an offset argument if it was not provided.
        const bool has_offset = args.Back()->Type()->IsSignedIntegerVector();
        const bool needs_offset = tex_type->Dim() == core::type::TextureDimension::k2d ||
                                  tex_type->Dim() == core::type::TextureDimension::k2dArray;
        if (needs_offset && !has_offset) {
            args.Push(b.Zero<vec2<i32>>());
        }

        // Add the component argument if needed, converting it to u32 if necessary.
        if (component) {
            if (component->Type()->Is<core::type::I32>()) {
                component = b.Constant(component->Value()->ValueAs<u32>());
            }
            args.Push(ir.CreateValue<msl::ir::Component>(component->Value()));
        }

        // Call the `gather()` member function.
        auto* call = b.MemberCallWithResult<msl::ir::MemberBuiltinCall>(
            builtin->DetachResult(), msl::BuiltinFn::kGather, tex, std::move(args));
        call->InsertBefore(builtin);
        TextureSampleClampArrayIndexHelper(call);
        builtin->Destroy();
    }

    /// Replace a textureGatherCompare call with the equivalent MSL intrinsic.
    /// @param builtin the builtin call instruction
    void TextureGatherCompare(core::ir::CoreBuiltinCall* builtin) {
        // The MSL intrinsic is a member function, so we split the first argument off as the object.
        auto args = Vector<core::ir::Value*, 4>(builtin->Args().Offset(1));
        auto* call = b.MemberCallWithResult<msl::ir::MemberBuiltinCall>(
            builtin->DetachResult(), msl::BuiltinFn::kGatherCompare, builtin->Args()[0],
            std::move(args));
        call->InsertBefore(builtin);
        TextureSampleClampArrayIndexHelper(call);
        builtin->Destroy();
    }

    /// Replace a textureLoad call with the equivalent MSL intrinsic.
    /// @param builtin the builtin call instruction
    void TextureLoad(core::ir::CoreBuiltinCall* builtin) {
        uint32_t next_arg = 0;
        auto* tex = builtin->Args()[next_arg++];
        auto* tex_type = tex->Type()->As<core::type::Texture>();

        // Extract the arguments from the core builtin call.
        auto* coords = builtin->Args()[next_arg++];
        core::ir::Value* index = nullptr;
        core::ir::Value* lod_or_sample = nullptr;
        if (tex_type->Dim() == core::type::TextureDimension::k2dArray) {
            index = builtin->Args()[next_arg++];
        }
        if (tex_type->Dim() != core::type::TextureDimension::k1d &&
            !tex_type->Is<core::type::StorageTexture>()) {
            lod_or_sample = builtin->Args()[next_arg++];
        }

        b.InsertBefore(builtin, [&] {
            // Convert the coordinates to unsigned integers if necessary.
            if (coords->Type()->IsSignedIntegerScalarOrVector()) {
                coords = b.Convert(ty.MatchWidth(ty.u32(), coords->Type()), coords)->Result(0);
            }

            // Call the `read()` member function.
            Vector<core::ir::Value*, 4> args{coords};
            if (index) {
                args.Push(index);
            }
            if (lod_or_sample) {
                args.Push(lod_or_sample);
            }
            b.MemberCallWithResult<msl::ir::MemberBuiltinCall>(
                builtin->DetachResult(), msl::BuiltinFn::kRead, tex, std::move(args));
        });
        builtin->Destroy();
    }

    /// Replace a textureNum* call with the equivalent MSL intrinsic.
    /// @param builtin the builtin call instruction
    /// @param fn the MSL intrinsic function
    void TextureNumHelper(core::ir::CoreBuiltinCall* builtin, msl::BuiltinFn fn) {
        // The MSL intrinsic is a member function, so we split the first argument off as the object.
        auto* tex = builtin->Args()[0];
        auto* call =
            b.MemberCallWithResult<msl::ir::MemberBuiltinCall>(builtin->DetachResult(), fn, tex);
        call->InsertBefore(builtin);
        builtin->Destroy();
    }

    /// Replace texture sample call signed integer array argument with clamped code.
    /// @param builtin the builtin call instruction
    void TextureSampleClampArrayIndexHelper(msl::ir::MemberBuiltinCall* builtin) {
        // The MSL intrinsic is a member function, so we split the first argument off as the
        // object.
        b.InsertBefore(builtin, [&] {
            auto* tex = builtin->Object();
            auto* tex_type = tex->Type()->As<core::type::Texture>();
            if (IsTextureArray(tex_type->Dim())) {
                const uint32_t kArrayIndex = 2;
                auto* index_arg = builtin->Args()[kArrayIndex];
                if (index_arg->Type()->IsSignedIntegerScalar()) {
                    builtin->SetArg(kArrayIndex, b.Call(ty.i32(), core::BuiltinFn::kMax, index_arg,
                                                        b.Zero<i32>())
                                                     ->Result(0));
                }
            }
        });
    }

    /// Replace a textureNumLayers call with the equivalent MSL intrinsic.
    /// @param builtin the builtin call instruction
    void TextureNumLayers(core::ir::CoreBuiltinCall* builtin) {
        TextureNumHelper(builtin, msl::BuiltinFn::kGetArraySize);
    }

    /// Replace a textureNumLevels call with the equivalent MSL intrinsic.
    /// @param builtin the builtin call instruction
    void TextureNumLevels(core::ir::CoreBuiltinCall* builtin) {
        TextureNumHelper(builtin, msl::BuiltinFn::kGetNumMipLevels);
    }

    /// Replace a textureNumSamples call with the equivalent MSL intrinsic.
    /// @param builtin the builtin call instruction
    void TextureNumSamples(core::ir::CoreBuiltinCall* builtin) {
        TextureNumHelper(builtin, msl::BuiltinFn::kGetNumSamples);
    }

    /// Replace a textureSample call with the equivalent MSL intrinsic.
    /// @param builtin the builtin call instruction
    void TextureSample(core::ir::CoreBuiltinCall* builtin) {
        // The MSL intrinsic is a member function, so we split the first argument off as the object.
        auto args = Vector<core::ir::Value*, 4>(builtin->Args().Offset(1));
        auto* call = b.MemberCallWithResult<msl::ir::MemberBuiltinCall>(
            builtin->DetachResult(), msl::BuiltinFn::kSample, builtin->Args()[0], std::move(args));
        call->InsertBefore(builtin);
        TextureSampleClampArrayIndexHelper(call);
        builtin->Destroy();
    }

    /// Replace a textureSampleBias call with the equivalent MSL intrinsic.
    /// @param builtin the builtin call instruction
    void TextureSampleBias(core::ir::CoreBuiltinCall* builtin) {
        // The MSL intrinsic is a member function, so we split the first argument off as the object.
        auto* tex = builtin->Args()[0];
        auto* tex_type = tex->Type()->As<core::type::Texture>();
        auto args = Vector<core::ir::Value*, 4>(builtin->Args().Offset(1));

        b.InsertBefore(builtin, [&] {
            // Wrap the bias argument in a constructor for the MSL `bias` builtin type.
            uint32_t bias_idx = 2;
            if (tex_type->Dim() == core::type::TextureDimension::k2dArray ||
                tex_type->Dim() == core::type::TextureDimension::kCubeArray) {
                bias_idx = 3;
            }
            args[bias_idx] = b.Construct(ty.Get<msl::type::Bias>(), args[bias_idx])->Result(0);
        });
        // Call the `sample()` member function.
        auto* call = b.MemberCallWithResult<msl::ir::MemberBuiltinCall>(
            builtin->DetachResult(), msl::BuiltinFn::kSample, tex, std::move(args));
        call->InsertBefore(builtin);
        TextureSampleClampArrayIndexHelper(call);
        builtin->Destroy();
    }

    /// Replace a textureSampleCompare call with the equivalent MSL intrinsic.
    /// @param builtin the builtin call instruction
    void TextureSampleCompare(core::ir::CoreBuiltinCall* builtin) {
        // The MSL intrinsic is a member function, so we split the first argument off as the object.
        auto args = Vector<core::ir::Value*, 4>(builtin->Args().Offset(1));
        auto* call = b.MemberCallWithResult<msl::ir::MemberBuiltinCall>(
            builtin->DetachResult(), msl::BuiltinFn::kSampleCompare, builtin->Args()[0],
            std::move(args));
        call->InsertBefore(builtin);
        TextureSampleClampArrayIndexHelper(call);
        builtin->Destroy();
    }

    /// Replace a textureSampleCompareLevel call with the equivalent MSL intrinsic.
    /// @param builtin the builtin call instruction
    void TextureSampleCompareLevel(core::ir::CoreBuiltinCall* builtin) {
        // The MSL intrinsic is a member function, so we split the first argument off as the object.
        auto* tex = builtin->Args()[0];
        auto args = Vector<core::ir::Value*, 4>(builtin->Args().Offset(1));

        // The overloads that don't use an offset all have the depth_ref as their final argument.
        const bool has_offset = !args.Back()->Type()->Is<core::type::F32>();

        b.InsertBefore(builtin, [&] {
            // Insert a constant zero LOD argument.
            // The LOD goes before the offset if there is one, otherwise at the end.
            auto* lod = b.Construct(ty.Get<msl::type::Level>(), u32(0))->Result(0);
            if (has_offset) {
                args.Insert(args.Length() - 1, lod);
            } else {
                args.Push(lod);
            }
        });
        // Call the `sample_compare()` member function.
        auto* call = b.MemberCallWithResult<msl::ir::MemberBuiltinCall>(
            builtin->DetachResult(), msl::BuiltinFn::kSampleCompare, tex, std::move(args));
        call->InsertBefore(builtin);
        TextureSampleClampArrayIndexHelper(call);
        builtin->Destroy();
    }

    /// Replace a textureSampleGrad call with the equivalent MSL intrinsic.
    /// @param builtin the builtin call instruction
    void TextureSampleGrad(core::ir::CoreBuiltinCall* builtin) {
        // The MSL intrinsic is a member function, so we split the first argument off as the object.
        auto* tex = builtin->Args()[0];
        auto* tex_type = tex->Type()->As<core::type::Texture>();
        auto args = Vector<core::ir::Value*, 4>(builtin->Args().Offset(1));

        b.InsertBefore(builtin, [&] {
            // Find the ddx and ddy arguments.
            uint32_t grad_idx = 2;
            if (tex_type->Dim() == core::type::TextureDimension::k2dArray ||
                tex_type->Dim() == core::type::TextureDimension::kCubeArray) {
                grad_idx = 3;
            }
            auto* ddx = args[grad_idx];
            auto* ddy = args[grad_idx + 1];

            // Wrap the ddx and ddy arguments in a constructor for the MSL `gradient` builtin type.
            enum type::Gradient::Dim dim;
            switch (tex_type->Dim()) {
                case core::type::TextureDimension::k2d:
                case core::type::TextureDimension::k2dArray:
                    dim = type::Gradient::Dim::k2d;
                    break;
                case core::type::TextureDimension::k3d:
                    dim = type::Gradient::Dim::k3d;
                    break;
                case core::type::TextureDimension::kCube:
                case core::type::TextureDimension::kCubeArray:
                    dim = type::Gradient::Dim::kCube;
                    break;
                case core::type::TextureDimension::k1d:
                case core::type::TextureDimension::kNone:
                    TINT_UNREACHABLE();
            }
            args[grad_idx] = b.Construct(ty.Get<msl::type::Gradient>(dim), ddx, ddy)->Result(0);

            // Resize the argument list as the gradient argument only takes up one argument.
            // Move the offset argument back one place if present.
            const bool has_offset = args.Back()->Type()->IsSignedIntegerVector();
            if (has_offset) {
                args[args.Length() - 2] = args.Back();
            }
            args.Resize(args.Length() - 1);
            // Call the `sample()` member function.
            auto* call = b.MemberCallWithResult<msl::ir::MemberBuiltinCall>(
                builtin->DetachResult(), msl::BuiltinFn::kSample, tex, std::move(args));
            TextureSampleClampArrayIndexHelper(call);
        });

        builtin->Destroy();
    }

    /// Replace a textureSampleLevel call with the equivalent MSL intrinsic.
    /// @param builtin the builtin call instruction
    void TextureSampleLevel(core::ir::CoreBuiltinCall* builtin) {
        // The MSL intrinsic is a member function, so we split the first argument off as the object.
        auto* tex = builtin->Args()[0];
        auto* tex_type = tex->Type()->As<core::type::Texture>();
        auto args = Vector<core::ir::Value*, 4>(builtin->Args().Offset(1));

        b.InsertBefore(builtin, [&] {
            // Wrap the LOD argument in a constructor for the MSL `level` builtin type.
            uint32_t lod_idx = 2;
            if (tex_type->Dim() == core::type::TextureDimension::k2dArray ||
                tex_type->Dim() == core::type::TextureDimension::kCubeArray) {
                lod_idx = 3;
            }
            args[lod_idx] = b.Construct(ty.Get<msl::type::Level>(), args[lod_idx])->Result(0);
            // Call the `sample()` member function.
            auto* call = b.MemberCallWithResult<msl::ir::MemberBuiltinCall>(
                builtin->DetachResult(), msl::BuiltinFn::kSample, tex, std::move(args));
            TextureSampleClampArrayIndexHelper(call);
        });
        builtin->Destroy();
    }

    /// Replace a textureStore call with the equivalent MSL intrinsic.
    /// @param builtin the builtin call instruction
    void TextureStore(core::ir::CoreBuiltinCall* builtin) {
        auto* tex = builtin->Args()[0];
        auto* tex_type = tex->Type()->As<core::type::StorageTexture>();

        // Extract the arguments from the core builtin call.
        auto* coords = builtin->Args()[1];
        core::ir::Value* value = nullptr;
        core::ir::Value* index = nullptr;
        if (tex_type->Dim() == core::type::TextureDimension::k2dArray) {
            index = builtin->Args()[2];
            value = builtin->Args()[3];
        } else {
            value = builtin->Args()[2];
        }

        b.InsertBefore(builtin, [&] {
            // Convert the coordinates to unsigned integers if necessary.
            if (coords->Type()->IsSignedIntegerScalarOrVector()) {
                coords = b.Convert(ty.MatchWidth(ty.u32(), coords->Type()), coords)->Result(0);
            }

            // Call the `write()` member function.
            Vector<core::ir::Value*, 4> args;
            args.Push(value);
            args.Push(coords);
            if (index) {
                args.Push(index);
            }
            b.MemberCall<msl::ir::MemberBuiltinCall>(ty.void_(), msl::BuiltinFn::kWrite, tex,
                                                     std::move(args));

            // If we are writing to a read-write texture, add a fence to ensure that the written
            // values are visible to subsequent reads from the same thread.
            if (tex_type->Access() == core::Access::kReadWrite) {
                b.MemberCall<msl::ir::MemberBuiltinCall>(ty.void_(), msl::BuiltinFn::kFence, tex);
            }
        });
        builtin->Destroy();
    }

    /// Replace a barrier builtin with the `threadgroupBarrier()` intrinsic.
    /// @param builtin the builtin call instruction
    /// @param type the barrier type
    void ThreadgroupBarrier(core::ir::CoreBuiltinCall* builtin, BarrierType type) {
        // Replace the builtin call with a call to the msl.threadgroup_barrier intrinsic.
        auto args = Vector<core::ir::Value*, 1>{b.Constant(u32(type))};
        auto* call = b.CallWithResult<msl::ir::BuiltinCall>(
            builtin->DetachResult(), msl::BuiltinFn::kThreadgroupBarrier, std::move(args));
        call->InsertBefore(builtin);
        builtin->Destroy();
    }

    /// Replace a quadSwap* builtin with the `quad_shuffle_xor()` intrinsic.
    /// @param builtin the builtin call instruction
    /// @param mask the shuffle mask
    void QuadSwap(core::ir::CoreBuiltinCall* builtin, uint32_t mask) {
        // Replace the builtin call with a call to the msl.quad_shuffle_xor intrinsic.
        auto args = Vector<core::ir::Value*, 2>{builtin->Args()[0], b.Constant(u32(mask))};
        auto* call = b.CallWithResult<msl::ir::BuiltinCall>(
            builtin->DetachResult(), msl::BuiltinFn::kQuadShuffleXor, std::move(args));
        call->InsertBefore(builtin);
        builtin->Destroy();
    }

    /// Polyfill an Unpack2x16Float call.
    /// @param builtin the builtin call instruction
    void Unpack2x16Float(core::ir::CoreBuiltinCall* builtin) {
        // Replace the call with `float2(as_type<half2>(value))`.
        b.InsertBefore(builtin, [&] {
            auto* bitcast = b.Bitcast<vec2<f16>>(builtin->Args()[0]);
            b.ConvertWithResult(builtin->DetachResult(), bitcast);
        });
        builtin->Destroy();
    }

    /// Replace a subgroupMatrixLoad builtin.
    /// @param builtin the builtin call instruction
    void SubgroupMatrixLoad(core::ir::CoreBuiltinCall* builtin) {
        b.InsertBefore(builtin, [&] {
            auto* p = builtin->Args()[0];
            auto* offset = builtin->Args()[1];
            auto* col_major = builtin->Args()[2];
            auto* stride = builtin->Args()[3];

            auto* ptr = p->Type()->As<core::type::Pointer>();
            auto* arr = ptr->StoreType()->As<core::type::Array>();

            // Make a pointer to the first element of the array that we will read from.
            auto* elem_ptr = ty.ptr(ptr->AddressSpace(), arr->ElemType(), ptr->Access());
            auto* src = b.Access(elem_ptr, p, offset);

            // The origin is always (0, 0), as we use `offset` to set the start of the data.
            auto* matrix_origin = b.Zero<vec2<u64>>();

            // Convert the u32 stride to the ulong that MSL expects.
            auto* elements_per_row =
                b.Call<msl::ir::BuiltinCall>(ty.u64(), msl::BuiltinFn::kConvert, stride);

            // Declare a local variable to load the matrix into.
            auto* tmp = b.Var(ty.ptr<function>(builtin->Result(0)->Type()));
            // Note: We need to use a `load` instruction to pass the variable, as the intrinsic
            // definition expects a value type (as we do not have reference types in the IR). The
            // printer will just fold away the load, which achieves the pass-by-reference semantics
            // that we want.
            b.Call<msl::ir::BuiltinCall>(ty.void_(), msl::BuiltinFn::kSimdgroupLoad,
                                         b.Load(tmp->Result(0)), src, elements_per_row,
                                         matrix_origin, col_major);
            b.LoadWithResult(builtin->DetachResult(), tmp);
        });
        builtin->Destroy();
    }

    /// Replace a subgroupMatrixStore builtin.
    /// @param builtin the builtin call instruction
    void SubgroupMatrixStore(core::ir::CoreBuiltinCall* builtin) {
        b.InsertBefore(builtin, [&] {
            auto* p = builtin->Args()[0];
            auto* offset = builtin->Args()[1];
            auto* value = builtin->Args()[2];
            auto* col_major = builtin->Args()[3];
            auto* stride = builtin->Args()[4];

            auto* ptr = p->Type()->As<core::type::Pointer>();
            auto* arr = ptr->StoreType()->As<core::type::Array>();

            // Make a pointer to the first element of the array that we will write to.
            auto* elem_ptr = ty.ptr(ptr->AddressSpace(), arr->ElemType(), ptr->Access());
            auto* dst = b.Access(elem_ptr, p, offset);

            // Convert the u32 stride to the ulong that MSL expects.
            auto* elements_per_row =
                b.Call<msl::ir::BuiltinCall>(ty.u64(), msl::BuiltinFn::kConvert, stride);

            // The origin is always (0, 0), as we use `offset` to set the start of the data.
            auto* matrix_origin = b.Zero<vec2<u64>>();

            b.Call<msl::ir::BuiltinCall>(ty.void_(), msl::BuiltinFn::kSimdgroupStore, value, dst,
                                         elements_per_row, matrix_origin, col_major);
        });
        builtin->Destroy();
    }

    /// Replace a subgroupMatrixMultiply builtin.
    /// @param builtin the builtin call instruction
    void SubgroupMatrixMultiply(core::ir::CoreBuiltinCall* builtin) {
        b.InsertBefore(builtin, [&] {
            auto* left = builtin->Args()[0];
            auto* right = builtin->Args()[1];

            // Declare a local variable to receive the result.
            auto* tmp = b.Var(ty.ptr<function>(builtin->Result(0)->Type()));
            // Note: We need to use a `load` instruction to pass the variable, as the intrinsic
            // definition expects a value type (as we do not have reference types in the IR). The
            // printer will just fold away the load, which achieves the pass-by-reference semantics
            // that we want.
            b.Call<msl::ir::BuiltinCall>(ty.void_(), msl::BuiltinFn::kSimdgroupMultiply,
                                         b.Load(tmp->Result(0)), left, right);
            b.LoadWithResult(builtin->DetachResult(), tmp);
        });
        builtin->Destroy();
    }

    /// Replace a subgroupMatrixMultiplyAccumulate builtin.
    /// @param builtin the builtin call instruction
    void SubgroupMatrixMultiplyAccumulate(core::ir::CoreBuiltinCall* builtin) {
        b.InsertBefore(builtin, [&] {
            auto* left = builtin->Args()[0];
            auto* right = builtin->Args()[1];
            auto* acc = builtin->Args()[2];

            // Declare a local variable to receive the result.
            auto* tmp = b.Var(ty.ptr<function>(builtin->Result(0)->Type()));
            // Note: We need to use a `load` instruction to pass the variable, as the intrinsic
            // definition expects a value type (as we do not have reference types in the IR). The
            // printer will just fold away the load, which achieves the pass-by-reference semantics
            // that we want.
            b.Call<msl::ir::BuiltinCall>(ty.void_(), msl::BuiltinFn::kSimdgroupMultiplyAccumulate,
                                         b.Load(tmp->Result(0)), left, right, acc);
            b.LoadWithResult(builtin->DetachResult(), tmp);
        });
        builtin->Destroy();
    }
};

}  // namespace

Result<SuccessType> BuiltinPolyfill(core::ir::Module& ir) {
    auto result =
        ValidateAndDumpIfNeeded(ir, "msl.BuiltinPolyfill",
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
