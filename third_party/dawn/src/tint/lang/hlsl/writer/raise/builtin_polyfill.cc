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

#include "src/tint/lang/hlsl/writer/raise/builtin_polyfill.h"

#include <tuple>
#include <vector>

#include "src/tint/lang/core/fluent_types.h"  // IWYU pragma: export
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/subgroup_matrix.h"
#include "src/tint/lang/core/type/texture.h"
#include "src/tint/lang/core/type/u16.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/u64.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/hlsl/builtin_fn.h"
#include "src/tint/lang/hlsl/ir/builtin_call.h"
#include "src/tint/lang/hlsl/ir/member_builtin_call.h"
#include "src/tint/lang/hlsl/ir/ternary.h"
#include "src/tint/lang/hlsl/type/int8_t4_packed.h"
#include "src/tint/lang/hlsl/type/matrix_layout.h"
#include "src/tint/lang/hlsl/type/uint8_t4_packed.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/math/hash.h"

namespace tint::hlsl::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The configuration for the transform.
    const BuiltinPolyfillConfig& config;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    // Polyfill functions for bitcast expression, BitcastType indicates the source type and the
    // destination type.
    using BitcastType =
        tint::UnorderedKeyWrapper<std::tuple<const core::type::Type*, const core::type::Type*>>;
    Hashmap<BitcastType, core::ir::Function*, 4> bitcast_funcs_{};

    using MatrixScalarKey =
        tint::UnorderedKeyWrapper<std::tuple<const core::type::SubgroupMatrix*, core::BinaryOp>>;
    Hashmap<MatrixScalarKey, core::ir::Function*, 4> matrix_scalar_funcs_{};

    using MatrixMultiplyAccumulateKey = tint::UnorderedKeyWrapper<
        std::tuple<const core::type::SubgroupMatrix*, const core::type::SubgroupMatrix*>>;
    Hashmap<MatrixMultiplyAccumulateKey, core::ir::Function*, 4>
        matrix_multiply_accumulate_funcs_{};

    /// Process the module.
    void Process() {
        // Find the bitcasts that need replacing.
        Vector<core::ir::CoreBuiltinCall*, 4> bitcast_worklist;
        std::vector<std::function<void()>> call_worklist;
        call_worklist.reserve(128);

        for (auto* inst : ir.Instructions()) {
            if (auto* call = inst->As<core::ir::CoreBuiltinCall>()) {
                if (call->Func() == core::BuiltinFn::kBitcast) {
                    bitcast_worklist.Push(call);
                    continue;
                }
                switch (call->Func()) {
                    case core::BuiltinFn::kAcosh:
                        call_worklist.push_back([this, call] { Acosh(call); });
                        break;
                    case core::BuiltinFn::kAsinh:
                        call_worklist.push_back([this, call] { Asinh(call); });
                        break;
                    case core::BuiltinFn::kAtanh:
                        call_worklist.push_back([this, call] { Atanh(call); });
                        break;
                    case core::BuiltinFn::kAtomicAdd:
                        call_worklist.push_back([this, call] { AtomicAdd(call); });
                        break;
                    case core::BuiltinFn::kAtomicSub:
                        call_worklist.push_back([this, call] { AtomicSub(call); });
                        break;
                    case core::BuiltinFn::kAtomicMin:
                        call_worklist.push_back([this, call] { AtomicMin(call); });
                        break;
                    case core::BuiltinFn::kAtomicMax:
                        call_worklist.push_back([this, call] { AtomicMax(call); });
                        break;
                    case core::BuiltinFn::kAtomicAnd:
                        call_worklist.push_back([this, call] { AtomicAnd(call); });
                        break;
                    case core::BuiltinFn::kAtomicOr:
                        call_worklist.push_back([this, call] { AtomicOr(call); });
                        break;
                    case core::BuiltinFn::kAtomicXor:
                        call_worklist.push_back([this, call] { AtomicXor(call); });
                        break;
                    case core::BuiltinFn::kAtomicLoad:
                        call_worklist.push_back([this, call] { AtomicLoad(call); });
                        break;
                    case core::BuiltinFn::kAtomicStore:
                        call_worklist.push_back([this, call] { AtomicStore(call); });
                        break;
                    case core::BuiltinFn::kAtomicExchange:
                        call_worklist.push_back([this, call] { AtomicExchange(call); });
                        break;
                    case core::BuiltinFn::kAtomicCompareExchangeWeak:
                        call_worklist.push_back([this, call] { AtomicCompareExchangeWeak(call); });
                        break;
                    case core::BuiltinFn::kCountOneBits:
                        call_worklist.push_back([this, call] {
                            BitcastToIntOverloadCall(call);  // See crbug.com/tint/1550.
                        });
                        break;
                    case core::BuiltinFn::kDot4I8Packed:
                        call_worklist.push_back([this, call] { Dot4I8Packed(call); });
                        break;
                    case core::BuiltinFn::kDot4U8Packed:
                        call_worklist.push_back([this, call] { Dot4U8Packed(call); });
                        break;
                    case core::BuiltinFn::kFrexp:
                        call_worklist.push_back([this, call] { Frexp(call); });
                        break;
                    case core::BuiltinFn::kModf:
                        call_worklist.push_back([this, call] { Modf(call); });
                        break;
                    case core::BuiltinFn::kPack2X16Float:
                        call_worklist.push_back([this, call] { Pack2x16Float(call); });
                        break;
                    case core::BuiltinFn::kPack2X16Snorm:
                        call_worklist.push_back([this, call] { Pack2x16Snorm(call); });
                        break;
                    case core::BuiltinFn::kPack2X16Unorm:
                        call_worklist.push_back([this, call] { Pack2x16Unorm(call); });
                        break;
                    case core::BuiltinFn::kPack4X8Snorm:
                        call_worklist.push_back([this, call] { Pack4x8Snorm(call); });
                        break;
                    case core::BuiltinFn::kPack4X8Unorm:
                        call_worklist.push_back([this, call] { Pack4x8Unorm(call); });
                        break;
                    case core::BuiltinFn::kPack4XI8:
                        call_worklist.push_back([this, call] { Pack4xI8(call); });
                        break;
                    case core::BuiltinFn::kPack4XU8:
                        call_worklist.push_back([this, call] { Pack4xU8(call); });
                        break;
                    case core::BuiltinFn::kPack4XI8Clamp:
                        call_worklist.push_back([this, call] { Pack4xI8Clamp(call); });
                        break;
                    case core::BuiltinFn::kQuantizeToF16:
                        call_worklist.push_back([this, call] { QuantizeToF16(call); });
                        break;
                    case core::BuiltinFn::kReverseBits:
                        call_worklist.push_back([this, call] {
                            BitcastToIntOverloadCall(call);  // See crbug.com/tint/1550.
                        });
                        break;
                    case core::BuiltinFn::kSelect:
                        call_worklist.push_back([this, call] { Select(call); });
                        break;
                    case core::BuiltinFn::kSign:
                        call_worklist.push_back([this, call] { Sign(call); });
                        break;
                    case core::BuiltinFn::kSubgroupAnd:
                    case core::BuiltinFn::kSubgroupOr:
                    case core::BuiltinFn::kSubgroupXor:
                        call_worklist.push_back([this, call] { BitcastToIntOverloadCall(call); });
                        break;
                    case core::BuiltinFn::kSubgroupShuffleXor:
                    case core::BuiltinFn::kSubgroupShuffleUp:
                    case core::BuiltinFn::kSubgroupShuffleDown:
                        call_worklist.push_back([this, call] { SubgroupShuffle(call); });
                        break;
                    case core::BuiltinFn::kSubgroupInclusiveAdd:
                    case core::BuiltinFn::kSubgroupInclusiveMul:
                        call_worklist.push_back([this, call] { SubgroupInclusive(call); });
                        break;
                    case core::BuiltinFn::kSubgroupMatrixMultiply:
                        call_worklist.push_back([this, call] { SubgroupMatrixMultiply(call); });
                        break;
                    case core::BuiltinFn::kSubgroupMatrixScalarAdd:
                        call_worklist.push_back(
                            [this, call] { SubgroupMatrixScalar(call, core::BinaryOp::kAdd); });
                        break;
                    case core::BuiltinFn::kSubgroupMatrixScalarSubtract:
                        call_worklist.push_back([this, call] {
                            SubgroupMatrixScalar(call, core::BinaryOp::kSubtract);
                        });
                        break;
                    case core::BuiltinFn::kSubgroupMatrixScalarMultiply:
                        call_worklist.push_back([this, call] {
                            SubgroupMatrixScalar(call, core::BinaryOp::kMultiply);
                        });
                        break;
                    case core::BuiltinFn::kSubgroupMatrixMultiplyAccumulate:
                        call_worklist.push_back(
                            [this, call] { SubgroupMatrixMultiplyAccumulate(call); });
                        break;
                    case core::BuiltinFn::kSubgroupMatrixLoad:
                        call_worklist.push_back([this, call] { SubgroupMatrixLoad(call); });
                        break;
                    case core::BuiltinFn::kSubgroupMatrixStore:
                        call_worklist.push_back([this, call] { SubgroupMatrixStore(call); });
                        break;
                    case core::BuiltinFn::kTextureDimensions:
                        call_worklist.push_back([this, call] { TextureDimensions(call); });
                        break;
                    case core::BuiltinFn::kTextureGather:
                        call_worklist.push_back([this, call] { TextureGather(call); });
                        break;
                    case core::BuiltinFn::kTextureGatherCompare:
                        call_worklist.push_back([this, call] { TextureGatherCompare(call); });
                        break;
                    case core::BuiltinFn::kTextureLoad:
                        call_worklist.push_back([this, call] { TextureLoad(call); });
                        break;
                    case core::BuiltinFn::kTextureNumLayers:
                        call_worklist.push_back([this, call] { TextureNumLayers(call); });
                        break;
                    case core::BuiltinFn::kTextureNumLevels:
                        call_worklist.push_back([this, call] { TextureNumLevels(call); });
                        break;
                    case core::BuiltinFn::kTextureNumSamples:
                        call_worklist.push_back([this, call] { TextureNumSamples(call); });
                        break;
                    case core::BuiltinFn::kTextureSample:
                        call_worklist.push_back([this, call] { TextureSample(call); });
                        break;
                    case core::BuiltinFn::kTextureSampleBias:
                        call_worklist.push_back([this, call] { TextureSampleBias(call); });
                        break;
                    case core::BuiltinFn::kTextureSampleCompare:
                    case core::BuiltinFn::kTextureSampleCompareLevel:
                        call_worklist.push_back([this, call] { TextureSampleCompare(call); });
                        break;
                    case core::BuiltinFn::kTextureSampleGrad:
                        call_worklist.push_back([this, call] { TextureSampleGrad(call); });
                        break;
                    case core::BuiltinFn::kTextureSampleLevel:
                        call_worklist.push_back([this, call] { TextureSampleLevel(call); });
                        break;
                    case core::BuiltinFn::kTextureStore:
                        call_worklist.push_back([this, call] { TextureStore(call); });
                        break;
                    case core::BuiltinFn::kTrunc:
                        if (config.polyfill_trunc) {
                            call_worklist.push_back([this, call] { Trunc(call); });
                        }
                        break;
                    case core::BuiltinFn::kUnpack2X16Float:
                        call_worklist.push_back([this, call] { Unpack2x16Float(call); });
                        break;
                    case core::BuiltinFn::kUnpack2X16Snorm:
                        call_worklist.push_back([this, call] { Unpack2x16Snorm(call); });
                        break;
                    case core::BuiltinFn::kUnpack2X16Unorm:
                        call_worklist.push_back([this, call] { Unpack2x16Unorm(call); });
                        break;
                    case core::BuiltinFn::kUnpack4X8Snorm:
                        call_worklist.push_back([this, call] { Unpack4x8Snorm(call); });
                        break;
                    case core::BuiltinFn::kUnpack4X8Unorm:
                        call_worklist.push_back([this, call] { Unpack4x8Unorm(call); });
                        break;
                    case core::BuiltinFn::kUnpack4XI8:
                        call_worklist.push_back([this, call] { Unpack4xI8(call); });
                        break;
                    case core::BuiltinFn::kUnpack4XU8:
                        call_worklist.push_back([this, call] { Unpack4xU8(call); });
                        break;
                    case core::BuiltinFn::kAddSat:
                        call_worklist.push_back([this, call] { AddSat(call); });
                        break;
                    default:
                        break;
                }
                continue;
            }
        }

        // Replace the bitcasts that we found.
        for (auto* bitcast : bitcast_worklist) {
            auto* src_type = bitcast->Args()[0]->Type();
            auto* dst_type = bitcast->Result()->Type();
            auto* dst_deepest = dst_type->DeepestElement();
            auto* src_deepest = src_type->DeepestElement();

            if (src_type == dst_type) {
                ReplaceBitcastWithValue(bitcast);
            } else if (src_deepest->Size() == 2 && dst_deepest->Size() == 2) {
                // Same-width 16-bit bitcast (e.g. f16 <-> u16), scalar or vector.
                // Must be checked before the f16 vector cases below.
                Replace16BitBitcastWith16BitConstruct(bitcast);
            } else if (src_deepest->Size() == 2 && src_type->Is<core::type::Vector>()) {
                // 16-bit vector to 32-bit bitcast (e.g., vec2<f16> -> u32 or vec2<u16> -> u32)
                ReplaceBitcastWithFrom16BitPolyfill(bitcast);
            } else if (dst_deepest->Size() == 2 && dst_type->Is<core::type::Vector>()) {
                // 32-bit to 16-bit vector bitcast (e.g., u32 -> vec2<f16> or u32 -> vec2<u16>)
                ReplaceBitcastWithTo16BitPolyfill(bitcast);
            } else if (src_deepest->Size() == 4 && dst_deepest->Size() == 8) {
                // 32-bit vec2 to 64-bit bitcast (e.g. vec2<u32> -> u64)
                ReplaceBitcastWithTo64BitPolyfill(bitcast);
            } else {
                ReplaceBitcastWithAs(bitcast);
            }
        }

        // Replace the builtin calls that we found
        for (auto& cb : call_worklist) {
            cb();
        }
    }

    void Acosh(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        auto* result_ty = call->Result()->Type();

        // log(x + sqrt(x*x - 1));
        b.InsertBefore(call, [&] {
            bool is_f16 = result_ty->DeepestElement()->Is<core::type::F16>();

            auto* one =
                b.MatchWidth(is_f16 ? b.ConstantValue(1_h) : b.ConstantValue(1_f), result_ty);
            auto* mul = b.Multiply(args[0], args[0]);
            auto* sub = b.Subtract(mul, one);
            auto* sqrt = b.Call(result_ty, core::BuiltinFn::kSqrt, sub);
            auto* add = b.Add(args[0], sqrt);
            b.CallWithResult(call->DetachResult(), core::BuiltinFn::kLog, add);
        });
        call->Destroy();
    }

    void Asinh(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        auto* result_ty = call->Result()->Type();

        // log(x + sqrt(x*x + 1));
        b.InsertBefore(call, [&] {
            bool is_f16 = result_ty->DeepestElement()->Is<core::type::F16>();

            auto* one =
                b.MatchWidth(is_f16 ? b.ConstantValue(1_h) : b.ConstantValue(1_f), result_ty);
            auto* mul = b.Multiply(args[0], args[0]);
            auto* add_one = b.Add(mul, one);
            auto* sqrt = b.Call(result_ty, core::BuiltinFn::kSqrt, add_one);
            auto* add = b.Add(args[0], sqrt);
            b.CallWithResult(call->DetachResult(), core::BuiltinFn::kLog, add);
        });
        call->Destroy();
    }

    void Atanh(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        auto* result_ty = call->Result()->Type();

        // log((1+x) / (1-x)) * 0.5
        b.InsertBefore(call, [&] {
            //
            bool is_f16 = result_ty->DeepestElement()->Is<core::type::F16>();

            auto* one =
                b.MatchWidth(is_f16 ? b.ConstantValue(1_h) : b.ConstantValue(1_f), result_ty);
            auto* half =
                b.MatchWidth(is_f16 ? b.ConstantValue(0.5_h) : b.ConstantValue(0.5_f), result_ty);
            auto* one_plus_x = b.Add(one, args[0]);
            auto* one_minus_x = b.Subtract(one, args[0]);
            auto* div = b.Divide(one_plus_x, one_minus_x);
            auto* log = b.Call(result_ty, core::BuiltinFn::kLog, div);
            auto* mul = b.Multiply(log, half);

            call->Result()->ReplaceAllUsesWith(mul->Result());
        });
        call->Destroy();
    }

    void Interlocked(core::ir::CoreBuiltinCall* call, BuiltinFn fn) {
        auto args = call->Args();
        auto* type = args[1]->Type();

        b.InsertBefore(call, [&] {
            auto* original_value = b.Var(ty.ptr(function, type));
            original_value->SetInitializer(b.Zero(type));

            b.Call<hlsl::ir::BuiltinCall>(ty.void_(), fn, args[0], args[1], original_value);
            b.LoadWithResult(call->DetachResult(), original_value)->Result();
        });
        call->Destroy();
    }

    void AtomicAnd(core::ir::CoreBuiltinCall* call) {
        Interlocked(call, BuiltinFn::kInterlockedAnd);
    }

    void AtomicOr(core::ir::CoreBuiltinCall* call) { Interlocked(call, BuiltinFn::kInterlockedOr); }

    void AtomicXor(core::ir::CoreBuiltinCall* call) {
        Interlocked(call, BuiltinFn::kInterlockedXor);
    }

    void AtomicMin(core::ir::CoreBuiltinCall* call) {
        Interlocked(call, BuiltinFn::kInterlockedMin);
    }

    void AtomicMax(core::ir::CoreBuiltinCall* call) {
        Interlocked(call, BuiltinFn::kInterlockedMax);
    }

    void AtomicAdd(core::ir::CoreBuiltinCall* call) {
        Interlocked(call, BuiltinFn::kInterlockedAdd);
    }

    void AtomicExchange(core::ir::CoreBuiltinCall* call) {
        Interlocked(call, BuiltinFn::kInterlockedExchange);
    }

    // An atomic sub is a negated atomic add
    void AtomicSub(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        auto* type = args[1]->Type();

        b.InsertBefore(call, [&] {
            auto* original_value = b.Var(ty.ptr(function, type));
            original_value->SetInitializer(b.Zero(type));
            auto* val = b.Subtract(b.Zero(type), args[1]);
            b.Call<hlsl::ir::BuiltinCall>(ty.void_(), BuiltinFn::kInterlockedAdd, args[0], val,
                                          original_value);
            b.LoadWithResult(call->DetachResult(), original_value)->Result();
        });
        call->Destroy();
    }

    void AtomicCompareExchangeWeak(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        auto* type = args[1]->Type();
        b.InsertBefore(call, [&] {
            auto* original_value = b.Var(ty.ptr(function, type));
            original_value->SetInitializer(b.Zero(type));

            auto* cmp = args[1];
            b.Call<hlsl::ir::BuiltinCall>(ty.void_(), BuiltinFn::kInterlockedCompareExchange,
                                          args[0], cmp, args[2], original_value);

            auto* o = b.Load(original_value);
            b.ConstructWithResult(call->DetachResult(), o, b.Equal(o, cmp));
        });
        call->Destroy();
    }

    // An atomic load is an Or with 0
    void AtomicLoad(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        auto* type = call->Result()->Type();
        b.InsertBefore(call, [&] {
            auto* original_value = b.Var(ty.ptr(function, type));
            original_value->SetInitializer(b.Zero(type));

            b.Call<hlsl::ir::BuiltinCall>(ty.void_(), BuiltinFn::kInterlockedOr, args[0],
                                          b.Zero(type), original_value);
            b.LoadWithResult(call->DetachResult(), original_value)->Result();
        });
        call->Destroy();
    }

    void AtomicStore(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        auto* type = args[1]->Type();

        b.InsertBefore(call, [&] {
            auto* original_value = b.Var(ty.ptr(function, type));
            original_value->SetInitializer(b.Zero(type));

            b.Call<hlsl::ir::BuiltinCall>(ty.void_(), BuiltinFn::kInterlockedExchange, args[0],
                                          args[1], original_value);
        });
        call->Destroy();
    }

    void Select(core::ir::CoreBuiltinCall* call) {
        auto args = Vector<core::ir::Value*, 4>{call->Args()};
        if (config.use_hlsl_2021_select) {
            // HLSL's argument order for select is (condition, trueval, falseval), which is the
            // opposite of WGSL/Tint.
            auto* hlsl_select = b.CallWithResult<hlsl::ir::BuiltinCall>(
                call->DetachResult(), hlsl::BuiltinFn::kSelect, args[2], args[1], args[0]);
            hlsl_select->InsertBefore(call);
        } else {
            auto* ternary = b.ir.CreateInstruction<hlsl::ir::Ternary>(call->DetachResult(), args);
            ternary->InsertBefore(call);
        }
        call->Destroy();
    }

    // FXC's trunc is broken for very large/small float values.
    // See crbug.com/tint/1883
    //
    // Replace with:
    //   value < 0 ? ceil(value) : floor(value)
    void Trunc(core::ir::CoreBuiltinCall* call) {
        auto* val = call->Args()[0];

        auto* type = call->Result()->Type();
        Vector<core::ir::Value*, 4> args;
        b.InsertBefore(call, [&] {
            args.Push(b.Call(type, core::BuiltinFn::kFloor, val)->Result());
            args.Push(b.Call(type, core::BuiltinFn::kCeil, val)->Result());
            args.Push(b.LessThan(val, b.Zero(type))->Result());
        });
        auto* trunc = b.ir.CreateInstruction<hlsl::ir::Ternary>(call->DetachResult(), args);
        trunc->InsertBefore(call);

        call->Destroy();
    }

    /// Replaces an identity bitcast result with the value.
    void ReplaceBitcastWithValue(core::ir::CoreBuiltinCall* bitcast) {
        bitcast->Result()->ReplaceAllUsesWith(bitcast->Args()[0]);
        bitcast->Destroy();
    }

    /// Replaces a bitcast with a 16-bit as function for f16 <-> u16 conversions (scalar and vec).
    /// Uses asuint16 (f16 -> u16) or asfloat16 (u16 -> f16).
    void Replace16BitBitcastWith16BitConstruct(core::ir::CoreBuiltinCall* bitcast) {
        auto* dst_type = bitcast->Result()->Type();
        auto* dst_deepest = dst_type->DeepestElement();

        BuiltinFn fn = BuiltinFn::kNone;
        if (dst_deepest->Is<core::type::U16>()) {
            fn = BuiltinFn::kAsuint16;
        } else if (dst_deepest->Is<core::type::F16>()) {
            fn = BuiltinFn::kAsfloat16;
        } else {
            TINT_IR_ICE(ir) << "unexpected 16-bit bitcast destination type: "
                            << dst_deepest->FriendlyName();
        }

        b.InsertBefore(bitcast, [&] {
            b.CallWithResult<hlsl::ir::BuiltinCall>(bitcast->DetachResult(), fn,
                                                    bitcast->Args()[0]);
        });
        bitcast->Destroy();
    }

    void ReplaceBitcastWithAs(core::ir::CoreBuiltinCall* bitcast) {
        auto* dst_type = bitcast->Result()->Type();
        auto* dst_deepest = dst_type->DeepestElement();

        BuiltinFn fn = BuiltinFn::kNone;
        tint::Switch(
            dst_deepest,                                                //
            [&](const core::type::I32*) { fn = BuiltinFn::kAsint; },    //
            [&](const core::type::U32*) { fn = BuiltinFn::kAsuint; },   //
            [&](const core::type::F32*) { fn = BuiltinFn::kAsfloat; },  //
            TINT_ICE_ON_NO_MATCH);

        // TODO(crbug.com/361794783): work around DXC failing on 'as' casts of constant integral
        // splats by wrapping it with an explicit vector constructor.
        // e.g. asuint(123.xx) -> asuint(int2(123.xx)))
        bool castToSrcType = false;
        auto* src_type = bitcast->Args()[0]->Type();
        if (src_type->IsIntegerVector()) {
            if (auto* c = bitcast->Args()[0]->As<core::ir::Constant>()) {
                castToSrcType = c->Value()->Is<core::constant::Splat>();
            }
        }

        b.InsertBefore(bitcast, [&] {
            auto* source = bitcast->Args()[0];
            if (castToSrcType) {
                source = b.Construct(src_type, source)->Result();
            }
            b.CallWithResult<hlsl::ir::BuiltinCall>(bitcast->DetachResult(), fn, source);
        });
        bitcast->Destroy();
    }

    // Bitcast 16-bit vector types (f16 or u16) to 32-bit types by bitcasting via u16.
    core::ir::Function* CreateBitcastFrom16Bit(const core::type::Type* src_type,
                                               const core::type::Type* dst_type) {
        return bitcast_funcs_.GetOrAdd(
            BitcastType{{src_type, dst_type}}, [&]() -> core::ir::Function* {
                TINT_IR_ASSERT(ir, src_type->Is<core::type::Vector>());

                ir.properties.Add(core::ir::Property::kAllow16BitIntegers);
                auto* src_vec = src_type->As<core::type::Vector>();
                bool src_is_u16 = src_vec->Type()->Is<core::type::U16>();
                auto* u16_vec_ty = src_is_u16 ? src_type : ty.MatchWidth(ty.u16(), src_vec);

                // Generates a helper that packs a 16-bit vector into a 32-bit scalar.
                // For a vec2<u16> source, the emitted HLSL looks like:
                //
                //   uint tint_bitcast_from_u16(vector<uint16_t, 2> src) {
                //     uint2 r_uint = (uint2(src) & 65535u.xx) << uint2(0u, 16u);
                //     return r_uint.x | r_uint.y;
                //   }
                //
                // For a vec2<f16> source, asuint16 is applied first:
                //
                //   uint tint_bitcast_from_f16(vector<float16_t, 2> src) {
                //     vector<uint16_t, 2> r = asuint16(src);
                //     uint2 r_uint = (uint2(r) & 65535u.xx) << uint2(0u, 16u);
                //     return r_uint.x | r_uint.y;
                //   }

                auto fn_name =
                    b.ir.symbols.New(src_is_u16 ? "tint_bitcast_from_u16" : "tint_bitcast_from_f16")
                        .Name();

                auto* f = b.Function(fn_name, dst_type);
                auto* src = b.FunctionParam("src", src_type);
                f->SetParams({src});

                b.Append(f->Block(), [&] {
                    // If source is f16, first reinterpret as u16 via asuint16
                    core::ir::Value* u16_src = src;
                    if (!src_is_u16) {
                        u16_src =
                            b.Call<hlsl::ir::BuiltinCall>(u16_vec_ty, BuiltinFn::kAsuint16, src)
                                ->Result();
                    }

                    auto* uint_src_ty = ty.MatchWidth(ty.u32(), src_type);
                    core::ir::Instruction* v = b.Convert(uint_src_ty, u16_src);

                    auto width = src_vec->Width();
                    v = b.And(v, b.Splat(uint_src_ty, 0xffff_u));
                    if (width == 2) {
                        v = b.ShiftLeft(v, b.Construct(uint_src_ty, 0_u, 16_u));
                        auto* a1 = b.Access(ty.u32(), v, 0_u);
                        v = b.Or(a1, b.Access(ty.u32(), v, 1_u));
                    } else {
                        v = b.ShiftLeft(v, b.Construct(uint_src_ty, 0_u, 16_u, 0_u, 16_u));
                        auto* a1 = b.Access(ty.u32(), v, 0_u);
                        auto* v1 = b.Or(a1, b.Access(ty.u32(), v, 1_u));
                        auto* a2 = b.Access(ty.u32(), v, 2_u);
                        auto* v2 = b.Or(a2, b.Access(ty.u32(), v, 3_u));
                        v = b.Construct(ty.vec2(ty.u32()), v1, v2);
                    }
                    if (dst_type->DeepestElement()->Is<core::type::F32>()) {
                        v = b.Call<hlsl::ir::BuiltinCall>(dst_type, BuiltinFn::kAsfloat, v);
                    } else if (dst_type->DeepestElement()->Is<core::type::I32>()) {
                        v = b.Call<hlsl::ir::BuiltinCall>(dst_type, BuiltinFn::kAsint, v);
                    }
                    b.Return(f, v);
                });
                return f;
            });
    }

    /// Replaces a bitcast with a call to the From16Bit polyfill for the given types
    void ReplaceBitcastWithFrom16BitPolyfill(core::ir::CoreBuiltinCall* bitcast) {
        auto* src_type = bitcast->Args()[0]->Type();
        auto* dst_type = bitcast->Result()->Type();

        auto* f = CreateBitcastFrom16Bit(src_type, dst_type);
        b.InsertBefore(bitcast,
                       [&] { b.CallWithResult(bitcast->DetachResult(), f, bitcast->Args()[0]); });
        bitcast->Destroy();
    }

    // Bitcast 32-bit types to 16-bit vector types (f16 or u16) by reinterpreting
    // their bits as u16.
    core::ir::Function* CreateBitcastTo16Bit(const core::type::Type* src_type,
                                             const core::type::Type* dst_type) {
        return bitcast_funcs_.GetOrAdd(
            BitcastType{{src_type, dst_type}}, [&]() -> core::ir::Function* {
                TINT_IR_ASSERT(ir, dst_type->Is<core::type::Vector>());

                ir.properties.Add(core::ir::Property::kAllow16BitIntegers);
                auto* dst_vec = dst_type->As<core::type::Vector>();
                bool dst_is_u16 = dst_vec->Type()->Is<core::type::U16>();
                auto* u16_vec_ty = dst_is_u16 ? dst_type : ty.MatchWidth(ty.u16(), dst_vec);

                // Generates a helper that converts via 16-bit integers.
                // For a vec2<u16> destination, the emitted HLSL looks like:
                //
                //   vector<uint16_t, 2> tint_bitcast_to_u16(float src) {
                //     uint v = asuint(src);
                //     uint2 v2 = v.xx;
                //     vector<uint16_t, 2> r =
                //       vector<uint16_t, 2>((v2 >> uint2(0u, 16u)) & (65535u).xx);
                //     return r;
                //   }
                //
                // For a vec2<f16> destination, asfloat16 is applied to the final u16 result:
                //
                //   vector<float16_t, 2> tint_bitcast_to_f16(float src) {
                //     uint v = asuint(src);
                //     uint2 v2 = v.xx;
                //     vector<uint16_t, 2> r =
                //       vector<uint16_t, 2>((v2 >> uint2(0u, 16u)) & (65535u).xx);
                //     return asfloat16(r);
                //   }

                auto fn_name =
                    b.ir.symbols.New(dst_is_u16 ? "tint_bitcast_to_u16" : "tint_bitcast_to_f16")
                        .Name();

                auto* f = b.Function(fn_name, dst_type);
                auto* src = b.FunctionParam("src", src_type);
                f->SetParams({src});
                b.Append(f->Block(), [&] {
                    const core::type::Type* uint_ty = ty.MatchWidth(ty.u32(), src_type);

                    core::ir::Instruction* v = nullptr;
                    tint::Switch(
                        src_type->DeepestElement(),                            //
                        [&](const core::type::U32*) { v = b.Let("v", src); },  //
                        [&](const core::type::I32*) {
                            v = b.Let("v", b.Call<hlsl::ir::BuiltinCall>(uint_ty,
                                                                         BuiltinFn::kAsuint, src));
                        },
                        [&](const core::type::F32*) {
                            v = b.Let("v", b.Call<hlsl::ir::BuiltinCall>(uint_ty,
                                                                         BuiltinFn::kAsuint, src));
                        },
                        TINT_ICE_ON_NO_MATCH);

                    bool src_vec = src_type->Is<core::type::Vector>();
                    const core::type::Type* uint_dst_ty = ty.MatchWidth(ty.u32(), dst_type);

                    // v is now the uint bitcast of src
                    core::ir::Instruction* shifted = nullptr;
                    core::ir::Instruction* masked = nullptr;
                    // Make a double wide src and then shift and mask the result to produce u16
                    // values.
                    if (src_vec) {
                        // Since the src was vec2, we swizzle so the value is
                        // equivalent to: src.xxyy
                        auto* swizzle = b.Swizzle(uint_dst_ty, v, {0_u, 0_u, 1_u, 1_u});
                        shifted = b.ShiftRight(
                            swizzle, b.Construct(uint_dst_ty, 0_u, 16_u, 0_u, 16_u)->Result());
                    } else {
                        v = b.Construct(uint_dst_ty, v, v);
                        shifted = b.ShiftRight(v, b.Construct(uint_dst_ty, 0_u, 16_u)->Result());
                    }
                    masked = b.And(shifted, b.Splat(uint_dst_ty, 0xffff_u));
                    core::ir::Instruction* v16 = b.Let("v16", b.Convert(u16_vec_ty, masked));
                    if (!dst_is_u16) {
                        v16 = b.Call<hlsl::ir::BuiltinCall>(dst_type, BuiltinFn::kAsfloat16, v16);
                    }
                    b.Return(f, v16);
                });
                return f;
            });
    }

    // The HLSL `sign` method always returns an `int` result (scalar or vector). In WGSL the result
    // is expected to be the same type as the argument. This injects a cast to the expected WGSL
    // result type after the call to `hlsl.sign`.
    core::ir::Instruction* BuildSign(core::ir::Value* value) {
        const auto* result_ty = ty.MatchWidth(ty.i32(), value->Type());
        core::ir::Instruction* sign =
            b.Call<hlsl::ir::BuiltinCall>(result_ty, hlsl::BuiltinFn::kSign, value);
        if (sign->Result()->Type() != value->Type()) {
            sign = b.Convert(value->Type(), sign);
        }
        return sign;
    }

    void Sign(core::ir::BuiltinCall* call) {
        b.InsertBefore(call, [&] {
            auto* sign = BuildSign(call->Args()[0]);
            sign->SetResult(call->DetachResult());
        });
        call->Destroy();
    }

    /// Replaces a bitcast with a call to the To16Bit polyfill for the given types
    void ReplaceBitcastWithTo16BitPolyfill(core::ir::CoreBuiltinCall* bitcast) {
        auto* src_type = bitcast->Args()[0]->Type();
        auto* dst_type = bitcast->Result()->Type();

        auto* f = CreateBitcastTo16Bit(src_type, dst_type);
        b.InsertBefore(bitcast,
                       [&] { b.CallWithResult(bitcast->DetachResult(), f, bitcast->Args()[0]); });
        bitcast->Destroy();
    }

    void ReplaceBitcastWithTo64BitPolyfill(core::ir::CoreBuiltinCall* bitcast) {
        auto* src = bitcast->Args()[0];
        auto* dst_type = bitcast->Result()->Type();
        TINT_IR_ASSERT(ir, dst_type->Is<core::type::U64>());
        TINT_IR_ASSERT(ir, src->Type()->Is<core::type::Vector>());

        b.InsertBefore(bitcast, [&] {
            // Bitcast the source to vec2<u32> to ensure we can swizzle it correctly.
            auto* low = b.Swizzle(ty.u32(), src, {0u});
            auto* high = b.Swizzle(ty.u32(), src, {1u});
            auto* low64 = b.Convert(ty.u64(), low);
            auto* high64 = b.Convert(ty.u64(), high);
            auto* shifted_high = b.ShiftLeft(high64, b.Convert(ty.u64(), b.Constant(u32(32))));
            auto* result = b.Or(shifted_high, low64);

            bitcast->Result()->ReplaceAllUsesWith(result->Result());
        });
        bitcast->Destroy();
    }

    void TextureNumLayers(core::ir::CoreBuiltinCall* call) {
        auto* tex = call->Args()[0];
        auto* tex_type = tex->Type()->As<core::type::Texture>();

        TINT_IR_ASSERT(ir, tex_type->Dim() == core::type::TextureDimension::k2dArray ||
                               tex_type->Dim() == core::type::TextureDimension::kCubeArray);

        const core::type::Type* query_ty = ty.vec(ty.u32(), 3);
        b.InsertBefore(call, [&] {
            core::ir::Instruction* out = b.Var(ty.ptr(function, query_ty));

            b.MemberCall<hlsl::ir::MemberBuiltinCall>(
                ty.void_(), hlsl::BuiltinFn::kGetDimensions, tex,
                Vector<core::ir::Value*, 3>{b.Access(ty.ptr<function, u32>(), out, 0_u)->Result(),
                                            b.Access(ty.ptr<function, u32>(), out, 1_u)->Result(),
                                            b.Access(ty.ptr<function, u32>(), out, 2_u)->Result()});

            out = b.Swizzle(ty.u32(), b.Load(out), {2_u});
            call->Result()->ReplaceAllUsesWith(out->Result());
        });
        call->Destroy();
    }

    void TextureNumLevels(core::ir::CoreBuiltinCall* call) {
        auto* tex = call->Args()[0];
        auto* tex_type = tex->Type()->As<core::type::Texture>();

        Vector<uint32_t, 2> swizzle{};
        uint32_t query_size = 0;
        switch (tex_type->Dim()) {
            case core::type::TextureDimension::kNone:
                TINT_IR_ICE(ir) << "texture dimension is kNone";
            case core::type::TextureDimension::k1d:
                query_size = 2;
                swizzle = {1_u};
                break;
            case core::type::TextureDimension::k2d:
            case core::type::TextureDimension::kCube:
                query_size = 3;
                swizzle = {2_u};
                break;
            case core::type::TextureDimension::k2dArray:
            case core::type::TextureDimension::k3d:
            case core::type::TextureDimension::kCubeArray:
                query_size = 4;
                swizzle = {3_u};
                break;
        }

        const core::type::Type* query_ty = ty.vec(ty.u32(), query_size);
        b.InsertBefore(call, [&] {
            Vector<core::ir::Value*, 5> args;
            // Pass the `level` parameter so the `num_levels` overload is used.
            args.Push(b.Value(0_u));

            core::ir::Instruction* out = b.Var(ty.ptr(function, query_ty));
            for (uint32_t i = 0; i < query_size; ++i) {
                args.Push(b.Access(ty.ptr<function, u32>(), out, u32(i))->Result());
            }

            b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), hlsl::BuiltinFn::kGetDimensions,
                                                      tex, args);

            out = b.Swizzle(ty.u32(), b.Load(out), swizzle);
            call->Result()->ReplaceAllUsesWith(out->Result());
        });
        call->Destroy();
    }

    void TextureDimensions(core::ir::CoreBuiltinCall* call) {
        auto* tex = call->Args()[0];
        auto* tex_type = tex->Type()->As<core::type::Texture>();
        bool has_level = call->Args().size() > 1;
        bool is_ms =
            tex_type
                ->IsAnyOf<core::type::MultisampledTexture, core::type::DepthMultisampledTexture>();

        Vector<uint32_t, 2> swizzle{};
        uint32_t query_size = 0;
        switch (tex_type->Dim()) {
            case core::type::TextureDimension::kNone:
                TINT_IR_ICE(ir) << "texture dimension is kNone";
            case core::type::TextureDimension::k1d:
                query_size = 1;
                break;
            case core::type::TextureDimension::k2d:
                if (is_ms) {
                    query_size = 3;
                    swizzle = {0_u, 1_u};
                } else {
                    query_size = 2;
                }
                break;
            case core::type::TextureDimension::k2dArray:
                query_size = is_ms ? 4 : 3;
                swizzle = {0_u, 1_u};
                break;
            case core::type::TextureDimension::k3d:
                query_size = 3;
                break;
            case core::type::TextureDimension::kCube:
                query_size = 2;
                break;
            case core::type::TextureDimension::kCubeArray:
                query_size = 3;
                swizzle = {0_u, 1_u};
                break;
        }

        // Query with a `level` adds a `number_of_levels` output parameter
        if (has_level) {
            // If there was no swizzle, we will need to swizzle out the required query parameters as
            // the query will increase by one item.
            if (swizzle.IsEmpty()) {
                for (uint32_t i = 0; i < query_size; ++i) {
                    swizzle.Push(i);
                }
            }
            query_size += 1;
        }

        auto* query_ty = ty.MatchWidth(ty.u32(), query_size);

        b.InsertBefore(call, [&] {
            Vector<core::ir::Value*, 5> args;

            // Push the level if needed
            if (has_level) {
                args.Push(b.InsertConvertIfNeeded(ty.u32(), call->Args()[1]));
            }

            core::ir::Instruction* query = b.Var(ty.ptr(function, query_ty));
            if (query_size == 1) {
                args.Push(query->Result());
            } else {
                for (uint32_t i = 0; i < query_size; ++i) {
                    args.Push(b.Access(ty.ptr<function, u32>(), query, u32(i))->Result());
                }
            }

            b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), hlsl::BuiltinFn::kGetDimensions,
                                                      tex, args);
            query = b.Load(query);
            if (!swizzle.IsEmpty()) {
                query = b.Swizzle(ty.MatchWidth(ty.u32(), swizzle.Length()), query, swizzle);
            }
            call->Result()->ReplaceAllUsesWith(query->Result());
        });
        call->Destroy();
    }

    void TextureNumSamples(core::ir::CoreBuiltinCall* call) {
        auto* tex = call->Args()[0];
        auto* tex_type = tex->Type()->As<core::type::Texture>();

        TINT_IR_ASSERT(ir, tex_type->Dim() == core::type::TextureDimension::k2d);
        TINT_IR_ASSERT(ir, (tex_type->IsAnyOf<core::type::DepthMultisampledTexture,
                                              core::type::MultisampledTexture>()));

        const core::type::Type* query_ty = ty.vec(ty.u32(), 3);
        b.InsertBefore(call, [&] {
            core::ir::Instruction* out = b.Var(ty.ptr(function, query_ty));

            b.MemberCall<hlsl::ir::MemberBuiltinCall>(
                ty.void_(), hlsl::BuiltinFn::kGetDimensions, tex,
                Vector<core::ir::Value*, 3>{b.Access(ty.ptr<function, u32>(), out, 0_u)->Result(),
                                            b.Access(ty.ptr<function, u32>(), out, 1_u)->Result(),
                                            b.Access(ty.ptr<function, u32>(), out, 2_u)->Result()});

            out = b.Swizzle(ty.u32(), b.Load(out), {2_u});
            call->Result()->ReplaceAllUsesWith(out->Result());
        });
        call->Destroy();
    }

    void TextureLoad(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        auto* tex = args[0];
        auto* tex_type = tex->Type()->As<core::type::Texture>();

        Vector<uint32_t, 2> swizzle;
        const core::type::Type* ret_ty = tint::Switch(
            tex_type,  //
            [&](const core::type::SampledTexture* sampled) { return sampled->Type(); },
            [&](const core::type::StorageTexture* storage) { return storage->Type(); },
            [&](const core::type::MultisampledTexture* ms) { return ms->Type(); },
            [&](const core::type::DepthTexture*) {
                swizzle.Push(0u);
                return ty.f32();
            },
            [&](const core::type::DepthMultisampledTexture*) {
                swizzle.Push(0);
                return ty.f32();
            },
            TINT_ICE_ON_NO_MATCH);

        bool is_ms = tex_type->Is<core::type::MultisampledTexture>() ||
                     tex_type->Is<core::type::DepthMultisampledTexture>();
        bool is_storage = tex_type->Is<core::type::StorageTexture>();
        b.InsertBefore(call, [&] {
            Vector<core::ir::Value*, 2> call_args;
            switch (tex_type->Dim()) {
                case core::type::TextureDimension::k1d: {
                    auto* coord = b.InsertConvertIfNeeded(ty.i32(), args[1]);
                    core::ir::Value* lvl = nullptr;
                    if (is_storage) {
                        lvl = b.Constant(0_i);
                    } else {
                        lvl = b.InsertConvertIfNeeded(ty.i32(), args[2]);
                    }
                    call_args.Push(b.Construct(ty.vec2i(), coord, lvl)->Result());
                    break;
                }
                case core::type::TextureDimension::k2d: {
                    auto* coord = b.InsertConvertIfNeeded(ty.vec2i(), args[1]);
                    if (is_ms) {
                        // Pass coords and sample index as separate parameters
                        call_args.Push(coord);
                        call_args.Push(b.InsertConvertIfNeeded(ty.i32(), args[2]));
                    } else {
                        core::ir::Value* lvl = nullptr;
                        if (is_storage) {
                            lvl = b.Constant(0_i);
                        } else {
                            lvl = b.InsertConvertIfNeeded(ty.i32(), args[2]);
                        }
                        call_args.Push(b.Construct(ty.vec3i(), coord, lvl)->Result());
                    }
                    break;
                }
                case core::type::TextureDimension::k2dArray: {
                    auto* coord = b.InsertConvertIfNeeded(ty.vec2i(), args[1]);
                    auto* ary_idx = b.InsertConvertIfNeeded(ty.i32(), args[2]);
                    core::ir::Value* lvl = nullptr;
                    if (is_storage) {
                        lvl = b.Constant(0_i);
                    } else {
                        lvl = b.InsertConvertIfNeeded(ty.i32(), args[3]);
                    }
                    call_args.Push(b.Construct(ty.vec4i(), coord, ary_idx, lvl)->Result());
                    break;
                }
                case core::type::TextureDimension::k3d: {
                    auto* coord = b.InsertConvertIfNeeded(ty.vec3i(), args[1]);
                    core::ir::Value* lvl = nullptr;
                    if (is_storage) {
                        lvl = b.Constant(0_i);
                    } else {
                        lvl = b.InsertConvertIfNeeded(ty.i32(), args[2]);
                    }
                    call_args.Push(b.Construct(ty.vec4i(), coord, lvl)->Result());
                    break;
                }
                default:
                    TINT_IR_UNREACHABLE(ir);
            }

            auto* member_call = b.MemberCall<hlsl::ir::MemberBuiltinCall>(
                ty.vec4(ret_ty), hlsl::BuiltinFn::kLoad, tex, call_args);

            core::ir::Instruction* builtin = member_call;
            if (!swizzle.IsEmpty()) {
                builtin = b.Swizzle(ty.f32(), builtin, swizzle);
            } else {
                if (builtin->Result()->Type() != call->Result()->Type()) {
                    builtin = b.Convert(call->Result()->Type(), builtin);
                }
            }
            call->Result()->ReplaceAllUsesWith(builtin->Result());
        });

        call->Destroy();
    }

    // Just re-write the arguments so we have the needed arrays, and then the printer will turn it
    // into the correct assignment instruction.
    void TextureStore(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        auto* tex = args[0];
        auto* tex_type = tex->Type()->As<core::type::StorageTexture>();
        TINT_IR_ASSERT(ir, tex_type);

        Vector<core::ir::Value*, 3> new_args;
        new_args.Push(tex);

        b.InsertBefore(call, [&] {
            if (tex_type->Dim() == core::type::TextureDimension::k2dArray) {
                auto* coords = args[1];
                auto* array_idx = args[2];

                auto* coords_ty = coords->Type()->As<core::type::Vector>();
                TINT_IR_ASSERT(ir, coords_ty);

                auto* new_coords =
                    b.Construct(ty.vec3(coords_ty->Type()), coords,
                                b.InsertConvertIfNeeded(coords_ty->Type(), array_idx));
                new_args.Push(new_coords->Result());

                new_args.Push(args[3]);
            } else {
                new_args.Push(args[1]);
                new_args.Push(args[2]);
            }

            b.CallWithResult<hlsl::ir::BuiltinCall>(call->DetachResult(),
                                                    hlsl::BuiltinFn::kTextureStore, new_args);
        });
        call->Destroy();
    }

    void TextureGather(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            core::ir::Value* tex = nullptr;
            hlsl::BuiltinFn fn = hlsl::BuiltinFn::kGather;

            core::ir::Value* coords = nullptr;

            Vector<core::ir::Value*, 4> params;

            uint32_t idx = 0;
            if (!args[idx]->Type()->Is<core::type::Texture>()) {
                auto* comp = args[idx++]->As<core::ir::Constant>();
                TINT_IR_ASSERT(ir, comp);

                switch (comp->Value()->ValueAs<int32_t>()) {
                    case 0:
                        fn = hlsl::BuiltinFn::kGatherRed;
                        break;
                    case 1:
                        fn = hlsl::BuiltinFn::kGatherGreen;
                        break;
                    case 2:
                        fn = hlsl::BuiltinFn::kGatherBlue;
                        break;
                    case 3:
                        fn = hlsl::BuiltinFn::kGatherAlpha;
                        break;
                    default:
                        TINT_IR_UNREACHABLE(ir);
                }
            }

            tex = args[idx++];

            auto* tex_type = tex->Type()->As<core::type::Texture>();
            TINT_IR_ASSERT(ir, tex_type);

            bool is_depth = tex_type->Is<core::type::DepthTexture>();

            params.Push(args[idx++]);  // sampler
            coords = args[idx++];

            uint32_t offset_idx = 0;

            switch (tex_type->Dim()) {
                case core::type::TextureDimension::k2d:
                    params.Push(coords);
                    offset_idx = is_depth ? 3 : 4;
                    break;
                case core::type::TextureDimension::k2dArray:
                    params.Push(
                        b.Construct(ty.vec3f(), coords, b.Convert<f32>(args[idx++]))->Result());
                    offset_idx = is_depth ? 4 : 5;
                    break;
                case core::type::TextureDimension::kCube:
                    params.Push(coords);
                    break;
                case core::type::TextureDimension::kCubeArray:
                    params.Push(
                        b.Construct(ty.vec4f(), coords, b.Convert<f32>(args[idx++]))->Result());
                    break;
                default:
                    TINT_IR_UNREACHABLE(ir);
            }
            if (offset_idx > 0 && args.size() > offset_idx) {
                params.Push(args[offset_idx]);
            }

            b.MemberCallWithResult<hlsl::ir::MemberBuiltinCall>(call->DetachResult(), fn, tex,
                                                                params);
        });
        call->Destroy();
    }

    void TextureGatherCompare(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* tex = args[0];

            Vector<core::ir::Value*, 4> params;
            params.Push(args[1]);

            auto* coords = args[2];

            auto* tex_type = tex->Type()->As<core::type::Texture>();
            TINT_IR_ASSERT(ir, tex_type);

            switch (tex_type->Dim()) {
                case core::type::TextureDimension::k2d:
                    params.Push(coords);
                    params.Push(args[3]);

                    if (args.size() > 4) {
                        params.Push(args[4]);
                    }
                    break;
                case core::type::TextureDimension::k2dArray:
                    params.Push(b.Construct(ty.vec3f(), coords, b.Convert<f32>(args[3]))->Result());
                    params.Push(args[4]);
                    if (args.size() > 5) {
                        params.Push(args[5]);
                    }
                    break;
                case core::type::TextureDimension::kCube:
                    params.Push(coords);
                    params.Push(args[3]);
                    break;
                case core::type::TextureDimension::kCubeArray:
                    params.Push(b.Construct(ty.vec4f(), coords, b.Convert<f32>(args[3]))->Result());
                    params.Push(args[4]);
                    break;
                default:
                    TINT_IR_UNREACHABLE(ir);
            }

            b.MemberCallWithResult<hlsl::ir::MemberBuiltinCall>(
                call->DetachResult(), hlsl::BuiltinFn::kGatherCmp, tex, params);
        });
        call->Destroy();
    }

    void TextureSample(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            core::ir::Value* tex = args[0];

            Vector<core::ir::Value*, 4> params;

            auto* tex_type = tex->Type()->As<core::type::Texture>();
            TINT_IR_ASSERT(ir, tex_type);

            params.Push(args[1]);  // sampler
            core::ir::Value* coords = args[2];

            switch (tex_type->Dim()) {
                case core::type::TextureDimension::k1d:
                case core::type::TextureDimension::k2d:
                    params.Push(coords);

                    if (args.size() > 3) {
                        params.Push(args[3]);
                    }
                    break;
                case core::type::TextureDimension::k2dArray:
                    params.Push(b.Construct(ty.vec3f(), coords, b.Convert<f32>(args[3]))->Result());
                    if (args.size() > 4) {
                        params.Push(args[4]);
                    }
                    break;
                case core::type::TextureDimension::k3d:
                case core::type::TextureDimension::kCube:
                    params.Push(coords);
                    if (args.size() > 3) {
                        params.Push(args[3]);
                    }
                    break;
                case core::type::TextureDimension::kCubeArray:
                    params.Push(b.Construct(ty.vec4f(), coords, b.Convert<f32>(args[3]))->Result());
                    break;
                default:
                    TINT_IR_UNREACHABLE(ir);
            }

            core::ir::Instruction* result = b.MemberCall<hlsl::ir::MemberBuiltinCall>(
                ty.vec4f(), hlsl::BuiltinFn::kSample, tex, params);
            if (tex_type->Is<core::type::DepthTexture>()) {
                // Swizzle x from vec4 result for depth textures
                TINT_IR_ASSERT(ir, call->Result()->Type()->Is<core::type::F32>());
                result = b.Swizzle(ty.f32(), result, {0});
            }
            result->SetResult(call->DetachResult());
        });
        call->Destroy();
    }

    void TextureSampleBias(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            core::ir::Value* tex = args[0];

            Vector<core::ir::Value*, 4> params;

            auto* tex_type = tex->Type()->As<core::type::Texture>();
            TINT_IR_ASSERT(ir, tex_type);

            params.Push(args[1]);  // sampler
            core::ir::Value* coords = args[2];

            switch (tex_type->Dim()) {
                case core::type::TextureDimension::k2d:
                    params.Push(coords);
                    params.Push(args[3]);  // bias

                    if (args.size() > 4) {
                        params.Push(args[4]);
                    }
                    break;
                case core::type::TextureDimension::k2dArray:
                    params.Push(b.Construct(ty.vec3f(), coords, b.Convert<f32>(args[3]))->Result());
                    params.Push(args[4]);

                    if (args.size() > 5) {
                        params.Push(args[5]);
                    }
                    break;
                case core::type::TextureDimension::k3d:
                case core::type::TextureDimension::kCube:
                    params.Push(coords);
                    params.Push(args[3]);

                    if (args.size() > 4) {
                        params.Push(args[4]);
                    }
                    break;
                case core::type::TextureDimension::kCubeArray:
                    params.Push(b.Construct(ty.vec4f(), coords, b.Convert<f32>(args[3]))->Result());
                    params.Push(args[4]);
                    break;
                default:
                    TINT_IR_UNREACHABLE(ir);
            }

            b.MemberCallWithResult<hlsl::ir::MemberBuiltinCall>(
                call->DetachResult(), hlsl::BuiltinFn::kSampleBias, tex, params);
        });
        call->Destroy();
    }

    void TextureSampleCompare(core::ir::CoreBuiltinCall* call) {
        hlsl::BuiltinFn fn = call->Func() == core::BuiltinFn::kTextureSampleCompare
                                 ? hlsl::BuiltinFn::kSampleCmp
                                 : hlsl::BuiltinFn::kSampleCmpLevelZero;

        auto args = call->Args();
        b.InsertBefore(call, [&] {
            core::ir::Value* tex = args[0];

            Vector<core::ir::Value*, 4> params;

            auto* tex_type = tex->Type()->As<core::type::Texture>();
            TINT_IR_ASSERT(ir, tex_type);

            params.Push(args[1]);  // sampler
            core::ir::Value* coords = args[2];

            switch (tex_type->Dim()) {
                case core::type::TextureDimension::k2d:
                    params.Push(coords);
                    params.Push(args[3]);  // depth ref

                    if (args.size() > 4) {
                        params.Push(args[4]);
                    }
                    break;
                case core::type::TextureDimension::k2dArray:
                    params.Push(b.Construct(ty.vec3f(), coords, b.Convert<f32>(args[3]))->Result());
                    params.Push(args[4]);

                    if (args.size() > 5) {
                        params.Push(args[5]);
                    }
                    break;
                case core::type::TextureDimension::kCube:
                    params.Push(coords);
                    params.Push(args[3]);

                    if (args.size() > 4) {
                        params.Push(args[4]);
                    }
                    break;
                case core::type::TextureDimension::kCubeArray:
                    params.Push(b.Construct(ty.vec4f(), coords, b.Convert<f32>(args[3]))->Result());
                    params.Push(args[4]);
                    break;
                default:
                    TINT_IR_UNREACHABLE(ir);
            }

            b.MemberCallWithResult<hlsl::ir::MemberBuiltinCall>(call->DetachResult(), fn, tex,
                                                                params);
        });
        call->Destroy();
    }

    void TextureSampleGrad(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            core::ir::Value* tex = args[0];

            Vector<core::ir::Value*, 4> params;

            auto* tex_type = tex->Type()->As<core::type::Texture>();
            TINT_IR_ASSERT(ir, tex_type);

            params.Push(args[1]);  // sampler
            core::ir::Value* coords = args[2];

            switch (tex_type->Dim()) {
                case core::type::TextureDimension::k2d:
                    params.Push(coords);
                    params.Push(args[3]);  // ddx
                    params.Push(args[4]);  // ddy

                    if (args.size() > 5) {
                        params.Push(args[5]);
                    }
                    break;
                case core::type::TextureDimension::k2dArray:
                    params.Push(b.Construct(ty.vec3f(), coords, b.Convert<f32>(args[3]))->Result());
                    params.Push(args[4]);
                    params.Push(args[5]);

                    if (args.size() > 6) {
                        params.Push(args[6]);
                    }
                    break;
                case core::type::TextureDimension::k3d:
                case core::type::TextureDimension::kCube:
                    params.Push(coords);
                    params.Push(args[3]);
                    params.Push(args[4]);

                    if (args.size() > 5) {
                        params.Push(args[5]);
                    }
                    break;
                case core::type::TextureDimension::kCubeArray:
                    params.Push(b.Construct(ty.vec4f(), coords, b.Convert<f32>(args[3]))->Result());
                    params.Push(args[4]);
                    params.Push(args[5]);
                    break;
                default:
                    TINT_IR_UNREACHABLE(ir);
            }

            b.MemberCallWithResult<hlsl::ir::MemberBuiltinCall>(
                call->DetachResult(), hlsl::BuiltinFn::kSampleGrad, tex, params);
        });
        call->Destroy();
    }

    void TextureSampleLevel(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            core::ir::Value* tex = args[0];

            Vector<core::ir::Value*, 4> params;

            auto* tex_type = tex->Type()->As<core::type::Texture>();
            TINT_IR_ASSERT(ir, tex_type);

            params.Push(args[1]);  // sampler
            core::ir::Value* coords = args[2];

            switch (tex_type->Dim()) {
                case core::type::TextureDimension::k1d:
                case core::type::TextureDimension::k2d:
                    params.Push(coords);
                    params.Push(b.InsertConvertIfNeeded(ty.f32(), args[3]));  // Level

                    if (args.size() > 4) {
                        params.Push(args[4]);
                    }
                    break;
                case core::type::TextureDimension::k2dArray:
                    params.Push(b.Construct(ty.vec3f(), coords, b.Convert<f32>(args[3]))->Result());
                    params.Push(b.InsertConvertIfNeeded(ty.f32(), args[4]));  // Level
                    if (args.size() > 5) {
                        params.Push(args[5]);
                    }
                    break;
                case core::type::TextureDimension::k3d:
                case core::type::TextureDimension::kCube:
                    params.Push(coords);
                    params.Push(b.InsertConvertIfNeeded(ty.f32(), args[3]));  // Level

                    if (args.size() > 4) {
                        params.Push(args[4]);
                    }
                    break;
                case core::type::TextureDimension::kCubeArray:
                    params.Push(b.Construct(ty.vec4f(), coords, b.Convert<f32>(args[3]))->Result());
                    params.Push(b.InsertConvertIfNeeded(ty.f32(), args[4]));  // Level
                    break;
                default:
                    TINT_IR_UNREACHABLE(ir);
            }

            core::ir::Instruction* result = b.MemberCall<hlsl::ir::MemberBuiltinCall>(
                ty.vec4f(), hlsl::BuiltinFn::kSampleLevel, tex, params);
            if (tex_type->Is<core::type::DepthTexture>()) {
                // Swizzle x from vec4 result for depth textures
                TINT_IR_ASSERT(ir, call->Result()->Type()->Is<core::type::F32>());
                result = b.Swizzle(ty.f32(), result, {0});
            }
            result->SetResult(call->DetachResult());
        });
        call->Destroy();
    }

    void Pack2x16Float(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();

        b.InsertBefore(call, [&] {
            auto* bc =
                b.Call<hlsl::ir::BuiltinCall>(ty.vec2u(), hlsl::BuiltinFn::kF32Tof16, args[0]);

            auto* lower = b.Swizzle(ty.u32(), bc, {0});
            auto* upper = b.ShiftLeft(b.Swizzle(ty.u32(), bc, {1}), 16_u);
            auto* res = b.Or(lower, upper);
            call->Result()->ReplaceAllUsesWith(res->Result());
        });
        call->Destroy();
    }

    void Unpack2x16Float(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* x = b.And(args[0], 0xffff_u);
            auto* y = b.ShiftRight(args[0], 16_u);
            auto* conv = b.Construct(ty.vec2u(), x, y);

            b.CallWithResult<hlsl::ir::BuiltinCall>(call->DetachResult(),
                                                    hlsl::BuiltinFn::kF16Tof32, conv);
        });
        call->Destroy();
    }

    void Pack2x16Snorm(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* clamp_lower = b.Splat(ty.vec2f(), -1_f);
            auto* clamp_upper = b.Splat(ty.vec2f(), 1_f);
            auto* clamp = b.Clamp(args[0], clamp_lower, clamp_upper);
            auto* mul = b.Multiply(clamp, 32767_f);
            auto* round = b.Call(ty.vec2f(), core::BuiltinFn::kRound, mul);
            auto* conv = b.Convert(ty.vec2i(), round);
            auto* res = b.And(conv, b.Splat(ty.vec2i(), 0xffff_i));

            auto* lower = b.Swizzle(ty.i32(), res, {0});
            auto* upper = b.ShiftLeft(b.Swizzle(ty.i32(), res, {1}), 16_u);

            b.CallWithResult<hlsl::ir::BuiltinCall>(call->DetachResult(), hlsl::BuiltinFn::kAsuint,
                                                    b.Or(lower, upper));
        });
        call->Destroy();
    }

    void Unpack2x16Snorm(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* conv = b.Convert(ty.i32(), args[0]);
            auto* x = b.ShiftLeft(conv, 16_u);

            auto* vec = b.Construct(ty.vec2i(), x, conv);
            auto* v = b.ShiftRight(vec, b.Composite(ty.vec2u(), 16_u));

            auto* flt = b.Convert(ty.vec2f(), v);
            auto* scale = b.Divide(flt, 32767_f);

            auto* lower = b.Splat(ty.vec2f(), -1_f);
            auto* upper = b.Splat(ty.vec2f(), 1_f);
            b.Clamp(scale, lower, upper)->SetResult(call->DetachResult());
        });
        call->Destroy();
    }

    void Pack2x16Unorm(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* clamp_lower = b.Splat(ty.vec2f(), 0_f);
            auto* clamp_upper = b.Splat(ty.vec2f(), 1_f);
            auto* clamp = b.Clamp(args[0], clamp_lower, clamp_upper);
            auto* mul = b.Multiply(clamp, 65535_f);
            auto* round = b.Call(ty.vec2f(), core::BuiltinFn::kRound, mul);
            auto* conv = b.Convert(ty.vec2u(), round);
            auto* lower = b.Swizzle(ty.u32(), conv, {0});
            auto* upper = b.ShiftLeft(b.Swizzle(ty.u32(), conv, {1}), 16_u);
            auto* result = b.Or(lower, upper);
            call->Result()->ReplaceAllUsesWith(result->Result());
        });
        call->Destroy();
    }

    void Unpack2x16Unorm(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* x = b.And(args[0], 0xffff_u);
            auto* y = b.ShiftRight(args[0], 16_u);
            auto* conv = b.Construct(ty.vec2u(), x, y);
            auto* flt_conv = b.Convert(ty.vec2f(), conv);
            auto* scale = b.Divide(flt_conv, 0xffff_f);

            call->Result()->ReplaceAllUsesWith(scale->Result());
        });
        call->Destroy();
    }

    void Pack4x8Snorm(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* clamp_lower = b.Splat(ty.vec4f(), -1_f);
            auto* clamp_upper = b.Splat(ty.vec4f(), 1_f);
            auto* clamp = b.Clamp(args[0], clamp_lower, clamp_upper);
            auto* mul = b.Multiply(clamp, 127_f);
            auto* round = b.Call(ty.vec4f(), core::BuiltinFn::kRound, mul);
            auto* conv = b.Convert(ty.vec4i(), round);
            auto* band = b.And(conv, b.Splat(ty.vec4i(), 0xff_i));
            auto* x = b.Swizzle(ty.i32(), band, {0});
            auto* y = b.ShiftLeft(b.Swizzle(ty.i32(), band, {1}), 8_u);
            auto* z = b.ShiftLeft(b.Swizzle(ty.i32(), band, {2}), 16_u);
            auto* w = b.ShiftLeft(b.Swizzle(ty.i32(), band, {3}), 24_u);
            b.CallWithResult<hlsl::ir::BuiltinCall>(call->DetachResult(), hlsl::BuiltinFn::kAsuint,
                                                    b.Or(x, b.Or(y, b.Or(z, w))));
        });
        call->Destroy();
    }

    void Unpack4x8Snorm(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* conv = b.Convert(ty.i32(), args[0]);
            auto* x = b.ShiftLeft(conv, 24_u);
            auto* y = b.ShiftLeft(conv, 16_u);
            auto* z = b.ShiftLeft(conv, 8_u);
            auto* cons = b.Construct(ty.vec4i(), x, y, z, conv);
            auto* shr = b.ShiftRight(cons, b.Composite(ty.vec4u(), 24_u));
            auto* flt = b.Convert(ty.vec4f(), shr);
            auto* scale = b.Divide(flt, 127_f);

            auto* lower = b.Splat(ty.vec4f(), -1_f);
            auto* upper = b.Splat(ty.vec4f(), 1_f);
            b.Clamp(scale, lower, upper)->SetResult(call->DetachResult());
        });
        call->Destroy();
    }

    void Pack4x8Unorm(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* clamp_lower = b.Splat(ty.vec4f(), 0_f);
            auto* clamp_upper = b.Splat(ty.vec4f(), 1_f);
            auto* clamp = b.Clamp(args[0], clamp_lower, clamp_upper);
            auto* mul = b.Multiply(clamp, 255_f);
            auto* round = b.Call(ty.vec4f(), core::BuiltinFn::kRound, mul);
            auto* conv = b.Convert(ty.vec4u(), round);
            auto* x = b.Swizzle(ty.u32(), conv, {0});
            auto* y = b.ShiftLeft(b.Swizzle(ty.u32(), conv, {1}), 8_u);
            auto* z = b.ShiftLeft(b.Swizzle(ty.u32(), conv, {2}), 16_u);
            auto* w = b.ShiftLeft(b.Swizzle(ty.u32(), conv, {3}), 24_u);
            auto* res = b.Or(x, b.Or(y, b.Or(z, w)));

            call->Result()->ReplaceAllUsesWith(res->Result());
        });
        call->Destroy();
    }

    void Unpack4x8Unorm(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* val = args[0];
            auto* x = b.And(val, 0xff_u);
            auto* y = b.And(b.ShiftRight(val, 8_u), 0xff_u);
            auto* z = b.And(b.ShiftRight(val, 16_u), 0xff_u);
            auto* w = b.ShiftRight(val, 24_u);
            auto* cons = b.Construct(ty.vec4u(), x, y, z, w);
            auto* conv = b.Convert(ty.vec4f(), cons);
            auto* scale = b.Divide(conv, 255_f);

            call->Result()->ReplaceAllUsesWith(scale->Result());
        });
        call->Destroy();
    }

    void Pack4xI8(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* type = ty.Get<hlsl::type::Int8T4Packed>();
            auto* packed = b.Call<hlsl::ir::BuiltinCall>(type, hlsl::BuiltinFn::kPackS8, args[0]);
            auto* conv = b.Call<hlsl::ir::BuiltinCall>(ty.u32(), hlsl::BuiltinFn::kConvert, packed);

            call->Result()->ReplaceAllUsesWith(conv->Result());
        });
        call->Destroy();
    }

    void Unpack4xI8(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* type = ty.Get<hlsl::type::Int8T4Packed>();
            auto* conv = b.CallExplicit<hlsl::ir::BuiltinCall>(
                type, hlsl::BuiltinFn::kConvert, Vector<core::ir::TemplateParameter, 1>{type},
                args[0]);

            b.CallWithResult<hlsl::ir::BuiltinCall>(call->DetachResult(),
                                                    hlsl::BuiltinFn::kUnpackS8S32, conv);
        });
        call->Destroy();
    }

    void Pack4xU8(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* type = ty.Get<hlsl::type::Uint8T4Packed>();
            auto* packed = b.Call<hlsl::ir::BuiltinCall>(type, hlsl::BuiltinFn::kPackU8, args[0]);
            auto* conv = b.Call<hlsl::ir::BuiltinCall>(ty.u32(), hlsl::BuiltinFn::kConvert, packed);

            call->Result()->ReplaceAllUsesWith(conv->Result());
        });
        call->Destroy();
    }

    void Unpack4xU8(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* type = ty.Get<hlsl::type::Uint8T4Packed>();
            auto* conv = b.CallExplicit<hlsl::ir::BuiltinCall>(
                type, hlsl::BuiltinFn::kConvert, Vector<core::ir::TemplateParameter, 1>{type},
                args[0]);

            b.CallWithResult<hlsl::ir::BuiltinCall>(call->DetachResult(),
                                                    hlsl::BuiltinFn::kUnpackU8U32, conv);
        });
        call->Destroy();
    }

    void Pack4xI8Clamp(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* type = ty.Get<hlsl::type::Int8T4Packed>();
            auto* packed =
                b.Call<hlsl::ir::BuiltinCall>(type, hlsl::BuiltinFn::kPackClampS8, args[0]);
            auto* conv = b.Call<hlsl::ir::BuiltinCall>(ty.u32(), hlsl::BuiltinFn::kConvert, packed);

            call->Result()->ReplaceAllUsesWith(conv->Result());
        });
        call->Destroy();
    }

    void Dot4I8Packed(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* acc = b.Var("accumulator", b.Zero(ty.i32()));
            b.CallWithResult<hlsl::ir::BuiltinCall>(
                call->DetachResult(), hlsl::BuiltinFn::kDot4AddI8Packed, args[0], args[1], acc);
        });
        call->Destroy();
    }

    void Dot4U8Packed(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        b.InsertBefore(call, [&] {
            auto* acc = b.Var("accumulator", b.Zero(ty.u32()));
            b.CallWithResult<hlsl::ir::BuiltinCall>(
                call->DetachResult(), hlsl::BuiltinFn::kDot4AddU8Packed, args[0], args[1], acc);
        });
        call->Destroy();
    }

    void Frexp(core::ir::CoreBuiltinCall* call) {
        auto arg = call->Args()[0];
        b.InsertBefore(call, [&] {
            auto* arg_ty = arg->Type();
            auto* arg_i32_ty = ty.MatchWidth(ty.i32(), arg_ty);
            // Note: WGSL's frexp expects an i32 for exp, but HLSL expects f32 (same type as first
            // arg), so we use a temp f32 var that we convert to i32 later.
            auto* exp_out = b.Var(ty.ptr<function>(arg_ty));
            // HLSL frexp writes exponent part to second out param, and returns the fraction
            // (mantissa) part.
            core::ir::Instruction* fract = b.Call<hlsl::ir::BuiltinCall>(
                arg_ty, hlsl::BuiltinFn::kFrexp, arg, b.Load(exp_out));
            // The returned fraction is always positive, but for WGSL, we want it to keep the sign
            // of the input value.
            auto* arg_sign = BuildSign(arg);
            fract = b.Multiply(arg_sign, fract);
            // Replace the call with new result struct
            b.ConstructWithResult(call->DetachResult(), fract,
                                  b.Convert(arg_i32_ty, b.Load(exp_out)));
        });
        call->Destroy();
    }

    void Modf(core::ir::CoreBuiltinCall* call) {
        auto arg = call->Args()[0];
        b.InsertBefore(call, [&] {
            auto* arg_ty = arg->Type();
            auto* whole = b.Var(ty.ptr<function>(arg_ty));
            // HLSL modf writes whole (integer) part to second out param, and returns the fractional
            // part.
            auto* call_result =
                b.Call<hlsl::ir::BuiltinCall>(arg_ty, hlsl::BuiltinFn::kModf, arg, b.Load(whole));
            // Replace the call with new result struct
            b.ConstructWithResult(call->DetachResult(), call_result, b.Load(whole));
        });
        call->Destroy();
    }

    void QuantizeToF16(core::ir::CoreBuiltinCall* call) {
        auto* u32_type = ty.MatchWidth(ty.u32(), call->Result()->Type());
        b.InsertBefore(call, [&] {
            auto* inner = b.Call<hlsl::ir::BuiltinCall>(u32_type, hlsl::BuiltinFn::kF32Tof16,
                                                        call->Args()[0]);
            b.CallWithResult<hlsl::ir::BuiltinCall>(call->DetachResult(),
                                                    hlsl::BuiltinFn::kF16Tof32, inner);
        });
        call->Destroy();
    }

    // Some HLSL methods do not support a signed int overload or the signed int overload has bugs.
    // This helper function wraps the argument in `asuint` and the result in `asint` to use the
    // unsigned int overload. It currently supports only single argument function signatures.
    void BitcastToIntOverloadCall(core::ir::CoreBuiltinCall* call) {
        TINT_IR_ASSERT(ir, call->Args().size() == 1);
        auto* arg = call->Args()[0];
        auto* arg_type = arg->Type()->UnwrapRef();
        if (arg_type->IsSignedIntegerScalarOrVector()) {
            auto* result_ty = call->Result()->Type();
            auto* u32_type = ty.MatchWidth(ty.u32(), result_ty);
            b.InsertBefore(call, [&] {
                core::ir::Value* val = arg;
                // Bitcast of literal int vectors fails in DXC so extract arg to a var. See
                // github.com/microsoft/DirectXShaderCompiler/issues/6851.
                if (arg_type->IsSignedIntegerVector() && arg->Is<core::ir::Constant>()) {
                    val = b.Let("arg", arg)->Result();
                }
                auto* inner =
                    b.Call<hlsl::ir::BuiltinCall>(u32_type, hlsl::BuiltinFn::kAsuint, val);
                auto* func = b.Call(u32_type, call->Func(), inner);
                b.CallWithResult<hlsl::ir::BuiltinCall>(call->DetachResult(),
                                                        hlsl::BuiltinFn::kAsint, func);
            });
            call->Destroy();
        }
    }

    // The following subgroup builtin functions are translated to HLSL as follows:
    // +---------------------+----------------------------------------------------------------+
    // |        WGSL         |                              HLSL                              |
    // +---------------------+----------------------------------------------------------------+
    // | subgroupShuffleXor  | WaveReadLaneAt with index equal subgroup_invocation_id ^ mask  |
    // | subgroupShuffleUp   | WaveReadLaneAt with index equal subgroup_invocation_id - delta |
    // | subgroupShuffleDown | WaveReadLaneAt with index equal subgroup_invocation_id + delta |
    // +---------------------+----------------------------------------------------------------+
    void SubgroupShuffle(core::ir::CoreBuiltinCall* call) {
        TINT_IR_ASSERT(ir, call->Args().size() == 2);

        b.InsertBefore(call, [&] {
            auto* id = b.Call<hlsl::ir::BuiltinCall>(ty.u32(), hlsl::BuiltinFn::kWaveGetLaneIndex);
            auto* arg2 = call->Args()[1];

            core::ir::Instruction* inst = nullptr;
            switch (call->Func()) {
                case core::BuiltinFn::kSubgroupShuffleXor:
                    inst = b.Xor(id, arg2);
                    break;
                case core::BuiltinFn::kSubgroupShuffleUp:
                    inst = b.Subtract(id, arg2);
                    break;
                case core::BuiltinFn::kSubgroupShuffleDown:
                    inst = b.Add(id, arg2);
                    break;
                default:
                    TINT_IR_UNREACHABLE(ir);
            }
            b.CallWithResult<hlsl::ir::BuiltinCall>(
                call->DetachResult(), hlsl::BuiltinFn::kWaveReadLaneAt, call->Args()[0], inst);
        });
        call->Destroy();
    }

    // The following subgroup builtin functions are translated to HLSL as follows:
    // +-----------------------+----------------------+
    // |        WGSL           |       HLSL           |
    // +-----------------------+----------------------+
    // | subgroupInclusiveAdd  | WavePrefixSum(x) + x |
    // | subgroupInclusiveMul  | WavePrefixMul(x) * x |
    // +-----------------------+----------------------+
    void SubgroupInclusive(core::ir::CoreBuiltinCall* call) {
        TINT_IR_ASSERT(ir, call->Args().size() == 1);
        b.InsertBefore(call, [&] {
            auto builtin_sel = core::BuiltinFn::kNone;

            switch (call->Func()) {
                case core::BuiltinFn::kSubgroupInclusiveAdd:
                    builtin_sel = core::BuiltinFn::kSubgroupExclusiveAdd;
                    break;
                case core::BuiltinFn::kSubgroupInclusiveMul:
                    builtin_sel = core::BuiltinFn::kSubgroupExclusiveMul;
                    break;
                default:
                    TINT_IR_UNREACHABLE(ir);
            }

            auto* arg1 = call->Args()[0];
            auto call_type = arg1->Type();
            auto* exclusive_call = b.Call<core::ir::CoreBuiltinCall>(call_type, builtin_sel, arg1);

            core::ir::Instruction* inst = nullptr;
            switch (call->Func()) {
                case core::BuiltinFn::kSubgroupInclusiveAdd:
                    inst = b.Add(exclusive_call, arg1);
                    break;
                case core::BuiltinFn::kSubgroupInclusiveMul:
                    inst = b.Multiply(exclusive_call, arg1);
                    break;
                default:
                    TINT_IR_UNREACHABLE(ir);
            }
            call->Result()->ReplaceAllUsesWith(inst->Result());
        });
        call->Destroy();
    }

    void SubgroupMatrixMultiply(core::ir::CoreBuiltinCall* call) {
        b.InsertBefore(call, [&] {
            auto* left = call->Args()[0];
            auto* right = call->Args()[1];
            auto* result_ty = call->Result()->Type();
            auto* sm_ty = result_ty->As<core::type::SubgroupMatrix>();
            TINT_IR_ASSERT(ir, sm_ty);

            b.CallExplicitWithResult<hlsl::ir::BuiltinCall>(
                call->DetachResult(), hlsl::BuiltinFn::kMultiply,
                Vector<core::ir::TemplateParameter, 1>{sm_ty->Type()}, left, right);
        });
        call->Destroy();
    }

    core::ir::Function* GetMatrixScalarHelper(const core::type::SubgroupMatrix* sm_ty,
                                              core::BinaryOp op) {
        return matrix_scalar_funcs_.GetOrAdd(
            MatrixScalarKey{{sm_ty, op}}, [&]() -> core::ir::Function* {
                auto* elem_ty = sm_ty->Type();
                auto* scalar_ty = elem_ty;
                if (elem_ty->Is<core::type::I8>()) {
                    scalar_ty = ty.i32();
                } else if (elem_ty->Is<core::type::U8>()) {
                    scalar_ty = ty.u32();
                }

                auto fn_name = b.ir.symbols.New("tint_subgroup_matrix_scalar_op").Name();
                auto* f = b.Function(fn_name, sm_ty);
                auto* m_param = b.FunctionParam("m", sm_ty);
                auto* s_param = b.FunctionParam("s", scalar_ty);
                f->SetParams({m_param, s_param});

                b.Append(f->Block(), [&] {
                    core::ir::Value* input = m_param;

                    // If the element type does not match the scalar type (e.g. 8-bit integer), then
                    // we need to upcast the matrix to the scalar type to ensure that the type we
                    // operate on is natively supported in HLSL.
                    // NOTE: This requires that the same geometries are supported with the wider
                    // component type, which is not guaranteed by the LinAlg API. This means that we
                    // have to filter out support for 8-bit matrices on the API side unless the
                    // device supports the equivalent geometry with the wider type.
                    if (scalar_ty != elem_ty) {
                        auto* casted_matrix_ty = ty.subgroup_matrix(
                            sm_ty->Kind(), scalar_ty, sm_ty->Columns(), sm_ty->Rows());
                        input = b.MemberCallExplicit<hlsl::ir::MemberBuiltinCall>(
                                     casted_matrix_ty, hlsl::BuiltinFn::kCast, m_param,
                                     Vector<core::intrinsic::TemplateParameter, 1>{scalar_ty})
                                    ->Result();
                    }

                    auto* res = b.Var<function>("result", input);
                    auto* length = b.MemberCall<hlsl::ir::MemberBuiltinCall>(
                        ty.u32(), hlsl::BuiltinFn::kLength, res);
                    b.LoopRange(0_u, length, 1_u, [&](core::ir::Value* idx) {
                        auto* val = b.MemberCall<hlsl::ir::MemberBuiltinCall>(
                            scalar_ty, hlsl::BuiltinFn::kGet, b.Load(res), idx);
                        auto* binary = b.Binary(op, scalar_ty, val, s_param);
                        b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), hlsl::BuiltinFn::kSet,
                                                                  res, idx, binary);
                    });

                    if (scalar_ty != elem_ty) {
                        // Downcast the matrix back to the original element type if needed.
                        b.Return(f, b.MemberCallExplicit<hlsl::ir::MemberBuiltinCall>(
                                        sm_ty, hlsl::BuiltinFn::kCast, b.Load(res),
                                        Vector<core::intrinsic::TemplateParameter, 1>{elem_ty}));
                    } else {
                        b.Return(f, b.Load(res));
                    }
                });
                return f;
            });
    }

    void SubgroupMatrixScalar(core::ir::CoreBuiltinCall* call, core::BinaryOp op) {
        auto* mat = call->Args()[0];
        auto* scalar = call->Args()[1];
        auto* sm_ty = mat->Type()->As<core::type::SubgroupMatrix>();

        auto* helper = GetMatrixScalarHelper(sm_ty, op);
        b.InsertBefore(call, [&] {  //
            b.CallWithResult(call->DetachResult(), helper, mat, scalar);
        });
        call->Destroy();
    }

    core::ir::Function* GetMatrixMultiplyAccumulateHelper(
        const core::type::SubgroupMatrix* res_ty,
        const core::type::SubgroupMatrix* left_ty,
        const core::type::SubgroupMatrix* right_ty) {
        return matrix_multiply_accumulate_funcs_.GetOrAdd(
            MatrixMultiplyAccumulateKey{{res_ty, left_ty}}, [&]() -> core::ir::Function* {
                auto fn_name = b.ir.symbols.New("tint_MatrixMultiplyAccumulate").Name();
                auto* f = b.Function(fn_name, res_ty);
                auto* a_param = b.FunctionParam("a", left_ty);
                auto* b_param = b.FunctionParam("b", right_ty);
                auto* c_param = b.FunctionParam("c", res_ty);
                f->SetParams({a_param, b_param, c_param});

                b.Append(f->Block(), [&] {
                    auto* acc = b.Var("acc", c_param);
                    b.MemberCall<hlsl::ir::MemberBuiltinCall>(
                        ty.void_(), hlsl::BuiltinFn::kMultiplyAccumulate, acc, a_param, b_param);
                    b.Return(f, b.Load(acc));
                });
                return f;
            });
    }

    void SubgroupMatrixMultiplyAccumulate(core::ir::CoreBuiltinCall* call) {
        auto* left = call->Args()[0];
        auto* right = call->Args()[1];
        auto* acc = call->Args()[2];

        auto* left_ty = left->Type()->As<core::type::SubgroupMatrix>();
        auto* right_ty = right->Type()->As<core::type::SubgroupMatrix>();
        auto* res_ty = call->Result()->Type()->As<core::type::SubgroupMatrix>();

        auto* helper = GetMatrixMultiplyAccumulateHelper(res_ty, left_ty, right_ty);

        b.InsertBefore(call,
                       [&] { b.CallWithResult(call->DetachResult(), helper, left, right, acc); });
        call->Destroy();
    }

    core::ir::Constant* ColMajorToMatrixLayout(core::ir::Value* col_major) {
        auto* const_col_major = col_major->As<core::ir::Constant>();
        TINT_IR_ASSERT(ir, const_col_major);
        return b.Constant(ir.constant_values.Get<core::constant::Scalar<u32>>(
            ty.Get<type::MatrixLayout>(),
            u32(const_col_major->Value()->ValueAs<bool>() ? type::MatrixLayoutEnum::kColMajor
                                                          : type::MatrixLayoutEnum::kRowMajor)));
    }

    void SubgroupMatrixLoad(core::ir::CoreBuiltinCall* call) {
        auto* ptr = call->Args()[0];
        auto* ptr_ty = ptr->Type()->As<core::type::Pointer>();
        uint32_t arr_stride = ptr_ty->StoreType()->As<core::type::Array>()->ImplicitStride();
        TINT_IR_ASSERT(ir, ptr_ty);
        if (ptr_ty->AddressSpace() != core::AddressSpace::kWorkgroup) {
            return;
        }

        b.InsertBefore(call, [&] {
            TINT_IR_ASSERT(ir, call->ExplicitTemplateParams().Length() == 2);
            auto* offset = call->Args()[1];
            auto* stride = call->Args()[2];

            auto* sm_ty = call->Result()->Type()->As<core::type::SubgroupMatrix>();
            TINT_IR_ASSERT(ir, sm_ty);

            // Workgroup load offset and stride are in terms of the matrix element type.
            // Workgroup function only take u32 args.
            if (auto* const_offset = offset->As<core::ir::Constant>()) {
                offset = b.Constant(u32(const_offset->Value()->ValueAs<uint32_t>() * arr_stride /
                                        sm_ty->Type()->Size()));
            } else {
                offset =
                    b.Call<hlsl::ir::BuiltinCall>(ty.u32(), BuiltinFn::kAsuint, offset)->Result();
                offset = b.Multiply(offset, u32(arr_stride / sm_ty->Type()->Size()))->Result();
            }
            if (auto* const_stride = stride->As<core::ir::Constant>()) {
                stride = b.Constant(u32(const_stride->Value()->ValueAs<uint32_t>() * arr_stride /
                                        sm_ty->Type()->Size()));
            } else {
                stride =
                    b.Call<hlsl::ir::BuiltinCall>(ty.u32(), BuiltinFn::kAsuint, stride)->Result();
                stride = b.Multiply(stride, u32(arr_stride / sm_ty->Type()->Size()))->Result();
            }

            // TODO(crbug.com/512455144): 8-bit components need additional work to support.
            if (sm_ty->Type()->IsAnyOf<core::type::I8, core::type::U8>()) {
                TINT_IR_UNIMPLEMENTED(ir)
                    << "8-bit subgroup matrix load from workgroup not supported";
            }

            TINT_IR_ASSERT(
                ir, std::holds_alternative<core::Majorness>(call->ExplicitTemplateParams()[1]));
            auto* col_major =
                b.Constant(std::get<core::Majorness>(call->ExplicitTemplateParams()[1]) ==
                           core::Majorness::kColMajor);
            auto* layout = ColMajorToMatrixLayout(col_major);

            b.CallExplicitWithResult<hlsl::ir::BuiltinCall>(
                call->DetachResult(), hlsl::BuiltinFn::kLoad,
                Vector<core::ir::TemplateParameter, 1>{sm_ty}, ptr, offset, stride, layout);
        });
        call->Destroy();
    }

    void SubgroupMatrixStore(core::ir::CoreBuiltinCall* call) {
        auto* ptr = call->Args()[0];
        auto* ptr_ty = ptr->Type()->As<core::type::Pointer>();
        uint32_t arr_stride = ptr_ty->StoreType()->As<core::type::Array>()->ImplicitStride();
        TINT_IR_ASSERT(ir, ptr_ty);

        if (ptr_ty->AddressSpace() != core::AddressSpace::kWorkgroup) {
            return;
        }

        b.InsertBefore(call, [&] {
            TINT_IR_ASSERT(ir, call->ExplicitTemplateParams().Length() == 1);
            auto* offset = call->Args()[1];
            auto* matrix = call->Args()[2];
            auto* stride = call->Args()[3];

            auto* sm_ty = matrix->Type()->As<core::type::SubgroupMatrix>();
            TINT_IR_ASSERT(ir, sm_ty);

            // Workgroup store offset and stride are in terms of the matrix element type.
            // Workgroup function only take u32 args.
            if (auto* const_offset = offset->As<core::ir::Constant>()) {
                offset = b.Constant(u32(const_offset->Value()->ValueAs<uint32_t>() * arr_stride /
                                        sm_ty->Type()->Size()));
            } else {
                offset =
                    b.Call<hlsl::ir::BuiltinCall>(ty.u32(), BuiltinFn::kAsuint, offset)->Result();
                offset = b.Multiply(offset, u32(arr_stride / sm_ty->Type()->Size()))->Result();
            }
            if (auto* const_stride = stride->As<core::ir::Constant>()) {
                stride = b.Constant(u32(const_stride->Value()->ValueAs<uint32_t>() * arr_stride /
                                        sm_ty->Type()->Size()));
            } else {
                stride =
                    b.Call<hlsl::ir::BuiltinCall>(ty.u32(), BuiltinFn::kAsuint, stride)->Result();
                stride = b.Multiply(stride, u32(arr_stride / sm_ty->Type()->Size()))->Result();
            }

            // TODO(crbug.com/512455144): 8-bit components need additional work to support.
            if (sm_ty->Type()->IsAnyOf<core::type::I8, core::type::U8>()) {
                TINT_IR_UNIMPLEMENTED(ir)
                    << "8-bit subgroup matrix store to workgroup not supported";
            }

            TINT_IR_ASSERT(
                ir, std::holds_alternative<core::Majorness>(call->ExplicitTemplateParams()[0]));
            auto* col_major =
                b.Constant(std::get<core::Majorness>(call->ExplicitTemplateParams()[0]) ==
                           core::Majorness::kColMajor);
            auto* layout = ColMajorToMatrixLayout(col_major);

            b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), hlsl::BuiltinFn::kStore, matrix,
                                                      ptr, offset, stride, layout);
        });
        call->Destroy();
    }

    void AddSat(core::ir::CoreBuiltinCall* call) {
        auto* type = call->Result()->Type();
        core::ir::CoreBuiltinCall* select = nullptr;
        b.InsertBefore(call, [&] {
            auto* add = b.Add(call->Args()[0], call->Args()[1]);
            auto* lt = b.LessThan(add, call->Args()[0]);
            core::ir::Value* sat = (type->Is<core::type::Vector>() ? b.Splat(type, u32(0xffffffff))
                                                                   : b.Constant(u32(0xffffffff)));
            select = b.CallWithResult(call->DetachResult(), core::BuiltinFn::kSelect, add, sat, lt);
        });
        call->Destroy();
        Select(select);
    }
};

}  // namespace

Result<SuccessType> BuiltinPolyfill(core::ir::Module& ir, const BuiltinPolyfillConfig& config) {
    AssertValid(ir, "before hlsl.BuiltinPolyfill");

    State{ir, config}.Process();

    ir.properties.Add(core::ir::Property::kAllowNonCoreTypes);
    ir.properties.Add(core::ir::Property::kAllowVectorElementPointer);

    return Success;
}

}  // namespace tint::hlsl::writer::raise
